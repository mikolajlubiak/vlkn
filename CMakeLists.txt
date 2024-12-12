cmake_minimum_required(VERSION 3.5)
project(vlkn)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CheckCXXCompilerFlag)
include(FindPkgConfig)

# Enable flag if supported
function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

set(ImGui_DIR "cmake-imgui/build/dist/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH ${ImGui_DIR})

# Gather source and header files
file(GLOB_RECURSE vlkn_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE vlkn_HEADERS CONFIGURE_DEPENDS "src/*.hpp")
file(GLOB_RECURSE vlkn_MODELS CONFIGURE_DEPENDS "models/*")
file(GLOB_RECURSE vlkn_TEXTURES CONFIGURE_DEPENDS "textures/*")
file(GLOB_RECURSE vlkn_SHADERS "shaders/*.frag" "shaders/*.vert")

# Set release build flags
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} - -Ofast -march=native -mtune=native -DNDEBUG")

# Add library dependencies
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(ImGui CONFIG REQUIRED)

# Copy models to build directory
foreach (_model ${vlkn_MODELS})
	file(RELATIVE_PATH _relative_path ${CMAKE_SOURCE_DIR} ${_model})
	configure_file(${_model} ${CMAKE_BINARY_DIR}/${_relative_path} COPYONLY)
endforeach()

# Copy textures to build directory
foreach (_texture ${vlkn_TEXTURES})
	file(RELATIVE_PATH _relative_path ${CMAKE_SOURCE_DIR} ${_texture})
	configure_file(${_texture} ${CMAKE_BINARY_DIR}/${_relative_path} COPYONLY)
endforeach()

# Compile shaders
foreach(_shader ${vlkn_SHADERS})
  get_filename_component(_file_name ${_shader} NAME)
  set(_spirv ${CMAKE_BINARY_DIR}/shaders/${_file_name}.spv)

  add_custom_command(
    OUTPUT ${_spirv}
    COMMAND ${CMAKE_COMMAND} -E make_directory shaders
    COMMAND glslangValidator -V ${_shader} -o ${_spirv}
    DEPENDS ${_shader}
  )

  list(APPEND vlkn_SPIRV ${_spirv})
endforeach()

# Enable extra warnings
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")

# Create executable
add_executable(${PROJECT_NAME} ${vlkn_SOURCES})

# Add shader compilation as a dependency
add_custom_target(compile_shaders DEPENDS ${vlkn_SPIRV})
add_dependencies(${PROJECT_NAME} compile_shaders)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE src include vulkan glfw glm::glm)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE vulkan glfw glm::glm ImGui::imgui)
