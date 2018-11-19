#pragma once

#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include "Integrator.hpp"
#include "Image.hpp"
#include "Scene.hpp"
#include "random.h"

namespace Lykta {
	class Renderer {
	private:
		Image<glm::vec3> image;
		std::shared_ptr<Scene> scene;
		std::unique_ptr<Integrator> integrator;
		Integrator::Type integratorType;
		std::vector<RandomSampler> samplers;
		glm::ivec2 resolution;
		unsigned iteration;

	public:
		Renderer();

		void openScene(const std::string& filename);
		void refresh();
		
		void renderFrame();

		Image<glm::vec3>& getImage() {
			return image;
		}

		const glm::ivec2& getResolution() const {
			return resolution;
		}

		bool isSceneOpen() const {
			return scene != nullptr;
		}

		void changeIntegrator(Integrator::Type type) {
			integratorType = type;
		}

	};
}