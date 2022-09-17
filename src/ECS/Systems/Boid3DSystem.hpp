#pragma once
#include <glm/glm.hpp>
#include <vector>

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

    Boid3DSystem() {}

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
            seek = glm::vec3(0.0f);

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
        }
    }
};