#include "glwidgetcustom.h"
#include <QOpenGLShader>
#include <QRandomGenerator>
#include <QDebug>
#include <QIODevice>
#include <QFile>
#include <QMouseEvent>

// Vertex shader source code with instancing support
const char *vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 offset;
layout (location = 2) in vec3 instanceColor;  // Add instance color attribute

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 fragColor;  // Output color from raster data

void main() {
    // Apply model, view, and projection transformations
    gl_Position = projection * view * model * vec4(position.x + offset.x, position.y + offset.y, position.z, 1.0);
    // gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);
    fragColor = instanceColor;  // Pass the color to the fragment shader
    // fragColor = vec3(0.0f, 1.0f, 0.0f);  // Pass the color to the fragment shader
}
)";


// Fragment shader source code
const char *fragmentShaderSource = R"(
#version 410 core
in vec3 fragColor;  // Color passed from the vertex shader
out vec4 FragColor;

void main() {
    // FragColor = vec4(0.0f,1.0f,0.0f, 1.0);  // Use the color from rasterData
    FragColor = vec4(fragColor, 1.0);  // Use the color from rasterData
}
)";

GLWidgetCustom::GLWidgetCustom(QWidget *parent)
{
    setParent(parent);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    lastView = std::tuple<float,float,float>(1.0f,1.0f,1.0f);
    lastProjection = lastView;
    panX = 0.0f;
    panY = 0.0f;
    pixelScale = 0.0001f;
    initPixelScaleX = 0.0f;
    initPixelScaleY = 0.0f;
    pixelScaleX = 0.0f;
    pixelScaleY = 0.0f;
    panning = false;
    instanceCount = 0;
    prevXForPan = 0;
    prevYForPan = 0;
    vizData = new VizData();
    inspectMode = false;
}



GLWidgetCustom::~GLWidgetCustom()
{
    makeCurrent();
    vbo.destroy();
    instanceVBO.destroy();
    vao.destroy();
    delete shaderProgram;
    doneCurrent();
}

void GLWidgetCustom::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    setupShaders();
    setupVertexBuffers();

    // Initialize model, view, and projection matrices
    model.setToIdentity();
    view.setToIdentity();
    view.translate(0.0f, 0.0f, -2.0f);  // Move the camera back a bit
    projection.setToIdentity();

    // Output OpenGL context information for debugging
    qDebug() << "OpenGL context version:" << reinterpret_cast<const char*>(glGetString(GL_VERSION));
    qDebug() << "Vendor:" << reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    qDebug() << "Renderer:" << reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

void GLWidgetCustom::setupShaders()
{
    shaderProgram = new QOpenGLShaderProgram();

    // Compile shaders
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
        qDebug() << "Vertex shader compilation failed" << shaderProgram->log();
    if (!shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
        qDebug() << "Fragment shader compilation failed" << shaderProgram->log();

    // Link the shader program
    if (!shaderProgram->link())
        qDebug() << "Shader program linking failed" << shaderProgram->log();

    // Bind uniform locations for MVP matrices
    shaderProgram->bind();
    shaderProgram->uniformLocation("projection");
    shaderProgram->uniformLocation("view");
    shaderProgram->uniformLocation("model");
}


void GLWidgetCustom::setupVertexBuffers()
{
    vao.create();
    vao.bind();

    GLenum err;

    // Create and bind the VBO for the triangle vertices
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

    updateVertexData();  // Populate the vertex data


    qDebug() << "initPixelScaleX before: " << initPixelScaleX << " initPixelScaleY: " << initPixelScaleY;
    initPixelScaleX = pixelScaleX;
    initPixelScaleY = pixelScaleY;
    qDebug() << "initPixelScaleX after: " << initPixelScaleX << " initPixelScaleY: " << initPixelScaleY;

    // Allocate data to VBO
    vbo.allocate(vertices.constData(), vertices.size() * sizeof(float));

    // Check for errors after buffer allocation
    err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "Error in vertex VBO allocation:" << err;
    }

    // Set up the vertex position attribute (location 0)
    shaderProgram->enableAttributeArray(0);
    shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);  // 3 components per vertex (x, y, z)

    // Check for errors after setting the attribute
    err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "Error in setting vertex attribute pointer:" << err;
    }

    vbo.release();

    // Create the VBO for instance offsets
    instanceVBO.create();
    instanceVBO.bind();
    instanceVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);

    // Allocate instance data to VBO
    instanceVBO.allocate(instanceOffsets.constData(), instanceOffsets.size() * sizeof(float));

    // Check for errors after buffer allocation
    err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "Error in instance VBO allocation:" << err;
    }

    // Set up the instance offset attribute (location 1)
    shaderProgram->enableAttributeArray(1);
    shaderProgram->setAttributeBuffer(1, GL_FLOAT, 0, 2);  // 2 components per instance (x, y)
    glVertexAttribDivisor(1, 1);  // Tell OpenGL this is per-instance data

    // Check for errors after setting the attribute
    err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "Error in setting instance attribute pointer:" << err;
    }

    instanceVBO.release();

    // Now set up the instance color VBO
    instanceColorVBO.create();
    instanceColorVBO.bind();
    instanceColorVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);

    instanceColorVBO.allocate(instanceColors.constData(), instanceColors.size() * sizeof(QVector3D));

    // Check for errors after color buffer allocation
    err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "Error in color VBO allocation:" << err;
    }

    shaderProgram->enableAttributeArray(2);
    shaderProgram->setAttributeBuffer(2, GL_FLOAT, 0, 3);  // 3 components per color (r, g, b)
    glVertexAttribDivisor(2, 1);  // Per-instance color

    // Check for errors after setting the color attribute
    err = glGetError();
    if (err != GL_NO_ERROR) {
        qDebug() << "Error in setting color attribute pointer:" << err;
    }

    instanceColorVBO.release();

    vao.release();
}


