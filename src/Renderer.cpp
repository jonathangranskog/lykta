#include <random>
#include <iostream>
#include "Renderer.hpp"
#include "RandomPool.hpp"
#include "omp.h"

using namespace Lykta;

Renderer::Renderer() {
	resolution = glm::ivec2(800, 800);
	image = Image<glm::vec3>(resolution.x, resolution.y);
	integratorType = Integrator::Type::BSDF;
}

void Renderer::openScene(const std::string& filename) {
	scene = Scene::parseFile(filename);
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

	RND::init();
	integrator->preprocess(scene);
}

void Renderer::renderFrame() {
	float blend = 1.f / (iteration + 1);

	// Create a batch of camera rays
	std::vector<Ray> cameraRays;
	std::vector<glm::vec3> cameraColors;
	scene->getCamera()->createRayBatch(cameraRays, cameraColors);

	#pragma omp parallel for schedule(dynamic) 
	for (int it = 0; it < resolution.y * resolution.x; it++) {
		int i = it % resolution.x;
		int j = it / resolution.x;
		
		// Integrate
		glm::vec3 result = cameraColors[it] * integrator->evaluate(cameraRays[it], scene);

		if (iteration > 0) image[it] = (1 - blend) * image[it] + blend * result;
		else image[it] = result;
	}

	iteration++;
}