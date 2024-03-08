#ifndef GL_ENTITY_CTX_H
#define GL_ENTITY_CTX_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QUuid>

#include "gl_draw_params.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class gl_entity_ctx : public QObject
{
    Q_OBJECT
public:
    explicit gl_entity_ctx(QObject *parent = 0);
    ~gl_entity_ctx();

    virtual void cleanup(void) {}

signals:
    void progress(QVariantList param);
    void done(QObject *x);

public slots:
    void load(void);

public:
    QVector3D *origin;

    virtual QString getCaption(void);

    QVariantMap info;
    QMatrix4x4 local;

    virtual int rebuildRequest(void){return 0;}   //rebuild VBO

    virtual int prepare_gl(void);
    virtual int pertialPrepare_gl(void){return 0;}

    virtual void draw_gl(gl_draw_ctx_t &draw)=0;
    virtual int update_draw_gl(gl_draw_ctx_t &draw){ Q_UNUSED(draw); return 0;}  //return 1 when you change parameter.

    int is_valid(void) {return valid;}  // result of load()

    void setShow(Qt::CheckState x);
    void setGroupShow(Qt::CheckState x);
    Qt::CheckState show(void);

    virtual QVector3D getCenter(void);

    QUuid uniqueId(void) {return _unique_id;}

    virtual bool isAlphaBlend(void) { return false; }
    virtual bool isPickable(void) { return true; }

    int setMasterOriginFromLocal(const QVector3D &localPos);

    QVector3D &localOrigin(void) {return _localOrigin;}

    static int originOffsetCore(QMatrix4x4 &ret,const QVector3D &baseOrigin,const QVector3D &targetOrigin);

    virtual bool isExportable(void) {return false;}
    virtual bool exportData(QVariantMap &result){Q_UNUSED(result); return false;}

    virtual bool isUnloadable(void) {return false;}

    virtual bool isReference(void) {return _reference;}

    void setReference(bool newReference);

protected:
    int valid;                          // result of load()
    virtual void term_thread(void);

    void emitProgress(quint64 current, quint64 total,QString label,bool done=false);

    virtual void setBounding(QVector3D tlh, QVector3D brl);

    QVector3D _localOrigin;

    int originOffset(QMatrix4x4 &ret);
    int originOffsetFromMeToTarget(QMatrix4x4 &ret,const QVector3D &targetOrigin) const;

    int checkState(void);//this entity's status, ignore group

private:
    QUuid _unique_id;
    int _show;
    int _showGroup;
    bool _reference;
    QVector3D _bounding[3];  //0: top-left-high, 1:bottom-right-low

};

typedef QMap<QUuid,gl_entity_ctx*> gl_entities_t;

#define EXPORT_TYPE_POINTS "multipoint"
#define EXPORT_TYPE_POLYGON "polygon"
#define EXPORT_TYPE_POLYLINE "polyline"

#define ENTITY_INFO_TARGET_FILENAME "targetFileName"
#define ENTITY_INFO_TARGET_BYTES "targetByteArray"

#endif // GL_ENTITY_CTX_H
