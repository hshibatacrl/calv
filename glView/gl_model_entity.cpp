/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "gl_model_entity.h"

#if __GNUC__==7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif __GNUC__>7
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <cmath>
#define d2r (M_PI/180.0)

#include <QOpenGLShaderProgram>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QThread>

static QVector4D expand_material(const double x[4]);
static size_t model_object_compile(object_type &object,material_list &materials,vertex_list &gv, model_elements_t &elements, vbo_source_t &src);
static void model_load_all_texture(model_type *model, textures_t &textures);

gl_model_entity::gl_model_entity(QObject *parent) : gl_entity_ctx(parent)
{
    reset_model(&model);
    inc=0;
    setObjectName("Model");
}

gl_model_entity::~gl_model_entity()
{

}

QString gl_model_entity::getFileName(const QString &fileName)
{
    QString ret=fileName;
    if(fileName.startsWith(":/"))
    {
        QString temp=QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QFileInfo fi(fileName);
        if(fi.exists())
        {
            QDir dir=fi.absoluteDir();
            auto fil=dir.entryInfoList(QStringList()<<fi.baseName()+".*",QDir::Files|QDir::Readable);
            foreach(auto &i,fil)
            {
                QFile::copy(i.absoluteFilePath(), temp+"/"+i.fileName());
                if(fi.fileName() == i.fileName())
                {
                    ret = temp+"/"+i.fileName();
                }
            }
        }
    }
    return ret;
}

void gl_model_entity::load(void)
{
    thread()->setPriority(QThread::LowPriority);

    model_elements.clear();

    QString fileName = getFileName(info[ENTITY_INFO_TARGET_FILENAME].toString());


    num_vertex=0;
    vbo_src.clear();
    valid= model_import(&model, fileName.toStdString(), &param);
    if(valid)
    {
        //update obj_id (group id)
        unsigned int h=0,a;
        for(object_list::iterator i=model.obj.begin();i!=model.obj.end();i++)
        {
            if(!(*i).visible) continue;

            if(sscanf_s((*i).name.c_str(),"#OBJ%d_",&a)==1)
            {
                (*i).obj_id=a;
                if(h<a) h=a;
            }
            else
            {
                (*i).obj_id=0;
            }
        }
        //sort by object group
        for(unsigned int j=0;j<=h;j++)
        {
            int k=0;
            for(object_list::iterator i=model.obj.begin();i!=model.obj.end();i++)
            {
                if(!(*i).visible) continue;
                if((*i).obj_id!=j) continue;
                (*i).top_of_group=(k==0);
                num_vertex += model_object_compile( (*i), model.mate, model.vtx, model_elements, vbo_src);
                k++;
            }

            {
                char prefix[32];
                sprintf(prefix,"#OBJ%d_",j);
                model_update_mask_prefix(&model,prefix,(1<<j));
                double _cg[3];
                model_object_get_cg(&model,(1<<j),_cg);
                cg.push_back( QVector3D((GLfloat)_cg[0],(GLfloat)_cg[1],(GLfloat)_cg[2]) );
                qDebug()<<"CG"<<j<<cg.back();


                joint_map::iterator jm=model.joi.find(j);
                if(jm!=model.joi.end())
                {
                    const joint_type &joint=jm->second;

                    axis.push_back(QVector3D(joint.axis[0],joint.axis[1],joint.axis[2]));
                }
                else
                {
                    axis.push_back(QVector3D(0.0f, 0.0f, 0.0f)); //no rotation
                }
            }
        }

    }

    emit done(this);
}

