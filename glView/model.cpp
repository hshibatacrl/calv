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

#if __GNUC__==7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif __GNUC__>7
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <algorithm>
#include <cctype>
#include <string>
#include <cstring>
#include <cmath>

#include "model.h"

void reset_model_prop(model_prop_type *p)
{
	int i;
	p->visible=1;

	for(i=0;i<3;i++)
	{
		p->pos[i]=0.0;
		p->rpy[i]=0.0;
		p->scale[i]=1.0;
	}
}

void reset_object(object_type *o)
{
	o->visible=15;
	o->shading=1;
	o->scale[0]=o->scale[1]=o->scale[2]=1.0;
	o->rot[0]  =o->rot[1]  =o->rot[2]=0.0;
	o->trans[0]=o->trans[1]=o->trans[2]=0.0;
	o->facet=50.0;
    o->name="";
	o->v.clear();
	o->f.clear();
	o->obj_id=0;
	o->mask_id=1;
    o->top_of_group=0;
}

void reset_model(model_type *m)
{
	m->vtx.clear();
	m->mate.clear();
	m->obj.clear();
    m->path="";
	m->flag=0;
}


void reset_material(material_type *m)
{
	int i;
	for(i=0;i<4;i++)
	{
		m->amb[i]=(i==3)?(1.0):(0.0);
		m->dif[i]=(i==3)?(1.0):(0.0);
		m->emi[i]=(i==3)?(1.0):(0.0);
		m->spc[i]=(i==3)?(1.0):(0.0);
		m->col[i]=1.0;
	}
	m->pwr=0.0;
    m->name="";
    m->tex="";
	m->tex_id=0;
}

void reset_face(face_type *f)
{
	int i;
	f->n=0;
	f->m=-1;
	f->v[0]=f->v[1]=f->v[2]=f->v[3]=-1;
	for(i=0;i<8;i++) f->uv[i]=0.0;
	f->col=0;
}

int model_import(model_type *model, const std::string &fname, model_import_params_t *para)
{
	int r=0;
    fs::path ps(fname);
    std::string ext=ps.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });

    if(ext==".mqo")
	{
        r=import_mqo(fname.c_str(),model,para);
	}
    else if(ext==".obj")
	{
        r=import_obj(fname.c_str(),model);
	}
    else if(ext==".w2r")
	{
        r=import_w2r(fname.c_str(),model);
	}
    else if(ext=="")
	{//empty, may be stock model
		r=2;
	}

	if(!r)
	{
    //    qDebug()<< fname << "load failed";
	}
	else if(r==1)
	{
    //    qDebug()<< fname << "load ok";
	}

	return r;
}


//------------------------------------------------------------------


#if 0
void model_terminate(model_type *model)
{
	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
		GLuint id=(*i).obj_id;
		if(id>0)
		{
			glDeleteLists(id,1);
			(*i).obj_id=0;
		}
	}
	for(material_list::iterator i=model->mate.begin();i!=model->mate.end();i++)
	{
		GLuint id=(*i).tex_id;
		if(id>=0)
		{
			glDeleteTextures(1,&id);
			(*i).tex_id=0;
		}
	}
}
#endif

static void vertex_scaling(vertex_list &v,double scale)
{
	for(vertex_list::iterator i=v.begin();i!=v.end();i++)
	{
		(*i).x *=scale;
		(*i).y *=scale;
		(*i).z *=scale;
	}
}

void model_scaling(model_type *model,double scale)
{
	vertex_scaling(model->vtx,scale);
	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
		vertex_scaling((*i).v,scale);
	}
}

static void vertex_offset(vertex_list &v,const double x,const double y,const double z)
{
	for(vertex_list::iterator i=v.begin(); i!=v.end();i++)
	{
		i->x+=x;
		i->y+=y;
		i->z+=z;
	}
}

void model_offset(model_type *model,const double x,const double y,const double z)
{
	vertex_offset(model->vtx,x,y,z);
	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
		vertex_offset((*i).v,x,y,z);
	}
}

int model_object_get_cg(object_type *obj,double *ret)
{
	if(obj->v.size())
	{
		ret[0]=ret[1]=ret[2]=0.0;
		for(vertex_list::iterator i=obj->v.begin(); i!=obj->v.end();i++)
		{
			ret[0]+=i->x;
			ret[1]+=i->y;
			ret[2]+=i->z;
		}
		double in=1.0/(obj->v.size());
		ret[0]*=in;
		ret[1]*=in;
		ret[2]*=in;
		return 1;
	}
	return 0;
}

