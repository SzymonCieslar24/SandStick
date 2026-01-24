#version 330 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec3 pos = aPos; 

    float freq = 0.2;
    float amp = 0.5;
    float speed = 1.5;
    
    float angle = pos.x * freq + time * speed + pos.z * 0.1;

    pos.y = sin(angle) * amp;

    float dYdX = freq * amp * cos(angle);
    float dYdZ = 0.1 * amp * cos(angle);

    vec3 tangentX = vec3(1.0, dYdX, 0.0);
    vec3 tangentZ = vec3(0.0, dYdZ, 1.0);

    vec3 localNormal = normalize(cross(tangentZ, tangentX));

    Normal = mat3(transpose(inverse(model))) * localNormal;

    FragPos = vec3(model * vec4(pos, 1.0));
    gl_Position = projection * view * model * vec4(pos, 1.0);
}