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
    zoomFactor = 1.0f;
    panX = 0.0f;
    panY = 0.0f;
    panning = false;
    instanceCount = 0;
    setParent(parent);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::OpenHandCursor);
    prevXForPan = 0;
    prevYForPan = 0;
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

    // Initialize projection matrix (will be set in resizeGL())

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
    // Define a small square in normalized device coordinates (NDC)
    vertices.clear();

    float scale = 0.01f;  // Scale down the square size to fit more on the screen

    // First triangle (Top-left to bottom-right)
    // Vertex 1 (Top left)
    vertices.append(-0.5f * scale);  // x
    vertices.append(0.5f * scale);   // y
    vertices.append(0.0f);  // z

    // Vertex 2 (Bottom left)
    vertices.append(-0.5f * scale);  // x
    vertices.append(-0.5f * scale);  // y
    vertices.append(0.0f);   // z

    // Vertex 3 (Top right)
    vertices.append(0.5f * scale);   // x
    vertices.append(0.5f * scale);   // y
    vertices.append(0.0f);   // z

    // Second triangle (Bottom-left to top-right)
    // Vertex 4 (Bottom left - reused)
    vertices.append(-0.5f * scale);  // x
    vertices.append(-0.5f * scale);  // y
    vertices.append(0.0f);   // z

    // Vertex 5 (Bottom right)
    vertices.append(0.5f * scale);   // x
    vertices.append(-0.5f * scale);  // y
    vertices.append(0.0f);   // z

    // Vertex 6 (Top right - reused)
    vertices.append(0.5f * scale);   // x
    vertices.append(0.5f * scale);   // y
    vertices.append(0.0f);   // z
}


void GLWidgetCustom::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    // Update the projection matrix to account for window size (perspective projection)
    projection.setToIdentity();
    float aspect = float(w) / float(h);
    projection.perspective(45.0f, aspect, 0.1f, 100.0f);  // FOV, aspect ratio, near, far planes
}


void GLWidgetCustom::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!shaderProgram) return;

    shaderProgram->bind();

    // Pass MVP matrices to the shader
    shaderProgram->setUniformValue("projection", projection);
    shaderProgram->setUniformValue("view", view);
    shaderProgram->setUniformValue("model", model);

    vao.bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);  // Draw instances
    vao.release();

    shaderProgram->release();
}

QVector3D interpolate(const QVector3D& color1, const QVector3D& color2, float factor) {
    return (1.0f - factor) * color1 + factor * color2;
}

// Define extended color stops for a more detailed gradient
QVector<QVector3D> colorStops = {
    QVector3D(0.1f, 0.1f, 0.1f),  // Blue
    QVector3D(0.0f, 0.25f, 0.0f),  // Midpoint between Blue and Light Blue
    QVector3D(0.0f, 0.5f, 1.0f),  // Light Blue
    QVector3D(0.0f, 0.75f, 1.0f),  // Midpoint between Light Blue and Cyan
    QVector3D(0.0f, 1.0f, 1.0f),  // Cyan
    QVector3D(0.0f, 1.0f, 0.75f),  // Midpoint between Cyan and Light Green-Cyan
    QVector3D(0.0f, 1.0f, 0.5f),  // Light Green-Cyan
    QVector3D(0.0f, 1.0f, 0.25f),  // Midpoint between Light Green-Cyan and Green
    QVector3D(0.0f, 1.0f, 0.0f),  // Green
    QVector3D(0.25f, 1.0f, 0.0f),  // Midpoint between Green and Yellow-Green
    QVector3D(0.5f, 1.0f, 0.0f),  // Yellow-Green
    QVector3D(0.75f, 1.0f, 0.0f),  // Midpoint between Yellow-Green and Yellow
    QVector3D(1.0f, 1.0f, 0.0f),  // Yellow
    QVector3D(1.0f, 0.75f, 0.0f),  // Midpoint between Yellow and Orange
    QVector3D(1.0f, 0.5f, 0.0f),  // Orange
    QVector3D(1.0f, 0.25f, 0.0f),  // Midpoint between Orange and Red
    QVector3D(1.0f, 0.0f, 0.0f),  // Red
    QVector3D(0.75f, 0.0f, 0.25f),  // Midpoint between Red and Magenta-Purple
    QVector3D(0.5f, 0.0f, 0.5f),  // Magenta-Purple
    QVector3D(0.5f, 0.0f, 0.75f),  // Midpoint between Magenta-Purple and Purple
    QVector3D(0.5f, 0.0f, 1.0f),  // Purple
    // QVector3D(0.25f, 0.0f, 1.0f),  // Midpoint between Purple and Blue
    // QVector3D(0.0f, 0.0f, 1.0f)   // Blue
};

