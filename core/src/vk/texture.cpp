﻿
#include "stdafx_psl.h"
#include "vk/texture.h"
#include "meta/texture.h"
#include "vk/context.h"
#include "vk/buffer.h"
#include "data/buffer.h"
#include "vk/sampler.h"
#ifdef fseek
#define cached_fseek fseek
#define cached_fclose fclose
#define cached_fwrite fwrite
#define cached_fread fread
#define cached_ftell ftell
#undef fseek
#undef fclose
#undef fwrite
#undef fread
#undef ftell
#endif
#include "gli/gli.hpp"
#ifdef cached_fseek
#define fseek cached_fseek
#define fclose cached_fclose
#define fwrite cached_fwrite
#define fread cached_fread
#define ftell cached_ftell
#endif
using namespace psl;
using namespace core::gfx;
using namespace core::resource;

texture::texture(const UID& uid, core::resource::cache& cache, psl::meta::file* metaFile, handle<context> context) : texture(uid, cache, metaFile, context, {})
{

}
texture::texture(const UID& uid, core::resource::cache& cache, psl::meta::file* metaFile, handle<context> context, core::resource::handle<core::gfx::buffer> stagingBuffer)
	:
	m_Cache(cache), m_Context(context), m_Meta(m_Cache.library().get<core::meta::texture>(metaFile->ID()).value_or(nullptr)), m_StagingBuffer(stagingBuffer)
{
	if(!m_Meta)
	{
		core::ivk::log->error("ivk::texture could not resolve the meta uid: {0}. is the meta file present in the metalibrary?", utility::to_string(metaFile->ID()));
		return;
	}
	// AddReference(m_Context);
	if(cache.library().is_physical_file(m_Meta->ID()))
	{
		auto result = cache.library().load(m_Meta->ID());
		if(!result) goto fail;
		m_TextureData = new gli::texture(gli::load(result.value().data(), result.value().size()));
		switch(m_Meta->image_type())
		{
		case vk::ImageViewType::e2D: load_2D(); break;
		// case vk::ImageViewType::eCube: load_cube(); break;
		default: debug_break();
		}
	}
	else
	{
		// this is a generated file;
		switch(m_Meta->image_type())
		{
		case vk::ImageViewType::e2D: create_2D(); break;
		// case vk::ImageViewType::eCube: load_cube(); break;
		default: debug_break();
		}
	}

fail:
	return;
}

texture::~texture()
{
	for(auto& item : m_Descriptors)
	{
		delete(item.second);
	}
	if(m_TextureData != nullptr) delete m_TextureData;
	m_Context->device().destroyImageView(m_View, nullptr);
	m_Context->device().destroyImage(m_Image, nullptr);
	m_Context->device().freeMemory(m_DeviceMemory, nullptr);
}


const vk::Image& texture::image() const noexcept { return m_Image; }
const vk::ImageView& texture::view() const noexcept { return m_View; }
const vk::ImageLayout& texture::layout() const noexcept { return m_ImageLayout; }
const vk::DeviceMemory& texture::memory() const noexcept { return m_DeviceMemory; }
const vk::ImageSubresourceRange& texture::subResourceRange() const noexcept { return m_SubresourceRange; }
const core::meta::texture& texture::meta() const noexcept { return *m_Meta; }
uint32_t texture::mip_levels() const noexcept { return m_MipLevels; }

vk::DescriptorImageInfo& texture::descriptor(const UID& sampler)
{
	auto it = m_Descriptors.find(sampler);
	if(it != m_Descriptors.end())
	{
		return *(it->second);
	}

	auto samplerHandle = m_Cache.find<core::gfx::sampler>(sampler);
	if(!samplerHandle)
		samplerHandle = core::resource::create<core::gfx::sampler>(m_Cache, sampler);

	vk::DescriptorImageInfo* descriptor = new vk::DescriptorImageInfo();
	descriptor->sampler					= samplerHandle->get(mip_levels());
	descriptor->imageView				= view();
	descriptor->imageLayout				= vk::ImageLayout::eGeneral;

	m_Descriptors[sampler] = descriptor;
	return *descriptor;
}

