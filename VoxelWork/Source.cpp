#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>

#include "ResourceManager.h"
#include "camera.h";

#include "chunk.h"
#include "globals.h"

// Meta-Game Consts
const std::string GAMENAME = "Voxel Test";
const std::string VERSION = "0.0.2";

//



GLFWwindow* window = nullptr;

void init_GL();
bool init_Window(GLFWwindow** toInit);

//Callbacks for opengl
void framebuffer_size_callback(GLFWwindow* wd, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


void processInput(GLFWwindow* window);
void Update();
void Tick();
void Render();
void Events();
unsigned int& genTestTriangle();

glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)1280.f / (float)720.f, 0.1f, 100.0f);
camera* currentCamera;
glm::mat4 view = glm::mat4(1.0f);


node* currentScene;

chunk* testChunk;


int main()
{
	//Main Program
	// --Setup--
	init_GL();
	//Move back View
	
	
	

	if (!init_Window(&window))
	{
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, 1280, 720);
	// Add window resize callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	// Enable mouse input
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);


	//Enable Depth Testing and backface culling
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Load Shaders into memory
	ResourceManager::LoadShader("tri.vs", "tri.ps", nullptr, "triangle");
	


	//Create a new scene
	currentScene = new node();

	// Create a new camera
	currentCamera = new camera();
	// --Loop--
	Update();
	
	// --Shutdown--

	//Definite Deletes
	delete currentCamera;
	delete currentScene;
	//Temporary Deletes


	glfwTerminate();
	return 0;
}






// Init GL
void init_GL()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
bool init_Window(GLFWwindow** toInit)
{
	*toInit = glfwCreateWindow(1280, 720, "Artifice Engine", NULL, NULL);
	if (*toInit == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	return true;
}


void framebuffer_size_callback(GLFWwindow* wd, int width, int height)
{
	glViewport(0, 0, width, height);
}


float deltaTime = 0.0f;	// Time between current frame and last frame, used for maintaining rates between frames
float lastFrame = 0.f; // Time of last frame


bool blockPlaced = true;
bool keyPressed = false;
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//--TEMP--
	// Camera Input
	currentCamera->processInput(window,deltaTime);


	if (!keyPressed && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // Temp to demonstrate updateGeom and chunkUpdates
	{

		if (blockPlaced)
		{
			testChunk->deleteBlock(0, 0, 0);
			blockPlaced = false;
		}
		else
		{
			testChunk->setBlock(0, 0, 0, 3);
			blockPlaced = true;
		}
		keyPressed = true;
	}
	if (keyPressed && glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
	{
		keyPressed = false;
	}
}


void Update()
{
	


	//Select the triangle shader
	unsigned int testTriVAO = genTestTriangle();
	glm::mat4 trans = glm::mat4(1.0f);

	//Set the shader uniforms for view and projection
	float totalCount = 0.f;
	
	bool chunkMade = false;
	testChunk = new chunk();
	testChunk->createFullChunk();

	//Window Title Stringstream (Credit : https://stackoverflow.com/questions/18412120/displaying-fps-in-glfw-window-title)
	std::stringstream title;
	float titleUpdateTime=0.f;


	
	testChunk->deleteBlock(3, 0, 0);
	testChunk->deleteBlock(3, 0, 1);
	testChunk->deleteBlock(0, 0, 3);
	testChunk->deleteBlock(1, 0, 3);
	testChunk->deleteBlock(0, 1, 0);
	testChunk->setBlock(2, 0, 0, 2);
	testChunk->setBlock(0, 0, 0, 2);
	testChunk->setBlock(1, 0, 3, 2);
	

	// ---Add objects to scene---
	currentScene->addChild(*currentCamera);

	//Add test chunk as a child
	currentScene->addChild(*testChunk);

	while (!glfwWindowShouldClose(window))
	{
		//PROCESS
		//UPDATE
		//DRAW
		deltaTime = glfwGetTime() - lastFrame;
		dt = deltaTime;
		lastFrame = glfwGetTime();
		titleUpdateTime += deltaTime;
		//Update Window Title
		if (titleUpdateTime > .5f)
		{
			title << GAMENAME << " " << VERSION << " | " << (float)1.f / deltaTime << " FPS |";
			glfwSetWindowTitle(window, title.str().c_str());
			title.str(std::string()); // Update the title every 0.5 seconds
			titleUpdateTime = 0.f;
		}
		
		
		//Process Input and Events
		Events();
		


		
		//Tick
		Tick();

		
		


		
		
		
		//Render here

		glClearColor(0.2f, 0.55f, 0.75f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		Render();


		

		//Call Events and Swap Buffers
		glfwSwapBuffers(window);
		
	}
}




//Test Code for generating a VAO of a testing triangle, used as a backup in case all rendering fails
unsigned int& genTestTriangle()
{

	float vertices[] = {
		-0.5f, -0.5f, 1.0f,
		 0.5f, -0.5f, 1.0f,
		 0.0f,  0.5f, 1.0f
	};
	unsigned int VBO, triVAO;
	glGenVertexArrays(1, &triVAO); //Generate buffers and arrays
	glGenBuffers(1, &VBO);

	//Bind OpenGL to VAO object
	glBindVertexArray(triVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0); //Unbind Array buffer

	//Unbind Vertex Array Object so we don't end up (accidentally modifying it)
	glBindVertexArray(0);
	return triVAO;
}


float lastPosX = 640;
float lastPosY = 360;
float xOffset;
float yOffset;
float sensitivity = 0.1f;

bool mouseEnteredWindow = false;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!mouseEnteredWindow)
	{
		//Update the mouse so that the starting position is set here
		lastPosX = xpos;
		lastPosY = ypos;
		mouseEnteredWindow = true; //Mouse has now entered the window
	}
	xOffset = xpos - lastPosX;
	yOffset = lastPosY - ypos; // bottom to top Y SPace
	lastPosX = xpos;
	lastPosY = ypos;


	xOffset *= sensitivity;
	yOffset *= sensitivity;

	currentCamera->RotateView(xOffset, yOffset);
}

void Events()
{
	glfwPollEvents();
	processInput(window);
}

void Tick()
{
	//Update the current scene
	currentScene->tick();

}

void Render()
{
	currentScene->render(*currentCamera);
}

