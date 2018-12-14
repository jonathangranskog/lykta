#pragma once

#include <cassert>
#include <string>
#include <map>
#include <tuple>
#include <filesystem/path.h>
#include <filesystem/resolver.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <fstream>
#include "Camera.hpp"
#include "RealisticCamera.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Emitter.hpp"
#include "Texture.hpp"

namespace Lykta {

	class JSONHelper {
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
			rotate *= (M_PI / 180.f); // make into radians

			// Default transform order
			std::string order = "SRT";
			if (transformValue.HasMember("order")) order = std::string(transformValue["order"].GetString());
			for (char& c : order) {
				if (c == 'S') matrix = glm::scale(matrix, scale);
				else if (c == 'R') {
					// rotation order could also be changed later...
					matrix = glm::rotate(matrix, rotate.x, glm::vec3(1, 0, 0));
					matrix = glm::rotate(matrix, rotate.y, glm::vec3(0, 1, 0));
					matrix = glm::rotate(matrix, rotate.z, glm::vec3(0, 0, 1));
				}
				else if (c == 'T') matrix = glm::translate(matrix, translate);
			}

			return matrix;
		}

        static inline bool getRealPath(std::string& filename, filesystem::path& scenepath) {
            filesystem::path filepath = filesystem::path(filename);
            // If file is not found using relative path, make absolute path
            if (!filepath.is_file()) filepath = scenepath/filepath;
            else {
                filename = filepath.str();
                return true;
            }

            // If file is still not found, then print error.
            if (!filepath.is_file()) {
                std::cerr << filepath.str() << " could not be found -- skipping!" << std::endl;
                return false;
            }

            filename = filepath.str();
            return true;
        }

        static TexturePtr<float> readFloatTexture(const std::string& name, const rapidjson::Value& val,
                                             filesystem::path& scenepath) {
            TexturePtr<float> ptr = nullptr;
            if (val.HasMember(name.c_str())) {
                const rapidjson::Value& file = val[name.c_str()];
                if (file.IsString()) {
                    std::string filename = std::string(file.GetString());
                    if (getRealPath(filename, scenepath)) return TexturePtr<float>(new Texture<float>(filename));
                    else return nullptr;
                } else {
                    std::cout << "Texture: " << name.c_str() << " is not a string!" << std::endl;
                }
            }

            return ptr;
        }

        static TexturePtr<glm::vec3> readVec3Texture(const std::string& name, const rapidjson::Value& val, filesystem::path& scenepath) {
            TexturePtr<glm::vec3> ptr = nullptr;
            if (val.HasMember(name.c_str())) {
                const rapidjson::Value& file = val[name.c_str()];
                if (file.IsString()) {
                    std::string filename = std::string(file.GetString());
                    if (getRealPath(filename, scenepath)) return TexturePtr<glm::vec3>(new Texture<glm::vec3>(filename));
                    else return nullptr;
                } else {
                    std::cout << "Texture: " << name.c_str() << " is not a string!" << std::endl;
                }
            }

            return ptr;
        }

        static TexturePtr<glm::vec4> readVec4Texture(const std::string& name, const rapidjson::Value& val, filesystem::path& scenepath) {
            TexturePtr<glm::vec4> ptr = nullptr;
            if (val.HasMember(name.c_str())) {
                const rapidjson::Value& file = val[name.c_str()];
                if (file.IsString()) {
                    std::string filename = std::string(file.GetString());
                    if (getRealPath(filename, scenepath)) return TexturePtr<glm::vec4>(new Texture<glm::vec4>(filename));
                    else return nullptr;
                } else {
                    std::cout << "Texture: " << name.c_str() << " is not a string!" << std::endl;
                }
            }

            return ptr;
        }

	public:

