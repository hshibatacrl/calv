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

#include "customImageWidget.h"

#include <cmath>

#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QDebug>
#include <QMutex>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QCursor>

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

customImageWidget::customImageWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    _vertices = QVector<GLfloat> {
       //  positions                 texture coords
            1.0f   ,  1.0f   , 0.0f, 1.0f, 0.0f, // top right
            1.0f   , -1.0f   , 0.0f, 1.0f, 1.0f, // bottom right
           -1.0f   ,  1.0f   , 0.0f, 0.0f, 0.0f, // top left
           -1.0f   , -1.0f   , 0.0f, 0.0f, 1.0f  // bottom left
       };

    _vbo = nullptr;
    _program = nullptr;
    _texture = nullptr;
    _textureSize=QSize();

    _overlayTexture = nullptr;
    _overlayTextureSize = QSize();

    _modelView.setToIdentity();
    _imageDCM.setToIdentity();
    _isUpsideDown = false;

    _zoomList = QList<float> { 20.0f, 19.0f, 18.0f, 17.0f, 16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.5f, 1.25f, 1.0f, 1.0f/1.25f, 1.0f/1.5f, 1.0f/2.0f, 1.0f/3.0f, 1.0f/4.0f, 1.0f/5.0f, 1.0f/6.0f, 1.0f/7.0f, 1.0f/8.0f, 1.0f/9.0f, 1.0f/10.0f };
    _zoomIndex = _zoomList.indexOf(1.0f);
    _zoom = _zoomList[_zoomIndex];

    _preventSync=2; //ignore for first 2 updates

    _cursorMode = 0;
    _mouseCursorPosition = QVector2D();
    _cursorAngle = 0.0f;

}

customImageWidget::~customImageWidget()
{
    makeCurrent();

    //cleanup objects within "Current" context
    if(_vbo!=nullptr)
    {
        _vbo->destroy();
        delete _vbo;
    }

    if(_program!=nullptr)
    {
        _program->deleteLater();
    }

    if(_texture!=nullptr)
    {
        _texture->destroy();
        delete _texture;
    }

    if(_overlayTexture!=nullptr)
    {
        _overlayTexture->destroy();
        delete _overlayTexture;
    }

    doneCurrent();
}

QSize customImageWidget::sizeHint() const
{
    return QSize(640,400);
}


void customImageWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.2f,0.2f,0.2f,1.0f);

    _program = new QOpenGLShaderProgram(this);
    if(!_program->addShaderFromSourceFile(QOpenGLShader::Vertex,":/img/image.vsh"))
    {
        qDebug()<<"vertex shader error"<<__FUNCTION__;
    }
    if(!_program->addShaderFromSourceFile(QOpenGLShader::Fragment,":/img/image.fsh"))
    {
        qDebug()<<"fragment shader error"<<__FUNCTION__;
    }
    _program->bindAttributeLocation("position", PROGRAM_VERTEX_ATTRIBUTE);
    _program->bindAttributeLocation("texture_uv", PROGRAM_TEXCOORD_ATTRIBUTE);
    _program->link();
    _program->bind();
    _program->setUniformValue("tex1", 0);
    _program->setUniformValue("tex2", 1);
    _program->release();

    _vbo=new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _vbo->create();
    if(_vbo->bind())
    {
        _vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
        _vbo->allocate(_vertices.constData(),_vertices.size()*sizeof(GLfloat));
        _vbo->release();
    }


    buildTexture();

    setMouseTracking(true);
}

void customImageWidget::resizeGL(int w, int h)
{
    _viewPort = QRect(0,0,w,h);
}

