#version 430 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;

out vec4 out_color;

uniform mat4 matrix;

void main()
{
   gl_Position = matrix * vec4(pos.x, pos.y, 0.f, 1.f);
   out_color = color;
};
