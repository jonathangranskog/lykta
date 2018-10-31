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
		float specular;
		float metallic;
		float roughness;
		float ior;
	};

	// All in local shading space where z axis is along normal
	struct SurfaceInteraction {
		glm::vec2 uv;
		glm::vec3 pos;
		glm::vec3 wo;
		glm::vec3 wi;
		float pdf;
	};

	inline float roughnessToBeckmannAlpha(float roughness) {
		return roughness;
		//return sqrtf(2 / (roughness * roughness + 2));
	}

	inline float BeckmannD(const glm::vec3& wh, float alpha) {
		float t2 = localCosTheta(wh) * localCosTheta(wh);
		float expo = t2 - 1 / (alpha * alpha * t2);
		return expf(expo) / (M_PI * alpha * alpha * t2 * t2);
	}

	inline float BeckmannG1(const glm::vec3& wi, const glm::vec3& wh, float alpha) {
		float tantheta = localTanTheta(wi);
		if (tantheta == 0.f) return 1.f;
		if (glm::dot(wh, wi) * localCosTheta(wi) <= 0) return 0.f;
		float a = 1.f / (alpha * tantheta);
		if (a >= 1.6f) return 1.f;
		float a2 = a * a;
		// Approximation
		return (3.535f * a + 2.181f * a2) / (1.0f + 2.276f * a + 2.577f * a2);
	}

	// Not Schlick approximated fresnel
	inline float fresnel(float cti, float extIOR, float intIOR) {
		float etaI = extIOR, etaT = intIOR;
		if (extIOR == intIOR) return 0.f;
		if (cti < 0.f) {
			std::swap(etaI, etaT);
			cti = -cti;
		}
		float eta = etaI / etaT;
		float stt2 = eta * eta * (1 - cti * cti);
		if (stt2 > 1.f) return 1.f; // total internal reflection

		float ctt = sqrtf(1.f - stt2);
		float rs = (etaI * cti - etaT * ctt) / (etaI * cti + etaT * ctt);
		float rp = (etaT * cti - etaI * ctt) / (etaT * cti + etaI * ctt);
		return 0.5f * (rs * rs + rp * rp);
	}

	inline glm::vec3 evalSpecular(const SurfaceMaterial& mat, SurfaceInteraction& si) {
		
		if (localCosTheta(si.wo) <= 0.f || localCosTheta(si.wi) <= 0.f) {
			si.pdf = 0.f;
			return glm::vec3(0.f);
		}
		
		glm::vec3 wh = glm::normalize(si.wi + si.wo);
		float alpha = roughnessToBeckmannAlpha(mat.roughness);
		float D = BeckmannD(wh, alpha);
		float F = fresnel(fabsf(glm::dot(si.wi, wh)), 1.f, mat.ior);
		float G = BeckmannG1(si.wi, wh, alpha) * BeckmannG1(si.wo, wh, alpha);
		float jacob = 1.f / (4 * fabsf(localCosTheta(si.wi) * localCosTheta(si.wo)));
		glm::vec3 color = (1.f - mat.metallic) * glm::vec3(1.f) + mat.metallic * mat.diffuseColor;

		si.pdf = beckmannPdf(wh, alpha);
		return D * F * G * jacob * color;
	}

	inline void sampleSpecular(const glm::vec2& sample, const SurfaceMaterial& mat, SurfaceInteraction& si) {
		float alpha = roughnessToBeckmannAlpha(mat.roughness);
		glm::vec3 wh = squareToBeckmann(sample, alpha);
		si.wo = -si.wi + 2 * glm::dot(si.wi, wh) * wh;
	}

	inline glm::vec3 evalDiffuse(const SurfaceMaterial& mat, SurfaceInteraction& si) {
		// If on opposite side of normal
		if (localCosTheta(si.wo) <= 0.f || localCosTheta(si.wi) <= 0.f) {
			si.pdf = 0.f;
			return glm::vec3(0.f);
		}

		si.pdf = cosineHemispherePdf(si.wo);
		return INV_PI * mat.diffuseColor;
	}

	inline void sampleDiffuse(const glm::vec2& sample, const SurfaceMaterial& mat, SurfaceInteraction& si) {
		si.wo = squareToCosineHemisphere(sample);
	}

	// Evaluate computes PDF and the contribution for the GIVEN incident and outgoing vectors
	inline glm::vec3 evaluateSurfaceMaterial(const SurfaceMaterial& mat, SurfaceInteraction& si) {
		glm::vec3 diffuseEval = evalDiffuse(mat, si);
		float diffusePdf = si.pdf;
		glm::vec3 specularEval = evalSpecular(mat, si);
		float specularPdf = si.pdf;

		si.pdf = (1 - mat.specular) * diffusePdf + mat.specular * specularPdf;
		if (si.pdf < FLT_EPSILON) return glm::vec3(0.f);
		return (1 - mat.specular) * diffuseEval + mat.specular * specularEval;
	}

	// Samples an outgoing direction GIVEN incident direction
	inline glm::vec3 sampleSurfaceMaterial(const glm::vec2& sample, const SurfaceMaterial& mat, SurfaceInteraction& si) {
		glm::vec2 s = sample;

		if (s.x < mat.specular) {
			s.x /= mat.specular;
			sampleSpecular(s, mat, si);
		}
		else {
			s.x = (s.x - mat.specular) / (1.f - mat.specular);
			sampleDiffuse(s, mat, si);
		}

		glm::vec3 eval = evaluateSurfaceMaterial(mat, si);
		if (si.pdf < FLT_EPSILON) return glm::vec3(0.f);
		return eval / si.pdf * fabsf(localCosTheta(si.wo));
	}

}
