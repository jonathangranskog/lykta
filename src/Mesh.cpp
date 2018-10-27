#include <chrono>
#include <iostream>
#include "Mesh.hpp"
#include "tinyobj/tiny_obj_loader.h"

std::vector<Lykta::Mesh> Lykta::Mesh::openObj(const std::string& filename) {
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
		return std::vector<Lykta::Mesh>();
	}
	
	std::cout << "Loaded file: " << filename << " in " << loadTime << " seconds." << std::endl;

	// Create Lykta meshes
	std::vector<Lykta::Mesh> meshes = std::vector<Lykta::Mesh>();

	for (int i = 0; i < shapes.size(); i++) {
		const tinyobj::mesh_t& mesh = shapes[i].mesh;
		const std::vector<tinyobj::index_t>& indices = mesh.indices;
		
		std::vector<Lykta::Triangle> triangles = std::vector<Lykta::Triangle>();
		std::vector<glm::vec3> vertices = std::vector<glm::vec3>();
		
		// Read vertices
		for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
			glm::vec3 vertex = glm::vec3(static_cast<float>(attrib.vertices[3 * v + 0]), static_cast<float>(attrib.vertices[3 * v + 1]), static_cast<float>(attrib.vertices[3 * v + 2]));
			vertices.push_back(vertex);
		}

		// Assume equal number of normals and texture coordinates as vertices.
		std::vector<glm::vec3> normals = std::vector<glm::vec3>(vertices.size());
		std::vector<glm::vec2> texcoords = std::vector<glm::vec2>(vertices.size());

		// Read triangles
		size_t index_offset = 0;
		for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
			size_t fnum = shapes[i].mesh.num_face_vertices[f];

			// Assuming triangulated...
			unsigned x = indices[index_offset + 0].vertex_index;
			unsigned y = indices[index_offset + 1].vertex_index;
			unsigned z = indices[index_offset + 2].vertex_index;

			Lykta::Triangle triangle = Lykta::Triangle();
			triangle.x = x;
			triangle.y = y;
			triangle.z = z;
			triangles.push_back(triangle);

			// Make sure that normals and texture coords are in same positions as vertices
			unsigned nx = indices[index_offset + 0].normal_index;
			unsigned ny = indices[index_offset + 1].normal_index;
			unsigned nz = indices[index_offset + 2].normal_index;
			unsigned tx = indices[index_offset + 0].texcoord_index;
			unsigned ty = indices[index_offset + 1].texcoord_index;
			unsigned tz = indices[index_offset + 2].texcoord_index;

			normals[x] = glm::vec3(static_cast<float>(attrib.normals[nx * 3]), static_cast<float>(attrib.normals[nx * 3 + 1]), static_cast<float>(attrib.normals[nx * 3 + 2]));
			normals[y] = glm::vec3(static_cast<float>(attrib.normals[ny * 3]), static_cast<float>(attrib.normals[ny * 3 + 1]), static_cast<float>(attrib.normals[ny * 3 + 2]));
			normals[z] = glm::vec3(static_cast<float>(attrib.normals[nz * 3]), static_cast<float>(attrib.normals[nz * 3 + 1]), static_cast<float>(attrib.normals[nz * 3 + 2]));

			texcoords[x] = glm::vec2(static_cast<float>(attrib.texcoords[tx * 2]), static_cast<float>(attrib.texcoords[tx * 2 + 1]));
			texcoords[y] = glm::vec2(static_cast<float>(attrib.texcoords[ty * 2]), static_cast<float>(attrib.texcoords[ty * 2 + 1]));
			texcoords[z] = glm::vec2(static_cast<float>(attrib.texcoords[tz * 2]), static_cast<float>(attrib.texcoords[tz * 2 + 1]));
		
			index_offset += fnum;
		}

		Lykta::Mesh m = Lykta::Mesh();
		m.setPositions(vertices);
		m.setNormals(normals);
		m.setTextureCoordinates(texcoords);
		m.setTriangles(triangles);
		meshes.push_back(m);
	}

	return meshes;
}