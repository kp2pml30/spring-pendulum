cmake_minimum_required(VERSION 3.18)

project(double-spring-pendulum)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)
unset(supports_fast_math CACHE)
check_cxx_compiler_flag(-ffast-math supports_fast_math)
if (supports_fast_math)
	add_compile_options(-ffast-math)
else()
	message("No fast math")
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
add_executable(double-spring-pendulum main.cpp)
add_executable(plot-test plot-test.cpp)
target_link_libraries(double-spring-pendulum glfw GLU "${GLEW_LIBRARIES}" ${OPENGL_LIBRARIES})
target_link_libraries(plot-test glfw GLU "${GLEW_LIBRARIES}" ${OPENGL_LIBRARIES})
