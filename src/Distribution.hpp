#pragma once

#include <vector>
#include <algorithm>
#include "common.h"
#include "Texture.hpp"

namespace Lykta {
	
	class Distribution1D {
	private:
		std::vector<float> cdf;

	public:

		// Build CDF distribution
		Distribution1D(const std::vector<float>& pdf) {
			cdf = std::vector<float>(pdf.size());

			cdf[0] = pdf[0];
			for (int i = 1; i < pdf.size(); i++) {
				cdf[i] = pdf[i] + cdf[i - 1];
			}

			// normalize
			for (int i = 0; i < cdf.size(); i++) {
				cdf[i] /= cdf[cdf.size() - 1];
			}
		}

		Distribution1D() {}
		~Distribution1D() {}

		inline float pdf(int index) const {
			if (index == 0) return cdf[0];
			return cdf[index] - cdf[index - 1];
		}

		// binary search correct index
		int sample(float r, float& pdf) const {
			int idx = cdf.size();
			auto it = std::lower_bound(cdf.begin(), cdf.end(), r);
			if (it != cdf.end()) idx = std::distance(cdf.begin(), it);
			return idx;
		}

	};

	class Distribution2D {
	private:
		std::vector<Distribution1D> widthDistributions;
		Distribution1D heightDistribution;

	public:

		// Build distributions
		Distribution2D(const std::vector<std::vector<float>>& image) {
			assert(image.size() > 0);
			std::vector<float> widthSums = std::vector<float>(image.size(), 0.f);
			widthDistributions = std::vector<Distribution1D>(image.size());

			#pragma omp parallel for
			for (int j = 0; j < image.size(); j++) {
				for (int i = 0; i < image[0].size(); i++) {
					widthSums[j] += image[j][i];
				}

				widthDistributions[j] = Distribution1D(image[j]);
			}

			heightDistribution = Distribution1D(widthSums);
		}

		Distribution2D() {}
		~Distribution2D() {}

		float pdf(const glm::ivec2& index) const {
			float colpdf = heightDistribution.pdf(index.y);
			float rowpdf = widthDistributions[index.y].pdf(index.x);
			return colpdf * rowpdf;
		}

		glm::ivec2 sample(const glm::vec2& sample, float& pdf) const {
			float colpdf, rowpdf;
			int rowselect = heightDistribution.sample(sample.y, colpdf);
			int colselect = widthDistributions[rowselect].sample(sample.x, rowpdf);
			pdf = colpdf * rowpdf;
			return glm::ivec2(colselect, rowselect);
		}
	};

}