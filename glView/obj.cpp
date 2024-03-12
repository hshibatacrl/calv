/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "model.h"

#if __GNUC__==7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif __GNUC__>7
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <cstdio>
#include <cstring>
#include <string>
#include <map>

typedef std::map<std::string,int> mtl_map;

class objImport
{
#define LINE_BUF_SIZE 1024
    std::string line_;
    std::string temp_;
    char *line;
    char *temp;
    FILE *fp;

	int get_line(void);

    int parse_material(const char *name, material_list *mate, mtl_map *map);
	int parse_face(object_type &o,int cur_mtl,vertex_list &vt);

public:
    objImport();
    ~objImport();
    int load(const char *x,model_type *data);
};


objImport::objImport()
{
    line_.resize(LINE_BUF_SIZE);
    temp_.resize(LINE_BUF_SIZE);
    line = &line_[0];
    temp = &temp_[0];
    fp=NULL;
}

objImport::~objImport()
{
}

int objImport::get_line(void)
{
    return fgets(&line[0],LINE_BUF_SIZE,fp)!=NULL;
}

#if 0
newmtl texture
Ka 0.00000 0.00000 0.00000
Kd 1.00000 1.00000 1.00000
Ks 0.20000 0.20000 0.20000
Ns 20.00000
map_Kd x.bmp
#endif

int objImport::parse_material(const char *fname,material_list *mate,mtl_map *map)
{
	material_type m;
	reset_material(&m);
	map->clear();
	int idx=1;	//start from 1

	double alpha;
    fp=fopen(fname,"rt");
    if(fp!=NULL)
	{
        char *p;
		while(get_line())
		{
            if(1==sscanf(line,"Ns %lf",&m.pwr))/* wavefront shininess is from [0, 1000], so scale for OpenGL */
			{
				m.pwr/=1000.0;
				m.pwr*=128.0;
			}
            else if(1==sscanf(line,"d %lf",&alpha))
			{
				m.amb[3]=alpha;
				m.dif[3]=alpha;
				m.emi[3]=alpha;
				m.spc[3]=alpha;
				m.col[3]=alpha;
			}
            else if(3==sscanf(line,"Ka %lf %lf %lf",&m.amb[0],&m.amb[1],&m.amb[2]))
			{
			}
            else if(3==sscanf(line,"Kd %lf %lf %lf",&m.dif[0],&m.dif[1],&m.dif[2]))
			{
			}
            else if(3==sscanf(line,"Ks %lf %lf %lf",&m.spc[0],&m.spc[1],&m.spc[2]))
			{
			}
            else if( (p=strstr(line,"newmtl "))==line )
			{
                if(1==sscanf(line,"newmtl %s",temp))
				{
                    if(m.name!="")
					{
						mate->push_back(m);
						(*map)[m.name]=idx++;
						reset_material(&m);
					}
					m.name=temp;
				}
			}
            else if( (p=strstr(line,"map_Kd "))==line )
			{
                if(1==sscanf(line,"map_Kd %s",temp))
				{
					m.tex=temp;
				}
			}
		}
        if(m.name!="")
		{
			mate->push_back(m);
			(*map)[m.name]=idx;
		}
		fclose(fp);
	}
	return 1;
}

static int face(char *token,int *v,int *vt,int *vn)
{
	*v=-1;
	*vn=-1;
	*vt=-1;
    if( sscanf(token,"%d/%d/%d",v,vt,vn)==3 ) return 1;
    if( sscanf(token,"%d/%d",v,vt)==2 ) return 1;
    if( sscanf(token,"%d//%d",v,vn)==2 ) return 1;
	return 0;
}

int objImport::parse_face(object_type &o,int cur_mtl,vertex_list &vt)
{
	int ret=0;
	face_type f;
    char *p,*q;
	int t[4],vn[4],n;

	reset_face(&f);
	f.m=cur_mtl;
	n=0;

    p=&line[2];	//skip "f "
	q=p;

    while( (p=strstr(q," "))!=NULL )
	{
		*p=0;
		if( face(q,&f.v[n],&t[n],&vn[n]) )
		{
			f.v[n]--;
			n++;
			if(n>=4) break;
		}
		else
		{
			break;
		}
		q=p+1;
	}

	if(q!=NULL && n<4)
	{
		if( face(q,&f.v[n],&t[n],&vn[n]) )
		{
			f.v[n]--;
			n++;
		}
	}
	if(n==3||n==4)
	{
		int i;
		f.n=n;
		for(i=0;i<n;i++)
		{
			if(t[i]>0)
			{
				t[i]--;
				f.uv[i*2+0]=vt[ t[i] ].x;
				f.uv[i*2+1]=vt[ t[i] ].y;
			}
		}
		o.f.push_back(f);		
	}

	return ret;
}

int objImport::load(const char *fname, model_type *data)
{
	int ret=1;
	vertex_type a;
	vertex_list v,vt,vn;
	object_type o;
	mtl_map map;
	int cur_mat=-1;

	reset_object(&o);
	reset_model(data);

    fp=fopen(fname,"rt");
    if(fp!=NULL)
	{
		int n;
        char *p;
		while(get_line())
		{
            if(line[0]=='v')
			{
                if(line[1]==' ')
				{
                    if(3==sscanf(line,"v %lf %lf %lf",&a.x,&a.y,&a.z))
					{
						v.push_back(a);
					}
				}
                else if(line[1]=='t')
				{
                    if(2==sscanf(line,"vt %lf %lf",&a.x,&a.y))
					{
						a.y=1.0-a.y;
						vt.push_back(a);
					}
				}
                else if(line[1]=='n')
				{
                    if(3==std::sscanf(line,"vn %lf %lf %lf",&a.x,&a.y,&a.z))
					{
						vn.push_back(a);
					}
				}
			}
            else if(line[0]=='g')
			{
                if(1==sscanf(line,"g %s",temp))
				{
                    if(o.name!="") data->obj.push_back(o);
					reset_object(&o);
					o.name=temp;
				}
			}
            else if(line[0]=='f')
			{
				parse_face(o,cur_mat,vt);
			}
            else if( (p=strstr(line,"usemtl "))==line )
			{
                if(1==sscanf(line,"usemtl %s",temp))
				{
                    std::string ma=temp;
					cur_mat=map[ma];
					cur_mat--;	//start from 0
				}
			}
            else if( (p=strstr(line,"mtllib "))==line )
			{
                if(1==sscanf(line,"mtllib %s",temp))
				{
					FILE *fp_save;
                    fs::path ps(fname);
                    std::string matFileName=ps.replace_filename(temp).string();
					fp_save=fp;
                    n=parse_material(matFileName.c_str(),&data->mate,&map);
					fp=fp_save;
					if(n) continue;
					ret=0;
					break;
				}
			}
		}
        if(o.name!="") data->obj.push_back(o);
		fclose(fp);
	}
	if(ret)
	{
		data->vtx=v;
		data->path=fname;
	}
	return ret;


}


int import_obj(const char *fname,model_type *m)
{
    objImport x;
	return x.load(fname,m);
}
