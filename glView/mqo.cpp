/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "model.h"
#include <stdio.h>
#include <cstring>

class mqoImport
{
#define LINE_BUF_SIZE 1024
    std::string line_;
    std::string temp_;
    char *line;
    char *temp;
    FILE *fp;

	int get_line(void);
	int parse_material(int n,material_list *mate);
    int parse_w2r(int n,model_type *data);
    int parse_object(char *name,object_list *obj);
	int parse_vertex(int n,vertex_list *vertex);
	int parse_face(int n,face_list *face);

	int discard_hidden;
	double vertex_mat[16];
    joint_map joi;
	void parse_param(model_import_params_t *para);
public:
    mqoImport();
    ~mqoImport();
    int load(const char *x,model_type *data,model_import_params_t *para);
};


mqoImport::mqoImport()
{
    line_.resize(LINE_BUF_SIZE);
    temp_.resize(LINE_BUF_SIZE);
    line=&line_[0];
    temp=&temp_[0];
	fp=NULL;
}

mqoImport::~mqoImport()
{
}

int mqoImport::get_line(void)
{
    return fgets(line,LINE_BUF_SIZE,fp)!=NULL;
}

static void expand_material(double x,double c[4],double *ret)
{
	ret[0]=c[0]*x;
	ret[1]=c[1]*x;
	ret[2]=c[2]*x;
	ret[3]=c[3];
}

int mqoImport::parse_w2r(int n,model_type *data)
{
	int i;
    char *p;
    std::string name;
	model_import_params_t par;

	for(i=0;i<n;i++)
	{
		if(get_line())
		{
            if( (p=strstr(line,"\""))!=NULL )
			{
                if(1!=sscanf(++p,"%[^\"]s",temp))
				{
					return 0;
				}
				name=temp;
                p+=strlen(temp);
				p++;
                if( (p=strstr(p,"\""))!=NULL )
				{
                    if(1!=sscanf(++p,"%[^\"]s",temp))
					{
						return 0;
					}
					par[name]=temp;
				}
			}
		}
	}
    if(par.size()==(size_t)n)
	{
		parse_param(&par);
	}
	return 1;
}


int mqoImport::parse_material(int n,material_list *mate)
{
	int i;
    char *p;
	material_type m;
	double dif,amb,emi,spc;

	for(i=0;i<n;i++)
	{
		if(get_line())
		{
//"1" shader(3) col(1.000 1.000 1.000 1.000) dif(0.498) amb(1.000) emi(0.000) spc(0.000) power(5.00) tex("C:\Users\hideki\Desktop\Sofa-37\HST1-2.jpg")		
			dif=amb=emi=spc=0.0;
			reset_material(&m);
            if( (p=strstr(line,"\""))!=NULL )
			{
                if(1!=sscanf(++p,"%[^\"]s",temp))
				{
					return 0;
				}
				m.name=temp;
			}
            if( (p=strstr(line,"col("))!=NULL )
			{
                if(4!=sscanf(p,"col(%lf %lf %lf %lf",&m.col[0],&m.col[1],&m.col[2],&m.col[3]))
				{
					return 0;
				}
			}
            if( (p=strstr(line,"dif("))!=NULL )
			{
                if(1!=sscanf(p,"dif(%lf",&dif))
				{
					return 0;
				}
			}
            if( (p=strstr(line,"amb("))!=NULL )
			{
                if(1!=sscanf(p,"amb(%lf",&amb))
				{
					return 0;
				}
			}
            if( (p=strstr(line,"emi("))!=NULL )
			{
                if(1!=sscanf(p,"emi(%lf",&emi))
				{
					return 0;
				}
			}
            if( (p=strstr(line,"spc("))!=NULL )
			{
                if(1!=sscanf(p,"spc(%lf",&spc))
				{
					return 0;
				}
			}
            if( (p=strstr(line,"power("))!=NULL )
			{
                if(1!=sscanf(p,"power(%lf",&m.pwr))
				{
					return 0;
				}
			}
            if( (p=strstr(line,"tex("))!=NULL )
			{
                if(1!=sscanf(p,"tex(\"%[^\"]s",temp))
				{
					return 0;
				}
				m.tex=temp;
			}
			expand_material(dif,m.col,m.dif);
			expand_material(amb,m.col,m.amb);
			expand_material(emi,m.col,m.emi);
			expand_material(spc,m.col,m.spc);
			mate->push_back(m);
		}
	}
	return 1;
}

