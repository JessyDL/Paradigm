#include "data/skeleton.h"
#include "resource/resource.hpp"

using namespace core::data;
using namespace core::resource;


skeleton::skeleton(core::resource::cache& cache, const core::resource::metadata& metaData,
				   psl::meta::file* metaFile) noexcept
{}

bool skeleton::has_bones() const noexcept { return m_Bones.value.size() > 0; }
size_t skeleton::size() const noexcept { return m_Bones.value.size(); }


const psl::array<skeleton::bone>& skeleton::bones() const noexcept { return m_Bones.value; }
void skeleton::bones(psl::array<skeleton::bone> bones) noexcept { m_Bones.value = std::move(bones); }


size_t skeleton::bone::size() const noexcept { return m_WeightIDs.value.size(); }


psl::string_view skeleton::bone::name() const noexcept { return m_Name.value; }
void skeleton::bone::name(psl::string name) noexcept { m_Name.value = std::move(name); }

const psl::mat4x4& skeleton::bone::inverse_bindpose() const noexcept { return m_InverseBindPoseMatrix.value; }
void skeleton::bone::inverse_bindpose(psl::mat4x4 value) noexcept { m_InverseBindPoseMatrix.value = std::move(value); }

psl::array_view<skeleton::index_size_t> skeleton::bone::weight_ids() const noexcept { return m_WeightIDs.value; }
void skeleton::bone::weight_ids(psl::array<skeleton::index_size_t> value) noexcept
{
	m_WeightIDs.value = std::move(value);
}

psl::array_view<float> skeleton::bone::weights() const noexcept { return m_WeighValue.value; }
void skeleton::bone::weights(psl::array<float> value) noexcept { m_WeighValue.value = std::move(value); }