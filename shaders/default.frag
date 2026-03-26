#version 450

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 worldNormal;
layout(location = 2) in vec3 worldTangent;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform MaterialUBO {
    vec3 albedo;
    float metallic;
    float roughness;
    
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;    
} material;

layout(set = 1, binding = 0) uniform SceneUBO {
    vec4 lightPos;
    vec3 lightColor;
    vec3 ambientLight;

    vec3 camPos;
} scene;

#define PI 3.1415926535
vec3 F_schlick(vec3 v, vec3 h) {
    float vdoth = max(dot(v,h), 0.0);
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
    return F0 + (1.0 - F0) * pow(1.0 - vdoth, 5.0);
}

vec3 fd_lambert(vec3 l) {
    float ndotl = max(dot(normalize(worldNormal), l), 0.0);
    return material.albedo / PI * ndotl * scene.lightColor;
}

vec3 fs_blinnphong(vec3 h) {
    float ndoth = max(dot(normalize(worldNormal), h), 0.0);
    float shininess = pow(2.0, mix(1.0, 10.0, 1.0 - material.roughness));
    return (shininess + 2) * (0.5/PI) * pow(ndoth, shininess) * scene.lightColor ;
}

void main() {
    vec3 l;
    if (scene.lightPos.w == 1.0)
        l = normalize(scene.lightPos.xyz - worldPosition);
    else
        l = normalize(-scene.lightPos.xyz);

    vec3 v = normalize(scene.camPos - worldPosition);
    vec3 h = normalize(v+l);

    vec3 color = scene.ambientLight * material.albedo * 0.1; 
    color += fd_lambert(l);

    color += fs_blinnphong(h) * 0.1 * max(dot(normalize(worldNormal), l), 0);

    

    outColor = vec4(color, 1.0);
}