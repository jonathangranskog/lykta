#pragma once
#include <vector>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Lykta {

	template <typename T>
	class Image {

	private:
		std::vector<T> data;
		int width, height;

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
