#pragma once

#include <glm.hpp>

#include <vector>

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;

};

const std::vector<Vertex> vertices =
{
	{ { -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f },{ 1.0f, 1.0f, 1.0f } }
};

class VertexBuffer
{
public:
	~VertexBuffer();

	bool create();
};