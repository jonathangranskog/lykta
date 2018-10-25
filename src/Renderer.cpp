#include <random>
#include "Renderer.hpp"

void Lykta::Renderer::openScene(const std::string& filename) {
	iteration = 0;

	// TODO: Initialize the scene here!
	scene = std::unique_ptr<Scene>(Scene::parseFile(filename));
	resolution = scene->getResolution();
	image = std::vector<glm::vec3>(resolution.x * resolution.y);
	Ray ray(glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), glm::vec2(0, INFINITY));
	scene->intersect(ray);
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
			glm::vec3 result = glm::vec3(imageSample.x / resolution.x, imageSample.y / resolution.y, 0.f);

			if (iteration > 0)
				image[j * resolution.x + i] = (1 - blend) * image[j * resolution.x + i] + blend * result;
			else {
				image[j * resolution.x + i] = result;
			}
		}
	}

	iteration++;
}