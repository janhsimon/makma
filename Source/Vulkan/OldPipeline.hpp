#pragma once

#include "..\Window.hpp"

class Pipeline
{
public:
	~Pipeline();

	bool create(const Window *window);
};