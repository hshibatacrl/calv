#ifndef GL_DRAW_PARAMS_H
#define GL_DRAW_PARAMS_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QMatrix4x4>
#include <QVector3D>

typedef struct
{
    int color_mode;         // 0: height, 1: range, 2: amp, 3:amp-BW, 4:depth(for picking) 6:texture(DEM)
    int flags;
    float psz;
    float amp[2];
    float rng[2];
    float hgt[2];
    float flt_amp[2];
    float flt_rng[2];
    float flt_hgt[2];
    bool ignoreDraft;
} opt_pointcloud_t;

#define OPT_PC_CM_HEIGHT 0
#define OPT_PC_CM_RANGE 1
#define OPT_PC_CM_AMP 2
#define OPT_PC_CM_AMP_BW 3
#define OPT_PC_CM_DEPTH 4
#define OPT_PC_CM_HEIGHTxAMP 5
#define OPT_PC_CM_TEXTURE 6
#define OPT_PC_CM_INDEX 7

typedef struct
{
    int width,height;
    int viewport[4];
    QMatrix4x4 proj;    // projection matrix
    QMatrix4x4 world;   // world matrix
    QMatrix4x4 camera;  // camera matrix
    QVector3D lightDir; // light Direction
    QVector3D eyeDir;   // eye Direction
    int mode;           // GL_DRAW_NORMAL, GL_DRAW_PICK, GL_DRAW_TEMP
    double aspectRatio;
    int pointAntiAlias;
    int eyeDomeLighting;
    bool draftOnly;
    opt_pointcloud_t opt_pc;
} gl_draw_ctx_t;

#define GL_DRAW_NORMAL  1
#define GL_DRAW_PICK  2
#define GL_DRAW_TEMP  3

#endif // GL_DRAW_PARAMS_H
