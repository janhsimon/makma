#pragma once

#include "Buffers.hpp"
#include "Material.hpp"
#include "..\Logic\Transform.hpp"

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
	std::vector<Mesh*> meshes;
	std::string filename;

	Mesh *loadMeshData(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const aiMesh *mesh, const aiMaterial *material);
	void appendDataToIndexBuffer(const std::shared_ptr<Buffers> buffers, const aiMesh *mesh);
	void appendDataToVertexBuffer(const std::shared_ptr<Buffers> buffers, const aiMesh *mesh);

public:
	Model(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::string &filename);
	~Model() { for (size_t i = 0; i < meshes.size(); ++i) delete meshes[i]; };

	void finalizeMaterials(const std::shared_ptr<Descriptor> descriptor);

	std::vector<Mesh*> *getMeshes() { return &meshes; }
};