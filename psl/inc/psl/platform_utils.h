#pragma once

#include "psl/platform_def.h"
#include <vector>
#include "psl/ustring.h"
#include "psl/string_utils.h"
#include <cctype>
#include <clocale>
#ifndef PLATFORM_ANDROID
#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std::filesystem
{
	using namespace experimental::filesystem;
}
#endif
#endif
#include <optional>
#include <unordered_map>
#include <thread>


// https://en.wikipedia.org/wiki/Path_%28computing%29#Representations_of_paths_by_operating_system_and_shell

namespace utility
{
	/// \brief contains some debug information, such as trace information, as well as utilities to debug.
	class debug
	{
	  public:
		/// \brief contains the structure of a trace
		struct trace_info
		{
			/// \brief UTF-8 encoded psl::string that contains the demangled information of a trace.
			psl::string name;
			/// \brief address location in memory of the traced function.
			std::uintptr_t addr;
		};

		/// \brief method to get trace information of the current callstack.
		/// \param[in] offset what height of the callstack to start from (0 being the bottom, not root of the
		/// callstack). \param[in] depth how far up should we get information from (i.e. a depth of 1 would only get the
		/// current information, while 255 would likely get you to the root). \returns an std::vector containing a
		/// struct of trace_info of the current callstack (excluding this method).
		static std::vector<trace_info> trace(size_t offset = 0u, size_t depth = 255u,
											 std::optional<std::thread::id> id = std::nullopt);


		/// \brief method to get raw trace information of the current callstack.
		/// \param[in] offset what height of the callstack to start from (0 being the bottom, not root of the
		/// callstack). \param[in] depth how far up should we get information from (i.e. a depth of 1 would only get the
		/// current information, while 255 would likely get you to the root). \returns an std::vector containing void
		/// pointers pointing to the address of the current callstack (excluding this method).
		static std::vector<void*> raw_trace(size_t offset = 0u, size_t depth = 255u);

		static utility::debug::trace_info demangle(void* target);
		/// \brief helper method to give a name to a specific thread.
		/// \param[in] id thread::id that will be named.
		/// \param[in] name the name to assign to the given thread::id.
		static void register_thread(const std::thread::id& id, const psl::string8_t& name)
		{
			m_ThreadMap.insert(std::pair<std::thread::id, psl::string8_t>(id, name));
		}

		/// \brief helper method to name the current (this) thread.
		/// \param[in] name the name to assign to std::this_thread.
		static void register_thread(const psl::string8_t& name) { register_thread(std::this_thread::get_id(), name); }

		/// \brief gets the name of the given thread::id.
		/// \param[in] id the id of the thread to look up.
		/// \returns the name (if any) that is assigned to the given thread::id.
		static const psl::string8_t thread_name(const std::thread::id& id) { return m_ThreadMap[id]; }

		/// \brief gets the name of the current (this) thread.
		/// \returns the name (if any) that is assigned to the current thread.
		static const psl::string8_t thread_name() { return thread_name(std::this_thread::get_id()); }

		/// \brief helper method to extract just the class from a trace.
		/// \param[in] fullFuncName the signature of the trace.
		/// \returns the class name.
		static const psl::string8_t func_to_class(const char* fullFuncName)
		{
			psl::string8_t fullFuncNameStr(fullFuncName);
			size_t pos = fullFuncNameStr.find_last_of("::");
			if(pos == psl::string8_t::npos)
			{
				return fullFuncNameStr;
			}
			psl::string8_t newName = fullFuncNameStr.substr(0, pos - 1);
			pos					   = newName.find_last_of("::");
			if(pos == psl::string8_t::npos)
			{
				return newName;
			}
			return newName.substr(pos + 1);
		}

		static const psl::string8_t func_to_namespace(const char* fullFuncName)
		{
			psl::string8_t fullFuncNameStr(fullFuncName);
			size_t pos = fullFuncNameStr.find_last_of("::");
			if(pos == psl::string8_t::npos)
			{
				return fullFuncNameStr;
			}
			return fullFuncNameStr.substr(0, pos - 1);
		}

	  private:
		static std::unordered_map<std::thread::id, psl::string8_t> m_ThreadMap;
	};
} // namespace utility

