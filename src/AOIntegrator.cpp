#include "Integrator.hpp"
#include "warp.h"

glm::vec3 Lykta::AOIntegrator::evaluate(const Lykta::Ray& ray, const std::shared_ptr<Lykta::Scene> scene, Lykta::RandomSampler* sampler) {
	Lykta::Hit hit;
	bool intersected = scene->intersect(ray, hit);
    
	if (!intersected) {
		return glm::vec3(0.f);
	}

	Lykta::Basis basis = Lykta::Basis(hit.normal);
	glm::vec3 out = basis.fromLocalSpace(Lykta::squareToCosineHemisphere(sampler->next2D()));
	Ray occlusionRay = Lykta::Ray(hit.pos, out, glm::vec2(EPS, maxlen));
	bool shadowed = scene->shadowIntersect(occlusionRay);
	return glm::vec3((float)!shadowed);
}