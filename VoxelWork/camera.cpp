#include "camera.h"

camera::camera()
{
	//Set the base position
	position= glm::vec3(0.0f, 0.0f, 3.0f);

	//Setup the reverse direction
	cameraDirection = glm::normalize(position - cameraTarget);

	//Set the Axis, Right then Up
	cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	cameraUp = glm::cross(cameraDirection, cameraRight);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

	cameraView = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
}

void camera::tick()
{
	node::tick();
	cameraView = glm::lookAt(position, position + cameraFront, cameraUp);


	//Update the rotation of the camera by the given yaw and pitch - using the euler angle formula
	rotation.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	rotation.y = sin(glm::radians(pitch));
	rotation.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(rotation);
}

void camera::processInput(GLFWwindow* const& windowRef,float dt)
{
	const float cameraSpeed = speed*dt; // adjust accordingly
	if (glfwGetKey(windowRef, GLFW_KEY_W) == GLFW_PRESS)
		position += cameraSpeed * cameraFront;
	if (glfwGetKey(windowRef, GLFW_KEY_S) == GLFW_PRESS)
		position -= cameraSpeed * cameraFront;
	if (glfwGetKey(windowRef, GLFW_KEY_A) == GLFW_PRESS)
		position -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(windowRef, GLFW_KEY_D) == GLFW_PRESS)
		position += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void camera::RotateView(float x, float y)
{
	yaw += x;
	pitch += y;
	//Lock vertical looking
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
}
