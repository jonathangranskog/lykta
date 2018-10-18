#pragma once

#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include "Scene.hpp"

namespace Lykta {
	class Renderer {
	private:
		std::vector<glm::vec3> image;
		std::unique_ptr<Scene> scene;
		unsigned width, height;
		unsigned iteration;

	public:
		void openScene(const std::string& filename);
		void renderFrame();

		const std::vector<glm::vec3>& getImage() const {
			return image;
		}

	};
}