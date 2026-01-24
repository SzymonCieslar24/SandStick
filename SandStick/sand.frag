#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texSlot1;
uniform sampler2D texSlot2;

uniform vec3 lightColor;
uniform vec3 dirLightDir;
uniform vec3 viewPos;
uniform vec3 fogColor;

uniform float waterLevel; 

void main()
{
    vec2 tiledCoords = TexCoords * 8.0; 

    vec3 dryColor = texture(texSlot1, tiledCoords).rgb;
    vec3 wetColor = texture(texSlot2, tiledCoords).rgb;

    float h = smoothstep(waterLevel - 0.5, waterLevel + 1.5, FragPos.y);

    vec3 baseColor = mix(wetColor, dryColor, h);

    float specularStrength = mix(0.5, 0.0, h); 
    float shininess = mix(32.0, 2.0, h);

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-dirLightDir);

    float sandExposure = 0.6; 

    vec3 ambient = lightColor * 0.05 * baseColor; 

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * lightColor * baseColor * sandExposure;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * specularStrength * lightColor * 0.5; 

    vec3 result = ambient + diffuse + specular;

    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((dist - 20.0) / 130.0, 0.0, 1.0);
    
    vec3 finalColor = mix(result, fogColor, fogFactor);

    finalColor = min(finalColor, vec3(1.0));

    FragColor = vec4(finalColor, 1.0);
}