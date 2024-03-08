#ifndef MODEL_H__
#define MODEL_H__

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdint>
#include <vector>
#include <string>
#include <map>

typedef struct
{
	double col[4];
	double dif[4];
	double amb[4];
	double emi[4];
	double spc[4];
	double pwr;
    std::string name;
    std::string tex;
	unsigned int tex_id;
}material_type;

typedef std::vector<material_type> material_list;

typedef struct
{
	double x,y,z;
}vertex_type;

typedef struct
{
	int n;				//num of vertex(3 or 4)
	int m;				//material index (-1 is no material)
	int v[4];			//index for vertex
	double uv[8];		//uv numbers
	unsigned long col;	//hexcolor
}face_type;


typedef std::vector<vertex_type> vertex_list;
typedef std::vector<face_type> face_list;


typedef struct
{
	int visible;
	int shading;
	double scale[3];
	double rot[3];
	double trans[3];
	double facet;

    std::string name;
	vertex_list v;			//local vertex
	face_list f;

	unsigned int obj_id;
    int mask_id;
    int top_of_group;
}object_type;


typedef std::vector<object_type> object_list;


typedef struct
{
    int obj_id;
    int mode;
    double axis[3];
    double offset[3];
}joint_type;
#define JOINT_ROTATE 1
typedef std::map<int,joint_type> joint_map;


typedef struct
{
	material_list mate;
	object_list obj;
	vertex_list vtx;		//global vertex 
    joint_map joi;
    std::string path;
	int flag;
} model_type;

typedef struct
{
	double pos[3];		//position (local)
	double rpy[3];		//attitude [deg]
	double scale[3];	//scale
	int visible;
} model_prop_type;

void reset_model_prop(model_prop_type *p);
void reset_model(model_type *m);
void reset_face(face_type *f);
void reset_material(material_type *m);
void reset_object(object_type *o);

typedef std::map<std::string,std::string> model_import_params_t;
typedef std::pair<std::string,std::string> model_import_param_t;

int model_import(model_type *model,const std::string &fname,model_import_params_t *para=NULL);

int import_mqo(const char *fname,model_type *m,model_import_params_t *para);
int import_obj(const char *fname,model_type *m);
int import_w2r(const char *fname,model_type *data);

int export_w2r(const char *fname,model_type *data);

void model_scaling(model_type *model,double scale);
void model_offset(model_type *model,const double x,const double y,const double z);

int model_draw_objects(model_type *model,model_prop_type *prop=NULL,int mask_bits=0xffffffff);
void model_terminate(model_type *m);

object_type *model_get_object(model_type *model, const std::string &name);
int model_object_get_cg(object_type *obj,double *ret);

int model_object_get_cg(model_type *model,int mask_bits,double *ret);

int model_update_mask_prefix(model_type *model, const std::string &prefix, int mask_bits);

#endif // MODEL_H__

