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

#include "rot.h"

#include <cfloat>

namespace rot
{

void euler_to_quat(QVector4D &quat, const QVector3D &euler)
{
    float r, p, y, sr, cr, sp, cp, sy, cy;

    r = euler[0] * 0.5f;
    p = euler[1] * 0.5f;
    y = euler[2] * 0.5f;

    sr = std::sin(r); cr = std::cos(r);
    sp = std::sin(p); cp = std::cos(p);
    sy = std::sin(y); cy = std::cos(y);

    quat[0] = cr * cp * cy + sr * sp * sy;
    quat[1] = -cr * sp * sy + sr * cp * cy;
    quat[2] = cr * sp * cy + sr * cp * sy;
    quat[3] = cr * cp * sy - sr * sp * cy;
}

void dcm_from_quat(QMatrix3x3 &dcm, const QVector4D &quat)
{
#define q0 quat[0]
#define q1 quat[1]
#define q2 quat[2]
#define q3 quat[3]
    dcm(0,0) = q0*q0 + q1*q1 - q2*q2 - q3*q3;
    dcm(0,1) = 2.0f * (q1 * q2 - q0 * q3);
    dcm(0,2) = 2.0f * (q1 * q3 + q0 * q2);
    dcm(1,0) = 2.0f * (q1 * q2 + q0 * q3);
    dcm(1,1) = q0*q0 - q1*q1 + q2*q2 - q3*q3;
    dcm(1,2) = 2.0f * (q2 * q3 - q0 * q1);
    dcm(2,0) = 2.0f * (q1 * q3 - q0 * q2);
    dcm(2,1) = 2.0f * (q2 * q3 + q0 * q1);
    dcm(2,2) = q0*q0 - q1*q1 - q2*q2 + q3*q3;
#undef q0
#undef q1
#undef q2
#undef q3
}

void quat_skew(QMatrix4x4 &m, const QVector3D &rate)
{
    float wp, wq, wr;
    wp = 0.5f * rate.x();
    wq = 0.5f * rate.y();
    wr = 0.5f * rate.z();

    m(0,0) = 0.0;	m(0,1) = -wp;	m(0,2) = -wq;	m(0,3) = -wr;
    m(1,0) = wp;	m(1,1) = 0.0;	m(1,2) = -wr;	m(1,3) = wq;
    m(2,0) = wq;	m(2,1) = wr;	m(2,2) = 0.0;	m(2,3) = -wp;
    m(3,0) = wr;	m(3,1) = -wq;	m(3,2) = wp;	m(3,3) = 0.0;
}

void quat_from_axis_angle(QVector4D &quat, const QVector3D &axis, const float angle)
{
    float c, s, a;
    a = 0.5f * angle;
    c = std::cos(a);
    s = std::sin(a);
    quat[0] = c;
    quat[1] = axis[0] * s;
    quat[2] = axis[1] * s;
    quat[3] = axis[2] * s;
}

void dcm_to_quat(QVector4D &quat, const QMatrix3x3 &dcm)
{
    float tr=dcm(0,0)+dcm(1,1)+dcm(2,2);

    if (tr > 0.0f)
    {
        double p0 = std::sqrt(1.0f + tr);
        quat[0] = p0 * 0.5f;
        quat[1] = 0.5f * (dcm(2,1) - dcm(1,2)) / p0;		//C(2, 1) - C(1, 2)
        quat[2] = 0.5f * (dcm(0,2) - dcm(2,0)) / p0;		//C(0, 2) - C(2, 0)
        quat[3] = 0.5f * (dcm(1,0) - dcm(0,1)) / p0;		//C(1, 0) - C(0, 1)
    }
    else
    {
        if (dcm(0,0) > dcm(1,1) && dcm(0,0) > dcm(2,2))
        {
            float s = 2.0f * std::sqrt(1.0f + dcm(0,0) - dcm(1,1) - dcm(2,2));
            quat[0] = (dcm(2,1) - dcm(1,2)) / s;		//C(2, 1) - C(1, 2)
            quat[1] = s * 0.25f;
            quat[2] = (dcm(1,0) + dcm(0,1)) / s;		//C(1, 0) + C(0, 1)
            quat[3] = (dcm(0,2) + dcm(2,0)) / s;		//C(0, 2) + C(2, 0)
        }
        else if (dcm(1,1) > dcm(2,2))
        {
            float s = 2.0f * sqrt(1.0f - dcm(0,0) + dcm(1,1) - dcm(2,2));
            quat[0] = (dcm(0,2) - dcm(2,0)) / s;		//C(0, 2) - C(2, 0)
            quat[1] = (dcm(1,0) + dcm(0,1)) / s;		//C(1, 0) + C(0, 1)
            quat[2] = s * 0.25f;
            quat[3] = (dcm(2,1) + dcm(1,2)) / s;		//C(2, 1) + C(1, 2)
        }
        else
        {
            float s = 2.0f * std::sqrt(1.0f - dcm(0,0) - dcm(1,1) + dcm(2,2));
            quat[0] = (dcm(1,0) - dcm(0,1)) / s;		//C(1, 0) - C(0, 1)
            quat[1] = (dcm(0,2) + dcm(2,0)) / s;		//C(0, 2) + C(2, 0)
            quat[2] = (dcm(2,1) + dcm(1,2)) / s;		//C(2, 1) + C(1, 2)
            quat[3] = s * 0.25f;
        }
    }
}

/*
 * rotate camera from angler velocity on camera frame
 * update dcm via quaternion
 */
void rot(float wp,float wq,float wr, QVector4D &quat)
{
    QMatrix4x4 rot;
    QMatrix3x3 dcm;
    QVector3D wc(wp*d2r, wq*d2r, wr*d2r);    //rotation vector in body(camra) frame
    QVector3D wg;                            //wg is rotation vector in nav(global) frame

    dcm_from_quat( dcm,quat );


    wg[0] = dcm(0,0) * wc[0] + dcm(0,1) * wc[1] + dcm(0,2) * wc[2];
    wg[1] = dcm(1,0) * wc[0] + dcm(1,1) * wc[1] + dcm(1,2) * wc[2];
    wg[2] = dcm(2,0) * wc[0] + dcm(2,1) * wc[1] + dcm(2,2) * wc[2];

    quat_skew( rot, wg );

    quat += rot * quat;

    quat.normalize();
}

float normalize180(float x)
{
    if(x>180.0f) x-=360.0f;
    else if(x<-180.0f) x+=360.0f;
    return x;
}


void dcm_from_rodrigues(QMatrix3x3 &R, QVector3D &r)
{
    float theta = r.length();

    if( theta < FLT_EPSILON )
    {
        R.setToIdentity();
    }
    else
    {
        float c = std::cos(theta);
        float s = std::sin(theta);
        float c1 = 1.0f - c;
        float itheta = theta ? 1.0f/theta : 0.0f;

        r *= itheta;

        QMatrix3x3 rrt, r_x, I;
        rrt(0,0) = r.x() * r.x(); rrt(0,1) = r.x() * r.y(); rrt(0,2) = r.x() * r.z();
        rrt(1,0) = r.x() * r.y(); rrt(1,1) = r.y() * r.y(); rrt(1,2) = r.y() * r.z();
        rrt(2,0) = r.x() * r.z(); rrt(2,1) = r.y() * r.z(); rrt(2,2) = r.z() * r.z();

        r_x(0,0) =   0.0f; r_x(0,1) = -r.z(); r_x(0,2) =  r.y();
        r_x(1,0) =  r.z(); r_x(1,1) =   0.0f; r_x(1,2) = -r.x();
        r_x(2,0) = -r.y(); r_x(2,1) =  r.x(); r_x(2,2) =   0.0f;

        I.setToIdentity();

        // R = cos(theta)*I + (1 - cos(theta))*r*rT + sin(theta)*[r_x]
        R = c*I + c1*rrt + s*r_x;
    }
}

// SQ(sin(89.9)) to detect gimbal lock state
#define SQ_SIN89_9 (float)0.99999695382889519073899096539312

int dcm_to_euler(QVector3D euler, const QMatrix3x3 dcm)
{
    double sq_sp;

    sq_sp = dcm(2,0) * dcm(2,0);
    if (sq_sp > SQ_SIN89_9) return 0;	// How about sending me a fourth gimbal for Christmas?

    euler[1] = std::atan2(-dcm(2,0), std::sqrt(1.0f - sq_sp));
    euler[0] = std::atan2(dcm(2,1), dcm(2,2));
    euler[2] = std::atan2(dcm(1,0), dcm(0,0));

    return 1;
}

void dcm4x4(QMatrix4x4 &R, const QMatrix3x3 & dcm)
{
    R.setToIdentity();
    for(int row=0;row<3;row++)
        for(int col=0;col<3;col++)
            R(row,col) = dcm(row,col);
}

} // namespace rot
