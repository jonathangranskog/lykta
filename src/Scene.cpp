#include <iostream>
#include "common.h"
#include "Scene.hpp"

bool Lykta::Scene::intersect(const Ray& r) const {
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
	
	return true;
}

// Static function for parsing a scene file
Lykta::Scene* Lykta::Scene::parseFile(const std::string& filename) {
	Scene* scene = new Scene();
	scene->camera = std::unique_ptr<Camera>(new PerspectiveCamera());

	std::vector<Mesh> meshes = std::vector<Mesh>();

	// Create temporary geometry
	Mesh mesh = Mesh();
	std::vector<glm::vec3> vertices;
	std::vector<Triangle> triangles;
	Triangle t1 = Triangle(0, 1, 2);
	Triangle t2 = Triangle(1, 3, 2);
	glm::vec3 v0 = glm::vec3(-1, -1, -1);
	glm::vec3 v1 = glm::vec3(-1, -1, +1);
	glm::vec3 v2 = glm::vec3(+1, -1, -1);
	glm::vec3 v3 = glm::vec3(+1, -1, +1);
	vertices.push_back(v0); vertices.push_back(v1);
	vertices.push_back(v2); vertices.push_back(v3);
	triangles.push_back(t1); triangles.push_back(t2);
	mesh.setPositions(vertices);
	mesh.setTriangles(triangles);
	meshes.push_back(mesh);
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
	
	glm::vec3* vertices = (glm::vec3*) rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), mesh.positions.size());
	std::memcpy(vertices, (void*)mesh.positions.data(), sizeof(glm::vec3) * mesh.positions.size());
	
	glm::vec3* normals = (glm::vec3*) rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, sizeof(glm::vec3), mesh.normals.size());
	std::memcpy(normals, (void*)mesh.normals.data(), sizeof(glm::vec3) * mesh.normals.size());

	glm::vec2* texcoords = (glm::vec2*) rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT2, sizeof(glm::vec2), mesh.texcoords.size());
	std::memcpy(texcoords, (void*)mesh.texcoords.data(), sizeof(glm::vec2) * mesh.texcoords.size());

	Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), mesh.triangles.size());
	std::memcpy(triangles, (void*)mesh.triangles.data(), sizeof(Triangle) * mesh.triangles.size());

	rtcCommitGeometry(geometry);
	unsigned geomID = rtcAttachGeometry(embree_scene, geometry);
	rtcReleaseGeometry(geometry);
	return geomID;
}