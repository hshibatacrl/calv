/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "gl_pcloud_entity.h"
#include "pointcloud_packet.h"

#include <QOpenGLShaderProgram>
#include <QFileInfo>
#include <QStandardPaths>
#include <QThread>


#define DRAFT_DRAW_POINTS (1000000)

QMap<int, QOpenGLShaderProgram*> gl_pcloud_entity::_prg;
std::mutex gl_pcloud_entity::_prgMutex;
int gl_pcloud_entity::_prgCount=0;

gl_pcloud_entity::gl_pcloud_entity(QObject *parent) : gl_entity_ctx(parent)
{
    _format = 0;

    _vertex=nullptr;
    _nVertex=0;
    _nElement=0;

    _rgb = -1;
    _amp = -1;
    _rng = -1;
    _flg = -1;

    setObjectName("PointCloud");
}

gl_pcloud_entity::~gl_pcloud_entity()
{
    cleanup();
    qDebug()<<"gl_pcloud_entity::~gl_pcloud_entity()";
}


void gl_pcloud_entity::init_opt_pc(opt_pointcloud_t &p)
{
    p.color_mode = OPT_PC_CM_HEIGHT;
    p.flags = 0;
    p.psz=3.0f;
    p.amp[0]=0.0f;
    p.amp[1]=255.0f;
    p.rng[0]=0.0f;
    p.rng[1]=255.0f;
    p.hgt[0]=0.0f;
    p.hgt[1]=255.0f;
    p.flt_amp[0]=p.flt_amp[1]=0.0f;
    p.flt_rng[0]=p.flt_rng[1]=0.0f;
    p.flt_hgt[0]=p.flt_hgt[1]=0.0f;
    p.ignoreDraft=true;
}

void gl_pcloud_entity::cleanup(void)
{
    if(_vertex!=NULL)
    {
        delete [] _vertex;
        _vertex=nullptr;
    }

    {
        std::lock_guard lock(_prgMutex);

        if(!--_prgCount)
        {
            foreach(auto &i, _prg)
            {
                i->deleteLater();
            }
            _prg.clear();
        }
    }
}


int gl_pcloud_entity::load_mem(const uint8_t *buf, size_t length)
{
    const pc_packet_header_t *header = (const pc_packet_header_t *) buf;
    const uint8_t *top = buf;
    if (header->magic == PC_MAGIC)
    {
        if(header->length <= length)
        {
            top+= sizeof(pc_packet_header_t);
            if(header->type & PC_ORG)
            {//optional
                 const pc_origin_t *org = (const pc_origin_t *)top;

                 GLfloat x = org->xyz[0];    //Right
                 GLfloat y = org->xyz[1];    //Down
                 GLfloat z = org->xyz[2];    //Forward
                 _localOrigin = QVector3D(z,-x,-y); // East-North-Up
                 top+=sizeof(pc_origin_t);
            }

            if(header->type & PC_TXT)
            {//optional
                const pc_text_t *txt = (const pc_text_t *)top;
                top += sizeof(pc_text_t);
                std::string text( (const char*)top, txt->length );
                setObjectName(QString::fromStdString(text));
                top += txt->length;
            }

            //required block
            const pc_payload_t* pc = (const pc_payload_t*)top;

            int p=3;
            if(pc->format & PC_RGB){ _rgb = p; p+=3; }
            if(pc->format& PC_AMP){ _amp = p; p+=1; }
            if(pc->format & PC_RNG){ _rng = p; p+=1; }
            _flg = p; p+=1; //add a flag

            _format = pc->format & 0x000000ff;
            _nElement = p;
            _nVertex = (quint64) pc->nPoints;
            _vertex = new GLfloat [_nElement*_nVertex];

            top+=sizeof(pc_payload_t);
            const GLfloat *v=(const GLfloat *)top;
            GLfloat *w=_vertex;
            for(quint64 i=0;i<_nVertex;i++)
            {
                GLfloat x = *v++;    //Right
                GLfloat y = *v++;    //Down
                GLfloat z = *v++;    //Forward
                *w++ =  z; //East
                *w++ = -x; //North
                *w++ = -y; //Up
                memcpy(w, v, sizeof(GLfloat)*(_nElement-1-3));
                v+=_nElement-1-3;
                w+=_nElement-1-3;
                *w=0.0f;    //flag
                w++;
            }

            return _nVertex;
        }
    }
    return 0;
}

void gl_pcloud_entity::load(void)
{
    thread()->setPriority(QThread::LowPriority);
    valid=0;
    int r=0;
    QString targetFileName=info[ENTITY_INFO_TARGET_FILENAME].toString();
    QByteArray targetBytes = info[ENTITY_INFO_TARGET_BYTES].toByteArray();
    if(!targetFileName.isEmpty())
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
        qDebug()<<"gl_pcloud_entity::load error";
    }

    if(r)
    {

        valid=1;
    }

    _vboCtx.total=_nVertex;
    _vboCtx.remain=_nVertex;
    _vboCtx.vertex=&_vertex[0];
    _vboCtx.curTop=&_vertex[0];
    _vboCtx.mode=0;
   // emitProgress(0,0,"",true);
    emit done(this);
}

