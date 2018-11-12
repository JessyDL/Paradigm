﻿#include "stdafx.h"
#include "gfx/drawgroup.h"
#include "gfx/material.h"
#include "vk/geometry.h"
#include "data/geometry.h"
#include "vk/pipeline.h"
#include "vk/framebuffer.h"
#include "vk/swapchain.h"
#include "vk/buffer.h"

using namespace core::gfx;

void drawgroup::build(vk::CommandBuffer cmdBuffer, core::resource::handle<framebuffer> framebuffer, uint32_t index,
					  std::optional<core::resource::handle<core::gfx::material>> replacement)
{
	if(replacement) replacement.value()->bind_pipeline(cmdBuffer, framebuffer, index);

	for(auto& drawLayer : m_Group)
	{
		for(auto& drawCall : drawLayer.second)
		{
			auto material = drawCall.m_Material;
			if(replacement)
			{
				material = replacement.value();
			}
			else
			{
				material->bind_pipeline(cmdBuffer, framebuffer, index);
			}

			for(auto& geometryHandle : drawCall.m_Geometry)
			{
				uint32_t instance_n = material->instances(geometryHandle);
				if(instance_n == 0 || !geometryHandle->compatible(material)) continue;

				geometryHandle->bind(cmdBuffer, material);
				material->bind_geometry(cmdBuffer, geometryHandle);

				cmdBuffer.drawIndexed(geometryHandle->data()->indices().size(), instance_n, 0, 0, 0);
			}
		}
	}
}


void drawgroup::build(vk::CommandBuffer cmdBuffer, core::resource::handle<swapchain> swapchain, uint32_t index,
					  std::optional<core::resource::handle<core::gfx::material>> replacement)
{
	if(replacement) replacement.value()->bind_pipeline(cmdBuffer, swapchain, index);

	for(auto& drawLayer : m_Group)
	{
		for(auto& drawCall : drawLayer.second)
		{
			auto material = drawCall.m_Material;
			if(replacement)
			{
				material = replacement.value();
			}
			else
			{
				material->bind_pipeline(cmdBuffer, swapchain, index);
			}

			for(auto& geometryHandle : drawCall.m_Geometry)
			{
				uint32_t instance_n = material->instances(geometryHandle);
				if(instance_n == 0 || !geometryHandle->compatible(material)) continue;

				geometryHandle->bind(cmdBuffer, material);
				material->bind_geometry(cmdBuffer, geometryHandle);

				cmdBuffer.drawIndexed(geometryHandle->data()->indices().size(), instance_n, 0, 0, 0);
			}
		}
	}
}

const drawlayer& drawgroup::layer(const psl::string& layer, uint32_t priority) noexcept 
{
	auto it = std::find_if(std::begin(m_Group), std::end(m_Group), [&layer](const auto& element)
						   {
							   return element.first.name == layer;
						   });
	if(it != std::end(m_Group))
		return it->first;

	return m_Group.emplace(std::pair<drawlayer, std::vector<drawcall>>(drawlayer{layer, priority}, {})).first->first;
}

bool drawgroup::contains(const psl::string& layer) const noexcept
{
	return std::find_if(std::begin(m_Group), std::end(m_Group), [&layer](const auto& element)
						{
							return element.first.name == layer;
						}) != std::end(m_Group);
}
std::optional<std::reference_wrapper<const drawlayer>> drawgroup::get(const psl::string& layer) const noexcept
{
	auto it = std::find_if(std::begin(m_Group), std::end(m_Group), [&layer](const auto& element)
						   {
							   return element.first.name == layer;
						   });

	if(it != std::end(m_Group))
		return it->first;
	return std::nullopt;
}
bool drawgroup::priority(drawlayer& layer, uint32_t priority) noexcept
{
	auto it = std::find_if(std::begin(m_Group), std::end(m_Group), [&layer](const auto& element)
						   {
							   return element.first.name == layer.name;
						   });

	if(it != std::end(m_Group))
	{
		std::vector<drawcall> copy = std::move(it->second);
		m_Group.erase(it);
		layer = m_Group.emplace(std::pair<drawlayer, std::vector<drawcall>>(drawlayer{layer.name, priority}, std::move(copy))).first->first;
		return true;
	}
	return false;
}

drawcall& drawgroup::add(const drawlayer& layer, core::resource::handle<core::gfx::material> material) noexcept
{
	return m_Group[layer].emplace_back(material);
}