#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include "common.h"
#include "Scene.hpp"
#include "JSONHelper.hpp"

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

		if (tri.nx != -1 && tri.ny != -1 && tri.nz != -1) {
			result.normal = w * mesh.normals[tri.nx] + u * mesh.normals[tri.ny] + v * mesh.normals[tri.nz];
			result.normal = glm::normalize(result.normal);
		}
		else {
			result.normal = glm::vec3(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
		}

		if (tri.tx != -1 && tri.ty != -1 && tri.tz != -1) {
			result.texcoord = w * mesh.texcoords[tri.tx] + u * mesh.texcoords[tri.ty] + v * mesh.texcoords[tri.tz];
		}
		else {
			result.texcoord = glm::vec2(0);
		}

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
	
	std::ifstream in(filename.c_str());
	std::stringstream sstr;
	sstr << in.rdbuf();

	rapidjson::Document jsonDocument;
	jsonDocument.Parse(sstr.str().c_str());
	
	assert(jsonDocument.IsObject());

	filesystem::path scene_path = filesystem::path(filename);
	scene_path = scene_path.parent_path();

	std::map<std::string, std::pair<unsigned, Lykta::SurfaceMaterial> > materials = Lykta::JSONHelper::readMaterials(jsonDocument);
	scene->meshes = Lykta::JSONHelper::readMeshes(jsonDocument, materials, scene_path);

	unsigned numMaterials = materials.size();
	std::vector<Lykta::SurfaceMaterial> materialVector = std::vector<Lykta::SurfaceMaterial>(numMaterials);
	for (auto it = materials.begin(); it != materials.end(); it++) {
		materialVector[it->second.first] = it->second.second;
	}
	scene->materials = materialVector;
	scene->camera = std::unique_ptr<Lykta::Camera>(Lykta::JSONHelper::readCamera(jsonDocument));
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
	rtcCommitGeometry(geometry);
	unsigned geomID = rtcAttachGeometry(embree_scene, geometry);
	rtcReleaseGeometry(geometry);
	return geomID;
}