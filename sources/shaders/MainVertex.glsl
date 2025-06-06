#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextCoord;


out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
    vec2 TextCoord;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform bool enableWind;
uniform float time; 
uniform float windStrength;
uniform vec3 windDirection;

void main() {
    vec3 finalPos = aPos;

    if (enableWind) {
        //disable if the element is too far from the camera
        float distanceFromCamera = length(model * vec4(aPos, 1.0));
        if (distanceFromCamera < 30.0) {
            vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;

            float windEffect = sin(time * 2.0 + worldPos.x * 1.0 + worldPos.z * 1.0) * 0.5 + 0.5;
            windEffect += sin(time * 3.5 + worldPos.y * 2.0) * 0.1;

            vec3 windOffset = windDirection * windStrength * windEffect * 0.2;

            finalPos = aPos + windOffset;
        }
    }

    gl_Position = projection * view * model * vec4(finalPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(finalPos, 1.0));
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vs_out.TextCoord = aTextCoord;
}