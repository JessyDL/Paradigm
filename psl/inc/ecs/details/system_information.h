#pragma once
#include <functional>
#include "component_key.h"
#include "template_utils.h"
#include "array_view.h"
#include "../pack.h"
#include <chrono>

namespace psl::ecs
{
	enum class threading
	{
		seq		   = 0,
		sequential = seq,
		par		   = 1,
		parallel   = par,
		main	   = 2
	};

	class state;

	struct info
	{
		const state& state;
		std::chrono::duration<float> dTime;
		std::chrono::duration<float> rTime;
	};
} // namespace psl::ecs

namespace psl::ecs::details
{
	/// \brief describes a set of dependencies for a given system
	///
	/// systems can have various dependencies, for example a movement system could have
	/// dependencies on both a psl::ecs::components::transform component and a psl::ecs::components::renderable
	/// component. This dependency will output a set of psl::ecs::entity's that have all required
	/// psl::ecs::components present. Certain systems could have sets of dependencies, for example the render
	/// system requires knowing about both all `psl::ecs::components::renderable` that have a
	/// `psl::ecs::components::transform`, but also needs to know all `psl::ecs::components::camera's`. So that
	/// system would require several dependency_pack's.
	class dependency_pack
	{
		friend class psl::ecs::state;
		template <std::size_t... Is, typename T>
		auto create_dependency_filters(std::index_sequence<Is...>, psl::templates::type_container<T>)
		{
			(add(psl::templates::type_container<
				 typename std::remove_reference<decltype(std::declval<T>().template get<Is>())>::type>{}),
			 ...);
		}

		template <typename F>
		void select_impl(std::vector<component_key_t>& target)
		{
			if constexpr(!std::is_same<typename std::decay<F>::type, psl::ecs::entity>::value)
			{
				using component_t			  = F;
				constexpr component_key_t key = details::key_for<component_t>();
				target.emplace_back(key);
				m_Sizes[key] = sizeof(component_t);
			}
		}

		template <std::size_t... Is, typename T>
		auto select(std::index_sequence<Is...>, T, std::vector<component_key_t>& target)
		{
			(select_impl<typename std::tuple_element<Is, T>::type>(target), ...);
		}

		template <typename T>
		psl::array_view<T> fill_in(psl::templates::type_container<psl::array_view<T>>)
		{
			if constexpr(std::is_same<T, psl::ecs::entity>::value)
			{
				return m_Entities;
			}
			else if constexpr(std::is_const<T>::value)
			{
				constexpr component_key_t int_id = details::key_for<T>();
				return *(psl::array_view<T>*)&m_RBindings[int_id];
			}
			else
			{
				constexpr component_key_t int_id = details::key_for<T>();
				return *(psl::array_view<T>*)&m_RWBindings[int_id];
			}
		}

		template <std::size_t... Is, typename T>
		T to_pack_impl(std::index_sequence<Is...>, psl::templates::type_container<T>)
		{
			using pack_t	  = T;
			using pack_view_t = typename pack_t::pack_t;
			using range_t	 = typename pack_t::pack_t::range_t;

			return T{pack_view_t(
				fill_in(psl::templates::type_container<typename std::tuple_element<Is, range_t>::type>())...)};
		}

	  public:
		template <typename T>
		dependency_pack(psl::templates::type_container<T>)
		{
			using pack_t = T;
			create_dependency_filters(std::make_index_sequence<std::tuple_size_v<typename pack_t::pack_t::range_t>>{},
									  psl::templates::type_container<T>{});
			select(std::make_index_sequence<std::tuple_size<typename pack_t::filter_t>::value>{},
				   typename pack_t::filter_t{}, filters);
			select(std::make_index_sequence<std::tuple_size<typename pack_t::add_t>::value>{}, typename pack_t::add_t{},
				   on_add);
			select(std::make_index_sequence<std::tuple_size<typename pack_t::remove_t>::value>{},
				   typename pack_t::remove_t{}, on_remove);
			select(std::make_index_sequence<std::tuple_size<typename pack_t::break_t>::value>{},
				   typename pack_t::break_t{}, on_break);
			select(std::make_index_sequence<std::tuple_size<typename pack_t::combine_t>::value>{},
				   typename pack_t::combine_t{}, on_combine);
			select(std::make_index_sequence<std::tuple_size<typename pack_t::except_t>::value>{},
				   typename pack_t::except_t{}, except);

			if constexpr(std::is_same<psl::ecs::partial, typename pack_t::policy_t>::value)
			{
				m_IsPartial = true;
			}
		};


