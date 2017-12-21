#pragma once

#include "Buffers.hpp"
#include "Material.hpp"
#include "..\Transform.hpp"

#include <assimp\scene.h>

struct Mesh
{
	uint32_t firstIndex, indexCount;
	std::shared_ptr<Material> material;
};

class Model : public Transform
{
private:
	std::shared_ptr<Context> context;
	std::shared_ptr<Buffers> buffers;
	std::vector<std::shared_ptr<Mesh>> meshes;

	std::shared_ptr<Mesh> loadMeshData(const aiMesh *mesh, const aiMaterial *material, const std::string &path, const std::string &filename);
	void appendDataToIndexBuffer(const aiMesh *mesh);
	void appendDataToVertexBuffer(const aiMesh *mesh);

public:
	Model(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::string &path, const std::string &filename);

	void finalizeMaterials(const std::shared_ptr<Descriptor> descriptor);

	std::vector<std::shared_ptr<Mesh>> *getMeshes() { return &meshes; }
};