#version 430 core

uniform sampler2D mainTexture;

uniform vec3 cameraPosition;
uniform vec3 lightPos;
uniform vec3 lightColor;

in vec2 gTexCoord;
in vec3 gPosition;
in vec3 gTangent;

out vec4 color;

void main()
{
    vec3 colorOfHair = vec3(texture(mainTexture, gTexCoord));
    //Using The Kajiya-Kay lighting model
    vec3 light = normalize(lightPos - gPosition);
    float diffuseCoefficient = length(cross(light, gTangent));
    if(diffuseCoefficient < 0.7)  diffuseCoefficient = 0.7;
    vec3 diffuse = lightColor * colorOfHair * diffuseCoefficient;

    vec3 viewDirection = normalize(gPosition - cameraPosition);
    float shininess = 50;
    float specularExponent = pow(max(dot(gTangent, light) * dot(gTangent, viewDirection) + length(cross(gTangent,light))*length(cross(gTangent,viewDirection)), 0), shininess);
    vec3 specular = lightColor * colorOfHair * 0.5 * specularExponent;

    color = vec4(diffuse + specular, 0.9);
}