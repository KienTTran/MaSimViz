#ifndef GLWIDGETCUSTOM_H
#define GLWIDGETCUSTOM_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>

#include <QMatrix4x4>  // Required for matrices
#include "vizdata.h"

class GLWidgetCustom : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:

public:
    GLWidgetCustom(QWidget *parent = nullptr);
    ~GLWidgetCustom();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Mouse interaction handlers
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupShaders();
    void checkOpenGLError(const QString &functionName);
    void setRasterColor(const QVector3D &color);

    QOpenGLShaderProgram *shaderProgram;
    QOpenGLBuffer vbo;
    QOpenGLBuffer instanceVBO;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer instanceColorVBO;

    QVector<float> vertices;
    QVector<float> instanceOffsets;
    QVector<QVector3D> instanceColors;

    int instanceCount;

    // Matrices for transformations
    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;

    // For panning and zooming
    QPoint lastMousePosition;
    float zoomFactor;
    float panX;
    float panY;
    float aspectRatio;

    bool panning;
    float prevXForPan;
    float prevYForPan;

public:
    VizData *vizData;
    float pixelScale;

public:
    void updateInstanceData(int width, int height);
    void updateInstanceDataAll(int width, int height);
    void updateInstanceDataMedian(int dataIndex, int month);
    void setupVertexBuffers();
    Q_INVOKABLE void updateVertexData();
    Q_INVOKABLE void updateVertexBuffers();
};




#endif // GLWIDGETCUSTOM_H
