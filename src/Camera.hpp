#pragma once

#include "common.h"
#include "glm/vec2.hpp"
#include "glm/mat4x4.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Lykta {
	class Camera {
	protected:
		glm::ivec2 resolution;
		glm::mat4 projectionToCamera;
		glm::mat4 cameraToWorld;

		void makeProjectionToCamera(float aspect) {
			projectionToCamera = glm::translate(projectionToCamera, glm::vec3(1.f, 1.f / aspect, 0.f));
			projectionToCamera = glm::inverse(projectionToCamera);
			projectionToCamera = glm::scale(projectionToCamera, glm::vec3(0.5f, -0.5f * aspect, 1.f));
		}

	public:
		virtual glm::vec3 createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const = 0;

		const glm::vec2& getResolution() const { return resolution; }
	};

	class PerspectiveCamera : public Camera {
	private:
		float fov;
		float nearClip;
		float farClip;

	public:
		PerspectiveCamera() {
			resolution = glm::ivec2(1024, 768);
			float aspect = resolution.x / (float)resolution.y;
			fov = 45.f;
			nearClip = EPS;
			farClip = 1e9;
			cameraToWorld = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(0, 1, 0));
			projectionToCamera = glm::perspective(fov, aspect, nearClip, farClip);
			makeProjectionToCamera(aspect);
		}

		PerspectiveCamera(glm::mat4 proj, glm::mat4 camToWorld, glm::ivec2 res, float f, float nc, float fc) 
		{
			float aspect = res.x / (float)res.y;
			cameraToWorld = camToWorld;
			resolution = res;
			fov = f;
			nearClip = nc;
			farClip = fc;
			projectionToCamera = proj;
			makeProjectionToCamera(aspect);
		}

		virtual glm::vec3 createRay(Ray& ray, 
									const glm::vec2& pixel,
									const glm::vec2& sample) const 
		{
			// If multiple backends, this function should be moved into a utilities header file
			glm::vec4 p = projectionToCamera * glm::vec4(pixel.x / resolution.x, pixel.y / resolution.y, 0, 1);
			glm::vec4 d = glm::vec4(glm::normalize(glm::vec3(p)), 0);
			float zinv = 1.f / d.z;
			
			ray.o = glm::vec3(cameraToWorld * glm::vec4(0, 0, 0, 1));
			ray.d = glm::vec3(cameraToWorld * d);
			ray.tmin = nearClip * zinv;
			ray.tmax = farClip * zinv;

			return glm::vec3(1.f);
		}

	};
}