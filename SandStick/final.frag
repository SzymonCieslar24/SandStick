#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D scene;
uniform sampler2D bloomBlur;

void main() {             
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // Dodajemy poœwiatê
    vec3 result = hdrColor*0.5 + bloomColor; 
    
    // Tone Mapping (Reinhard) - ¿eby nie by³o "bia³ej plamy"
    vec3 mapped = result / (result + vec3(1.0));
    
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2));
    
    FragColor = vec4(mapped, 1.0);
}