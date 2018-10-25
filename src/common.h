#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#define EPS 1e-6f
#define M_PI 3.14159265359
#define INV_PI 0.31830988618

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

	inline glm::mat4 lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
		glm::vec3 z = glm::normalize(center - eye);
		glm::vec3 x = glm::normalize(glm::cross(z, up));
		glm::vec3 y = glm::normalize(glm::cross(x, z));

		glm::mat4 result = glm::mat4();
		result[0] = glm::vec4(x, 0);
		result[1] = glm::vec4(y, 0);
		result[2] = glm::vec4(z, 0);
		result[3] = glm::vec4(eye, 1);

		return result;
	}
}
