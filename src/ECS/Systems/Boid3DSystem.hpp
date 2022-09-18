#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include <Canis/Math.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/External/entt.hpp>
#include <Canis/DataStucture/OctTree.hpp>

#include <Canis/ECS/Components/TransformComponent.hpp>
#include <Canis/ECS/Components/SphereColliderComponent.hpp>

#include "../Components/Boid3DComponent.hpp"

struct BoidTransformCombo
{
    Boid3DComponent boid;
    Canis::TransformComponent transform;
};

class Boid3DSystem
{
private:
public:
    std::vector<glm::vec3> targets = {};
    Canis::OctTree *octTree = new Canis::OctTree(glm::vec3(0.0f), 300.0f);
    std::vector<BoidTransformCombo> boidTransformCombos = {};

    float maxDistance = 10.0f;
    float minDistanceToChangeTarget = 2.0f;

    float maxSeparationDistance = 2.0f;
    float maxAlignmentDistance = 3.0f;
    float maxCohesionDistance = 3.0f;

    float seekWeight = 0.8f;
    float separationWeight = 3.0f;
    float alignmentWeight = 0.3f;
    float cohesionWeight = 0.3f;
    entt::registry *registry;
    float deltaTime;
    int boidSize = 0;

    entt::basic_view<entt::entity, entt::get_t<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>, entt::exclude_t<>, void> view;

    Boid3DSystem() {}

    void UpdateBoid(float deltaTime, Canis::TransformComponent &transform, Boid3DComponent &boid)
    {
        boid.velocity += (boid.acceleration * boid.speed) * deltaTime;

        // Canis::Log(std::to_string(glm::length(boid.velocity)));
        if (glm::length(boid.velocity) > boid.maxSpeed * deltaTime)
        {
            boid.velocity = glm::normalize(boid.velocity);
            boid.velocity *= boid.maxSpeed * deltaTime;
        }

        // boid.velocity *= boid.drag;

        Canis::MoveTransformPosition(transform, boid.velocity);
    }