const char *gl_model_entity::get_vertex_shader(void) const
{
    static const char *vertexShaderSource =
        "attribute vec2 texCoord;\n"
        "attribute vec3 normal;\n"
        "attribute vec3 vertex;\n"
        "varying vec3 vert;\n"
        "varying vec3 vertNormal;\n"
        "varying vec4 vertColor;\n"
        "varying vec2 vertTexCoord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 mvMatrix;\n"
        "uniform mat3 normalMatrix;\n"
        "uniform highp vec3 lightPos;\n"
        "uniform vec4 matCol;\n"
        "uniform vec4 matAmb;\n"
        "uniform vec4 matEmi;\n"
        "uniform vec4 matDif;\n"
        "uniform vec4 matSpc;\n"
        "uniform highp int enaShading;\n"
        "uniform highp int enaTex;\n"

        "void main() {\n"
        "   if(enaShading==1){\n"
        "   vec3 P= vec3(mvMatrix * vec4(vertex, 1.0));\n"  //position in camera view
        "   vec3 L= normalize(vec3(lightPos)-P);\n"         //light direction unit vector
        "   vec3 N= normalize(normalMatrix*normal);\n"    //unit normal vector
        "   float dotLN=max(dot(L,N),0.0);\n"
        "   vec4  diffuseP=vec4(dotLN);\n"
        "   vec4  diffuse=diffuseP*matDif;\n"
        "   vertColor = matAmb + diffuse;\n"
        "   }else{\n"
        "   vertColor = matCol;\n"
        "   }\n"
        "   gl_Position = projMatrix * mvMatrix * vec4(vertex, 1.0);\n"
        "    vertTexCoord = texCoord;\n"
        "}\n";

    return vertexShaderSource;
}

const char *gl_model_entity::get_fragment_shader(void) const
{
    static const char *fragmentShaderSource =
        "varying lowp vec4 vertColor;\n"
        "varying lowp vec2 vertTexCoord;\n"
        "uniform highp int mode;\n"
        "uniform highp int enaShading;\n"
        "uniform highp int enaTex;\n"
        "uniform sampler2D texture;\n"
        "void main() {\n"
        "   if(mode==4){\n"
        "    highp int depth,r,g,b;\n"
//      "    depth=int(gl_FragCoord.z*4294967295.0);\n"
        "    depth=int(gl_FragCoord.z*16777216.0);\n"
        "    r=depth/16777216; depth=depth -r*16777216;\n"
        "    g=depth/65536;    depth=depth -g*65536;\n"
        "    b=depth/256;      depth=depth -b*256;\n"
        "    gl_FragColor = vec4(float(depth),float(b),float(g),float(r))/255.0;\n"
        "   }\n"
        "   else if(enaTex>0){\n"
        "    gl_FragColor = texture2D(texture, vertTexCoord);\n"
        "   }else{\n"
        "    lowp vec3 col = clamp(vertColor.xyz, 0.0,1.0);\n"
        "    gl_FragColor = vec4(col, 1.0);\n"
        "   }\n"
        "}\n";

    return fragmentShaderSource;
}

int gl_model_entity::prepare_gl(void)
{
    term_thread();

    qDebug() << "prepare_gl";

    prg = new QOpenGLShaderProgram;
    prg->addShaderFromSourceCode(QOpenGLShader::Vertex,  get_vertex_shader() );
    prg->addShaderFromSourceCode(QOpenGLShader::Fragment,get_fragment_shader() );

    prg->bindAttributeLocation("texCoord",0);
    prg->bindAttributeLocation("normal", 1);
    prg->bindAttributeLocation("vertex", 2);

    prg->link();
    prg->bind();

    projMat =prg->uniformLocation("projMatrix");
    mvMat   =prg->uniformLocation("mvMatrix");
    norMat  =prg->uniformLocation("normalMatrix");
    litPos  =prg->uniformLocation("lightPos");
    matCol  =prg->uniformLocation("matCol");
    matAmb  =prg->uniformLocation("matAmb");
    matEmi  =prg->uniformLocation("matEmi");
    matDif  =prg->uniformLocation("matDif");
    matSpc  =prg->uniformLocation("matSpc");
    enaShad =prg->uniformLocation("enaShading");
    enaTex  =prg->uniformLocation("enaTex");

    prg->setUniformValue("texture", 0);

    vbo_allocate();

    prg->release();

    model_load_all_texture(&model,textures);
    return 0;
}

