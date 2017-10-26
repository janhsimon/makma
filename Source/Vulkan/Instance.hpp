#pragma once

#include "..\Window.hpp"

class Instance
{
public:
	~Instance();

	bool create(const Window *window);
};