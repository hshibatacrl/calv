#version 120
uniform sampler2D tex1; // Frame buffer 1 (primary image)
uniform sampler2D tex2; // Frame buffer 2 (overlay image)
varying vec2 t;         //texture coordinates
uniform int useOverlay;

void main(void)
{
    if(useOverlay==1)
    {
       vec4 b=texture2D(tex1,t);
       vec4 o=texture2D(tex2,t);
       float g=(o.r+o.g+o.b)/3.0;

       if(g>0.0)
       {
           g=clamp(3.0*g,0.0,1.0);
           gl_FragColor.rgb = g * o.rgb + (1.0-g) * b.rgb;
       }
       else
       {
           gl_FragColor.rgb = b.rgb;
       }
//       gl_FragColor.rgb= vec3(clamp(o.r+b.r,0.0,1.0),clamp(o.g+b.g,0.0,1.0),clamp(o.b+b.b,0.0,1.0));
       gl_FragColor.a=1.0;
    }
    else
    {
        gl_FragColor =texture2D(tex1,t);
    }
}

