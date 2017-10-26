#pragma once

#include <vector>

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

class IndexBuffer
{
public:
	~IndexBuffer();

	bool create();
};