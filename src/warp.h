#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "common.h"

inline glm::vec2 squareToUniformDisk(const glm::vec2& sample) {
	float r = sqrtf(sample.x);
	float theta = sample.y * M_PI * 2;
	return glm::vec2(r * cosf(theta), r * sinf(theta));
}

inline glm::vec3 squareToCosineHemisphere(const glm::vec2& sample) {
	glm::vec2 disk = squareToUniformDisk(sample);
	return glm::vec3(disk.x, disk.y, 1 - disk.x * disk.x - disk.y * disk.y);
}

inline float cosineHemispherePdf(const glm::vec3& v) {
	float d = v.z;
	if (d < 0) return 0;
	return d * INV_PI;
}
