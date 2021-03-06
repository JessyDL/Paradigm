#include "ecs/systems/debug/grid.h"
#include "psl/ecs/state.h"

#include "resource/resource.hpp"

#include "gfx/pipeline_cache.h"

#include "data/geometry.h"
#include "data/material.h"
#include "gfx/context.h"
#include "gfx/geometry.h"
#include "gfx/bundle.h"
#include "gfx/material.h"
#include "gfx/buffer.h"
#include "meta/shader.h"

#include "ecs/components/transform.h"
#include "ecs/components/camera.h"
#include "ecs/components/renderable.h"

#include "utility/geometry.h"

using namespace core::ecs::systems::debug;
using namespace core::ecs::components;
using namespace psl::ecs;
using namespace psl;
using namespace core;
using namespace core::gfx;


grid::grid(state& state, entity target, resource::cache& cache, resource::handle<context> context,
		   resource::handle<buffer> vertexBuffer, resource::handle<buffer> indexBuffer,
		   resource::handle<pipeline_cache> pipeline_cache,
		   resource::handle<shader_buffer_binding> instanceMaterialBuffer,
		   resource::handle<buffer> instanceVertexBuffer, psl::vec3 scale, psl::vec3 offset)
	: m_Target(target), m_Scale(scale), m_Offset(offset)
{
	// create geometry
	auto boxData = utility::geometry::create_line_cube(cache, scale);
	boxData->transform(core::data::geometry::constants::POSITION, [](psl::vec3& pos) { pos -= psl::vec3::one * 0.5f; });
	utility::geometry::copy_channel(boxData, core::data::geometry::constants::POSITION,
									core::data::geometry::constants::COLOR);
	boxData->transform(core::data::geometry::constants::COLOR, [](psl::vec3& color) {
		color = psl::vec3{0.015f, 0.035f, 0.005f};
	});
	array<vec3> positions;
	for(auto x = -8; x < 8; ++x)
	{
		for(auto y = -8; y < 8; ++y)
		{
			for(auto z = -8; z < 8; ++z)
			{
				positions.emplace_back(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
				positions[positions.size() - 1] *= scale;
				positions[positions.size() - 1] += offset;
			}
		}
	}
	utility::geometry::replicate(boxData, positions);

	m_Geometry = cache.create<core::gfx::geometry>(context, boxData, vertexBuffer, indexBuffer);

	auto boxData2 = utility::geometry::create_box(cache, scale);
	boxData2->transform(core::data::geometry::constants::POSITION,
						[](psl::vec3& pos) { pos -= psl::vec3::one * 0.5f; });
	utility::geometry::copy_channel(boxData2, core::data::geometry::constants::POSITION,
									core::data::geometry::constants::COLOR);
	boxData2->transform(core::data::geometry::constants::COLOR, [](psl::vec3& color) {
		color = psl::vec3{0.015f, 0.035f, 0.005f};
	});
	// utility::geometry::replicate(boxData2, positions);
	auto geometry2 = cache.create<core::gfx::geometry>(context, boxData2, vertexBuffer, indexBuffer);
	// create bundle
	auto vertShaderMeta = cache.library().get<core::meta::shader>("3982b466-58fe-4918-8735-fc6cc45378b0"_uid).value();
	auto fragShaderMeta = cache.library().get<core::meta::shader>("cb4b4d45-bde8-cba7-b732-b59babc8c1bc"_uid).value();

	// load the example material
	auto matData = cache.create<data::material>();
	matData->from_shaders(cache.library(), {vertShaderMeta, fragShaderMeta});
	matData->blend_states({core::data::material::blendstate::pre_multiplied_transparent(0)});
	matData->depth_test(true);
	matData->depth_write(true);
	matData->cull_mode(cullmode::back);
	matData->wireframe(false);
	auto material1 =
		cache.create<core::gfx::material>(context, matData, pipeline_cache, instanceMaterialBuffer->buffer);
	auto bundle1 = cache.create<gfx::bundle>(instanceVertexBuffer, instanceMaterialBuffer);
	bundle1->set_material(material1, 2501);
	bundle1->set(material1, "color", psl::vec4{1.f, 0.45f, 0.3f, 1.0f} * 0.25f);

	auto matData2 = cache.create<data::material>();
	matData2->from_shaders(cache.library(), {vertShaderMeta, fragShaderMeta});
	matData2->blend_states({core::data::material::blendstate::soft_additive(0)});
	matData2->depth_test(true);
	matData2->depth_write(false);
	matData2->cull_mode(cullmode::front);
	matData2->wireframe(true);
	auto material2 =
		cache.create<core::gfx::material>(context, matData2, pipeline_cache, instanceMaterialBuffer->buffer);

	m_Bundle = cache.create<gfx::bundle>(instanceVertexBuffer, instanceMaterialBuffer);
	// m_Bundle->set_material(material1, 2500);
	m_Bundle->set_material(material2, 2501);
	m_Bundle->set(material2, "color", psl::vec4{0.3f, 0.45f, 1.f, 1.f} * 0.25f);
	// create entity
	m_Entities = state.create(2, psl::array<renderable>{{m_Bundle, m_Geometry}, {bundle1, geometry2}}, transform{},
							  empty<grid::tag>{}, empty<dynamic_tag>{});
	state.declare(&grid::tick, this);
}

void grid::tick(info& info, pack<entity, const transform, psl::ecs::filter<camera>> pack,
				psl::ecs::pack<transform, psl::ecs::filter<grid::tag>> grid_pack)
{
	for(auto [entity, tr] : pack)
	{
		if(entity != m_Target) continue;

		auto pos = tr.position;

		pos = static_cast<psl::ivec3>(pos) - (static_cast<psl::ivec3>(pos) % static_cast<psl::ivec3>(m_Scale));

		for(auto [target] : grid_pack)
		{
			target.position = pos;
		}
	}
}