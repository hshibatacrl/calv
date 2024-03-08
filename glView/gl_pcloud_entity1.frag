#version 120

// Copyright 2021 Wagon Wheel Robotics
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

varying highp vec3 vert;
varying lowp float col_z;
varying lowp float col_a;
varying lowp float filtered;
uniform highp int mode;
uniform highp int antiAlias;


const lowp vec3 c1= vec3(0.0, 1.0, 0.0);
const lowp vec3 c2= vec3(1.0, 0.0, 0.0);
const lowp vec3 c3= vec3(0.0, 0.0, 0.0); //black
const lowp vec3 c4= vec3(1.0, 1.0, 1.0); //white
const lowp float s=1.0;

void main()
{
    lowp vec3 col;

    if(filtered>0.5) discard;

    if(antiAlias==1)
    {
        vec3 n;
        n.xy = gl_PointCoord * 2.0 - 1.0;
        n.z = 1.0 - dot(n.xy, n.xy);
        if (n.z < 0.0) discard;
    }

    if(mode==4)
    {
        highp int depth,r,g,b;
        depth=int(gl_FragCoord.z*16777216.0);   // depth=int(gl_FragCoord.z*4294967295.0);
        r=depth/16777216; depth=depth -r*16777216;
        g=depth/65536;    depth=depth -g*65536;
        b=depth/256;      depth=depth -b*256;
        gl_FragColor = vec4(float(depth),float(b),float(g),float(r))/255.0;
    }
    else if(mode==3)
    {
        col = clamp(mix(c3, c4, col_z) , 0.0, 1.0);
        gl_FragColor = vec4(col, 1.0);
    }
    else
    {
        highp float f,h,c;
        highp int i;
        c=clamp(col_z, 0.0,1.0);
        h=4.0-c*4.0;

        i=int(h);
        f=h-float(i);
        if(i==0)     {col=vec3(1.0,          1.0-s*(1.0-f),1.0-s         );}
        else if(i==1){col=vec3(1.0-s*f,      1.0,          1.0-s         );}
        else if(i==2){col=vec3(1.0-s,        1.0,          1.0-s*(1.0-f) );}
        else if(i==3){col=vec3(1.0-s,        1.0-s*f,      1.0           );}
        else if(i==4){col=vec3(1.0-s*(1.0-f),1.0-s,        1.0           );}
        else         {col=vec3(1.0,          1.0-s,        1.0-s*f       );}
        // else{col = clamp(mix(c1, c2, col_z) , 0.0, 1.0);

        gl_FragColor = vec4(col*col_a, 1.0);

//        gl_FragColor = vec4(col*col_a, 1.0);
    }
};


