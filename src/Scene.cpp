#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include "common.h"
#include "Scene.hpp"
#include "JSONHelper.hpp"
#include "RandomPool.hpp"

using namespace Lykta;

bool Scene::intersect(const Ray& r, Hit& result) const {
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	RTCRay ray = r.createRTCRay();

	RTCHit hit;
	hit.geomID = RTC_INVALID_GEOMETRY_ID;
	hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

	RTCRayHit rayhit;
	rayhit.ray = ray;
	rayhit.hit = hit;
	rtcIntersect1(embree_scene, &ctx, &rayhit);
	
	if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
		unsigned geomID = rayhit.hit.geomID;
		// tfar contains hit distance
		result.pos = r.o + rayhit.ray.tfar * r.d;
		const MeshPtr mesh = meshes[geomID];
		mesh->setHitAttributes(rayhit.hit, result);
		result.geomID = geomID;

		return true;
	}

	return false;
}

bool Scene::shadowIntersect(const Ray& r) const {
	RTCIntersectContext ctx;
	rtcInitIntersectContext(&ctx);
	RTCRay ray = r.createRTCRay();

	// Fires Embree shadow ray
	rtcOccluded1(embree_scene, &ctx, &ray);

	// tfar is set to -inf if hit, hence < 0 check
	return ray.tfar < 0.f;
}

// Define static member to prevent linker error
ScenePtr Scene::activeScene;

// Static function for parsing a scene file
ScenePtr Scene::parseFile(const std::string& filename) {
	ScenePtr scene = ScenePtr(new Scene());

	if (activeScene) {
		rtcReleaseScene(activeScene->embree_scene);
		rtcReleaseDevice(activeScene->embree_device);
		activeScene->meshes.clear();
		activeScene->materials.clear();
		activeScene->emitters.clear();
		activeScene->camera.release();
		activeScene.reset();
	}

	std::ifstream in(filename.c_str());
	std::stringstream sstr;
	sstr << in.rdbuf();

	rapidjson::Document jsonDocument;
	jsonDocument.Parse(sstr.str().c_str());
	
	assert(jsonDocument.IsObject());

	filesystem::path scenepath = filesystem::path(filename);
	scenepath = scenepath.parent_path();
	
	std::vector<EmitterPtr> emitters;
    std::map<std::string, std::pair<unsigned, MaterialPtr> > materials = JSONHelper::readMaterials(jsonDocument, scenepath);
	
	scene->meshes = JSONHelper::readMeshes(jsonDocument, materials, emitters, scenepath);

	// Create material vector from material map used for name matching
	unsigned numMaterials = materials.size();
	std::vector<MaterialPtr> materialVector = std::vector<MaterialPtr>(numMaterials);
	for (auto it = materials.begin(); it != materials.end(); it++) {
		materialVector[it->second.first] = it->second.second;
	}

	scene->environment = JSONHelper::readEnvironment(jsonDocument, emitters, scenepath);
	scene->materials = materialVector;
	scene->emitters = emitters;
	scene->camera = std::unique_ptr<Camera>(JSONHelper::readCamera(jsonDocument, scenepath));
	scene->generateEmbreeScene();
	activeScene = scene;
	return scene;
}

void Scene::generateEmbreeScene() {
	embree_device = rtcNewDevice(NULL);
	embree_scene = rtcNewScene(embree_device);
	
	for (unsigned i = 0; i < meshes.size(); i++) {
		unsigned geomID = createEmbreeGeometry(meshes[i]);
	}

	rtcCommitScene(embree_scene);
}

unsigned Scene::createEmbreeGeometry(MeshPtr mesh) {
	RTCGeometry geometry = rtcNewGeometry(embree_device, RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, (void*)mesh->positions.data(), 0, sizeof(glm::vec3), mesh->positions.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, (void*)mesh->triangles.data(), 0, sizeof(Triangle), mesh->triangles.size());
	rtcSetSharedGeometryBuffer(geometry, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3, (void*)mesh->normals.data(), 0, sizeof(glm::vec3), mesh->normals.size());
	
	rtcSetGeometryIntersectFilterFunction(geometry, opacityIntersectFilter);
	rtcSetGeometryOccludedFilterFunction(geometry, opacityIntersectFilter);
	
	rtcCommitGeometry(geometry);
	unsigned geomID = rtcAttachGeometry(embree_scene, geometry);
	rtcReleaseGeometry(geometry);
	return geomID;
}


void Scene::opacityIntersectFilter(const RTCFilterFunctionNArguments* args) {
	int* valid = args->valid;
	RTCRay* ray = (RTCRay*)args->ray;
	RTCHit* hit = (RTCHit*)args->hit;
	
	float u = hit->u;
	float v = hit->v;
	float w = 1.f - u - v;
	unsigned geomID = hit->geomID;

	const MaterialPtr material = activeScene->getMaterial(geomID);
	const TexturePtr<float> opacityTex = material->getOpacityTexture();

	if (opacityTex) {
		const MeshPtr mesh = activeScene->getMeshes()[geomID];
		const Triangle& tri = mesh->triangles[hit->primID];
		
		glm::vec2 texcoord;
		if (tri.tx != -1 && tri.ty != -1 && tri.tz != -1) {
			texcoord = w * mesh->texcoords[tri.tx] + u * mesh->texcoords[tri.ty] + v * mesh->texcoords[tri.tz];
		}
		else {
			texcoord = glm::vec2(0);
		}

		float eval = opacityTex->eval(texcoord);
		if (eval > 1.f - EPS) {
			valid[0] = 1;
		}
		else if (eval < EPS) {
			valid[0] = 0;
		}
		else {
			float rand = RND::next1D();
			if (rand < eval) {
				valid[0] = 1;
			}
			else {
				valid[0] = 0;
			}
		}

	}
	else {
		valid[0] = 1;
	}
}