void GLWidgetCustom::updateInstanceData(VizData *vizData, int width, int height)
{
    // Set instance offsets based on rasterData
    instanceOffsets.clear();
    instanceColors.clear();
    instanceCount = 0;  // Reset instance count

    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::min();

    // Calculate min and max values in rasterData to normalize the data
    for (const auto &row : vizData->rasterData->values) {
        for (double value : row) {
            if (value != vizData->rasterData->nodata_value) {
                minValue = std::min(minValue, static_cast<float>(value));
                maxValue = std::max(maxValue, static_cast<float>(value));
            }
        }
    }

    // OpenGL normalized device coordinate ranges are [-1, 1]
    float screenWidth = width;
    float screenHeight = height;

    // Loop through rasterData to get valid positions and set colors based on value
    for (int row = 0; row < vizData->rasterData->values.size(); ++row) {
        for (int col = 0; col < vizData->rasterData->values[row].size(); ++col) {
            double value = vizData->rasterData->values[row][col];

            // Only consider points that are not equal to nodata_value
            if (value != vizData->rasterData->nodata_value) {
                // Scale col to OpenGL range [-1, 1]
                float offsetX = (2.0f * col / static_cast<float>(vizData->rasterData->ncols)) - 1.0f;
                // Scale row to OpenGL range [-1, 1] and invert y-axis
                float offsetY = 1.0f - (2.0f * row / static_cast<float>(vizData->rasterData->nrows));

                // Add the offsets to the instanceOffsets list
                instanceOffsets.append(offsetX);
                instanceOffsets.append(offsetY);

                vizData->rasterData->locationPair[instanceCount] = std::make_pair(row, col);
                // Increment instance count for each valid point
                instanceCount++;

                // Normalize the value to range [0, 1] based on min and max values
                float normalizedValue = (static_cast<float>(value) - minValue) / (maxValue - minValue);

                // Determine which color stop range this value falls into
                int nColorSteps = colorStops.size() - 1;
                float stepSize = 1.0f / nColorSteps;
                int lowerStep = qFloor(normalizedValue / stepSize);
                float factor = (normalizedValue - lowerStep * stepSize) / stepSize;

                // Ensure we don't go out of bounds
                if (lowerStep >= nColorSteps) {
                    lowerStep = nColorSteps - 1;
                    factor = 1.0f;
                }

                // Interpolate between the two adjacent colors
                QVector3D color = interpolate(colorStops[lowerStep], colorStops[lowerStep + 1], factor);

                instanceColors.append(color);
            }
        }
    }
    vizData->rasterData->locationRaster = instanceCount;
    qDebug() << "Number of valid data points:" << instanceCount;
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
    case Qt::RightButton:
        // Handle other mouse buttons if needed
        break;

    case Qt::LeftButton:
        if (!panning)
        {
            panning = true;
            setCursor(Qt::ClosedHandCursor);  // Change cursor when panning
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
        setCursor(Qt::OpenHandCursor);
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
    float delta = event->angleDelta().y() / 500.f;  // Increase zoom sensitivity slightly for better response

    // Retrieve the current distance (z-axis value) from the view matrix
    float distance = view.column(3).z();

    // qDebug() << "Before Zoom: Distance =" << distance;

    // Modify the distance based on the zoom input (delta)
    distance += delta;

    // Clamp the zoom range to prevent too much zoom in/out
    distance = qMin(-0.2f, qMax(-5.0f, distance));  // Keep the distance negative for proper zooming

    // qDebug() << "After Zoom: Distance =" << distance;

    // Update only the Z translation of the view matrix to reflect zoom
    view.setColumn(3, QVector4D(panX, -panY, distance, 1.f));  // Preserve panX and panY during zoom

    // Trigger a repaint to reflect the changes
    update();
}


void GLWidgetCustom::updateInstanceDataMedian(VizData *vizData, int dataIndex, int month)
{
    // Set instance offsets based on rasterData
    instanceColors.clear();
    instanceCount = 0;  // Reset instance count

    // Helper function to linearly interpolate between two values
    auto interpolate = [](const QVector3D& color1, const QVector3D& color2, float factor) -> QVector3D {
        return (1.0f - factor) * color1 + factor * color2;
    };

    double minValue = vizData->statsData[dataIndex].medianMin;
    double maxValue = vizData->statsData[dataIndex].medianMax;

    // Loop through rasterData to get valid positions and set colors based on value
    for (int loc = 0; loc < vizData->statsData[dataIndex].median[month].size(); ++loc) {
        double value = vizData->statsData[dataIndex].median[month][loc];

        // Ensure value is within the valid range
        if (value < minValue) {
            value = minValue;
        } else if (value > maxValue) {
            value = maxValue;
        }

        // Determine which color stop range this value falls into based on the value itself
        int nColorSteps = colorStops.size() - 1;
        double stepSize = (maxValue - minValue) / nColorSteps;
        int lowerStep = qFloor((value - minValue) / stepSize);
        float factor = (value - (minValue + lowerStep * stepSize)) / stepSize;

        // Ensure we don't go out of bounds
        if (lowerStep >= nColorSteps) {
            lowerStep = nColorSteps - 1;
            factor = 1.0f;
        }

        // Interpolate between the two adjacent colors
        QVector3D color = interpolate(colorStops[lowerStep], colorStops[lowerStep + 1], factor);

        instanceColors.append(color);

        // Increment instance count for each valid point
        instanceCount++;
    }
}








