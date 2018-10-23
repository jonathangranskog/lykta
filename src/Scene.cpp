#include <iostream>
#include "common.h"
#include "Scene.hpp"
#include <embree3/rtcore.h>
#include <embree3/rtcore_scene.h>

bool Lykta::Scene::intersect(const Ray& ray) const {
	// TODO: Implement intersection!
	return true;
}

// Static function for parsing a scene file
Lykta::Scene* Lykta::Scene::parseFile(const std::string& filename) {
	Scene* scene = new Scene();
	scene->camera = std::unique_ptr<Camera>(new PerspectiveCamera());

	// Create temporary Embree geometry
	RTCDevice embree_device = rtcNewDevice(NULL);
	RTCScene embree_scene = rtcNewScene(embree_device);
	RTCGeometry geom = rtcNewGeometry(embree_device, RTC_GEOMETRY_TYPE_TRIANGLE);
	glm::vec3* vertices = (glm::vec3*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), 4);
	vertices[0].x = -1; vertices[0].y = -1; vertices[0].z = -1;
	vertices[1].x = -1; vertices[1].y = -1; vertices[1].z = +1;
	vertices[2].x = +1; vertices[2].y = -1; vertices[2].z = -1;
	vertices[3].x = +1; vertices[3].y = -1; vertices[3].z = +1;

	glm::ivec3* triangles = (glm::ivec3*) rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(glm::ivec3), 2);
	triangles[0].x = 0; triangles[0].y = 1; triangles[0].z = 2;
	triangles[1].x = 1; triangles[1].y = 3; triangles[1].z = 2;
	
	rtcCommitGeometry(geom);
	unsigned int geomID = rtcAttachGeometry(embree_scene, geom);
	rtcReleaseGeometry(geom);

	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	RTCRay ray;
	ray.org_x = 0.f; ray.org_y = 1.f; ray.org_z = 0.f;
	ray.dir_x = 0; ray.dir_y = -1; ray.dir_z = 0;
	ray.tnear = 0.f; ray.tfar = INFINITY; ray.time = 0.f;

	RTCHit hit;
	hit.geomID = geomID;

	RTCRayHit rayhit;
	rayhit.ray = ray;
	rayhit.hit = hit;
	rtcIntersect1(embree_scene, &ctx, &rayhit);

	std::cout << rayhit.hit.geomID << std::endl;

	return scene;
}