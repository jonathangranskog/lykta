#pragma once

#include <glm/vec3.hpp>

class Scene;

namespace Lykta {
	class Integrator {

	public:
		virtual ~Integrator() {}
		
		virtual void preprocess(const Scene* scene) {}

		virtual glm::vec3 evaluate(const Ray& ray, const Scene* scene) const = 0;

		virtual void postprocess(const Scene* scene) {}

	};
}
