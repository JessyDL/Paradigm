#pragma once
#include "psl/platform_def.h"

#ifdef WIN32
#else
#include <stdio.h>
#endif
//#include <string>
//#include <istream>
//#include <iostream>
#include <fstream>
#include <cstring>

//#include <codecvt>
// seeing the standard omits this for LLVM specific reasons (ridiculous..)
// see: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3512.html (Modifications to the rest of the standard
// library)

#ifdef UNICODE
#define __T(x) L##x
#else
#define __T(x) x
#endif

#define _T(x) __T(x)
#define _TEXT(x) __T(x)

using namespace std::string_literals;

/// \brief paradigm standard library
///
/// contains various utilities and constructs to aid you. In this you'll find constructs that allow event signalling,
/// dealing with UTF encoded strings, etc...
namespace psl
{
	/// \brief UTF-8 class namespace that contains the typedefs for the string objects, as well as helper methods.
	namespace string8
	{
		using char_t   = char;
		using view	 = std::basic_string_view<char_t>;
		using stream   = std::basic_stringstream<char_t>;
		using ostream  = std::basic_ostream<char_t>;
		using ofstream = std::basic_ofstream<char_t>;
		using istream  = std::basic_istream<char_t>;
		using ifstream = std::basic_ifstream<char_t>;
		using fstream  = std::basic_fstream<char_t>;
	} // namespace string8

	/// \brief UTF-16 class namespace that contains the typedefs for the string objects, as well as helper methods.
	namespace string16
	{
		using char_t   = char16_t;
		using view	 = std::basic_string_view<char_t>;
		using stream   = std::basic_stringstream<char_t>;
		using ostream  = std::basic_ostream<char_t>;
		using ofstream = std::basic_ofstream<char_t>;
		using istream  = std::basic_istream<char_t>;
		using ifstream = std::basic_ifstream<char_t>;
		using fstream  = std::basic_fstream<char_t>;

	} // namespace string16

	/// \brief UTF-32 class namespace that contains the typedefs for the string objects, as well as helper methods.
	namespace string32
	{
		using char_t   = char32_t;
		using view	 = std::basic_string_view<char_t>;
		using stream   = std::basic_stringstream<char_t>;
		using ostream  = std::basic_ostream<char_t>;
		using ofstream = std::basic_ofstream<char_t>;
		using istream  = std::basic_istream<char_t>;
		using ifstream = std::basic_ifstream<char_t>;
		using fstream  = std::basic_fstream<char_t>;
	} // namespace string32

	namespace platform
	{
#if defined(UNICODE)
		using char_t = wchar_t;
#else
		using char_t = char;
#endif
		using view	 = std::basic_string_view<char_t>;
		using stream   = std::basic_stringstream<char_t>;
		using ostream  = std::basic_ostream<char_t>;
		using ofstream = std::basic_ofstream<char_t>;
		using istream  = std::basic_istream<char_t>;
		using ifstream = std::basic_ifstream<char_t>;
		using fstream  = std::basic_fstream<char_t>;
	} // namespace platform

	using string8_t  = std::basic_string<string8::char_t, std::char_traits<string8::char_t>>;
	using string16_t = std::basic_string<string16::char_t, std::char_traits<string16::char_t>>;
	using string32_t = std::basic_string<string32::char_t, std::char_traits<string32::char_t>>;

	/// \brief platform conditional string type, turns into char or wchar depending on the platform and compile options
	using pchar_t   = platform::char_t;
	using pstring_t = std::basic_string<pchar_t, std::char_traits<pchar_t>>;
#if defined(STRING_16_BIT)
	static constexpr size_t uchar_size{sizeof(string16::char_t)};
	using string	   = string16_t;
	using char_t	   = string16::char_t;
	using string_view  = string16::view;
	using stringstream = string16::stream;
	using ostream	  = string16::ostream;
	using ofstream	 = string16::ofstream;
	using istream	  = string16::istream;
	using ifstream	 = string16::ifstream;
	using fstream	  = string16::fstream;
#else
#if !defined(STRING_8_BIT)
#define STRING_8_BIT
#endif
	static constexpr size_t uchar_size{sizeof(string8::char_t)};
	using string = string8_t;
	using char_t = string8::char_t;
	using string_view = string8::view;
	using stringstream = string8::stream;
	using ostream = string8::ostream;
	using ofstream = string8::ofstream;
	using istream = string8::istream;
	using ifstream = string8::ifstream;
	using fstream = string8::fstream;
#endif
	namespace string8
	{
		/// \brief converts a UTF-8 string into a UTF-16 string.
		/// \param[in] s UTF-8 encoded string to convert.
		/// \returns a psl::string16_t based on the input UTF-8 string.
		psl::string16_t to_string16_t(const psl::string8_t& s);

