#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "common.h"

namespace Lykta {
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
		if (v.z < 0) return 0;
		return v.z * INV_PI;
	}

	// Inverse sampling according to D
	inline glm::vec3 squareToGGX(const glm::vec2& sample, float alpha) {
		float a2 = alpha * alpha;
		float theta = acosf(sqrtf((1 - sample.x) / (sample.x * (a2 - 1) + 1)));
		float phi = 2 * M_PI * sample.y;
		return glm::vec3(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
	}

	inline float GGXPdf(const glm::vec3& wh, const glm::vec3& wi, float alpha) {
		float nh = localCosTheta(wh);
		float a2 = alpha * alpha;
		float tmp = nh * nh * (a2 - 1) + 1;
		float D = a2 / (M_PI * tmp * tmp);
		return nh * D / (4 * localCosTheta(wi));
	}
}
