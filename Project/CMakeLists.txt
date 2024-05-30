# Settings
# --------
set(PROJECT_NAME "Project")

# Includes
# --------
include(FetchContent)

# Vulkan library
# --------------
find_package(Vulkan REQUIRED)

# GLFW library
# ------------
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.4"
    GIT_SHALLOW TRUE
)

set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)

FetchContent_MakeAvailable(glfw)

# GLM library
# -----------
FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG "1.0.1"
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    tinyobj
    GIT_REPOSITORY "https://github.com/tinyobjloader/tinyobjloader.git"
    GIT_TAG "release"
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    stb
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG "master"
    GIT_SHALLOW TRUE
    UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(glm tinyobj stb)

# Find source files
# -----------------
file(GLOB_RECURSE SOURCES
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    CONFIGURE_DEPENDS
    "*.h"
    "*.cpp"
)

# Add project library
# -------------------
add_executable(${PROJECT_NAME} ${SOURCES} 
"MachienWindow.h" "MachienWindow.cpp" "MachienApp.h" "MachienApp.cpp" "MachienPipeline.h" "MachienPipeline.cpp" "MachienDevice.h" "MachienDevice.cpp" "MachienModel.h" "MachienModel.cpp" "MachienObject.h" "MachienObject.cpp" "MachienRenderer.h" "MachienRenderer.cpp" "MachienRenderSystem.h" "MachienRenderSystem.cpp" "MachienCamera.h" "MachienCamera.cpp" "MachienKeyboard.h" "MachienKeyboard.cpp" "MachienUtils.h" "MachienBuffer.h" "MachienBuffer.cpp" "MachienObject2D.h" "MachienObject2D.cpp" "MachienFrameInfo.h" "MachienDescriptors.h" "MachienDescriptors.cpp" "MachienTexture.h" "MachienTexture.cpp")

# Configure project
# -----------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
    COMPILE_WARNING_AS_ERROR TRUE
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${stb_SOURCE_DIR}"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        glm::glm
        glfw
        tinyobjloader
)
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory 
"${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_CURRENT_BINARY_DIR}/resources" )
#set(MODEL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

file(GLOB_RECURSE GLSL_SOURCE_FILES CONFIGURE_DEPENDS
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    SOURCES ${GLSL_SOURCE_FILES}
)

add_dependencies(
   ${PROJECT_NAME} Shaders
)
