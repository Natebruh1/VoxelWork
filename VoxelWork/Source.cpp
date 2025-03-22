#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>

#include "ResourceManager.h"
#include "camera.h";

#include "LightManager.h"
#include "WorldSpace.h"
#include "PartSpace.h"
#include "Model.h"
#include "chunk.h"
#include "globals.h"
ModelLibrary modelLibrary;


// LUA Scripting
#include "LuaCPP/LuaCpp.hpp"
using namespace LuaCpp;
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;


// Meta-Game Consts
const std::string GAMENAME = "Voxel Test";
const std::string VERSION = "0.0.4";

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

glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)1280.f / (float)720.f, 0.1f, 30000.0f);
camera* currentCamera;
glm::mat4 view = glm::mat4(1.0f);


node* currentScene;

chunk* testChunk;
chunk* chunkThree;
WorldSpace* wSpace;
PartSpace* testPart;
node3D* testModel;
node3D* testModel2;

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

    //Add a new chunkSpace
    wSpace = new WorldSpace();

    //Create a new scene
    currentScene = new node();

    // Create a new camera
    currentCamera = new camera();





    // --Loop--
    Update();

    // --Shutdown--

    //Save remaining chunks
    nlohmann::json data;

    //wSpace->loadFromDisc();
    wSpace->serialize(data, true);   //LAG CAUSERS -- Serialize all loaded chunks and then save, currently about 100kb per 70 chunks ~ for 12^3 chunks that is about 2mb without zipping
    // or without accounting for the fact that a lot of chunks are pure air
    wSpace->saveToDisc();           //AND THIS
    data.clear();

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


float deltaTime = 0.0f; // Time between current frame and last frame, used for maintaining rates between frames
float lastFrame = 0.f; // Time of last frame


bool blockPlaced = true;
bool keyPressed = false;
Model testModelModel;
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {

        glfwSetWindowShouldClose(window, true);
        std::cout << "EXITING" << std::endl;



    }


    //--TEMP--
    // Camera Input
    currentCamera->processInput(window, deltaTime);


    if (!keyPressed && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // Temp to demonstrate updateGeom and chunkUpdates
    {

        if (blockPlaced)
        {
            testChunk->deleteBlock(0, 0, 0);
            //testChunk->updateChunkLighting();
            wSpace->getLightsManager()->setLightUpdated(0);


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


    for (int x = 0; x < 5; x++)
    {
        for (int y = 0; y < 5; y++)
        {
            for (int z = 0; z < 5; z++)
            {
                if (x * x * x + y * y * y + z * z * z < 100)
                {
                    testChunk->deleteBlock(x, y, z);
                }
            }
        }
    }

    testChunk->deleteBlock(3, 0, 0);
    testChunk->deleteBlock(3, 0, 1);
    testChunk->deleteBlock(0, 0, 3);
    testChunk->deleteBlock(1, 0, 3);
    testChunk->deleteBlock(0, 1, 0);
    testChunk->setBlock(2, 0, 0, 2);
    testChunk->setBlock(0, 0, 0, 2);
    testChunk->setBlock(1, 0, 3, 2);
    testChunk->deleteBlock(5, 1, 0);
    testChunk->deleteBlock(0, 1, 5);
    testChunk->deleteBlock(0, 1, 6);

    // ---Add objects to scene---
    currentScene->addChild(*currentCamera);
    //Add models to world
    for (auto& m : ModelLibrary::modelList)
    {
        Model* mod = (ModelLibrary::modelList[(m.first)]);
        currentScene->addChild(*mod);

    }



    //Add test chunk as a child
    currentScene->addChild(*wSpace);

    //Generate Chunk from ChunkSpace
    wSpace->addChunk(0, 0, 0, *testChunk);




    chunk* testChunk2 = wSpace->addChunk(0, 1, 0); //Add chunk by default to chunkSpace
    testChunk2->createFullChunk();


    //Set an externally loaded voxel
    testChunk2->setBlock(0, 0, 0, 4);


    //Test Model Parts
    testPart = new PartSpace();
    testPart->RegisterToLibrary("SpiderHead", "parts/SpiderHead.txt");

    PartSpace testPart2;
    testPart2.RegisterToLibrary("SpiderLeg", "parts/SpiderLeg.txt");
    testModel = new node3D();
    testModel->transform = glm::scale(testModel->transform, glm::vec3(1.f, 4.5f, 1.f));
    testModel2 = new node3D();
    testModel2->transform = glm::scale(testModel2->transform, glm::vec3(1.f / 16.f, 1.f / 16.f, 1.f / 16.f));

    
    //testModel2->transform = glm::rotate(testModel2->transform, 0.8f, glm::vec3(1.f, 1.f, 0.f));
    //testModel->transform = glm::translate(testModel->transform,glm::vec3(0.f,8.f,0.f));
    //PartSpace::SetBlock("Part", 0, 0, 0, 0);
    //PartSpace::SetBlock("Part", 1, 0, 0, 4);
    //PartSpace::SetBlock("Part", 0, 1, 0, 4);
    //PartSpace::SetBlock("Part", 0, 0, 1, 4);

    //Generate using noise chunks
    /*for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            for (int k = 0; k < 5; k++)
            {
                wSpace->generate(glm::ivec3( - i - 1, 1 - j, -k - 1));
            }
        }
    }*/
    //wSpace->generate(0, 0, -2);

    //Window Title Stringstream (Credit : https://stackoverflow.com/questions/18412120/displaying-fps-in-glfw-window-title)
    std::stringstream title;
    float titleUpdateTime = 0.f;
    bool loadCHunk = true;
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
        glm::ivec3 chunkCoord = (currentCamera->getPosition()) / 16.f;
        //chunkCoord.y -= 1;
        for (int x = -2; x < 3; x++)
        {
            for (int y = -2; y < 0; y++)
            {
                for (int z = -2; z < 3; z++)
                {
                    wSpace->generate(chunkCoord + glm::ivec3(x, y, z));
                }
            }
        }








        //Render here

        glClearColor(0.2f, 0.55f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        Render();




        //Call Events and Swap Buffers
        glfwSwapBuffers(window);

        //Update loaded regions (for saving/loading from file)
        //wSpace->updateLoadedRegions(currentCamera->getPosition(),8);
    }
    std::cout << "UPDATING FINISHED" << std::endl;
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
    currentScene->deltaTime = deltaTime;
    currentScene->tick();
    testModelModel.tick();
}
glm::mat4 t = glm::mat4(1.f);

void Render()
{

    t = glm::rotate(t, 0.3f * deltaTime, glm::vec3(0.f, 1.f, 0.f));
    currentScene->render(*currentCamera);
    testModelModel.render(*currentCamera);
    //PartSpace::RenderPart("Part", *testModel, *currentCamera);
    //PartSpace::RenderPartTransformed("SpiderLeg", *testModel2,t, *currentCamera);
}