#pragma once

#include <iostream>
#include <filesystem/path.h>
#include <filesystem/resolver.h>
#include "Renderer.hpp"

namespace Lykta {
	class CommandLine {
	private:
		std::unique_ptr<Renderer> renderer;
	public:

		CommandLine(int argc, char** argv) {
			renderer = std::unique_ptr<Renderer>(new Renderer());

			char* sceneFile = argv[1];
			int samples = 128;
			if (argc == 3) {
				char* end;
				samples = strtol(argv[2], &end, 10);
			}
			
			std::string filename = std::string(sceneFile);
			render(filename, samples);
		}

		void render(const std::string& filename, int numSamples) {
			std::cout << "Opening scene file: " << filename << std::endl;
			renderer->openScene(filename);
			
			std::cout << "Starting render..." << std::endl;
			if (!renderer->isSceneOpen()) {
				std::cout << "Scene failed to open!" << std::endl;
				return;
			}

			for (int i = 0; i < numSamples; i++) {
				std::cout << "Rendering sample: " << i + 1 << "/" << numSamples << std::endl;
				renderer->renderFrame();
			}

			filesystem::path scenePath = filesystem::path(filename);
			std::string file = scenePath.filename();
			file.append(".png");
			filesystem::path folder = scenePath.parent_path();
			filesystem::path image = filesystem::path(file);
			filesystem::path imageFile = folder / image;

			renderer->getImage().save(imageFile.str());
		}


	};
}