include(FetchContent)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

# glm
FetchContent_Declare(glm_lib
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 89e52e327d7a3ae61eb402850ba36ac4dd111987 # 0.9.8
)

FetchContent_GetProperties(glm_lib)
if(NOT glm_lib_POPULATED)
	FetchContent_Populate(glm_lib)
	add_subdirectory(${glm_lib_SOURCE_DIR} ${glm_lib_BINARY_DIR})
endif()

# stb
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb)

# glfw
FetchContent_Declare(glfw_lib
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 7d5a16ce714f0b5f4efa3262de22e4d948851525 # 3.3.6
)

FetchContent_GetProperties(glfw_lib)
if(NOT glfw_lib_POPULATED)
	FetchContent_Populate(glfw_lib)
	add_subdirectory(${glfw_lib_SOURCE_DIR} ${glfw_lib_BINARY_DIR})
endif()

# glad
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# imgui
FetchContent_Declare(imgui_lib
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG docking
)

FetchContent_GetProperties(imgui_lib)
if(NOT imgui_lib_POPULATED)
	FetchContent_Populate(imgui_lib)
	file(GLOB imgui_SOURCE_FILES ${imgui_lib_SOURCE_DIR}/*.cpp)
	file(GLOB imgui_HEADER_FILES ${imgui_lib_SOURCE_DIR}/*.h)
	add_library(imgui STATIC ${imgui_SOURCE_FILES} 
							${imgui_lib_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
							${imgui_lib_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
							${imgui_lib_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
							${imgui_HEADER_FILES}
							${imgui_lib_SOURCE_DIR}/backends/imgui_impl_glfw.h
							${imgui_lib_SOURCE_DIR}/backends/imgui_impl_opengl3.h
							${imgui_lib_SOURCE_DIR}/misc/cpp/imgui_stdlib.h)
	
	target_include_directories(imgui PUBLIC ${imgui_lib_SOURCE_DIR}
									PUBLIC ${imgui_lib_SOURCE_DIR}/backends)
	target_link_libraries(imgui PRIVATE glfw
								PRIVATE glad)
endif()

# imguizmo
FetchContent_Declare(imguizmo_lib
	GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
	GIT_TAG master
)

FetchContent_GetProperties(imguizmo_lib)
if(NOT imguizmo_lib_POPULATED)
	FetchContent_Populate(imguizmo_lib)
	file(GLOB imguizmo_SOURCE_FILES ${imguizmo_lib_SOURCE_DIR}/*.cpp)
	file(GLOB imguizmo_HEADER_FILES ${imguizmo_lib_SOURCE_DIR}/*.h)
	add_library(imguizmo STATIC ${imguizmo_SOURCE_FILES} 
							${imguizmo_HEADER_FILES})
	
	target_include_directories(imguizmo PUBLIC ${imguizmo_lib_SOURCE_DIR})
	target_link_libraries(imguizmo  PRIVATE glfw
									PRIVATE glad
									PRIVATE imgui)
endif()

# Slang
FetchContent_Declare(slang_lib
	URL https://github.com/shader-slang/slang/releases/download/v2024.14.5/slang-2024.14.5-windows-x86_64.zip
)

if(NOT slang_lib_POPULATED)
	FetchContent_Populate(slang_lib)
	add_library(slang INTERFACE)
	target_link_libraries(slang INTERFACE ${slang_lib_SOURCE_DIR}/lib/slang.lib ${slang_lib_SOURCE_DIR}/lib/gfx.lib ${slang_lib_SOURCE_DIR}/lib/slang-rt.lib)
	target_include_directories(slang INTERFACE ${slang_lib_SOURCE_DIR}/include)
endif()