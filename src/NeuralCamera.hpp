#pragma once

#include "RealisticCamera.hpp"
#include <string>
#include <memory>
#include <torch/torch.h>

namespace Lykta {
    class NeuralCamera : public RealisticCamera {
    private:

        struct Net : torch::nn::Module {
            Net() {
                fc1 = register_module("fc1", torch::nn::Linear(5, 128));
                fc2 = register_module("fc2", torch::nn::Linear(128, 128));
                fc3 = register_module("fc3", torch::nn::Linear(128, 128));
                fc4 = register_module("fc4", torch::nn::Linear(128, 7));
            }

            torch::Tensor forward(torch::Tensor x) {
                x = torch::tanh(fc1->forward(x));
                x = torch::tanh(fc2->forward(x));
                x = torch::tanh(fc3->forward(x));
                x = fc4->forward(x);
                return x;
            }

            torch::nn::Linear fc1{nullptr};
            torch::nn::Linear fc2{nullptr};
            torch::nn::Linear fc3{nullptr};
            torch::nn::Linear fc4{nullptr};            
        };

        std::shared_ptr<torch::jit::script::Module> module = nullptr;
        std::shared_ptr<Net> network = nullptr;
        // These should all be fetched from the python script
        float rearRadius;
        // means: [0, 1, 2, 3, 4] for input
        // means: [5, 6, 7, 8, 9, 10, 11] for output
        std::vector<float> means;
        std::vector<float> stds;

        void normalizeInput(glm::vec2& orig, glm::vec3& dir) const;
        void normalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const;
        void denormalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const;
        glm::vec2 projectToZero(const glm::vec3& sensorPos, const glm::vec3& dir) const;
        void calculateMeanAndStd();
        void train(int epochs, int batchSize);
        std::pair<torch::Tensor, torch::Tensor> generateBatch(int batchSize);

    public:
        NeuralCamera(std::vector<LensInterface> elements, float shift, glm::mat4 camToWorld, glm::ivec2 res);
        NeuralCamera() {}
        ~NeuralCamera() {}

        virtual glm::vec3 createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const;

        virtual void createRayBatch(std::vector<Ray>& rays, std::vector<glm::vec3>& colors, std::vector<RandomSampler>& samplers) const;
    };
}