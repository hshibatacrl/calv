#ifndef CUSTOMGLWIDGET_H
#define CUSTOMGLWIDGET_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMutex>
#include <QTimer>

#ifdef USE_EDL
#include <QOpenGLExtensions>
#endif
#include <QOpenGLFunctions_2_1>

#include "gl_entity_ctx.h"
#include "qt_opengl_unproj.h"

typedef struct
{
    double persFOV;
    double persNear;
    double persFar;
    double orthNear;
    double orthFar;
    int pointAntiAlias;
} viewOptions;

#ifdef USE_EDL
class ccFrameBufferObject;
class ccGlFilter;
#endif

class customGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    customGLWidget(QWidget *parent);
    virtual ~customGLWidget();

    virtual QSize sizeHint() const;

    QVector3D *get_origin(){return &_origin;}

    void lockEntities(void);
    void unlockEntities(void);
    gl_entities_t &getEntities(void)
    {
        return _entities;
    }

    void rebuildRequest(QUuid id);

    void delayLoad(gl_entity_ctx *ctx, const char *path);
    void delayLoad(gl_entity_ctx *ctx, const QByteArray &data);

signals:
    void initialized(void);
    void poiUpdated(QStringList _poi);
    void entityUnloaded(QObject *x);
    void entityLoadedByWidget(QObject *x);
    void onDrawingOptionUpdated(void);
    void keyPressFromGLWidget(int key);


public slots:
    void entityLoaded(QObject *x);
    void entityUnload(QObject *x);
    void redrawEntity(void);
    void entityClicked(gl_entity_ctx *a);
    void viewOptionsTriggered(void);

protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    //camera control
    int zoomInput(double dx);
    int translateInput(double dx,double dy,double dz);
    int leftAxisInput(double dx,double dy);
    int rightAxisInput(double dx,double dy);
    int trackballInput(double dx,double dy,double dz);
    int pottersWheelInput(double dx,double dy);
    int cameraLookingDown(void);

    using OpenGLFunctions = QOpenGLFunctions_2_1;

    //! Returns the set of OpenGL functions
    inline OpenGLFunctions* functions() const { return context() ? context()->versionFunctions<OpenGLFunctions>() : nullptr; }

    void update_by_poi(void);

    int unproj(int winX, int winY, QVector3D &ret);

    void draftUpdate(void);
    void poiIndicatorUpdate(void);

private slots:
    void onCmPerspectiveView(bool checked);
    void onCmOrthoView(bool checked);

    void pertialPrepare(void);

private:
    void load_stock(void);
    size_t getEntitiesCount(void);
    void draw_core(int mode);
    void updateDepth(void);

#ifdef USE_EDL
    bool initFBOSafe(ccFrameBufferObject* &fbo, int w, int h);
    void removeFBOSafe(ccFrameBufferObject* &fbo);
    void removeFBO();
    bool initFBO(int w, int h);

    void removeGLFilter();
    bool initGLFilter(int w, int h, bool silent/*=false*/);

    bool bindFBO(ccFrameBufferObject* fbo);
    void setStandardOrthoCorner();
    float computePerspectiveZoom() const;
    void makeCurrent();
#endif


private:
    QVector3D _origin;

    int _next_mode;
    gl_draw_ctx_t _draw;

    double _pot[2];
    QVector4D _quat;
    double _range;   //poi to camera distance [m]
    QVector3D _poi;  //poi location
    QVector3D _poc;  //camera location

    gl_entities_t _entities;
    gl_entities_t _entitiesNotCompleted;

#ifdef USE_EDL
    ccFrameBufferObject* m_activeFbo;
    ccFrameBufferObject* m_fbo;
    ccGlFilter* m_activeGLFilter;
    QOpenGLExtension_ARB_framebuffer_object	m_glExtFunc;
    bool m_glExtFuncSupported;
#endif

    gl_entity_ctx *_poiIndicator;

    QTimer _timer4update;
    QTimer _timer4pertialPrepare;

    QPoint _mouseMoveLastPos;
    QPoint _RightPressedPos;

    QMutex _mtxEntities;

    int _depthSearchRadius;
    QList<QPoint> _depthSearchArea;

    int _altDepthRequired;
    depthContext *_depthContext;

    int _cameraMode;
    int _cameraControl;

    QVariantMap _viewOptionsStorage;
    viewOptions _viewOptions;

};

#define CAM_PERSPECTIVE 0
#define CAM_ORTHO 1

#define CAM_CTRL_LEGACY 0         // trackball
#define CAM_CTRL_POTTERSWHEEL 1   // potter's wheel

#endif // CUSTOMGLWIDGET_H