		static inline std::vector<LensInterface> readLensFile(const std::string& filename) {
			std::vector<LensInterface> interfaces;
			std::ifstream in("E:/Projects/lykta/scenes/spheres/petzval.json");
			std::stringstream sstr;
			sstr << in.rdbuf();
			rapidjson::Document document;
			document.Parse(sstr.str().c_str());
			assert(document.IsObject());
			assert(document.HasMember("interfaces"));
			const rapidjson::Value& arr = document["interfaces"];
			for (rapidjson::SizeType i = 0; i < arr.Size(); i++) {
				float curvature = arr[i]["curvature"].GetFloat();
				float thickness = arr[i]["thickness"].GetFloat();
				float ior = arr[i]["ior"].GetFloat();
				float aperture = arr[i]["aperture"].GetFloat();
				LensInterface element;
				element.curvature = 0.001f * curvature;
				element.thickness = 0.001f * thickness;
				element.eta = ior;
				element.aperture = 0.0005f * aperture;
				interfaces.push_back(element);
			}
			return interfaces;
		}

		// Reads in meshes from JSON document...
		// Also creates mesh emitters if emission is turned on
		// Assigns materials too based on name string
		static std::vector<MeshPtr> readMeshes(rapidjson::Document& document,
			std::map<std::string, std::pair<unsigned, MaterialPtr> >& materials,
			std::vector<EmitterPtr>& emitters,
			filesystem::path& scenepath) {
			std::vector<MeshPtr> meshes = std::vector<MeshPtr>();

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
				
				// If file is not found using relative path, make absolute path
				if (!filepath.is_file()) filepath = scenepath/filepath;

				// If file is still not found, then print error.
				if (!filepath.is_file()) {
					std::cerr << filepath.str() << " could not be found -- skipping!" << std::endl;
					continue;
				}

				std::vector<MeshPtr> imported = Mesh::openObj(filepath.str());
				
				// Get material
				std::string materialLookup = std::string(mat.GetString());
				assert(materials.find(materialLookup) != materials.end());
				bool isEmitter = (maxComponent(materials[materialLookup].second->getEmission())) > 0.f;
				unsigned index = materials[materialLookup].first;
				
				for (MeshPtr m : imported) {
					if (isEmitter) {
						EmitterPtr emitter = EmitterPtr(new MeshEmitter(m));
						m->emitter = emitter;
						emitters.push_back(emitter);
					}

					m->material = materials[materialLookup].second;
				}

				meshes.insert(meshes.end(), imported.begin(), imported.end());
			}