int mqoImport::parse_vertex(int n,vertex_list *vertex)
{
	int i,k;
	double x,y,z;
	vertex_type v;
	k=0;
	for(i=0;i<n;i++)
	{
		if(!get_line()) return 0;
        if(3!=sscanf(line,"%lf %lf %lf",&x,&y,&z))
		{
			return 0;
		}
		//mqo to opengl
        //v.x=z;
        //v.y=x;
        //v.z=y;
		v.x=vertex_mat[0]*x+vertex_mat[1]*y+vertex_mat[2]*z+vertex_mat[3];
		v.y=vertex_mat[4]*x+vertex_mat[5]*y+vertex_mat[6]*z+vertex_mat[7];
		v.z=vertex_mat[8]*x+vertex_mat[9]*y+vertex_mat[10]*z+vertex_mat[11];
		vertex->push_back(v);
		k++;
	}
	if(get_line())
	{
        if( strstr(line,"}")!=NULL )
		{
			return n==k;
		}
	}
	return 0;
}

static void swap(double &x,double &y)
{
	double t;
	t=x;
	x=y;
	y=t;
}
static void swap(int &x,int &y)
{
	int t;
	t=x;
	x=y;
	y=t;
}

static void reverse_face_mqo_to_opengl(face_type *x)
{
	if(x->n==3)
	{
		swap(x->v[0],x->v[2]);
		swap(x->uv[0],x->uv[4]);
		swap(x->uv[1],x->uv[5]);
	}
	else if(x->n==4)
	{
		swap(x->v[0],x->v[3]);
		swap(x->v[1],x->v[2]);
		swap(x->uv[0],x->uv[6]);
		swap(x->uv[1],x->uv[7]);
		swap(x->uv[2],x->uv[4]);
		swap(x->uv[3],x->uv[5]);
	}
}

int mqoImport::parse_face(int n,face_list *face)
{
	int i,k,vn;
    char *p;
	face_type f;

	k=0;
	for(i=0;i<n;i++)
	{
		if(!get_line()) return 0;
		reset_face(&f);
        if(1==sscanf(line,"%d",&vn))
		{
			if(vn==3 || vn==4)
			{
				f.n=vn;
			
                if( (p=strstr(line,"V("))!=NULL )
				{
                    if(vn!=sscanf(p,"V(%d %d %d %d",&f.v[0],&f.v[1],&f.v[2],&f.v[3]))
					{
						return 0;
					}
				}
                if( (p=strstr(line,"M("))!=NULL )
				{
                    if(1!=sscanf(p,"M(%d",&f.m))
					{
						return 0;
					}
				}
                if( (p=strstr(line,"UV("))!=NULL )
				{
                    if((vn*2)!=sscanf(p,"UV(%lf %lf %lf %lf %lf %lf %lf %lf",&f.uv[0],&f.uv[1],&f.uv[2],&f.uv[3],&f.uv[4],&f.uv[5],&f.uv[6],&f.uv[7]))
					{
						return 0;
					}
					f.uv[1]=1.0-f.uv[1];
					f.uv[3]=1.0-f.uv[3];
					f.uv[5]=1.0-f.uv[5];
					f.uv[7]=1.0-f.uv[7];
				}
                if( (p=strstr(line,"COL("))!=NULL )
				{
                    if(1!=sscanf(p,"COL(%lu",&f.col))
					{
						return 0;
					}
				}
				reverse_face_mqo_to_opengl(&f);
				face->push_back(f);
				k++;
			}
			else
			{
				return 0;
			}
		}
	}
	if(get_line())
	{
        if( strstr(line,"}")!=NULL )
		{
			return n==k;
		}
	}
	return 0;
}

static void mirror_vertex(vertex_type &v,int axis)
{
	v.x=((axis & 1) != 0) ? -v.x : v.x;
	v.y=((axis & 2) != 0) ? -v.y : v.y;
	v.z=((axis & 4) != 0) ? -v.z : v.z;
}

static void conv_mirror(object_type &o,int mirror_axis)
{	
	for(vertex_list::iterator i=o.v.begin();i!=o.v.end();i++)
	{
		mirror_vertex( (*i),mirror_axis );
	}

	for(face_list::iterator i=o.f.begin();i!=o.f.end();i++)
	{
		reverse_face_mqo_to_opengl( &(*i) );
	}
    o.name+="(mirror)";
}

