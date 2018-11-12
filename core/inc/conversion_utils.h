﻿#pragma once
#include "ustring.h"
#include "string_utils.h"
#include <vector>
#include "math/matrix.h"
#include "math/vec.h"
#include "math/quaternion.h"


namespace serialization::converters
{
	// -----------------------------------------------------------------------------
	// psl::tvec
	// -----------------------------------------------------------------------------
	template<typename precision_t, size_t size>
	static psl::string8_t to_string(const psl::tvec<precision_t, size>& value) noexcept
	{
		psl::string8_t res{utility::converter<precision_t>().to_string(value.value[0])};
		for(size_t i = 1; i < size; ++i)
			res += ", " + utility::converter<precision_t>().to_string(value.value[i]);
		return res;
	}

	template<typename precision_t, size_t size>
	static bool to_string(const psl::tvec<precision_t, size>& value, psl::string8_t& out) noexcept
	{
		out = utility::converter<precision_t>().to_string(value.value[0]);
		for(size_t i = 1; i < size; ++i)
			out += ", " + utility::converter<precision_t>().to_string(value.value[i]);
		return true;
	}

	template<typename precision_t, size_t size>
	static psl::tvec<precision_t, size> from_string(psl::string8::view str) noexcept
	{
		auto split = utility::string::split(str, psl::string8::view{","});
		psl::tvec<precision_t, size> res;
		for(size_t i = 0; i < size; ++i)
			res.value[i] = utility::converter<precision_t>().from_string(split[i]);
		return res;
	}

	template<typename precision_t, size_t size>
	static void from_string(psl::string8::view str, psl::tvec<precision_t, size>& out) noexcept
	{
		auto split = utility::string::split(str, ",");
		for(size_t i = 0; i < size; ++i)
			out[i] = utility::converter<precision_t>().from_string(split[i]);
	}

	// -----------------------------------------------------------------------------
	// psl::tquat
	// -----------------------------------------------------------------------------
	template<typename precision_t>
	static psl::string8_t to_string(const psl::tquat<precision_t>& value) noexcept
	{
		psl::string8_t res{utility::converter<precision_t>().to_string(value.value[0])};
		for(size_t i = 1; i < 4; ++i)
			res += ", " + utility::converter<precision_t>().to_string(value.value[i]);
		return res;
	}

	template<typename precision_t>
	static bool to_string(const psl::tquat<precision_t>& value, psl::string8_t& out) noexcept
	{
		out = utility::converter<precision_t>().to_string(value.value[0]);
		for(size_t i = 1; i < 4; ++i)
			out += ", " + utility::converter<precision_t>().to_string(value.value[i]);
		return true;
	}

	template<typename precision_t>
	static psl::tquat<precision_t> from_string(psl::string8::view str) noexcept
	{
		auto split = utility::string::split(str, psl::string8::view{","});
		psl::tquat<precision_t> res;
		for(size_t i = 0; i < 4; ++i)
			res.value[i] = utility::converter<precision_t>().from_string(split[i]);
		return res;
	}

	template<typename precision_t>
	static void from_string(psl::string8::view str, psl::tquat<precision_t>& out) noexcept
	{
		auto split = utility::string::split(str, ",");
		for(size_t i = 0; i < 4; ++i)
			out[i] = utility::converter<precision_t>().from_string(split[i]);
	}

	// -----------------------------------------------------------------------------
	// psl::tmat
	// -----------------------------------------------------------------------------
	template<typename precision_t, size_t nX, size_t nY>
	static psl::string8_t to_string(const psl::tmat<precision_t, nX, nY>& value) noexcept
	{
		psl::string8_t res{utility::converter<precision_t>().to_string(value.value[0])};
		for(size_t i = 1; i < nX * nY; ++i)
			res += ", " + utility::converter<precision_t>().to_string(value.value[i]);
		return res;
	}

	template<typename precision_t, size_t nX, size_t nY>
	static bool to_string(const psl::tmat<precision_t, nX, nY>& value, psl::string8_t& out) noexcept
	{
		out = utility::converter<precision_t>().to_string(value.value[0]);
		for(size_t i = 1; i < nX * nY; ++i)
			out += ", " + utility::converter<precision_t>().to_string(value.value[i]);
		return true;
	}

