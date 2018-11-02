#include "Emitter.hpp"

using namespace Lykta;

glm::vec3 MeshEmitter::eval(EmitterInteraction& ei) const {
	if (glm::dot(-ei.wi, ei.n) < 0) {
		ei.pdf = 0.f;
		return glm::vec3(0.f);
	}

	float areaPDF = mesh->pdf();
	float dist = glm::length(ei.p - ei.ref);
	ei.pdf = areaPDF * (dist * dist) / fabsf(glm::dot(ei.n, -ei.wi));
	return mesh->material->getEmission();
}

glm::vec3 MeshEmitter::sample(const glm::vec3& s, EmitterInteraction& ei) const {
	MeshSample info;
	mesh->sample(s, info);
	float areaPDF = mesh->pdf();
	
	ei.p = info.pos;
	ei.wi = info.pos - ei.ref;
	float maxdist = glm::length(ei.wi);
	ei.wi = glm::normalize(ei.wi);
	ei.shadowRay = Ray(ei.p, ei.wi, glm::vec2(EPS, maxdist - EPS));
	ei.pdf = areaPDF * (maxdist * maxdist) / fabsf(glm::dot(info.normal, -ei.wi));
	return mesh->material->getEmission() / ei.pdf;
}