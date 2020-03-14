#pragma once

#include "Context.hpp"

class Shader
{
private:
	std::shared_ptr<Context> context;
	vk::ShaderStageFlagBits shaderStageFlags;

	static vk::ShaderModule *createShaderModule(std::shared_ptr<Context> context, const std::vector<char> &code);
	std::function<void(vk::ShaderModule*)> shaderModuleDeleter = [this](vk::ShaderModule *shaderModule) { if (context->getDevice()) context->getDevice()->destroyShaderModule(*shaderModule); };
	std::unique_ptr<vk::ShaderModule, decltype(shaderModuleDeleter)> shaderModule;

public:
	Shader(std::shared_ptr<Context> context, const std::string &filename, vk::ShaderStageFlagBits shaderStageFlags);

	vk::PipelineShaderStageCreateInfo getPipelineShaderStageCreateInfo();
};
