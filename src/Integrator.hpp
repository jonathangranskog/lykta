#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include "common.h"
#include "Scene.hpp"

namespace Lykta {
	class Scene;
    
    class Integrator {

	public:
		virtual ~Integrator() {}
		
		virtual void preprocess(const Scene* scene) {}

		virtual glm::vec3 evaluate(const Ray& ray, const std::shared_ptr<Scene> scene) = 0;

		virtual void postprocess(const Scene* scene) {}

	};

    class AOIntegrator : public Integrator{
    private:
        float maxlen = 1.f;
    public:
        AOIntegrator() {}
        ~AOIntegrator() {}
        virtual glm::vec3 evaluate(const Ray& ray, const std::shared_ptr<Scene> scene);
    };
}
