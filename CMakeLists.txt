cmake_minimum_required(VERSION 3.5)
project(vlkn)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Gather source and header files
file(GLOB_RECURSE vlkn_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE vlkn_HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS "shaders/shader.*")

# Set include directories
set(vlkn_INCLUDE_DIRS "")
foreach(_headerFile ${vlkn_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list(APPEND vlkn_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES vlkn_INCLUDE_DIRS)

# Set release build flags
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} - -Ofast -march=native -mtune=native -DNDEBUG")

# Custom command to compile shaders
add_custom_command(
    OUTPUT shaders/vert.spv shaders/frag.spv
    COMMAND ${CMAKE_COMMAND} -E make_directory shaders
    COMMAND glslc -O ../shaders/shader.vert -o shaders/vert.spv
    COMMAND glslc -O ../shaders/shader.frag -o shaders/frag.spv
    DEPENDS ${SHADER_SOURCES}
    COMMENT "Compiling shaders"
)

# Add library dependencies
include(FindPkgConfig)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Create executable
add_executable(${PROJECT_NAME} ${vlkn_SOURCES})

# Add shader compilation as a dependency
add_custom_target(compile_shaders DEPENDS ${CMAKE_BINARY_DIR}/shaders/vert.spv ${CMAKE_BINARY_DIR}/shaders/frag.spv)
add_dependencies(${PROJECT_NAME} compile_shaders)

# Include directories
include_directories(${vlkn_INCLUDE_DIRS} vulkan glfw glm)

# Link libraries
target_link_libraries(${PROJECT_NAME} vulkan glfw glm::glm)

