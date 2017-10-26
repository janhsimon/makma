#include "Shader.hpp"
#include "..\Window.hpp"

#include <fstream>

vk::ShaderModule *Shader::createShaderModule(const std::vector<char> &code, std::shared_ptr<Context> context)
{
	auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo().setCodeSize(code.size()).setPCode(reinterpret_cast<const uint32_t*>(&code[0]));
	auto shaderModule = context->getDevice()->createShaderModule(shaderModuleCreateInfo);
	return new vk::ShaderModule(shaderModule);
}

Shader::Shader(const std::string &filename, std::shared_ptr<Context> context)
{
	this->context = context;

	std::ifstream file(filename, std::ios::binary);

	if (file.fail())
	{
		throw std::runtime_error("Failed to open shader file \"" + filename + "\".");
	}

	std::streampos begin = file.tellg();
	file.seekg(0, std::ios::end);
	std::streampos end = file.tellg();

	std::vector<char> code(static_cast<size_t>(end - begin));
	file.seekg(0, std::ios::beg);
	file.read(&code[0], end - begin);
	file.close();

	if (code.size() == 0)
	{
		throw std::runtime_error("Shader file \"" + filename + "\" is empty.");
	}

	shaderModule = std::unique_ptr<vk::ShaderModule, decltype(shaderModuleDeleter)>(createShaderModule(code, context), shaderModuleDeleter);
}