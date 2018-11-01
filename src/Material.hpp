#pragma once

#include "common.h"

namespace Lykta {
	// All in local shading space where z axis is along normal
	struct SurfaceInteraction {
		glm::vec2 uv;
		glm::vec3 pos;
		glm::vec3 wo;
		glm::vec3 wi;
		float pdf;
	};

	class SurfaceMaterial {
	private:
		glm::vec3 diffuseColor;
		glm::vec3 emissiveColor;
		float specular;
		float specularTint;
		float roughness;
		float ior;

		float alpha;
		float alpha2;
		
	public:
		SurfaceMaterial(const glm::vec3& diffuse, const glm::vec3& emission, float spec, float spectint, float rough, float ior_ ) {
			diffuseColor = diffuse;
			emissiveColor = emission;
			specular = spec;
			specularTint = spectint;
			roughness = rough;
			ior = ior_;

			alpha = rough * rough;
			alpha2 = alpha * alpha;
		};
		~SurfaceMaterial() {};
		SurfaceMaterial() {};

		glm::vec3 getEmission() const {
			return emissiveColor;
		}

		glm::vec3 evalSpecular(SurfaceInteraction& si) const;
		glm::vec3 evalDiffuse(SurfaceInteraction& si) const;
		void sampleSpecular(const glm::vec2& sample, SurfaceInteraction& si) const;
		void sampleDiffuse(const glm::vec2& sample, SurfaceInteraction& si) const;
		
		glm::vec3 evaluate(SurfaceInteraction& si) const;
		glm::vec3 sample(const glm::vec2& sample, SurfaceInteraction& si) const;

	};
}
// Class for generic surface materials
