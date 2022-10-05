#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <execution>

#include <Canis/Math.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/External/entt.hpp>
#include <Canis/DataStucture/OctTree.hpp>

#include <Canis/ECS/Components/TransformComponent.hpp>
#include <Canis/ECS/Components/SphereColliderComponent.hpp>

#include "../Components/Boid3DComponent.hpp"

class Boid3DSystem
{
private:
    

public:
    float maxDistance;
    float minDistanceToChangeTarget;
    float maxSeparationDistance;
    float maxAlignmentDistance;
    float maxCohesionDistance;
    float seekWeight;
    float separationWeight;
    float alignmentWeight;
    float cohesionWeight;
    float dt;
    entt::registry *reg;
    std::vector<glm::vec3> targets;
    Canis::OctTree *octTree;

    Boid3DSystem() {
        maxDistance = 10.0f;
        minDistanceToChangeTarget = 2.0f;
        maxSeparationDistance = 2.0f;
        maxAlignmentDistance = 3.0f;
        maxCohesionDistance = 3.0f;
        seekWeight = 0.8f;
        separationWeight = 3.0f;
        alignmentWeight = 0.3f;
        cohesionWeight = 0.3f;
        dt = 0.0f;
        reg = nullptr;
        targets = {};
        octTree = new Canis::OctTree(glm::vec3(0.0f),300.0f);
    }

    static void UpdateBoid(float deltaTime, Canis::TransformComponent &transform, Boid3DComponent &boid)
    {
        boid.velocity += (boid.acceleration * boid.speed) * deltaTime;

        //Canis::Log(std::to_string(glm::length(boid.velocity)));
        if (glm::length(boid.velocity) > boid.maxSpeed * deltaTime) {
            boid.velocity = glm::normalize(boid.velocity);
            boid.velocity *= boid.maxSpeed * deltaTime;
        }

        //boid.velocity *= boid.drag;

        Canis::MoveTransformPosition(transform, boid.velocity);
    }

    void UpdateComponents(float deltaTime, entt::registry &registry)
    {
        delete octTree;
        octTree = new Canis::OctTree(glm::vec3(0.0f),300.0f);
        dt = deltaTime;
        reg = &registry;
        

        auto view = registry.view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : view.each())
        {
            octTree->AddPoint(transform.position);
        }
        std::for_each(std::execution::par_unseq, view.begin(), view.end(), [&view, this](auto entity) {
        //for (auto [entity, transform, sphere, boid] : view.each())
            auto [transform, sphere, boid] = reg->get<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>(entity);
            glm::vec3 seek = glm::vec3(0.0f);
            glm::vec3 alignment = glm::vec3(0.0f);
            glm::vec3 cohesion = glm::vec3(0.0f);
            glm::vec3 separation = glm::vec3(0.0f);
            float distance = 0.0f;
            glm::vec3 target = targets[boid.index];
            glm::quat quat_look_at;
            glm::vec3 rot;
            glm::mat4 temp_transform;
            float distance_target = 0.0f;

            
            if (minDistanceToChangeTarget >= glm::distance(transform.position, target)) {
                boid.index++;
                if (boid.index >= targets.size()) {
                    boid.index = 0;
                }
                target = targets[boid.index];
            }

            // seek
            seek = glm::normalize(target - transform.position);

            // alignment
            float num_of_agents = 0.0f;
            float num_of_agents_c = 0.0f;
            std::vector<glm::vec3> points = {};
            if(octTree->PointsQuery(transform.position, 3.0f, points))
            {
                //Canis::Log(std::to_string(points.size()));
                for(int i = 0; i < points.size(); i++)
                {
                    if (transform.position != points[i])
                    {
                        distance = glm::distance(transform.position, points[i]);

                        if (maxCohesionDistance > distance)
                        {
                            num_of_agents_c++;
                            cohesion += points[i];

                            if (maxAlignmentDistance > distance)
                            {
                                num_of_agents++;
                                alignment += transform.position + points[i];

                                separation += (maxSeparationDistance > distance) ? transform.position - points[i] : glm::vec3(0.0f);
                            }

                        }
                    }
                }
            }

            alignment = (alignment == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(alignment / num_of_agents);


            // end of alignment

            // cohesion
            cohesion /= (num_of_agents_c+0.0f);

            quat_look_at = Canis::RotationBetweenVectors(
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::normalize(cohesion - transform.position)
            );
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

            boid.acceleration = seek * seekWeight + alignment * alignmentWeight + cohesion * cohesionWeight + separation * separationWeight;

            UpdateBoid(dt, transform, boid);
        });
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