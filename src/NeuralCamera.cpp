#include "NeuralCamera.hpp"
#include <torch/script.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Lykta;

NeuralCamera::NeuralCamera(const std::string& modelFile, const std::string& dataFile, glm::mat4 camToWorld, glm::ivec2 res, float shift) {
    module = torch::jit::load("/Users/jonathan/Documents/NeuroLens/lenses/petzval.pt");
    resolution = res;
    aspect = resolution.x / (float)resolution.y;
    sensorSize = glm::vec2(0.024f, 0.024f * 1.f/aspect);
    sensorShift = shift;
    cameraToWorld = camToWorld;
    
    std::ifstream file;
    file.open("/Users/jonathan/Documents/NeuroLens/lenses/petzval.txt");
    if (file.is_open()) {
        std::string meanLine, stdLine, frontZLine, rearRadiusLine;
        std::getline(file, meanLine);
        std::getline(file, stdLine);
        std::getline(file, frontZLine);
        std::getline(file, rearRadiusLine);
        std::stringstream meanss(meanLine);
        std::string token;
        while (std::getline(meanss, token, ' ')) {
            means.push_back(std::stof(token));
        }
        std::stringstream stdss(stdLine);
        while (std::getline(stdss, token, ' ')) {
            stds.push_back(std::stof(token));
        }
        frontZ = std::stof(frontZLine);
        rearRadius = std::stof(rearRadiusLine);
    }
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
    success = success * stds[5] + means[5];
    orig.x = orig.x * stds[6] + means[6];
    orig.y = orig.y * stds[7] + means[7];
    orig.z = orig.z * stds[8] + means[8];
    dir.x = dir.x * stds[9] + means[9];
    dir.y = dir.y * stds[10] + means[10];
    dir.z = dir.z * stds[11] + means[11];
}

// Project sensor position that is shifted off zero to z=0
glm::vec2 NeuralCamera::projectToZero(const glm::vec3& sensorPos, const glm::vec3& dir) const {
    if (sensorPos.z < EPS) return glm::vec2(sensorPos.x, sensorPos.y);
    float t = -sensorPos.z / dir.z;
    glm::vec3 proj = sensorPos + t * dir;
    return glm::vec2(proj.x, proj.y);
}

glm::vec3 NeuralCamera::createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const {
    
    if (module) {
        std::vector<torch::jit::IValue> inputs;
        // Create ray and create input
        glm::vec2 np = glm::vec2(pixel.x / resolution.x, 1.f - pixel.y / resolution.y);
        glm::vec3 pFilm = glm::vec3(np * sensorSize - sensorSize / 2.f, sensorShift);
        glm::vec3 pRear = glm::vec3(-rearRadius + sample.x * 2 * rearRadius, -rearRadius + sample.y * 2 * rearRadius, frontZ);
        glm::vec3 direction = pRear - pFilm;
        direction = glm::normalize(direction);
        glm::vec2 zeroProjection = projectToZero(pFilm, direction);

        normalizeInput(zeroProjection, direction);

        // Create input tensor
        torch::Tensor input = torch::ones({5});
        input[0] = zeroProjection.x;
        input[1] = zeroProjection.y;
        input[2] = direction.x;
        input[3] = direction.y;
        input[4] = direction.z;
        inputs.push_back(input);

        // Run through neural network
        at::Tensor output = module->forward(inputs).toTensor();
        output = output.to(at::kFloat);
        float* data = output.data<float>();
        
        // Extract output [success, orig.x, orig.y, orig.z, dir.x, dir.y, dir.z]
        float success = data[0];
        glm::vec3 pos = glm::vec3(data[1], data[2], data[3]);
        glm::vec3 dir = glm::vec3(data[4], data[5], data[6]);

        denormalizeOutput(success, pos, dir);

        bool passed = success > 0.5f;

        if (passed) {
            dir = glm::normalize(dir);
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