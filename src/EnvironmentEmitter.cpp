#include "Emitter.hpp"
#include "Sampling.hpp"

using namespace Lykta;

glm::vec2 EnvironmentEmitter::dir2uv(const glm::vec3& dir) const {
	float theta = std::acos(dir.y);
	float phi = std::atan2(dir.z, dir.x) + M_PI;
	phi /= 2 * M_PI;
	theta /= M_PI;
	return glm::vec2(phi, theta);
}

glm::vec3 EnvironmentEmitter::uv2dir(const glm::vec2& uv) const {
	float theta = uv.y * M_PI;
	float phi = uv.x * 2 * M_PI - M_PI;
	float y = cos(theta);
	float xz = sqrtf(1 - y * y);
	float x = xz * cos(phi);
	float z = xz * sin(phi);
	return glm::vec3(x, y, z);
}

glm::vec3 EnvironmentEmitter::eval(EmitterInteraction& ei) const {
	ei.pdf = Sampling::uniformSpherePdf(ei.direction);
	return intensity * map->eval(dir2uv(ei.direction));
}

glm::vec3 EnvironmentEmitter::sample(const glm::vec3& s, EmitterInteraction& ei) const {
	glm::vec3 dir = Sampling::uniformSphere(glm::vec2(s.x, s.y));
	ei.position = ei.origin + dir * 10000.0f;
	ei.direction = dir;
	ei.normal = -dir;
	ei.shadowRay = Ray(ei.origin, ei.direction);
	ei.pdf = Sampling::uniformSpherePdf(ei.direction);
	return intensity * map->eval(dir2uv(ei.direction)) / ei.pdf;
}