	template<typename precision_t, size_t nX, size_t nY>
	static psl::tmat<precision_t, nX, nY> from_string(psl::string8::view str) noexcept
	{
		auto split = utility::string::split(str, psl::string8::view{","});
		psl::tmat<precision_t, nX, nY> res;
		for(size_t i = 0; i < nX *nY; ++i)
			res.value[i] = utility::converter<precision_t>().from_string(split[i]);
		return res;
	}

	template<typename precision_t, size_t nX, size_t nY>
	static void from_string(psl::string8::view str, psl::tmat<precision_t, nX, nY>& out) noexcept
	{
		auto split = utility::string::split(str, ",");
		for(size_t i = 0; i < nX * nY; ++i)
			out[i] = utility::converter<precision_t>().from_string(split[i]);
	}
}
namespace utility
{
	template<typename precision_t, size_t size>
	struct converter<psl::tvec<precision_t, size>>
	{
		using value_t = psl::tvec<precision_t, size>;
		using view_t = psl::string8::view;
		using encoding_t = psl::string8_t;

		static encoding_t to_string(const value_t& x)
		{
			encoding_t res{utility::converter<precision_t>().to_string(x.value[0])};
			for(size_t i = 1; i < size; ++i)
				res += ", " + utility::converter<precision_t>().to_string(x.value[i]);
			return res;
		}

		static value_t from_string(view_t str)
		{
			auto split = utility::string::split(str, ",");
			value_t res;
			for(size_t i = 0; i < size; ++i)
				res.value[i] = utility::converter<precision_t>().from_string(split[i]);
			return res;
		}

		static void from_string(value_t& out, view_t str)
		{
			auto split = utility::string::split(str, ",");
			for(size_t i = 0; i < size; ++i)
				out[i] = utility::converter<precision_t>().from_string(split[i]);
		}

		static bool is_valid(view_t str)
		{
			return true;
		}
	};

	template<typename precision_t>
	struct converter<psl::tquat<precision_t>>
	{
		using value_t = psl::tquat<precision_t>;
		using view_t = psl::string8::view;
		using encoding_t = psl::string8_t;

		static encoding_t to_string(const value_t& x)
		{
			encoding_t res{utility::converter<precision_t>().to_string(x.value[0])};
			for(size_t i = 1; i < 4; ++i)
				res += ", " + utility::converter<precision_t>().to_string(x.value[i]);
			return res;
		}

		static value_t from_string(view_t str)
		{
			auto split = utility::string::split(str, ",");
			value_t res;
			for(size_t i = 0; i < 4; ++i)
				res.value[i] = utility::converter<precision_t>().from_string(split[i]);
			return res;
		}

		static void from_string(value_t& out, view_t str)
		{
			auto split = utility::string::split(str, ",");
			for(size_t i = 0; i < 4; ++i)
				out[i] = utility::converter<precision_t>().from_string(split[i]);
		}

		static bool is_valid(view_t str)
		{
			return true;
		}
	};


	template<typename precision_t, size_t nX, size_t nY>
	struct converter<psl::tmat<precision_t, nX, nY>>
	{
		using value_t = psl::tmat<precision_t, nX, nY>;
		using view_t = psl::string8::view;
		using encoding_t = psl::string8_t;

		static encoding_t to_string(const value_t& x)
		{
			encoding_t res{utility::converter<precision_t>().to_string(x.value[0])};
			for(size_t i = 1; i < nX * nY; ++i)
				res += ", " + utility::converter<precision_t>().to_string(x.value[i]);
			return res;
		}

		static value_t from_string(view_t str)
		{
			auto split = utility::string::split(str, ",");
			value_t res;
			for(size_t i = 0; i < nX * nY; ++i)
				res.value[i] = utility::converter<precision_t>().from_string(split[i]);
			return res;
		}

		static void from_string(value_t& out, view_t str)
		{
			auto split = utility::string::split(str, ",");
			for(size_t i = 0; i < nX * nY; ++i)
				out[i] = utility::converter<precision_t>().from_string(split[i]);
		}

		static bool is_valid(view_t str)
		{
			return true;
		}
	};
}