# in tree builds are not allowed
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "ERROR: detected an in-tree build. please create a sub-directory and invoke cmake from there, or a location outside the project.")
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    message(FATAL_ERROR "we require a 64bit build platform")
endif()

cmake_minimum_required(VERSION 3.11)
set(PROJECT_PARADIGM paradigm CACHE INTERNAL "")
project(${PROJECT_PARADIGM} VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMakeDependentOption)

if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(LINUX TRUE)
endif()

###############################################################################
###                    general options and setup                            ###
###############################################################################
OPTION(PE_PCH "generate and use precompiled headers (if supported)" OFF)
OPTION(PE_SOURCE_ONLY "set to off to not compile the sub-projects" OFF)
set(PE_MODE $ENV{PE_MODE} CACHE STRING "should the core library be compiled as library (default), executable, or library without tests.")
set_property(CACHE PE_MODE PROPERTY STRINGS LIB EXE, LIB_NO_TESTS)
OPTION(PE_PROFILER "enables the internal profiler" OFF)
OPTION(PE_ANALYZE "enables static analyzers for the given platform" OFF)
OPTION(PE_VERBOSE "verbose error checking in the default compile options" OFF)
set(VK_VERSION "" CACHE STRING "The vulkan version to use")
OPTION(VK_STATIC "Should we statically bind Vulkan (if possible)" OFF)
OPTION(PE_GLES "controls OpenGL ES support" ON)
OPTION(PE_VULKAN "controls Vulkan support (experimental - not done)" ON)
OPTION(PE_MOLTEN "controls Vulkan support (experimental - not done)" OFF)

include(FindXCB)
include(FindWayland)
OPTION(PE_WSI_D2D "use Direct to Display" OFF)
CMAKE_DEPENDENT_OPTION(PE_WSI_WAYLAND "use Wayland" ON "Wayland_FOUND" OFF)
CMAKE_DEPENDENT_OPTION(PE_WSI_XCB "use XCB" ON "XCB_FOUND" OFF)
CMAKE_DEPENDENT_OPTION(PE_WSI_WIN32 "use win32 surface" ON "WIN32" OFF)

set(PE_PLATFORM "AUTO" CACHE STRING "Target platform to build for, leave to 'AUTO' to detect and select the platform for you.")
set_property(CACHE PE_PLATFORM PROPERTY STRINGS AUTO WINDOWS LINUX ANDROID)
set(PE_INSTRUCTION_SET "fallback" CACHE STRING "Instruction set to use for SIMD instructions")
set_property(CACHE PE_INSTRUCTION_SET PROPERTY STRINGS fallback SSE AVX AVX2)

OPTION(VS_USE_NATVIS "Configure project to use Visual Studio native visualizers when building for visual studio" TRUE)
OPTION(PE_BENCHMARKS "Enable the generation of benchmarks" OFF)

set(PE_BUILD_DIR "${CMAKE_BINARY_DIR}/builds/" CACHE PATH "target location where to build the project to")
set(PE_DEFINES -DUNICODE;-D_UNICODE;-DNOMINMAX CACHE INTERNAL "")
OPTION(PE_DEFAULT_COMPILE_OPTIONS "Use the default set of compile options (check 'compile options' section in the CMakelists.txt file)" TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(PE_MODE STREQUAL "EXE")
	list(APPEND PE_DEFINES -DCORE_EXECUTABLE)
endif()

if(PE_GLES)
	message("enabling GLES")
	list(APPEND PE_DEFINES -DPE_GLES)
endif()


list(APPEND PE_DEFINES -DPLATFORM_${PE_PLATFORM})

# experimental early support for not supporting vulkan
if(PE_VULKAN)
	message("enabling VULKAN")
	list(APPEND PE_DEFINES -DPE_VULKAN)
endif()

if(PE_MOLTEN)
	message("transforming graphics API calls to metal using Molten")
endif()

if(PE_INSTRUCTION_SET STREQUAL "SSE")	
	list(APPEND PE_DEFINES -DINSTRUCTION_SET=1)
elseif(PE_INSTRUCTION_SET STREQUAL "AVX")	
	list(APPEND PE_DEFINES -DINSTRUCTION_SET=2)
elseif(PE_INSTRUCTION_SET STREQUAL "AVX2")	
	list(APPEND PE_DEFINES -DINSTRUCTION_SET=3)
else()
	list(APPEND PE_DEFINES -DINSTRUCTION_SET=0)
endif()

if(PE_PROFILER)
	list(APPEND PE_DEFINES -DPE_PROFILER)
endif()


if(NOT VK_STATIC AND NOT PE_SOURCE_ONLY)	
	list(APPEND PE_DEFINES -DVK_NO_PROTOTYPES;-DVK_DYNAMIC_DISPATCH_VOLK)
elseif(VK_STATIC)
	list(APPEND PE_DEFINES -DVK_STATIC)
endif()

###############################################################################
###                    validation                                           ###
###############################################################################

if(WIN32)
	list(APPEND PE_DL_LIBS Shlwapi.lib)
endif()

###############################################################################
###                    setup output directories                             ###
###############################################################################
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PE_BUILD_DIR}/default/lib 
	CACHE PATH "Output directory for static libraries.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PE_BUILD_DIR}/default/lib
	CACHE PATH "Output directory for shared libraries.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PE_BUILD_DIR}/default/bin
	CACHE PATH "Output directory for executables and DLL's.")
	