namespace utility::platform
{
	/// \brief helper class that contains directory specific I/O manipulation methods and platform utilities.
	class directory
	{
	  public:
		/// \brief the seperator character that all commands will be assumed to use, and all platform specific commands
		/// will be translated to/from
		static constexpr psl::string_view seperator = "/";
#ifdef PLATFORM_WINDOWS
		/// \brief the platform specific seperator for the current platform (i.e. the one the platform by default uses)
		static constexpr psl::string_view seperator_platform = "\\";
#else
		static constexpr psl::string_view seperator_platform = "/";
#endif
		/// \brief identifier that signifies the "current location" symbol in a file path.
		static constexpr psl::string_view current_symbol = ".";

		/// \brief identifier that signifies the "go up one location" symbol in a file path.
		static constexpr psl::string_view parent_symbol = "..";

		/// \brief translated the given path to one that is accepted on Unix systems
		/// \param[in] path the path to translate to one that works in unix systems.
		/// \returns a string that *should* work on unix systems, and satisfies the requirements.
		static psl::string to_unix(psl::string_view path)
		{
			psl::string dir;
			while(path.size() > 0 && (path[0] == '\"' || path[0] == '\''))
			{
				path = path.substr(1);
			}

			while(path.size() > 0 && (path[path.size() - 1] == '\"' || path[path.size() - 1] == '\''))
			{
				path = path.substr(0, path.size() - 1);
			}

			if(auto find = path.find(":\\\\"); find == 1u)
			{
				psl::char_t drive = utility::string::to_lower(path.substr(0u, 1u))[0];
				dir				  = "/mnt/" + psl::string(1, drive) + seperator + path.substr(4u);
			}
			else if(find = path.find(":\\"); find == 1u)
			{
				psl::char_t drive = utility::string::to_lower(path.substr(0u, 1u))[0];
				dir				  = "/mnt/" + psl::string(1, drive) + seperator + path.substr(3u);
			}
			else if(find = path.find(":/"); find == 1u)
			{
				psl::char_t drive = utility::string::to_lower(path.substr(0u, 1u))[0];
				dir				  = "/mnt/" + psl::string(1, drive) + seperator + path.substr(3u);
			}
			else
			{
				dir = path;
			}

			auto find = dir.find('\\');
			while(find < dir.size())
			{
				dir.replace(find, 1, "/");
				find = dir.find('\\');
			}

			return dir;
		}

		/// \brief translated the given path to one that is accepted on Windows systems
		/// \param[in] path the path to translate to one that works in Windows systems.
		/// \returns a string that *should* work on Windows systems, and satisfies the requirements.
		static psl::string to_windows(psl::string_view path)
		{
			psl::string dir;
			while(path.size() > 0 && (path[0] == '\"' || path[0] == '\''))
			{
				path = path.substr(1);
			}

			while(path.size() > 0 && (path[path.size() - 1] == '\"' || path[path.size() - 1] == '\''))
			{
				path = path.substr(0, path.size() - 1);
			}

			if(path[0] == '/')
			{
				auto offset = 1;
				if(path.substr(1, 3) == "mnt") offset = 5;
				auto index = path.find('/', offset);
				dir		   = path.substr(offset, index - offset) + ":" + path.substr(index);
			}
			else
			{
				dir = path;
			}

			auto find = dir.find('\\');
			while(find < dir.size())
			{
				dir.replace(find, 1, "/");
				find = dir.find('\\');
			}

			find = dir.find("//");
			while(find < dir.size())
			{
				dir.erase(find, 1);
				find = dir.find("//");
			}
			return dir;
		}

		/// \brief translated the given path to one that is accepted on the current platform
		/// \param[in] path the path to translate to one that works on the current platform.
		/// \returns a string that *should* work on the current platform, and satisfies the requirements.
		static psl::string to_platform(psl::string_view path)
		{
#ifdef PLATFORM_WINDOWS
			return to_windows(path);
#else
			return to_unix(path);
#endif
		}

		/// \brief translates the given path to the default path encoding that is used across this application as the
		/// standard. \param[in] path the path to translate. \returns a string that is translated to the
		/// application-wide standard encoding (unix).
		static psl::string to_generic(psl::string_view path) { return to_unix(path); }

		/// \brief checks if the given path is a directory.
		/// \param[in] path the path to check.
		/// \returns true in case it is a directory.
		/// \todo android platform always returns false. Check if there is a way around this or redesign this.
		static bool is_directory(psl::string_view path)
		{
#ifdef PLATFORM_ANDROID
			return false;
#else
			return std::filesystem::is_directory(to_platform(path));
#endif
		}

		/// \brief sanitizes the seperators into the application wide standard one.
		/// \param[in] path the path to sanitize.
		/// \returns the transformed input path.
		static psl::string sanitize(psl::string_view path) { return utility::string::replace_all(path, "\\", "/"); }

