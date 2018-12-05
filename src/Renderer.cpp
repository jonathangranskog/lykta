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

	// Create a batch of camera rays
	std::vector<Ray> cameraRays;
	std::vector<glm::vec3> cameraColors;
	scene->getCamera()->createRayBatch(cameraRays, cameraColors, samplers);

	// Integrate each pixel
	#pragma omp parallel for schedule(dynamic) 
	for (int it = 0; it < resolution.y * resolution.x; it++) {
		int i = it % resolution.x;
		int j = it / resolution.x;
		int thread = omp_get_thread_num();
		int index = j * resolution.x + i;
		RandomSampler* sampler = &samplers[thread];

		// Third integrate
		glm::vec3 result = cameraColors[index] * integrator->evaluate(cameraRays[index], scene, sampler);

		if (iteration > 0)
			image[index] = (1 - blend) * image[index] + blend * result;
		else
			image[index] = result;
	}

	iteration++;
}