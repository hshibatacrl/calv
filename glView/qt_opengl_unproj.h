#ifndef __QT_OPENGL_UNPROJ_H__
#define __QT_OPENGL_UNPROJ_H__

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

class depthContext
{
    GLfloat *_depth;
    uint32_t *_depthi;
    int _width,_height;
    int _which_buf;

public:
    depthContext(int w, int h)
    {
        _which_buf=0;
        _width=w;
        _height=h;
        _depth=new GLfloat[w*h];
        _depthi=new uint32_t[w*h];
        memset(_depth,0,sizeof(GLfloat)*w*h);
        memset(_depthi,0,sizeof(uint32_t)*w*h);
    }
    virtual ~depthContext()
    {
        delete [] _depth;
        delete [] _depthi;
    }

    int get_depth(int x, int y, GLfloat &z)
    {
        if(_which_buf==1)
        {
            z=_depth[x+y*_width];
            return 1;
        }
        else if(_which_buf==2)
        {
            uint32_t zi=_depthi[x+y*_width];
            double zd=zi/(double)(0x00ffffff);
            z=(GLfloat)zd;
            return 1;
        }
        return 0;
    }

    int width() { return _width;}
    int height() { return _height;}
    void setWhich(int x) {_which_buf=x;}
    uint32_t *depthi() {return _depthi;}
    GLfloat *depth() {return _depth;}
};

class qt_opengl_depth : protected QOpenGLFunctions
{
    depthContext *context;
    QOpenGLFramebufferObject *fbo;
    QOpenGLTexture *tex;

public:
    qt_opengl_depth(depthContext *ctx);
    ~qt_opengl_depth();

    int read(void);

};


extern int qt_opengl_unproj(const QVector3D &window, const QMatrix4x4 &proj_modelview, const int *viewport, QVector3D &object);


#endif // __QT_OPENGL_UNPROJ_H__
