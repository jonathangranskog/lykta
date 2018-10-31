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

	inline glm::vec3 squareToBeckmann(const glm::vec2& sample, float alpha) {
		float theta = atanf(sqrtf(-alpha * alpha * logf(1 - sample.x)));
		float phi = 2 * M_PI * sample.y;
		return glm::vec3(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
	}

	inline float beckmannPdf(const glm::vec3& wh, float alpha) {
		float costheta = localCosTheta(wh);
		if (costheta < 0.f) return 0.f;
		float tantheta = localTanTheta(wh);
		float nominator = expf(-tantheta * tantheta / (alpha * alpha));
		float denominator = M_PI * alpha * alpha * costheta * costheta * costheta;
		return nominator / denominator;
	}
}
