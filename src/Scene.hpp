#pragma once

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <embree3/rtcore.h>
#include <embree3/rtcore_scene.h>
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "random.h"

namespace Lykta {
	class Scene {
	private:
		std::unique_ptr<Camera> camera;
		std::vector<MaterialPtr> materials;
		std::vector<MeshPtr> meshes;
		std::vector<EmitterPtr> emitters;
		EmitterPtr environment = nullptr;
		
		// Embree specific variables
		RTCDevice embree_device;
		RTCScene embree_scene;
		static ScenePtr activeScene;
		
		// Embree functions
		void generateEmbreeScene();
		unsigned createEmbreeGeometry(MeshPtr mesh);
		static void opacityIntersectFilter(const RTCFilterFunctionNArguments* args);

	public:
		Scene() {};
		~Scene() {};

		bool intersect(const Ray& ray, Hit& result) const;
		bool shadowIntersect(const Ray& ray) const;
		
		
		const glm::ivec2 getResolution() const {
			return camera->getResolution();
		}

		const MaterialPtr getMaterial(unsigned geomID) {
			return meshes[geomID]->material;
		}

		const EmitterPtr getRandomEmitter(float r) {
			if (emitters.size() == 0) return nullptr;
			return emitters[(int)(r * emitters.size())];
		}

		const std::vector<EmitterPtr> getEmitters() {
			return emitters;
		}

		const EmitterPtr getEnvironment() {
			return environment;
		}

		const std::vector<MeshPtr> getMeshes() {
			return meshes;
		}

		const std::vector<MaterialPtr> getMaterials() {
			return materials;
		}

		const std::unique_ptr<Camera>& getCamera() const {
			return camera;
		}

		static ScenePtr parseFile(const std::string& filename);

		
	};
}