		/// \brief tries to erase/delete the given item this path points to.
		///
		/// tries to delete the file this path points  to, or the folder this path points to.
		/// \param[in] path the path to what you wish to be erased.
		/// \returns true if erasing the target item is successful.
		/// \todo android platform always fails this, is it possible this is not always the case?
		inline static bool erase(psl::string_view path)
		{
#ifdef PLATFORM_ANDROID
			return false;
#else
			return std::filesystem::remove(directory::to_platform(path));
#endif
		}
		/// \brief checks if the given path (absolute) points to a directory or not.
		/// \param[in] absolutePath the path to check.
		/// \returns true in case this path is valid and points to a directory.
		static bool exists(psl::string_view absolutePath);

		/// \brief creates a directory that satisfies this path.
		/// \param[in] absolutePath the path to create the directory at.
		/// \param[in] recursive should we make all non-existing parent directories as well (true) or fail (false) when
		/// the path to the given directory does not exist? \returns true if the path is now fully valid and the
		/// directory exists.
		static bool create(psl::string_view absolutePath, bool recursive = false);

		/// \brief gets all files in the target directory, no folders.
		///
		/// gets all files in the target directory exluding the sub-directories. It however can recursively walk the
		/// sub-directories when needed, it will only return their contained items though.
		/// \param[in] target_directory the directory to run a search from.
		/// \param[in] recursive should we also search the sub-directories (true), or not (false).
		/// \returns a container of elements of the type psl::string, of all found items, that are transformed to the
		/// generic path format. \todo on android no search is run.
		static std::vector<psl::string> all_files(psl::string_view target_directory, bool recursive)
		{
#ifdef PLATFORM_ANDROID
			return {};
#else
			auto folder = to_platform(target_directory);
			std::vector<psl::string> names;
			if(recursive)
			{
				for(std::filesystem::recursive_directory_iterator i(folder), end; i != end; ++i)
				{
					if(!std::filesystem::is_directory(i->path()))
					{
#ifdef UNICODE
						auto filename = utility::string::replace_all(psl::to_string8_t(i->path().generic_wstring()),
																	 "\\", seperator);
#else
						auto filename = utility::string::replace_all(i->path().generic_string(), "\\", seperator);
#endif
						names.push_back(filename);
					}
				}
			}
			else
			{
				for(std::filesystem::directory_iterator i(folder), end; i != end; ++i)
				{
					if(!std::filesystem::is_directory(i->path()))
					{
#ifdef UNICODE
						auto filename = utility::string::replace_all(psl::to_string8_t(i->path().generic_wstring()),
																	 "\\", seperator);
#else
						auto filename = utility::string::replace_all(i->path().generic_string(), "\\", seperator);
#endif
						names.push_back(filename);
					}
				}
			}
			return names;
#endif
		}
	};

	/// \brief file i/o and manpulations utilities namespace
	namespace file
	{
		/// \brief checks if the given path points to a file or not.
		///
		/// checks if the given path points to a file or not. This path can be relative to the current working
		/// directory, usually the app root is the working directory, or an absolute path. \param[in] filename the path
		/// to the file to check. \returns true in case the target is indeed a file, otherwise in all scenarios it
		/// returns false. \todo android lacks an implementation.
		inline static bool exists(psl::string_view filename)
		{
#ifdef PLATFORM_ANDROID
			return false;
#else
			return std::filesystem::exists(directory::to_platform(filename));
#endif
		}

		/// \brief erases the file ath the given path.
		/// \param[in] filename the path to the file.
		/// \returns true in case the file was successfuly erased.
		/// \todo android lacks an implementation.
		inline static bool erase(psl::string_view filename)
		{
#ifdef PLATFORM_ANDROID
			return false;
#else
			return std::filesystem::remove(directory::to_platform(filename));
#endif
		}

		/// \brief reads the given filepath's contents into a psl::char_t container.
		/// \param[in] filename the path to the file.
		/// \param[in, out] out the output container to read into.
		/// \returns true on successfuly reading the file. This can be false in case something went wrong.
		/// \note this function might not work in all scenarios on non-desktop platforms that have sandboxed away the
		/// interaction with the filesystem. Please verify that your target platform actually supports this function for
		/// the file you wish to load.
		static bool read(psl::string_view filename, std::vector<psl::char_t>& out,
						 size_t count = std::numeric_limits<size_t>::max())
		{
			psl::string file_name = directory::to_platform(filename);
			psl::ifstream file(file_name, std::ios::binary | std::ios::ate);
			if(!file.is_open())
			{
				psl::fprintf(stderr, "Cannot open file %s!\n", file_name.c_str());
				return false;
			}
			size_t size = std::min(static_cast<size_t>(file.tellg()), count);
			file.seekg(0u, std::ios::beg);
			out.resize(size);
			file.read(&out[0], size);
			file.close();
			return true;
		}

