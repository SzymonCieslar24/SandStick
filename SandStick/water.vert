#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 FragPos; // Wyjœcie do Fragment Shadera

void main()
{
    vec3 pos = aPos;
    
    // Animacja fal (prosta sinusoida)
    float wave = sin(pos.x * 0.1 + time * 1.0) * 0.5 + 
                 cos(pos.z * 0.05 + time * 0.8) * 0.5;
    pos.y += wave;

    // Obliczamy pozycjê w œwiecie (World Space)
    vec4 worldPos = model * vec4(pos, 1.0);
    
    // Wysy³amy do fragment shadera
    FragPos = vec3(worldPos);

    gl_Position = projection * view * worldPos;
}