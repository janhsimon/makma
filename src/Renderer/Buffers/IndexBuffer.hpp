#pragma once

#include "Buffer.hpp"

class IndexBuffer
{
private:
	std::vector<uint32_t> indices;
	std::unique_ptr<Buffer> buffer;

public:
	void finalize(const std::shared_ptr<Context> context);

	std::vector<uint32_t> *getIndices() { return &indices; }
	Buffer *getBuffer() const { return buffer.get(); }
};