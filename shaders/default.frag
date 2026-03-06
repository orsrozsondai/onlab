#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 color;

layout(set = 0, binding = 1) uniform UniformBufferObject {
    vec3 albedo;
    float metallic;
    float roughness;

    vec4 lightPos;
    vec3 lightColor;

    vec3 camPos;
} ubo;

void main() {
    outColor = vec4(ubo.albedo, 1);
}
