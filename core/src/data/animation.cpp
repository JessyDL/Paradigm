#include "data/animation.h"
#include "resource/resource.hpp"

using namespace core::data;
using namespace core::resource;


animation::animation(core::resource::cache& cache, const core::resource::metadata& metaData,
					 psl::meta::file* metaFile) noexcept
{}

float animation::seconds() const noexcept
{
	return static_cast<float>(m_Duration.value) / static_cast<float>(m_Fps.value);
}

psl::string8::view animation::name() const noexcept { return m_Name.value; }
void animation::name(psl::string8_t value) noexcept { m_Name.value = std::move(value); }

uint32_t animation::duration() const noexcept { return m_Duration.value; }
void animation::duration(uint32_t value) noexcept { m_Duration.value = value; }

uint32_t animation::fps() const noexcept { return m_Fps.value; }
void animation::fps(uint32_t value) noexcept { m_Fps.value = value; }


psl::array_view<animation::bone> animation::bones() const noexcept { return m_Bones.value; }
void animation::bones(psl::array<animation::bone> value) noexcept { m_Bones = std::move(value); }

psl::string8::view animation::bone::name() const noexcept { return m_Name.value; }
void animation::bone::name(psl::string8_t value) noexcept { m_Name.value = std::move(value); }