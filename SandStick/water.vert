#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Nie u¿ywamy, ale musi pasowaæ do formatu Vertex

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time; // Czas do animacji fal

void main()
{
    vec3 pos = aPos;
    
    // Prosta symulacja fal: Y zale¿y od sinusa czasu i pozycji X/Z
    // Fala g³ówna + mniejsza fala zak³ócaj¹ca
    float wave = sin(pos.x * 0.2 + time * 1.5) * 0.2 + 
                 cos(pos.z * 0.3 + time * 1.0) * 0.2;
    
    pos.y += wave;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}