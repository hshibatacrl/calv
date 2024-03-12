/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_poses_entity.h"

#include "pose_packet.h"
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
    const pose_packet_header_t *header = (const pose_packet_header_t *) buf;
    const uint8_t *top = buf;
    if (header->magic == POSE_MAGIC)
    {
        if(header->length <= length)
        {
            top+= sizeof(pose_packet_header_t);
            if(header->type & POSE_ORG)
            {//optional
                 const pose_origin_t *org = (const pose_origin_t *)top;
                 GLfloat x = org->xyz[0];    //Right
                 GLfloat y = org->xyz[1];    //Down
                 GLfloat z = org->xyz[2];    //Forward
                 _localOrigin = QVector3D(z,-x,-y); // East-North-Up
                 top+=sizeof(pose_origin_t);
            }

            if(header->type & POSE_TXT)
            {//optional
                const pose_text_t *txt = (const pose_text_t *)top;
                top += sizeof(pose_text_t);
                std::string text( (const char*)top, txt->length );
                setObjectName(QString::fromStdString(text));
                top += txt->length;
            }

            //required block
            const pose_payload_t* p = (const pose_payload_t*)top;
            for(uint32_t i=0;i<header->numPose;i++)
            {
                pose_t pose;

                GLfloat rx = p[i].rvec[0];    //Right
                GLfloat ry = p[i].rvec[1];    //Down
                GLfloat rz = p[i].rvec[2];    //Forward

                QMatrix3x3 R;
                QVector3D rvec(rz, -rx, -ry);   // convert to E-N-U (Forward-Left-Up)
                rot::dcm_from_rodrigues(R,rvec);
                rot::dcm_to_quat(pose.q,R);

                GLfloat x = p[i].tvec[0];    //Right
                GLfloat y = p[i].tvec[1];    //Down
                GLfloat z = p[i].tvec[2];    //Forward
                pose.p=QVector3D(z, -x, -y);  // convert to E-N-U (Forward-Left-Up)

                _poses.append(pose);
                ret++;
            }
        }
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
