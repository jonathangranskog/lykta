#pragma once

#include <memory>
#include "common.h"
#include "Mesh.hpp"

namespace Lykta {

	struct EmitterInteraction {
		glm::vec3 position;
		glm::vec3 origin;
		glm::vec3 direction;
		glm::vec3 normal;
		Ray shadowRay;
		float pdf;

		EmitterInteraction() {}

		EmitterInteraction(const glm::vec3& orig) : origin(orig) {}

		EmitterInteraction(const glm::vec3& pos, const glm::vec3& orig, const glm::vec3& n) {
			position = pos;
			origin = orig;
			normal = n;
		}

		EmitterInteraction(const glm::vec3& pos, const glm::vec3& orig, const glm::vec3& n, const glm::vec3& dir) {
			position = pos;
			origin = orig;
			normal = n;
			direction = dir;
		}

	};

	class Emitter {
	public:
		virtual ~Emitter() {}
		virtual glm::vec3 eval(EmitterInteraction& ei) const = 0;
		virtual glm::vec3 sample(const glm::vec3& s, EmitterInteraction& ei) const = 0;
	};

	class MeshEmitter : public Emitter {
	private:
		MeshPtr mesh;

	public:
		MeshEmitter(MeshPtr m) : mesh(m) {}
		MeshEmitter() {}
		~MeshEmitter() {}

		virtual glm::vec3 eval(EmitterInteraction& ei) const;
		virtual glm::vec3 sample(const glm::vec3& s, EmitterInteraction& ei) const;

	};
}