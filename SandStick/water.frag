#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 fogColor;
uniform vec3 lightColor;
uniform vec3 dirLightDir;

void main()
{
    vec3 baseColor = vec3(0.0, 0.2, 0.8); 

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLightDir);

    vec3 ambient = lightColor * 0.1;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.6;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = spec * lightColor * 10.0; 

    vec3 result = (ambient + diffuse) * baseColor;

    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((dist - 50.0) / 250.0, 0.0, 1.0);
    result = mix(result, fogColor, fogFactor);

    result += specular;

    FragColor = vec4(result, 1.0);
}