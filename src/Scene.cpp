#include <iostream>
#include "common.h"
#include "Scene.hpp"

bool Lykta::Scene::intersect(const Lykta::Ray& r, Lykta::Hit& result) const {
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	RTCRay ray;
	ray.org_x = r.o.x; ray.org_y = r.o.y; ray.org_z = r.o.z;
	ray.dir_x = r.d.x; ray.dir_y = r.d.y; ray.dir_z = r.d.z;
	ray.tnear = r.t.x; ray.tfar = r.t.y; ray.time = 0.f;

	RTCHit hit;
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	RTCRayHit rayhit;
	rayhit.ray = ray;
	rayhit.hit = hit;
	rtcIntersect1(embree_scene, &ctx, &rayhit);
	
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
		unsigned geomID = rayhit.hit.geomID;
		result.pos = r.o + rayhit.ray.tfar * r.d;
		
		const Mesh& mesh = meshes[geomID];
		const Triangle& tri = mesh.triangles[rayhit.hit.primID];
		float u = rayhit.hit.u, v = rayhit.hit.v;
		float w = 1.f - u - v;

		result.normal = w * mesh.normals[tri.x] + u * mesh.normals[tri.y] + v * mesh.normals[tri.z];
		result.normal = glm::normalize(result.normal);
		result.texcoord = w * mesh.texcoords[tri.x] + u * mesh.texcoords[tri.y] + v * mesh.texcoords[tri.z];

		return true;
	}

	return false;
}

bool Lykta::Scene::shadowIntersect(const Lykta::Ray& r) const {
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	RTCRay ray;
	ray.org_x = r.o.x; ray.org_y = r.o.y; ray.org_z = r.o.z;
	ray.dir_x = r.d.x; ray.dir_y = r.d.y; ray.dir_z = r.d.z;
	ray.tnear = r.t.x; ray.tfar = r.t.y; ray.time = 0.f;

	rtcOccluded1(embree_scene, &ctx, &ray);
	return ray.tfar < r.t.y;
}

// Static function for parsing a scene file
Lykta::Scene* Lykta::Scene::parseFile(const std::string& filename) {
	Scene* scene = new Scene();
	scene->camera = std::unique_ptr<Camera>(new PerspectiveCamera());

	std::vector<Mesh> f1 = Mesh::openObj("E:/Projects/lykta/spheres.obj");
	std::vector<Mesh> meshes;
	meshes.insert(meshes.end(), f1.begin(), f1.end());
	
	scene->meshes = meshes;

	scene->generateEmbreeScene();
	return scene;
}

void Lykta::Scene::generateEmbreeScene() {
	embree_device = rtcNewDevice(NULL);
	embree_scene = rtcNewScene(embree_device);
	
	for (unsigned i = 0; i < meshes.size(); i++) {
		unsigned geomID = createEmbreeGeometry(meshes[i]);
	}

	rtcCommitScene(embree_scene);
}

unsigned Lykta::Scene::createEmbreeGeometry(Mesh& mesh) {
	RTCGeometry geometry = rtcNewGeometry(embree_device, RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, (void*)mesh.positions.data(), 0, sizeof(glm::vec3), mesh.positions.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, (void*)mesh.triangles.data(), 0, sizeof(Triangle), mesh.triangles.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, (void*)mesh.normals.data(), 0, sizeof(glm::vec3), mesh.normals.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT2, (void*)mesh.texcoords.data(), 0, sizeof(glm::vec2), mesh.texcoords.size());
	rtcCommitGeometry(geometry);
	unsigned geomID = rtcAttachGeometry(embree_scene, geometry);
	rtcReleaseGeometry(geometry);
	return geomID;
}