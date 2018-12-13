#include "Integrator.hpp"
#include "Emitter.hpp"

using namespace Lykta;

glm::vec3 BSDFIntegrator::evaluate(const Ray& ray, const std::shared_ptr<Scene> scene) {
	glm::vec3 result = glm::vec3(0.f);
	glm::vec3 throughput = glm::vec3(1.f);
	Ray r = ray;
	unsigned bounces = 0;

	while (true) {
		Lykta::Hit hit;
		if (!scene->intersect(r, hit)) {
			EmitterPtr environmentMap = scene->getEnvironment();
			if (environmentMap) {
				EmitterInteraction ei;
				ei.direction = r.d;
				result += throughput * environmentMap->eval(ei);
			}
			break;
		}

		const MaterialPtr material = scene->getMaterial(hit.geomID);

		if (maxComponent(material->getEmission()) > 0.f) {
			result += throughput * material->getEmission();
		}

		// RR
		float s = RND::next1D();
		float success = fminf(0.75f, luminance(throughput));
		if (s < (1 - success)) break;
		throughput /= success;

		// Sample BSDF
		Basis basis = Basis(hit.normal);
		SurfaceInteraction si;
		si.uv = hit.texcoord;
		si.pos = hit.pos;
		si.wi = glm::normalize(basis.toLocalSpace(-r.d));
        MaterialParameters params = material->evalMaterialParameters(si.uv);
        glm::vec3 color = material->sample(RND::next2D(), si, params);
		glm::vec3 out = glm::normalize(basis.fromLocalSpace(si.wo));

		throughput *= color;
		r = Ray(hit.pos, out);
		bounces++;
	}

	return result;
}