void texture::create_2D()
{

	m_MipLevels = m_Meta->mip_levels();
	vk::FormatProperties formatProperties;
	if(m_Meta->format() == vk::Format::eUndefined)
	{
		LOG_ERROR("Undefined format property in: ", utility::to_string(m_Meta->ID()));
	}
	m_Context->physical_device().getFormatProperties(m_Meta->format(), &formatProperties);

	// Create optimal tiled target image
	vk::ImageCreateInfo imageCreateInfo;
	imageCreateInfo.pNext		  = NULL;
	imageCreateInfo.imageType	 = (vk::ImageType)m_Meta->image_type();
	imageCreateInfo.format		  = m_Meta->format();
	imageCreateInfo.mipLevels	 = m_MipLevels;
	imageCreateInfo.arrayLayers   = 1;
	imageCreateInfo.samples		  = vk::SampleCountFlagBits::e1;
	imageCreateInfo.tiling		  = vk::ImageTiling::eOptimal;
	imageCreateInfo.usage		  = vk::ImageUsageFlagBits::eSampled;
	imageCreateInfo.sharingMode   = vk::SharingMode::eExclusive;
	imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageCreateInfo.extent		  = vk::Extent3D{m_Meta->width(), m_Meta->height(), m_Meta->depth()};
	imageCreateInfo.usage		  = m_Meta->usage();

	utility::vulkan::check(m_Context->device().createImage(&imageCreateInfo, nullptr, &m_Image));


	// Create and bind the memory
	vk::MemoryRequirements memReqs;
	vk::MemoryAllocateInfo memAllocInfo;
	memAllocInfo.pNext			 = NULL;
	memAllocInfo.allocationSize  = 0;
	memAllocInfo.memoryTypeIndex = 0;
	m_Context->device().getImageMemoryRequirements(m_Image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex =
		m_Context->memory_type(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	utility::vulkan::check(m_Context->device().allocateMemory(&memAllocInfo, nullptr, &m_DeviceMemory));
	m_Context->device().bindImageMemory(m_Image, m_DeviceMemory, 0);

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	vk::ImageViewCreateInfo view;
	view.image		= nullptr;
	view.viewType   = m_Meta->image_type();
	view.format		= m_Meta->format();
	view.components = vk::ComponentMapping();
	view.subresourceRange.aspectMask =
		(utility::vulkan::has_depth(m_Meta->format())) ? vk::ImageAspectFlagBits::eDepth : m_Meta->aspect_mask();
	view.subresourceRange.baseMipLevel   = 0;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount	 = m_Meta->layers();
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = m_MipLevels;
	m_SubresourceRange				 = view.subresourceRange;
	view.image						 = m_Image;
	utility::vulkan::check(m_Context->device().createImageView(&view, nullptr, &m_View));
}


void texture::load_2D()
{
	gli::texture2d* m_Texture2DData = (gli::texture2d*)m_TextureData;
	if(m_Texture2DData->empty())
	{
		LOG_ERROR("Empty texture");
		debug_break();
	}

	if(m_Meta->width() != (uint32_t)(*m_Texture2DData)[0].extent().x)
		m_Meta->width((uint32_t)(*m_Texture2DData)[0].extent().x);

	if(m_Meta->height() != (uint32_t)(*m_Texture2DData)[0].extent().y)
		m_Meta->height((uint32_t)(*m_Texture2DData)[0].extent().y);

	if(m_Meta->depth() != (uint32_t)(*m_Texture2DData)[0].extent().z)
		m_Meta->depth((uint32_t)(*m_Texture2DData)[0].extent().z);

	m_MipLevels = (uint32_t)m_Texture2DData->levels();
	m_Meta->mip_levels(m_MipLevels);
	vk::FormatProperties formatProperties;

	// todo we can map gli::format to the vulkan mappings.
	if(m_Meta->format() == vk::Format::eUndefined)
	{
		LOG_ERROR("Undefined format property in: ", m_Meta->ID().to_string());
	}
	m_Context->physical_device().getFormatProperties(m_Meta->format(), &formatProperties);

	// Only use linear tiling if requested (and supported by the device)
	// Support for linear tiling is mostly limited, so prefer to use
	// optimal tiling instead
	// On most implementations linear tiling will only support a very
	// limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
	vk::Bool32 useStaging = m_StagingBuffer;

	// Only use linear tiling if forced
	if(!useStaging)
	{
		// Don't use linear if format is not supported for (linear) shader sampling
		useStaging = !(formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
	}

	// todo write the version without staging
	useStaging = true;

	if(useStaging)
	{
		auto stagingBuffer = m_StagingBuffer;
		if(!m_StagingBuffer)
		{
			core::ivk::log->warn("missing a staging buffer in ivk::texture, will create one dynamically, but this is inneficient");
			auto tempBuffer = core::resource::create<core::data::buffer>(m_Cache);
			tempBuffer.load(vk::BufferUsageFlagBits::eTransferSrc,
								   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
								   memory::region{m_Texture2DData->size() + 1024, 4, new memory::default_allocator(false)});
			stagingBuffer = create<gfx::buffer>(m_Cache);
			stagingBuffer.load(m_Context, tempBuffer);
		}
		if(!stagingBuffer)
		{
			core::ivk::log->error("could not create a staging buffer in ivk::texture");
			return;
		}
		memory::segment segment;
		if(auto segmentOpt = stagingBuffer->reserve((vk::DeviceSize)m_Texture2DData->size()); segmentOpt)
		{
			segment = segmentOpt.value();
			buffer::commit_instruction instr;
			instr.segment = segment;
			instr.size = (vk::DeviceSize)m_Texture2DData->size();
			instr.source = (std::uintptr_t)m_Texture2DData->data();
			if(!stagingBuffer->commit({instr}))
			{
				core::ivk::log->error("could not commit an ivk::texture in a staging buffer");
			}
		}
		else
		{
			core::ivk::log->error("could not allocate a segment in the staging buffer for an ivk::texture");
			return;
		}
		//stagingBuffer->map(m_Texture2DData->data(), (vk::DeviceSize)m_Texture2DData->size(), 0);

		// Setup buffer copy regions for each mip level
		std::vector<vk::BufferImageCopy> bufferCopyRegions;
		uintptr_t offset = segment.range().begin;

		for(uint32_t i = 0; i < m_MipLevels; i++)
		{
			vk::BufferImageCopy bufferCopyRegion;
			bufferCopyRegion.imageSubresource.aspectMask	 = vk::ImageAspectFlagBits::eColor;
			bufferCopyRegion.imageSubresource.mipLevel		 = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount	 = m_Meta->layers();
			bufferCopyRegion.imageExtent.width				 = (uint32_t)(*m_Texture2DData)[i].extent().x;
			bufferCopyRegion.imageExtent.height				 = (uint32_t)(*m_Texture2DData)[i].extent().y;
			bufferCopyRegion.imageExtent.depth				 = 1;
			bufferCopyRegion.bufferOffset					 = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += (*m_Texture2DData)[i].size();
		}

		// Create optimal tiled target image
		vk::ImageCreateInfo imageCreateInfo;
		imageCreateInfo.pNext		  = NULL;
		imageCreateInfo.imageType	 = (vk::ImageType)m_Meta->image_type();
		imageCreateInfo.format		  = m_Meta->format();
		imageCreateInfo.mipLevels	 = m_MipLevels;
		imageCreateInfo.arrayLayers   = 1;
		imageCreateInfo.samples		  = vk::SampleCountFlagBits::e1;
		imageCreateInfo.tiling		  = vk::ImageTiling::eOptimal;
		imageCreateInfo.usage		  = vk::ImageUsageFlagBits::eSampled;
		imageCreateInfo.sharingMode   = vk::SharingMode::eExclusive;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.extent		  = vk::Extent3D{m_Meta->width(), m_Meta->height(), m_Meta->depth()};
		imageCreateInfo.usage		  = m_Meta->usage();

		utility::vulkan::check(m_Context->device().createImage(&imageCreateInfo, nullptr, &m_Image));

		vk::MemoryRequirements memReqs;
		vk::MemoryAllocateInfo memAllocInfo;
		memAllocInfo.pNext			 = NULL;
		memAllocInfo.allocationSize  = 0;
		memAllocInfo.memoryTypeIndex = 0;
		m_Context->device().getImageMemoryRequirements(m_Image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex =
			m_Context->memory_type(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		utility::vulkan::check(m_Context->device().allocateMemory(&memAllocInfo, nullptr, &m_DeviceMemory));
		m_Context->device().bindImageMemory(m_Image, m_DeviceMemory, 0);

		vk::CommandBuffer copyCmd = utility::vulkan::create_cmd_buffer(m_Context->device(), m_Context->command_pool(),
																	   vk::CommandBufferLevel::ePrimary, true, 1);

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount   = m_MipLevels;
		subresourceRange.layerCount   = m_Meta->layers();
		// Image barrier for optimal image (target)
		// Optimal image will be used as destination for the copy
		utility::vulkan::set_image_layout(copyCmd, m_Image, vk::ImageLayout::eUndefined,
										  vk::ImageLayout::eTransferDstOptimal, subresourceRange);

		// Copy mip levels from staging buffer
		copyCmd.copyBufferToImage(stagingBuffer->gpu_buffer(), m_Image, vk::ImageLayout::eTransferDstOptimal,
								  (uint32_t)bufferCopyRegions.size(), bufferCopyRegions.data());

		// Change texture image layout to shader read after all mip levels have been copied
		m_ImageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		utility::vulkan::set_image_layout(copyCmd, m_Image, vk::ImageLayout::eTransferDstOptimal, m_ImageLayout,
										  subresourceRange);

		m_Context->flush(copyCmd, true);
	}


	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	vk::ImageViewCreateInfo view;
	view.image							 = nullptr;
	view.viewType						 = m_Meta->image_type();
	view.format							 = m_Meta->format();
	view.components						 = vk::ComponentMapping();
	view.subresourceRange.aspectMask	 = m_Meta->aspect_mask();
	view.subresourceRange.baseMipLevel   = 0;
	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.layerCount	 = m_Meta->layers();
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = (useStaging) ? m_MipLevels : 1;
	m_SubresourceRange				 = view.subresourceRange;
	view.image						 = m_Image;
	utility::vulkan::check(m_Context->device().createImageView(&view, nullptr, &m_View));
}