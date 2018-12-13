#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include "common.h"
#include "RandomPool.hpp"
#include "Scene.hpp"

namespace Lykta {
	class Scene;
    class Integrator {

	public:
		enum Type {
			BSDF = 0,
			AO = 1,
			PT = 2
		};

		virtual ~Integrator() {}
		
		virtual void preprocess(const std::shared_ptr<Scene> scene) {}

		virtual glm::vec3 evaluate(const Ray& ray, const std::shared_ptr<Scene> scene) = 0;

		virtual void postprocess(const std::shared_ptr<Scene> scene) {}

	};

    class AOIntegrator : public Integrator{
    private:
        float maxlen = 10.f;
    public:
        AOIntegrator() {}
        ~AOIntegrator() {}
        virtual glm::vec3 evaluate(const Ray& ray, const std::shared_ptr<Scene> scene);
    };

	class BSDFIntegrator : public Integrator {
	private:

	public:
		BSDFIntegrator() {}
		~BSDFIntegrator() {}
		virtual glm::vec3 evaluate(const Ray& ray, const std::shared_ptr<Scene> scene);
	};

	class Unidirectional : public Integrator {
	private:

	public:
		Unidirectional() {}
		~Unidirectional() {}
		virtual glm::vec3 evaluate(const Ray& ray, const std::shared_ptr<Scene> scene);
	};
}
