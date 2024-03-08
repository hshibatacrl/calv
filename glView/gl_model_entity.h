#ifndef GL_MODEL_ENTITY_H
#define GL_MODEL_ENTITY_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_entity_ctx.h"
#include "model.h"

#include <vector>
#include <map>

#include <QOpenGLTexture>

typedef std::map<GLuint,QOpenGLTexture*> textures_t;
typedef std::vector<GLfloat> vbo_source_t;

typedef struct
{
    int shadeModel;    //GL_SMOOTH or GL_FLAT
    int idx_material;
    size_t src_top;
    size_t num_vertex;
    int group_id;
    int group_top;
} model_element_t;

typedef std::vector<model_element_t*> model_elements_t;
typedef std::vector<QVector3D> vector3ds_t;

class gl_model_entity : public gl_entity_ctx
{
    Q_OBJECT

public:
    explicit gl_model_entity(QObject *parent = 0);
    virtual ~gl_model_entity();
    virtual int prepare_gl(void);

public slots:
    void load(void);

protected:
    virtual const char *get_vertex_shader(void) const;
    virtual const char *get_fragment_shader(void) const;

    virtual void vbo_allocate(void);
    virtual void vbo_bind(void);

    virtual void update_group_matrix(int id,QMatrix4x4 &local);

    virtual QString getFileName(const QString &fileName);

public:
    virtual void draw_gl(gl_draw_ctx_t &draw);

private:
    QOpenGLBuffer vbo;
    QOpenGLShaderProgram *prg;

    model_type model;
    vector3ds_t cg;
    vector3ds_t axis;

    double inc;

    model_import_params_t param;

    model_elements_t model_elements;
    vbo_source_t vbo_src;
    size_t num_vertex;

    textures_t textures;

    int projMat;
    int mvMat;
    int norMat;
    int litPos;
    int matCol;
    int matAmb;
    int matDif;
    int matEmi;
    int matSpc;
    int enaShad;
    int enaTex;
};

#endif // GL_MODEL_ENTITY_H

