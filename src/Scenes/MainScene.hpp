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

#include <Canis/ECS/Components/TransformComponent.hpp>
#include <Canis/ECS/Components/ColorComponent.hpp>
#include <Canis/ECS/Components/RectTransformComponent.hpp>
#include <Canis/ECS/Components/TextComponent.hpp>
#include <Canis/ECS/Components/MeshComponent.hpp>
#include <Canis/ECS/Components/SphereColliderComponent.hpp>
#include <Canis/ECS/Components/Sprite2DComponent.hpp>
#include <Canis/ECS/Components/DirectionalLightComponent.hpp>
#include <Canis/ECS/Components/SpotLightComponent.hpp>
#include <Canis/ECS/Components/PointLightComponent.hpp>

#include "../ECS/Systems/Boid3DSystem.hpp"

class MainScene : public Canis::Scene
{
    private:
        entt::registry entity_registry;

        entt::entity directionalLight, spotLight, pointLight0, pointLight1;
        entt::entity fpsText;

        Canis::Shader shader;
        Canis::Shader spriteShader;

        Canis::RenderHDRSystem *renderHDRSystem;
        Canis::RenderMeshSystem *renderMeshSystem;
        Canis::RenderSkyboxSystem *renderSkyboxSystem;
        Canis::RenderTextSystem *renderTextSystem;
        Canis::SpriteRenderer2DSystem *spriteRenderer2DSystem;

        Boid3DSystem *boid3DSystem;

        bool firstMouseMove = true;
        bool mouseLock = false;

        int cubeModelId = 0;
        int antonioFontId = 0;

        SDL_Thread* threadID = 0;

        float delta;

        Canis::GLTexture diffuseColorPaletteTexture = {};
        Canis::GLTexture specularColorPaletteTexture = {};
        Canis::GLTexture supperPupStudioLogoTexture = {};

