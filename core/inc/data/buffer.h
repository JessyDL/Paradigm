﻿#pragma once
#include "stdafx.h"

/// \brief contains all data types that can be serialized to/from disk.
namespace core::data
{
	/// \brief container class for GPU data. 
	///
	/// core::data::buffer is a data container for anything that will be uploaded to the GPU. This means that this can not contain any complex types (such as indirections).
	/// core::data::buffer can be incorrectly set up when giving incompatible memory::region bundled with vk::BufferUsageFlags.
	/// \note the memory::region you pass to this object will also dictate the **size** and **alignment** requirements of this specific resource on the GPU.
	/// \todo figure out a way around incompatible core::data::buffer setups, perhaps by using structs to construct the class.
	/// \author Jessy De Lannoit
	class buffer
	{
		friend class serialization::accessor;
	public:
		/// will do the minimal setup needed, no allocations happen at this point yet.
		/// \param[in] uid the UID that is assigned to this object
		/// \param[in] cache which cache this object has been allocated in
		/// \param[in] usage the usage flags that signify how the resource should be used by the GPU
		/// \param[in] memoryPropertyFlags what are the properties of the memory (i.e. where does it live)
		/// \param[in] memory_region what is the owning region of this memory. Note that this parameter also will dictate the size and alignment of the resource.
		buffer(const UID& uid, core::resource::cache& cache, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memoryPropertyFlags, memory::region&& memory_region);

		~buffer();

		buffer(const buffer&) = delete;
		buffer(buffer&& other)	:	m_Region(std::move(other.m_Region)), m_Segments(std::move(other.m_Segments)), m_Usage(other.m_Usage), m_MemoryPropertyFlags(other.m_MemoryPropertyFlags)
		{

		};
		buffer& operator=(const buffer&) = delete;
		buffer& operator=(buffer&&) = delete;

		/// \returns the total size in the memory::region that this buffer occupies.
		size_t size() const;

		/// \returns the vk::BufferUsageFlags of this instance, showing the type of resource this is for the GPU
		vk::BufferUsageFlags usage() const;

		/// \returns the vk::MemoryPropertyFlags, so you know where the data lives.
		vk::MemoryPropertyFlags memoryPropertyFlags() const;

		/// \returns the associated memory::region, where the data lives.
		/// \note the data can exist "virtually" as well, for example in the case this is a pure GPU resource.
		const memory::region& region() const;

		/// \returns all committed memory::segments in the memory::region. Using this you can calculate fragmentation, and allocated size.
		const std::vector<memory::segment>& segments() const;

		/// Will try to allocate a new memory::segment of _atleast_ the given size. 
		/// This method will adhere to the alignment requirements of the underlaying memory::region.
		///
		/// \returns optionally, a new allocated memory::segment of _atleast_ the given size.
		/// \param[in] size the minimum expected size, this will auto-align/grow to the alignment requirements.
		std::optional<memory::segment> allocate(size_t size);

		/// \returns true in case the deallocation was successful.
		/// \param[in] segment the target segment to deallocate.
		/// \note \a segment gets invalidated on success.
		bool deallocate(memory::segment& segment);
	private:
		template <typename S>
		void serialize(S& serializer)
		{
			if constexpr(serialization::details::is_encoder<S>::value)
			{
				serialization::property<size_t, const_str("SIZE", 4)> size{m_Region.size()};
				serialization::property<size_t, const_str("ALIGNMENT", 9)> alignment{m_Region.alignment()};
				serialization::property<std::vector<psl::string8_t>, const_str("DATA", 4)> data{};

				for(auto it : m_Segments)
				{
					data.value.emplace_back((char*)it.range().begin, it.range().size());
				}

				serializer << m_Usage << m_MemoryPropertyFlags << size << alignment << data;
			}
			else
			{
				serialization::property<size_t, const_str("SIZE", 4)> size{0u};
				serialization::property<size_t, const_str("ALIGNMENT", 9)> alignment{4u};
				serialization::property<std::vector<psl::string8_t>, const_str("DATA", 4)> data{};

				serializer << m_Usage << m_MemoryPropertyFlags << size << alignment << data;
				for(auto it : data.value)
				{
					if(auto segm = m_Region.allocate(it.size()); segm)
					{
						segm.value().unsafe_set(it.data(), it.size());
						m_Segments.push_back(segm.value());
					}
					else
						LOG_ERROR("Could not allocate a segment on the region of the size specified.");
				}
			}
		}

		static constexpr const char serialization_name[7]{"BUFFER"};

		memory::region m_Region;
		std::vector<memory::segment> m_Segments;
		serialization::property<vk::BufferUsageFlags, const_str("USAGE", 5)> m_Usage;
		serialization::property<vk::MemoryPropertyFlags, const_str("PROPERTIES", 10)> m_MemoryPropertyFlags;
	};
}
