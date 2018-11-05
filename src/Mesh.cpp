#include <chrono>
#include <iostream>
#include <algorithm>
#include "common.h"
#include "Mesh.hpp"
#include "Sampling.hpp"
#include "tinyobj/tiny_obj_loader.h"

using namespace Lykta;

void Mesh::constructCDF() {
	if (triangles.size() == 0) return;

	std::vector<float> areas = std::vector<float>(triangles.size(), 0.f);
	#pragma omp parallel for
	for (int i = 0; i < triangles.size(); i++) {
		const glm::vec3& v0 = positions[triangles[i].px];
		const glm::vec3& v1 = positions[triangles[i].py];
		const glm::vec3& v2 = positions[triangles[i].pz];

		areas[i] = glm::length(glm::cross(v1 - v0, v2 - v0)) * 0.5f;
	}

	cumulativeAreas = std::vector<float>(triangles.size(), 0.f);
	cumulativeAreas[0] = areas[0];
	for (int i = 1; i < triangles.size(); i++) {
		cumulativeAreas[i] = areas[i] + cumulativeAreas[i - 1];
	}
}

float Mesh::pdf() const {
	if (cumulativeAreas.size() == 0) return 0.f;
	else return 1.f / cumulativeAreas.back();
}

void Mesh::sample(const glm::vec3& s, MeshSample& info) const {
	int index = cumulativeAreas.size() - 1;
	float select = s.z * cumulativeAreas.back();
	auto it = std::lower_bound(cumulativeAreas.begin(), cumulativeAreas.end(), select);
	if (it != cumulativeAreas.end()) {
		index = std::distance(cumulativeAreas.begin(), it);
	}
	
	// Sample barycentric coordinates
	glm::vec3 bc = Sampling::uniformTriangle(glm::vec2(s.x, s.y));
	
	// Set sample information
	const Triangle& tri = triangles[index];
	glm::vec3 pt = bc.x * positions[tri.px] + bc.y * positions[tri.py] + bc.z * positions[tri.pz];

	glm::vec3 n;
	if (tri.nx == -1 || tri.ny == -1 || tri.nz == -1) 
		n = glm::cross(positions[tri.py] - positions[tri.px], positions[tri.pz] - positions[tri.px]);
	else
		n = bc.x * normals[tri.nx] + bc.y * normals[tri.ny] + bc.z * normals[tri.nz];
	n = glm::normalize(n);

	glm::vec2 uv;
	if (tri.tx == -1 || tri.ty == -1 || tri.tz == -1)
		uv = glm::vec2(0.f);
	else
		uv = bc.x * texcoords[tri.tx] + bc.y * texcoords[tri.ty] + bc.z * texcoords[tri.tz];

	info.pos = pt;
	info.normal = n;
	info.uv = uv;
	info.pdf = pdf();
}

std::vector<MeshPtr> Mesh::openObj(const std::string& filename) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning, error;

	auto startTime = std::chrono::system_clock::now();
	// Imports and triangulates obj
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filename.c_str());
	auto endTime = std::chrono::system_clock::now();
	float loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.f;
	
	if (!warning.empty()) {
		std::cout << "WARNING (TINYOBJ): " << warning << std::endl;
	}

	if (!error.empty()) {
		std::cout << "ERROR (TINYOBJ): " << error << std::endl;
	}

	if (!ret) {
		std::cout << "Failed to load file: " << filename << std::endl;
		return std::vector<MeshPtr>();
	}
	
	std::cout << "Loaded file: " << filename << " in " << loadTime << " seconds." << std::endl;

	// Create Lykta meshes
	std::vector<MeshPtr> meshes = std::vector<MeshPtr>();

	for (int i = 0; i < shapes.size(); i++) {
		const tinyobj::mesh_t& mesh = shapes[i].mesh;
		const std::vector<tinyobj::index_t>& indices = mesh.indices;
		
		std::vector<Triangle> triangles = std::vector<Triangle>();
		std::vector<glm::vec3> vertices = std::vector<glm::vec3>();
		std::vector<glm::vec3> normals = std::vector<glm::vec3>();
		std::vector<glm::vec2> texcoords = std::vector<glm::vec2>();

		// Read vertices
		for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
			glm::vec3 vertex = glm::vec3(static_cast<float>(attrib.vertices[3 * v + 0]), static_cast<float>(attrib.vertices[3 * v + 1]), static_cast<float>(attrib.vertices[3 * v + 2]));
			vertices.push_back(vertex);
		}

		for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
			glm::vec3 normal = glm::vec3(static_cast<float>(attrib.normals[3 * v + 0]), static_cast<float>(attrib.normals[3 * v + 1]), static_cast<float>(attrib.normals[3 * v + 2]));
			normals.push_back(normal);
		}

		for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
			glm::vec2 texcoord = glm::vec2(static_cast<float>(attrib.texcoords[2 * v + 0]), static_cast<float>(attrib.texcoords[2 * v + 1]));
			texcoords.push_back(texcoord);
		}

		// Read triangles
		size_t index_offset = 0;
		for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
			size_t fnum = shapes[i].mesh.num_face_vertices[f];

			// Assuming triangulated...
			Triangle triangle;
			triangle.px = indices[index_offset + 0].vertex_index;
			triangle.py = indices[index_offset + 1].vertex_index;
			triangle.pz = indices[index_offset + 2].vertex_index;
			triangle.nx = indices[index_offset + 0].normal_index;
			triangle.ny = indices[index_offset + 1].normal_index;
			triangle.nz = indices[index_offset + 2].normal_index;
			triangle.tx = indices[index_offset + 0].texcoord_index;
			triangle.ty = indices[index_offset + 1].texcoord_index;
			triangle.tz = indices[index_offset + 2].texcoord_index;
			triangles.push_back(triangle);
			index_offset += fnum;
		}

		MeshPtr m = MeshPtr(new Mesh());
		m->positions = vertices;
		m->normals = normals;
		m->texcoords = texcoords;
		m->triangles = triangles;
		m->constructCDF();
		meshes.push_back(m);
	}

	return meshes;
}