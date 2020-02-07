#include "gles/sampler.h"
#include "data/sampler.h"
#include "resource/resource.hpp"
#include "gfx/types.h"
#include "gles/igles.h"
#include "gles/conversion.h"

using namespace core::resource;
using namespace core::igles;
using namespace core::gfx::conversion;
namespace data = core::data;

sampler::sampler(core::resource::cache& cache, const core::resource::metadata& metaData, psl::meta::file* metaFile,
				 handle<data::sampler> sampler_data)
{
	size_t iterationCount = (sampler_data->mipmaps()) ? 14 : 1; // 14 == 8096 // todo: this is a hack

	glGenSamplers(1, &m_Sampler);

	glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER,
						to_gles(sampler_data->filter_min(), sampler_data->mip_mode()));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_MAG_FILTER,
						(sampler_data->filter_max() == core::gfx::filter::linear)?GL_LINEAR:GL_NEAREST);
	glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_S, to_gles(sampler_data->addressU()));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_T, to_gles(sampler_data->addressV()));
	glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_R, to_gles(sampler_data->addressW()));

	to_gles(sampler_data->border_color(), m_Sampler);

	if(sampler_data->mipmaps())
	{
		if(sampler_data->anisotropic_filtering())
			glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, sampler_data->max_anisotropy());

		glSamplerParameterf(m_Sampler, GL_TEXTURE_MIN_LOD, sampler_data->mip_minlod());
		glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_LOD, sampler_data->mip_maxlod());

		glSamplerParameteri(m_Sampler, GL_TEXTURE_COMPARE_FUNC,
							core::gfx::conversion::to_gles(sampler_data->compare_op()));
	}
	else
	{
		glSamplerParameterf(m_Sampler, GL_TEXTURE_MIN_LOD, 0.0f);
		glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_LOD, 0.0f);
	}
	// todo figure out use case for GL_TEXTURE_COMPARE_MODE
	glGetError();
}

sampler::~sampler() { glDeleteSamplers(1, &m_Sampler); }