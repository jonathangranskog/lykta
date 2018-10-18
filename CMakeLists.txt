cmake_minimum_required(VERSION 3.5)

project(lykta)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Embree
find_package(embree 3.0 REQUIRED)
include_directories(${EMBREE_INCLUDE_DIRS})

# GLM
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)

add_subdirectory(ext/nanogui)
include_directories(ext/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

file(GLOB src_files
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/*.h	
	)

add_executable(lykta ${src_files})

target_link_libraries(lykta nanogui ${NANOGUI_EXTRA_LIBS})