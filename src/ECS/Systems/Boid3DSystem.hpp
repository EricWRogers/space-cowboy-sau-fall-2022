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
    glm::vec3 target;

    float maxSeparationDistance = 0.5f;
    float maxAlignmentDistance = 3.0f;
    float maxCohesionDistance = 2.0f;

    float seekWeight = 0.15f;
    float separationWeight = 1.0f;
    float alignmentWeight = 0.3f;
    float cohesionWeight = 0.1f;

    Boid3DSystem() {}

    void UpdateBoid(float deltaTime, Canis::TransformComponent &transform, Boid3DComponent &boid)
    {
        boid.velocity += (boid.acceleration * boid.speed) * deltaTime;

        if ((boid.velocity.x*boid.velocity.x+boid.velocity.y*boid.velocity.y+boid.velocity.z*boid.velocity.z) > boid.maxSpeed) {
            boid.velocity = glm::normalize(boid.velocity);
            boid.velocity *= boid.maxSpeed;
        }

        boid.velocity *= boid.drag;

        Canis::MoveTransformPosition(transform, boid.velocity);
    }

    void UpdateComponents(float deltaTime, entt::registry &registry)
    {
        auto view = registry.view<Canis::TransformComponent, Canis::SphereColliderComponent, Boid3DComponent>();
        for (auto [entity, transform, sphere, boid] : view.each())
        {
            glm::vec3 seek = glm::vec3(0.0f);
            glm::vec3 alignment = glm::vec3(0.0f);
            glm::vec3 cohesion = glm::vec3(0.0f);
            glm::vec3 separation = glm::vec3(0.0f);

            // seek
            // agent.GlobalTransform.origin.DirectionTo(target);
            glm::quat quat_look_at = Canis::RotationBetweenVectors(
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::normalize(target - transform.position)
            );
            glm::vec3 rot = glm::eulerAngles(quat_look_at);
            glm::mat4 temp_transform = glm::mat4(1);
            temp_transform = glm::translate(temp_transform, transform.position);
            temp_transform = glm::rotate(temp_transform, rot.x, glm::vec3(1, 0, 0));
            temp_transform = glm::rotate(temp_transform, rot.y, glm::vec3(0, 1, 0));
            temp_transform = glm::rotate(temp_transform, rot.z, glm::vec3(0, 0, 1));
            temp_transform = glm::scale(temp_transform, transform.scale);

            seek = glm::normalize(temp_transform[3]);

            seek = glm::normalize(target - transform.position);

            // alignment
            float num_of_agents = 0.0f;

            for (auto [n_entity, n_transform, n_sphere, n_boid] : view.each())
            {
                if (n_entity != entity && maxDistance > glm::distance(transform.position, n_transform.position))
                {
                    num_of_agents++;
                    alignment += transform.position + n_transform.position;
                }
            }

            alignment = (alignment == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(alignment / num_of_agents);


            // end of alignment

            // cohesion

            for (auto [n_entity, n_transform, n_sphere, n_boid] : view.each())
            {
                if (n_entity != entity && maxDistance > glm::distance(transform.position, n_transform.position))
                {
                    cohesion += n_transform.position;
                }
            }

            cohesion /= (num_of_agents+0.0f);

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

            for (auto [n_entity, n_transform, n_sphere, n_boid] : view.each())
            {
                if (n_entity != entity && maxDistance > glm::distance(transform.position, n_transform.position))
                {
                    separation += transform.position - n_transform.position;
                }
            }

            separation = (separation == glm::vec3(0.0f)) ? glm::vec3(0.0f) : glm::normalize(separation);

            // end of separation

            /*
            boid.Acceleration = seek * SeekWeight + alignment * AlignmentWeight + cohesion * CohesionWeight + separation * SeparationWeight;

            boid.Rotate(boid.GlobalTransform.origin.Normalized(), Mathf.Deg2Rad(Mathf.Atan2(boid.Acceleration.x, boid.Acceleration.z)));

            boid.UpdateBoid(delta);
            */

            boid.acceleration = seek * seekWeight + alignment * alignmentWeight + cohesion * cohesionWeight + separation * separationWeight;

            quat_look_at = Canis::RotationBetweenVectors(
                glm::vec3(0.0f, 0.0f, 1.0f),
                glm::normalize(target - transform.position)
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

            UpdateBoid(deltaTime, transform, boid);
        }
    }
};