#include "NeuralCamera.hpp"
#include <torch/script.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

using namespace Lykta;

NeuralCamera::NeuralCamera(std::vector<LensInterface> elements,  float shift, glm::mat4 camToWorld, glm::ivec2 res) {
    sensorShift = shift;
    resolution = res;
    aspect = resolution.x / (float)resolution.y;
    cameraToWorld = camToWorld;
    sensorSize = glm::vec2(0.024f, 0.024f * 1.f/aspect);
    
    interfaces = elements;

    frontZ = 0.f;
    for (int i = 0; i < interfaces.size(); i++) {
        frontZ += interfaces[i].thickness;
    }

    rearRadius = interfaces.back().aperture;

    means = std::vector<float>(12, 0.f);
    stds = std::vector<float>(12, 1.f);

    train(1000, 512, 5);
}

// TODO: Better way of doing this........
void NeuralCamera::normalizeInput(glm::vec2& orig, glm::vec3& dir) const {
    orig.x = (orig.x - means[0]) / stds[0];
    orig.y = (orig.y - means[1]) / stds[1];
    dir.x = (dir.x - means[2]) / stds[2];
    dir.y = (dir.y - means[3]) / stds[3];
    dir.z = (dir.z - means[4]) / stds[4];
}

void NeuralCamera::normalizeOutput(float& success, glm::vec3& orig, glm::vec3& dir) const {
    success = (success - means[5]) / stds[5];
    orig.x = (orig.x - means[6]) / stds[6];
    orig.y = (orig.y - means[7]) / stds[7];
    orig.z = (orig.z - means[8]) / stds[8];
    dir.x = (dir.x - means[9]) / stds[9];
    dir.y = (dir.y - means[10]) / stds[10];
    dir.z = (dir.z - means[11]) / stds[11];
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

// TODO: Generate a block of input values and evaluate simultaneously
void NeuralCamera::createRayBatch(std::vector<Ray>& rays, std::vector<glm::vec3>& colors, std::vector<RandomSampler>& samplers) const {
    rays.assign(resolution.x * resolution.y, Ray());
    colors.assign(resolution.x * resolution.y, glm::vec3(0.f));

    if (networks.size() == 0) return;

    // Create input
    torch::Tensor input = torch::ones({resolution.x * resolution.y, 5});

    #pragma omp parallel for
    for (int it = 0; it < resolution.x * resolution.y; it++) {
        int i = it % resolution.x;
        int j = it / resolution.x;
        int thread = omp_get_thread_num();
        RandomSampler* sampler = &samplers[thread];

        glm::vec2 pixel = glm::vec2(i, j) + sampler->next2D();
        glm::vec2 sample = sampler->next2D();

        // Create ray and create input
        glm::vec2 np = glm::vec2(pixel.x / resolution.x, 1.f - pixel.y / resolution.y);
        glm::vec3 pFilm = glm::vec3(np * sensorSize - sensorSize / 2.f, sensorShift);
        glm::vec3 pRear = glm::vec3(-rearRadius + sample.x * 2 * rearRadius, -rearRadius + sample.y * 2 * rearRadius, frontZ);
        glm::vec3 direction = pRear - pFilm;
        direction = glm::normalize(direction);
        glm::vec2 sensorPos = glm::vec2(pFilm.x, pFilm.y);
        normalizeInput(sensorPos, direction);

        input[it][0] = sensorPos.x;
        input[it][1] = sensorPos.y;
        input[it][2] = direction.x;
        input[it][3] = direction.y;
        input[it][4] = direction.z;
    }

    // Evaluate networks
    at::Tensor output = torch::zeros({resolution.x * resolution.y, 7});
    for (int i = 0; i < networks.size(); i++) {
        at::Tensor result = networks[i]->forward(input).to(at::kFloat);
        output += result;
    }
    at::Tensor size = torch::zeros({1});
    size[0] = (float)networks.size();
    output /= size;
    float* data = output.data<float>();

    // Set outgoing rays
    //#pragma omp parallel for
    for (int it = 0; it < resolution.x * resolution.y; it++) {
        float success = data[it * 7];
        glm::vec3 orig = glm::vec3(data[it * 7 + 1], data[it * 7 + 2], data[it * 7 + 3]);
        glm::vec3 dir = glm::vec3(data[it * 7 + 4], data[it * 7 + 5], data[it * 7 + 6]);
        denormalizeOutput(success, orig, dir);
        bool passed = success > 0.5f;
        dir = glm::normalize(dir);
        rays[it].o = glm::vec3(cameraToWorld * glm::vec4(orig, 1));
        rays[it].d = glm::vec3(cameraToWorld * glm::vec4(dir, 0));
        rays[it].t = glm::vec2(EPS, INFINITY);
        colors[it] = glm::vec3((float)passed);
    }
}

void NeuralCamera::calculateMeanAndStd() {
    std::cout << "Approximating means and stds..." << std::endl;
    int n = 100000;
    
    at::Tensor input = at::ones({n, 5});
    at::Tensor output = at::ones({n, 7});
    at::Tensor randnums = at::rand({n * 4});
    float* rnd = randnums.data<float>();

    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        // Generate rays
        float resx = rnd[i * 4] * resolution.x;
        float resy = rnd[i * 4 + 1] * resolution.y;
        float sx = rnd[i * 4 + 2];
        float sy = rnd[i * 4 + 3];
        Ray sensor = generateSensorRay(glm::vec2(resx, resy), 
                                       glm::vec2(sx, sy));
        Ray out;
        bool success = trace(sensor, out);

        // Set input
        input[i][0] = sensor.o.x;
        input[i][1] = sensor.o.y;
        input[i][2] = sensor.d.x;
        input[i][3] = sensor.d.y;
        input[i][4] = sensor.d.z;

        // Set output
        output[i][0] = (float)success;
        output[i][1] = out.o.x;
        output[i][2] = out.o.y;
        output[i][3] = out.o.z;
        output[i][4] = out.d.x;
        output[i][5] = out.d.y;
        output[i][6] = out.d.z;
    }

    at::Tensor sensorMean = at::mean(input, 0, at::kFloat);
    at::Tensor outMean = at::mean(output, 0, at::kFloat);
    at::Tensor sensorStd = at::std(input, 0, true, false);
    at::Tensor outStd = at::std(output, 0, true, false);
    float* smptr = sensorMean.data<float>();
    float* omptr = outMean.data<float>();
    float* ssptr = sensorStd.data<float>();
    float* osptr = outStd.data<float>();

    for (int i = 0; i < 5; i++) {
        means[i] = smptr[i];
        stds[i] = ssptr[i];
    }

    for (int i = 0; i < 7; i++) {
        means[i + 5] = omptr[i];
        stds[i + 5] = osptr[i];
    }
}

