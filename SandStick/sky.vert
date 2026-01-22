#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    // Usuwamy translacjê z macierzy widoku, ¿eby niebo "pod¹¿a³o" za graczem
    // (¿ebyœ nigdy nie doszed³ do œciany nieba)
    mat4 viewRot = mat4(mat3(view)); 
    vec4 pos = projection * viewRot * vec4(aPos, 1.0);
    
    // Trick: Ustawiamy g³êbiê na maksymaln¹ (z = w), ¿eby niebo by³o zawsze "z ty³u"
    gl_Position = pos.xyww;
}