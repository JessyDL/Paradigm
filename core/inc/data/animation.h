#pragma once
#include "fwd/resource/resource.h"
#include "psl/serialization.h"
#include "psl/array.h"
#include "psl/array_view.h"
#include "psl/math/matrix.h"
#include "conversion_utils.h"


namespace core::data
{
	class animation
	{

		template <typename T, char... Char>
		using prop = psl::serialization::property<T, Char...>;

	  public:
		friend class psl::serialization::accessor;
		class bone
		{
		  public:
			friend class psl::serialization::accessor;
			bone()  = default;
			~bone() = default;

			bone(const bone& other)		= default;
			bone(bone&& other) noexcept = default;
			bone& operator=(const bone& other) = default;
			bone& operator=(bone&& other) noexcept = default;

			psl::string8::view name() const noexcept;
			void name(psl::string8_t value) noexcept;
			
		  private:
			template <typename S>
			void serialize(S& serializer)
			{
				serializer << m_Name;
			};

			static constexpr const char serialization_name[5]{"BONE"};

			prop<psl::string8_t, const_str("NAME", 4)> m_Name;
		};
		animation(core::resource::cache& cache, const core::resource::metadata& metaData,
				  psl::meta::file* metaFile) noexcept;
		animation()  = default;
		~animation() = default;

		animation(const animation& other)	 = default;
		animation(animation&& other) noexcept = default;
		animation& operator=(const animation& other) = default;
		animation& operator=(animation&& other) noexcept = default;

		/// \brief returns the expected time in seconds for this animation
		/// \details using the current set duration, and current set fps
		/// this method will return the expected time in seconds to cycle the animation
		/// \returns time in seconds
		float seconds() const noexcept;

		psl::string8::view name() const noexcept;
		void name(psl::string8_t value) noexcept;

		uint32_t duration() const noexcept;
		void duration(uint32_t value) noexcept;

		uint32_t fps() const noexcept;
		void fps(uint32_t value) noexcept;

		psl::array_view<bone> bones() const noexcept;
		void bones(psl::array<bone> value) noexcept;

	  private:
		template <typename S>
		void serialize(S& serializer)
		{
			serializer << m_Name << m_Duration << m_Fps << m_Bones;
		};


		static constexpr const char serialization_name[10]{"ANIMATION"};

		prop<psl::string8_t, const_str("NAME", 4)> m_Name;
		prop<uint32_t, const_str("DURATION", 8)> m_Duration;
		prop<uint32_t, const_str("FPS", 3)> m_Fps;
		prop<psl::array<bone>, const_str("BONE", 4)> m_Bones;
	};
} // namespace core::data