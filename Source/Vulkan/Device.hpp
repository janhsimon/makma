#pragma once

class Device
{
private:
	bool selectPhysicalDevice();
	bool selectQueueFamilyIndex();
	bool createDevice();

public:
	~Device();

	bool create();
};