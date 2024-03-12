
/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_stock_entity.h"

#include <QOpenGLShaderProgram>

gl_stock_entity::gl_stock_entity(QObject *parent):gl_entity_ctx(parent)
{
    vertex=NULL;
    prg=NULL;
    setObjectName("Stock");
}

gl_stock_entity::~gl_stock_entity()
{
    if(vertex!=NULL) delete [] vertex;
    if(prg!=NULL)
    {
        delete prg;
        prg=NULL;
    }
}

void gl_stock_entity::load(void)
{
    valid= n_vertex>0 && vertex!=NULL;
}


int gl_stock_entity::prepare_gl(void)
{
    qDebug() << "gl_stock_entity::prepare_gl";

    prg = new QOpenGLShaderProgram;
    prg->addShaderFromSourceCode(QOpenGLShader::Vertex,  get_vertex_shader() );
    prg->addShaderFromSourceCode(QOpenGLShader::Fragment,get_fragment_shader() );

    prg->bindAttributeLocation("vertex", 0);
    prg->bindAttributeLocation("color", 1);

    prg->link();
    prg->bind();

    projMat =prg->uniformLocation("projMatrix");
    mvMat   =prg->uniformLocation("mvMatrix");

    vbo_allocate();

    prg->release();

    //vertex is not necessary any more
    delete [] vertex;
    vertex=NULL;

    return 0;
}

void gl_stock_entity::vbo_bind(void)
{
    if(vbo.bind())
    {
        QOpenGLFunctions *fc = QOpenGLContext::currentContext()->functions();
        fc->glEnableVertexAttribArray(0);
        fc->glEnableVertexAttribArray(1);
        fc->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, n_element * sizeof(GLfloat), 0);    //vertex
        fc->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, n_element * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat))); //color
    }
    else
    {
        qDebug() << "VBO BIND ERROR";
    }
}

void gl_stock_entity::vbo_allocate(void)
{
    vbo.create();

    uint64_t m=MAX_VBO_SIZE;

    m/=sizeof(GLfloat);
    m/=n_element;
    if(n_vertex>m-1)
    {
        qDebug() << "number of vertex is overflow" << n_vertex;
        n_vertex=m-1;
        qDebug() << "number of vertex is turncated" << n_vertex;
    }

    int n=n_vertex*n_element*sizeof(GLfloat);
    qDebug() << "vbo_size" << n << "bytes.";

    if(vbo.bind())
    {
        vbo.allocate(&vertex[0], n);
        vbo.release();
    }
}


void gl_stock_entity::draw_gl(gl_draw_ctx_t &draw)
{
    if(!show()) return;

    GLfloat psz=2.0f;
    quint64 n=n_vertex;
    int mode=0;

    if(draw.mode==GL_DRAW_TEMP)
    {
        psz=1.0f;
    }
    else if(draw.mode==GL_DRAW_PICK)
    {
        if(!isPickable()) return;
        psz=10.0f;
        mode=OPT_PC_CM_DEPTH;
    }



    QOpenGLShaderProgram *p=prg;

    if(p)
    {
        QOpenGLFunctions *fc = QOpenGLContext::currentContext()->functions();

//        if(draw.mode!=GL_DRAW_NORMAL) fc->glDisable(GL_DEPTH_TEST);

        QMatrix4x4 modelview=draw.camera * draw.world * local;
        p->bind();
        vbo_bind();
        fc->glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        fc->glLineWidth(get_LineWidth());
        p->setUniformValue(projMat, draw.proj);
        p->setUniformValue(mvMat, modelview);
        p->setUniformValue("mode", mode);
        p->setUniformValue("pointsize", psz);
        fc->glDrawArrays(GL_LINES, 0, n);
        fc->glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        vbo.release();
        fc->glDisableVertexAttribArray(0);
        fc->glDisableVertexAttribArray(1);
        p->release();

//        if(draw.mode!=GL_DRAW_NORMAL) fc->glEnable(GL_DEPTH_TEST);
    }
}




const char *gl_stock_entity::get_vertex_shader(void) const
{

    static const char *vertexShaderSource =
        "attribute vec3 vertex;\n"
        "attribute vec3 color;\n"
        "varying highp vec3 vert;\n"
        "varying lowp vec3 col;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 mvMatrix;\n"
        "uniform highp int mode;\n"
        "uniform highp float pointsize;\n"
        "void main() {\n"
        "   vert = vertex;\n"
        "   col  = color;\n"
        "   gl_Position = projMatrix * mvMatrix * vec4(vertex, 1.0);\n"
        "   gl_PointSize = pointsize;\n"
        "}\n";

    return vertexShaderSource;
}

const char *gl_stock_entity::get_fragment_shader(void) const
{

    static const char *fragmentShaderSource =
        "varying highp vec3 vert;\n"
        "varying lowp vec3 col;\n"
        "uniform highp int mode;\n"
        "void main() {\n"
        "   if(mode==4){\n"
        "    highp int depth,r,g,b;\n"
//      "    depth=int(gl_FragCoord.z*4294967295.0);\n"
        "    depth=int(gl_FragCoord.z*16777216.0);\n"
        "    r=depth/16777216; depth=depth -r*16777216;\n"
        "    g=depth/65536;    depth=depth -g*65536;\n"
        "    b=depth/256;      depth=depth -b*256;\n"
        "    gl_FragColor = vec4(float(depth),float(b),float(g),float(r))/255.0;\n"
        "   }else{\n"
        "    gl_FragColor = vec4(col, 1.0);\n"
        "   }\n"
        "}\n";

    return fragmentShaderSource;
}
