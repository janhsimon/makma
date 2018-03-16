#include "UI.hpp"
#include "../Shader.hpp"
#include "../Buffers/Buffer.hpp"

int UI::shadowMapCascadeCount = 4;

vk::Buffer *UI::createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	auto bufferCreateInfo = vk::BufferCreateInfo().setSize(size).setUsage(usage);
	auto buffer = context->getDevice()->createBuffer(bufferCreateInfo);
	return new vk::Buffer(buffer);
}

vk::DeviceMemory *UI::createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto memoryRequirements = context->getDevice()->getBufferMemoryRequirements(*buffer);
	auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	bool foundMatch = false;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
		{
			memoryTypeIndex = i;
			foundMatch = true;
			break;
		}
	}

	if (!foundMatch)
	{
		throw std::runtime_error("Failed to find suitable memory type for buffer.");
	}

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	context->getDevice()->bindBufferMemory(*buffer, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);

}
vk::Image *UI::createFontImage(const std::shared_ptr<Context> context, uint32_t width, uint32_t height)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(width, height, 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm).setInitialLayout(vk::ImageLayout::eUndefined).setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *UI::createFontImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto memoryRequirements = context->getDevice()->getImageMemoryRequirements(*image);
	auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	bool foundMatch = false;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
		{
			memoryTypeIndex = i;
			foundMatch = true;
			break;
		}
	}

	if (!foundMatch)
	{
		throw std::runtime_error("Failed to find suitable memory type for font image.");
	}

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	context->getDevice()->bindImageMemory(*image, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);
}

vk::ImageView *UI::createFontImageView(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eR8G8B8A8Unorm);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	auto fontImageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(fontImageView);
}

vk::Sampler *UI::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setAddressModeV(vk::SamplerAddressMode::eClampToEdge).setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

vk::PipelineLayout *UI::createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts)
{
	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(static_cast<uint32_t>(setLayouts.size())).setPSetLayouts(setLayouts.data());
	auto pushConstantRange = vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(glm::vec2) * 2);
	pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
	pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *UI::createPipeline(const std::shared_ptr<Window> window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, std::shared_ptr<Context> context)
{
	Shader vertexShader(context, "Shaders/UI.vert.spv", vk::ShaderStageFlagBits::eVertex);
	Shader fragmentShader(context, "Shaders/UI.frag.spv", vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShader.getPipelineShaderStageCreateInfo() };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(ImDrawVert));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(ImDrawVert, pos));
	auto texCoord = vk::VertexInputAttributeDescription().setLocation(1).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(ImDrawVert, uv));
	auto color = vk::VertexInputAttributeDescription().setLocation(2).setFormat(vk::Format::eR8G8B8A8Unorm).setOffset(offsetof(ImDrawVert, col));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position, texCoord, color };
	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vertexInputBindingDescription);
	vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInputAttributeDescriptions.size()));
	vertexInputStateCreateInfo.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions.data());

	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto viewport = vk::Viewport().setWidth(1).setHeight(1).setMaxDepth(0.0f);
	auto scissor = vk::Rect2D().setOffset(vk::Offset2D(0, 0)).setExtent(vk::Extent2D(1, 1));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);

	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eNone).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

	std::vector<vk::DynamicState> dynamicStateEnables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	auto dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo().setDynamicStateCount(static_cast<uint32_t>(dynamicStateEnables.size())).setPDynamicStates(dynamicStateEnables.data());

	auto depthStenctilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(false).setDepthWriteEnable(false).setDepthCompareOp(vk::CompareOp::eLessOrEqual);

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setBlendEnable(true).setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha).setSrcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
	colorBlendAttachmentState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha).setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(1).setPAttachments(&colorBlendAttachmentState);

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPDepthStencilState(&depthStenctilStateCreateInfo);
	pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo).setPDynamicState(&dynamicStateCreateInfo).setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

