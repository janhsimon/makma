#pragma once

#include "Buffers.hpp"
#include "Material.hpp"
#include "..\Logic\Transform.hpp"

struct Mesh
{
	uint32_t firstIndex, indexCount;
	std::string materialName, diffuseTextureFilename, normalTextureFilename;
	Material *material;
};

class Model : public Transform
{
private:
	std::shared_ptr<Context> context;
	std::vector<Mesh*> meshes;
	std::string filename;

public:
	Model(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::string &filename);
	
	void loadMaterials(const std::shared_ptr<Descriptor> descriptor);

	std::vector<Mesh*> *getMeshes() { return &meshes; }
};