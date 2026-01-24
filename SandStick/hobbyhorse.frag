#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

uniform sampler2D texSlot1;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform PointLight lights[2];
uniform vec3 dirLightDir;

uniform vec3 fogColor;
uniform vec3 lightColor;

void main()
{
    vec3 baseColor;
    float shininess;      
    float specularStrength; 

    baseColor = texture(texSlot1, TexCoords).rgb;

    specularStrength = 0.1;
    shininess = 16.0;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLightDir);

    float sunIntensity = max(lightColor.r, max(lightColor.g, lightColor.b));
    float dayFactor = smoothstep(0.1, 0.8, sunIntensity);
    float exposure = mix(2.5, 0.5, dayFactor);
    float ambientStrength = mix(0.4, 0.05, dayFactor);

    vec3 ambient = lightColor * ambientStrength * baseColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * baseColor * exposure;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * lightColor * 0.5; 

    vec3 result = ambient + diffuse + specular;

    float dist = length(viewPos - FragPos);
    float fogStart = 20.0;
    float fogEnd = 150.0;
    float fogFactor = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    vec3 finalColor = mix(result, fogColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}