QVector3D gl_pcloud_entity::getCenter(void)
{
    QVector3D ret(0.0f,0.0f,0.0f);
    if(_nVertex>3)
    {
        ret.setX(_vertex[0]);
        ret.setY(_vertex[1]);
        ret.setZ(_vertex[2]);
    }
    return ret;
}

int gl_pcloud_entity::update_draw_gl(gl_draw_ctx_t &draw)
{
    int ret=0;
    return ret;
}

int gl_pcloud_entity::rebuildRequest(void)   //rebuild VBO
{
    _vboCtx.mode=1;  //rebuild
    _vboCtx.counter=0;
    _vboCtx.remain=_nVertex;
    _vboCtx.curTop=&_vertex[0];
    return 1;
}

int gl_pcloud_entity::prepare_gl(void)
{
    term_thread();

    {
        std::lock_guard lock(_prgMutex);
        if(!_prg.size())
        {
            auto x=new QOpenGLShaderProgram;
            if(x->addShaderFromSourceFile(QOpenGLShader::Vertex,":/gl/gl_pcloud_entity.vert"))
            {
                qDebug()<<"gl_pcloud_entity Vertex Shader OK";
            }
            if(x->addShaderFromSourceFile(QOpenGLShader::Fragment,":/gl/gl_pcloud_entity1.frag"))
            {
                qDebug()<<"gl_pcloud_entity Fragment Shader1 OK";
            }

            x->bindAttributeLocation("vertex", 0);
            x->bindAttributeLocation("rgb", 1);
            x->bindAttributeLocation("amp", 2);
            x->bindAttributeLocation("range", 3);
            x->bindAttributeLocation("flags", 4);
            x->link();
            _prg[0] = x;

            auto y=new QOpenGLShaderProgram;
            if(y->addShaderFromSourceFile(QOpenGLShader::Vertex,":/gl/gl_pcloud_entity.vert"))
            {
                qDebug()<<"gl_pcloud_entity Vertex Shader OK";
            }
            if(y->addShaderFromSourceFile(QOpenGLShader::Fragment,":/gl/gl_pcloud_entity2.frag"))
            {
                qDebug()<<"gl_pcloud_entity Fragment Shader2 OK";
            }

            y->bindAttributeLocation("vertex", 0);
            y->bindAttributeLocation("rgb", 1);
            y->bindAttributeLocation("amp", 2);
            y->bindAttributeLocation("range", 3);
            y->bindAttributeLocation("flags", 4);
            y->link();
            _prg[1] = y;

        }
        _prgCount++;
    }

    partialVBOallocation();

/*  KEEP IT FOR FILTERING
    //vertex is not necessary any more
    delete [] vertex;
    vertex=NULL;
*/

    return _vboCtx.remain>0;
}

void gl_pcloud_entity::vbo_bind(QOpenGLBuffer &vbo,QOpenGLFunctions *f)
{
    if(vbo.bind())
    {
        f->glEnableVertexAttribArray(0);
        if(_rgb>0) f->glEnableVertexAttribArray(1);
        if(_amp>0) f->glEnableVertexAttribArray(2);
        if(_rng>0) f->glEnableVertexAttribArray(3);
        if(_flg>0) f->glEnableVertexAttribArray(4);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, _nElement * sizeof(GLfloat), 0);
        if(_rgb>0) f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, _nElement * sizeof(GLfloat), reinterpret_cast<void *>(_rgb * sizeof(GLfloat)));
        if(_amp>0) f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, _nElement * sizeof(GLfloat), reinterpret_cast<void *>(_amp * sizeof(GLfloat)));
        if(_rng>0) f->glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, _nElement * sizeof(GLfloat), reinterpret_cast<void *>(_rng * sizeof(GLfloat)));
        if(_flg>0) f->glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, _nElement * sizeof(GLfloat), reinterpret_cast<void *>(_flg * sizeof(GLfloat)));
    }
    else
    {
        qDebug() << "VBO BIND ERROR";
    }
}

void gl_pcloud_entity::vbo_release(QOpenGLBuffer &vbo,QOpenGLFunctions *fc)
{
    vbo.release();
    fc->glDisableVertexAttribArray(0);
    if(_rgb>0) fc->glDisableVertexAttribArray(1);
    if(_amp>0) fc->glDisableVertexAttribArray(2);
    if(_rng>0) fc->glDisableVertexAttribArray(3);
    if(_flg>0) fc->glDisableVertexAttribArray(4);
}


int gl_pcloud_entity::pertialPrepare_gl(void)
{
    partialVBOallocation();
    partialVBOallocation();
    partialVBOallocation();
    partialVBOallocation();
    return _vboCtx.remain>0;
}

