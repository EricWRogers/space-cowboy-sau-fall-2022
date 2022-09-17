#pragma once

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <math.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <Canis/Canis.hpp>
#include <Canis/Debug.hpp>
#include <Canis/Math.hpp>
#include <Canis/Time.hpp>
#include <Canis/Window.hpp>
#include <Canis/Shader.hpp>
#include <Canis/Camera.hpp>
#include <Canis/IOManager.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/Scene.hpp>
#include <Canis/SceneManager.hpp>
#include <Canis/AssetManager.hpp>
#include <Canis/Data/GLTexture.hpp>
#include <Canis/External/entt.hpp>
#include <Canis/GameHelper/AStar.hpp>

#include <Canis/ECS/Systems/RenderHDRSystem.hpp>
#include <Canis/ECS/Systems/RenderMeshSystem.hpp>
#include <Canis/ECS/Systems/RenderSkyboxSystem.hpp>
#include <Canis/ECS/Systems/RenderTextSystem.hpp>
#include <Canis/ECS/Systems/SpriteRenderer2DSystem.hpp>

#include <Canis/ECS/Components/ColorComponent.hpp>
#include <Canis/ECS/Components/RectTransformComponent.hpp>
#include <Canis/ECS/Components/TextComponent.hpp>
#include <Canis/ECS/Components/Sprite2DComponent.hpp>

class SplashScene : public Canis::Scene
{
private:
    entt::registry entity_registry;

    Canis::Shader shader;
    Canis::Shader spriteShader;

    Canis::SpriteRenderer2DSystem *spriteRenderer2DSystem;

    bool firstMouseMove = true;
    bool mouseLock = false;

    int cubeModelId = 0;
    int antonioFontId = 0;

    Canis::GLTexture diffuseColorPaletteTexture = {};
    Canis::GLTexture specularColorPaletteTexture = {};
    Canis::GLTexture logoScreenTexture = {};
    Canis::GLTexture supperPupStudioLogoTexture = {};

public:
    SplashScene(std::string _name) : Canis::Scene(_name) {}
    ~SplashScene()
    {
        delete spriteRenderer2DSystem;
    }

    void PreLoad()
    {
        Canis::Scene::PreLoad();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_ALPHA);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
        spriteShader.Compile(
            "assets/shaders/sprite.vs",
            "assets/shaders/sprite.fs"
        );
        spriteShader.AddAttribute("vertexPosition");
        spriteShader.AddAttribute("vertexColor");
        spriteShader.AddAttribute("vertexUV");
        spriteShader.Link();


        // Load texture
        diffuseColorPaletteTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                                                                           Canis::AssetManager::GetInstance().LoadTexture("assets/textures/palette/diffuse.png"))
                                         ->GetTexture();
        specularColorPaletteTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                                                                            Canis::AssetManager::GetInstance().LoadTexture("assets/textures/palette/specular.png"))
                                          ->GetTexture();
        logoScreenTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                                                                            Canis::AssetManager::GetInstance().LoadTexture("assets/textures/LogoScreen.png"))
                                          ->GetTexture();

        // load model
        cubeModelId = Canis::AssetManager::GetInstance().LoadModel("assets/models/white_block.obj");

        // load font
        antonioFontId = Canis::AssetManager::GetInstance().LoadText("assets/fonts/Antonio-Bold.ttf", 48);

        spriteRenderer2DSystem = new Canis::SpriteRenderer2DSystem();

        spriteRenderer2DSystem->window = window;
        spriteRenderer2DSystem->Init(Canis::GlyphSortType::TEXTURE, &spriteShader);
    }

    void Load()
    {
        camera->Position = glm::vec3(20.0f, 20.0f, -20.0f);
        camera->WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        camera->Pitch = Canis::PITCH - 35.0f;
        camera->Yaw = Canis::YAW + 135.0f;
        camera->override_camera = false;
        camera->UpdateCameraVectors();
        mouseLock = true;
        window->MouseLock(mouseLock);

        

        { // sprite test supperPupStudioLogoTexture
            entt::entity spriteEntity = entity_registry.create();
            entity_registry.emplace<Canis::RectTransformComponent>(spriteEntity,
                true, // active
                glm::vec2(0.0f, 0.0f), // position
                glm::vec2(window->GetScreenWidth(),window->GetScreenHeight()), // size
                glm::vec2(0.0f, 0.0f), // rotation
                1.0f, // scale
                1.0f // depth
            );
            entity_registry.emplace<Canis::ColorComponent>(spriteEntity,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
            );
            entity_registry.emplace<Canis::Sprite2DComponent>(spriteEntity,
                glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // uv
                logoScreenTexture // texture
            );
        }
    }

    void UnLoad()
    {
        Canis::Log("Canis Clear");
        entity_registry.clear();
    }

    void Update()
    {
    }

    void LateUpdate()
    {
        
    }

    void Draw()
    {
        glDepthFunc(GL_LESS);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(0);

        spriteRenderer2DSystem->UpdateComponents(deltaTime, entity_registry);     
    }

    void InputUpdate()
    {
        if (inputManager->justPressedKey(SDLK_ESCAPE))
        {
            mouseLock = !mouseLock;

            window->MouseLock(mouseLock);
        }

        if (inputManager->justPressedKey(SDLK_RETURN))
        {
            Canis::Log("Load Scene");
            ((Canis::SceneManager *)sceneManager)->Load("MainMenuScene");
        }
    }
};