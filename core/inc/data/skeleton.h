#pragma once
#include "fwd/resource/resource.h"
#include "psl/serialization.h"
#include "psl/array.h"
#include "psl/array_view.h"
#include "psl/math/matrix.h"
#include "geometry.h"
#include "conversion_utils.h"

namespace core::data
{
	class skeleton
	{
		friend class psl::serialization::accessor;

		template <typename T, char... Char>
		using prop = psl::serialization::property<T, Char...>;

		using index_size_t = core::data::geometry::index_size_t;

	  public:
		class bone
		{
			friend class psl::serialization::accessor;
		  public:
			bone()  = default;
			~bone() = default;

			bone(const bone& other)		= default;
			bone(bone&& other) noexcept = default;
			bone& operator=(const bone& other) = default;
			bone& operator=(bone&& other) noexcept = default;
			size_t size() const noexcept;

			psl::string_view name() const noexcept;
			void name(psl::string name) noexcept;

			const psl::mat4x4& inverse_bindpose() const noexcept;
			void inverse_bindpose(psl::mat4x4 value) noexcept;

			psl::array_view<index_size_t> weight_ids() const noexcept;
			void weight_ids(psl::array<index_size_t> value) noexcept;

			psl::array_view<float> weights() const noexcept;
			void weights(psl::array<float> value) noexcept;
		  private:
			/// \brief serialization method to be used by the serializer when writing this container to the disk.
			/// \param[in] serializer the serialization object, consult the serialization namespace for more
			/// information.
			template <typename S>
			void serialize(S& serializer)
			{
				serializer << m_Name << m_InverseBindPoseMatrix << m_WeightIDs << m_WeighValue;
			};

			/// \brief serialization name to be used by the serializer when writing and reading this container to and
			/// from disk.
			static constexpr const char serialization_name[5]{"BONE"};

			prop<psl::mat4x4, const_str("INV_BIND_POSE", 13)> m_InverseBindPoseMatrix;
			prop<psl::array<index_size_t>, const_str("WEIGHT_ID", 9)> m_WeightIDs; // vertex id for the given weight
			prop<psl::array<float>, const_str("WEIGHT_VALUE", 12)>
				m_WeighValue; // how much the current bone will affect the vertex
			prop<psl::string8_t, const_str("NAME", 4)> m_Name;
		};

		skeleton() = default;
		skeleton(core::resource::cache& cache, const core::resource::metadata& metaData,
				 psl::meta::file* metaFile) noexcept;
		~skeleton() = default;

		skeleton(const skeleton& other)		= delete;
		skeleton(skeleton&& other) noexcept = delete;
		skeleton& operator=(const skeleton& other) = delete;
		skeleton& operator=(skeleton&& other) noexcept = delete;

		bool has_bones() const noexcept;
		size_t size() const noexcept;

		const psl::array<bone>& bones() const noexcept;
		void bones(psl::array<bone> bones) noexcept;
	  private:
		/// \brief serialization method to be used by the serializer when writing this container to the disk.
		/// \param[in] serializer the serialization object, consult the serialization namespace for more information.
		template <typename S>
		void serialize(S& serializer)
		{
			serializer << m_Bones;
		};

		/// \brief serialization name to be used by the serializer when writing and reading this container to and from
		/// disk.
		static constexpr const char serialization_name[9]{"SKELETON"};
		prop<psl::array<bone>, const_str("BONES", 5)> m_Bones;
	};
} // namespace core::data