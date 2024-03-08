#ifndef CUSTOMIMAGEWIDGET_H
#define CUSTOMIMAGEWIDGET_H

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

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector>
#include <QMatrix4x4>
#include <QVector2D>

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLTexture;

class customImageWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    customImageWidget(QWidget *parent=nullptr);
    virtual ~customImageWidget();

    virtual QSize sizeHint() const;

    void setUpsideDown(bool isUpsideDown);

    void setImage(QImage &image);

    void setOverlay(QImage &image);

    void buildTexture(void);

    void buildOverlayTexture(void);

    void syncView(QVariantMap x);

    void setCursorMode(int cursorMode);

signals:
    void infoUpdate(QVariantMap x);

protected:
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void wheelEvent(QWheelEvent *e) override;

    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

private:
    void updateZoom(void);
    void emitInfoUpdate(void);

    void translate(QVector2D diff);
protected:

    QImage _image;
    QOpenGLTexture *_texture;
    QSize _textureSize;

    QImage _overlay;
    QOpenGLTexture *_overlayTexture;
    QSize _overlayTextureSize;

    QVector2D _mousePressPosition;
    QVector2D _mouseCurrentPosition;
    QVector2D _cursor;  // pixel coordinates

    QOpenGLShaderProgram *_program;
    QOpenGLBuffer *_vbo;

    QVector<GLfloat> _vertices;

    QMatrix4x4 _imageDCM;
    bool _isUpsideDown;

    QMatrix4x4 _modelView;
    QMatrix4x4 _projection;
    QRect _viewPort;

    QList<float> _zoomList;
    int _zoomIndex;

    float _zoom;

    int _preventSync;

    int _cursorMode;
    QVector2D _mouseCursorPosition;
    float _cursorAngle;
};

#endif // CUSTOMIMAGEWIDGET_H