int model_object_get_cg(model_type *model,int mask_bits,double *ret)
{
	int n=0;
	ret[0]=0.0;
	ret[1]=0.0;
	ret[2]=0.0;
	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
		if(i->mask_id & mask_bits)
		{
			double temp[3];
			if(model_object_get_cg(&(*i),temp))
			{
				ret[0]+=temp[0];
				ret[1]+=temp[1];
				ret[2]+=temp[2];
				n++;
			}
		}
	}
	if(n)
	{
		double in=1.0/n;
		ret[0]*=in;
		ret[1]*=in;
		ret[2]*=in;
		return 1;
	}
	return 0;
}


object_type *model_get_object(model_type *model, const std::string &name)
{
	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
        if(name == i->name)
		{
			return &(*i);
		}
	}
	return NULL;
}

int model_update_mask_prefix(model_type *model,const std::string &prefix,int mask_bits)
{
	int ret=0;
	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
        if(strncmp(prefix.c_str(),i->name.c_str(),strlen(prefix.c_str()))==0)
		{
			i->mask_id=mask_bits;
			ret++;
		}
	}
	return ret;
}
#if 0
int model_draw_objects(model_type *model,model_prop_type *prop,int mask_bits)
{
	if(prop!=NULL)
	{
		if(!prop->visible) return 0;
	}

	glPushAttrib( GL_ALL_ATTRIB_BITS );

	if(prop!=NULL)
	{
		glEnable(GL_NORMALIZE);
		glTranslated(prop->pos[0],prop->pos[1],prop->pos[2]);
		glRotated(prop->rpy[0],1.0,0.0,0.0);
		glRotated(prop->rpy[1],0.0,1.0,0.0);
		glRotated(prop->rpy[2],0.0,0.0,1.0);
		glScaled(prop->scale[0],prop->scale[1],prop->scale[2]);
	}

	for(object_list::iterator i=model->obj.begin();i!=model->obj.end();i++)
	{
		if((*i).visible && (mask_bits & (*i).mask_id))
		{
			if( (*i).obj_id==0 )
			{
				model_load_all_texture(model);
				if(!model_object_compile((*i),model->mate,model->vtx))
				{
					(*i).visible=0;	//error
				}
			}
			if( (*i).obj_id>0 )
			{
				glCallList((*i).obj_id);
			}
		}
	}
	if(prop!=NULL)
	{
		glDisable(GL_NORMALIZE);
	}

	glPopAttrib();

	return 1;
}
#endif


size_t export_string(FILE *fp,const std::string &x)
{
	size_t ret=0;
	uint64_t n=x.length();
	ret+=fwrite(&n,sizeof(uint64_t),1,fp);
    ret+=fwrite(&x[0],sizeof(char),n,fp);
	return ret;
}

int import_string(FILE *fp,std::string &x)
{
	int ret=0;
	uint64_t n;
	if(fread(&n,sizeof(uint64_t),1,fp))
	{
		if(n)
		{
            char *buf=new char [n+1];
            if(fread(buf,sizeof(char),n,fp)==n)
			{
				buf[n]=0;
				x=buf;
				ret=1;
			}
            delete [] buf;
		}
		else
		{
            x="";
			ret=1;
		}
	}
	return ret;
}

template<class T> size_t export_model_type(FILE *fp,const T &x)
{
	size_t ret=0;
	ret=fwrite(&x,sizeof(T),1,fp);
	return ret;
}

template<class T> int import_model_type(FILE *fp,T &x)
{
	int ret=0;
	if(fread(&x,sizeof(T),1,fp))
	{
		ret=1;
	}
	return ret;
}

template<class T> size_t export_model_list(FILE *fp,const std::vector<T> &x)
{
	size_t ret=0;
	uint64_t n=x.size();
	ret+=fwrite(&n,sizeof(uint64_t),1,fp);
    for(auto i=x.cbegin();i!=x.cend();i++)
	{
		ret+=export_model_type(fp,(*i));
	}
	return ret;
}

template<class T> int import_model_list(FILE *fp,std::vector<T> &x)
{
	uint64_t i,n;
	T y;
	x.clear();
	if(fread(&n,sizeof(uint64_t),1,fp))
	{
		for(i=0;i<n;i++)
		{
			if(import_model_type(fp,y))
			{
				x.push_back(y);
			}
			else
			{
				break;
			}
		}
	}
	return x.size()==(size_t)n;
}



