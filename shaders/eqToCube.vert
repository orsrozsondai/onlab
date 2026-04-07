#version 450

layout(location = 0) out vec3 fragDir;

layout(push_constant) uniform PushConstants {
    mat4 view;
    mat4 proj;
} pc;

vec3 vertices[36] = vec3[](
    // Cube vertices (36 total for 12 triangles)
    vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0), vec3( 1.0,  1.0, -1.0),
    vec3( 1.0,  1.0, -1.0), vec3(-1.0,  1.0, -1.0), vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0,  1.0), vec3( 1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0, -1.0,  1.0),
    vec3(-1.0,  1.0,  1.0), vec3(-1.0,  1.0, -1.0), vec3(-1.0, -1.0, -1.0),
    vec3(-1.0, -1.0, -1.0), vec3(-1.0, -1.0,  1.0), vec3(-1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0, -1.0, -1.0),
    vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0), vec3( 1.0,  1.0,  1.0),
    vec3(-1.0, -1.0, -1.0), vec3( 1.0, -1.0, -1.0), vec3( 1.0, -1.0,  1.0),
    vec3( 1.0, -1.0,  1.0), vec3(-1.0, -1.0,  1.0), vec3(-1.0, -1.0, -1.0),
    vec3(-1.0,  1.0, -1.0), vec3( 1.0,  1.0, -1.0), vec3( 1.0,  1.0,  1.0),
    vec3( 1.0,  1.0,  1.0), vec3(-1.0,  1.0,  1.0), vec3(-1.0,  1.0, -1.0)
);

void main() {
    vec3 pos = vertices[gl_VertexIndex];
    fragDir = pos;
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
}