		/// \brief converts a UTF-16 string into a UTF-8 string.
		/// \param[in] s UTF-16 encoded string to convert.
		/// \returns a psl::string8_t based on the input UTF-16 string.
		psl::string8_t from_string16_t(const psl::string16_t& s);

		/// \brief converts a UTF-16 string into a UTF-8 string.
		/// \param[in] s UTF-16 encoded string to convert.
		/// \returns a psl::string8_t based on the input UTF-16 string.
		psl::string8_t from_string16_t(psl::string16::view s);


		/// \brief converts a wstring into a UTF-8 string.
		///
		/// when the wstring is already 8bit, this turns into a no-op.
		/// \param[in] s wstring to convert.
		/// \returns a psl::string8_t based on the input wstring.
		psl::string8_t from_wstring(const std::wstring& s);

		/*/// \brief alias for std::cin, prefer this version
		static decltype(std::cin)& cin = std::cin;
		/// \brief alias for std::cout, prefer this version
		static decltype(std::cout)& cout = std::cout;
		/// \brief alias for std::cerr, prefer this version
		static decltype(std::cerr)& cerr = std::cerr;
		/// \brief alias for std::clor, prefer this version
		static decltype(std::clog)& clog = std::clog;*/
	} // namespace string8

	namespace string16
	{
		/// \brief converts a UTF-16 string into a UTF-8 string.
		/// \param[in] s UTF-16 encoded string to convert.
		/// \returns a psl::string8_t based on the input UTF-16 string.
		psl::string8_t to_string8_t(const psl::string16_t& s);

		/// \brief converts a UTF-8 string into a UTF-16 string.
		/// \param[in] s UTF-8 encoded string to convert.
		/// \returns a psl::string16_t based on the input UTF-8 string.
		psl::string16_t from_string8_t(const psl::string8_t& s);

		/// \brief converts a UTF-8 string into a UTF-16 string.
		/// \param[in] s UTF-8 encoded string to convert.
		/// \returns a psl::string16_t based on the input UTF-8 string.
		psl::string16_t from_string8_t(psl::string8::view s);

		/*/// \brief alias for std::wcin, prefer this version
		static decltype(std::wcin)& cin = std::wcin;
		/// \brief alias for std::wcout, prefer this version
		static decltype(std::wcout)& cout = std::wcout;
		/// \brief alias for std::wcerr, prefer this version
		static decltype(std::wcerr)& cerr = std::wcerr;
		/// \brief alias for std::wclog, prefer this version
		static decltype(std::wclog)& clog = std::wclog;*/
	} // namespace string16

#if defined(STRING_16_BIT)
	inline psl::string8_t to_string8_t(const psl::string& s)
	{
		return (std::wstring_convert<std::codecvt_utf8<char_t>, char_t>{}).to_bytes(s);
	}

	inline psl::string from_string8_t(const psl::string8_t& s)
	{
		return (std::wstring_convert<std::codecvt_utf8<char_t>, char_t>{}).from_bytes(s);
	}

	inline psl::string8_t to_string8_t(psl::string_view s) { return to_string8_t(psl::string(s)); }

	inline psl::string from_string8_t(psl::string8::view s) { return from_string8_t(psl::string8_t(s)); }

	inline psl::ustring_t to_wstring(const psl::string& s)
	{
		psl::ustring_t res{(const wchar_t*)s.c_str(), s.size()};
		return res;
	}

	inline psl::ustring_t to_wstring(psl::string_view s)
	{
		psl::ustring_t res{(const wchar_t*)s.data(), s.size()};
		return res;
	}

	inline psl::string from_wstring(psl::ustring::view s)
	{
		psl::string res{(psl::char_t*)s.data(), s.size()};
		return res;
	}
#else
	/// \brief converts a psl::string into a UTF-8 string.
	///
	/// converts a psl::string into a UTF-8 string, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a psl::string8_t based on the input psl::string.
	inline psl::string8_t to_string8_t(const psl::string& s) { return psl::string8_t(std::begin(s), std::end(s)); }

