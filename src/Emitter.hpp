#pragma once

#include <memory>
#include "common.h"
#include "Mesh.hpp"

namespace Lykta {

	struct EmitterInteraction {
		glm::vec3 p;
		glm::vec3 ref;
		glm::vec3 wi;
		glm::vec3 n;
		Ray shadowRay;
		float pdf;
	};

	class Emitter {
	public:
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