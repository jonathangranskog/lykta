#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "common.h"
#include "warp.h"

namespace Lykta {
	// TODO: Implement textures as indices into an array
	struct SurfaceMaterial {
		glm::vec3 diffuseColor;
		glm::vec3 emissiveColor;
	};

	// All in local shading space where z axis is along normal
	struct SurfaceInteraction {
		glm::vec2 uv;
		glm::vec3 pos;
		glm::vec3 wo;
		glm::vec3 wi;
		float pdf;
	};

	// TODO: Implement specular model.

	// Evaluate computes PDF and the contribution for the GIVEN incident and outgoing vectors
	inline glm::vec3 evaluateSurfaceMaterial(const SurfaceMaterial& mat, SurfaceInteraction& si) {
		// If on opposite side of normal
		if (si.wo.z <= 0.f || si.wi.z <= 0.f) {
			si.pdf = 0.f;
			return glm::vec3(0.f);
		}
		
		si.pdf = INV_PI * si.wo.z;
		return INV_PI * mat.diffuseColor;
	}

	// Samples an outgoing direction GIVEN incident direction
	inline glm::vec3 sampleSurfaceMaterial(const glm::vec2& sample, const SurfaceMaterial& mat, SurfaceInteraction& si) {
		if (si.wi.z <= 0.f) {
			si.pdf = 0.f;
			return glm::vec3(0.f);
		}

		si.wo = squareToCosineHemisphere(sample);

		return mat.diffuseColor;
	}

}
