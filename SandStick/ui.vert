#version 330 core
layout (location = 0) in vec2 aPos; // Pozycje 2D (X, Y)

void main()
{
    // Nie mno¿ymy przez ¿adne macierze! 
    // Pozycje s¹ w NDC (od -1 do 1), gdzie (0,0) to œrodek ekranu.
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}