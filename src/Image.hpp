#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Lykta {

	template <typename T>
	class Image {

	private:
		std::vector<T> data;
		int width, height;

		inline unsigned char linear_to_srgb(float linear) const {
			float v = (linear <= 0.0031308f) ? 12.92f * linear : (1 + 0.055f) * pow(linear, 1.f / 2.4f) - 0.055f;
			float u = 255 * v;
			unsigned char result = std::max(std::min(u, 255.f), 0.f);
			return result;
		}

	public:
		Image(const std::string& path);
		Image(int w, int h);
		Image() {}

		T& operator[](int i) {
			return data[i];
		}

        T read(int i) {
            return data[i];
        }

		void save(const std::string& path) const;

        glm::ivec2 getDims() const {
            return glm::ivec2(width, height);
        }

		T* getData() {
			return data.data();
		}

	};
}
