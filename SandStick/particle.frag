#version 330 core
out vec4 FragColor;

uniform vec3 lightColor; 

void main()
{
    // Bazowy kolor piasku
    vec3 sandColor = vec3(0.937, 0.894, 0.690);

    // NAPRAWA:
    // 1. Usun¹³em funkcjê max(). Teraz jak lightColor jest czarny, cz¹steczka te¿ jest czarna.
    // 2. Doda³em mno¿nik (* 1.5). Cz¹steczki s¹ ma³e i ³api¹ du¿o œwiat³a w locie, 
    //    wiêc warto je lekko podbiæ, ¿eby odcina³y siê od t³a, ALE zachowuj¹c kolor nocy.
    
    vec3 finalRGB = sandColor * lightColor * 1.5;

    FragColor = vec4(finalRGB, 1.0);
}