void GLWidgetCustom::updateVertexData()
{
    // qDebug() << "updateVertexData aspectRatio: " << aspectRatio << " pixelScale: " << pixelScale;

    if(width() > height())
        aspectRatio = static_cast<double>(width()) / static_cast<double>(height());
    else
        aspectRatio = static_cast<double>(height()) / static_cast<double>(width());

    pixelScaleX = width() * pixelScale;
    pixelScaleY = height() * pixelScale;
    if(vizData->rasterData->raster->NCOLS > vizData->rasterData->raster->NROWS)
        pixelScaleX *= aspectRatio;
    else
        pixelScaleY *= aspectRatio;

    // Define a small square in normalized device coordinates (NDC)
    vertices.clear();  // Scale down the square size to fit more on the screen

    // First triangle (Top-left to bottom-right)
    // Vertex 1 (Top left)
    vertices.append(-0.5f * pixelScaleX);  // x
    vertices.append(0.5f * pixelScaleY);   // y
    vertices.append(0.0f);  // z

    // Vertex 2 (Bottom left)
    vertices.append(-0.5f * pixelScaleX);  // x
    vertices.append(-0.5f * pixelScaleY);  // y
    vertices.append(0.0f);   // z

    // Vertex 3 (Top right)
    vertices.append(0.5f * pixelScaleX);   // x
    vertices.append(0.5f * pixelScaleY);   // y
    vertices.append(0.0f);   // z

    // Second triangle (Bottom-left to top-right)
    // Vertex 4 (Bottom left - reused)
    vertices.append(-0.5f * pixelScaleX);  // x
    vertices.append(-0.5f * pixelScaleY);  // y
    vertices.append(0.0f);   // z

    // Vertex 5 (Bottom right)
    vertices.append(0.5f * pixelScaleX);   // x
    vertices.append(-0.5f * pixelScaleY);  // y
    vertices.append(0.0f);   // z

    // Vertex 6 (Top right - reused)
    vertices.append(0.5f * pixelScaleX);   // x
    vertices.append(0.5f * pixelScaleY);   // y
    vertices.append(0.0f);   // z
}


void GLWidgetCustom::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    // Update the projection matrix to account for window size (perspective projection)
    projection.setToIdentity();
    // double oldAspectRatio = aspectRatio;
    // aspectRatio = float(w) / float(h);
    // if (aspectRatio != oldAspectRatio) {
    //     qDebug() << "Aspect ratio changed from" << oldAspectRatio << "to" << aspectRatio << "at" << (aspectRatio/oldAspectRatio);
    //     if(aspectRatio > oldAspectRatio){
    //         pixelScale = pixelScale / (aspectRatio/oldAspectRatio);
    //     }
    //     else{
    //         pixelScale = pixelScale * (oldAspectRatio/aspectRatio);
    //     }
    // }
    projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);  // FOV, aspect ratio, near, far planes
}


void GLWidgetCustom::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!shaderProgram) return;

    shaderProgram->bind();

    if(inspectMode){
        model.setToIdentity();
        view.setToIdentity();
        view.translate(0.0f, 0.0f, -2.0f);
        projection.setToIdentity();
        aspectRatio = float(width()) / float(height());
        projection.perspective(45.0f, aspectRatio, 0.1f, 100.0f);
        panX = 0.0f;
        panY = 0.0f;

        qDebug() << "Inspect Mode";
    }

    // Pass MVP matrices to the shader
    shaderProgram->setUniformValue("projection", projection);
    shaderProgram->setUniformValue("view", view);
    shaderProgram->setUniformValue("model", model);

    vao.bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);  // Draw instances
    vao.release();

    shaderProgram->release();
}

