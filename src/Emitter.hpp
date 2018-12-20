#pragma once

#include <memory>
#include "common.h"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Distribution.hpp"

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

	class EnvironmentEmitter : public Emitter {
	private:
		TexturePtr<glm::vec3> map;
		Distribution2D samplingDistribution;
		glm::ivec2 dims;
		float intensity;
		float rotation;

		glm::vec2 dir2uv(const glm::vec3& dir) const;
		glm::vec3 uv2dir(const glm::vec2& uv) const;
		glm::vec2 uv2img(const glm::vec2& uv) const;
		glm::vec2 img2uv(const glm::vec2& img) const;
		inline glm::vec3 rotateDir(const glm::vec3& dir) const;
	
	public:
		EnvironmentEmitter(TexturePtr<glm::vec3> m, float intens = 1.f, float rot = 0.f);
		EnvironmentEmitter() {}
		~EnvironmentEmitter() {}

		virtual glm::vec3 eval(EmitterInteraction& ei) const;
		virtual glm::vec3 sample(const glm::vec3& s, EmitterInteraction& ei) const;
	};
}