#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform vec3 topColor;
uniform vec3 bottomColor;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 dir = normalize(TexCoords);
    float factor = clamp(dir.y, 0.0, 1.0);
    
    factor = pow(factor, 0.6); 
    
    vec3 finalColor = mix(bottomColor, topColor, factor);
    
    float noise = rand(gl_FragCoord.xy) * 0.005; 
    finalColor += noise;

    finalColor = min(finalColor, vec3(0.99)); 

    FragColor = vec4(finalColor, 1.0);
}