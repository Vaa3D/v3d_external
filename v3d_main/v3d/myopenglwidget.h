#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>



class myopenglwidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    myopenglwidget(QWidget* parent=0);
    ~myopenglwidget();


    //override
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    QOpenGLContext* myContext;
    QOpenGLBuffer *m_vbo;
    QOpenGLVertexArrayObject *m_vao;
    QOpenGLShaderProgram* m_shader;

public slots:
    void showImg();

};

#endif // MYOPENGLWIDGET_H
