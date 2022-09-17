#pragma once
#include <glm/glm.hpp>

struct Boid3DComponent
{
    float drag = 0.9f;
    float maxSpeed = 10.0f;
    float speed = 5.0f;
    float turnRate = 4.0f;
    int index = -1;
    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 acceleration = glm::vec3(0);
};
