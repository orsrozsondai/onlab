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

layout(set = 0, binding = 2) uniform SceneUBO {
    vec4 lightPos;
    vec3 lightColor;
    vec3 ambientLight;

    vec3 camPos;
} scene;

void main() {
    
    outColor = vec4(material.albedo, 1.0);
}
