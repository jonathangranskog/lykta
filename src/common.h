#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#define EPS 1e-6f

namespace Lykta {

	struct Ray {
		glm::vec3 o;
		glm::vec3 d;
		glm::vec2 t;

		Ray() {};
		Ray(glm::vec3 orig, glm::vec3 dir, glm::vec2 tz) : o(orig), d(dir), t(tz) {}
	};

	struct Triangle {
		unsigned x, y, z;

		Triangle() {};
		Triangle(unsigned x_, unsigned y_, unsigned z_) : x(x_), y(y_), z(z_) {}
	};
}
