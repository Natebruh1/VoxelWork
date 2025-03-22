#include "camera.h"
#include "WorldSpace.h"
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

camera::~camera()
{
	node::~node();
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
	yaw = fmod(yaw, 360.f);
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
	if (glfwGetKey(windowRef, GLFW_KEY_SPACE) == GLFW_PRESS)
		position += cameraUp * cameraSpeed*2.f; //Go up faster
	if (glfwGetKey(windowRef, GLFW_KEY_E) == GLFW_PRESS)
		position -= cameraUp * cameraSpeed * 2.f; //Go up faster

	if (glfwGetKey(windowRef, GLFW_KEY_L) == GLFW_PRESS)
		speed += dt * 5.f;
	if (glfwGetKey(windowRef, GLFW_KEY_K) == GLFW_PRESS)
		speed -= dt * 5.f;
	if (glfwGetMouseButton(windowRef, GLFW_MOUSE_BUTTON_LEFT))
		DeleteClosestBlock();
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

float camera::getYaw()
{
	return yaw;
}

glm::vec3& camera::getFront()
{
	return cameraFront;
}

void camera::DeleteClosestBlock()
{
	glm::ivec3 cameraBlockPos = glm::ivec3(floor(getPosition()));
	glm::ivec3 finalPos = glm::ivec3(floor(getPosition() + getFront() * 8.f));
	int x0 = cameraBlockPos.x;
	int y0 = cameraBlockPos.y;
	int z0 = cameraBlockPos.z;

	int x1 = finalPos.x;
	int y1 = finalPos.y;
	int z1 = finalPos.z;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int dz = abs(z1 - z0);

	int xs;
	int ys;
	int zs;

	x1 > x0 ? xs = 1 : xs = -1;
	y1 > y0 ? ys = 1 : ys = -1;
	z1 > z0 ? zs = 1 : zs = -1;

	glm::ivec3 currentPoint{};
	bool blockFound = false;
	if (dx > dy && dx > dz)
	{
		int p1 = 2 * dy - dx;
		int p2 = 2 * dz - dx;
		while (x0 != x1 and !blockFound)
		{
			x0 += xs;
			if (p1 > 0)
			{
				y0 += ys;
				p1 -= 2 * dx;
			}
			if (p2 >= 0)
			{
				z0 += zs;
				p2 -= 2 * dx;
			}
			p1 += 2 * dy;
			p2 -= 2 * dx;
			if (WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f)))
			{
				//Check block isn't air
				if (WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f))->getBlock(((x0 % 16) * (x0 >= 0)) + ((x0 < 0) * (16 - (-x0 % 16))), ((y0 % 16) * (y0 >= 0)) + ((y0 < 0) * (16 - (-y0 % 16))), ((z0 % 16) * (z0 >= 0)) + ((z0 < 0) * (16 - (-z0 % 16)))).id != 0)
				{
					blockFound = true;
					break;
				}
				
			}
			else
			{
				//Chunk not found
				break;
			}
		}
		if (blockFound)
		{
			WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f))->setBlock(((x0 % 16) * (x0 >= 0)) + ((x0 < 0) * (16 - (-x0 % 16))), ((y0 % 16) * (y0 >= 0)) + ((y0 < 0) * (16 - (-y0 % 16))), ((z0 % 16) * (z0 >= 0)) + ((z0 < 0) * (16 - (-z0 % 16))),0);
		}
		else
		{
			return;
		}
	}
	else if (dy >= dx && dy >= dz)
	{
		int p1 = 2 * dx - dy;
		int p2 = 2 * dz - dy;
		while (y0 != y1)
		{
			y0 += ys;
			if (p1 >= 0)
			{
				x0 += xs;
				p1 -= 2 * dy;
			}
			if (p2 >= 0)
			{
				z0 += zs;
				p2 -= 2 * dy;
			}
			p1 += 2 * dx;
			p2 += 2 * dz;
			if (WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f)))
			{
				//Check block isn't air
				if (WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f))->getBlock(((x0 % 16) * (x0 >= 0)) + ((x0 < 0) * (16 - (-x0 % 16))), ((y0 % 16) * (y0 >= 0)) + ((y0 < 0) * (16 - (-y0 % 16))), ((z0 % 16) * (z0 >= 0)) + ((z0 < 0) * (16 - (-z0 % 16)))).id != 0)
				{
					blockFound = true;
					break;
				}

			}
			else
			{
				//Chunk not found
				break;
			}
		}
		if (blockFound)
		{
			WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f))->setBlock(((x0 % 16) * (x0 >= 0)) + ((x0 < 0) * (16 - (-x0 % 16))), ((y0 % 16) * (y0 >= 0)) + ((y0 < 0) * (16 - (-y0 % 16))), ((z0 % 16) * (z0 >= 0)) + ((z0 < 0) * (16 - (-z0 % 16))),0);
		}
		else
		{
			return;
		}
		// Driving axis is Z-axis"
	}
	else
	{
		int p1 = 2 * dy - dz;
		int p2 = 2 * dx - dz;
		while (z0 != z1)
		{
			z0 += zs;
			if (p1 >= 0)
			{
				y0 += ys;
				p1 -= 2 * dz;
			}
			if (p2 >= 0)
			{
				x0 += xs;
				p2 -= 2 * dz;
			}
			p1 += 2 * dy;
			p2 += 2 * dx;
			if (WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f)))
			{
				//Check block isn't air
				if (WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f))->getBlock(((x0 % 16)*(x0>=0)) + ((x0<0) * (16 - (-x0 % 16))), ((y0 % 16) * (y0 >= 0)) + ((y0 < 0) * (16 - (-y0 % 16))), ((z0 % 16) * (z0 >= 0)) + ((z0 < 0) * (16 - (-z0 % 16)))).id != 0)
				{
					blockFound = true;
					break;
				}

			}
			else
			{
				//Chunk not found
				break;
			}
		}
		if (blockFound)
		{
			WorldSpace::CurrentWorld->getChunk(floor((float)x0 / 16.f), floor((float)y0 / 16.f), floor((float)z0 / 16.f))->setBlock(((x0 % 16)* (x0 >= 0)) + ((x0 < 0) * (16 - (-x0 % 16))), ((y0 % 16)* (y0 >= 0)) + ((y0 < 0) * (16 - (-y0 % 16))), ((z0 % 16)* (z0 >= 0)) + ((z0 < 0) * (16 - (-z0 % 16))),0);
		}
		else
		{
			return;
		}
	}
}