        void LoadECS()
        {
            { // direction light
            directionalLight = entity_registry.create();
            entity_registry.emplace<Canis::TransformComponent>(directionalLight,
                true, // active
                glm::vec3(-5.0f, 10.0f, -5.0f), // position
                glm::vec3(-0.2f, -1.0f, -0.3f), // rotation
                glm::vec3(1, 1, 1) // scale
            );
            entity_registry.emplace<Canis::DirectionalLightComponent>(directionalLight,
                glm::vec3(0.05f, 0.05f, 0.05f), // ambient
                glm::vec3(0.8f, 0.8f, 0.8f), // diffuse
                glm::vec3(0.5f, 0.5f, 0.5f) // specular
            );
            }

            { // point light 0
            pointLight0 = entity_registry.create();
            entity_registry.emplace<Canis::TransformComponent>(pointLight0,
                true, // active
                glm::vec3(5.0f, 2.0f, 0.0f), // position
                glm::vec3(0.0f,0.0f,0.0f), // rotation
                glm::vec3(1, 1, 1) // scale
            );
            entity_registry.emplace<Canis::PointLightComponent>(pointLight0,
                1.0f,                           // constant
                0.09f,                          // linear
                0.032f,                         // quadratic
                glm::vec3(0.0f, 0.05f, 0.05f), // ambient
                glm::vec3(0.0f, 0.8f, 0.8f),    // diffuse
                glm::vec3(0.0f, 1.0f, 1.0f)    // specular
            );
            }

            { // point light 1
            pointLight1 = entity_registry.create();
            entity_registry.emplace<Canis::TransformComponent>(pointLight1,
                true, // active
                glm::vec3(-5.0f, 2.0f, 0.0f), // position
                glm::vec3(0.0f,0.0f,0.0f), // rotation
                glm::vec3(1, 1, 1) // scale
            );
            entity_registry.emplace<Canis::PointLightComponent>(pointLight1,
                1.0f,                           // constant
                0.09f,                          // linear
                0.032f,                         // quadratic
                glm::vec3(0.05f, 0.05f, 0.0f), // ambient
                glm::vec3(0.8f, 0.8f, 0.0f),    // diffuse
                glm::vec3(1.0f, 1.0f, 0.0f)    // specular
            );
            }

            { // spot light
            spotLight = entity_registry.create();
            entity_registry.emplace<Canis::TransformComponent>(spotLight,
                true, // active
                camera->Position, // position
                camera->Front, // rotation
                glm::vec3(1, 1, 1) // scale
            );
            float cutOff = glm::cos(glm::radians(12.5f));
            float outerCutOff = glm::cos(glm::radians(15.0f));
            entity_registry.emplace<Canis::SpotLightComponent>(spotLight,
                cutOff,                         // cutOff
                outerCutOff,                    // outerCutOff
                1.0f,                           // constant
                0.09f,                          // linear
                0.032f,                         // quadratic
                glm::vec3(0.05f, 0.05f, 0.0f),  // ambient
                glm::vec3(0.8f, 0.8f, 0.0f),    // diffuse
                glm::vec3(1.0f, 1.0f, 0.0f)     // specular
            );
            }

            { // target cube
            entt::entity target_cube_entity = entity_registry.create();
            entity_registry.emplace<Canis::TransformComponent>(target_cube_entity,
                true, // active
                glm::vec3(0.0f,5.0f,0.0f), // position
                glm::vec3(0.0f, 0.0f, 0.0f), // rotation
                glm::vec3(1.0f, 1.0f, 1.0f) // scale
            );
            entity_registry.emplace<Canis::ColorComponent>(target_cube_entity,
                glm::vec4(1.0f,0.0f,0.0f,1.0f)
            );
            entity_registry.emplace<Canis::MeshComponent>(target_cube_entity,
                cubeModelId,
                Canis::AssetManager::GetInstance().Get<Canis::ModelAsset>(cubeModelId)->GetVAO(),
                Canis::AssetManager::GetInstance().Get<Canis::ModelAsset>(cubeModelId)->GetSize(),
                true
            );
            entity_registry.emplace<Canis::SphereColliderComponent>(target_cube_entity,
                glm::vec3(0.0f),
                1.0f
            );
            }
            
            int bigNum = 30;
            int numCount = 0;
            for(int x = 0; x < bigNum; x++) {
                for(int y = 0; y < bigNum; y++) {
                    for(int z = 0; z < bigNum; z++) {
                        numCount++;
                        entt::entity boid_entity = entity_registry.create();
                        entity_registry.emplace<Canis::TransformComponent>(boid_entity,
                            true, // active
                            glm::vec3(25.0f + (x*1.5f), 0.5f + (y*1.5f), 0.0f + (z*1.5f)), // position
                            glm::vec3(0.0f, 0.0f, 0.0f), // rotation
                            glm::vec3(1, 1, 1) // scale
                        );
                        entity_registry.emplace<Canis::ColorComponent>(boid_entity,
                            glm::vec4(((rand() % 100 + 1)/100.0f),((rand() % 100 + 1)/100.0f),((rand() % 100 + 1)/100.0f),1.0f)
                        );
                        entity_registry.emplace<Canis::MeshComponent>(boid_entity,
                            cubeModelId,
                            Canis::AssetManager::GetInstance().Get<Canis::ModelAsset>(cubeModelId)->GetVAO(),
                            Canis::AssetManager::GetInstance().Get<Canis::ModelAsset>(cubeModelId)->GetSize(),
                            true
                        );
                        entity_registry.emplace<Canis::SphereColliderComponent>(boid_entity,
                            glm::vec3(0.0f),
                            1.0f
                        );
                        entity_registry.emplace<Boid3DComponent>(boid_entity);
                    }
                }
            }

            std::cout << numCount << std::endl;
            { // ground
            entt::entity ground_entity = entity_registry.create();
            entity_registry.emplace<Canis::TransformComponent>(ground_entity,
                false, // active
                glm::vec3(0.0f, -0.5f, 0.0f), // position
                glm::vec3(0.0f, 0.0f, 0.0f), // rotation
                glm::vec3(20.0f, 0.1f, 20.0f) // scale
            );
            entity_registry.emplace<Canis::ColorComponent>(ground_entity,
                glm::vec4(1.0f)
            );
            entity_registry.emplace<Canis::MeshComponent>(ground_entity,
                cubeModelId,
                Canis::AssetManager::GetInstance().Get<Canis::ModelAsset>(cubeModelId)->GetVAO(),
                Canis::AssetManager::GetInstance().Get<Canis::ModelAsset>(cubeModelId)->GetSize(),
                true
            );
            entity_registry.emplace<Canis::SphereColliderComponent>(ground_entity,
                glm::vec3(0.0f),
                1.0f
            );
            }

            { // fps text
            fpsText = entity_registry.create();
            entity_registry.emplace<Canis::RectTransformComponent>(fpsText,
                true, // active
                glm::vec2(25.0f, window->GetScreenHeight() - 65.0f), // position
                glm::vec2(0.0f,0.0f), // size
                glm::vec2(0.0f, 0.0f), // rotation
                1.0f, // scale
                0.0f // depth
            );
            entity_registry.emplace<Canis::ColorComponent>(fpsText,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) // #26854c
            );
            entity_registry.emplace<Canis::TextComponent>(fpsText,
                Canis::AssetManager::GetInstance().LoadText("assets/fonts/Antonio-Bold.ttf", 48),
                new std::string("FPS : 0") // text
            );
            }
        }

    public:
        MainScene(std::string _name) : Canis::Scene(_name) {}
        ~MainScene()
        {
            delete renderHDRSystem;
            delete renderSkyboxSystem;
            delete renderMeshSystem;
            delete renderTextSystem;
            delete spriteRenderer2DSystem;
            delete boid3DSystem;
        }
        
        void PreLoad()
        {
            Canis::Scene::PreLoad();

            // configure global opengl state
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_ALPHA);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
            // glEnable(GL_CULL_FACE);
            // build and compile our shader program
            shader.Compile("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
            shader.AddAttribute("aPos");
            shader.AddAttribute("aNormal");
            shader.AddAttribute("aTexcoords");
            shader.Link();

            spriteShader.Compile(
                "assets/shaders/sprite.vs",
                "assets/shaders/sprite.fs"
            );
            spriteShader.AddAttribute("vertexPosition");
            spriteShader.AddAttribute("vertexColor");
            spriteShader.AddAttribute("vertexUV");
            spriteShader.Link();

            // Load color palette
            diffuseColorPaletteTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                Canis::AssetManager::GetInstance().LoadTexture("assets/textures/palette/diffuse.png")
            )->GetTexture();
            specularColorPaletteTexture = Canis::AssetManager::GetInstance().Get<Canis::TextureAsset>(
                Canis::AssetManager::GetInstance().LoadTexture("assets/textures/palette/specular.png")
            )->GetTexture();

            // load model
            cubeModelId = Canis::AssetManager::GetInstance().LoadModel("assets/models/white_block.obj");

            // load font
            antonioFontId = Canis::AssetManager::GetInstance().LoadText("assets/fonts/Antonio-Bold.ttf", 48);

            renderHDRSystem = new Canis::RenderHDRSystem(window);
            renderSkyboxSystem = new Canis::RenderSkyboxSystem();
            renderMeshSystem = new Canis::RenderMeshSystem();
            renderTextSystem = new Canis::RenderTextSystem();
            spriteRenderer2DSystem = new Canis::SpriteRenderer2DSystem();
            boid3DSystem = new Boid3DSystem();

            renderHDRSystem->renderMeshSystem = renderMeshSystem;
            renderHDRSystem->renderSkyboxSystem = renderSkyboxSystem;

            renderSkyboxSystem->window = window;
            renderSkyboxSystem->camera = camera;
            renderSkyboxSystem->Init();

            renderTextSystem->camera = camera;
            renderTextSystem->window = window;
            renderTextSystem->Init();

            renderMeshSystem->shader = &shader;
            renderMeshSystem->camera = camera;
            renderMeshSystem->window = window;
            renderMeshSystem->diffuseColorPaletteTexture = &diffuseColorPaletteTexture;
            renderMeshSystem->specularColorPaletteTexture = &specularColorPaletteTexture;

            spriteRenderer2DSystem->window = window;
            spriteRenderer2DSystem->Init(Canis::GlyphSortType::TEXTURE, &spriteShader);

            boid3DSystem->targets.push_back(glm::vec3(0.0f,-40.0f,0.0f));
            boid3DSystem->targets.push_back(glm::vec3(50.0f,30.0f,0.0f));
            boid3DSystem->targets.push_back(glm::vec3(0.0f,30.0f,50.0f));
            boid3DSystem->targets.push_back(glm::vec3(-50.0f,30.0f,0.0f));
            boid3DSystem->targets.push_back(glm::vec3(0.0f,30.0f,-50.0f));
            boid3DSystem->targets.push_back(glm::vec3(0.0f,80.0f,0.0f));

            // Draw mode
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

       
        void Load()
        {            
            camera->Position = glm::vec3(20.0f,20.0f,-20.0f);
            camera->WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
            camera->Pitch = Canis::PITCH-35.0f;
            camera->Yaw = Canis::YAW+135.0f;
            camera->override_camera = false;
            camera->UpdateCameraVectors();
            mouseLock = false;
            window->MouseLock(mouseLock);

            LoadECS();
        }

        void UnLoad()
        {
            Canis::Log("Canis Clear");
            entity_registry.clear();
        }

        static int ThreadUpdate( void* a)
        {
            MainScene *mainScene = static_cast<MainScene*>(a);

            if (mainScene->delta > 0.0001f && mainScene->delta < 0.09f) {
                mainScene->boid3DSystem->UpdateComponents(mainScene->delta, mainScene->entity_registry);
            }

            return 0;
        }
        
        void Update()
        {
            delta = deltaTime;
            threadID = 0;
            threadID = SDL_CreateThread( ThreadUpdate, "ThreadUpdate", this);

            //if (deltaTime > 0.0001f && deltaTime < 0.09f) {
            //    boid3DSystem->UpdateComponents(deltaTime, entity_registry);
            //}

            //fpsText
            auto [rect, text] = entity_registry.get<Canis::RectTransformComponent, Canis::TextComponent>(fpsText);
            (*text.text) = "fps : " + std::to_string(int(window->fps));
        }

        void LateUpdate()
        {
            const Uint8 *keystate = SDL_GetKeyboardState(NULL);

            if (keystate[SDL_SCANCODE_W] && mouseLock)
            {
                camera->ProcessKeyboard(Canis::Camera_Movement::FORWARD, deltaTime);
            }

            if (keystate[SDL_SCANCODE_S] && mouseLock)
            {
                camera->ProcessKeyboard(Canis::Camera_Movement::BACKWARD, deltaTime);
            }

            if (keystate[SDL_SCANCODE_A] && mouseLock)
            {
                camera->ProcessKeyboard(Canis::Camera_Movement::LEFT, deltaTime);
            }

            if (keystate[SDL_SCANCODE_D] && mouseLock)
            {
                camera->ProcessKeyboard(Canis::Camera_Movement::RIGHT, deltaTime);
            }

            if (inputManager->justPressedKey(SDLK_ESCAPE))
            {
                mouseLock = !mouseLock;
                //camera->override_camera = !camera->override_camera;

                window->MouseLock(mouseLock);
            }

            if (inputManager->justPressedKey(SDLK_F5))
            {
                Canis::Log("Load Scene");
                ((Canis::SceneManager*)sceneManager)->Load("MainScene");
            }

            auto [transform, spotlight] = entity_registry.get<Canis::TransformComponent, Canis::SpotLightComponent>(spotLight);
            transform.position = camera->Position;
            transform.rotation = camera->Front;
        }

        void Draw()
        {
            if (threadID == 0)
                return;
            glDepthFunc(GL_LESS);
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!
            
            // render hdr will call these two
            renderSkyboxSystem->UpdateComponents(deltaTime, entity_registry);
            renderMeshSystem->UpdateComponents(deltaTime, entity_registry);
            
            //renderHDRSystem->UpdateComponents(deltaTime, entity_registry);
            renderTextSystem->UpdateComponents(deltaTime, entity_registry);
            spriteRenderer2DSystem->UpdateComponents(deltaTime, entity_registry);
            

            /*window->SetWindowName("Canis : Template | fps : " + std::to_string(int(window->fps))
            + " deltaTime : " + std::to_string(deltaTime)
            + " Enitity : " + std::to_string(entity_registry.size())
            + " Rendered : " + std::to_string(renderMeshSystem->entities_rendered));*/
            int threadReturnValue;
            SDL_WaitThread(threadID, &threadReturnValue);
        }

        void InputUpdate()
        {

        }

        
};