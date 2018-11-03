#include "Emitter.hpp"

using namespace Lykta;

glm::vec3 MeshEmitter::eval(EmitterInteraction& ei) const {
	float areaPDF = mesh->pdf();
	float dist = glm::length(ei.position - ei.origin);
	ei.pdf = areaPDF * (dist * dist) / fabsf(glm::dot(ei.normal, -ei.direction));
	return mesh->material->getEmission();
}

glm::vec3 MeshEmitter::sample(const glm::vec3& s, EmitterInteraction& ei) const {
	MeshSample info;
	mesh->sample(s, info);
	float areaPDF = info.pdf;
	
	ei.position = info.pos;
	ei.direction = info.pos - ei.origin;
	float maxdist = glm::length(ei.direction);
	ei.direction = glm::normalize(ei.direction);
	ei.shadowRay = Ray(ei.origin, ei.direction, glm::vec2(EPS, maxdist - EPS));
	ei.pdf = areaPDF * (maxdist * maxdist) / fabsf(glm::dot(info.normal, -ei.direction));
	return mesh->material->getEmission() / ei.pdf;
}