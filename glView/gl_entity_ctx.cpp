/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_entity_ctx.h"

#include <QOpenGLShaderProgram>
#include <QThread>

gl_entity_ctx::gl_entity_ctx(QObject *parent) : QObject(parent)
{
    valid=0;
    _show=1;
    _showGroup=1;
    _reference=false;
    local.setToIdentity();

    _unique_id=QUuid::createUuid();

    _bounding[0]=QVector3D();
    _bounding[1]=QVector3D();
    _bounding[2]=QVector3D();

    setObjectName("base_class");
}

//[static]
int gl_entity_ctx::originOffsetCore(QMatrix4x4 &ret, const QVector3D &baseOrigin, const QVector3D &targetOrigin)
{
    QVector3D localVec=targetOrigin-baseOrigin;

    if(localVec.length()>10000.0)
    {
        return 0;
    }

    ret.setToIdentity();
    ret.translate(localVec);
    return 1;
}

int gl_entity_ctx::originOffset(QMatrix4x4 &ret)
{
    return originOffsetCore(ret,*origin,_localOrigin);
}

int gl_entity_ctx::originOffsetFromMeToTarget(QMatrix4x4 &ret,const QVector3D &targetOrigin) const
{
    return originOffsetCore(ret,targetOrigin,_localOrigin);
}

int gl_entity_ctx::setMasterOriginFromLocal(const QVector3D &localPos)
{
    if(localPos.isNull()) return 0;
    if(origin==nullptr) return 0;
    if(origin->isNull()) return 0;
    if(_localOrigin.isNull()) return 0;
    _localOrigin = localPos;
    return 1;
}

void gl_entity_ctx::setBounding(QVector3D tlh, QVector3D brl)
{
    _bounding[0]=tlh;
    _bounding[1]=brl;
    _bounding[2]=(_bounding[0]+_bounding[1])/2;
}

QVector3D gl_entity_ctx::getCenter(void)
{
    QVector4D local4D=local * QVector4D(0.0,0.0,0.0,1.0);
    QVector3D ret=local4D.toVector3D() + _bounding[2];
    return ret;
}

QString gl_entity_ctx::getCaption(void)
{
    return objectName();
}

gl_entity_ctx::~gl_entity_ctx()
{
    //qDebug()<< "~gl_entity_ctx";
}

void gl_entity_ctx::term_thread(void)
{
    thread()->terminate();
}

void gl_entity_ctx::emitProgress(quint64 current, quint64 total,QString label, bool done)
{
    emit progress(QVariantList()<<current<<total<<label<<_unique_id<<info[ENTITY_INFO_TARGET_FILENAME].toString()<<done);
}

int gl_entity_ctx::prepare_gl(void)
{
    term_thread();

    qDebug() << "prepare_gl";

    return 0;
}

void gl_entity_ctx::load(void)
{
}

//this entity's status, ignore group
int gl_entity_ctx::checkState(void)
{
    if(_show) return  Qt::Checked;
    return Qt::Unchecked;
}

void gl_entity_ctx::setReference(bool newReference)
{
    _reference = newReference;
}

void gl_entity_ctx::setShow(Qt::CheckState x)
{
    if(x==Qt::Checked) _show=1;
    else _show=0;
    //qDebug()<<"setShow"<<getCaption()<<x;
}

void gl_entity_ctx::setGroupShow(Qt::CheckState x)
{
    if(x==Qt::Checked) _showGroup=1;
    else _showGroup=0;
    //qDebug()<<"setGroupShow"<<getCaption()<<x;
}

Qt::CheckState gl_entity_ctx::show(void)
{
    if(_show && _showGroup) return Qt::Checked;
    return Qt::Unchecked;
}