template <> size_t export_model_type<material_type>(FILE *fp,const material_type &m)
{
	size_t ret=0;
	ret+=fwrite(&m.col,sizeof(double),4,fp);
	ret+=fwrite(&m.dif,sizeof(double),4,fp);
	ret+=fwrite(&m.amb,sizeof(double),4,fp);
	ret+=fwrite(&m.emi,sizeof(double),4,fp);
	ret+=fwrite(&m.spc,sizeof(double),4,fp);
	ret+=fwrite(&m.pwr,sizeof(double),1,fp);
    ret+=export_string(fp,m.name);
    ret+=export_string(fp,m.tex);
	return ret;
}

template <> int import_model_type<material_type>(FILE *fp,material_type &m)
{
	int ret=0;

	m.tex_id=0;
	ret=fread(&m.col,sizeof(double),4,fp)==4;
	ret&=fread(&m.dif,sizeof(double),4,fp)==4;
	ret&=fread(&m.amb,sizeof(double),4,fp)==4;
	ret&=fread(&m.emi,sizeof(double),4,fp)==4;
	ret&=fread(&m.spc,sizeof(double),4,fp)==4;
	ret&=fread(&m.pwr,sizeof(double),1,fp)==1;

	if(ret)
	{
		ret=0;
        if(import_string(fp,m.name))
		{
            if(import_string(fp,m.tex))
			{
				ret=1;
			}
		}
	}

	return ret;
}

template <> size_t export_model_type<object_type>(FILE *fp,const object_type &x)
{
	size_t ret=0;
	ret+=fwrite(&x.visible,sizeof(int),1,fp);
	ret+=fwrite(&x.shading,sizeof(int),1,fp);
	ret+=fwrite(&x.scale,sizeof(double),3,fp);
	ret+=fwrite(&x.rot,sizeof(double),3,fp);
	ret+=fwrite(&x.trans,sizeof(double),3,fp);
	ret+=fwrite(&x.facet,sizeof(double),1,fp);
	ret+=fwrite(&x.obj_id,sizeof(unsigned int),1,fp);
	ret+=export_model_list(fp,x.v);
	ret+=export_model_list(fp,x.f);
    ret+=export_string(fp,x.name);
	return ret;
}

template <> int import_model_type<object_type>(FILE *fp,object_type &x)
{
	int ret=0;
	ret=fread(&x.visible,sizeof(int),1,fp)==1;
	ret&=fread(&x.shading,sizeof(int),1,fp)==1;
	ret&=fread(&x.scale,sizeof(double),3,fp)==3;
	ret&=fread(&x.rot,sizeof(double),3,fp)==3;
	ret&=fread(&x.trans,sizeof(double),3,fp)==3;
	ret&=fread(&x.facet,sizeof(double),1,fp)==1;
	ret&=fread(&x.obj_id,sizeof(unsigned int),1,fp)==1;
	x.mask_id=1;
	if(ret)
	{
		ret=0;
		if(import_model_list(fp,x.v))
		{
			if(import_model_list(fp,x.f))
			{
                if(import_string(fp,x.name))
				{
					ret=1;
				}
			}
		}
	}

	return ret;
}


template <> size_t export_model_type<model_type>(FILE *fp,const model_type &x)
{
	size_t ret=0;
    ret+=export_string(fp,x.path);
	ret+=export_model_list(fp,x.mate);
	ret+=export_model_list(fp,x.obj);
	ret+=export_model_list(fp,x.vtx);
	return ret;
}

template <> int import_model_type<model_type>(FILE *fp,model_type &x)
{
	int ret=0;
    if(import_string(fp,x.path))
	{
		if(import_model_list(fp,x.mate))
		{
			if(import_model_list(fp,x.obj))
			{
				if(import_model_list(fp,x.vtx))
				{
					ret=1;
				}
			}
		}
	}
	return ret;
}

int import_w2r(const char *fname,model_type *data)
{
	int ret=0;
	FILE *fp;
	reset_model(data);
    fp=fopen(fname,"rb");
    if(fp!=NULL)
	{
		ret=import_model_type(fp,*data);
		fclose(fp);
	}
	if(ret)
	{
		data->path=fname;
	}
	return ret;
}

int export_w2r(const char *fname, model_type *data)
{
	int ret=0;
	FILE *fp;
//	reset_model(data);
    fp=fopen(fname,"wb");
    if(fp!=NULL)
	{
		ret=export_model_type(fp,*data);
		fclose(fp);
	}
	return ret;
}
