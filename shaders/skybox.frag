#version 450

layout(location = 0) in vec3 fragDir;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

void main() {
    vec3 dir = normalize(fragDir);
    vec3 color = texture(environmentMap, dir).rgb;
    // color = vec3(1.0, 0, 0);
    outColor = vec4(color, 1.0);
}