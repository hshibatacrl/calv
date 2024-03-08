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

#include "colorizer.h"

#include <cmath>

#include <QDebug>
#include <QVariant>

colorizer::colorizer(const QVariantMap &params)
{
    _col[0] = params["minColor"].value<QColor>();
    _col[1] = params["midColor"].value<QColor>();
    _col[2] = params["maxColor"].value<QColor>();
    _nos = params["numOfSteps"].toInt();
    _delta_a = 2.0 / (double)(_nos);
    _delta_c = 2.0 / (double)(_nos-1);

    _v[0] = params["colMin"] .toDouble();
    _v[1] = params["colMid"] .toDouble();
    _v[2] = params["colMax"] .toDouble();

    _x2a[0] = new interp1d<double>(_v[0],_v[1],0.0,1.0);
    _x2a[1] = new interp1d<double>(_v[1],_v[2],1.0,2.0);
}

colorizer::~colorizer()
{
    delete _x2a[0];
    delete _x2a[1];
}

static QColor interpColor(const QColor &c0, const QColor &c1, double a)
{
    double r,g,b;
    r=c0.red()   * (1.0-a) + c1.red()   * a;
    g=c0.green() * (1.0-a) + c1.green() * a;
    b=c0.blue()  * (1.0-a) + c1.blue()  * a;
    return QColor(r,g,b);
}

QColor colorizer::solve(double x)
{
    if(x<_v[0]) x=_v[0];
    else if(_v[2]<x) x=_v[2];

    double a=_x2a[ (x<_v[1]) ? 0:1 ]->interp(x);
    double a_prime=std::floor(a/_delta_a)*_delta_c;
    if(a_prime>2.0) a_prime=2.0;
    //qDebug("x=%.2lf, a=%.2lf, a'=%.2lf",x,a,a_prime);
    if(a_prime<1.0)
    {   //0<=a<1: cmin-cmid
        return interpColor(_col[0],_col[1], a_prime);
    }
    //1<=a<=2 cmid-cmax
    return interpColor(_col[1],_col[2], a_prime-1.0);
}
