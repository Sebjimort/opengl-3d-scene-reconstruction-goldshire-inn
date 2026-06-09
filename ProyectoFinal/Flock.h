// Flock.h
#ifndef FLOCK_H
#define FLOCK_H

#include <vector>
#include "Boid.h"

class Flock {
public:
    std::vector<Boid> boids;
    glm::vec3 tankMin;
    glm::vec3 tankMax;

    Flock(glm::vec3 minBoundary, glm::vec3 maxBoundary);
    void addSchool(int numBoids, int type);

    void updateFlock(float deltaTime);
};

#endif