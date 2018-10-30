#include <random>
#include <iostream>
#include "Renderer.hpp"
#include "omp.h"

void Lykta::Renderer::openScene(const std::string& filename) {
	scene = std::shared_ptr<Lykta::Scene>(Scene::parseFile(filename));
	resolution = scene->getResolution();
	refresh();
}

void Lykta::Renderer::refresh() {
	image = std::vector<glm::vec3>(resolution.x * resolution.y);
	iteration = 0;

	// Select integrator
	if (integratorType == Integrator::Type::BSDF) {
		integrator = std::unique_ptr<Integrator>(new BSDFIntegrator());
	}
	else if (integratorType == Integrator::Type::AO) {
		integrator = std::unique_ptr<Integrator>(new AOIntegrator());
	}

	integrator->preprocess(scene);

	// Init samplers
	samplers.clear();
	samplers = std::vector<RandomSampler>(omp_get_max_threads());
	for (int i = 0; i < omp_get_max_threads(); i++) {
		samplers[i].seed(i);
	}
}

void Lykta::Renderer::renderFrame() {
	float blend = 1.f / (iteration + 1);

	#pragma omp parallel for schedule(dynamic) 
	for (int it = 0; it < resolution.y * resolution.x; it++) {
		int i = it % resolution.x;
		int j = it / resolution.x;
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
		else
			image[j * resolution.x + i] = result;
	}

	iteration++;
}