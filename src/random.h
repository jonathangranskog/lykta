#pragma once

/*
 * Tiny self-contained version of the PCG Random Number Generation for C++
 * put together from pieces of the much larger C/C++ codebase.
 * Wenzel Jakob, February 2015
 *
 * The PCG random number generator was developed by Melissa O'Neill
 * <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 */

// https://github.com/wjakob/pcg32
// Changes by Jonathan Granskog: Removal of some functions and added glm vec2 function
// also changed name to random.h

#include <inttypes.h>
#include <glm/vec2.hpp>

#define PCG32_DEFAULT_STATE  0x853c49e6748fea9bULL
#define PCG32_DEFAULT_STREAM 0xda3e39cb94b95bdbULL
#define PCG32_MULT           0x5851f42d4c957f2dULL

namespace Lykta {
	struct RandomSampler {

		uint64_t state, inc;

		RandomSampler() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}

		void seed(uint64_t initstate, uint64_t initseq = 1) {
			state = 0U;
			inc = (initseq << 1u) | 1u;
			nextUInt();
			state += initstate;
			nextUInt();
		}

		uint32_t nextUInt() {
			uint64_t oldstate = state;
			state = oldstate * PCG32_MULT + inc;
			uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
			uint32_t rot = (uint32_t)(oldstate >> 59u);
			return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
		}

		float next() {
			union {
				uint32_t u;
				float f;
			} x;
			x.u = (nextUInt() >> 9) | 0x3f800000u;
			return x.f - 1.0f;
		}

		glm::vec2 next2D() {
			return glm::vec2(next(), next());
		}
	};
}
