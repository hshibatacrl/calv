#ifndef GL_POSES_ENTITY_H
#define GL_POSES_ENTITY_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_entity_ctx.h"

#include <QMatrix4x4>

typedef struct
{
    QVector4D q;    //quaternion body to nav
    QVector3D p;    //position (X,Y,Z) East-North-Up
} pose_t;

class gl_poses_entity : public gl_entity_ctx
{
    Q_OBJECT
public:
    explicit gl_poses_entity(gl_entity_ctx *model, QObject *parent = nullptr);
    virtual ~gl_poses_entity();

    virtual void draw_gl(gl_draw_ctx_t &draw);

    virtual bool isUnloadable(void) {return true;}

    virtual QVector3D getCenter(void);

public slots:
    void load(void);

protected:
    virtual int load_mem(const uint8_t *buf, size_t length);

    QVector<pose_t> _poses;

    gl_entity_ctx *_model;
};

#endif // GL_POSES_ENTITY_H
