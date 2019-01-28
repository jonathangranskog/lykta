#pragma once

#include "common.h"
#include "Texture.hpp"

namespace Lykta {
	// All in local shading space where z axis is along normal
	struct SurfaceInteraction {
		glm::vec2 uv;
		glm::vec3 pos;
		glm::vec3 wo;
		glm::vec3 wi;
		float pdf;
	};

    // Input into material functions
    // These values contain values that are combined
    // with values evaluated from textures
    struct MaterialParameters {
        glm::vec3 diffuseColor;
        float specular;
        float specularTint;
		float refractivity;
        float roughness;
        float ior;
        float alpha;
        float alpha2;
    };

	class SurfaceMaterial {
	private:
        // Constant parameters
		glm::vec3 diffuseColor;
        glm::vec3 emissiveColor;
        float specular;
		float specularTint;
		float refractivity;
		float roughness;
		float ior;

		float alpha;
		float alpha2;

		bool isTwoSided;

        // Textures
        TexturePtr<glm::vec3> diffuseTexture;
        TexturePtr<float> specularTexture;
        TexturePtr<float> tintTexture;
		TexturePtr<float> refractionTexture;
        TexturePtr<float> roughnessTexture;
		TexturePtr<float> opacityTexture;

	public:
		SurfaceMaterial(const glm::vec3& diffuse, const glm::vec3& emission, float spec, float spectint, float refr, float rough, float ior_, bool twosided) {
			diffuseColor = diffuse;
			emissiveColor = emission;
			specular = spec;
			specularTint = spectint;
			refractivity = refr;
			roughness = rough;
			ior = ior_;

			alpha = rough * rough;
			alpha2 = alpha * alpha;
			
			isTwoSided = twosided;

            diffuseTexture = nullptr;
            specularTexture = nullptr;
            tintTexture = nullptr;
			refractionTexture = nullptr;
            roughnessTexture = nullptr;
			opacityTexture = nullptr;
		};

        SurfaceMaterial(const glm::vec3 &diffuse, const glm::vec3 &emission, float spec,
                        float spectint, float refr, float rough, float ior_, bool twosided,
                        TexturePtr<glm::vec3> diffTex, TexturePtr<float> specTex, TexturePtr<float> tintTex,
						TexturePtr<float> refrTex, TexturePtr<float> roughTex, TexturePtr<float> opacTex) {
            diffuseColor = diffuse;
            emissiveColor = emission;
            specular = spec;
            specularTint = spectint;
			refractivity = refr;
            roughness = rough;
            ior = ior_;

            alpha = rough * rough;
            alpha2 = alpha * alpha;

			isTwoSided = twosided;

            diffuseTexture = diffTex;
            specularTexture = specTex;
            tintTexture = tintTex;
			refractionTexture = refrTex;
            roughnessTexture = roughTex;
			opacityTexture = opacTex;
        };

		~SurfaceMaterial() {};
		SurfaceMaterial() {};

        MaterialParameters evalMaterialParameters(const glm::vec2& uv) const;
		void evalShadingNormal(glm::vec3& normal, const glm::vec3& view, const glm::vec2& uv) const;

		glm::vec3 getEmission() const {
			return emissiveColor;
		}

		const TexturePtr<float> getOpacityTexture() const {
			return opacityTexture;
		}

        glm::vec3 evalSpecular(SurfaceInteraction& si, const MaterialParameters& params) const;
        glm::vec3 evalDiffuse(SurfaceInteraction& si, const MaterialParameters& params) const;
		glm::vec3 evalRefraction(SurfaceInteraction& si, const MaterialParameters& params) const;
		
		glm::vec3 sampleSpecular(const glm::vec2& sample, SurfaceInteraction& si, const MaterialParameters& params) const;
        glm::vec3 sampleDiffuse(const glm::vec2& sample, SurfaceInteraction& si, const MaterialParameters& params) const;
		glm::vec3 sampleRefraction(const glm::vec2& sample, SurfaceInteraction& si, const MaterialParameters& params) const;

        glm::vec3 evaluate(SurfaceInteraction& si, const MaterialParameters& params) const;
        glm::vec3 sample(const glm::vec2& sample, SurfaceInteraction& si, const MaterialParameters& params) const;

	};
}
// Class for generic surface materials