vk::DescriptorSet *UI::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const vk::ImageView *fontImageView, const vk::Sampler *sampler)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool->getPool()).setDescriptorSetCount(1).setPSetLayouts(descriptorPool->getFontLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto descriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*fontImageView).setSampler(*sampler);
	auto samplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	samplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&descriptorImageInfo);

	context->getDevice()->updateDescriptorSets(1, &samplerWriteDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

UI::UI(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, std::vector<vk::DescriptorSetLayout> setLayouts, vk::RenderPass *renderPass)
{
	this->window = window;
	this->context = context;

	vertexCount = indexCount = 0;

	imGuiContext = ImGui::CreateContext();

	// color scheme
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

	// dimensions
	ImGuiIO &io = ImGui::GetIO();
	io.DisplaySize = ImVec2(window->getWidth(), window->getHeight());
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	// create font texture
	unsigned char *fontData;
	int texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> stagingBuffer;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> stagingBufferMemory;

	stagingBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, uploadSize, vk::BufferUsageFlagBits::eTransferSrc), bufferDeleter);
	stagingBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, stagingBuffer.get(), uploadSize, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent), bufferMemoryDeleter);

	auto memory = context->getDevice()->mapMemory(*stagingBufferMemory, 0, uploadSize);
	memcpy(memory, fontData, uploadSize);
	context->getDevice()->unmapMemory(*stagingBufferMemory);

	fontImage = std::unique_ptr<vk::Image, decltype(fontImageDeleter)>(createFontImage(context, texWidth, texHeight), fontImageDeleter);
	fontImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(fontImageMemoryDeleter)>(createFontImageMemory(context, fontImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), fontImageMemoryDeleter);

	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	auto commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	auto barrier = vk::ImageMemoryBarrier().setOldLayout(vk::ImageLayout::eUndefined).setNewLayout(vk::ImageLayout::eTransferDstOptimal).setImage(*fontImage);
	barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)).setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	auto region = vk::BufferImageCopy().setImageExtent(vk::Extent3D(texWidth, texHeight, 1)).setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
	commandBuffer.copyBufferToImage(*stagingBuffer, *fontImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);

	barrier = vk::ImageMemoryBarrier().setOldLayout(vk::ImageLayout::eTransferDstOptimal).setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImage(*fontImage);
	barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eShaderRead);
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer.end();

	auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);
	context->getQueue().submit({ submitInfo }, nullptr);
	context->getQueue().waitIdle();
	context->getDevice()->freeCommandBuffers(*context->getCommandPool(), 1, &commandBuffer);

	fontImageView = std::unique_ptr<vk::ImageView, decltype(fontImageViewDeleter)>(createFontImageView(context, fontImage.get()), fontImageViewDeleter);
	sampler = std::unique_ptr<vk::Sampler, decltype(samplerDeleter)>(createSampler(context), samplerDeleter);

	descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptorPool, fontImageView.get(), sampler.get()));

	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, setLayouts), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass, pipelineLayout.get(), context), pipelineDeleter);
}