void customImageWidget::paintGL()
{

    QPainter painter(this);
    painter.beginNativePainting();

    if(_texture!=nullptr)
    {
        glEnable(GL_TEXTURE_2D);
        glClear(GL_COLOR_BUFFER_BIT);

        if(_program->bind())
        {
            float imageAspectRatio=1.6;

            glActiveTexture(GL_TEXTURE0);
            _texture->bind();

            if(_texture->isBound())
            {
                imageAspectRatio = (float)_textureSize.width() / (float)(_textureSize.height()? _textureSize.height(): 1);

                float screenAspectRatio = float(_viewPort.width()) / float(_viewPort.height() ? _viewPort.height() : 1);
                float gain = screenAspectRatio/imageAspectRatio;
                float length=1.0*_zoom;
                _projection.setToIdentity();
                if(gain>1.0f)
                {
                    _projection.ortho( -length*gain,length*gain,-length,length,-1.0,1.0);
                }
                else
                {
                    _projection.ortho( -length,length,-length/gain,length/gain,-1.0,1.0);
                }

                if(_overlayTexture!=nullptr)
                {
                    glActiveTexture(GL_TEXTURE1);
                    _overlayTexture->bind();
                }

                _program->setUniformValue("mvp_matrix",_projection*_modelView*_imageDCM);

                _program->setUniformValue("useOverlay", _overlayTexture!=nullptr ? 1:0);

                if(_vbo->bind())
                {
                    glEnableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
                    glEnableVertexAttribArray(PROGRAM_TEXCOORD_ATTRIBUTE);
                    glVertexAttribPointer(PROGRAM_VERTEX_ATTRIBUTE,3,GL_FLOAT,GL_FALSE,5*sizeof(float),reinterpret_cast<void*>(0)); //coordinate xyz
                    glVertexAttribPointer(PROGRAM_TEXCOORD_ATTRIBUTE,2,GL_FLOAT,GL_FALSE,5*sizeof(float),reinterpret_cast<void*>(3*sizeof(GLfloat))); //texture uv

                    glDrawArrays(GL_TRIANGLE_STRIP,0,_vertices.size()/5);

                    glDisableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
                    glDisableVertexAttribArray(PROGRAM_TEXCOORD_ATTRIBUTE);
                    _vbo->release();
                }

                if(_overlayTexture!=nullptr)
                {
                    glActiveTexture(GL_TEXTURE1);
                    _overlayTexture->release();
                }

                glActiveTexture(GL_TEXTURE0);
                _texture->release();
            }
            _program->release();
        }
        glDisable(GL_TEXTURE_2D);
        glFlush();
    }
    painter.endNativePainting();

    if(_cursorMode && !_mouseCursorPosition.isNull())
    {
        float a=_cursorAngle * (float)(M_PI)/180.0f;
        float sa=std::sin(a);
        float ca=std::cos(a);
        float r=_viewPort.width()+_viewPort.height();
        float x=_mouseCursorPosition.x();
        float y=_mouseCursorPosition.y();

        QLineF l1(x-r*ca,y+r*sa,x+r*ca,y-r*sa),l2(x+r*sa,y+r*ca,x-r*sa,y-r*ca);

        painter.setRenderHint(QPainter::Antialiasing,true);
        painter.setPen(QPen(Qt::red));
        painter.drawLine(l1);
        painter.drawLine(l2);
    }
    //drawRectangle(&painter);
    //painter.drawText(32,32,QString("%1x%2 %3,%4").arg(_image.width()).arg(_image.height()).arg(_cursor.x(),0,'f',2).arg(_cursor.y(),0,'f',2));

    painter.end();

    emitInfoUpdate();
}


void customImageWidget::mousePressEvent(QMouseEvent *e)
{
    _mouseCurrentPosition = QVector2D();

    // Save mouse press position
    if(e->buttons() & Qt::LeftButton)
    {
        _mousePressPosition = QVector2D(e->localPos());
    }
    else if(e->buttons() & Qt::RightButton)
    {
        _mousePressPosition = QVector2D(e->localPos());
    }
    else
    {

    }
    update();
}

void customImageWidget::translate(QVector2D diff)
{
    const float *p=_projection.data();
    float width=2.0f/p[0];
    float height=-2.0/p[5];

    _modelView.translate(width*diff.x()/_viewPort.width(),height*diff.y()/_viewPort.height());
}

