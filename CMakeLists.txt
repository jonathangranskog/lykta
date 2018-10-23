cmake_minimum_required(VERSION 3.5)

project(lykta)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Embree
find_package(embree 3.0 REQUIRED)
include_directories(${EMBREE_INCLUDE_DIRS})

# GLM
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)

# NANOGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
add_subdirectory(ext/nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
include_directories(ext/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

file(GLOB src_files
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.h	
	)

add_executable(lykta ${src_files})

target_link_libraries(lykta nanogui ${NANOGUI_EXTRA_LIBS} ${EMBREE_LIBRARY})