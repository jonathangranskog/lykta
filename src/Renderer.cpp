#include <random>
#include <iostream>
#include "Renderer.hpp"
#include "omp.h"

using namespace Lykta;

Renderer::Renderer() {
	resolution = glm::ivec2(800, 800);
	image = Image<glm::vec3>(resolution.x, resolution.y);
	integratorType = Integrator::Type::BSDF;
}

void Renderer::openScene(const std::string& filename) {
	scene = std::shared_ptr<Scene>(Scene::parseFile(filename));
	resolution = scene->getResolution();
	image = Image<glm::vec3>(resolution.x, resolution.y);
	refresh();
}

void Renderer::refresh() {
	iteration = 0;

	// Select integrator
	if (integratorType == Integrator::Type::BSDF) {
		integrator = std::unique_ptr<Integrator>(new BSDFIntegrator());
	}
	else if (integratorType == Integrator::Type::AO) {
		integrator = std::unique_ptr<Integrator>(new AOIntegrator());
	}
	else if (integratorType == Integrator::Type::PT) {
		integrator = std::unique_ptr<Integrator>(new Unidirectional());
	}

	integrator->preprocess(scene);

	// Init samplers
	samplers.clear();
	samplers = std::vector<RandomSampler>(omp_get_max_threads());
	for (int i = 0; i < omp_get_max_threads(); i++) {
		samplers[i].seed(i);
	}
}

void Renderer::renderFrame() {
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
		glm::vec3 W = scene->getCamera()->createRay(ray, imageSample, sampler->next2D());

		// Third integrate
		glm::vec3 result = W * integrator->evaluate(ray, scene, sampler);

		if (iteration > 0)
			image[j * resolution.x + i] = (1 - blend) * image[j * resolution.x + i] + blend * result;
		else
			image[j * resolution.x + i] = result;
	}

	std::cout << iteration << std::endl;

	iteration++;
}