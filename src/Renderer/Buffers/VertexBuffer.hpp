#pragma once

#include "Buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

class VertexBuffer
{
private:
	std::vector<Vertex> vertices;
	std::unique_ptr<Buffer> buffer;

public:
	void finalize(const std::shared_ptr<Context> context);

	std::vector<Vertex> *getVertices() { return &vertices; }
	Buffer *getBuffer() const { return buffer.get(); }
};