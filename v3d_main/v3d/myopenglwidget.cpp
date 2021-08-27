#include "myopenglwidget.h"

static const char* VERTEX_SHADER_CODE =
        "#version 330 \n"
                "layout(location = 0) in vec3 posVertex;\n"
                "void main() {\n"
                "  gl_Position = vec4(posVertex, 1.0f);\n"
                "}\n";

static const char* FRAGMENT_SHADER_CODE =
        "#version 330\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "  fragColor = vec4(0.3f, 0.4f, 0.5f, 1.0f);\n"
        "}\n";

myopenglwidget::myopenglwidget(QWidget* parent)
    : QOpenGLWidget(parent)
{

}

myopenglwidget::~myopenglwidget()
{

}

void myopenglwidget::initializeGL()
{
    QOpenGLFunctions* f = this->context()->functions();
    m_shader = new QOpenGLShaderProgram();
    m_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER_CODE);
    m_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER_CODE);
    if (m_shader->link())
    {
        qDebug("Shaders link success.");
    }
    else
    {
        qDebug("Shaders link failed!");
    }

    //初始化VAO VBO
    m_vao = new QOpenGLVertexArrayObject();
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);

    m_vao->create();
    m_vao->bind();

    //点的坐标
    static const GLfloat VERTEX_DATA[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f
        };

    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(VERTEX_DATA, 3*3*sizeof (GLfloat));

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof (GLfloat), 0);

    m_vbo->release();
    m_vao->release();

}

void myopenglwidget::paintGL()
{
    QOpenGLFunctions *f = this->context()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    f->glClearColor(0.0f, 0.2f, 0.0f, 1.0f);

    m_vao->bind();
    m_shader->bind();

    f->glDrawArrays(GL_TRIANGLES, 0, 3);
    m_shader->release();
    m_vao->release();

}

void myopenglwidget::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

}

void myopenglwidget::showImg()
{
        myopenglwidget s = new myopenglwidget(this);
        s.resize(800,600);
        s.show();
}

