#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QTimer>
#include <vector>

class GLRasterWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    GLRasterWidget(QWidget* parent = nullptr);
    void setData(const std::vector<float>& values, int width, int height, float minVal, float maxVal);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram shaderProgram;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo{ QOpenGLBuffer::VertexBuffer };
    QTimer frameTimer;
    GLuint textureID = 0;

    std::vector<float> rasterData;
    int rasterWidth = 0;
    int rasterHeight = 0;
    float minValue = 0.0f;
    float maxValue = 1.0f;
};
