#pragma once
#include "node2D.h"
#include "Shader.h"
#include "Texture.h"
#include "ResourceManager.h"
struct SpriteRenderer
{
    Shader shader;
    unsigned int quadVAO;
    SpriteRenderer(Shader& shader)
    {
        this->shader = shader;
        this->initRenderData();
    }
    ~SpriteRenderer()
    {
        glDeleteVertexArrays(1, &this->quadVAO);
    }
    void DrawSprite(Texture& texture, glm::vec2 position,
        glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f,
        glm::vec3 color = glm::vec3(1.0f))
    {
        // Disable depth writing for transparent textures (so they don't overwrite behind geometry)
        //glDepthMask(GL_FALSE);
        // Enable blending for transparency
       // glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set up the shader and model matrix
        this->shader.Use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
        model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));

        this->shader.SetMatrix4("model", model);
        this->shader.SetVector3f("spriteColor", color);

        // Bind the texture and draw
        glActiveTexture(GL_TEXTURE0);
        texture.Bind();
        glBindVertexArray(this->quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Disable blending and re-enable depth writing for the next rendering pass
        //glDisable(GL_BLEND);
        //glDepthMask(GL_TRUE);  // Re-enable depth writing
    }
    void initRenderData()
    {
        unsigned int VBO;
        float vertices[] =
        {
            // pos      // tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,

            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &this->quadVAO);
        glGenBuffers(1, &VBO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(this->quadVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
    }
};

class Sprite :
    public node2D
{
public:
    virtual void render(camera& cam) override;
    void SetTexture(std::string texName);
    static bool UIEnabled;
    virtual ~Sprite() override;
    SpriteRenderer* renderer;
private:
    //Orthographic projection for flat look
    glm::mat4 projection = glm::ortho(0.0f, 1280.f,720.f, 0.0f, -1.0f, 1.0f);
    
    std::string textureName = "";
    Texture* tex;
};

