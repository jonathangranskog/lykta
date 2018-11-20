#include "Texture.hpp"

using namespace Lykta;

template<>
Texture<glm::vec3>::Texture(const std::string &path) {
    image = std::unique_ptr<Image<glm::vec3>>(new Image<glm::vec3>(path));
}

template<>
Texture<glm::vec4>::Texture(const std::string& path) {
    image = std::unique_ptr<Image<glm::vec4>>(new Image<glm::vec4>(path));
}

template<>
Texture<float>::Texture(const std::string& path) {
    image = std::unique_ptr<Image<float>>(new Image<float>(path));
}

template<typename T>
glm::vec2 Texture<T>::uvNormalize(const glm::vec2& uv) const {
    glm::vec2 st = uv;
    while (st.x < 0) st.x += 1.f;
    while (st.y <= 0) st.y += 1.f;
    while (st.x >= 1) st.x -= 1.f;
    while (st.y > 1) st.y -= 1.f;
    return st;
}

template<typename T>
int Texture<T>::getIndex(const glm::vec2& st) const {
    glm::ivec2 dims = image->getDims();
    int x = clamp(dims.x * st.x, 0.f, dims.x - 1);
    int y = clamp(dims.y * (1 - st.y), 0.f, dims.y - 1);
    return y * dims.x + x;
}

template<>
glm::vec3 Texture<glm::vec3>::operator()(const glm::vec2& uv) const {
    glm::vec2 st = uvNormalize(uv);
    int index = getIndex(st);
    return image->read(index);
}

template<>
glm::vec4 Texture<glm::vec4>::operator()(const glm::vec2& uv) const {
    glm::vec2 st = uvNormalize(uv);
    int index = getIndex(st);
    return image->read(index);
}

template<>
float Texture<float>::operator()(const glm::vec2& uv) const {
    glm::vec2 st = uvNormalize(uv);
    int index = getIndex(st);
    return image->read(index);
}


