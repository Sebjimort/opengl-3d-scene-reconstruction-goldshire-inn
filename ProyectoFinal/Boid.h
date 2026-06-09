// Boid.h
#ifndef BOID_H
#define BOID_H

#include <glm/glm.hpp>

// Parámetros ajustables para el comportamiento
const float VISUAL_RANGE = 1.2f;
const float SEPARATION_RANGE = 0.2f;
const float MAX_SPEED = 1.5f;
const float MAX_FORCE = 6.0f;

class Boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    int type;

    Boid(glm::vec3 startPos, int fishType);
    void update(float deltaTime, glm::vec3 tankMin, glm::vec3 tankMax);
    void applyForce(glm::vec3 force);
};

#endif