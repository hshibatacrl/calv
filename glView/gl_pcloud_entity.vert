#version 120

// Copyright 2021 Wagon Wheel Robotics
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


attribute vec3 vertex;
attribute vec3 rgb;
attribute float amp;
attribute float range;
attribute float flags;
varying highp vec3 vert;
varying lowp float filtered;
varying lowp float col_z;
varying lowp float col_a;
uniform mat4 mvpMatrix;
uniform highp vec3 z_range;
uniform highp vec3 a_range;
uniform highp vec3 r_range;
uniform lowp vec2 fltA;
uniform lowp vec2 fltR;
uniform lowp vec2 fltZ;
uniform highp int fltAEnable;
uniform highp int fltREnable;
uniform highp int fltZEnable;
uniform highp int mode;
uniform highp float pointsize;

void main()
{
   vert = vertex;
   filtered=0.0;
   if(mod(flags,2)>0.5)
   {   //polygon filter
       filtered=1.0;       //just check bit0
   }
   else
   {
       if(fltAEnable==1)
       {
           if(amp<fltA.x || amp>fltA.y)
           {
               filtered=1.0;
           }
       }
       if(fltREnable==1)
       {
           if(range<fltR.x || range>fltR.y)
           {
               filtered=1.0;
           }
       }
       if(fltZEnable==1)
       {
           if(vertex.z<fltZ.x || vertex.z>fltZ.y)
           {
               filtered=1.0;
           }
       }
   }
   if(mode == 0)
   {
       col_z = (vertex.z-z_range.x)/z_range.z;
       col_a = 1.0;
   }
   else if(mode == 5)
   {
       col_z = (vertex.z-z_range.x)/z_range.z;
       col_a = (amp-a_range.x)/a_range.z;
   }
   else if(mode==1)
   {
       col_z = (range-r_range.x)/r_range.z;
       col_a = 1.0;
   }
   else
   {
       col_z = (amp-a_range.x)/a_range.z; col_a = 1.0;
   }
   gl_Position = mvpMatrix * vec4(vertex, 1.0);
   gl_PointSize = pointsize;
}
