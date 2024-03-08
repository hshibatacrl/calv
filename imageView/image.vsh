#version 120

attribute vec3 position;
attribute vec2 texture_uv;
varying vec2 t;
uniform mat4 mvp_matrix;

void main()
{
    t = texture_uv;
    gl_Position = mvp_matrix * vec4(position,1.0);
}
