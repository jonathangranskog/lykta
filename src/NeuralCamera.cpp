#include "NeuralCamera.hpp"
#include <torch/script.h>

using namespace Lykta;

// TODO: Implement realistic camera model and then make NeuralCamera inherit from that
// Also add in training and everything into NeuralCamera...

NeuralCamera::NeuralCamera(const std::string& filename, glm::ivec2 res, float rr, float fz, std::vector<float> avgs, std::vector<float> devs) {
    module = torch::jit::load(filename);
    resolution = res;
    aspect = resolution.x / (float)resolution.y;
    sensorSize = glm::vec2(0.024f, 0.024f * 1.f/aspect);
    frontZ = fz;
    rearRadius = rr;
    cameraToWorld = lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    means = avgs;
    stds = devs;
}

// TODO: Better way of doing this........
void NeuralCamera::normalizeInput(glm::vec2& orig, glm::vec3& dir) const {
    orig.x = (orig.x - means[0]) / stds[0];
    orig.y = (orig.y - means[1]) / stds[1];
    dir.x = (dir.x - means[2]) / stds[2];
    dir.y = (dir.y - means[3]) / stds[3];
    dir.z = (dir.z - means[4]) / stds[4];
}

void NeuralCamera::denormalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const {
    success = (success - means[5]) / stds[5];
    orig.x = (orig.x - means[6]) / stds[6];
    orig.y = (orig.y - means[7]) / stds[7];
    orig.z = (orig.z - means[8]) / stds[8];
    dir.x = (dir.x - means[9]) / stds[9];
    dir.y = (dir.y - means[10]) / stds[10];
    dir.z = (dir.z - means[11]) / stds[11];
}

glm::vec3 NeuralCamera::createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const {
    if (module) {
        std::vector<torch::jit::IValue> inputs;
        // Create ray and create input
        glm::vec2 np = glm::vec2(pixel.x / resolution.x, pixel.y / resolution.y);
        glm::vec2 pFilm = np * sensorSize - sensorSize / 2.f;
        glm::vec3 pRear = glm::vec3(-rearRadius + sample.x * 2 * rearRadius, -rearRadius + sample.y * 2 * rearRadius, frontZ);
        glm::vec3 direction = pRear - glm::vec3(pFilm, 0.f);
        direction = glm::normalize(direction);

        // Create input tensor
        torch::Tensor input = torch::ones({5});
        input[0] = pFilm.x;
        input[1] = pFilm.y;
        input[2] = direction.x;
        input[3] = direction.y;
        input[4] = direction.z;
        inputs.push_back(input);

        // Run through neural network
        at::Tensor output = module->forward(inputs).toTensor();
        output = output.to(at::kFloat);
        float* data = output.data<float>();

        // Extract output [success, orig.x, orig.y, orig.z, dir.x, dir.y, dir.z]
        bool success = data[0] > 0.5f;
        if (success) {
            glm::vec3 pos = glm::vec3(data[1], data[2], data[3]);
            glm::vec3 dir = glm::vec3(data[4], data[5], data[6]);
            ray.o = glm::vec3(cameraToWorld * glm::vec4(pos, 1));
            ray.d = glm::vec3(cameraToWorld * glm::vec4(dir, 0));
            ray.t = glm::vec2(EPS, INFINITY);
            return glm::vec3(1.f);
        }
    }

    // Failed -- return black and invalid ray
    ray.o = glm::vec3(0.f);
    ray.d = glm::vec3(0, 0, 1);
    ray.t = glm::vec2(0.f);
    return glm::vec3(0.f);  
}