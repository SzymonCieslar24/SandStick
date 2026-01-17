#version 330 core
out vec4 FragColor;

// Odbieramy kolor œwiat³a z C++ (zmienny w czasie dnia)
uniform vec3 lightColor; 

void main()
{
    // Bazowy kolor piasku (#EFE4B0)
    vec3 sandColor = vec3(0.937, 0.894, 0.690);

    // Mno¿ymy kolor piasku przez kolor œwiat³a.
    // Dziêki temu:
    // - W dzieñ: kolor pozostaje jasny (bo lightColor jest prawie bia³y)
    // - O zachodzie: robi siê pomarañczowy
    // - W nocy: robi siê ciemny/szary
    
    // Dodajemy "max(..., 0.3)", ¿eby ziarenka lekko "œwieci³y" w nocy 
    // i by³y widoczne na tle ciemnego nieba (opcjonalne, ale wygl¹da lepiej)
    vec3 finalRGB = sandColor * max(lightColor, vec3(0.3));

    FragColor = vec4(finalRGB, 1.0);
}