#pragma once

#include <vector>

namespace Lykta {
	class Renderer {
	private:
		std::vector<float> image;
		unsigned width, height;

	public:
		void render();

		const std::vector<float>& getImage() const {
			return image;
		}

	};
}