	/// \brief converts a UTF-8 string into a psl::string
	///
	/// converts a UTF-8 string into a psl::string, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a psl::string based on the input UTF-8 string.
	inline psl::string from_string8_t(const psl::string8_t& s) { return psl::string(std::begin(s), std::end(s)); }

	/// \brief converts a psl::string into a UTF-8 string.
	///
	/// converts a psl::string into a UTF-8 string, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a psl::string8_t based on the input psl::string.
	inline psl::string8_t to_string8_t(psl::string_view s) { return psl::string(std::begin(s), std::end(s)); }

	/// \brief converts a UTF-8 string into a psl::string
	///
	/// converts a UTF-8 string into a psl::string, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a psl::string based on the input UTF-8 string.
	inline psl::string from_string8_t(psl::string8::view s) { return psl::string8_t(std::begin(s), std::end(s)); }


	/// \brief converts a psl::string8_t int a psl::pstring
	///
	/// converts a psl::string8_t into a psl::pstring, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a psl::pstring_t based on the input psl::string.
	psl::pstring_t to_pstring(const psl::string8_t& s);

	/// \brief converts a psl::string int a psl::pstring
	///
	/// converts a psl::string into a psl::pstring, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a psl::pstring_t based on the input psl::string.
	psl::pstring_t to_pstring(psl::string8::view s);


	/// \brief converts a psl::string int a psl::pstring
	///
	/// converts a psl::string into a psl::pstring, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a std::wstring based on the input psl::string.
	psl::pstring_t to_pstring(const psl::string16_t& s);

	/// \brief converts a psl::string int a psl::pstring
	///
	/// converts a psl::string into a psl::pstring, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s string to convert.
	/// \returns a std::wstring based on the input psl::string.
	psl::pstring_t to_pstring(psl::string16::view s);

#if defined(UNICODE)
	/// \brief converts a std::string int a psl::string
	///
	/// converts a std::string into a psl::string, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s wstring to convert.
	/// \returns a psl::string based on the input std::wstring.
	psl::string8_t to_string8_t(psl::platform::view s);

	/// \brief converts a std::wstring int a psl::string
	///
	/// converts a std::wstring into a psl::string, depending on the bit-size (8 or 16) of psl::string
	/// this turns into a no-op.
	/// \param[in] s wstring to convert.
	/// \returns a psl::string based on the input std::wstring.
	psl::string8_t to_string8_t(const psl::pstring_t& s);
#endif


#endif
//#ifdef UNICODE
//	static decltype(std::wcin)& cin   = std::wcin;
//	static decltype(std::wcout)& cout = std::wcout;
//	static decltype(std::wcerr)& cerr = std::wcerr;
//	static decltype(std::wclog)& clog = std::wclog;
//#else
//	static decltype(std::cin)& cin = std::cin;
//	static decltype(std::cout)& cout = std::cout;
//	static decltype(std::cerr)& cerr = std::cerr;
//	static decltype(std::clog)& clog = std::clog;
//#endif

	/// \brief std::memset wrapper that auto-converts to std::wmemset when wchar == psl::char_t
	/// \param[in] _Dst destination location of what to memset
	/// \param[in] _Val the value to set the _Dst to.
	/// \param[in] _Size how many times this should be repeated.
	/// \returns the location on success, or a nullptr on failure.
	inline void* memset(char_t* _Dst, char_t _Val, size_t _Size)
	{
#if defined(PLATFORM_WINDOWS)
		if constexpr(sizeof(char_t) == sizeof(wchar_t))
		{
			return std::wmemset((wchar_t*)_Dst, _Val, _Size);
		}
		else
		{
			return std::memset(_Dst, _Val, _Size);
		}
#elif defined(PLATFORM_LINUX)
		return std::memset(_Dst, _Val, _Size);
#endif
		return nullptr;
	}

