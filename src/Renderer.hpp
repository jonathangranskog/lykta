#pragma once

#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include "Scene.hpp"
#include "Integrator.hpp"

namespace Lykta {
	class Renderer {
	private:
		std::vector<glm::vec3> image;
		std::unique_ptr<Scene> scene;
		std::unique_ptr<Integrator> integrator;
		glm::ivec2 resolution;
		unsigned iteration;

	public:
		void openScene(const std::string& filename);
		void renderFrame();

		const std::vector<glm::vec3>& getImage() const {
			return image;
		}

		const glm::ivec2& getResolution() const {
			return resolution;
		}

	};
}