void gl_model_entity::vbo_bind(void)
{
    if(vbo.bind())
    {
        QOpenGLFunctions *fc = QOpenGLContext::currentContext()->functions();

        fc->glEnableVertexAttribArray(0);
        fc->glEnableVertexAttribArray(1);
        fc->glEnableVertexAttribArray(2);
        fc->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);    //texture coord
        fc->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void *>(2 * sizeof(GLfloat)));    //normal
        fc->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void *>(5 * sizeof(GLfloat)));    //vertex
    }
    else
    {
        qDebug() << "VBO BIND ERROR";
    }
}

void gl_model_entity::vbo_allocate(void)
{
    vbo.create();
    if(!vbo.bind())
    {
        qDebug() << "VBO BIND ERROR";
    }
    else
    {
        qDebug()<<"VBO OBJECT ID#"<< vbo.bufferId();
        vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
        vbo.allocate(&vbo_src[0], (int)vbo_src.size()*sizeof(GLfloat));
        vbo.release();
    }
}

void gl_model_entity::update_group_matrix(int id,QMatrix4x4 &local)
{
    if(id<(int)axis.size())
    {
        if( axis[id].isNull() ) return;
    }
    QMatrix4x4 a,b,c;
    a.setToIdentity();  a.translate(-cg[id]);
    b.setToIdentity();  b.rotate(fmod(inc,360.0),axis[id]);
    c.setToIdentity();  c.translate(cg[id]);
    local=c*b*a;
}

void gl_model_entity::draw_gl(gl_draw_ctx_t &draw)
{
    if(!show()) return;

    QMatrix4x4 offset;
    if(!originOffset(offset)) return;

    int mode=0;

    if(draw.mode==GL_DRAW_PICK)
    {
        mode=OPT_PC_CM_DEPTH;
    }

    inc+=10.0;

    QOpenGLShaderProgram *p=prg;

    if(p)
    {
        QOpenGLFunctions *fc = QOpenGLContext::currentContext()->functions();

        p->bind();
        p->setUniformValue(projMat, draw.proj);
      //  p->setUniformValue(litPos, draw.lightpos);
        p->setUniformValue(litPos,QVector3D(0, 0, 100));
        fc->glEnable(GL_CULL_FACE);
        fc->glCullFace(GL_BACK);

        vbo_bind();

        for(model_elements_t::iterator i=model_elements.begin();i!=model_elements.end();i++)
        {
            QMatrix4x4 scale;
            scale.setToIdentity();
            scale.scale(draw.modelScale);
            if((*i)->group_top)
            {
                QMatrix4x4 x;
                x.setToIdentity();
                update_group_matrix((*i)->group_id, x);

                QMatrix4x4 modelview=draw.camera * draw.world * offset * local * x * scale;
                p->setUniformValue(mvMat, modelview);
                p->setUniformValue(norMat, modelview.normalMatrix());
            }

            const material_type& m= model.mate[ (*i)->idx_material ];
            p->setUniformValue(matCol, expand_material(m.col));
            p->setUniformValue(matAmb, expand_material(m.amb));
            p->setUniformValue(matEmi, expand_material(m.emi));
            p->setUniformValue(matDif, expand_material(m.dif));
            p->setUniformValue(matSpc, expand_material(m.spc));
            p->setUniformValue(enaShad,((*i)->shadeModel==GL_SMOOTH) );
            p->setUniformValue(enaTex,(int)m.tex_id );
            p->setUniformValue("mode", (int)mode);
            p->setUniformValue("texture", 0);
            if(m.tex_id>0)
            {
                textures[ m.tex_id ]->bind();
            }

            fc->glDrawArrays(GL_TRIANGLES, (GLint)(*i)->src_top , (GLsizei)(*i)->num_vertex);
//          fc->glDrawArrays(GL_POINTS, (GLint)(*i)->src_top , (GLsizei)(*i)->num_vertex);
        }
        fc->glDisable(GL_CULL_FACE);
        vbo.release();
        p->release();
    }
}


