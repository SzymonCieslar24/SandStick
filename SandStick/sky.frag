#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform vec3 topColor;
uniform vec3 bottomColor;

// Funkcja losuj¹ca (Triangular Noise) - lepsza ni¿ zwyk³y noise
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 dir = normalize(TexCoords);
    float factor = clamp(dir.y, 0.0, 1.0);
    
    // Nieliniowy gradient (³adniejsze przejœcie przy horyzoncie)
    factor = pow(factor, 0.6); 
    
    vec3 finalColor = mix(bottomColor, topColor, factor);
    
    // --- DELIKATNY DITHERING (Usuwa paski) ---
    // Zmniejszona si³a szumu z 0.01 na 0.005 (jest prawie niewidoczny)
    float noise = rand(gl_FragCoord.xy) * 0.005; 
    finalColor += noise;

    // Kaganiec na Bloom (¿eby niebo nie œwieci³o)
    finalColor = min(finalColor, vec3(0.99)); 

    FragColor = vec4(finalColor, 1.0);
}