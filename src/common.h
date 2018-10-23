#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#define EPS 1e-6

namespace Lykta {

	struct Ray {
		glm::vec3 o;
		glm::vec3 d;
		float tmin;
		float tmax;

		Ray() {};
		Ray(glm::vec3 orig, glm::vec3 dir) : o(orig), d(dir) {}
	};

	struct Vertex {
		glm::vec2 uv;
		glm::vec3 pos;
		glm::vec3 n;

		Vertex() {};
		Vertex(const glm::vec3& p, const glm::vec3& n_, const glm::vec3& tx) : pos(p), n(n_), uv(tx) {}
	};
}