void customImageWidget::setCursorMode(int cursorMode)
{
    _cursorMode = cursorMode;
}

void customImageWidget::mouseMoveEvent(QMouseEvent *e)
{
    int updateRequire = 0;

    _mouseCursorPosition = QVector2D();

    if(e->buttons() & Qt::LeftButton)
    {
        if(!_mousePressPosition.isNull())
        {
            QVector2D diff = QVector2D(e->localPos()) - _mousePressPosition;
            _mousePressPosition = QVector2D(e->localPos());
            _mouseCurrentPosition = QVector2D();

            translate(diff);

            updateRequire=1;
        }
    }
    else if(e->buttons() & Qt::RightButton)
    {
        _mouseCurrentPosition = QVector2D(e->localPos());
        updateRequire=1;
    }
    else
    {
        _mousePressPosition = QVector2D();
        _mouseCurrentPosition = QVector2D();

        if(_textureSize.isValid())
        {
            QVector3D worldPos = QVector3D( e->localPos().x(), size().height() -e->localPos().y(), 0.5f ).unproject(_modelView*_imageDCM,_projection,_viewPort);
            QSize s=_textureSize;
            _cursor = QVector2D(0.5*(worldPos.x()+1.0) * s.width(), 0.5*(1.0-worldPos.y()) * s.height());
            _preventSync++;
            emitInfoUpdate();
        }
    }

    if(_cursorMode)
    {
        _mouseCursorPosition = QVector2D(e->localPos());
        updateRequire = 1;
    }

    if(updateRequire) update();
}

void customImageWidget::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    if(!_mouseCurrentPosition.isNull() && !_mousePressPosition.isNull())
    {
#if 0
        if(_texture0Size.isValid())
        {
            if(_mouseCurrentPosition!=_mousePressPosition)
            {//valid ROI
                QSize s=_texture0Size;
                QVector3D pos1 = QVector3D( _mouseCurrentPosition.x(), size().height() -_mouseCurrentPosition.y(), 0.5f ).unproject(_modelView*_sensorDCM,_projection,_viewPort);
                QVector3D pos2 = QVector3D( _mousePressPosition.x(), size().height() -_mousePressPosition.y(), 0.5f ).unproject(_modelView*_sensorDCM,_projection,_viewPort);

                //convert to pixel coordinates
                float x1 =0.5f*(pos1.x()+1.0f)*s.width();
                float x2 =0.5f*(pos2.x()+1.0f)*s.width();
                float y1 =0.5f*(1.0f-pos1.y())*s.height();
                float y2 =0.5f*(1.0f-pos2.y())*s.height();

                float x0= x1<x2 ? x1:x2;
                float y0= y1<y2 ? y1:y2;
                float dx = abs(x1-x2);
                float dy = abs(y1-y2);

                QVariantList roi;
                roi << x0 << y0 << dx << dy;
                QMetaObject::invokeMethod(parentWidget(),"roiUpdated",Qt::QueuedConnection,Q_ARG(QVariantList,roi));
            }
            else
            {//zero area
                QMetaObject::invokeMethod(parentWidget(),"roiUpdated",Qt::QueuedConnection,Q_ARG(QVariantList,QVariantList()));
            }
        }
#endif
    }
    _mousePressPosition = QVector2D();
    _mouseCurrentPosition = QVector2D();
    update();
}

void customImageWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    _mousePressPosition = QVector2D();
    _mouseCurrentPosition = QVector2D();

    if(e->buttons() & Qt::LeftButton)
    {
        _zoomIndex = _zoomList.indexOf(1.0f);
        updateZoom();
        _modelView.setToIdentity();
        update();
    }
    if(e->buttons() & Qt::RightButton)
    {
    }

}

void customImageWidget::updateZoom(void)
{
    _zoom= 1.0f/_zoomList[_zoomIndex];
}

