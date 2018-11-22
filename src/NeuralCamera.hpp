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
        float rearRadius;
        float frontZ;
        std::vector<float> means;
        std::vector<float> stds;

    public:
        NeuralCamera(const std::string& filename, glm::ivec2 res, float rr, float fz);
        NeuralCamera() {}

        // Might want to create a function to create a batch of rays
        // so we can use GPU
        virtual glm::vec3 createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const;
    };
}