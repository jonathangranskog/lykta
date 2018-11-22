cmake_minimum_required(VERSION 3.12)

project(lykta CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Embree
find_package(embree 3.0 REQUIRED)
include_directories(${EMBREE_INCLUDE_DIRS})

# Torch
find_package(Torch)

# GLM
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)

# STB
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)

# OpenMP
if (APPLE)
# This works with default 'brew install libomp'
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I$(brew --prefix libomp)/include")
    include_directories(/usr/local/opt/libomp/include)
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY /usr/local/opt/libomp/lib/libomp.a)
endif()
find_package(OpenMP REQUIRED)

# NANOGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
add_subdirectory(ext/nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
include_directories(ext/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

# tinyobj
set(tiny_obj_files ${CMAKE_CURRENT_SOURCE_DIR}/src/tinyobj/tiny_obj_loader.h ${CMAKE_CURRENT_SOURCE_DIR}/src/tinyobj/tiny_obj_loader.cc)

# rapidjson
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL " " FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL " " FORCE)
add_subdirectory(ext/rapidjson/rapidjson)
include_directories(ext/rapidjson/rapidjson/include)

# filesystem
include_directories(ext/filesystem)

file(GLOB src_files
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_executable(lykta ${src_files} ${tiny_obj_files})

target_link_libraries(lykta nanogui ${NANOGUI_EXTRA_LIBS} ${EMBREE_LIBRARY} OpenMP::OpenMP_CXX "${TORCH_LIBRARIES}")