file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
file(MAKE_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
file(MAKE_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
	string(TOLOWER ${OUTPUTCONFIG} OUTPUTCONFIG_FOLDERNAME)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${PE_BUILD_DIR}/${OUTPUTCONFIG_FOLDERNAME}/${ARCHI}/bin" CACHE PATH "" FORCE)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${PE_BUILD_DIR}/${OUTPUTCONFIG_FOLDERNAME}/${ARCHI}/lib" CACHE PATH "" FORCE)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${PE_BUILD_DIR}/${OUTPUTCONFIG_FOLDERNAME}/${ARCHI}/lib" CACHE PATH "" FORCE)
	
	file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}}")
	file(MAKE_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}}")
	file(MAKE_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}}")
endforeach()

if(PE_ANALYZE)
	if(MSVC)
	
	else()
		set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)
	endif()
	
	find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
	if (CMAKE_CXX_CPPCHECK)
			list(
					APPEND CMAKE_CXX_CPPCHECK 
							"--enable=warning"
							"--inconclusive"
							"--force" 
							"--inline-suppr"
							"--suppressions-list=${CMAKE_SOURCE_DIR}/tools/CppCheckSuppressions.txt"
			)
	endif()
	
endif()

if(PE_VERBOSE AND NOT MSVC)
	set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)
endif()
###############################################################################
###                    compile options                                      ###
###############################################################################

set(CMAKE_CXX_EXTENSIONS OFF )
set(CXX_EXTENSIONS OFF )
list(APPEND PE_COMPILER_FEATURES cxx_std_17)

if(PE_GLES)		
		find_package(OpenGL REQUIRED)
		list(APPEND PE_DL_LIBS ${OPENGL_LIBRARIES})
endif()

if(MSVC) # we currently make no distinction for CLang (todo)
	add_compile_options(/MP)
	list(APPEND PE_COMPILE_OPTIONS /nologo;/sdl;/permissive-;/Zc:forScope;/Zc:inline;/Zc:rvalueCast;/Zc:wchar_t;/fp:precise;/JMC)
	list(APPEND PE_COMPILE_OPTIONS /wd4505)
	if(PE_VERBOSE)
		list(APPEND PE_COMPILE_OPTIONS /W4 /w14640 /w14242 /w14254 /wd4100)	
	else()
		list(APPEND PE_COMPILE_OPTIONS /W3)
	endif()
	list(APPEND PE_DEFINES -DWIN32_LEAN_AND_MEAN)
	
	
	list(APPEND PE_COMPILE_OPTIONS_EXE
							$<$<CONFIG:Debug>:             /MDd;/Od;/RTC1;/Z7>
							$<$<CONFIG:Release>:           /MD /O2;/Oi;/Gy-;/GL;/Zi>
							$<$<CONFIG:RelWithDebInfo>:    /MD;/O2;/Oi;/Gy-;/GL;/Zo;/Z7>
							$<$<CONFIG:RelNoOptimization>: /MDd;/Od;/RTC1;/Zi>)
	list(APPEND PE_COMPILE_OPTIONS_LIB
							$<$<CONFIG:Debug>:             /MDd;/Od;/RTC1;/Z7>
							$<$<CONFIG:Release>:           /MD;/O2;/Oi;/Gy-;/Zi>
							$<$<CONFIG:RelWithDebInfo>:    /MD;/O2;/Oi;/Gy-;/Zo;/Z7>
							$<$<CONFIG:RelNoOptimization>: /MDd;/Od;/RTC1;/Zi>)
							
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/INCREMENTAL /DEBUG /OPT:NOREF /OPT:NOICF")
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "/INCREMENTAL:NO /OPT:REF /OPT:ICF /LTCG")
	SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL /DEBUG /OPT:NOREF /OPT:NOICF /LTCG:incremental")
	SET(CMAKE_EXE_LINKER_FLAGS_RELNOOPTIMIZATION "/INCREMENTAL /OPT:NOREF /OPT:NOICF")
