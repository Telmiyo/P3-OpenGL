#include "camera.h"

Camera::Camera()
{
	position = vec3(0.0f, 0.0f, 0.0f);

	front = vec3(0.0f, -1.0f, 0.0f);
	right = vec3(1.0f, 0.0f, 0.0f);
	up = worldUp = vec3(0.0f, 1.0f, 0.0f);

	movementSpeed = 2.5f;
	mouseSensitivity = 0.1f;
	zoom = 45.0f;
}

Camera::Camera(vec3 _position, vec3 _up, float _yaw, float _pitch) : front(vec3(0.0f, 0.0f, -1.0f)), movementSpeed(2.5f), mouseSensitivity(0.1f), zoom(75.0f)
{
	position = _position;
	worldUp = _up;
	yaw = _yaw;
	pitch = _pitch;
	Update();
}

mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void Camera::Update()
{
	// calculate the new Front vector
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);
	// also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, front));
}

void Camera::UpdateKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;
	if (direction == CAMERA_FORWARD)
		position += front * velocity;
	if (direction == CAMERA_BACKWARD)
		position -= front * velocity;
	if (direction == CAMERA_LEFT)
		position -= right * velocity;
	if (direction == CAMERA_RIGHT)
		position += right * velocity;
}

void Camera::UpdateMouse(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	// update Front, Right and Up Vectors using the updated Euler angles
	Update();
}