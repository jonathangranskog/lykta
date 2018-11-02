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

namespace Lykta {
	class Scene {
	private:
		std::unique_ptr<Camera> camera;
		std::vector<MaterialPtr> materials;
		std::vector<MeshPtr> meshes;
		std::vector<EmitterPtr> emitters;
		// TODO: Add vector of textures
		// Embree specific variables
		RTCDevice embree_device;
		RTCScene embree_scene;

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

		const std::unique_ptr<Camera>& getCamera() const {
			return camera;
		}

		static Scene* parseFile(const std::string& filename);

		// Embree functions
		void generateEmbreeScene();
		unsigned createEmbreeGeometry(MeshPtr mesh);
	};
}