cmake_minimum_required(VERSION 3.16)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfig.txt")
    include("${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfig.txt")
endif()

# Set the project name and version
project(MyRender VERSION 0.1)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    add_compile_options(-fPIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs -std=c++17")
endif()


# Project options
option(MYRENDER_BUILD_EXAMPLES "Compile examples" ON)

# Specify the c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Get all project files
file(GLOB SOURCE_FILES src/*.cpp)
file(GLOB SHADERS_SOURCE_FILES src/shaders/*.cpp)
file(GLOB UTILS_SOURCE_FILES src/utils/*.cpp)

# Add libraries
add_library(${PROJECT_NAME} STATIC  ${SOURCE_FILES}
                                    ${SHADERS_SOURCE_FILES}
                                    ${UTILS_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC include/)
target_include_directories(${PROJECT_NAME} PRIVATE src/)

# Add shaders
# file(GLOB SHADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/src/render_engine/shaders 
#                        src/render_engine/shaders/*.frag 
#                        src/render_engine/shaders/*.vert
#                        src/render_engine/shaders/*.comp)

# foreach(SHADER IN LISTS SHADER_FILES)
#     add_custom_command(OUTPUT ${SHADER}
#             COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/src/render_engine/shaders/${SHADER} $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/${SHADER}
#             DEPENDS src/render_engine/shaders/${SHADER}
#         )
# endforeach()

# add_custom_target(copyShaders ALL SOURCES ${SHADER_FILES})

# Add dependencies
add_subdirectory(libs)

target_link_libraries(${PROJECT_NAME} PUBLIC glm slang glfw imgui imguizmo glad stb_image)
# add_dependencies(${PROJECT_NAME} copyShaders)

if(MYRENDER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()