// This should create two tensors input and output of size batchSize
std::pair<torch::Tensor, torch::Tensor> NeuralCamera::generateBatch(int batchSize) {
    // Initialize tensors
    torch::Tensor input = torch::ones({batchSize, 5});
    torch::Tensor output = torch::ones({batchSize, 7});
    at::Tensor randnums = at::rand({batchSize * 4});
    float* rnd = randnums.data<float>();

    #pragma omp parallel for
    for (int i = 0; i < batchSize; i++) {
        // Generate rays
        Ray sensor = generateSensorRay(glm::vec2(rnd[i * 4] * resolution.x, 
                                                 rnd[i * 4 + 1] * resolution.y),
                                       glm::vec2(rnd[i * 4 + 2], rnd[i * 4 + 3]));
        Ray out;
        bool success = trace(sensor, out);
        
        // Normalize input
        glm::vec2 sensorOrig = glm::vec2(sensor.o.x, sensor.o.y);
        glm::vec3 sensorDir = sensor.d;
        normalizeInput(sensorOrig, sensorDir);

        // Normalize output
        float succ = (float)success;
        glm::vec3 outOrig = out.o;
        glm::vec3 outDir = out.d;
        normalizeOutput(succ, outOrig, outDir);

        // Set input
        input[i][0] = sensorOrig.x;
        input[i][1] = sensorOrig.y;
        input[i][2] = sensorDir.x;
        input[i][3] = sensorDir.y;
        input[i][4] = sensorDir.z;

        // Set output
        output[i][0] = succ;
        output[i][1] = outOrig.x;
        output[i][2] = outOrig.y;
        output[i][3] = outOrig.z;
        output[i][4] = outDir.x;
        output[i][5] = outDir.y;
        output[i][6] = outDir.z;
    }

    return std::make_pair(input, output);
}

void NeuralCamera::train(int epochs, int batchSize, int numNetworks) {
    
    // Approximate means and stds to train on 0 mean and unit std.
    calculateMeanAndStd();

    // Train #numNetworks networks
    for (int i = 0; i < numNetworks; i++) {
        // Create network and optimizer
        NetPtr network = NetPtr(new Net());
        torch::optim::Adam optimizer(network->parameters(), 0.01f);

        // Train
        for (size_t epoch = 1; epoch <= epochs; ++epoch) {
            optimizer.zero_grad();
            std::pair<torch::Tensor, torch::Tensor> batch = generateBatch(batchSize);
            auto prediction = network->forward(batch.first);
            auto loss = torch::mse_loss(prediction, batch.second);
            loss.backward();
            optimizer.step();
            std::cout << "Training NeuralCamera epoch " << epoch << ": " << loss << std::endl;
        }

        // Add to vector of networks
        networks.push_back(network);
    }
}