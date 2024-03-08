/*
MIT License

Copyright (c) 2021 WagonWheelRobotics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "qt_opengl_unproj.h"

#include <QDebug>

qt_opengl_depth::qt_opengl_depth(depthContext *ctx)
{
    initializeOpenGLFunctions();
    //makeCurrent()

    context=ctx;


    fbo=new QOpenGLFramebufferObject(ctx->width(),ctx->height(),QOpenGLFramebufferObject::Attachment::Depth,GL_TEXTURE_2D);
    tex = new QOpenGLTexture(QOpenGLTexture::Target::Target2D);

    if(!tex->create())
    {
        qDebug()<<"CREATE TEXTURE FAILED.";
    }
 //   tex->setSize(w,h);
 //   tex->setFormat(QOpenGLTexture::RGBA8_UNorm);
 //   tex->allocateStorage();

    if(!fbo->isValid())
    {
        qDebug()<<"FBO is not valid";
    }
#if 0
    qDebug()<< "FBO TEXTURE_ID#"<<fbo->texture();
    qDebug()<< "TEXTURE_ID#"<<tex->textureId();
#endif

    fbo->bind();
    tex->bind(0);

    context->setWhich(0);
}

int qt_opengl_depth::read(void)
{
/*
OpenGL ES2.0 does not support depth buffer reading by glReadPixels
fragment shader program should write depth value into RGBA color buffer like this:

    "   if(mode==4){\n"
    "    highp int depth,r,g,b;\n"
    "    depth=int(gl_FragCoord.z*16777216.0);\n"
    "    r=depth/16777216; depth=depth -r*16777216;\n"
    "    g=depth/65536;    depth=depth -g*65536;\n"
    "    b=depth/256;      depth=depth -b*256;\n"
    "    gl_FragColor = vec4(float(depth),float(b),float(g),float(r))/255.0;\n"
    "   }\n"
    "   else if
*/

    GLenum error;

    fbo->bind();
    tex->bind(0);


    //try color buffer mode (ES2)
    glReadPixels(0,0,context->width(),context->height(),GL_RGBA,GL_UNSIGNED_BYTE,context->depthi());
    error=glGetError();
    if(!error)
    {
        context->setWhich(2); //depthi is available
        return 1;
    }
#if 1
    if(error==0x502) qDebug() << "DEPTHi : glReadPixels = GL_INVALID_OPERATION";
    else             qDebug() << "DEPTHi : glReadPixels = " <<error;
#endif

    glReadPixels(0,0,context->width(),context->height(),GL_DEPTH_COMPONENT,GL_FLOAT,context->depth());
    error=glGetError();
    if(!error)
    {
        context->setWhich(1); //depth is available
        return 1;
    }
#if 1
    if(error==0x502) qDebug() << "DEPTH : glReadPixels = GL_INVALID_OPERATION";
    else             qDebug() << "DEPTH : glReadPixels = " <<error;
#endif


    return 0;
}

qt_opengl_depth::~qt_opengl_depth()
{
    tex->release();
    fbo->release();
    tex->destroy();
    delete tex;
    delete fbo;
    //doneCurrent()
}



int qt_opengl_unproj(const QVector3D &window, const QMatrix4x4 &proj_modelview, const int *viewport, QVector3D &object)
{
    int ret=0;
    if(window.z()>0.0f && window.z()<1.0f)
    {
        QVector4D pos_view;
        QMatrix4x4 inv_proj_modelview;
        QVector4D pos_obj;
        bool inv;

        pos_view.setX(2.0f *(window.x() - viewport[0])/viewport[2] - 1.0f);
        pos_view.setY(2.0f *(window.y() - viewport[1])/viewport[3] - 1.0f);
        pos_view.setZ(2.0f*window.z()-1.0f);
        pos_view.setW(1.0f);
        //proj_modelview=draw.proj * draw.camera * draw.world;
        inv_proj_modelview=proj_modelview.inverted(&inv);

        if(inv)
        {
            pos_obj = inv_proj_modelview * pos_view;

#if 0
            qDebug()<<pos_view;
            qDebug()<<proj_modelview;
            qDebug()<<pos_obj;
            qDebug()<<pos_obj.x() / pos_obj.w()<<pos_obj.y() / pos_obj.w()<<pos_obj.z() / pos_obj.w();
#endif
            object=QVector3D( pos_obj.x() / pos_obj.w(), pos_obj.y() / pos_obj.w(), pos_obj.z() / pos_obj.w() );
            ret=1;
//            update_by_poi();
//            update();
        }
        else
        {
            qDebug()<< "matrix invert error";
        }
    }
    return ret;
}
