#include "Pipeline.hpp"
#include "Shader.hpp"
#include "VertexBuffer.hpp"
#include "Vulkan.hpp"

Pipeline::~Pipeline()
{
	if (vulkan.device)
	{
		if (vulkan.pipelineLayout)
		{
			vulkan.device.destroyPipelineLayout(vulkan.pipelineLayout);
		}

		if (vulkan.renderPass)
		{
			vulkan.device.destroyRenderPass(vulkan.renderPass);
		}

		if (vulkan.pipeline)
		{
			vulkan.device.destroyPipeline(vulkan.pipeline);
		}
	}
}

bool Pipeline::create(const Window *window)
{
	Shader vertexShader;
	if (!vertexShader.create("Shaders\\vert.spv")) return false;
	vk::PipelineShaderStageCreateInfo vertexShaderInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShader.getModule(), "main");

	Shader fragmentShader;
	if (!fragmentShader.create("Shaders\\frag.spv")) return false;
	vk::PipelineShaderStageCreateInfo fragmentShaderInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShader.getModule(), "main");

	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
	pipelineShaderStageCreateInfos.push_back(vertexShaderInfo);
	pipelineShaderStageCreateInfos.push_back(fragmentShaderInfo);
	
	vk::VertexInputBindingDescription vertexInputBindingDescription(0, sizeof(Vertex));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
	vertexInputAttributeDescriptions.push_back(vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position)));
	vertexInputAttributeDescriptions.push_back(vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)));
	vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(), 1, &vertexInputBindingDescription, static_cast<uint32_t>(vertexInputAttributeDescriptions.size()), vertexInputAttributeDescriptions.data());
	
	vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList);
	
	vk::Viewport viewport(0.0f, 0.0f, window->getWidth(), window->getHeight(), 0.0f, 1.0f);
	vk::Rect2D scissor(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight()));
	vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, &viewport, 1, &scissor);
	
	vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);
	
	vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(), vk::SampleCountFlagBits::e1, false, 1.0f);
	
	vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(vk::PipelineColorBlendStateCreateFlags(), false, vk::LogicOp::eClear, 1, &pipelineColorBlendAttachmentState);

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), 0);
	if (vulkan.device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vulkan.pipelineLayout) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create pipeline layout.");
		return false;
	}

	vk::AttachmentDescription colorAttachmentDescription(vk::AttachmentDescriptionFlags(), vulkan.surfaceFormat.format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
	vk::AttachmentReference colorAttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::SubpassDescription subpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachmentReference);
	vk::SubpassDependency subpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits(), vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
	vk::RenderPassCreateInfo renderPassCreateInfo(vk::RenderPassCreateFlags(), 1, &colorAttachmentDescription, 1, &subpassDescription, 1, &subpassDependency);
	if (vulkan.device.createRenderPass(&renderPassCreateInfo, nullptr, &vulkan.renderPass) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create render pass.");
		return false;
	}

	vk::GraphicsPipelineCreateInfo pipelineCreateInfo(vk::PipelineCreateFlags(), static_cast<uint32_t>(pipelineShaderStageCreateInfos.size()), pipelineShaderStageCreateInfos.data(), &pipelineVertexInputStateCreateInfo, &pipelineInputAssemblyStateCreateInfo, nullptr, &pipelineViewportStateCreateInfo, &pipelineRasterizationStateCreateInfo, &pipelineMultisampleStateCreateInfo, nullptr, &pipelineColorBlendStateCreateInfo, nullptr, vulkan.pipelineLayout, vulkan.renderPass);
	if (vulkan.device.createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &vulkan.pipeline) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create graphics pipeline.");
		return false;
	}
	
	return true;
}