void customImageWidget::emitInfoUpdate()
{
    QVariantMap x;
    x["Height"] = _image.height();
    x["Width"] = _image.width();
    x["Zoom"] = _zoomIndex;
    x["Cursor"] = _cursor.toPointF();
    x["UpsideDown"] = _isUpsideDown;
    x["modelView"] = _modelView;
    x["preventSync"] = _preventSync;
    if(_preventSync>0) --_preventSync;
    emit infoUpdate(x);
}

void customImageWidget::syncView(QVariantMap x)
{
    if(x.contains("Height") && x.contains("Width") && x.contains("Zoom") && x.contains("modelView"))
    {
        if(x["Height"].toInt()==_image.height() && x["Width"].toInt()==_image.width() )
        {
            _zoomIndex = x["Zoom"].toInt();
            updateZoom();
            _modelView = x["modelView"].value<QMatrix4x4>();
            _preventSync=1;
            update();
        }
    }

}



void customImageWidget::wheelEvent(QWheelEvent *e)
{

    if(e->modifiers() & Qt::ShiftModifier)
    {
        float gain = 1.0f/120.0f;
        if(e->modifiers() & Qt::ControlModifier) gain *=0.05f;
        _cursorAngle +=e->angleDelta().y()*gain;
        if(_cursorAngle>=90.0f) _cursorAngle-=90.0f;
        else if(_cursorAngle<=-90.0f) _cursorAngle+=90.0f;
    }
    else
    {
        QVector2D diff =  QVector2D(this->rect().center())-QVector2D(e->pos());
        translate(diff);

        QPoint centerg=mapToGlobal(rect().center());
        QCursor cursor;
        cursor.setPos(centerg);

        if(e->angleDelta().y()>0) _zoomIndex++;
        if(e->angleDelta().y()<0) _zoomIndex--;

        if(_zoomIndex<0) _zoomIndex=0;
        else if(_zoomIndex>=_zoomList.size()) _zoomIndex = _zoomList.size()-1;

        updateZoom();
    }

    update();
}

void customImageWidget::setUpsideDown(bool isUpsideDown)
{
    if(isUpsideDown)
    {
        _imageDCM.setToIdentity();
        QQuaternion roll180=QQuaternion::fromAxisAndAngle(QVector3D(0.0f,0.0f,1.0f), 180.0f);
        _imageDCM.rotate(roll180);
    }
    else
    {
        _imageDCM.setToIdentity();
    }
    _isUpsideDown = isUpsideDown;
    update();
}


void customImageWidget::setImage(QImage &image)
{
    _image=image.copy();
}

void customImageWidget::setOverlay(QImage &image)
{
    _overlay=image.copy();
}

void customImageWidget::buildTexture()
{
    makeCurrent();
    if(_texture!=nullptr)
    {
        _texture->destroy();
        delete _texture;
        _texture = nullptr;
        _textureSize=QSize();
    }
    _texture = new QOpenGLTexture(_image);
    _texture->setMagnificationFilter(QOpenGLTexture::Filter::Nearest);
    _texture->setMinificationFilter(QOpenGLTexture::Filter::LinearMipMapLinear);
    if(_texture->create())
    {
        _textureSize=QSize(_texture->width(),_texture->height());
    }
    else
    {
        delete _texture;
        _texture=nullptr;
    }

    doneCurrent();
}

void customImageWidget::buildOverlayTexture(void)
{
    makeCurrent();
    if(_overlayTexture!=nullptr)
    {
        _overlayTexture->destroy();
        delete _overlayTexture;
        _overlayTexture = nullptr;
        _overlayTextureSize = QSize();
     }
    _overlayTexture = new QOpenGLTexture(_overlay);
    _overlayTexture->setMagnificationFilter(QOpenGLTexture::Filter::Nearest);
    _overlayTexture->setMinificationFilter(QOpenGLTexture::Filter::LinearMipMapLinear);
    if(_overlayTexture->create())
    {
        _overlayTextureSize=QSize(_overlayTexture->width(),_overlayTexture->height());
    }
    else
    {
        delete _overlayTexture;
        _overlayTexture=nullptr;
    }

    doneCurrent();
}


