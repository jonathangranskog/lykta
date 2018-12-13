#include "Integrator.hpp"
#include "Emitter.hpp"
#include <iostream>
#include <cmath>

using namespace Lykta;

glm::vec3 Unidirectional::evaluate(const Ray& ray, const std::shared_ptr<Scene> scene) {
	glm::vec3 result = glm::vec3(0.f);
	glm::vec3 throughput = glm::vec3(1.f);
	Ray r = ray;
	const std::vector<EmitterPtr>& lights = scene->getEmitters();
	const std::vector<MeshPtr>& meshes = scene->getMeshes();
	unsigned numLights = lights.size();
	
	Lykta::Hit hit = Hit();
	bool intersected = scene->intersect(r, hit);
	EmitterPtr environment = scene->getEnvironment();

	if (!intersected) {
		if (environment) {
			EmitterInteraction envei;
			envei.direction = r.d;
			return environment->eval(envei);
		}
		else {
			return glm::vec3(0.f);
		}
	}

	MaterialPtr material = scene->getMaterial(hit.geomID);
	MeshPtr mesh = meshes[hit.geomID];
	EmitterPtr emitter = mesh->emitter;
	
	unsigned bounces = 1;
	float misWeightMat = 1.f, misWeightEmitter = 0.f;
	EmitterInteraction ei(hit.pos, r.o, hit.normal, r.d);
	glm::vec3 emitterEval = (emitter) ? emitter->eval(ei) : glm::vec3(0.f);
    MaterialParameters params = material->evalMaterialParameters(hit.texcoord);

	while (intersected) {

		// Add material contribution if hit emitter
		if (emitter != nullptr) {
			if (!std::isnan(misWeightMat))
				result += misWeightMat * throughput * emitterEval;
		}

		// RR
		// comes after material contribution to make sure bounce count is equal
		// between emitter sampling and material sampling
		float s = RND::next1D();
		float success = fminf(0.75f, Lykta::luminance(throughput));
		if (s < (1 - success)) break;
		throughput /= success;

		// Create basis
		Basis basis = Basis(hit.normal);

		// Sample emitter
		{
			ei = EmitterInteraction(hit.pos);
			const EmitterPtr emitter = scene->getRandomEmitter(RND::next1D());
			glm::vec3 Le = emitter->sample(RND::next3D(), ei);
			Hit tmp = Hit();
			if (!scene->shadowIntersect(ei.shadowRay)) {
				float emitterPDF = ei.pdf;
				SurfaceInteraction si = SurfaceInteraction();
				si.wi = glm::normalize(basis.toLocalSpace(-r.d));
				si.wo = glm::normalize(basis.toLocalSpace(ei.direction));
				si.pos = hit.pos;
				si.uv = hit.texcoord;
                glm::vec3 materialEval = material->evaluate(si, params);
				float materialPDF = si.pdf;

				misWeightEmitter = balanceHeuristic(emitterPDF, materialPDF);
				if (!std::isnan(misWeightEmitter)) {
					float nl = abs(glm::dot(ei.direction, hit.normal));
					result += numLights * misWeightEmitter * nl * throughput * materialEval * Le;
				}
			}
		}
		
		// Sample material
		SurfaceInteraction si = SurfaceInteraction();
		si.uv = hit.texcoord;
		si.pos = hit.pos;
		si.wi = glm::normalize(basis.toLocalSpace(-r.d));
        glm::vec3 color = material->sample(RND::next2D(), si, params);
		glm::vec3 out = glm::normalize(basis.fromLocalSpace(si.wo));
		r = Ray(hit.pos, out);
		hit = Hit();
		intersected = scene->intersect(r, hit);
		throughput *= color;

		if (intersected) {
			// Reinit variables
			material = scene->getMaterial(hit.geomID);
			mesh = meshes[hit.geomID];
			emitter = mesh->emitter;
            params = material->evalMaterialParameters(hit.texcoord);

			// compute material MIS weight and evaluate emitter
			if (emitter != nullptr) {
				ei = EmitterInteraction(hit.pos, r.o, hit.normal, r.d);
				emitterEval = emitter->eval(ei);
				float materialPDF = si.pdf;
				float emitterPDF = ei.pdf;
				misWeightMat = balanceHeuristic(materialPDF, emitterPDF);
			}
		}
		else if (environment) {
			ei = EmitterInteraction();
			ei.direction = r.d;
			emitterEval = environment->eval(ei);
			misWeightMat = balanceHeuristic(si.pdf, ei.pdf);
			result += misWeightMat * throughput * emitterEval;
		}
		
		bounces++;
	}

	return result;
}
