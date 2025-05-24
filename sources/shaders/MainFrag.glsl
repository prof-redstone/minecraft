#version 330 core

out vec4 FragColor;

struct Light {
    int lightType;
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float distance; //-1.0 => 0 attenuation, <0.0 => dist to no light (aprox)

    mat4 lightSpaceMatrix;
    int castshadow;
    int shadowIndex;
    int PCFSize;
};

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
    vec2 TextCoord;
} fs_in;

struct Material {
    vec4 color;
    int shininess;
    float ambianteLightMult;
    int enableTexture;
    sampler2D text;
};



#define MAX_LIGHTS 8
#define MAX_SHADOWS 4
uniform Light lights[MAX_LIGHTS];
uniform sampler2D shadowMaps[MAX_SHADOWS];
uniform int numLights;
uniform float gamma = 0.7;
uniform vec3 viewPos;
uniform Material mat;


float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 lightDir, int PCFSize) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(fs_in.Normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -PCFSize; x <= PCFSize; ++x) {
        for (int y = -PCFSize; y <= PCFSize; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= (1 + 2 * PCFSize) * (1 + 2 * PCFSize);

    vec3 distanceFromEdge;
    distanceFromEdge.x = min(projCoords.x, 1.0 - projCoords.x);
    distanceFromEdge.y = min(projCoords.y, 1.0 - projCoords.y);
    distanceFromEdge.z = min(projCoords.z, 1.0 - projCoords.z);
    float distToEdge = min(min(distanceFromEdge.x, distanceFromEdge.y), distanceFromEdge.z) * 10.0;
    float fadeout = clamp(distToEdge, 0.0, 1.0);
    shadow *= fadeout;
    return shadow;
}

vec3 computeLight(Light l, vec3 lightDir, vec3 viewDir, float shadow) {
    vec3 result = vec3(0.0, 0.0, 0.0);
    vec3 reflectDir = reflect(-lightDir, fs_in.Normal);

    result += max(dot(fs_in.Normal, lightDir), 0.0) * l.diffuse;
    result += pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess) * l.specular;
    result *= 1.0 - shadow;
    result += l.ambient*mat.ambianteLightMult;

    if (l.distance != -1.0) {
        float d = length(l.position - fs_in.FragPos);
        float attenuation = 1.0 / (1.0 + (5.0 / l.distance) * d + (75.0 / (l.distance * l.distance)) * (d * d));
        result *= attenuation;
    }

    return result * mat.color.rgb;
}

void main() {
    vec3 result = vec3(0.0);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    if (dot(viewDir, fs_in.Normal) < 0.0)
        discard;

    for (int i = 0; i < numLights; ++i) {
        float shadow = 0.0;
        vec3 lightDir = vec3(0.0, 0.0, 0.0);

        if (lights[i].lightType == 1) {
            lightDir = normalize(lights[i].position - fs_in.FragPos);
        }
        else if (lights[i].lightType == 0) {
            lightDir = normalize(lights[i].direction);
        }

        if ((lights[i].castshadow != 0) && (lights[i].shadowIndex >= 0)) {
            vec4 fragPosLightSpace = lights[i].lightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
            shadow = ShadowCalculation(fragPosLightSpace, shadowMaps[lights[i].shadowIndex], lightDir, lights[i].PCFSize);
        }

        result += computeLight(lights[i], lightDir, viewDir, shadow);
    }

    FragColor = vec4(result*gamma, mat.color.a);
    if (mat.enableTexture == 1) {
        vec4 textureCol = texture(mat.text, fs_in.TextCoord);
        if (textureCol.w < 0.1)
            discard;
        FragColor = vec4(result * gamma, mat.color.a) * textureCol;
    }
}