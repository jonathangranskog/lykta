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
		
		const Lykta::Mesh& mesh = meshes[geomID];
		const Lykta::Triangle& tri = mesh.triangles[rayhit.hit.primID];
		float u = rayhit.hit.u, v = rayhit.hit.v;
		float w = 1.f - u - v;

		result.normal = w * mesh.normals[tri.x] + u * mesh.normals[tri.y] + v * mesh.normals[tri.z];
		result.normal = glm::normalize(result.normal);
		result.texcoord = w * mesh.texcoords[tri.x] + u * mesh.texcoords[tri.y] + v * mesh.texcoords[tri.z];
		result.material = mesh.materialId;

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
	Lykta::Scene* scene = new Lykta::Scene();
	scene->camera = std::unique_ptr<Lykta::Camera>(new Lykta::PerspectiveCamera());
	Lykta::SurfaceMaterial material;
	material.diffuseColor = glm::vec3(1);
	material.emissiveColor = glm::vec3(0);
	scene->materials = std::vector<Lykta::SurfaceMaterial>();
	scene->materials.push_back(material);

	Lykta::SurfaceMaterial material2;
	material2.diffuseColor = glm::vec3(1, 0, 0);
	material2.emissiveColor = glm::vec3(0);
	scene->materials.push_back(material2);

	Lykta::SurfaceMaterial material3;
	material3.diffuseColor = glm::vec3(0, 1, 0);
	material3.emissiveColor = glm::vec3(0);
	scene->materials.push_back(material3);

	Lykta::SurfaceMaterial material4;
	material4.diffuseColor = glm::vec3(0);
	material4.emissiveColor = glm::vec3(10);
	scene->materials.push_back(material4);

	std::vector<Lykta::Mesh> f1 = Lykta::Mesh::openObj("E:/Projects/lykta/white_walls.obj");
	for (Lykta::Mesh& mesh : f1) {
		mesh.materialId = 0;
	}

	std::vector<Lykta::Mesh> f2 = Lykta::Mesh::openObj("E:/Projects/lykta/right_wall.obj");
	for (Lykta::Mesh& mesh : f2) {
		mesh.materialId = 1;
	}

	std::vector<Lykta::Mesh> f3 = Lykta::Mesh::openObj("E:/Projects/lykta/left_wall.obj");
	for (Lykta::Mesh& mesh : f3) {
		mesh.materialId = 2;
	}

	std::vector<Lykta::Mesh> f4 = Lykta::Mesh::openObj("E:/Projects/lykta/emitter.obj");
	for (Lykta::Mesh& mesh : f4) {
		mesh.materialId = 3;
	}

	std::vector<Lykta::Mesh> f5 = Lykta::Mesh::openObj("E:/Projects/lykta/spheres.obj");
	for (Lykta::Mesh& mesh : f5) {
		mesh.materialId = 0;
	}

	std::vector<Lykta::Mesh> meshes;
	meshes.insert(meshes.end(), f1.begin(), f1.end());
	meshes.insert(meshes.end(), f2.begin(), f2.end());
	meshes.insert(meshes.end(), f3.begin(), f3.end());
	meshes.insert(meshes.end(), f4.begin(), f4.end());
	meshes.insert(meshes.end(), f5.begin(), f5.end());
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
	rtcCommitGeometry(geometry);
	unsigned geomID = rtcAttachGeometry(embree_scene, geometry);
	rtcReleaseGeometry(geometry);
	return geomID;
}