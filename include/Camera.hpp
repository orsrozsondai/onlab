#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
class Camera {
private:
    glm::vec3 target, position;
    float distance,  nearPlane, farPlane, aspectRatio, fov;

    glm::mat4 viewMat, projMat;
    
    float yaw;
    float pitch;

    void updateView();
    void updateProjection();

public:
    Camera(const glm::vec3& target, float distance, float aspectRatio, float fov);
    
    void setAspectRatio(float aspectRatio);

    void lookAt(const glm::vec3& p);

    void setFov(float fov);

    void rotate(float yawOffset, float pitchOffset);
    void zoom(float d);

    glm::mat4 proj() const;

    glm::mat4 view() const;

    glm::vec3 getPosition() const;

    float getFov() const;

};