int mqoImport::parse_object(char *name, object_list *obj)
{
	int n;
	int mirror,mirror_axis;
    char *p;
	object_type o;

	reset_object(&o);
	o.name=name;
	mirror=0;
	mirror_axis=0;
	for(;;)
	{
		if(!get_line()) return 0;
        if( (p=strstr(line,"visible "))!=NULL )
		{
            if(1!=sscanf(p,"visible %d",&o.visible))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"shading "))!=NULL )
		{
            if(1!=sscanf(p,"shading %d",&o.shading))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"mirror "))!=NULL )
		{
            if(1!=sscanf(p,"mirror %d",&mirror))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"mirror_axis "))!=NULL )
		{
            if(1!=sscanf(p,"mirror_axis %d",&mirror_axis))
			{
				return 0;
			}
		}

        if( (p=strstr(line,"scale "))!=NULL )
		{
            if(3!=sscanf(p,"scale %lf %lf %lf",&o.scale[0],&o.scale[1],&o.scale[2]))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"rotation "))!=NULL )
		{
            if(3!=sscanf(p,"rotation %lf %lf %lf",&o.rot[0],&o.rot[1],&o.rot[2]))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"translation "))!=NULL )
		{
            if(3!=sscanf(p,"translation %lf %lf %lf",&o.trans[0],&o.trans[1],&o.trans[2]))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"facet "))!=NULL )
		{
            if(1!=sscanf(p,"facet %lf",&o.facet))
			{
				return 0;
			}
		}
        if( (p=strstr(line,"vertex "))!=NULL )
		{
            if(1==sscanf(p,"vertex %d",&n))
			{
				if(parse_vertex(n,&o.v)) continue;
				return 0;
			}
		}
        if( (p=strstr(line,"face "))!=NULL )
		{
            if(1==sscanf(p,"face %d",&n))
			{
				if(parse_face(n,&o.f)) continue;
				return 0;
			}
		}
        if( strstr(line,"}")!=NULL )
		{
			if(o.visible || (!o.visible && !discard_hidden))
			{
				obj->push_back(o);
				if(mirror/*==1*/ && mirror_axis)
				{
					conv_mirror(o,mirror_axis);
					obj->push_back(o);
				}
			}
			return 1;
		}
	}
	return 0;
}

void mqoImport::parse_param(model_import_params_t *para)
{
	discard_hidden=0;
	memset(vertex_mat,0,sizeof(double)*16);
	vertex_mat[0]=vertex_mat[5]=vertex_mat[10]=vertex_mat[15]=1.0;
	if(para!=NULL)
	{
		model_import_params_t::iterator i;
        i=para->find("discard_hidden");
        if(i!=para->end()) discard_hidden=atoi(i->second.c_str());

        i=para->find("vertex_mat");
		if(i!=para->end())
		{
			double x[16];
            int r=sscanf(i->second.c_str(),"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&x[0],&x[1],&x[2],&x[3],
				&x[4],&x[5],&x[6],&x[7],
				&x[8],&x[9],&x[10],&x[11],
				&x[12],&x[13],&x[14],&x[15]);
			if(r==9)
			{
				vertex_mat[0]=x[0];	vertex_mat[1]=x[1];	vertex_mat[2]=x[2];
				vertex_mat[4]=x[3];	vertex_mat[5]=x[4];	vertex_mat[6]=x[5];
				vertex_mat[8]=x[6];	vertex_mat[9]=x[7];	vertex_mat[10]=x[8];
			}
			else if(r==16)
			{
				for(int i=0;i<16;i++) vertex_mat[i]=x[i];
			}
		}
        int an=para->size()+8;
        for(int a=0;a<an;a++)
        {
            char name[32];
            sprintf(name,"axis%d",a);
            i=para->find(name);
            if(i!=para->end())
            {
                double x[3];
                int r=sscanf(i->second.c_str(),"%lf %lf %lf",&x[0],&x[1],&x[2]);
                if(r==3)
                {
                    joint_type j;
                    j.obj_id=a;
                    j.mode=JOINT_ROTATE;
                    j.axis[0]=x[0];
                    j.axis[1]=x[1];
                    j.axis[2]=x[2];
                    j.offset[0]=0.0;
                    j.offset[1]=0.0;
                    j.offset[2]=0.0;
                    joi[a]=j;
                }
            }
        }
	}
}

int mqoImport::load(const char *fname,model_type *data,model_import_params_t *para)
{
	int ret=0;
	reset_model(data);
	parse_param(para);
    fp=fopen(fname,"rt");
    if(fp!=NULL)
	{
		int n;
        char *p;
		ret=1;
		while(get_line())
		{
            if(1==sscanf(line,"Material %d",&n))
			{
				if(parse_material(n,&data->mate)) continue;
				ret=0;
				break;
			}
            else if( (p=strstr(line,"Object "))==line )
			{
                if(1==sscanf(line,"Object \"%[^\"]s",temp))
				{
					if(parse_object(temp,&data->obj)) continue;
					ret=0;
					break;
				}
			}
            else if(1==sscanf(line,"W2R %d",&n))
			{
                if(parse_w2r(n,data)) continue;
				ret=0;
				break;
			}
		}
		fclose(fp);
	}
	if(ret)
	{
        data->joi=joi;
		data->path=fname;
	}
	return ret;
}

int import_mqo(const char *fname,model_type *m,model_import_params_t *para)
{
    mqoImport x;
	return x.load(fname,m,para);
}
