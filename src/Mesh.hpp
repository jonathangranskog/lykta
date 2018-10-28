#pragma once

#include "common.h"
#include <vector>
#include <string>

namespace Lykta {
	class Mesh {	
	public:
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		std::vector<Triangle> triangles;
		unsigned materialId = 0;

		Mesh() {}
		~Mesh() {}

		void setTriangles(std::vector<Triangle>& tris) {
			triangles = tris;
		}

		void setPositions(std::vector<glm::vec3>& p) {
			positions = p;
		}

		void setNormals(std::vector<glm::vec3>& n) {
			normals = n;
		}

		void setTextureCoordinates(std::vector<glm::vec2>& tex) {
			texcoords = tex;
		}

		static std::vector<Mesh> openObj(const std::string& filename);
	};
}