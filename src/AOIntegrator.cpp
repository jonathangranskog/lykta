#include "Integrator.hpp"

glm::vec3 Lykta::AOIntegrator::evaluate(const Lykta::Ray& ray, const std::shared_ptr<Lykta::Scene> scene) {
    bool hit = scene->intersect(ray);
    return glm::vec3((float)hit);
}