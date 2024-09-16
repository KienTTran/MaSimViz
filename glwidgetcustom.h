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
    // void paintEvent(QPaintEvent *event) override;

private:
    void setupShaders();
    void checkOpenGLError(const QString &functionName);
    void setRasterColor(const QVector3D &color);
    void renderTextAboveSquare(QPainter &painter, int col, int row, float x, float y);

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
    std::tuple<float,float,float> lastView;
    std::tuple<float,float,float> lastProjection;
    float panX;
    float panY;
    float aspectRatio;

    bool panning;
    float prevXForPan;
    float prevYForPan;
    float distance;


public:
    VizData *vizData;
    float pixelScale;
    float initPixelScaleX;
    float initPixelScaleY;
    float pixelScaleX;
    float pixelScaleY;
    bool inspectMode;

public:
    void updateInstanceData();
    void updateInstanceDataAll();
    void updateInstanceDataMedian(int dataIndex, int month);
    void setupVertexBuffers();
    Q_INVOKABLE void updateVertexData();
    Q_INVOKABLE void updateVertexBuffers();

signals:
    void mouseMoved(const QPoint &pos);
};




#endif // GLWIDGETCUSTOM_H
