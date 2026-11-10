cmake_minimum_required(VERSION 3.16)
project(PerlinNoiseTerrainSimulator LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Tìm hoặc tải GLFW
find_package(glfw3 3.3 QUIET)
if (NOT glfw3_FOUND)
    message(STATUS "Không tìm thấy GLFW, tự động tải GLFW qua FetchContent...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# 2. Tìm hoặc tải GLM
find_package(glm QUIET)
if (NOT glm_FOUND)
    message(STATUS "Không tìm thấy GLM, tự động tải GLM qua FetchContent...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

# 3. Nguồn mã của dự án
set(SOURCES
    src/glad.c
    src/PerlinNoise.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Terrain.cpp
    src/Water.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
    src/main.cpp
)

add_executable(terrain_simulator ${SOURCES})

# Đảm bảo mã hóa UTF-8 tiếng Việt chuẩn xác trên MSVC (Visual Studio)
if (MSVC)
    target_compile_options(terrain_simulator PRIVATE "/utf-8")
endif()

# Thư mục include
target_include_directories(terrain_simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
)

# Liên kết thư viện
if (TARGET glfw)
    target_link_libraries(terrain_simulator PRIVATE glfw)
else()
    target_link_libraries(terrain_simulator PRIVATE glfw)
endif()

if (TARGET glm::glm)
    target_link_libraries(terrain_simulator PRIVATE glm::glm)
elseif(TARGET glm)
    target_link_libraries(terrain_simulator PRIVATE glm)
endif()

if (UNIX AND NOT APPLE)
    target_link_libraries(terrain_simulator PRIVATE GL dl pthread)
endif()

# Tự động sao chép thư mục shaders và assets (font tiếng Việt) vào thư mục build
add_custom_command(TARGET terrain_simulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:terrain_simulator>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:terrain_simulator>/assets
)
