/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_3axis_entity.h"

#include <QOpenGLShaderProgram>
#include <QVector>

typedef QVector<QVector3D> vtx_list_t;

gl_3axis_entity::gl_3axis_entity(QObject *parent):gl_stock_entity(parent)
{
    info[ENTITY_INFO_TARGET_FILENAME]="3 axis";

    setObjectName("axis");
}

gl_3axis_entity::~gl_3axis_entity()
{

}

static vtx_list_t get_arrow(void)
{
    vtx_list_t x;
    x.push_back(QVector3D(0.0f,0.0f,0.0f));
    x.push_back(QVector3D(1.0f,0.0f,0.0f));
    x.push_back(QVector3D(1.0f,0.0f,0.0f));
    x.push_back(QVector3D(0.9f,-0.05f,0.0f));
    x.push_back(QVector3D(1.0f,0.0f,0.0f));
    x.push_back(QVector3D(0.9f,0.05f,0.0f));
    return x;
}

static vtx_list_t get_N(void)
{
    vtx_list_t x;
    GLfloat sz=0.1f;
    x.push_back(QVector3D(1.0f,    sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f+sz, sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f+sz, sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f   ,-sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f   ,-sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f+sz,-sz*0.5f,0.0f));
    return x;
}

static vtx_list_t get_E(void)
{
    vtx_list_t x;
    GLfloat sz=0.1f;
    x.push_back(QVector3D(1.0f+sz,-sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f+sz, sz*0.5f,0.0f));

    x.push_back(QVector3D(1.0f+sz, sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f   , sz*0.5f,0.0f));

    x.push_back(QVector3D(1.0f   , sz*0.5f,0.0f));
    x.push_back(QVector3D(1.0f   ,-sz*0.5f,0.0f));

    x.push_back(QVector3D(1.0f+sz*0.5f,-sz*0.3f,0.0f));
    x.push_back(QVector3D(1.0f+sz*0.5f, sz*0.5f,0.0f));

    return x;
}

static int push_vector(GLfloat *p, const vtx_list_t &vtx, const QMatrix4x4 &mat, const QVector3D &col )
{
    QVector4D v;
    int ret=0;
    for(int i=0;i<vtx.size();i++)
    {
        v=QVector4D(vtx[i],0.0f);
        v=mat*v;
        *p++=v.x();
        *p++=v.y();
        *p++=v.z();
        *p++=col.x();
        *p++=col.y();
        *p++=col.z();
        ret+=6;
    }
    return ret;
}

void gl_3axis_entity::load(void)
{
    vtx_list_t x,y,z,e,n;
    QMatrix4x4 rx,ry,rz;
    rx.setToIdentity();
    ry.setToIdentity(); ry.rotate(90.0f,0.0f, 0.0f,1.0f);
    rz.setToIdentity(); rz.rotate(90.0f,0.0f,-1.0f,0.0f);

    x=get_arrow();
    y=get_arrow();
    z=get_arrow();
    e=get_E();
    n=get_N();

    n_element=6;
    n_vertex=x.size()+y.size()+z.size()+e.size()+n.size();
    vertex=new GLfloat [n_vertex*n_element];

    int r;
    GLfloat *p=vertex;
    r=push_vector(p,x,rx,QVector3D(1.0f,0.0f,0.0f));    p+=r;
    r=push_vector(p,e,rx,QVector3D(1.0f,0.0f,0.0f));    p+=r;
    r=push_vector(p,y,ry,QVector3D(0.0f,1.0f,0.0f));    p+=r;
    r=push_vector(p,n,ry,QVector3D(0.0f,1.0f,0.0f));    p+=r;
    r=push_vector(p,z,rz,QVector3D(0.0f,0.0f,1.0f));    p+=r;

    gl_stock_entity::load();
}

QVector3D gl_3axis_entity::getCenter(void)
{
    QVector4D ret=local * QVector4D(0.0,0.0,0.0,1.0);
    return ret.toVector3D();
}

