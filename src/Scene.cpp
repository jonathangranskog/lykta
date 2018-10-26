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
	
	return (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID);
}

// Static function for parsing a scene file
Lykta::Scene* Lykta::Scene::parseFile(const std::string& filename) {
	Scene* scene = new Scene();
	scene->camera = std::unique_ptr<Camera>(new PerspectiveCamera());

	std::vector<Mesh> meshes = std::vector<Mesh>();

	// Create temporary geometry
	Mesh mesh = Mesh();
	std::vector<glm::vec3> vertices, normals;
	std::vector<glm::vec2> texcoords;
	std::vector<Triangle> triangles;
	Triangle t1 = Triangle(0, 1, 2);
	Triangle t2 = Triangle(1, 3, 2);
	glm::vec3 v0 = glm::vec3(-1, -1, -1);
	glm::vec3 v1 = glm::vec3(-1, -1, +1);
	glm::vec3 v2 = glm::vec3(+1, -1, -1);
	glm::vec3 v3 = glm::vec3(+1, -1, +1);
	glm::vec3 n0 = glm::vec3(0, 1, 0);
	glm::vec3 n1 = glm::vec3(0, 1, 0);
	glm::vec3 n2 = glm::vec3(0, 1, 0);
	glm::vec3 n3 = glm::vec3(0, 1, 0);
	glm::vec2 uv0 = glm::vec2(0, 0);
	glm::vec2 uv1 = glm::vec2(0, 1);
	glm::vec2 uv2 = glm::vec2(1, 0);
	glm::vec2 uv3 = glm::vec2(1, 1);
	
	vertices.push_back(v0); vertices.push_back(v1);
	vertices.push_back(v2); vertices.push_back(v3);
	normals.push_back(n0); normals.push_back(n1);
	normals.push_back(n2); normals.push_back(n3);
	texcoords.push_back(uv0); texcoords.push_back(uv1);
	texcoords.push_back(uv2); texcoords.push_back(uv3);

	triangles.push_back(t1); triangles.push_back(t2);
	mesh.setPositions(vertices);
	mesh.setTriangles(triangles);
	mesh.setNormals(normals);
	mesh.setTextureCoordinates(texcoords);
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
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, (void*)mesh.positions.data(), 0, sizeof(glm::vec3), mesh.positions.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, (void*)mesh.triangles.data(), 0, sizeof(Triangle), mesh.triangles.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, (void*)mesh.normals.data(), 0, sizeof(glm::vec3), mesh.normals.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT2, (void*)mesh.texcoords.data(), 0, sizeof(glm::vec2), mesh.texcoords.size());
	rtcCommitGeometry(geometry);
	unsigned geomID = rtcAttachGeometry(embree_scene, geometry);
	rtcReleaseGeometry(geometry);
	return geomID;
}