else()
	if(PE_VERBOSE)
		list(APPEND PE_COMPILE_OPTIONS -Wall;-Wextra)	
	endif()
	list(APPEND PE_COMPILE_OPTIONS -stdlib=libc++;-fstrict-aliasing;-Wno-error=unused-command-line-argument;-Wno-switch;-Wno-unknown-pragmas;-Wno-unused-function;-std=c++1z;-g)
	list(APPEND PE_DL_LIBS c++fs)
	
	list(APPEND PE_COMPILE_OPTIONS
							$<$<CONFIG:Debug>:             -gdwarf-2>
							$<$<CONFIG:Release>:           -fomit-frame-pointer;-ffunction-sections;-fdata-sections>)
							
	# Add colors to ninja builds
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options (-fdiagnostics-color=always)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_compile_options (-fcolor-diagnostics)
	endif()
	
	if (APPLE)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -dead_strip -g")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -dead_strip")
		
		# tell ranlib to ignore empty compilation units
		set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
		set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols <TARGET>")
		# prevents ar from invoking ranlib, let CMake do it
		set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qc -S <TARGET> <LINK_FLAGS> <OBJECTS>")
		set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qc -S <TARGET> <LINK_FLAGS> <OBJECTS>")
	else()
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}  -dead_strip -g")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -dead_strip")
	endif()
endif()

###############################################################################
###                    setup WSI defines                                    ###
###############################################################################

set(HAS_WSI FALSE)
if(NOT HAS_WSI AND PE_WSI_D2D)
	list(APPEND PE_DEFINES -DSURFACE_D2D)
	message("using D2D WSI")
	set(HAS_WSI TRUE)
endif()
if(NOT HAS_WSI AND PE_WSI_WAYLAND AND WAYLAND_FOUND)
	list(APPEND PE_DEFINES -DSURFACE_WAYLAND;-DVK_USE_PLATFORM_WAYLAND_KHR)
	set(WSI_LIB ${WAYLAND_CLIENT_LIBRARIES})
	set(WSI_INC ${WAYLAND_INCLUDE_DIR})
	message("using Wayland WSI")
	set(HAS_WSI TRUE)
endif()
if(NOT HAS_WSI AND PE_WSI_XCB AND XCB_FOUND)
	list(APPEND PE_DEFINES -DSURFACE_XCB;-DVK_USE_PLATFORM_XCB_KHR)
	set(WSI_LIB ${XCB_LIBRARIES})
	message("using XCB WSI")
	set(HAS_WSI TRUE)
endif()
if(NOT HAS_WSI AND PE_WSI_WIN32)
	list(APPEND PE_DEFINES -DSURFACE_WIN32;-DVK_USE_PLATFORM_WIN32_KHR)
	message("using WIN32 WSI")
	set(HAS_WSI TRUE)
endif()
if(NOT HAS_WSI)
	message(FATAL_ERROR "Could not find a suitable WSI, or none was selected")
endif()

###############################################################################
###                    Add the sub projects                                 ###
###############################################################################

add_definitions(${PE_DEFINES})

if(NOT PE_SOURCE_ONLY)	
	add_subdirectory(psl)
	add_subdirectory(core)
	if(PE_MODE STREQUAL "LIB")
		enable_testing()
		add_subdirectory(tests)
	endif()
	
	if(PE_BENCHMARKS)
		add_subdirectory(benchmarks)	
	endif()

	if(EXISTS "${PROJECT_SOURCE_DIR}/modules.txt")
		include(${PROJECT_SOURCE_DIR}/modules.txt)
	endif()
else()
	message("did not compile the project by request")
endif()

SET(Python_ADDITIONAL_VERSIONS 3 3.6 3.5 3.4 3.3 3.2 3.1 3.0)
find_package(PythonInterp REQUIRED)

message("Patching files in ${CMAKE_BINARY_DIR}")
EXECUTE_PROCESS(
			COMMAND ${PYTHON_EXECUTABLE} tools/patch.py ${CMAKE_BINARY_DIR}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)