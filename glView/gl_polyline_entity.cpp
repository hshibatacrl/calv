/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_polyline_entity.h"

#include <cmath>

#include <QOpenGLShaderProgram>
#include <QFileInfo>
#include <QThread>


gl_polyline_entity::gl_polyline_entity(QObject *parent):gl_entity_ctx(parent)
{
    setObjectName("polyline");
    _prg=nullptr;
    valid=0;
}

gl_polyline_entity::~gl_polyline_entity()
{

}


int gl_polyline_entity::load_mem(const uint8_t *buf, size_t length)
{
    //
    // Example code, have to re-implement for each application
    //

    int ret=0;

    double r=20.0;
    for(int i=0;i<360;i++)
    {
        double x = r*std::sin(i * M_PI/180.0);
        double y = r*std::cos(i * M_PI/180.0);
        double z = 1.0*std::sin(10.0*i * M_PI/180.0);
        QVector3D v(x,y,z);
        _vertices.append(v);
        ret++;
    }
    _chunks.append(ret/2);
    _chunks.append(ret/2);

    return ret;
}

void gl_polyline_entity::load(void)
{
    thread()->setPriority(QThread::LowPriority);
    valid=0;
    int r=0;
    QString targetFileName = info[ENTITY_INFO_TARGET_FILENAME].toString();
    QByteArray targetBytes = info[ENTITY_INFO_TARGET_BYTES].toByteArray();
    if(!targetFileName.isNull())
    {
        QFile f(targetFileName);
        QFileInfo fi(f);
        setObjectName(fi.baseName());
        if(f.open(QIODevice::ReadOnly))
        {
            auto m=f.map(0,f.size());
            r=load_mem((const uint8_t*)m,(size_t)(f.size()));
            f.unmap(m);
            f.close();
        }
    }
    else if(targetBytes.size())
    {
        r=load_mem((const uint8_t*)targetBytes.data(),(size_t)targetBytes.length());
    }
    else
    {
        qDebug()<<"gl_polyline_entity::load error";
    }

    if(r)
    {
        valid= _chunks.size()>0;
    }

    emit done(this);
}


static const char *get_vertex_shader(void)
{
    static const char *vertexShaderSource =
        "attribute vec3 pos;\n"
        "uniform mat4 mvpMatrix;\n"
        "void main() {\n"
        "   gl_Position = mvpMatrix * vec4(pos, 1.0);\n"
        "}\n";
    return vertexShaderSource;
}

static const char *get_fragment_shader(void)
{

    static const char *fragmentShaderSource =
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
        "    gl_FragColor = vec4(1.0,0.0,1.0, 1.0);\n"
        "   }\n"
        "}\n";

    return fragmentShaderSource;
}

int gl_polyline_entity::prepare_gl(void)
{
    //term_thread();

    _prg = new QOpenGLShaderProgram;

    _prg->addShaderFromSourceCode(QOpenGLShader::Vertex,  get_vertex_shader() );
    _prg->addShaderFromSourceCode(QOpenGLShader::Fragment,get_fragment_shader() );

    _prg->bindAttributeLocation("pos", 0);

    _prg->link();
    _prg->bind();

    _prg->release();

    if(_vbo.create())
    {
        if(_vbo.bind())
        {
            _vbo.allocate((const void *)_vertices.constData(), _vertices.size()*sizeof(GLfloat)*3);
            _vbo.release();
        }
    }

    return 0;
}

void gl_polyline_entity::vbo_bind(QOpenGLBuffer &vbo,QOpenGLFunctions *fc)
{
    if(vbo.bind())
    {
        fc->glEnableVertexAttribArray(0);
        fc->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    }
    else
    {
        qDebug() << "VBO BIND ERROR";
    }
}

void gl_polyline_entity::vbo_release(QOpenGLBuffer &vbo,QOpenGLFunctions *fc)
{
    vbo.release();
    fc->glDisableVertexAttribArray(0);
}

void gl_polyline_entity::draw_gl(gl_draw_ctx_t &draw)
{
    if(!show()) return;

    QMatrix4x4 offset;
    if(!originOffset(offset)) return;

    quint64 n=_vertices.size();
    int mode=0;

    if(draw.mode==GL_DRAW_TEMP)
    {
    }
    else if(draw.mode==GL_DRAW_PICK)
    {
        if(!isPickable()) return;
        mode=OPT_PC_CM_DEPTH;
    }

    QOpenGLShaderProgram *p=_prg;

    if(p)
    {
        QOpenGLFunctions *fc = QOpenGLContext::currentContext()->functions();

//        if(draw.mode!=GL_DRAW_NORMAL) fc->glDisable(GL_DEPTH_TEST);

        QMatrix4x4 modelviewProj=draw.proj * draw.camera * draw.world * offset * local;
        p->bind();
        vbo_bind(_vbo,fc);
        p->setUniformValue("mvpMatrix", modelviewProj);
        p->setUniformValue("mode", mode);
        fc->glLineWidth(1.0f);

        int m=0;
        foreach(int c,_chunks)
        {
            fc->glDrawArrays(GL_LINE_STRIP, m, c);
            m+=c;
        }

        vbo_release(_vbo,fc);
        p->release();

//        if(draw.mode!=GL_DRAW_NORMAL) fc->glEnable(GL_DEPTH_TEST);
    }
}
