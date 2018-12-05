#pragma once

#include "Camera.hpp"
#include <vector>
#include <iostream>

namespace Lykta {

    struct LensInterface {
        float curvature;
        float thickness;
        float eta;
        float aperture;
    };

    class RealisticCamera : public Camera {
    protected:
        std::vector<LensInterface> interfaces;
        glm::vec2 sensorSize;
        float frontZ;
        float sensorShift;

    protected:

        Ray generateSensorRay(const glm::vec2& pixel, const glm::vec2& sample) const;
        bool testAperture(int index, const glm::vec2& hit) const;
        bool intersectElement(int index, float z, const Ray& ray, float& t, glm::vec3& normal) const;

    public:
        RealisticCamera(std::vector<LensInterface> elements, float shift, glm::mat4 camToWorld, glm::ivec2 res);
        RealisticCamera() {}
        ~RealisticCamera() {}

        bool trace(const Ray& in, Ray& out) const;

        inline float getRearZ() const {
            return interfaces.back().thickness;
        }

        inline float getRearAperture() const {
            return interfaces.back().aperture;
        }

        virtual glm::vec3 createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const;
    };

}