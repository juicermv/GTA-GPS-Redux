#
# CMake build script for GPS Redux
#
# This script supports both MSVC (Visual Studio) and MinGW (GCC) compilers.
#
# --- How to Build ---
#
# 1. Create a build directory:
#    mkdir build && cd build
#
# 2. Generate the project files for your compiler:
#
#    For Visual Studio 2022 (32-bit):
#    cmake .. -G "Visual Studio 17 2022" -A Win32
#
#    For MinGW Makefiles (32-bit):
#    cmake .. -G "MinGW Makefiles"
#
# 3. Compile the project:
#    cmake --build . --config Release
#

# 1. Project Setup
cmake_minimum_required(VERSION 3.15)
project(GPS_Redux LANGUAGES CXX)

# Set C++ standard to C++23, matching the build scripts
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Fetch Dependencies using FetchContent
include(FetchContent)

# Define dependency URLs and subdirectories based on the compiler
if(MSVC)
    set(PLUGIN_SDK_URL "https://github.com/juicermv/plugin-sdk/releases/latest/download/msvc.zip")
    set(SIMDSTRING_URL "https://github.com/juicermv/SIMDString/releases/download/release/msvc.zip")
    set(SIMDSTRING_SUBDIR "SIMDStringX86")
else() # Assuming MinGW or other GCC-like compiler
    set(PLUGIN_SDK_URL "https://github.com/juicermv/plugin-sdk/releases/latest/download/mingw.zip")
    set(SIMDSTRING_URL "https://github.com/juicermv/SIMDString/releases/download/release/mingw.zip")
    # The MinGW zip does not have an extra subdirectory
    set(SIMDSTRING_SUBDIR "")
endif()

# Declare dependencies
FetchContent_Declare(plugin_sdk URL ${PLUGIN_SDK_URL})
FetchContent_Declare(simd_string URL ${SIMDSTRING_URL})

# Make dependencies available (downloads and extracts them into the build directory)
FetchContent_MakeAvailable(plugin_sdk simd_string)

# 3. Gather Source Files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 4. Define the Library Target
add_library(gps_redux SHARED ${SOURCES})

# Set the output name to SA.GPS.REDUX.asi
set_target_properties(gps_redux PROPERTIES
        OUTPUT_NAME "SA.GPS.REDUX"
        SUFFIX ".asi"
)

# 5. Configure Include and Link Paths
# Common include paths
target_include_directories(gps_redux PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/mini/src"
)

# Dependency include paths
target_include_directories(gps_redux PRIVATE
        "${plugin_sdk_SOURCE_DIR}/src"
        "${plugin_sdk_SOURCE_DIR}/src/plugin_sa"
        "${plugin_sdk_SOURCE_DIR}/src/plugin_sa/game_sa"
        "${plugin_sdk_SOURCE_DIR}/src/plugin_sa/game_sa/rw"
        "${plugin_sdk_SOURCE_DIR}/src/shared"
        "${plugin_sdk_SOURCE_DIR}/src/shared/game"
        "${simd_string_SOURCE_DIR}/${SIMDSTRING_SUBDIR}"
)

# Dependency library paths
target_link_directories(gps_redux PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/external/d3dx9"
        "${plugin_sdk_SOURCE_DIR}/lib/plugin_sa"
        "${simd_string_SOURCE_DIR}/${SIMDSTRING_SUBDIR}"
)

# 6. Configure Preprocessor Definitions
target_compile_definitions(gps_redux PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_NON_CONFORMING_SWPRINTFS
        GTASA
        "GTAGAME_NAME=\"San Andreas\""
        "GTAGAME_ABBR=\"SA\""
        "GTAGAME_ABBRLOW=\"sa\""
        "GTAGAME_PROTAGONISTNAME=\"CJ\""
        "GTAGAME_CITYNAME=\"San Andreas\""
        _LA_SUPPORT
        _DX9_SDK_INSTALLED
        PLUGIN_SGV_10US
        _USE_MATH_DEFINES
        RW
)

# Add _NDEBUG definition only for Release builds
target_compile_definitions(gps_redux PRIVATE
        $<$<CONFIG:Release>:_NDEBUG>
)

# 7. Compiler and Linker Specific Configuration
if(MSVC)
    # This project must be compiled as 32-bit (x86).
    # We check the pointer size to enforce this.
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "This project must be compiled as 32-bit (x86). Please use the correct generator platform (e.g., -A Win32 for Visual Studio).")
    endif()

    # MSVC Compiler Flags
    target_compile_options(gps_redux PRIVATE
            /std:c++latest /W3 /EHsc /MT
            # Optimization flags from script
            /Ox /Ob1 /GT /fp:fast /Gy /Oi
    )

    # MSVC Linker Flags
    target_link_options(gps_redux PRIVATE
            /SUBSYSTEM:WINDOWS /DLL
            # Optimization flags from script
            /OPT:ICF /OPT:REF
    )

    # MSVC Libraries
    target_link_libraries(gps_redux PRIVATE
            plugin.lib d3d9.lib d3dx9.lib simdstring.lib
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang") # MinGW/GCC/Clang
    # Enforce 32-bit build and enable C++23 with GNU extensions
    target_compile_options(gps_redux PRIVATE
            -m32 -std=gnu++23 -fpermissive
            # Optimization
            -Ofast
            # Add debug symbols only in Debug build type
            $<$<CONFIG:Debug>:-g>
    )

    # MinGW Linker Flags
    target_link_options(gps_redux PRIVATE
            -m32 --subsystem,windows --export-all-symbols
            # Statically link runtime libraries for better portability
            -static-libgcc -static-libstdc++
    )

    # MinGW Libraries (CMake handles lib prefix and .a suffix)
    target_link_libraries(gps_redux PRIVATE
            plugin d3d9 d3dx9 SIMDString
    )

else()
    message(WARNING "Compiler not recognized. Build might fail. Supported compilers: MSVC, MinGW/GCC.")
endif()

# 8. Post-Build Step: Copy additional files to the output directory
add_custom_command(
        TARGET gps_redux POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        "$<TARGET_FILE_DIR:gps_redux>/README.md"
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/SA.GPS.CONF.ini"
        "$<TARGET_FILE_DIR:gps_redux>/SA.GPS.CONF.ini"
        COMMENT "Copying additional files to output directory"
)