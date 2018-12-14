#include "Emitter.hpp"
#include "Sampling.hpp"

using namespace Lykta;

EnvironmentEmitter::EnvironmentEmitter(TexturePtr<glm::vec3> m, float intens) {
	map = m;
	intensity = intens;

	// Construct image for CDF
	ImagePtr<glm::vec3> img = m->getImage();
	dims = img->getDims();
	std::vector<std::vector<float>> distr;
	distr.assign(dims.y, std::vector<float>(dims.x));

	#pragma omp parallel for
	for (int j = 0; j < dims.y; j++) {
		float theta = M_PI * (j + 0.5f) / dims.y;
		float sinTheta = sin(theta);

		// It doesn't matter if we apply sinTheta to whole row or just the column distribution
		// As everything is normalized later anyways.
		for (int i = 0; i < dims.x; i++) {
			distr[j][i] = sinTheta * luminance(img->read(glm::ivec2(i, j)));
			distr[j][i] = fmaxf(distr[j][i], EPS);
		}
	}

	// Construct distribution
	samplingDistribution = Distribution2D(distr);
}

glm::vec2 EnvironmentEmitter::dir2uv(const glm::vec3& dir) const {
	float theta = std::acos(dir.y);
	float phi = std::atan2(dir.z, dir.x) + M_PI;
	phi /= 2 * M_PI;
	theta /= M_PI;
	return glm::vec2(phi, 1.f - theta);
}

glm::vec3 EnvironmentEmitter::uv2dir(const glm::vec2& uv) const {
	float theta = (1 - uv.y) * M_PI;
	float phi = uv.x * 2 * M_PI - M_PI;
	float y = cos(theta);
	float xz = sqrtf(1 - y * y);
	float x = xz * cos(phi);
	float z = xz * sin(phi);
	return glm::vec3(x, y, z);
}

glm::vec2 EnvironmentEmitter::uv2img(const glm::vec2& uv) const {
	glm::vec2 c = glm::vec2(uv.x * dims.x, (1.f - uv.y) * dims.y);
	return glm::vec2(clamp(c.x, 0, dims.x - 1), clamp(c.y, 0, dims.y - 1));
}

glm::vec2 EnvironmentEmitter::img2uv(const glm::vec2& img) const {
	return glm::vec2(img.x / dims.x, 1.f - img.y / dims.y);
}

glm::vec3 EnvironmentEmitter::eval(EmitterInteraction& ei) const {
	glm::vec2 uv = dir2uv(ei.direction);
	glm::ivec2 img = uv2img(uv);
	ei.pdf = samplingDistribution.pdf(img);
	return intensity * map->eval(uv);
}

glm::vec3 EnvironmentEmitter::sample(const glm::vec3& s, EmitterInteraction& ei) const {
	glm::vec2 img = samplingDistribution.sample(glm::vec2(s.x, s.y), ei.pdf);
	img += glm::vec2(0.5f);
	glm::vec2 uv = img2uv(img);
	glm::vec3 dir = uv2dir(uv);
	ei.position = ei.origin + dir * 10000.0f;
	ei.direction = dir;
	ei.normal = -dir;
	ei.shadowRay = Ray(ei.origin, ei.direction);
	float w = sin(M_PI * img.y / dims.y);
	ei.pdf = ei.pdf * dims.x * dims.y / (2 * M_PI * M_PI * w);
	return intensity * map->eval(uv) / ei.pdf;
}