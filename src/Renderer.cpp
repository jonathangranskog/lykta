#include <random>
#include "Renderer.hpp"

void Lykta::Renderer::render() {
	width = 1024;
	height = 768;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distr(0.f, 1.0f);
	
	image = std::vector<float>(width * height * 3);

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			image[i * height * 3 + j * 3 + 0] = distr(gen);
			image[i * height * 3 + j * 3 + 1] = distr(gen);
			image[i * height * 3 + j * 3 + 2] = distr(gen);
		}
	}
}