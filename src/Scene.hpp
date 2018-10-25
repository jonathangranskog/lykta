#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <embree3/rtcore.h>
#include <embree3/rtcore_scene.h>
#include "Camera.hpp"
#include "Mesh.hpp"

namespace Lykta {
	class Scene {
	private:
		// TODO: Add camera
		std::unique_ptr<Camera> camera;

		// TODO: Add vector of lights

		// TODO: Add vector of materials

		// TODO: Add vector of meshes
		std::vector<Mesh> meshes;

		// Embree specific variables
		RTCDevice embree_device;
		RTCScene embree_scene;


	public:
		Scene() {};
		~Scene() {};

		bool intersect(const Ray& ray) const;

		const glm::ivec2 getResolution() const {
			return camera->getResolution();
		}

		const std::unique_ptr<Camera>& getCamera() const {
			return camera;
		}

		static Scene* parseFile(const std::string& filename);

		// Embree functions
		void generateEmbreeScene();
		unsigned createEmbreeGeometry(Mesh& mesh);
	};
}