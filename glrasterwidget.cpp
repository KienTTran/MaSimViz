// glrasterwidget.cpp
#include "glrasterwidget.h"

GLRasterWidget::GLRasterWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setMinimumSize(512, 512);
    frameTimer.setInterval(16); // ~60 FPS
    connect(&frameTimer, &QTimer::timeout, this, QOverload<>::of(&GLRasterWidget::update));
    frameTimer.start();
}

void GLRasterWidget::setData(const std::vector<float>& values, int width, int height, float minVal, float maxVal) {
    rasterData = values;
    rasterWidth = width;
    rasterHeight = height;
    minValue = minVal;
    maxValue = maxVal;
    update();
}

void GLRasterWidget::initializeGL() {
    initializeOpenGLFunctions();

    static const char* vShader = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        out vec2 texCoord;
        void main() {
            texCoord = (position + 1.0) / 2.0;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    static const char* fShader = R"(
        #version 330 core
        in vec2 texCoord;
        out vec4 fragColor;
        uniform sampler2D rasterTex;
        void main() {
            float val = texture(rasterTex, texCoord).r;
            vec3 color = vec3(0.1); // default gray
            if (val > 0.0) {
                color = vec3(val, 1.0 - val, 0.3 + 0.5 * val); // simple gradient
            }
            fragColor = vec4(color, 1.0);
        }
    )";

    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vShader);
    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fShader);
    shaderProgram.link();

    vao.create();
    vao.bind();

    vbo.create();
    vbo.bind();
    float quadVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f,  1.0f,
        1.0f,  1.0f
    };
    vbo.allocate(quadVertices, sizeof(quadVertices));
    shaderProgram.enableAttributeArray(0);
    shaderProgram.setAttributeBuffer(0, GL_FLOAT, 0, 2);

    vao.release();
    vbo.release();

    glGenTextures(1, &textureID);
}

void GLRasterWidget::paintGL() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if (rasterData.empty()) return;

    std::vector<float> normalized(rasterData.size());
    for (size_t i = 0; i < rasterData.size(); ++i) {
        normalized[i] = (rasterData[i] - minValue) / (maxValue - minValue);
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, rasterWidth, rasterHeight, 0, GL_RED, GL_FLOAT, normalized.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    shaderProgram.bind();
    shaderProgram.setUniformValue("rasterTex", 0);
    vao.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    vao.release();
    shaderProgram.release();
}

void GLRasterWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}
