#include "Material.hpp"
#include "Sampling.hpp"

using namespace Lykta;

glm::vec3 SurfaceMaterial::evalSpecular(SurfaceInteraction& si) const {
	if (localCosTheta(si.wo) <= 0.f || localCosTheta(si.wi) <= 0.f) {
		si.pdf = 0.f;
		return glm::vec3(0.f);
	}

	// Microfacet formula:
	// eval = (F * G * D) / (4 * (wi.n) * (wo.n))
	// pdf (if sampling according to D) = D * wh.n / (4 * wi.wh)

	glm::vec3 wh = glm::normalize(si.wi + si.wo);

	float nh = localCosTheta(wh);
	float ni = localCosTheta(si.wi);
	float no = localCosTheta(si.wo);

	float tmp = nh * nh * (alpha2 -1) + 1;
	float D = alpha2 / (M_PI * tmp * tmp);

	float F = fresnel(fabsf(glm::dot(si.wo, wh)), 1.f, ior);
	
	float nom = 2 * ni * no;
	float denom1 = no * sqrtf(alpha2 + (1 - alpha2) * ni * ni);
	float denom2 = ni * sqrtf(alpha2 + (1 - alpha2) * no * no);
	float G = nom / (denom1 + denom2);

	float denom = 1.f / (4 * fabsf(localCosTheta(si.wi) * localCosTheta(si.wo)));
	
	glm::vec3 color = (1.f - specularTint) * glm::vec3(1.f) + specularTint * diffuseColor;
	si.pdf = Sampling::GGXPdf(wh, si.wi, alpha);
	
	return D * F * G * denom * color;
}

glm::vec3 SurfaceMaterial::evalDiffuse(SurfaceInteraction& si) const {
	// If on opposite side of normal
	if (localCosTheta(si.wo) <= 0.f || localCosTheta(si.wi) <= 0.f) {
		si.pdf = 0.f;
		return glm::vec3(0.f);
	}
	
	si.pdf = Sampling::cosineHemispherePdf(si.wo);
	return INV_PI * diffuseColor;
}

glm::vec3 SurfaceMaterial::sampleSpecular(const glm::vec2& sample, SurfaceInteraction& si) const {
	glm::vec3 wh = Sampling::GGX(sample, alpha);
	// Reflect incoming vector with sampled half vector
	si.wo = -si.wi + 2 * glm::dot(si.wi, wh) * wh;
	glm::vec3 eval = evalSpecular(si); // evalSpecular sets pdf
	if (si.pdf < FLT_EPS) {
		si.pdf = 0.f;
		return glm::vec3(0.f);
	}

	// Could also be simplified as terms cancel out in eval/pdf
	// however, easier to read this way.
	return eval / si.pdf * localCosTheta(si.wo);
}

glm::vec3 SurfaceMaterial::sampleDiffuse(const glm::vec2& sample, SurfaceInteraction& si) const {
	si.wo = Sampling::cosineHemisphere(sample);
	si.pdf = Sampling::cosineHemispherePdf(si.wo);
	if (si.pdf < FLT_EPS) {
		si.pdf = 0.f;
		return glm::vec3(0.f);
	}
	// can just return diffuse color as other terms cancel out
	// (eval * n.wo / pdf) = color/pi * n.wo / (n.wo / pi) = color
	return diffuseColor;
}

// Final evaluate computes combination of responses given incident and outgoing directions
glm::vec3 SurfaceMaterial::evaluate(SurfaceInteraction& si) const {
	glm::vec3 diffuseEval = evalDiffuse(si);
	float diffusePdf = si.pdf;
	glm::vec3 specularEval = evalSpecular(si);
	float specularPdf = si.pdf;

	si.pdf = (1 - specular) * diffusePdf + specular * specularPdf;
	if (si.pdf < FLT_EPS) {
		si.pdf = 0.f;
		return glm::vec3(0.f);
	}
	return (1 - specular) * diffuseEval + specular * specularEval;
}

// Randomly samples outgoing direction from either specular or diffuse
// could also return full color combination like eval
// however sampling color randomly too feels cleaner
glm::vec3 SurfaceMaterial::sample(const glm::vec2& sample, SurfaceInteraction& si) const {
	glm::vec2 s = sample;

	if (s.x < specular) {
		s.x /= specular;
		return sampleSpecular(s, si);
	}
	else {
		s.x = (s.x - specular) / (1.f - specular);
		return sampleDiffuse(s, si);
	}
}