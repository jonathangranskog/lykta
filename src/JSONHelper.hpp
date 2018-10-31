#pragma once

#include <cassert>
#include <string>
#include <map>
#include <tuple>
#include <filesystem/path.h>
#include <filesystem/resolver.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "Camera.hpp"
#include "material.h"
#include "Mesh.hpp"

namespace Lykta {

	static class JSONHelper {
	private:
		
		static glm::vec3 readVector3(const std::string& name, const rapidjson::Value& val) {
			glm::vec3 vec = glm::vec3(0.f);
			const rapidjson::Value& arr = val[name.c_str()];
			assert(arr.Size() == 3);
			vec.x = arr[0].GetFloat();
			vec.y = arr[1].GetFloat();
			vec.z = arr[2].GetFloat();
			return vec;
		}

		static glm::ivec2 readIVector2(const std::string& name, const rapidjson::Value& val) {
			glm::ivec2 vec = glm::ivec2(0);
			const rapidjson::Value& arr = val[name.c_str()];
			assert(arr.Size() == 2);
			vec.x = arr[0].GetInt();
			vec.y = arr[1].GetInt();
			return vec;
		}

		static glm::mat4 readTransform(const std::string& name, const rapidjson::Value& val) {
			glm::mat4 matrix = glm::mat4(1.f);
			const rapidjson::Value& transformValue = val[name.c_str()];

			glm::vec3 translate = (transformValue.HasMember("translate")) ? readVector3("translate", transformValue) : glm::vec3(0.f);
			glm::vec3 rotate = (transformValue.HasMember("rotate")) ? readVector3("rotate", transformValue) : glm::vec3(0.f);
			glm::vec3 scale = (transformValue.HasMember("scale")) ? readVector3("scale", transformValue) : glm::vec3(1.f);
			rotate *= (M_PI / 180.f);

			// Default order
			std::string order = "SRT";
			if (transformValue.HasMember("order")) order = std::string(transformValue["order"].GetString());
			for (char& c : order) {
				if (c == 'S') matrix = glm::scale(matrix, scale);
				else if (c == 'R') {
					matrix = glm::rotate(matrix, rotate.x, glm::vec3(1, 0, 0));
					matrix = glm::rotate(matrix, rotate.y, glm::vec3(0, 1, 0));
					matrix = glm::rotate(matrix, rotate.z, glm::vec3(0, 0, 1));
				}
				else if (c == 'T') matrix = glm::translate(matrix, translate);
			}

			return matrix;
		}

	public:
		static std::vector<Mesh> readMeshes(rapidjson::Document& document, 
			std::map<std::string, std::pair<unsigned, SurfaceMaterial>>& materials,
			filesystem::path& scene_path) {
			std::vector<Mesh> meshes = std::vector<Mesh>();

			if (!document.HasMember("objects")) return meshes;

			const rapidjson::Value& arr = document["objects"];

			for (rapidjson::SizeType i = 0; i < arr.Size(); i++) {
				assert(arr[i].HasMember("file"));
				assert(arr[i].HasMember("material"));
				const rapidjson::Value& file = arr[i]["file"];
				const rapidjson::Value& mat = arr[i]["material"];
				assert(file.IsString());
				assert(mat.IsString());
				std::string filename = std::string(file.GetString());
				filesystem::path filepath = filesystem::path(filename);
				
				if (!filepath.is_file()) {
					std::cerr << filepath.make_absolute() << " could not be found -- skipping!" << std::endl;
					continue;
				}

				std::vector<Mesh> imported = Mesh::openObj(filename);
				
				// Get material
				std::string materialLookup = std::string(mat.GetString());
				assert(materials.find(materialLookup) != materials.end());
				unsigned index = materials[materialLookup].first;
				for (Mesh& m : imported) m.materialId = index;
				meshes.insert(meshes.end(), imported.begin(), imported.end());
			}

			return meshes;
		}

		static std::map<std::string, std::pair<unsigned, SurfaceMaterial> > readMaterials(rapidjson::Document& document) {
			std::map<std::string, std::pair<unsigned, SurfaceMaterial> > materialMap;

			if (!document.HasMember("materials")) return materialMap;

			const rapidjson::Value& arr = document["materials"];

			for (size_t i = 0; i < arr.Size(); i++) {
				assert(arr[i].HasMember("name"));
				
				SurfaceMaterial mat;
				if (arr[i].HasMember("diffuseColor")) mat.diffuseColor = readVector3("diffuseColor", arr[i]);
				else mat.diffuseColor = glm::vec3(1.f);

				if (arr[i].HasMember("emissiveColor")) mat.emissiveColor = readVector3("emissiveColor", arr[i]);
				else mat.emissiveColor = glm::vec3(0.f);

				if (arr[i].HasMember("roughness")) mat.roughness = arr[i]["roughness"].GetFloat();
				else mat.roughness = 0.5f;

				if (arr[i].HasMember("specular")) mat.specular = arr[i]["specular"].GetFloat();
				else mat.specular = 0.f;

				if (arr[i].HasMember("metallic")) mat.metallic = arr[i]["metallic"].GetFloat();
				else mat.metallic = 0.f;

				if (arr[i].HasMember("ior")) mat.ior = arr[i]["ior"].GetFloat();
				else mat.ior = 1.33f;

				const rapidjson::Value& name = arr[i]["name"];
				assert(name.IsString());
				materialMap[name.GetString()] = std::pair<unsigned, SurfaceMaterial>((unsigned)i, mat);
			}

			return materialMap;
		}

		static Camera* readCamera(rapidjson::Document& document) {
			if (!document.HasMember("camera")) {
				Camera* cam = new PerspectiveCamera();
				return cam;
			}
				
			const rapidjson::Value& cameraValue = document["camera"];
			assert(cameraValue.HasMember("type"));
			
			const std::string type = cameraValue["type"].GetString();

			if (type == "PerspectiveCamera") {
				glm::mat4 cameraToWorld = glm::mat4();

				if (cameraValue.HasMember("lookat") && cameraValue.HasMember("center")) {
					glm::vec3 lookat = readVector3("lookat", cameraValue);
					glm::vec3 center = readVector3("center", cameraValue);
					glm::vec3 up = (cameraValue.HasMember("up")) ? readVector3("up", cameraValue) : glm::vec3(0, 1, 0);
					cameraToWorld = lookAt(center, lookat, glm::vec3(0, 1, 0));
				}
				else if (cameraValue.HasMember("transform")) {
					cameraToWorld = readTransform("transform", cameraValue);
				}
				
				glm::ivec2 resolution = readIVector2("resolution", cameraValue);
				float fov = cameraValue["fov"].GetFloat();
				float nearClip = cameraValue["nearclip"].GetFloat();
				float farClip = cameraValue["farclip"].GetFloat();
				
				Camera* cam = new PerspectiveCamera(cameraToWorld, resolution, fov, nearClip, farClip);
				return cam;
			}

			Camera* cam = new PerspectiveCamera();
			return cam;
		}
	};
}