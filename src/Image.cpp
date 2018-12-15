#include "Image.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include <algorithm>

using namespace Lykta;

template <>
Image<glm::vec3>::Image(int w, int h) {
	data = std::vector<glm::vec3>(w * h);
	width = w;
	height = h;
}

template <>
Image<float>::Image(int w, int h) {
	data = std::vector<float>(w * h);
	width = w;
	height = h;
}

template <>
Image<glm::vec4>::Image(int w, int h) {
	data = std::vector<glm::vec4>(w * h);
	width = w;
	height = h;
}

template <>
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

	delete[] out;
}

template <>
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

	delete[] out;
}

template <>
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

	delete[] out;
}

template <>
void Image<glm::vec3>::save(const std::string& path) const {
	std::vector<unsigned char> image = std::vector<unsigned char>(width * height * 3);
	for (int i = 0; i < width * height; i++) {
		unsigned char r = linear_to_srgb(data[i].x);
		unsigned char g = linear_to_srgb(data[i].y);
		unsigned char b = linear_to_srgb(data[i].z);

		image[i * 3 + 0] = r;
		image[i * 3 + 1] = g;
		image[i * 3 + 2] = b;
	}

	stbi_write_png(path.c_str(), width, height, 3, image.data(), 0);
}

template <>
void Image<glm::vec4>::save(const std::string& path) const {
	std::vector<unsigned char> image = std::vector<unsigned char>(width * height * 4);
	for (int i = 0; i < width * height; i++) {
		unsigned char r = linear_to_srgb(data[i].x);
		unsigned char g = linear_to_srgb(data[i].y);
		unsigned char b = linear_to_srgb(data[i].z);
		unsigned char a = linear_to_srgb(data[i].w);

		image[i * 4 + 0] = r;
		image[i * 4 + 1] = g;
		image[i * 4 + 2] = b;
		image[i * 4 + 3] = a;
	}

	stbi_write_png(path.c_str(), width, height, 4, image.data(), 0);
}

template <>
void Image<float>::save(const std::string& path) const {
	std::vector<unsigned char> image = std::vector<unsigned char>(width * height);
	for (int i = 0; i < width * height; i++) {
		image[i] = linear_to_srgb(data[i]);
	}

	stbi_write_png(path.c_str(), width, height, 1, image.data(), 0);
}