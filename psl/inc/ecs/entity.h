#pragma once
#include <cstdint>
#include <functional>

namespace psl::ecs
{
	/// \brief tag type to circumvent constructing an object in the backing data
	template <typename T>
	struct empty
	{};

	/// ----------------------------------------------------------------------------------------------
	/// Entity
	/// ----------------------------------------------------------------------------------------------
	//struct entity;

	/// \brief entity points to a collection of components
	using entity = uint32_t;

	/// \brief checks if an entity is valid or not
	/// \param[in] e the entity to check
	static bool valid(entity e) noexcept
	{
		return e != 0u;
	};

}
