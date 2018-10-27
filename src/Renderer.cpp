#include <random>
#include <iostream>
#include "Renderer.hpp"
#include "omp.h"

void Lykta::Renderer::openScene(const std::string& filename) {
	iteration = 0;
	scene = std::shared_ptr<Lykta::Scene>(Scene::parseFile(filename));
	resolution = scene->getResolution();
	image = std::vector<glm::vec3>(resolution.x * resolution.y);
	integrator = std::unique_ptr<Lykta::Integrator>(new Lykta::AOIntegrator());
	
	samplers = std::vector<RandomSampler>(omp_get_max_threads());
	for (int i = 0; i < omp_get_max_threads(); i++) {
		samplers[i].seed(i);
	}
}

void Lykta::Renderer::renderFrame() {
	float blend = 1.f / (iteration + 1);

	#pragma omp parallel for
	for (int j = 0; j < resolution.y; j++) {
		for (int i = 0; i < resolution.x; i++) {
			
			int thread = omp_get_thread_num();
			RandomSampler* sampler = &samplers[thread];
			// First, get pixel that is being rendered
			glm::vec2 imageSample = glm::vec2(i, j) + sampler->next2D();

			// Second, create ray with camera
			Ray ray;
			scene->getCamera()->createRay(ray, imageSample, sampler->next2D());

			// Third integrate
			glm::vec3 result = integrator->evaluate(ray, scene, sampler);

			if (iteration > 0)
				image[j * resolution.x + i] = (1 - blend) * image[j * resolution.x + i] + blend * result;
			else {
				image[j * resolution.x + i] = result;
			}
		}
	}

	iteration++;
}