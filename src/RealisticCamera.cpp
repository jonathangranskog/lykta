#include "JSONHelper.hpp"
#include "RealisticCamera.hpp"
#include <iostream>

using namespace Lykta;

RealisticCamera::RealisticCamera(std::vector<LensInterface> elements, float shift, glm::mat4 camToWorld, glm::ivec2 res) {
	interfaces = elements;
	sensorShift = shift;
	cameraToWorld = camToWorld;
	resolution = res;
	aspect = resolution.x / (float)resolution.y;
	sensorSize = glm::vec2(0.024f, 0.024f * 1.f / aspect);
	frontZ = 0.f;
	for (int i = 0; i < interfaces.size(); i++) {
		frontZ += interfaces[i].thickness;
	}
}

Ray RealisticCamera::generateSensorRay(const glm::vec2& pixel, const glm::vec2& sample) const {
	float rearRadius = getRearAperture();
	glm::vec2 np = glm::vec2(pixel.x / resolution.x, 1.f - pixel.y / resolution.y);
	glm::vec3 pFilm = glm::vec3(np * sensorSize - sensorSize / 2.f, sensorShift);
	glm::vec3 pRear = glm::vec3(-rearRadius + sample.x * 2 * rearRadius, -rearRadius + sample.y * 2 * rearRadius, frontZ);
	glm::vec3 direction = pRear - pFilm;
	Ray ray;
	ray.o = pFilm;
	ray.d = glm::normalize(direction);
	return ray;
}

bool RealisticCamera::intersectElement(int index, float z, const Ray& ray, float& t, glm::vec3& normal) const {
	// Basically sphere intersection
	glm::vec3 o = ray.o - glm::vec3(0, 0, z);
	float A = glm::dot(ray.d, ray.d);
	float B = 2 * glm::dot(ray.d, o);
	float C = glm::dot(o, o) - interfaces[index].curvature * interfaces[index].curvature;
	
	// Solve quadratic
	float disc = B * B - 4 * A * C;
	if (disc < 0.f) return false;
	float root = sqrtf(disc);
	float t0 = (-B + root) / (2 * A);
	float t1 = (-B - root) / (2 * A);
	bool useCloser = (ray.d.z > 0.f) != (interfaces[index].curvature < 0);
	t = (useCloser) ? fminf(t0, t1) : fmaxf(t0, t1);
	if (t < 0) return false;
	
	// Compute normal
	normal = glm::normalize(o + t * ray.d);
	if (glm::dot(-ray.d, normal) < 0) normal = -normal;
	return true;
}

bool RealisticCamera::testAperture(int index, const glm::vec2& hit) const {
	float r2 = glm::dot(hit, hit);
	if (r2 > interfaces[index].aperture * interfaces[index].aperture) return false;
	return true;
}

bool RealisticCamera::trace(const Ray& in, Ray& out) const {
	Ray lensSpace;

	// Transform to lens space
	lensSpace.o = glm::vec3(in.o.x, in.o.y, -in.o.z);
	lensSpace.d = glm::vec3(in.d.x, in.d.y, -in.d.z);

	// Trace through each element
	float elementZ = 0.f;
	for (int i = 0; i < interfaces.size(); i++) {
		const LensInterface& element = interfaces[i];
		elementZ -= element.thickness;

		// Intersect with element
		bool isStop = element.curvature == 0.f;
		float t = 0.f;
		glm::vec3 normal;

		if (isStop) t = (elementZ - lensSpace.o.z) / lensSpace.d.z;
		else {
			float z = elementZ + element.curvature;
			bool success = intersectElement(i, z, lensSpace, t, normal);
			if (!success) return false;
		}

		// Check if passes aperture
		glm::vec3 hit = lensSpace.o + t * lensSpace.d;
		if (!testAperture(i, glm::vec2(hit.x, hit.y))) return false;
		lensSpace.o = hit;

		// Refract direction
		if (!isStop) {
			float etai = element.eta;
			float etat = 1.f;
			if (i > 0 && interfaces[i - 1].eta != 0.f) {
				etat = interfaces[i - 1].eta;
			}
			glm::vec3 newdir;
			bool success = refract(-lensSpace.d, normal, etai / etat, newdir);
			if (!success) return false;
			lensSpace.d = newdir;
		}
	}

	// Transform back to camera space
	out.o = glm::vec3(lensSpace.o.x, lensSpace.o.y, -lensSpace.o.z);
	out.d = glm::vec3(lensSpace.d.x, lensSpace.d.y, -lensSpace.d.z);
	return true;
}
glm::vec3 RealisticCamera::createRay(Ray& ray, const glm::vec2& pixel, const glm::vec2& sample) const {
	Ray sensor = generateSensorRay(pixel, sample);
	bool success = trace(sensor, ray);

	if (success) {
		ray.o = glm::vec3(cameraToWorld * glm::vec4(ray.o, 1));
		ray.d = glm::normalize(glm::vec3(cameraToWorld * glm::vec4(ray.d, 0)));
		ray.t = glm::vec2(EPS, INFINITY);
		return glm::vec3(1.f);
	}
	else {
		return glm::vec3(0.f);
	}
}