static void normal_2(vertex_type &A,vertex_type &B,vertex_type &C,vertex_type *normal)
{
    double norm;
    vertex_type vec0,vec1;

    vec0.x = A.x-C.x;  vec0.y = A.y-C.y;  vec0.z = A.z - C.z;
    vec1.x = B.x-C.x;  vec1.y = B.y-C.y;  vec1.z = B.z - C.z;

    normal->x = vec0.y * vec1.z - vec0.z * vec1.y;
    normal->y = vec0.z * vec1.x - vec0.x * vec1.z;
    normal->z = vec0.x * vec1.y - vec0.y * vec1.x;

    norm = normal->x * normal->x + normal->y * normal->y + normal->z * normal->z;
    norm = sqrt ( norm );

    normal->x /= norm; normal->y /= norm;  normal->z /= norm;
}


#if 0
    s = acos(normal.x*N[F.v[i]].x + normal.y*N[F.v[i]].y + normal.z*N[F.v[i]].z);
    if ( obj->facet < s ) {
        mat->vertex_t[dpos].normal[0] = normal.x;
        mat->vertex_t[dpos].normal[1] = normal.y;
        mat->vertex_t[dpos].normal[2] = normal.z;
    }
    else {
        mat->vertex_t[dpos].normal[0] = N[F.v[i]].x;
        mat->vertex_t[dpos].normal[1] = N[F.v[i]].y;
        mat->vertex_t[dpos].normal[2] = N[F.v[i]].z;
    }

#endif


static void decide_normal(vertex_type &ret,vertex_type &face,vertex_type *vertex,double cos_sm)
{
    ret.x=face.x;
    ret.y=face.y;
    ret.z=face.z;

    if(vertex!=NULL)
    {
#if 0
    return; //use only face normal
#endif
#if 0	//use only vertex normal
    face.x=vertex->x;
    face.y=vertex->y;
    face.z=vertex->z;

    return;
#endif

        if(std::abs(face.x*vertex->x + face.y*vertex->y + face.z*vertex->z)<cos_sm)
        {
        }
        else
        {
            ret.x=vertex->x;
            ret.y=vertex->y;
            ret.z=vertex->z;
        }
    }
}

const int p_idx1[3]={0,1,2};
const int p_idx2[3]={0,2,3};


static void polygon(face_type &face,vertex_list &v,vertex_type *n,int tex,double cos_sm, vbo_source_t &vbo_src)
{
    vertex_type normal,a,b,c,face_n,*vertex_n=NULL;
    int i,k;

    a=v[ face.v[p_idx1[0]] ];
    b=v[ face.v[p_idx1[1]] ];
    c=v[ face.v[p_idx1[2]] ];
    normal_2(a,b,c,&face_n);
    for (k=0;k<3;k++)		//0-1-2
    {
        i=p_idx1[k];
        if(tex)
        {
         // glTexCoord2d( face.uv[i*2], face.uv[i*2+1]);
            vbo_src.push_back(face.uv[i*2]);
            vbo_src.push_back(face.uv[i*2+1]);
        }
        else
        {   //push dummy
            vbo_src.push_back(0.0f);
            vbo_src.push_back(0.0f);
        }
        if(n!=NULL)	vertex_n=&n[ face.v[i] ];
        decide_normal(normal,face_n,vertex_n,cos_sm);
        //glNormal3d(normal.x,normal.y,normal.z);
        vbo_src.push_back(normal.x);
        vbo_src.push_back(normal.y);
        vbo_src.push_back(normal.z);
        //glVertex3d(v[ face.v[i] ].x,v[ face.v[i] ].y,v[ face.v[i] ].z);
        vbo_src.push_back(v[ face.v[i] ].x);
        vbo_src.push_back(v[ face.v[i] ].y);
        vbo_src.push_back(v[ face.v[i] ].z);
    }
    if(face.n==4)
    {
        a=v[ face.v[p_idx2[0]] ];
        b=v[ face.v[p_idx2[1]] ];
        c=v[ face.v[p_idx2[2]] ];
        normal_2(a,b,c,&face_n);
        for (k=0;k<3;k++)	//0-2-3
        {
            i=p_idx2[k];
            if(tex)
            {
                //glTexCoord2d( face.uv[i*2], face.uv[i*2+1]);
                vbo_src.push_back(face.uv[i*2]);
                vbo_src.push_back(face.uv[i*2+1]);
            }
            else
            {   //push dummy
                vbo_src.push_back(0.0f);
                vbo_src.push_back(0.0f);
            }
            if(n!=NULL)	vertex_n=&n[ face.v[i] ];
            decide_normal(normal,face_n,vertex_n,cos_sm);
            //glNormal3d(normal.x,normal.y,normal.z);
            vbo_src.push_back(normal.x);
            vbo_src.push_back(normal.y);
            vbo_src.push_back(normal.z);
            //glVertex3d(v[ face.v[i] ].x,v[ face.v[i] ].y,v[ face.v[i] ].z);
            vbo_src.push_back(v[ face.v[i] ].x);
            vbo_src.push_back(v[ face.v[i] ].y);
            vbo_src.push_back(v[ face.v[i] ].z);
        }
    }
}

