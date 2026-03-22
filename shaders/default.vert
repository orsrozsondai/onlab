#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 worldNormal;
layout(location = 2) out vec3 worldTangent;


layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPos, 1.0);
    worldPosition = (mvp.model * vec4(inPos, 1.0)).xyz;
    worldNormal = (mvp.model * vec4(inNormal, 1.0)).xyz;
    worldTangent = (mvp.model * vec4(inTangent, 1.0)).xyz;

}
