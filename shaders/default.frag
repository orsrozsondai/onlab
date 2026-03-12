#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 color;

layout(set = 0, binding = 1) uniform MaterialUBO {
    vec3 albedo;
    float metallic;
    float roughness;

    
} material;

layout(set = 0, binding = 2) uniform SceneUBO {
    vec4 lightPos;
    vec3 lightColor;

    vec3 camPos;
} scene;

void main() {
    float brightness = color.r + color.g + color.b;
    brightness /= 3;
    outColor = vec4(material.albedo * brightness, 1);

}
