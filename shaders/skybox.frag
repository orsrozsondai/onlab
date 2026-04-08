#version 450

layout(location = 0) in vec3 fragDir;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

vec3 toneMapACES(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b)) / (x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 dir = normalize(fragDir);
    vec3 color = texture(environmentMap, dir).rgb;
    // color = vec3(1.0, 0, 0);
    outColor = vec4(toneMapACES(color), 1.0);
}