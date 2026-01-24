#version 330 core
out vec4 FragColor;

uniform vec3 lightColor; 

void main()
{
    vec3 sandColor = vec3(0.937, 0.894, 0.690);

    vec3 finalRGB = sandColor * lightColor * 1.5;

    FragColor = vec4(finalRGB, 1.0);
}