	/// \brief wrapper for the platform specific popen command, changes its internals depending on psl::char_t size.
	/// \param[in] _Command the command to execute.
	/// \param[in] _Mode mode of the returned stream.
	/// \returns a FILE* handle on success.
	/// \warning this method is platform specific, please review the internals before calling so that it satisfies the
	/// requirements of the platform you call it on.
	inline FILE* popen(const char_t* _Command, const char_t* _Mode)
	{
#if defined(PLATFORM_WINDOWS)
		if constexpr(sizeof(char_t) == sizeof(wchar_t))
		{
			return _wpopen((const wchar_t*)_Command, (const wchar_t*)_Mode);
		}
		else
		{
			return _popen((const char*)_Command, (const char*)_Mode);
		}
#elif defined(PLATFORM_LINUX)
		// return popen((const char*)_Command, (const char*)_Mode);
#endif
		return nullptr;
	}

	/// \brief wrapper around pclose.
	/// \param[in] _Stream the FILE* handle you wish to close.
	/// \returns a platform specific success code depending on what happened.
	/// \warning this method is platform specific, please review the internals before calling so that it satisfies the
	/// requirements of the platform you call it on.
	inline int pclose(FILE* _Stream)
	{
#if defined(PLATFORM_WINDOWS)
		return _pclose(_Stream);
#elif defined(PLATFORM_LINUX)
		// return pclose(_Stream);
#endif
		return 0;
	}

	/// \brief wrapper around std::memcpy that changes behaviour depending on the size of psl::char_t.
	/// \param[in] _Destination the destination to memcpy to.
	/// \param[in] _Source the source to memcpy from.
	/// \param[in] _SourceSize the amount of bytes to copy from the source.
	inline char_t* memcpy_s(char_t* const _Destination, char_t const* const _Source, size_t const _SourceSize)
	{
		if constexpr(sizeof(char_t) == sizeof(wchar_t))
		{
			return (char_t*)std::wmemcpy((wchar_t* const)_Destination, (wchar_t const* const)_Source, _SourceSize);
		}
		else
		{
			return (char_t*)std::memcpy(_Destination, _Source, _SourceSize);
		}
	}

	/// \brief wrapper around ::strlen that changes behaviour depending on the size of psl::char_t.
	/// \param[in] _Str the target string to count.
	/// \returns the size of the string.
	/// \note this method works on the "platforms expectations" of the string, not the actual UTF encoding.
	inline size_t strlen(const char_t* _Str)
	{
		if constexpr(sizeof(char_t) == sizeof(wchar_t))
		{
			return std::wcslen((const wchar_t*)_Str);
		}
		else
		{
			return ::strlen((const char*)_Str);
		}
	}

	/// \brief wrapper around fprintf that changes behaviour depending on the size of psl::char_t.
	/// \param[in] _Stream pointer to a FILE object that identifies an output stream.
	/// \param[in] _Format text you wish to write to the stream, with optional formatting instructions.
	/// \param[in] args stream of variables you wish to send to forward to fprintf, the behaviour is unchanged.
	template <typename... Args>
	inline void fprintf(FILE* const _Stream, char_t const* _Format, Args&&... args)
	{
		if constexpr(sizeof(char_t) == sizeof(wchar_t))
		{
			::fwprintf(_Stream, (const wchar_t*)_Format, std::forward<Args>(args)...);
		}
		else
		{
			::fprintf(_Stream, (const char*)_Format, std::forward<Args>(args)...);
		}
	}

	/// \brief wrapper around printf that changes behaviour depending on the size of psl::char_t.
	/// \param[in] _Format text you wish to write to the stream, with optional formatting instructions.
	/// \param[in] args stream of variables you wish to send to forward to fprintf, the behaviour is unchanged.
	template <typename... Args>
	inline void printf(char_t const* _Format, Args&&... args)
	{
		if constexpr(sizeof(char_t) == sizeof(wchar_t))
		{
			::wprintf((const wchar_t*)_Format, std::forward<Args>(args)...);
		}
		else
		{
			::printf((const char*)_Format, std::forward<Args>(args)...);
		}
	}
} // namespace psl

/// \brief helper operator that allows you to create a new psl::string8_t from adding it to a string_view
inline psl::string8_t operator+(const psl::string8_t& a, psl::string8::view b)
{
	psl::string8_t res{a};
	res.append(b.data(), b.size());
	return res;
}

/// \brief helper operator that allows you to create a new psl::string8_t from adding it to a string_view
inline psl::string8_t operator+(psl::string8::view a, const psl::string8_t& b)
{
	psl::string8_t res{a};
	res.append(b.data(), b.size());
	return res;
}