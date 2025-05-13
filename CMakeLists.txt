cmake_minimum_required(VERSION 3.15)
project(VisionModesApp)

# Detect macOS SDK path
execute_process(
    COMMAND xcrun --sdk macosx --show-sdk-path
    OUTPUT_VARIABLE OSX_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_OSX_SYSROOT ${OSX_SDK_PATH})

# Enable debug symbols
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Use pkg-config to find OpenCV (Homebrew-based)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED opencv4)

# Include OpenCV headers and flags
include_directories(${OpenCV_INCLUDE_DIRS})
add_definitions(${OpenCV_CFLAGS_OTHER})
link_directories(${OpenCV_LIBRARY_DIRS})

# Set output dir to repo root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add executable
add_executable(vision_modes main.cpp)

# Link OpenCV (all modules except gapi)
set(FILTERED_LIBS "")
foreach(lib ${OpenCV_LIBRARIES})
    if(NOT lib MATCHES "gapi")
        list(APPEND FILTERED_LIBS ${lib})
    endif()
endforeach()

target_link_libraries(vision_modes PRIVATE ${FILTERED_LIBS})
