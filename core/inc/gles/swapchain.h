#pragma once
#include "math/vec.h"
#include "resource/resource.hpp"

namespace core::os
{
	class surface;
}

namespace core::igles
{
	class context;
	class swapchain
	{
	  public:
		swapchain(const psl::UID& uid, core::resource::cache& cache, core::resource::handle<core::os::surface> surface,
				  core::resource::handle<core::igles::context> context, bool use_depth = true);
		~swapchain();

		swapchain(const swapchain& other)	 = default;
		swapchain(swapchain&& other) noexcept = default;
		swapchain& operator=(const swapchain& other) = default;
		swapchain& operator=(swapchain&& other) noexcept = default;

		bool present();
		/*/// returns the amount of images in the swapchain
		uint32_t size() const noexcept;

		/// \returns the width of the swapchain image
		uint32_t width() const noexcept;
		/// \returns the height of the swapchainimage.
		uint32_t height() const noexcept;

		/// \returns the clear color value assigned to the swapchain.
		const psl::vec4 clear_color() const noexcept;


		/// \returns the clear depth color of the swapchain
		/// \warning will return default value in case there is no depth texture.
		const float clear_depth() const noexcept;

		/// \returns the clear depth color of the swapchain
		/// \warning will return default value in case there is no depth texture.
		const uint32_t clear_stencil() const noexcept;

		/// \returns if the swapchain uses a depth texture or not.
		bool has_depth() const noexcept;

		/// \brief sets the clear color
		/// \param[in] color the new clear color value to use
		void clear_color(psl::vec4 color) noexcept;

		/// \returns false in case the window might be resizing.
		bool is_ready() const noexcept;*/

	  private:
		core::resource::handle<core::os::surface> m_Surface;
		core::resource::handle<core::igles::context> m_Context;
	};
} // namespace core::igles