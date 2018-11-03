#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

#undef M_PI
#define EPS 1e-4f
#define FLT_EPS 1e-15f
#define M_PI 3.14159265359f
#define INV_PI 0.31830988618f

namespace Lykta {
	
	class Emitter;
	class Mesh;
	class SurfaceMaterial;

	typedef std::shared_ptr<Emitter> EmitterPtr;
	typedef std::shared_ptr<Mesh> MeshPtr;
	typedef std::shared_ptr<SurfaceMaterial> MaterialPtr;

	struct Ray {
		glm::vec3 o;
		glm::vec3 d;
		glm::vec2 t;

		Ray() {};
		Ray(glm::vec3 orig, glm::vec3 dir) : o(orig), d(dir) {
			t = glm::vec2(EPS, INFINITY);
		}
		Ray(glm::vec3 orig, glm::vec3 dir, glm::vec2 tz) : o(orig), d(dir), t(tz) {}
	};

	struct Hit {
		glm::vec2 texcoord;
		glm::vec3 pos;
		glm::vec3 normal;
		unsigned geomID;
	};

	struct Triangle {
		unsigned px, py, pz;
		int nx, ny, nz;
		int tx, ty, tz;
	};

	struct Basis {
		glm::vec3 x, y, z;

		// http://jcgt.org/published/0006/01/01/
		Basis(const glm::vec3& n) {
			float sign = copysignf(1.0f, n.z);
			const float a = -1.0f / (sign + n.z);
			const float b = n.x * n.y * a;
			x = glm::vec3(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
			y = glm::vec3(b, sign + n.y * n.y * a, -n.y);
			z = n;
		}

		glm::vec3 toLocalSpace(const glm::vec3& v) {
			return glm::vec3(glm::dot(x, v), glm::dot(y, v), glm::dot(z, v));
		}

		glm::vec3 fromLocalSpace(const glm::vec3& v) {
			return v.x * x + v.y * y + v.z * z;
		}
	};

	inline glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
		glm::vec3 z = glm::normalize(center - eye);
		glm::vec3 x = glm::normalize(glm::cross(z, up));
		glm::vec3 y = glm::normalize(glm::cross(x, z));

		glm::mat4 result = glm::mat4();
		result[0] = glm::vec4(x, 0);
		result[1] = glm::vec4(y, 0);
		result[2] = glm::vec4(z, 0);
		result[3] = glm::vec4(eye, 1);

		return result;
	}

	inline float maxComponent(const glm::vec3& v) {
		return fmaxf(v.x, fmaxf(v.y, v.z));
	}

	inline float clamp(float a, float min, float max) {
		return fmaxf(fminf(a, max), min);
	}

	inline float luminance(const glm::vec3& v) {
		return v.x * 0.212671f + v.y * 0.71516f + v.z * 0.072169f;
	}

	inline float localCosTheta(const glm::vec3& v) {
		return v.z;
	}

	inline float localSinTheta2(const glm::vec3& v) {
		return 1 - v.z * v.z;
	}

	inline float localSinTheta(const glm::vec3& v) {
		float tmp = localSinTheta2(v);
		if (tmp <= 0.f) return 0.f;
		return sqrtf(tmp);
	}

	inline float localTanTheta(const glm::vec3& v) {
		float tmp = 1 - v.z * v.z;
		if (tmp <= 0.f) return 0.f;
		return sqrtf(tmp) / v.z;
	}

	// Not Schlick approximated fresnel
	inline float fresnel(float cti, float extIOR, float intIOR) {
		float etaI = extIOR, etaT = intIOR;
		if (extIOR == intIOR) return 0.f;
		if (cti < 0.f) {
			float tmp = etaI;
			etaI = etaT;
			etaT = tmp;
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

	inline float balanceHeuristic(float pdf1, float pdf2) {
		return pdf1 / (pdf1 + pdf2);
	}

}
