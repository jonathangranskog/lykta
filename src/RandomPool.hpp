#pragma once

#include "random.h"
#include "omp.h"
#include <vector>

// Class for easily getting random numbers
// just calls the sampler of the thread
namespace Lykta {
	class RND {
	public:
		static std::vector<RandomSampler> samplers;

		static void init() {
			// Init samplers
			samplers.clear();
			samplers = std::vector<RandomSampler>(omp_get_max_threads());
			for (int i = 0; i < omp_get_max_threads(); i++) {
				samplers[i].seed(i);
			}
		}
 
		static inline float next1D() {
			int thread = omp_get_thread_num();
			return samplers[thread].next();
		}

		static inline glm::vec2 next2D() {
			int thread = omp_get_thread_num();
			return samplers[thread].next2D();
		}

		static inline glm::vec3 next3D() {
			int thread = omp_get_thread_num();
			return samplers[thread].next3D();
		}

	};
}
