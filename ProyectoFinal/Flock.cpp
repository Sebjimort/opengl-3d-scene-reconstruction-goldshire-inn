// Flock.cpp
#include "Flock.h"
#include <cstdlib>

Flock::Flock(glm::vec3 minBoundary, glm::vec3 maxBoundary) {
    tankMin = minBoundary;
    tankMax = maxBoundary;
}

void Flock::addSchool(int numBoids, int type) {
    for (int i = 0; i < numBoids; i++) {
        float randX = tankMin.x + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (tankMax.x - tankMin.x)));
        float randY = tankMin.y + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (tankMax.y - tankMin.y)));
        float randZ = tankMin.z + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (tankMax.z - tankMin.z)));

        boids.push_back(Boid(glm::vec3(randX, randY, randZ), type));
    }
}

void Flock::updateFlock(float deltaTime) {
    for (auto& boid : boids) {
        glm::vec3 alignment(0.0f);
        glm::vec3 cohesion(0.0f);
        glm::vec3 separation(0.0f);
        int total = 0;
        int sepTotal = 0;

        for (auto& other : boids) {
            if (&boid != &other) {
                float d = glm::distance(boid.position, other.position);
                glm::vec3 diffToOther = other.position - boid.position;
                // SEPARACIÓN: Evitan chocar contra CUALQUIER pez (sin importar la especie)
                if (d > 0.001f && d < SEPARATION_RANGE) {
                    glm::vec3 diff = boid.position - other.position;
                    separation += glm::normalize(diff) / d;
                    sepTotal++;
                }

                // COHESIÓN Y ALINEACIÓN: Solo se agrupan con su MISMA especie
                if (boid.type == other.type) {
                    if (d > 0.001f && d < VISUAL_RANGE) {
                        // Calcular si el otro pez está frente a él o a sus lados (no atrás)
                        // Usamos Producto Punto. Si es > -0.5, está en su campo de visión (~240 grados)
                        float angle = glm::dot(glm::normalize(boid.velocity), glm::normalize(diffToOther));
                        if (angle > -0.5f) {
                            alignment += other.velocity;
                            cohesion += other.position;
                            total++;
                        }
                    }
                }

            }
        }

        if (total > 0) {
            alignment /= (float)total;
            alignment = glm::normalize(alignment) * MAX_SPEED - boid.velocity;

            cohesion /= (float)total;
            cohesion = glm::normalize(cohesion - boid.position) * MAX_SPEED - boid.velocity;
        }

        if (sepTotal > 0) {
            separation /= (float)sepTotal;
            separation = glm::normalize(separation) * MAX_SPEED - boid.velocity;
        }

        // --- Fuerza Wander (Caos biológico) ---
        // Generamos un vector pequeño y aleatorio que cambia ligeramente su rumbo
        glm::vec3 wanderForce(
            ((rand() % 100) / 100.0f) * 2.0f - 1.0f,
            ((rand() % 100) / 100.0f) * 2.0f - 1.0f,
            ((rand() % 100) / 100.0f) * 2.0f - 1.0f
        );
        wanderForce = glm::normalize(wanderForce) * 0.5f; // Es una fuerza suave

        // Aplicar todas las fuerzas al boid

        glm::vec3 flockingForce = alignment * 1.5f + cohesion * 3.0f + separation * 3.0f + wanderForce * 2.0f;

        boid.applyForce(flockingForce);
    }

    // Actualizar la física de cada boid
    for (auto& boid : boids) {
        boid.update(deltaTime, tankMin, tankMax);
    }
}