		~dependency_pack() noexcept					  = default;
		dependency_pack(const dependency_pack& other) = default;
		dependency_pack(dependency_pack&& other)	  = default;
		dependency_pack& operator=(const dependency_pack&) = default;
		dependency_pack& operator=(dependency_pack&&) = default;


		template <typename... Ts>
		psl::ecs::pack<Ts...> to_pack(psl::templates::type_container<psl::ecs::pack<Ts...>>)
		{
			using pack_t  = psl::ecs::pack<Ts...>;
			using range_t = typename pack_t::pack_t::range_t;

			return to_pack_impl(std::make_index_sequence<std::tuple_size<range_t>::value>{},
								psl::templates::type_container<pack_t>{});
		}

		bool allow_partial() const noexcept { return m_IsPartial; };

		size_t size_per_element() const noexcept
		{
			size_t res{0};
			for(const auto& binding : m_RBindings)
			{
				res += m_Sizes.at(binding.first);
			}

			for(const auto& binding : m_RWBindings)
			{
				res += m_Sizes.at(binding.first);
			}
			return res;
		}

		size_t entities() const noexcept { return m_Entities.size(); }
		dependency_pack slice(size_t begin, size_t end) const noexcept;

	  private:
		template <typename T>
		void add(psl::templates::type_container<psl::array_view<T>>) noexcept
		{
			constexpr component_key_t int_id = details::key_for<T>();
			m_RWBindings.emplace(int_id, psl::array_view<std::uintptr_t>{});
		}

		template <typename T>
		void add(psl::templates::type_container<psl::array_view<const T>>) noexcept
		{
			constexpr component_key_t int_id = details::key_for<T>();
			m_RBindings.emplace(int_id, psl::array_view<std::uintptr_t>{});
		}


		void add(psl::templates::type_container<psl::array_view<psl::ecs::entity>>) noexcept {}
		void add(psl::templates::type_container<psl::array_view<const psl::ecs::entity>>) noexcept {}

	  private:
		psl::array_view<psl::ecs::entity> m_Entities{};
		std::unordered_map<component_key_t, size_t> m_Sizes;
		std::unordered_map<component_key_t, psl::array_view<std::uintptr_t>> m_RBindings;
		std::unordered_map<component_key_t, psl::array_view<std::uintptr_t>> m_RWBindings;

		std::vector<component_key_t> filters;
		std::vector<component_key_t> on_add;
		std::vector<component_key_t> on_remove;
		std::vector<component_key_t> except;
		std::vector<component_key_t> on_combine;
		std::vector<component_key_t> on_break;
		bool m_IsPartial = false;
	};

	template <std::size_t... Is, typename T>
	std::vector<dependency_pack> expand_to_dependency_pack(std::index_sequence<Is...>,
														   psl::templates::type_container<T>)
	{
		std::vector<dependency_pack> res;
		(std::invoke([&]() {
			 res.emplace_back(
				 dependency_pack(psl::templates::type_container<typename std::tuple_element<Is, T>::type>{}));
		 }),
		 ...);
		return res;
	}


	template <std::size_t... Is, typename... Ts>
	std::tuple<Ts...> compress_from_dependency_pack(std::index_sequence<Is...>,
													psl::templates::type_container<std::tuple<Ts...>>,
													std::vector<dependency_pack>& pack)
	{
		return std::tuple<Ts...>{pack[Is].to_pack(
			psl::templates::type_container<
				typename std::remove_reference<decltype(std::get<Is>(std::declval<std::tuple<Ts...>>()))>::type>{})...};
	}

	class system_information final
	{
	  public:
		using pack_generator_type   = std::function<std::vector<details::dependency_pack>()>;
		using system_invocable_type = std::function<void(psl::ecs::info&, std::vector<details::dependency_pack>)>;
		system_information()		= default;
		system_information(psl::ecs::threading threading, pack_generator_type&& generator,
						   system_invocable_type&& invocable)
			: m_Threading(threading), m_PackGenerator(std::move(generator)), m_System(std::move(invocable)){};
		~system_information()						  = default;
		system_information(const system_information&) = default;
		system_information(system_information&&)	  = default;
		system_information& operator=(const system_information&) = default;
		system_information& operator=(system_information&&) = default;

		std::vector<details::dependency_pack> create_pack() { return std::invoke(m_PackGenerator); }

		void operator()(psl::ecs::info& info, std::vector<details::dependency_pack> packs)
		{
			std::invoke(m_System, info, packs);
		}

		psl::ecs::threading threading() const noexcept { return m_Threading; };
	  private:
		psl::ecs::threading m_Threading = threading::sequential;
		pack_generator_type m_PackGenerator;
		system_invocable_type m_System;
	};
} // namespace psl::ecs::details