		/// \brief reads the given filepath's contents into a psl::string container.
		/// \param[in] filename the path to the file.
		/// \param[in, out] out the output container to read into.
		/// \returns true on successfuly reading the file. This can be false in case something went wrong.
		/// \note this function might not work in all scenarios on non-desktop platforms that have sandboxed away the
		/// interaction with the filesystem. Please verify that your target platform actually supports this function for
		/// the file you wish to load.
		static bool read(psl::string_view filename, psl::string& out, size_t count = std::numeric_limits<size_t>::max())
		{
			psl::string file_name = directory::to_platform(filename);
			std::ifstream file(file_name.c_str(), std::ios::binary | std::ios::ate);
			if(!file.is_open())
			{
				psl::fprintf(stderr, "Cannot open file %s!\n", file_name.c_str());
				return false;
			}
			size_t size = std::min(static_cast<size_t>(file.tellg()), count);
			file.seekg(0u, std::ios::beg);
			out.resize(size);
			file.read(&out[0], size);
			file.close();
			return true;
		}

		/// \brief reads the given filepath's contents into a psl::string container on success.
		/// \param[in] filename the path to the file.
		/// \returns a psl::string on success, and a std::nullopt on failure.
		/// \note this function might not work in all scenarios on non-desktop platforms that have sandboxed away the
		/// interaction with the filesystem. Please verify that your target platform actually supports this function for
		/// the file you wish to load.
		static std::optional<psl::string> read(psl::string_view filename,
											   size_t count = std::numeric_limits<size_t>::max())
		{
			psl::string res;
			if(!read(filename, res, count)) return std::nullopt;
			return res;
		}

		/// \brief write the contents to the given location on the filesystem.
		/// \param[in] filename the path to the target location.
		/// \param[in] content the content to write at the given location.
		/// \returns true when the content has successfully been written at the target location.
		static bool write(psl::string_view filename, psl::string_view content)
		{
			auto file_name = directory::to_platform(filename);

			std::size_t found = file_name.find_last_of(directory::seperator);

			if(!directory::exists(file_name.substr(0, found)) && !directory::create(file_name.substr(0, found), true))
				return false;

			psl::ofstream output;
			try
			{
				output.open(file_name, std::ios::trunc | std::ios::out | std::ios::binary);
			}
			catch(...)
			{
				psl::fprintf(stderr, "Could not write the file: %s!\n", file_name.c_str());
				return false;
			}
			output.write(content.data(), content.size());
			output.close();
			return true;
		}

		/// \brief transforms the given path to the unix format.
		/// \param[in] path the path to transform.
		/// \returns the transformed path.
		static psl::string to_unix(psl::string_view path) { return directory::to_unix(path); }

		/// \brief transforms the given path to the windows format.
		/// \param[in] path the path to transform.
		/// \returns the transformed path.
		static psl::string to_windows(psl::string_view path) { return directory::to_windows(path); }

		/// \brief transforms the given path to the current platforms format.
		/// \param[in] path the path to transform.
		/// \returns the transformed path.
		static psl::string to_platform(psl::string_view path)
		{
#ifdef PLATFORM_WINDOWS
			return to_windows(path);
#else
			return to_unix(path);
#endif
		}

		/// \brief transforms the given path to the application wide standard format.
		/// \param[in] path the path to transform.
		/// \returns the transformed path.
		static psl::string to_generic(psl::string_view path) { return to_unix(path); }
	}; // namespace file

	/// \brief platform specific utility that can freeze the console running on the current thread.
	static void cmd_prompt_presskeytocontinue()
	{
		if constexpr(platform == platform_t::windows)
		{
			system("pause");
		}
		else if constexpr(platform == platform_t::osx)
		{
			system("read");
		}
		else
		{
			static_assert("CMDPromptPressAnyKeyToContinue has not been implemented for the given platform.");
		}
	}

	/// \brief OS specific utility functions
	class os
	{
	  public:
		static std::vector<unsigned int> to_virtual_keycode(const psl::string8_t& key);
	};
} // namespace utility::platform
