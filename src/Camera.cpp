#include "Camera.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/matrix_transform.hpp>


Camera::Camera(const glm::vec3& target, float distance, float aspectRatio, float fov)
    : target(target),
      distance(distance),
      aspectRatio(aspectRatio),
      yaw(90.0f),
      pitch(20.0f),
      fov(fov),
      nearPlane(0.1f),
      farPlane(10.f)
{
    updateView();
    updateProjection();
}
void Camera::updateView() {
    glm::vec3 offset;

    offset.x = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    offset.y = distance * sin(glm::radians(pitch));
    offset.z = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    position = target + offset;

    glm::vec3 up(0,1,0);

    viewMat = glm::lookAt(position, target, up);
}

void Camera::updateProjection() {
    projMat = glm::perspective(
        glm::radians(fov),
        aspectRatio,
        nearPlane,
        farPlane
    );
    projMat[1][1] *= -1;
}
void Camera::setAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    updateProjection();
}

void Camera::lookAt(const glm::vec3& p) {
    target = p;
    updateView();
}

void Camera::setFov(float fov) {
    this->fov = fov;
    updateProjection();
}

void Camera::rotate(float yawOffset, float pitchOffset) {
    yaw   += yawOffset;
    pitch += pitchOffset;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    updateView();
}
void Camera::zoom(float d) {
    distance -= d;

    distance = glm::clamp(distance, 1.0f, 50.0f);

    updateView();
}

glm::mat4 Camera::proj() const {
    return projMat;
}

glm::mat4 Camera::view() const {
    return viewMat;
}

glm::vec3 Camera::getPosition() const {
    return position;
}
float Camera::getFov() const {
    return fov;
}