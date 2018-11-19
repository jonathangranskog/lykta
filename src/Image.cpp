#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

using namespace Lykta;

Image<glm::vec3>::Image(int w, int h) {
	data = std::vector<glm::vec3>(w * h);
	width = w;
	height = h;
}

Image<float>::Image(int w, int h) {
	data = std::vector<float>(w * h);
	width = w;
	height = h;
}

Image<glm::vec4>::Image(int w, int h) {
	data = std::vector<glm::vec4>(w * h);
	width = w;
	height = h;
}

Image<glm::vec3>::Image(const std::string& path) {
	int channels = 0;
	float* out = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	data = std::vector<glm::vec3>(width * height);

	#pragma omp parallel for
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			glm::vec3 c = glm::vec3(0.f);
			if (channels > 0)
				c.x = out[j * width * channels + i * channels + 0];
			if (channels > 1)
				c.y = out[j * width * channels + i * channels + 1];
			if (channels > 2)
				c.z = out[j * width * channels + i * channels + 2];
			data[j * width + i] = c;
		}
	}
}

Image<glm::vec4>::Image(const std::string& path) {
	int channels = 0;
	float* out = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	data = std::vector<glm::vec4>(width * height);

	#pragma omp parallel for
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			glm::vec4 c = glm::vec4(0.f, 0.f, 0.f, 1.f);
			if (channels > 0) 
				c.x = out[j * width * channels + i * channels + 0];
			if (channels > 1)
				c.y = out[j * width * channels + i * channels + 1];
			if (channels > 2)
				c.z = out[j * width * channels + i * channels + 2];
			if (channels > 3)
				c.w = out[j * width * channels + i * channels + 3];
			data[j * width + i] = c;
		}
	}
}

Image<float>::Image(const std::string& path) {
	int channels = 0;
	float* out = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
	data = std::vector<float>(width * height);
	
	#pragma omp parallel for
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			// Read only first channel
			data[j * width + i] = out[j * width * channels + i * channels + 0];
		}
	}
}

void Image<glm::vec3>::save(const std::string& path) const {
	stbi_write_hdr(path.c_str(), width, height, 3, (float*)this->data.data());
}

void Image<glm::vec4>::save(const std::string& path) const {
	stbi_write_hdr(path.c_str(), width, height, 4, (float*)this->data.data());
}

void Image<float>::save(const std::string& path) const {
	stbi_write_hdr(path.c_str(), width, height, 1, this->data.data());
}