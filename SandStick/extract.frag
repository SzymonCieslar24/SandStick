#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D scene;

void main() {
    vec3 color = texture(scene, TexCoords).rgb;
    
    // Oblicz jasnoœæ
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // Zmieñ próg na bardzo niski (np. 0.8 lub 0.5)
    // Dziêki temu s³oñce (nawet s³abe) na pewno zostanie rozmyte.
    // T³o nieba ma zazwyczaj jasnoœæ ok 0.2-0.5, wiêc nie powinno zacz¹æ œwieciæ.
    if(brightness > 0.8)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}