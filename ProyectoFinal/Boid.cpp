// Boid.cpp
#include "Boid.h"
#include <cstdlib> // Para rand()

Boid::Boid(glm::vec3 startPos, int fishType) {
    position = startPos;
    type = fishType;
    // Velocidad inicial aleatoria
    float rx = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
    float rz = ((rand() % 100) / 100.0f) * 2.0f - 1.0f;
    velocity = glm::normalize(glm::vec3(rx, 0.0f, rz)) * MAX_SPEED;
    acceleration = glm::vec3(0.0f);
}

void Boid::applyForce(glm::vec3 force) {
    acceleration += force;
}

void Boid::update(float deltaTime, glm::vec3 tankMin, glm::vec3 tankMax) {
    // Limitar la fuerza aplicada (agilidad de giro)
    if (glm::length(acceleration) > MAX_FORCE) {
        acceleration = glm::normalize(acceleration) * MAX_FORCE;
    }

    velocity += acceleration * deltaTime;

    // Limitar la velocidad máxima
    if (glm::length(velocity) > MAX_SPEED) {
        velocity = glm::normalize(velocity) * MAX_SPEED;
    }

    position += velocity * deltaTime;
    acceleration = glm::vec3(0.0f); // Resetear aceleración para el próximo frame

    // Efecto Wrap-Around
    if (position.x > tankMax.x) position.x = tankMin.x;
    else if (position.x < tankMin.x) position.x = tankMax.x;

    if (position.y > tankMax.y) position.y = tankMin.y;
    else if (position.y < tankMin.y) position.y = tankMax.y;

    if (position.z > tankMax.z) position.z = tankMin.z;
    else if (position.z < tankMin.z) position.z = tankMax.z;
}