			return meshes;
		}

        static std::map<std::string, std::pair<unsigned, MaterialPtr>>readMaterials(rapidjson::Document& document, filesystem::path& scenepath) {
			std::map<std::string, std::pair<unsigned, MaterialPtr> > materialMap;

			if (!document.HasMember("materials")) return materialMap;

			const rapidjson::Value& arr = document["materials"];

			for (size_t i = 0; i < arr.Size(); i++) {
				assert(arr[i].HasMember("name"));
				
				glm::vec3 diffuseColor;
				if (arr[i].HasMember("diffuseColor")) diffuseColor = readVector3("diffuseColor", arr[i]);
				else diffuseColor = glm::vec3(1.f);

				glm::vec3 emissiveColor;
				if (arr[i].HasMember("emissiveColor")) emissiveColor = readVector3("emissiveColor", arr[i]);
				else emissiveColor = glm::vec3(0.f);

				float roughness;
				if (arr[i].HasMember("roughness")) roughness = arr[i]["roughness"].GetFloat();
				else roughness = 0.3f;
				roughness = clamp(roughness, 0.05f, 1);

				float specular;
				if (arr[i].HasMember("specular")) specular = arr[i]["specular"].GetFloat();
				else specular = 0.f;
				specular = clamp(specular, 0, 1);

				float specularTint;
				if (arr[i].HasMember("specularTint")) specularTint = arr[i]["specularTint"].GetFloat();
				else specularTint = 0.f;
				specularTint = clamp(specularTint, 0, 1);

				float ior;
				if (arr[i].HasMember("ior")) ior = arr[i]["ior"].GetFloat();
				else ior = 1.33f;

				bool twosided = false;
				if (arr[i].HasMember("twoSided")) twosided = arr[i]["twoSided"].GetBool();

                // Read textures
                TexturePtr<glm::vec3> diffuseTexture = nullptr;
                if (arr[i].HasMember("diffuseTexture")) diffuseTexture = readVec3Texture("diffuseTexture", arr[i], scenepath);

                TexturePtr<float> specularTexture = nullptr;
                if (arr[i].HasMember("specularTexture")) specularTexture = readFloatTexture("specularTexture", arr[i], scenepath);

                TexturePtr<float> tintTexture = nullptr;
                if (arr[i].HasMember("tintTexture")) tintTexture = readFloatTexture("tintTexture", arr[i], scenepath);

                TexturePtr<float> roughnessTexture = nullptr;
                if (arr[i].HasMember("roughnessTexture")) roughnessTexture = readFloatTexture("roughnessTexture", arr[i], scenepath);

				TexturePtr<float> opacityTexture = nullptr;
				if (arr[i].HasMember("opacityTexture")) opacityTexture = readFloatTexture("opacityTexture", arr[i], scenepath);

                // Create material
                MaterialPtr mat = MaterialPtr(new SurfaceMaterial(diffuseColor, emissiveColor,
                                                                  specular, specularTint,
                                                                  roughness, ior, twosided, diffuseTexture,
                                                                  specularTexture, tintTexture,
                                                                  roughnessTexture, opacityTexture));

				const rapidjson::Value& name = arr[i]["name"];
				assert(name.IsString());
				materialMap[name.GetString()] = std::pair<unsigned, MaterialPtr>((unsigned)i, mat);
			}

			return materialMap;
		}

		static Camera* readCamera(rapidjson::Document& document, filesystem::path& scenepath) {
			if (!document.HasMember("camera")) {
				Camera* cam = new PerspectiveCamera();
				return cam;
			}
				
			const rapidjson::Value& cameraValue = document["camera"];
			assert(cameraValue.HasMember("type"));
			
			const std::string type = cameraValue["type"].GetString();

			if (type == "PerspectiveCamera" || type == "RealisticCamera") {
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
				
				if (type == "PerspectiveCamera") {
					float fov = cameraValue["fov"].GetFloat();
					float nearClip = cameraValue["nearclip"].GetFloat();
					float farClip = cameraValue["farclip"].GetFloat();
					float apertureRadius = (cameraValue.HasMember("apertureRadius")) ? cameraValue["apertureRadius"].GetFloat() : 0.f;
					float focusDistance = (cameraValue.HasMember("focusDistance")) ? cameraValue["focusDistance"].GetFloat() : 1.f;

					Camera* cam = new PerspectiveCamera(cameraToWorld, resolution, fov, nearClip, farClip, apertureRadius, focusDistance);
					return cam;
				}
				else if (type == "RealisticCamera") {
					float sensorShift = 0.f;
					if (cameraValue.HasMember("sensorShift")) sensorShift = cameraValue["sensorShift"].GetFloat() * 0.001f;

					assert(cameraValue.HasMember("lenses"));
					std::string lensFile = cameraValue["lenses"].GetString();
					assert(getRealPath(lensFile, scenepath));
					std::vector<LensInterface> interfaces = readLensFile(lensFile);
					Camera* cam = new RealisticCamera(interfaces, sensorShift, cameraToWorld, resolution);
					return cam;
				}
				
			}

			Camera* cam = new PerspectiveCamera();
			return cam;
		}

		static EmitterPtr readEnvironment(rapidjson::Document& document,
									std::vector<EmitterPtr>& emitters,
									filesystem::path& scenepath) {
			if (!document.HasMember("environment")) return nullptr;

			const rapidjson::Value& environmentObject = document["environment"];
			assert(environmentObject.HasMember("map"));
			assert(environmentObject["map"].IsString());

			std::string filename = environmentObject["map"].GetString();
			// If file exists
			if (getRealPath(filename, scenepath)) {
				TexturePtr<glm::vec3> map = TexturePtr<glm::vec3>(new Texture<glm::vec3>(filename));
				float intensity = 1.f;
				if (environmentObject.HasMember("intensity") && environmentObject["intensity"].IsFloat()) 
					intensity = environmentObject["intensity"].GetFloat();
				EmitterPtr emitter = EmitterPtr(new EnvironmentEmitter(map, intensity));
				emitters.push_back(emitter);
				return emitter;
			}
			else {
				return nullptr;
			}
		}
	};
}