void GLWidgetCustom::updateInstanceData()
{
    // qDebug() << "updateInstanceData aspectRatio: " << aspectRatio << " pixelScale: " << pixelScale;

    // Set instance offsets based on rasterData
    instanceOffsets.clear();
    instanceColors.clear();
    instanceCount = 0;  // Reset instance count

    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::min();

    // Calculate min and max values in rasterData to normalize the data
    for(int row = 0; row < vizData->rasterData->raster->NROWS; ++row) {
        for(int col = 0; col < vizData->rasterData->raster->NCOLS; ++col) {
            float value = vizData->rasterData->raster->data[row][col];
            if (value != vizData->rasterData->raster->NODATA_VALUE) {
                minValue = std::min(minValue, static_cast<float>(value));
                maxValue = std::max(maxValue, static_cast<float>(value));
            }
        }
    }

    // // OpenGL normalized device coordinate ranges are [-1, 1]
    // float screenWidth = width();
    // float screenHeight = height();

    // // double aspectRatio = 1.0;
    // if(screenWidth > screenHeight)
    //     aspectRatio = static_cast<double>(screenWidth) / static_cast<double>(screenHeight);
    // else
    //     aspectRatio = static_cast<double>(screenHeight) / static_cast<double>(screenWidth);

    // Loop through rasterData to get valid positions and set colors based on value
    for (int row = 0; row < vizData->rasterData->raster->NROWS; ++row) {
        for (int col = 0; col < vizData->rasterData->raster->NCOLS; ++col) {
            double value = vizData->rasterData->raster->data[row][col];

            // Only consider points that are not equal to nodata_value
            if (value != vizData->rasterData->raster->NODATA_VALUE) {
                // Calculate the offset position for each square based on pixel scale
                float offsetX = (-1.0f + (col + 0.5f) * initPixelScaleX);
                float offsetY = (1.0f - (row + 0.5f) * initPixelScaleY);

                // Add the offsets to the instanceOffsets list
                instanceOffsets.append(offsetX);
                instanceOffsets.append(offsetY);

                vizData->rasterData->locationPair1DTo2D[instanceCount] = std::make_pair(row, col);
                vizData->rasterData->locationPair2DTo1D[std::make_pair(row, col)] = instanceCount;
                // Increment instance count for each valid point
                instanceCount++;

                // Normalize the value to range [0, 1] based on min and max values
                float normalizedValue = (static_cast<float>(value) - minValue) / (maxValue - minValue);


                // Determine which color stop range this value falls into
                int nColorSteps = vizData->colorMap.size() - 1;
                float stepSize = 1.0f / nColorSteps;
                int lowerStep = qFloor(normalizedValue / stepSize);
                float factor = (normalizedValue - lowerStep * stepSize) / stepSize;

                // Ensure we don't go out of bounds
                if (lowerStep >= nColorSteps) {
                    lowerStep = nColorSteps - 1;
                    factor = 1.0f;
                }

                // Interpolate between the two adjacent colors
                QVector3D color = vizData->interpolate(lowerStep, factor);

                instanceColors.append(color);
            }
        }
    }

    qDebug() << "Number of valid data points:" << instanceCount;
}


void GLWidgetCustom::updateInstanceDataAll()
{
}

void GLWidgetCustom::updateVertexBuffers(){

    // qDebug() << "instanceVBO: " << instanceVBO.size() << " instanceColorVBO: " << instanceColorVBO.size();
    vao.bind();

    // Update the VBO for instance offsets
    instanceVBO.bind();
    instanceVBO.allocate(instanceOffsets.constData(), instanceOffsets.size() * sizeof(float));
    instanceVBO.write(0, instanceOffsets.constData(), instanceOffsets.size() * sizeof(float));
    instanceVBO.release();

    // Update the VBO for instance colors
    instanceColorVBO.bind();
    instanceColorVBO.allocate(instanceColors.constData(), instanceColors.size() * sizeof(QVector3D));
    instanceColorVBO.write(0, instanceColors.constData(), instanceColors.size() * sizeof(QVector3D));
    instanceColorVBO.release();

    vao.release();

    update();
}

void GLWidgetCustom::checkOpenGLError(const QString &functionName)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        qDebug() << "OpenGL error in" << functionName << ": " << err;
    }
}

