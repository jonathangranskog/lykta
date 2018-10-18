#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Lykta {
	class Scene {
	private:
		// TODO: Add vector of lights

		// TODO: Add vector of materials

		// TODO: Add vector of meshes

		// TODO: Add integrator


	public:
		Scene() {};
		Scene(const std::string& filename);
		~Scene() {};

		glm::vec3 sample(const glm::vec2& pixel);

	};
}