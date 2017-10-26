#pragma once

#include "Context.hpp"

class Shader
{
private:
	std::shared_ptr<Context> context;

	static vk::ShaderModule *createShaderModule(const std::vector<char> &code, std::shared_ptr<Context> context);
	std::function<void(vk::ShaderModule*)> shaderModuleDeleter = [this](vk::ShaderModule *shaderModule) { if (context->getDevice()) context->getDevice()->destroyShaderModule(*shaderModule); };
	std::unique_ptr<vk::ShaderModule, decltype(shaderModuleDeleter)> shaderModule;

public:
	Shader(const std::string &filename, std::shared_ptr<Context> context);

	vk::ShaderModule *getShaderModule() const { return shaderModule.get(); }
};
