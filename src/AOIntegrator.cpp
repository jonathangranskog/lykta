#include "Integrator.hpp"
#include "Sampling.hpp"

glm::vec3 Lykta::AOIntegrator::evaluate(const Lykta::Ray& ray, const std::shared_ptr<Lykta::Scene> scene) {
	Lykta::Hit hit;
	bool intersected = scene->intersect(ray, hit);
    
	if (!intersected) {
		return glm::vec3(0.f);
	}

	Lykta::Basis basis = Lykta::Basis(hit.normal);
	glm::vec3 out = basis.fromLocalSpace(Lykta::Sampling::cosineHemisphere(RND::next2D()));
	Ray occlusionRay = Lykta::Ray(hit.pos, out, glm::vec2(EPS, maxlen));
	bool shadowed = scene->shadowIntersect(occlusionRay);
	return glm::vec3((float)!shadowed);
}