#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "Camera.hpp"

namespace Lykta {
	class Scene {
	private:
		// TODO: Add camera
		std::unique_ptr<Camera> camera;

		// TODO: Add vector of lights

		// TODO: Add vector of materials

		// TODO: Add vector of meshes

	public:
		Scene() {};
		~Scene() {};

		bool intersect(const Ray& ray) const;

		const glm::ivec2 getResolution() const {
			return camera->getResolution();
		}

		const std::unique_ptr<Camera>& getCamera() const {
			return camera;
		}

		static Scene* parseFile(const std::string& filename);

	};
}