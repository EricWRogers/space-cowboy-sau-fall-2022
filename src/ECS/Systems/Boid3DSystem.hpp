#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include <Canis/Math.hpp>
#include <Canis/InputManager.hpp>
#include <Canis/External/entt.hpp>

#include <Canis/ECS/Components/TransformComponent.hpp>
#include <Canis/ECS/Components/SphereColliderComponent.hpp>

#include "../Components/Boid3DComponent.hpp"

class Boid3DSystem
{
private:
    

public:
    float maxDistance = 10.0f;
    float minDistanceToChangeTarget = 2.0f;
    glm::vec3 target;

    float maxSeparationDistance = 2.0f;
    float maxAlignmentDistance = 3.0f;
    float maxCohesionDistance = 3.0f;

    float seekWeight = 0.6f;
    float separationWeight = 2.0f;
    float alignmentWeight = 0.3f;
    float cohesionWeight = 0.3f;

    std::vector<glm::vec3> targets = {};

    Boid3DSystem() {}

    void UpdateBoid(float deltaTime, Canis::TransformComponent &transform, Boid3DComponent &boid)
    {
        boid.velocity += (boid.acceleration * boid.speed) * deltaTime;

        //Canis::Log(std::to_string(glm::length(boid.velocity)));
        if (glm::length(boid.velocity) > boid.maxSpeed * deltaTime) {
            boid.velocity = glm::normalize(boid.velocity);
            boid.velocity *= boid.maxSpeed * deltaTime;
            Canis::Log("vel clamp");
        }

        //boid.velocity *= boid.drag;

        Canis::MoveTransformPosition(transform, boid.velocity);
    }

    void UpdateComponents(float deltaTime, entt::registry &registry)
    {
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

        auto view = registry.view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : view.each())
        {
            seek = glm::vec3(0.0f);
            alignment = glm::vec3(0.0f);
            cohesion = glm::vec3(0.0f);
            separation = glm::vec3(0.0f);
            distance = 0.0f;
            target = targets[boid.index];

            
            if (minDistanceToChangeTarget >= glm::distance(transform.position, target)) {
                boid.index++;
                if (boid.index >= targets.size()) {
                    boid.index = 0;
                }
                target = targets[boid.index];
                Canis::Log("Change Target");
            }

            // seek
            seek = glm::normalize(target - transform.position);

            // alignment
            num_of_agents = 0.0f;
            num_of_agents_c = 0.0f;

            for (auto [n_entity, n_transform, n_sphere, n_boid] : view.each())
            {
                if (n_entity != entity)
                {
                    distance = glm::distance(transform.position, n_transform.position);

                    /*cohesion += (maxCohesionDistance > distance) ? n_transform.position : 0.0f;
                    num_of_agents_c += (maxCohesionDistance > distance) ? 1 : 0;

                    alignment += (maxAlignmentDistance > distance) ? transform.position + n_transform.position : 0.0f;
                    num_of_agents += (maxAlignmentDistance > distance) ? 1 : 0;

                    separation += (maxSeparationDistance > distance) ? transform.position - n_transform.position : 0.0f;*/



                    if (maxCohesionDistance > distance)
                    {
                        num_of_agents_c++;
                        cohesion += n_transform.position;

                        if (maxAlignmentDistance > distance)
                        {
                            num_of_agents++;
                            alignment += transform.position + n_transform.position;

                            separation += (maxSeparationDistance > distance) ? transform.position - n_transform.position : glm::vec3(0.0f);
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

            UpdateBoid(deltaTime, transform, boid);
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