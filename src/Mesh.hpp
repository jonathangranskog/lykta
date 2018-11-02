#pragma once

#include "common.h"
#include "Material.hpp"
#include <vector>
#include <string>

namespace Lykta {
	
	struct MeshSample {
		glm::vec2 uv;
		glm::vec3 pos;
		glm::vec3 normal;
		float pdf;
	};

	class Mesh {
	public:
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		std::vector<Triangle> triangles;

		std::vector<float> cumulativeAreas;
		EmitterPtr emitter = nullptr;
		MaterialPtr material = nullptr;

		Mesh() {}
		~Mesh() {}

		void constructCDF();
		
		void sample(const glm::vec3& sample, MeshSample& info) const;
		
		// in area measure
		float pdf() const;

		static std::vector<std::shared_ptr<Mesh>> openObj(const std::string& filename);
	};
}