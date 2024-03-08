#ifndef GL_POLYLINE_ENTITY_H
#define GL_POLYLINE_ENTITY_H

#include "gl_entity_ctx.h"

class gl_polyline_entity : public gl_entity_ctx
{
    Q_OBJECT

public:
    gl_polyline_entity(QObject *parent=0);
    virtual ~gl_polyline_entity();
    virtual int prepare_gl(void);
    virtual void draw_gl(gl_draw_ctx_t &draw);

    virtual bool isUnloadable(void) {return true;}

public slots:
    void load(void);

protected:
    virtual void vbo_bind(QOpenGLBuffer &vbo, QOpenGLFunctions *fc);
    virtual void vbo_release(QOpenGLBuffer &vbo,QOpenGLFunctions *fc);

    virtual int load_mem(const uint8_t *buf, size_t length);

private:
    QOpenGLShaderProgram *_prg;
    QOpenGLBuffer _vbo;
    QVector<QVector3D> _vertices;
    QVector<int> _chunks;
};

#endif // GL_TRAJECTORY_ENTITY_H