// Mouse press event to handle panning initiation
void GLWidgetCustom::mousePressEvent(QMouseEvent *event)
{
    switch(event->button())
    {
    case Qt::LeftButton:
        {
            // Get the click coordinates in the widget
            int mouseX = event->x();
            int mouseY = event->y();

            // Get the width and height of the widget (OpenGL canvas)
            int widgetWidth = width();
            int widgetHeight = height();

            // Assuming the grid is rendered with fixed size squares and the whole widget is used for rendering
            int numRows = vizData->rasterData->raster->NROWS; // number of rows in your grid
            int numCols = vizData->rasterData->raster->NCOLS; // number of columns in your grid

            // Size of each square in terms of widget coordinates
            int squareWidth = widgetWidth / numCols;
            int squareHeight = widgetHeight / numRows;

            // Compute the row and column based on the click position
            int clickedCol = mouseX / squareWidth;
            int clickedRow = mouseY / squareHeight;

            // Note: The OpenGL coordinate system may require flipping the Y axis (depending on how you render)
            clickedRow = numRows - 1 - clickedRow; // If necessary to flip Y axis

            // Output the result (you can also trigger an event or update something on screen)
            qDebug() << "Clicked on square at: Row:" << clickedRow << " Col:" << clickedCol;

            emit mouseMoved(QPoint(clickedCol, clickedRow));
        }
        break;

    case Qt::RightButton:
        if (!panning)
        {
            panning = true;
            prevXForPan = event->position().x();
            prevYForPan = event->position().y();
        }
        break;

    default:
        break;
    }
}

// Mouse move event to handle panning
void GLWidgetCustom::mouseMoveEvent(QMouseEvent *event)
{
    if (!panning)
    {
        return;
    }

    float x = event->position().x();
    float y = event->position().y();

    // Retrieve the current distance (z-axis value) from the view matrix
    float distance = view.column(3).z();

    // Adjust the panning speed (the scaling factor is reduced for smoother panning)
    panX += (x - prevXForPan) * qFabs(distance) / 1000.f;
    panY += (y - prevYForPan) * qFabs(distance) / 1000.f;

    // Update the previous mouse position
    prevXForPan = x;
    prevYForPan = y;

    // Apply the pan by updating the view matrix's translation column
    view.setColumn(3, QVector4D(panX, -panY, distance, 1.f));

    // Trigger a repaint to reflect the changes
    update();
}

// Mouse release event to stop panning
void GLWidgetCustom::mouseReleaseEvent(QMouseEvent *event)
{
    if (panning)
    {
        panning = false;
    }
}

// Wheel event to handle zooming
void GLWidgetCustom::wheelEvent(QWheelEvent *event)
{
    // Adjust the zoom sensitivity
    float delta = event->angleDelta().y() / 250.f;  // Increase zoom sensitivity slightly for better response

    // Retrieve the current distance (z-axis value) from the view matrix
    float distance = view.column(3).z();

    // qDebug() << "Before Zoom: Distance =" << distance;

    // Modify the distance based on the zoom input (delta)
    distance += delta;

    // Clamp the zoom range to prevent too much zoom in/out
    distance = qMin(-1.0f, qMax(-20.0f, distance));  // Keep the distance negative for proper zooming

    // qDebug() << "After Zoom: Distance =" << distance;

    // Update only the Z translation of the view matrix to reflect zoom
    view.setColumn(3, QVector4D(panX, -panY, distance, 1.f));  // Preserve panX and panY during zoom

    // Trigger a repaint to reflect the changes
    update();
}


void GLWidgetCustom::updateInstanceDataMedian(QString colName, int month)
{
    // Set instance offsets based on rasterData
    instanceColors.clear();
    instanceCount = 0;  // Reset instance count

    // Helper function to linearly interpolate between two values
    auto interpolate = [](const QVector3D& color1, const QVector3D& color2, float factor) -> QVector3D {
        return (1.0f - factor) * color1 + factor * color2;
    };

    double minValue = vizData->statsData[colName].medianMin;
    double maxValue = vizData->statsData[colName].medianMax;

    // Loop through rasterData to get valid positions and set colors based on value
    for (int loc = 0; loc < vizData->statsData[colName].median[month].size(); ++loc) {
        double value = vizData->statsData[colName].median[month][loc];

        // Ensure value is within the valid range
        if (value < minValue) {
            value = minValue;
        } else if (value > maxValue) {
            value = maxValue;
        }

        // Determine which color stop range this value falls into based on the value itself
        int nColorSteps = vizData->colorMap.size() - 1;
        double stepSize = (maxValue - minValue) / nColorSteps;
        int lowerStep = qFloor((value - minValue) / stepSize);
        float factor = (value - (minValue + lowerStep * stepSize)) / stepSize;

        // Ensure we don't go out of bounds
        if (lowerStep >= nColorSteps) {
            lowerStep = nColorSteps - 1;
            factor = 1.0f;
        }

        // Interpolate between the two adjacent colors
        QVector3D color = vizData->interpolate(lowerStep,factor);

        instanceColors.append(color);

        // Increment instance count for each valid point
        instanceCount++;
    }
}








