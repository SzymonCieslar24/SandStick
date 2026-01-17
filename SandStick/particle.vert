#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 offset; // Pozycja startowa (patyka), jeœli chcemy przesuwaæ ca³y system, ale tutaj bêdziemy aktualizowaæ CPU

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = 4.0; // Wielkoœæ ziarenka piasku
}