#include "Scene.hpp"

Lykta::Scene::Scene(const std::string& filename) {
	// TODO: Read a file and construct everything
	// Create a JSON file parser and decide file structure
	// Include .obj file reader
}

glm::vec3 Lykta::Scene::sample(const glm::vec2& pixel) {
	return glm::vec3(pixel.x, pixel.y, 0.f);
}