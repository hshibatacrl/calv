#ifndef GL_STOCK_ENTITY_H
#define GL_STOCK_ENTITY_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_entity_ctx.h"

class gl_stock_entity : public gl_entity_ctx
{
    Q_OBJECT
public:
    explicit gl_stock_entity(QObject *parent = 0);
    virtual ~gl_stock_entity();

public slots:
    void load(void);

protected:
    GLfloat *vertex;
    quint64 n_vertex;
    int n_element;
    int projMat;
    int mvMat;
    QOpenGLBuffer vbo;
    QOpenGLShaderProgram *prg;
    virtual const char *get_vertex_shader(void) const;
    virtual const char *get_fragment_shader(void) const;
    virtual void vbo_allocate(void);
    virtual void vbo_bind(void);

    virtual GLfloat get_LineWidth(void) {return 1.0f;}

public:
    virtual int prepare_gl(void);
    virtual void draw_gl(gl_draw_ctx_t &draw);
};


#endif // GL_STOCK_ENTITY_H
