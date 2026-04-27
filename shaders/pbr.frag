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
    bool ibl;
    vec4 lightPos;
    vec3 lightColor;
    vec3 ambientLight;

    vec3 camPos;

    bool toneMapping;
    float exposure;
} scene;

layout(set = 2, binding = 0) uniform samplerCube irradiance;
layout(set = 2, binding = 1) uniform samplerCube prefilter;
layout(set = 2, binding = 2) uniform sampler2D brdfLUT;

const float PI = 3.1415926535;
vec3 F_schlick(vec3 v, vec3 h) {
    float vdoth = max(dot(v,h), 0.0);
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
    return F0 + (1.0 - F0) * pow(1.0 - vdoth, 5.0);
}

vec3 F_schlick_roughness(float ndotv, float roughness) {
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - ndotv, 5.0);
}

float G1(vec3 v) {
    float k = pow((material.roughness + 1), 2) / 8;
    float ndotv = max(dot(normalize(worldNormal), v), 0);
    return max(ndotv / (ndotv * (1-k) + k), 0);
}

float G_Smith(vec3 l, vec3 v) {
    return max(G1(v) * G1(l), 0);
}

float D_GGX(vec3 h) {
    float ndoth = max(dot(normalize(worldNormal), h), 0);
    float alpha2 = pow(material.roughness, 4);
    return alpha2/max(PI * pow(ndoth*ndoth * (alpha2-1) + 1, 2), 0.00000001);
}

vec3 fs_CookTorrance(vec3 v, vec3 l, vec3 h) {
    float ndotl = dot(normalize(worldNormal), l);
    float ndotv = dot(normalize(worldNormal), v);
    return (D_GGX(h) * F_schlick(v, h) * G_Smith(l, v)) / max(4 * ndotl * ndotv, 0.00000001);
}

vec3 fd_lambert() {
    return material.albedo / PI;
}

vec3 fs_blinnphong(vec3 h) {
    float ndoth = max(dot(normalize(worldNormal), h), 0.0);
    float shininess = pow(2.0, mix(1.0, 10.0, 1.0 - material.roughness));
    return (shininess + 2) * (0.5/PI) * pow(ndoth, shininess) * scene.lightColor ;
}
vec3 toneMapACES(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b)) / (x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 l;
    if (scene.lightPos.w == 1.0)
        l = normalize(scene.lightPos.xyz - worldPosition);
    else
        l = normalize(-scene.lightPos.xyz);

    vec3 v = normalize(scene.camPos - worldPosition);
    vec3 h = normalize(v+l);

    float ndotl = max(dot(normalize(worldNormal), l), 0.0);
    float ndotv = max(dot(normalize(worldNormal), v), 0.0);


    

    vec3 color = scene.ambientLight * material.albedo * 0.1; 
    
    if (scene.ibl) {
        vec3 kD = vec3(1) - F_schlick_roughness(ndotv, material.roughness);
        kD *= (1.0 - material.metallic);

        color += texture(irradiance, normalize(worldNormal)).rgb * fd_lambert() * kD;

        vec3 R = reflect(-v,normalize(worldNormal));
        vec3 prefilteredColor = textureLod(prefilter, R,  material.roughness * 11.0f).rgb;
        vec2 brdf = texture(brdfLUT, vec2(ndotv, material.roughness)).rg;
        vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);
        color += prefilteredColor * (F0 * brdf.x + brdf.y);
    }
    else {
        vec3 kD = vec3(1.0) - F_schlick(v, h);
        kD *= (1.0 - material.metallic);

        color += fd_lambert() * ndotl * scene.lightColor * kD;
        color += fs_CookTorrance(v, l, h) * ndotl * scene.lightColor;
    }
    if (scene.toneMapping) {
        color *= scene.exposure;
        color = toneMapACES(color);
    }

    // color = textureLod(prefilter, worldNormal,  10.0f).rgb;
    outColor = vec4(color, 1.0);

}