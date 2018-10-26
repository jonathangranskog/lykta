#include <random>
#include "Renderer.hpp"

void Lykta::Renderer::openScene(const std::string& filename) {
	iteration = 0;

	// TODO: Initialize the scene here!
	scene = std::shared_ptr<Lykta::Scene>(Scene::parseFile(filename));
	resolution = scene->getResolution();
	image = std::vector<glm::vec3>(resolution.x * resolution.y);
	integrator = std::unique_ptr<Lykta::Integrator>(new Lykta::AOIntegrator());
}

void Lykta::Renderer::renderFrame() {
	float blend = 1.f / (iteration + 1);

	for (int j = 0; j < resolution.y; j++) {
		for (int i = 0; i < resolution.x; i++) {
			
			// First, get pixel that is being rendered
			glm::vec2 imageSample = glm::vec2(i + 0.5f, j + 0.5f);

			// Second, create ray with camera
			Ray ray;
			scene->getCamera()->createRay(ray, imageSample, glm::vec2(0));

			// Third integrate
			glm::vec3 result = integrator->evaluate(ray, scene);

			if (iteration > 0)
				image[j * resolution.x + i] = (1 - blend) * image[j * resolution.x + i] + blend * result;
			else {
				image[j * resolution.x + i] = result;
			}
		}
	}

	iteration++;
}