#pragma once

#include "..\Window.hpp"

class Surface
{
public:
	~Surface();

	bool create(const Window *window);
};