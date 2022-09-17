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

class MainMenuScene : public Canis::Scene
{
private:
    entt::registry entity_registry;

    entt::entity directionalLight, spotLight, pointLight0, pointLight1;

    Canis::Shader shader;
    Canis::Shader spriteShader;

    Canis::RenderTextSystem *renderTextSystem;
    Canis::SpriteRenderer2DSystem *spriteRenderer2DSystem;

    bool firstMouseMove = true;
    bool mouseLock = false;

    int cubeModelId = 0;
    int antonioFontId = 0;

    Canis::GLTexture diffuseColorPaletteTexture = {};
    Canis::GLTexture specularColorPaletteTexture = {};
    Canis::GLTexture supperPupStudioLogoTexture = {};

public:
    MainMenuScene(std::string _name) : Canis::Scene(_name) {}
    ~MainMenuScene()
    {
        delete renderTextSystem;
        delete spriteRenderer2DSystem;
    }

    void PreLoad()
    {
        Canis::Scene::PreLoad();
        

        // Load color palette
        diffuseColorPaletteTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                                                                           Canis::AssetManager::GetInstance().LoadTexture("assets/textures/palette/diffuse.png"))
                                         ->GetTexture();
        specularColorPaletteTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                                                                            Canis::AssetManager::GetInstance().LoadTexture("assets/textures/palette/specular.png"))
                                          ->GetTexture();

        // load model
        cubeModelId = Canis::AssetManager::GetInstance().LoadModel("assets/models/white_block.obj");

        // load font
        antonioFontId = Canis::AssetManager::GetInstance().LoadText("assets/fonts/Antonio-Bold.ttf", 48);

        renderTextSystem = new Canis::RenderTextSystem();
        spriteRenderer2DSystem = new Canis::SpriteRenderer2DSystem();

        renderTextSystem->camera = camera;
        renderTextSystem->window = window;
        renderTextSystem->Init();

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

        { // text
            entt::entity text = entity_registry.create();
            entity_registry.emplace<Canis::RectTransformComponent>(text,
                                                                   true,                                                // active
                                                                   glm::vec2(25.0f, window->GetScreenHeight() - 65.0f), // position
                                                                   glm::vec2(0.0f, 0.0f),                               // size
                                                                   glm::vec2(0.0f, 0.0f),                               // rotation
                                                                   1.0f,                                                // scale
                                                                   0.0f                                                 // depth
            );
            entity_registry.emplace<Canis::ColorComponent>(text,
                                                           glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) // #26854c
            );
            entity_registry.emplace<Canis::TextComponent>(text,
                                                          Canis::AssetManager::GetInstance().LoadText("assets/fonts/Antonio-Bold.ttf", 48),
                                                          new std::string("Main Menu Scene") // text
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

        renderTextSystem->UpdateComponents(deltaTime, entity_registry);
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
            ((Canis::SceneManager *)sceneManager)->Load("MainScene");
        }
    }
};