void gl_pcloud_entity::partialVBOallocation(void)
{    
    if(_vboCtx.remain)
    {
        quint64 m=0x1ffff;
        //quint64 m=MAX_VBO_SIZE;//0x1ffff;
        quint64 remain=_vboCtx.remain;
        GLfloat *p=_vboCtx.curTop;

        vbo_t *vbo;

        if(_vboCtx.mode==0)
        {   //create
            vbo=new vbo_t;
            vbo->vbo.create();
        }
        else
        {   //update
            vbo= &_vvbo[ _vboCtx.counter++ ];
        }
        vbo->vbo.bind();
        int n;
        if(remain<=m)
        {
            n=remain;
            remain=0;
        }
        else
        {
            n=m;
            remain-=m;
        }
        if(_vboCtx.mode==0)
        {//create
            vbo->vbo.allocate(p, n*_nElement*sizeof(GLfloat));
        }
        else
        {
            vbo->vbo.write(0,p,n*_nElement*sizeof(GLfloat));
        }
        vbo->vbo.release();
        p+=n*_nElement;
        vbo->n=n;

        _vboCtx.remain=remain;
        _vboCtx.curTop=p;

        if(_vboCtx.mode==0)
        {
            _vvbo.push_back(*vbo);
        }

        if(remain)
        {
            qDebug() << "vbo_size" << n << "vtx " <<n*_nElement*sizeof(GLfloat) << "bytes." << remain <<"pts left.";
            emitProgress(_vboCtx.total-_vboCtx.remain,_vboCtx.total,"VBO",false);
        }
        else
        {
            qDebug() << "vbo allocate has done.";
            emitProgress(_vboCtx.total,_vboCtx.total,"VBO",true);
        }
    }
}



void gl_pcloud_entity::draw_gl(gl_draw_ctx_t &draw)
{
    if(!show()) return;

    QMatrix4x4 offset;
    if(!originOffset(offset)) return;


    QOpenGLShaderProgram *p=draw.pointAntiAlias?_prg[0]:_prg[1];
    GLfloat psz;
    quint64 n=_nVertex,m;
    int mode;

    GLfloat z0 = _localOrigin.z();

    mode=draw.opt_pc.color_mode;
    psz=draw.opt_pc.psz;

    if(draw.mode==GL_DRAW_PICK)
    {
        mode=OPT_PC_CM_DEPTH;
        psz=10.0f;
    }

    if(draw.mode==GL_DRAW_TEMP)
    {
        if(!draw.opt_pc.ignoreDraft)
        {
    //        n=_draftUpdatePoint;
        }
        //psz=1.0f;
    }

    if(p)
    {
        QOpenGLFunctions *fc = QOpenGLContext::currentContext()->functions();

        QMatrix4x4 modelViewProj=draw.proj*draw.camera * draw.world * offset * local;
        p->bind();

        fc->glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

        if(draw.pointAntiAlias) fc->glEnable(GL_POINT_SPRITE);

        p->setUniformValue("mvpMatrix", modelViewProj);

        const auto &amp = draw.opt_pc.amp;
        const auto &rng = draw.opt_pc.rng;
        const auto &hgt = draw.opt_pc.hgt;
        p->setUniformValue("a_range", QVector3D(amp[0], amp[1], amp[1]-amp[0]));
        p->setUniformValue("r_range", QVector3D(rng[0], rng[1], rng[1]-rng[0]));
        p->setUniformValue("z_range", QVector3D(hgt[0]-z0, hgt[1]-z0, hgt[1]-hgt[0]));
        p->setUniformValue("mode", (int)mode);
        p->setUniformValue("pointsize", psz);

        const auto &famp = draw.opt_pc.flt_amp;
        const auto &frng = draw.opt_pc.flt_rng;
        const auto &fhgt = draw.opt_pc.flt_hgt;

        p->setUniformValue("fltAEnable", (int)(famp[0]<famp[1]));
        p->setUniformValue("fltA", QVector2D(famp[0], famp[1]));

        p->setUniformValue("fltREnable", (int)(frng[0]<frng[1]));
        p->setUniformValue("fltR", QVector2D(frng[0], frng[1]));

        p->setUniformValue("fltZEnable", (int)(fhgt[0]<fhgt[1]));
        p->setUniformValue("fltZ",QVector2D(fhgt[0]-z0, fhgt[1]-z0));

        p->setUniformValue("antiAlias", (int)draw.pointAntiAlias);

        for(vvbo_t::iterator i=_vvbo.begin(); i!=_vvbo.end(); i++)
        {
            m=n;
            if(i->n<m) m=i->n;

            vbo_bind(i->vbo,fc);
            fc->glDrawArrays(GL_POINTS, 0,m);
            //qDebug()<< "glDrawArrays "<<i->n<<m;
            vbo_release(i->vbo,fc);

            n=n-m;
        }

        if(draw.pointAntiAlias) fc->glDisable(GL_POINT_SPRITE);

        fc->glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

        p->release();
    }

}
