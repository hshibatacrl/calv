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

#include "gl_poses_entity.h"

#include "rot.h"

#include <mutex>

#include <QVector3D>
#include <QThread>
#include <QFileInfo>
#include <QFile>

gl_poses_entity::gl_poses_entity(gl_entity_ctx * model, QObject *parent)
    : gl_entity_ctx{parent}
{
    _model = model;
}

gl_poses_entity::~gl_poses_entity()
{

}

int gl_poses_entity::load_mem(const uint8_t *buf, size_t length)
{
    int ret=0;
     _localOrigin = QVector3D(0.0f,0.0f,0.0f); // East-North-Up
     setObjectName("pose test");

    QVector<float> p;
    if(!buf[0])
    {   // x,y,z r,p,y
        p<< 0.0f << 0.0f << 0.0f << 0.0f << 0.0f << 0.0f;
        p<< 2.0f << 0.0f << 0.0f << 10.0f << 0.0f << 45.0f;
        p<< 4.0f << 0.0f << 0.0f << 20.0f << 0.0f << 90.0f;
        p<< 6.0f << 0.0f << 0.0f << 10.0f << 0.0f << 45.0f;
        p<< 8.0f << 0.0f << 0.0f << 0.0f << 10.0f << 0.0f;
        p<< 10.0f << 0.0f << 0.0f << 0.0f << 20.0f << -45.0f;
    }
    else
    {
        p<< 11.0f << 0.0f << 0.0f << 0.0f << 10.0f << -90.0f;
        p<< 13.0f << 0.0f << 0.0f << 10.0f << 0.0f << -45.0f;
        p<< 15.0f << 0.0f << 0.0f << 20.0f << 0.0f << 0.0f;
        p<< 17.0f << 0.0f << 0.0f << 10.0f << 0.0f << 0.0f;
        p<< 19.0f << 0.0f << 0.0f << 0.0f << 10.0f << 0.0f;
        p<< 21.0f << 0.0f << 0.0f << 0.0f << 20.0f << 0.0f;
    }

    pose_t pose;
    for(int i=0;i<p.size()/6;i++)
    {
        pose.p=QVector3D(p.at(i*6+0),p.at(i*6+1),p.at(i*6+2));
        QVector3D rpy(p.at(i*6+3)*rot::d2r,p.at(i*6+4)*rot::d2r,p.at(i*6+5)*rot::d2r);
        rot::euler_to_quat(pose.q,rpy);
        _poses.append(pose);
        ret++;
    }

    return ret;
}


void gl_poses_entity::load()
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
        valid= _poses.size()>0;
    }

    emit done(this);
}

void gl_poses_entity::draw_gl(gl_draw_ctx_t &draw)
{
    if(_model==nullptr) return;
    if(!show()) return;


    QMatrix4x4 s;
    s.setToIdentity();
    s.scale(2.0f);

    QVector3D origin =  localOrigin();
    foreach(auto &i,_poses)
    {
        QMatrix3x3 dcm;
        rot::dcm_from_quat(dcm,i.q);

        QMatrix4x4 R;
        rot::dcm4x4(R, dcm);

        QMatrix4x4 t;
        t.setToIdentity();

        t.translate(origin);
        t.translate(i.p);

        _model->local = t * R * s;

        _model->draw_gl(draw);
    }
}

QVector3D gl_poses_entity::getCenter()
{
    return _poses.front().p;
}
