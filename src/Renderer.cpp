#include <random>
#include "Renderer.hpp"

void Lykta::Renderer::openScene(const std::string& filename) {
	width = 1024;
	height = 768;
	image = std::vector<glm::vec3>(width * height);
	iteration = 0;

	// TODO: Initialize the scene here!
}

void Lykta::Renderer::renderFrame() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr(0.f, 1.0f);
	float blend = 1.f / (iteration + 1);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			glm::vec3 result = glm::vec3(distr(gen), distr(gen), distr(gen));

			if (iteration > 0)
				image[i * height + j] = (1 - blend) * image[i * height + j] + blend * result;
			else {
				image[i * height + j] = result;
			}
		}
	}

	iteration++;
}