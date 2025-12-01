cmake_minimum_required(VERSION 3.15)
project(Mp4GifConverter LANGUAGES CXX)

# 1. Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Define the path hint for Windows
# We use this variable to tell CMake where to look if it can't find FFmpeg in system folders.
set(FFMPEG_ROOT "D:/dev/opensource/ffmpeg_build" CACHE PATH "Path to FFmpeg install dir")

# VERIFICATION: Check if FFmpeg actually exists and is already built before trying to configure
if(NOT EXISTS "${FFMPEG_ROOT}/include")
    message(FATAL_ERROR "❌ FFmpeg headers not found! Checked: ${FFMPEG_ROOT}/include")
endif()
if(WIN32 AND NOT EXISTS "${FFMPEG_ROOT}/bin")
    message(FATAL_ERROR "❌ FFmpeg bin folder not found! Checked: ${FFMPEG_ROOT}/bin")
endif()
message(STATUS "✅ FFmpeg Root detected: ${FFMPEG_ROOT}")

# 3. Find Headers (Look for avformat.h)
find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h
    HINTS "${FFMPEG_ROOT}/include"
    PATH_SUFFIXES include
)

# 4. Find Libraries
# We use a macro to make this repetitive task cleaner
macro(find_ffmpeg_lib var_name lib_name)
    find_library(${var_name}
        NAMES ${lib_name}
        HINTS "${FFMPEG_ROOT}/lib"
        PATH_SUFFIXES lib
    )
endmacro()

find_ffmpeg_lib(AVFILTER_LIB  avfilter)
find_ffmpeg_lib(AVFORMAT_LIB  avformat)
find_ffmpeg_lib(AVCODEC_LIB   avcodec)
find_ffmpeg_lib(SWSCALE_LIB   swscale)
find_ffmpeg_lib(SWRESAMPLE_LIB swresample) # Usually required by avcodec
find_ffmpeg_lib(AVUTIL_LIB    avutil)

# 5. Define the Executable
add_executable(mp4gif src/mp4gif.cpp)

# 6. Link Libraries
# Note: Order matters! High-level libs first, low-level last.
if (AVFORMAT_INCLUDE_DIR AND AVFORMAT_LIB)
    target_include_directories(mp4gif PRIVATE ${AVFORMAT_INCLUDE_DIR})
    
    target_link_libraries(mp4gif PRIVATE
        ${AVFILTER_LIB}
        ${AVFORMAT_LIB}
        ${AVCODEC_LIB}
        ${SWSCALE_LIB}
        ${SWRESAMPLE_LIB} # Best practice to include this
        ${AVUTIL_LIB}
    )
    
    message(STATUS "FFmpeg found at: ${AVFORMAT_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "Could not find FFmpeg. Please check FFMPEG_ROOT.")
endif()

# 7. Platform Specifics: Windows System Libraries
# If linking statically on Windows, we need standard Windows libs that FFmpeg uses.
if(WIN32)
    target_link_libraries(mp4gif PRIVATE
        ws2_32  # Winsock
        bcrypt  # Cryptography
        secur32 # Security
        shlwapi # Shell API
        strmiids # Media streaming
    )
endif()


# 8. Post-Build: Create 'dist' and Copy dependency DLLs
if(WIN32)
    # A. Gather FFmpeg DLLs
    file(GLOB FFMPEG_DLLS "${FFMPEG_ROOT}/bin/*.dll")

    # B. Gather Compiler Runtime DLLs (MinGW/GCC specific)
    # We look in the same folder where g++.exe is located.
    set(COMPILER_DLLS "")
    if(MINGW)
        get_filename_component(COMPILER_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        
        # Look for standard MinGW runtime libraries
        # libstdc++ (C++ Standard Lib), libgcc (GCC Runtime), libwinpthread (Threading)
        file(GLOB COMPILER_DLLS 
            "${COMPILER_BIN_DIR}/libstdc++-*.dll"
            "${COMPILER_BIN_DIR}/libgcc_s_*.dll"
            "${COMPILER_BIN_DIR}/libwinpthread-*.dll"
            # Compression Libraries (Required by FFmpeg)
            "${COMPILER_BIN_DIR}/zlib1.dll"
            "${COMPILER_BIN_DIR}/libbz2-*.dll"
            "${COMPILER_BIN_DIR}/liblzma-*.dll"
            "${COMPILER_BIN_DIR}/libiconv-*.dll" # Character encoding        
        )
        
        message(STATUS "Found Compiler Runtime DLLs in: ${COMPILER_BIN_DIR}")
    endif()

    # C. Execute the Copy
    # We copy to ${CMAKE_CURRENT_BINARY_DIR}, which is the build folder containing the exe.
    add_custom_command(TARGET mp4gif POST_BUILD
        
        # 1. Copy FFmpeg DLLs
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${FFMPEG_DLLS} 
            "${CMAKE_CURRENT_BINARY_DIR}"

        # 2. Copy Compiler Runtime DLLs (if any were found)
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${COMPILER_DLLS} 
            "${CMAKE_CURRENT_BINARY_DIR}"

        COMMENT "Copying FFmpeg and Compiler Runtime DLLs next to executable..."
    )
endif()