void UI::update(const std::shared_ptr<Input> input, float delta)
{
	if (input->lockKeyPressed)
	{
		ImGuiIO &io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)window->getWidth(), (float)window->getHeight());
		io.DeltaTime = delta;

		io.MousePos = ImVec2(input->getMousePosition().x, input->getMousePosition().y);
		io.MouseDown[0] = input->leftMouseButtonPressed;
		io.MouseDown[1] = input->rightMouseButtonPressed;
	}

	ImGui::NewFrame();

	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
	
	ImGui::Begin("Options");
	
	if (ImGui::CollapsingHeader("Memory Management"))
	{
		ImGui::TextWrapped("This window is being created by the ShowDemoWindow() function. Please refer to the code in imgui_demo.cpp for reference.\n\n");
		ImGui::Text("USER GUIDE:");
		ImGui::ShowUserGuide();
	}
	if (ImGui::CollapsingHeader("Shadow Mapping"))
	{
		if (ImGui::SliderInt("Cascade Count", &shadowMapCascadeCount, 1, 16))
		{

		}
	}
	if (ImGui::CollapsingHeader("Volumetric Lighting"))
	{
		ImGui::TextWrapped("This window is being created by the ShowDemoWindow() function. Please refer to the code in imgui_demo.cpp for reference.\n\n");
		ImGui::Text("USER GUIDE:");
		ImGui::ShowUserGuide();
	}
	ImGui::End();
	
	//ImGui::ShowTestWindow();
	ImGui::Render();

	
	// update vertex/index buffers

	ImDrawData *imDrawData = ImGui::GetDrawData();

	// alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// vertex buffer
	if (vertexBuffer == nullptr || vertexCount != imDrawData->TotalVtxCount)
	{
		if (vertexBuffer)
		{
			context->getDevice()->unmapMemory(*vertexBuffer->getMemory());
			vertexBuffer.reset();
		}

		vertexBuffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eVertexBuffer, vertexBufferSize, vk::MemoryPropertyFlagBits::eHostVisible);
		vertexCount = imDrawData->TotalVtxCount;

		vertexBufferMemory = context->getDevice()->mapMemory(*vertexBuffer->getMemory(), 0, vertexBufferSize);
	}

	// index buffer
	if (indexBuffer == nullptr || indexCount < imDrawData->TotalIdxCount)
	{
		if (indexBuffer)
		{
			context->getDevice()->unmapMemory(*indexBuffer->getMemory());
			indexBuffer.reset();
		}

		indexBuffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eIndexBuffer, indexBufferSize, vk::MemoryPropertyFlagBits::eHostVisible);
		indexCount = imDrawData->TotalIdxCount;

		indexBufferMemory = context->getDevice()->mapMemory(*indexBuffer->getMemory(), 0, indexBufferSize);
	}

	// upload data
	ImDrawVert *vtxDst = (ImDrawVert*)vertexBufferMemory;
	ImDrawIdx *idxDst = (ImDrawIdx*)indexBufferMemory;

	for (int n = 0; n < imDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	std::vector<vk::MappedMemoryRange> memoryRanges;
	memoryRanges.push_back(vk::MappedMemoryRange(*vertexBuffer->getMemory(), 0, VK_WHOLE_SIZE));
	memoryRanges.push_back(vk::MappedMemoryRange(*indexBuffer->getMemory(), 0, VK_WHOLE_SIZE));
	context->getDevice()->flushMappedMemoryRanges(static_cast<uint32_t>(memoryRanges.size()), memoryRanges.data());
}

void UI::render(const vk::CommandBuffer *commandBuffer)
{
	ImGuiIO &io = ImGui::GetIO();

	commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, descriptorSet.get(), 0, nullptr);

	commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

	VkDeviceSize offsets[] = { 0 };
	commandBuffer->bindVertexBuffers(0, 1, vertexBuffer->getBuffer(), offsets);
	commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer(), 0, vk::IndexType::eUint16);

	auto viewport = vk::Viewport(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 1.0f);
	commandBuffer->setViewport(0, 1, &viewport);

	struct PushConstants { glm::vec2 translation; glm::vec2 scale; } pushConstants;
	pushConstants.translation = glm::vec2(-1.0f);
	pushConstants.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	commandBuffer->pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::vec2) * 2, &pushConstants);

	auto imDrawData = ImGui::GetDrawData();
	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;
	for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
	{
		const ImDrawList *cmd_list = imDrawData->CmdLists[i];
		for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; ++j)
		{
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];

			auto x = std::max((int32_t)(pcmd->ClipRect.x), 0);
			auto y = std::max((int32_t)(pcmd->ClipRect.y), 0);
			auto w = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
			auto h = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);

			auto scissor = vk::Rect2D(vk::Offset2D(x, y), vk::Extent2D(w, h));
			commandBuffer->setScissor(0, 1, &scissor);

			commandBuffer->drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
			indexOffset += pcmd->ElemCount;
		}

		vertexOffset += cmd_list->VtxBuffer.Size;
	}
}