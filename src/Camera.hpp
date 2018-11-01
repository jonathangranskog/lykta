#pragma once

#include "common.h"
#include "Sampling.hpp"
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace Lykta {
	class Camera {
	protected:
		glm::ivec2 resolution;
		glm::mat4 projectionToCamera;
		glm::mat4 cameraToWorld;
		float aspect;

		glm::mat4 makeProjectionToCamera(float fov, float aspect, float nearClip, float farClip) {
			float recip = 1.f / (farClip - nearClip);
			float cot = 1.f / std::tanf(fov / 180.f * M_PI);
			glm::mat4 persp = glm::mat4();
			persp[0] = glm::vec4(cot, 0, 0, 0);
			persp[1] = glm::vec4(0, cot, 0, 0);
			persp[2] = glm::vec4(0, 0, farClip * recip, 1);
			persp[3] = glm::vec4(0, 0, -nearClip * farClip * recip, 0);
			return glm::inverse(persp);
		}

	public:
		virtual glm::vec3 createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const = 0;

		virtual const glm::vec2 getResolution() const { return resolution; }
	};

	class PerspectiveCamera : public Camera {
	private:
		float fov;
		float nearClip;
		float farClip;
		float apertureRadius;
		float focusDistance;

	public:
		PerspectiveCamera() {
			resolution = glm::ivec2(800, 800);
			aspect = resolution.x / (float)resolution.y;
			fov = 45.f;
			nearClip = EPS;
			farClip = 1e5f;
			cameraToWorld = lookAt(glm::vec3(0, 0.5, 1.05), glm::vec3(0, 0.5, 0), glm::vec3(0, 1, 0));
			projectionToCamera = makeProjectionToCamera(fov, aspect, nearClip, farClip);
		}

		PerspectiveCamera(glm::mat4 camToWorld, glm::ivec2 res, float f, float nc, float fc, float ar = 0.f, float fd = 1.f) 
		{
			aspect = res.x / (float)res.y;
			cameraToWorld = camToWorld;
			resolution = res;
			fov = f;
			nearClip = nc;
			farClip = fc;
			apertureRadius = ar;
			focusDistance = fd;
			projectionToCamera = makeProjectionToCamera(fov, aspect, nearClip, farClip);
		}

		virtual glm::vec3 createRay(Ray& ray, 
									const glm::vec2& pixel,
									const glm::vec2& sample) const 
		{
			glm::vec2 imageplane_pos = glm::vec2(pixel.x / resolution.x * 2 - 1, -1.f/aspect * (pixel.y / resolution.y * 2 - 1));
			glm::vec4 p = projectionToCamera * glm::vec4(imageplane_pos, 1, 1);
			glm::vec3 d = glm::normalize(glm::vec3(p));
			float invZ = 1.f / d.z;
			
			glm::vec3 focusPoint = focusDistance * invZ * d;
			glm::vec3 apertureSample = glm::vec3(Sampling::uniformDisk(sample) * apertureRadius, 0);
			glm::vec3 dir = glm::normalize(focusPoint - apertureSample);

			ray.o = glm::vec3(cameraToWorld * glm::vec4(apertureSample, 1));
			ray.d = glm::vec3(cameraToWorld * glm::vec4(dir, 0));
			ray.d = glm::normalize(ray.d);
			ray.t = glm::vec2(nearClip, farClip);

			return glm::vec3(1.f);
		}

	};
}