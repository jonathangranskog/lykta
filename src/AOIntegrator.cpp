#include "Integrator.hpp"

glm::vec3 Lykta::AOIntegrator::evaluate(const Lykta::Ray& ray, const std::shared_ptr<Lykta::Scene> scene) {
	Lykta::Hit hit;
	bool intersected = scene->intersect(ray, hit);
    
	if (!intersected) {
		return glm::vec3(0.f);
	}

	return hit.normal;
}