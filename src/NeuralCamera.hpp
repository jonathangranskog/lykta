#pragma once

#include "Camera.hpp"
#include <string>
#include <memory>
#include <torch/torch.h>

namespace Lykta {
    class NeuralCamera : public Camera {
    private:
        std::shared_ptr<torch::jit::script::Module> module = nullptr;
        glm::vec2 sensorSize = glm::vec2(0.024f, 0.024f);
        // These should all be fetched from the python script
        float rearRadius;
        float frontZ;
        // means: [0, 1, 2, 3, 4] for input
        // means: [5, 6, 7, 8, 9, 10, 11] for output
        std::vector<float> means;
        std::vector<float> stds;

        void normalizeInput(glm::vec2& orig, glm::vec3& dir) const;
        void denormalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const;

    public:
        NeuralCamera(const std::string& filename, glm::ivec2 res, float rr, float fz, std::vector<float> avgs, std::vector<float> devs);
        NeuralCamera() {}

        // Might want to create a function to create a batch of rays
        // so we can use GPU
        virtual glm::vec3 createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const;
    };
}