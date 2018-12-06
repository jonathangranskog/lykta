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
                x = torch::relu(fc1->forward(x));
                x = torch::relu(fc2->forward(x));
                x = torch::relu(fc3->forward(x));
                x = fc4->forward(x);
                return x;
            }

            torch::nn::Linear fc1{nullptr};
            torch::nn::Linear fc2{nullptr};
            torch::nn::Linear fc3{nullptr};
            torch::nn::Linear fc4{nullptr};            
        };

        typedef std::shared_ptr<Net> NetPtr;

        std::vector<NetPtr> networks = std::vector<NetPtr>();
        float rearRadius;
        
        // means: [0, 1, 2, 3, 4] for input
        // means: [5, 6, 7, 8, 9, 10, 11] for output
        std::vector<float> means;
        std::vector<float> stds;

        void normalizeInput(glm::vec2& orig, glm::vec3& dir) const;
        void normalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const;
        void denormalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const;
        void calculateMeanAndStd();
        void train(int epochs, int batchSize, int numNetworks = 1);
        std::pair<torch::Tensor, torch::Tensor> generateBatch(int batchSize);

    public:
        NeuralCamera(std::vector<LensInterface> elements, float shift, glm::mat4 camToWorld, glm::ivec2 res);
        NeuralCamera() {}
        ~NeuralCamera() {}

        // Only replace ray batch function of realistic camera
        virtual void createRayBatch(std::vector<Ray>& rays, std::vector<glm::vec3>& colors, std::vector<RandomSampler>& samplers) const;
    };
}