#ifndef CAMERA_H
#define CAMERA_H

#include "platform.h"

typedef glm::vec3  vec3;
typedef glm::mat4  mat4;
typedef unsigned char GLboolean;

enum Camera_Movement {
	CAMERA_FORWARD,
	CAMERA_BACKWARD,
	CAMERA_LEFT,
	CAMERA_RIGHT
};

enum Camera_Mode {
	FREE,
	GUI,
};

class Camera
{
public:
	// Constructors
	Camera();
	Camera(vec3 position, vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);

	// Getters
	mat4 GetViewMatrix();
	
	// Functions
	void Update();
	void UpdateKeyboard(Camera_Movement direction, float deltaTime);
	void UpdateMouse(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void UpdateGUI();

	void ResetTransform();

	// Properties
	vec3 position;
	vec3 front;
	vec3 up;
	vec3 right;
	vec3 worldUp;

	float yaw = -90.0f;
	float pitch = 0.0f;
	float aspectRatio = 0.0f;

	float movementSpeed = 0.5f;
	float mouseSensitivity = 0.5f;
	float zoom = 0.5f;

	float alpha = PI / 2.0f;
	float camDist = 25.0f;
	float camHeight = 0.0f;

	Camera_Mode target;
};

#endif