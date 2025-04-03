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
#include "Sprite.h"
#include "globals.h"
ModelLibrary modelLibrary;

std::vector<Sprite*> sprites;
std::vector<Sprite> spritesView;

// LUA Scripting
#include "LuaCPP/LuaCpp.hpp"
using namespace LuaCpp;
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;


// Meta-Game Consts
const std::string GAMENAME = "Voxel Test";
const std::string VERSION = "0.0.5";

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

void SetupUI();

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
    ResourceManager::LoadShader("sprite.vs", "sprite.ps", nullptr, "2D");
    //Add a new chunkSpace
    wSpace = new WorldSpace();

    //Create a new scene
    currentScene = new node();

    // Create a new camera
    currentCamera = new camera();

    //Create models subsection of tree
    Models::models = new node();



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
//bool keyPressed = false;
Model testModelModel;

const int UIBarStart = 1;
const int maxHotbar = 5;
const float spacing = 400.f;
const float spacingScale = 0.8f;
SpriteRenderer* renderer;
int currentSpriteView = 0;
int localHand = 0;

std::unordered_map<unsigned int, bool> keyPressed;
bool keySet = false;
void processInput(GLFWwindow* window)
{
    
    if (!keySet)
    {
        //Set keyPressed
        keySet = true;
        for (int i = 0; i < 350; i++)
        {
            keyPressed[i] = false;
        }

    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {

        glfwSetWindowShouldClose(window, true);
        std::cout << "EXITING" << std::endl;



    }


    //--TEMP--
    // Camera Input
    currentCamera->processInput(window, deltaTime);


    if (!keyPressed[GLFW_KEY_P] && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) // Temp to demonstrate updateGeom and chunkUpdates
    {

        if (blockPlaced)
        {
            Sprite::UIEnabled = false;


            blockPlaced = false;
        }
        else
        {
            Sprite::UIEnabled = true;

            blockPlaced = true;
        }
        keyPressed[GLFW_KEY_P] = true;
    }

    if (keyPressed[GLFW_KEY_P] && glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
    {
        keyPressed[GLFW_KEY_P] = false;
    }

    if (!keyPressed[GLFW_KEY_PERIOD] && glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
    {
        if (auto sz=BlockLibrary::idBlockLookup.size() - 1;sz > currentSpriteView + 1+maxHotbar)
        {
            currentSpriteView += 1;
            for (int i = currentSpriteView; i < currentSpriteView+maxHotbar; i++)
            {
                spritesView[i].setPosition(glm::vec2(lerp(spacing, 1280.f - spacing, (float)(i-currentSpriteView) / (float)(maxHotbar - 1)), 620.f));
                spritesView[i].renderer = renderer;
                *sprites[i - currentSpriteView+1] = spritesView[i];
                //*sprites[i - currentSpriteView]
            }

        }
        keyPressed[GLFW_KEY_PERIOD] = true;

    }
    if (keyPressed[GLFW_KEY_PERIOD] && glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_RELEASE)
    {
        keyPressed[GLFW_KEY_PERIOD] = false;
    }
    if (!keyPressed[GLFW_KEY_COMMA] && glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
    {
        if (currentSpriteView - 1 >=0)
        {
            currentSpriteView -= 1;
            for (int i = currentSpriteView; i < currentSpriteView + maxHotbar; i++)
            {
                spritesView[i].setPosition(glm::vec2(lerp(spacing, 1280.f - spacing, (float)(i - currentSpriteView) / (float)(maxHotbar - 1)), 620.f));
                spritesView[i].renderer = renderer;
                *sprites[i - currentSpriteView + 1] = spritesView[i];
                //*sprites[i - currentSpriteView]
            }

        }
        keyPressed[GLFW_KEY_COMMA] = true;

    }
    if (keyPressed[GLFW_KEY_COMMA] && glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_RELEASE)
    {
        keyPressed[GLFW_KEY_COMMA] = false;


    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        localHand = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        localHand = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        localHand = 2;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        localHand = 3;
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        localHand = 4;
    }

}

glm::ivec3 chunkCoord{};
int chunkGenDist = 2;
float totalChunkStall = 0.f;



void SetupUI()
{
    //Setup Sprite Renderer
    renderer = new SpriteRenderer(*ResourceManager::GetShader("2D"));

    //Bar Back
    Sprite* spr = new Sprite();
    spr->setScale(glm::vec2(6.5f, 4.5f));
    spr->setPosition(glm::vec2(spacing * 0.9f, 600.f)); //This works
    spr->SetTexture("textures/barBack.png"); //This line definitely works 
    sprites.push_back(spr);
    spr->renderer = renderer;

    //Setup Blocks
    
    for (uint16_t i = 0; i < BlockLibrary::idBlockLookup.size()-1; i++)
    {
        Sprite* spr = new Sprite();
        spr->setScale(glm::vec2(0.5f, 0.5f));
        spr->setPosition(glm::vec2(lerp(spacing,1280.f-spacing,(float)(i)/(float)(maxHotbar-1)), 620.f)); //This works
        spr->SetTexture(BlockLibrary::BlockTextures[BlockLibrary::idBlockLookup[i+UIBarStart]][0]); //This line definitely works 
        spritesView.push_back(*spr);
        if (i<maxHotbar)
            sprites.push_back(spr);
        spr->renderer = renderer;
    }

    //Setup Crosshair
    Sprite* spr2 = new Sprite();
    spr2->setPosition(glm::vec2((1280.f / 2.f) - 4.f, (720.f / 2.f) - 4.f));
    spr2->SetTexture("textures/Crosshair.png"); //This line definitely works 
    sprites.push_back(spr2);
    spr2->renderer = renderer;
}


void Update()
{



    //Select the triangle shader
    //unsigned int testTriVAO = genTestTriangle();
    glm::mat4 trans = glm::mat4(1.0f);

    //Set the shader uniforms for view and projection
    float totalCount = 0.f;
    
    /*bool chunkMade = false;
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
    testChunk->deleteBlock(0, 1, 6);*/




    // ---------------------------- //
    // --- Add objects to scene --- //
    // ---------------------------- //
    
    SetupUI();
    for (auto& spr : sprites)
    {
        currentScene->addChild(spr);
    }
    //currentScene->addChild(sprites[0]);
    currentScene->addChild(*currentCamera);
    currentScene->addChild(Models::models);
    
    
    //Add models to world
    for (auto& m : ModelLibrary::modelList)
    {
        Model* mod = (ModelLibrary::modelList[(m.first)]);
        Models::models->addChild(*mod);

    }
    


    //Add test chunk as a child
    currentScene->addChild(*wSpace);

    //Generate Chunk from ChunkSpace
    //wSpace->addChunk(0, 0, 0, *testChunk);




    //chunk* testChunk2 = wSpace->addChunk(0, 1, 0); //Add chunk by default to chunkSpace
    //testChunk2->createFullChunk();


    //Set an externally loaded voxel
    //testChunk2->setBlock(0, 0, 0, 4);

    modelLibrary.LoadParts();
    
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
        if (chunkCoord == glm::ivec3((currentCamera->getPosition()) / 16.f)/* && wSpace->getChunk(chunkCoord.x + chunkGenDist, chunkCoord.y + chunkGenDist / 2 - 1, chunkCoord.z + chunkGenDist)*/)
        {
            #ifdef NDEBUG //Only increase render distance in non debug builds
                totalChunkStall += deltaTime;
                if (totalChunkStall > log((chunkGenDist+1.f) * chunkGenDist)*chunkGenDist)
                {
                    chunkGenDist += 1;
                    chunkGenDist = std::min(chunkGenDist, 5);
                    //totalChunkStall = 0.f;
                }
            #endif
        }
        else
        {
            chunkGenDist = 2;
            totalChunkStall = 0.f;
        }
        //chunkCoord.y -= 1;
        for (int x = -chunkGenDist; x < chunkGenDist+1; x++)
        {
            for (int y = -chunkGenDist; y < chunkGenDist/2; y++)
            {
                for (int z = -chunkGenDist; z < chunkGenDist+1; z++)
                {
                    wSpace->generate(chunkCoord + glm::ivec3(x, y, z));
                }
            }
        }
        chunkCoord = (currentCamera->getPosition()) / 16.f;







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
    WorldSpace::currentHand = localHand + currentSpriteView+1;
    //Update the current scene
    currentScene->deltaTime = deltaTime;
    currentScene->tick();
    
}
glm::mat4 t = glm::mat4(1.f);

void Render()
{

    
    currentScene->render(*currentCamera);
    
}