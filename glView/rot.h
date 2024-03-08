#ifndef ROT_H
#define ROT_H

/*
Copyright 2021 Wagon Wheel Robotics

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <QVector3D>
#include <QVector4D>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <cmath>

namespace rot
{

const float d2r = (float)(M_PI/180.0);
const float r2d = (float)(180.0/M_PI);

void euler_to_quat(QVector4D &quat, const QVector3D &euler);

void dcm_from_quat(QMatrix3x3 &dcm, const QVector4D &quat);

void quat_skew(QMatrix4x4 &m, const QVector3D &rate);

void quat_from_axis_angle(QVector4D &quat, const QVector3D &axis, const float angle);

void dcm_to_quat(QVector4D &quat, const QMatrix3x3 &dcm);

void rot(float wp,float wq,float wr, QVector4D &quat);

float normalize180(float x);

void dcm_from_rodrigues(QMatrix3x3 &R, QVector3D &r);

int dcm_to_euler(QVector3D euler, const QMatrix3x3 dcm);

void dcm4x4(QMatrix4x4 &R, const QMatrix3x3 & dcm);

}

#endif // ROT_H