static void expand_material(double x[4],GLfloat *ret)
{
    ret[0]=(GLfloat)x[0];
    ret[1]=(GLfloat)x[1];
    ret[2]=(GLfloat)x[2];
    ret[3]=(GLfloat)x[3];
}

static QVector4D expand_material(const double x[4])
{
    return QVector4D((GLfloat)x[0],(GLfloat)x[1],(GLfloat)x[2],(GLfloat)x[3]);
}

static void vertex_add_to(vertex_type &a,vertex_type &b)
{
    //a+=b;
    a.x+=b.x;
    a.y+=b.y;
    a.z+=b.z;
}

static void normalize_vertex(vertex_type &a)
{
    double t;
    t=a.x * a.x +a.y*a.y +a.z*a.z;
    if(t!=0.0)
    {
        t=sqrt(t);
        a.x/=t;
        a.y/=t;
        a.z/=t;
    }
}

vertex_type *get_normal(object_type &object,vertex_list &v)
{
    vertex_type normal,a,b,c;
    vertex_type *ret=NULL;
    size_t i,n;

    n=v.size();
    ret=new vertex_type [n];
    memset(ret,0,sizeof(vertex_type)*n);

    for(face_list::iterator j=object.f.begin();j!=object.f.end();j++)
    {
        face_type &face=(*j);

        a=v[ face.v[p_idx1[0]] ];
        b=v[ face.v[p_idx1[1]] ];
        c=v[ face.v[p_idx1[2]] ];
        normal_2(a,b,c,&normal);

        vertex_add_to(ret[face.v[p_idx1[0]]],normal);
        vertex_add_to(ret[face.v[p_idx1[1]]],normal);
        vertex_add_to(ret[face.v[p_idx1[2]]],normal);

        if(face.n==4)
        {
            a=v[ face.v[p_idx2[0]] ];
            b=v[ face.v[p_idx2[1]] ];
            c=v[ face.v[p_idx2[2]] ];
            normal_2(a,b,c,&normal);
            vertex_add_to(ret[face.v[p_idx2[0]]],normal);
            vertex_add_to(ret[face.v[p_idx2[1]]],normal);
            vertex_add_to(ret[face.v[p_idx2[2]]],normal);
        }
    }

    for(i=0;i<n;i++)
    {
        normalize_vertex(ret[i]);
    }
    return ret;
}

