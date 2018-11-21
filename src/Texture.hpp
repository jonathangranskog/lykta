#pragma once
#include "common.h"
#include "Image.hpp"
#include <memory>

namespace Lykta {

    template <typename T>
    class Texture {
    private:
        std::unique_ptr<Image<T>> image;
    public:
        Texture(const std::string& path);
        Texture() {}
        ~Texture() {}

        glm::vec2 uvNormalize(const glm::vec2& uv) const;
        int getIndex(const glm::vec2& st) const;

        // Read value from image based on UV coord
        T eval(const glm::vec2& uv) const;

    };

    template <typename T>
    using TexturePtr = std::shared_ptr<Texture<T>>;
}
