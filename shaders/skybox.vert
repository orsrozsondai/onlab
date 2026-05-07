#version 450

layout(location = 0) out vec3 fragDir;

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
} vp;

vec3 vertices[36] = vec3[](
    vec3(-1,-1,-1), vec3( 1,-1,-1), vec3( 1, 1,-1),
    vec3( 1, 1,-1), vec3(-1, 1,-1), vec3(-1,-1,-1),
    vec3(-1,-1, 1), vec3( 1,-1, 1), vec3( 1, 1, 1),
    vec3( 1, 1, 1), vec3(-1, 1, 1), vec3(-1,-1, 1),
    vec3(-1, 1, 1), vec3(-1, 1,-1), vec3(-1,-1,-1),
    vec3(-1,-1,-1), vec3(-1,-1, 1), vec3(-1, 1, 1),
    vec3( 1, 1, 1), vec3( 1, 1,-1), vec3( 1,-1,-1),
    vec3( 1,-1,-1), vec3( 1,-1, 1), vec3( 1, 1, 1),
    vec3(-1,-1,-1), vec3( 1,-1,-1), vec3( 1,-1, 1),
    vec3( 1,-1, 1), vec3(-1,-1, 1), vec3(-1,-1,-1),
    vec3(-1, 1,-1), vec3( 1, 1,-1), vec3( 1, 1, 1),
    vec3( 1, 1, 1), vec3(-1, 1, 1), vec3(-1, 1,-1)
);

void main() {
    vec3 pos = vertices[gl_VertexIndex];
    fragDir = pos;
    mat4 rotView = mat4(mat3(vp.view)); 
    gl_Position = vp.proj * rotView * vec4(pos, 1.0);
}