static size_t model_object_compile(object_type &object, material_list &materials, vertex_list &gv, model_elements_t &elements, vbo_source_t &src)
{
    int m=0;
    size_t ret=0;
    vertex_list *v;
    model_element_t *me=NULL;

    if(object.v.size()==0)
    {
        v=&gv;
    }
    else
    {
        v=&object.v;
    }

    vertex_type *normals=NULL;
    normals=get_normal(object,*v);

    //qDebug() << QString::fromUtf16((const ushort*)object.name.c_str());

    typedef std::map<int,vbo_source_t*> src_for_mat_t;
    src_for_mat_t src_for_mat;

    m=0;
    for(material_list::iterator i=materials.begin();i!=materials.end();i++)
    {
        src_for_mat[m]=new vbo_source_t;
        m++;
    }

    double facet=std::cos(object.facet*d2r);
    for(face_list::iterator j=object.f.begin();j!=object.f.end();j++)
    {
        int m=(*j).m;
        if((m<0) || (m>=(int)materials.size())) continue;
        polygon((*j),*v,normals, materials[m].tex!="" ,facet, *src_for_mat[m]);
    }


    m=0;
    for(material_list::iterator i=materials.begin();i!=materials.end();i++)
    {

        vbo_source_t *srcm=src_for_mat[m];

        //qDebug() << QString::fromUtf16((const ushort*)(*i).name.c_str()) << srcm->size();

        if(srcm->size())
        {
            me=new model_element_t;
            me->shadeModel=object.shading?GL_SMOOTH:GL_FLAT;
            me->idx_material=m;
            me->src_top=src.size()/8;           //top of vertex
            me->num_vertex = srcm->size()/8;    //number of vertex
            me->group_id=object.obj_id;
            me->group_top=object.top_of_group;
            for(vbo_source_t::iterator j=srcm->begin(); j!=srcm->end();j++) src.push_back( (*j) );
            elements.push_back(me);
            ret+=me->num_vertex;    //number of GLfloat variables
        }
        delete srcm;
        m++;

    }

    if(normals!=NULL) delete [] normals;
    return ret;
}


static GLuint model_load_texture(const char *filename,const char *model_path,textures_t &textures);

static void model_load_all_texture(model_type *model,textures_t &textures)
{
    fs::path ps(model->path);
    std::string model_path=ps.remove_filename().string();

    for(material_list::iterator i=model->mate.begin();i!=model->mate.end();i++)
    {
        if((*i).tex!="")
        {
            (*i).tex_id=model_load_texture( (*i).tex.c_str(),model_path.c_str(),textures );
        }
        else
        {
            (*i).tex_id=0;
        }
        //qDebug() << QString::fromUtf16((const ushort*)(*i).name.c_str()) << (*i).tex_id;
#if 0
        if( (*i).tex_id==0 && (*i).tex!=L"" )
        {
            (*i).tex_id=model_load_texture( (*i).tex.c_str(),model_path.c_str(),textures );
        }
#endif
    }
}

static GLuint model_load_texture(const char *filename, const char *model_path, textures_t &textures)
{
    GLuint ret=0;

    int img_ok=1;

    fs::path mp(model_path);
    std::string alt_filename=mp.replace_filename(filename).string();

    QString _filename=QString(filename);
    QString _alt_filename=QString(alt_filename.c_str());

    QImage image;
    if(!image.load(_filename))
    {
        qDebug()<< "Image file" << _filename << "load error. try alternative one...\n";
        if(!image.load(_alt_filename))
        {
            qDebug()<< "Alternative Image file" << _alt_filename << "load error.\n";
            img_ok=0;
        }
        else
        {
            qDebug()<< "Alternative Image file" << _alt_filename << "load ok.\n";
        }
    }
    else
    {
        qDebug()<< "Image file" << _filename << "load ok.\n";
    }
    if(!img_ok)
    {
        return 0;
    }

    QOpenGLTexture *texture;

    texture = new QOpenGLTexture(image.mirrored());
    texture->setMinificationFilter(QOpenGLTexture::Nearest);    // Set nearest filtering mode for texture minification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);    // Set bilinear filtering mode for texture magnification
    texture->setWrapMode(QOpenGLTexture::Repeat);               // Wrap texture coordinates by repeating (ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2))

    ret=texture->textureId();

    textures[ret]=texture;

    return ret;
}
















