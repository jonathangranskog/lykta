#include <random>
#include "Renderer.hpp"

void Lykta::Renderer::openScene(const std::string& filename) {
	width = 1024;
	height = 768;
	image = std::vector<glm::vec3>(width * height);
	iteration = 0;

	// TODO: Initialize the scene here!
	scene = std::unique_ptr<Scene>(new Scene(filename));
}

void Lykta::Renderer::renderFrame() {
	float blend = 1.f / (iteration + 1);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			glm::vec3 result = scene->sample(glm::vec2((float)i / width, (float)j / height));
			
			if (iteration > 0)
				image[j * width + i] = (1 - blend) * image[j * width + i] + blend * result;
			else {
				image[j * width + i] = result;
			}
		}
	}

	iteration++;
}