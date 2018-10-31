#include "Integrator.hpp"
#include "warp.h"

glm::vec3 Lykta::BSDFIntegrator::evaluate(const Lykta::Ray& ray, const std::shared_ptr<Lykta::Scene> scene, Lykta::RandomSampler* sampler) {
	glm::vec3 result = glm::vec3(0.f);
	glm::vec3 throughput = glm::vec3(1.f);
	Lykta::Ray r = ray;

	while (true) {
		Lykta::Hit hit;
		if (!scene->intersect(r, hit)) {
			break;
		}

		const Lykta::SurfaceMaterial& material = scene->getMaterial(hit.material);

		if (Lykta::maxComponent(material.emissiveColor) > 0.f) {
			result += throughput * material.emissiveColor;
		}

		// RR
		float s = sampler->next();
		float success = fminf(0.75f, Lykta::luminance(throughput));
		if (s < (1 - success)) break;
		throughput /= success;

		// Sample BSDF
		Lykta::Basis basis = Lykta::Basis(hit.normal);
		Lykta::SurfaceInteraction si;
		si.uv = hit.texcoord;
		si.pos = hit.pos;
		si.wi = basis.toLocalSpace(-r.d);
		glm::vec3 color = Lykta::sampleSurfaceMaterial(sampler->next2D(), material, si);
		glm::vec3 out = basis.fromLocalSpace(si.wo);

		throughput *= color;
		r = Lykta::Ray(hit.pos, out);
	}

	return result;
}