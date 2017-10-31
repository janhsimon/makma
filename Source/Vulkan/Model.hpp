#pragma once

#include "Buffers.hpp"

class Model
{
public:
	Model(const std::string &filename, const std::shared_ptr<Buffers> buffers);
};