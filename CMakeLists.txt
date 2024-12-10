cmake_minimum_required(VERSION 3.5)
project(vlkn)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Gather source and header files
file(GLOB_RECURSE vlkn_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE vlkn_HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE vlkn_MODELS CONFIGURE_DEPENDS "models/*.obj")
file(GLOB_RECURSE vlkn_SHADERS "shaders/*.frag" "shaders/*.vert")


# Set include directories
set(vlkn_INCLUDE_DIRS "")
foreach(_headerFile ${vlkn_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND vlkn_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES vlkn_INCLUDE_DIRS)

# Set release build flags
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} - -Ofast -march=native -mtune=native -DNDEBUG")

# Add library dependencies
include(FindPkgConfig)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Copy models to build directory
foreach (_model ${vlkn_MODELS})
	file(RELATIVE_PATH _relative_path ${CMAKE_SOURCE_DIR} ${_model})
	configure_file(${_model} ${CMAKE_BINARY_DIR}/${_relative_path} COPYONLY)
endforeach()

# Compile shaders
foreach(_shader ${vlkn_SHADERS})
  get_filename_component(_file_name ${_shader} NAME)
  set(_spirv ${CMAKE_BINARY_DIR}/"shaders/${_file_name}.spv")

  add_custom_command(
    OUTPUT ${_spirv}
    COMMAND ${CMAKE_COMMAND} -E make_directory shaders
    COMMAND glslangValidator -V ${_shader} -o ${_spirv}
    DEPENDS ${_shader}
  )

  list(APPEND vlkn_SPIRV ${_spirv})
endforeach()

# Create executable
add_executable(${PROJECT_NAME} ${vlkn_SOURCES})

# Add shader compilation as a dependency
add_custom_target(compile_shaders DEPENDS ${vlkn_SPIRV})
add_dependencies(${PROJECT_NAME} compile_shaders)

# Include directories
include_directories(${vlkn_INCLUDE_DIRS} vulkan glfw glm)

# Link libraries
target_link_libraries(${PROJECT_NAME} vulkan glfw glm::glm)

