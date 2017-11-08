#pragma once

#include "Buffers.hpp"
#include "Texture.hpp"

struct Mesh
{
	uint32_t firstIndex, indexCount, textureIndex;
};

class Model
{
private:
	std::vector<Mesh*> meshes;

public:
	Model(const std::string &filename, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<std::vector<Texture*>> textures);

	std::vector<Mesh*> *getMeshes() { return &meshes; }
};