    static int ThreadUpdate0(void *a)
    {
        Boid3DSystem *boidSystem = static_cast<Boid3DSystem *>(a);
        

        glm::vec3 seek = glm::vec3(0.0f);
        glm::vec3 alignment = glm::vec3(0.0f);
        glm::vec3 cohesion = glm::vec3(0.0f);
        glm::vec3 separation = glm::vec3(0.0f);
        float distance = 0.0f;
        float num_of_agents = 0.0f;
        float num_of_agents_c = 0.0f;
        glm::quat quat_look_at;
        glm::vec3 rot;
        glm::mat4 temp_transform;
        float distance_target = 0.0f;
        glm::vec3 target;

        // mainScene->boid3DSystem->UpdateComponents(mainScene->delta, mainScene->entity_registry);
        int low = 0;
        int high = 1000;
        int index = low;
         for (int i = low; i < high ;i++ )
        {
        
        //auto view = boidSystem->registry->view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        //for (auto [entity, transform, sphere, boid] : boidSystem->view.each())
        //{
            //if (index >= low && index < high)
            //{

                seek = glm::vec3(0.0f);
                alignment = glm::vec3(0.0f);
                cohesion = glm::vec3(0.0f);
                separation = glm::vec3(0.0f);
                distance = 0.0f;
                target = boidSystem->targets[boidSystem->boidTransformCombos[i].boid.index];

                if (boidSystem->minDistanceToChangeTarget >= glm::distance(boidSystem->boidTransformCombos[i].transform.position, target))
                {
                    boidSystem->boidTransformCombos[i].boid.index++;
                    if (boidSystem->boidTransformCombos[i].boid.index >= boidSystem->targets.size())
                    {
                        boidSystem->boidTransformCombos[i].boid.index = 0;
                    }
                    target = boidSystem->targets[boidSystem->boidTransformCombos[i].boid.index];
                }

                // seek
                seek = glm::normalize(target - boidSystem->boidTransformCombos[i].transform.position);

                // alignment
                num_of_agents = 0.0f;
                num_of_agents_c = 0.0f;
                std::vector<glm::vec3> points = {};
                if (boidSystem->octTree->PointsQuery(boidSystem->boidTransformCombos[i].transform.position, 3.0f, points))
                {
                    // Canis::Log(std::to_string(points.size()));
                    for (int i = 0; i < points.size(); i++)
                    {
                        if (boidSystem->boidTransformCombos[i].transform.position != points[i])
                        {
                            distance = glm::distance(boidSystem->boidTransformCombos[i].transform.position, points[i]);

                            if (boidSystem->maxCohesionDistance > distance)
                            {
                                num_of_agents_c++;
                                cohesion += points[i];

                                if (boidSystem->maxAlignmentDistance > distance)
                                {
                                    num_of_agents++;
                                    alignment += boidSystem->boidTransformCombos[i].transform.position + points[i];

                                    separation += (boidSystem->maxSeparationDistance > distance) ? boidSystem->boidTransformCombos[i].transform.position - points[i] : glm::vec3(0.0f);
                                }
                            }
                        }
                    }
                }

                alignment = (alignment == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(alignment / num_of_agents);

                // end of alignment

                // cohesion
                cohesion /= (num_of_agents_c + 0.0f);

                quat_look_at = Canis::RotationBetweenVectors(
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::normalize(cohesion - boidSystem->boidTransformCombos[i].transform.position));
                rot = glm::eulerAngles(quat_look_at);
                temp_transform = glm::mat4(1);
                temp_transform = glm::translate(temp_transform, boidSystem->boidTransformCombos[i].transform.position);
                temp_transform = glm::rotate(temp_transform, rot.x, glm::vec3(1, 0, 0));
                temp_transform = glm::rotate(temp_transform, rot.y, glm::vec3(0, 1, 0));
                temp_transform = glm::rotate(temp_transform, rot.z, glm::vec3(0, 0, 1));
                temp_transform = glm::scale(temp_transform, boidSystem->boidTransformCombos[i].transform.scale);

                cohesion = glm::normalize(temp_transform[3]);

                cohesion = glm::normalize(cohesion - boidSystem->boidTransformCombos[i].transform.position);

                // end of cohesion

                // separation
                separation = (separation == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(separation);

                // end of separation

                boidSystem->boidTransformCombos[i].boid.acceleration = seek * boidSystem->seekWeight + alignment * boidSystem->alignmentWeight + cohesion * boidSystem->cohesionWeight + separation * boidSystem->separationWeight;

                boidSystem->UpdateBoid(boidSystem->deltaTime, boidSystem->boidTransformCombos[i].transform, boidSystem->boidTransformCombos[i].boid);
            //}
            //index++;
        }

        return 0;
    }
    /*

    static int ThreadUpdate1(void *a)
    {
        Boid3DSystem *boidSystem = static_cast<Boid3DSystem *>(a);

        glm::vec3 seek = glm::vec3(0.0f);
        glm::vec3 alignment = glm::vec3(0.0f);
        glm::vec3 cohesion = glm::vec3(0.0f);
        glm::vec3 separation = glm::vec3(0.0f);
        float distance = 0.0f;
        float num_of_agents = 0.0f;
        float num_of_agents_c = 0.0f;
        glm::quat quat_look_at;
        glm::vec3 rot;
        glm::mat4 temp_transform;
        float distance_target = 0.0f;
        glm::vec3 target;

        // mainScene->boid3DSystem->UpdateComponents(mainScene->delta, mainScene->entity_registry);

        // for (int i = low; i < high ;i++ )
        //{
        int low = 1000;
        int high = boidSystem->boidSize;
        int index = low;
        //auto view = boidSystem->registry->view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : boidSystem->view.each())
        {
            if (index >= low && index < high)
            {

                seek = glm::vec3(0.0f);
                alignment = glm::vec3(0.0f);
                cohesion = glm::vec3(0.0f);
                separation = glm::vec3(0.0f);
                distance = 0.0f;
                target = boidSystem->targets[boid.index];

                if (boidSystem->minDistanceToChangeTarget >= glm::distance(transform.position, target))
                {
                    boid.index++;
                    if (boid.index >= boidSystem->targets.size())
                    {
                        boid.index = 0;
                    }
                    target = boidSystem->targets[boid.index];
                }

                // seek
                seek = glm::normalize(target - transform.position);

                // alignment
                num_of_agents = 0.0f;
                num_of_agents_c = 0.0f;
                std::vector<glm::vec3> points = {};
                if (boidSystem->octTree->PointsQuery(transform.position, 3.0f, points))
                {
                    // Canis::Log(std::to_string(points.size()));
                    for (int i = 0; i < points.size(); i++)
                    {
                        if (transform.position != points[i])
                        {
                            distance = glm::distance(transform.position, points[i]);

                            if (boidSystem->maxCohesionDistance > distance)
                            {
                                num_of_agents_c++;
                                cohesion += points[i];

                                if (boidSystem->maxAlignmentDistance > distance)
                                {
                                    num_of_agents++;
                                    alignment += transform.position + points[i];

                                    separation += (boidSystem->maxSeparationDistance > distance) ? transform.position - points[i] : glm::vec3(0.0f);
                                }
                            }
                        }
                    }
                }

                alignment = (alignment == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(alignment / num_of_agents);

                // end of alignment

                // cohesion
                cohesion /= (num_of_agents_c + 0.0f);

                quat_look_at = Canis::RotationBetweenVectors(
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::normalize(cohesion - transform.position));
                rot = glm::eulerAngles(quat_look_at);
                temp_transform = glm::mat4(1);
                temp_transform = glm::translate(temp_transform, transform.position);
                temp_transform = glm::rotate(temp_transform, rot.x, glm::vec3(1, 0, 0));
                temp_transform = glm::rotate(temp_transform, rot.y, glm::vec3(0, 1, 0));
                temp_transform = glm::rotate(temp_transform, rot.z, glm::vec3(0, 0, 1));
                temp_transform = glm::scale(temp_transform, transform.scale);

                cohesion = glm::normalize(temp_transform[3]);

                cohesion = glm::normalize(cohesion - transform.position);

                // end of cohesion

                // separation
                separation = (separation == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(separation);

                // end of separation

                boid.acceleration = seek * boidSystem->seekWeight + alignment * boidSystem->alignmentWeight + cohesion * boidSystem->cohesionWeight + separation * boidSystem->separationWeight;

                boidSystem->UpdateBoid(boidSystem->deltaTime, transform, boid);
            }
            index++;
        }

        return 0;
    }

    static int ThreadUpdate2(void *a)
    {
        Boid3DSystem *boidSystem = static_cast<Boid3DSystem *>(a);

        glm::vec3 seek = glm::vec3(0.0f);
        glm::vec3 alignment = glm::vec3(0.0f);
        glm::vec3 cohesion = glm::vec3(0.0f);
        glm::vec3 separation = glm::vec3(0.0f);
        float distance = 0.0f;
        float num_of_agents = 0.0f;
        float num_of_agents_c = 0.0f;
        glm::quat quat_look_at;
        glm::vec3 rot;
        glm::mat4 temp_transform;
        float distance_target = 0.0f;
        glm::vec3 target;

        // mainScene->boid3DSystem->UpdateComponents(mainScene->delta, mainScene->entity_registry);

        // for (int i = low; i < high ;i++ )
        //{
        int low = 1000;
        int high = 1500;
        int index = low;
        //auto view = boidSystem->registry->view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : boidSystem->view.each())
        {
            if (index >= low && index < high)
            {

                seek = glm::vec3(0.0f);
                alignment = glm::vec3(0.0f);
                cohesion = glm::vec3(0.0f);
                separation = glm::vec3(0.0f);
                distance = 0.0f;
                target = boidSystem->targets[boid.index];

                if (boidSystem->minDistanceToChangeTarget >= glm::distance(transform.position, target))
                {
                    boid.index++;
                    if (boid.index >= boidSystem->targets.size())
                    {
                        boid.index = 0;
                    }
                    target = boidSystem->targets[boid.index];
                }

                // seek
                seek = glm::normalize(target - transform.position);

                // alignment
                num_of_agents = 0.0f;
                num_of_agents_c = 0.0f;
                std::vector<glm::vec3> points = {};
                if (boidSystem->octTree->PointsQuery(transform.position, 3.0f, points))
                {
                    // Canis::Log(std::to_string(points.size()));
                    for (int i = 0; i < points.size(); i++)
                    {
                        if (transform.position != points[i])
                        {
                            distance = glm::distance(transform.position, points[i]);

                            if (boidSystem->maxCohesionDistance > distance)
                            {
                                num_of_agents_c++;
                                cohesion += points[i];

                                if (boidSystem->maxAlignmentDistance > distance)
                                {
                                    num_of_agents++;
                                    alignment += transform.position + points[i];

                                    separation += (boidSystem->maxSeparationDistance > distance) ? transform.position - points[i] : glm::vec3(0.0f);
                                }
                            }
                        }
                    }
                }

                alignment = (alignment == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(alignment / num_of_agents);

                // end of alignment

                // cohesion
                cohesion /= (num_of_agents_c + 0.0f);

                quat_look_at = Canis::RotationBetweenVectors(
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::normalize(cohesion - transform.position));
                rot = glm::eulerAngles(quat_look_at);
                temp_transform = glm::mat4(1);
                temp_transform = glm::translate(temp_transform, transform.position);
                temp_transform = glm::rotate(temp_transform, rot.x, glm::vec3(1, 0, 0));
                temp_transform = glm::rotate(temp_transform, rot.y, glm::vec3(0, 1, 0));
                temp_transform = glm::rotate(temp_transform, rot.z, glm::vec3(0, 0, 1));
                temp_transform = glm::scale(temp_transform, transform.scale);

                cohesion = glm::normalize(temp_transform[3]);

                cohesion = glm::normalize(cohesion - transform.position);

                // end of cohesion

                // separation
                separation = (separation == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(separation);

                // end of separation

                boid.acceleration = seek * boidSystem->seekWeight + alignment * boidSystem->alignmentWeight + cohesion * boidSystem->cohesionWeight + separation * boidSystem->separationWeight;

                boidSystem->UpdateBoid(boidSystem->deltaTime, transform, boid);
            }
            index++;
        }

        return 0;
    }

    static int ThreadUpdate3(void *a)
    {
        Boid3DSystem *boidSystem = static_cast<Boid3DSystem *>(a);

        glm::vec3 seek = glm::vec3(0.0f);
        glm::vec3 alignment = glm::vec3(0.0f);
        glm::vec3 cohesion = glm::vec3(0.0f);
        glm::vec3 separation = glm::vec3(0.0f);
        float distance = 0.0f;
        float num_of_agents = 0.0f;
        float num_of_agents_c = 0.0f;
        glm::quat quat_look_at;
        glm::vec3 rot;
        glm::mat4 temp_transform;
        float distance_target = 0.0f;
        glm::vec3 target;

        // mainScene->boid3DSystem->UpdateComponents(mainScene->delta, mainScene->entity_registry);

        // for (int i = low; i < high ;i++ )
        //{
        int low = 1500;
        int high = 2200;
        int index = low;
        //auto view = boidSystem->registry->view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : boidSystem->view.each())
        {
            if (index >= low && index < high)
            {

                seek = glm::vec3(0.0f);
                alignment = glm::vec3(0.0f);
                cohesion = glm::vec3(0.0f);
                separation = glm::vec3(0.0f);
                distance = 0.0f;
                target = boidSystem->targets[boid.index];

                if (boidSystem->minDistanceToChangeTarget >= glm::distance(transform.position, target))
                {
                    boid.index++;
                    if (boid.index >= boidSystem->targets.size())
                    {
                        boid.index = 0;
                    }
                    target = boidSystem->targets[boid.index];
                }

                // seek
                seek = glm::normalize(target - transform.position);

                // alignment
                num_of_agents = 0.0f;
                num_of_agents_c = 0.0f;
                std::vector<glm::vec3> points = {};
                if (boidSystem->octTree->PointsQuery(transform.position, 3.0f, points))
                {
                    // Canis::Log(std::to_string(points.size()));
                    for (int i = 0; i < points.size(); i++)
                    {
                        if (transform.position != points[i])
                        {
                            distance = glm::distance(transform.position, points[i]);

                            if (boidSystem->maxCohesionDistance > distance)
                            {
                                num_of_agents_c++;
                                cohesion += points[i];

                                if (boidSystem->maxAlignmentDistance > distance)
                                {
                                    num_of_agents++;
                                    alignment += transform.position + points[i];

                                    separation += (boidSystem->maxSeparationDistance > distance) ? transform.position - points[i] : glm::vec3(0.0f);
                                }
                            }
                        }
                    }
                }

                alignment = (alignment == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(alignment / num_of_agents);

                // end of alignment

                // cohesion
                cohesion /= (num_of_agents_c + 0.0f);

                quat_look_at = Canis::RotationBetweenVectors(
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::normalize(cohesion - transform.position));
                rot = glm::eulerAngles(quat_look_at);
                temp_transform = glm::mat4(1);
                temp_transform = glm::translate(temp_transform, transform.position);
                temp_transform = glm::rotate(temp_transform, rot.x, glm::vec3(1, 0, 0));
                temp_transform = glm::rotate(temp_transform, rot.y, glm::vec3(0, 1, 0));
                temp_transform = glm::rotate(temp_transform, rot.z, glm::vec3(0, 0, 1));
                temp_transform = glm::scale(temp_transform, transform.scale);

                cohesion = glm::normalize(temp_transform[3]);

                cohesion = glm::normalize(cohesion - transform.position);

                // end of cohesion

                // separation
                separation = (separation == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(separation);

                // end of separation

                boid.acceleration = seek * boidSystem->seekWeight + alignment * boidSystem->alignmentWeight + cohesion * boidSystem->cohesionWeight + separation * boidSystem->separationWeight;

                boidSystem->UpdateBoid(boidSystem->deltaTime, transform, boid);
            }
            index++;
        }

        return 0;
    }
    */
    void UpdateComponents(float dt, entt::registry &r)
    {
        delete octTree;
        octTree = new Canis::OctTree(glm::vec3(0.0f), 300.0f);
        //std::vector<BoidTransformCombo> boidTransformCombos = {};
        boidTransformCombos.clear();
        boidSize = 0;
        registry = &r;
        deltaTime = dt;

        view = r.view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : view.each())
        {
            octTree->AddPoint(transform.position);
            boidSize++;

            BoidTransformCombo boidTransformCombo;
            boidTransformCombo.boid = boid;
            boidTransformCombo.transform = transform;
            boidTransformCombos.push_back(boidTransformCombo);
        }

        SDL_Thread* threadID0 = 0;
        threadID0 = SDL_CreateThread( ThreadUpdate0, "ThreadUpdate0", this);
        //SDL_Thread* threadID1 = 0;
        //threadID1 = SDL_CreateThread( ThreadUpdate1, "ThreadUpdate1", this);
        /*SDL_Thread* threadID2 = 0;
        threadID2 = SDL_CreateThread( ThreadUpdate2, "ThreadUpdate2", this);
        SDL_Thread* threadID3 = 0;
        threadID3 = SDL_CreateThread( ThreadUpdate3, "ThreadUpdate3", this);*/



        int threadReturnValue;
        SDL_WaitThread(threadID0, &threadReturnValue);
        //SDL_WaitThread(threadID1, NULL);
        /*int threadReturnValue2;
        SDL_WaitThread(threadID2, &threadReturnValue2);
        int threadReturnValue3;
        SDL_WaitThread(threadID3, &threadReturnValue3);*/

        int index = 0;
        for (auto [entity, transform, sphere, boid] : view.each())
        {
            boid = boidTransformCombos[index].boid;
            transform = boidTransformCombos[index].transform;
            index++;
        }
    }
};

/*quat_look_at = Canis::RotationBetweenVectors(
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::normalize((transform.position * glm::normalize(boid.acceleration)) - transform.position)
);
rot = glm::eulerAngles(quat_look_at);
temp_transform = glm::mat4(1);
temp_transform = glm::translate(temp_transform, transform.position);
temp_transform = glm::rotate(temp_transform, rot.x, glm::vec3(1, 0, 0));
temp_transform = glm::rotate(temp_transform, rot.y, glm::vec3(0, 1, 0));
temp_transform = glm::rotate(temp_transform, rot.z, glm::vec3(0, 0, 1));
temp_transform = glm::scale(temp_transform, transform.scale);

transform.modelMatrix = temp_transform;
transform.position = transform.modelMatrix[3];

Canis::RotateTransformRotation(transform, glm::normalize(transform.position)*atan2(boid.acceleration.x, boid.acceleration.z));
*/