#include "MyRender/MainLoop.h"

#include <thread>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <ImGuizmo.h>
#include "MyRender/Window.h"
#include "MyRender/utils/Timer.h"

namespace myrender
{

void MainLoop::start(Scene& scene)
{
    Window window;
    window.start();

    Timer deltaTimer;
	deltaTimer.start();

	Timer fpsTimer;
	fpsTimer.start();

	Window::setCurrentWindow(&window);

	// window.disableVerticalSync();

	scene.start();

	while (!window.shouldClose()) {
		fpsTimer.start();

		if (mFpsTarget > 0) glfwPollEvents();
		else glfwWaitEvents();

		if (window.needResize()) {
			scene.resize(window.getWindowSize());
		}
		window.update();

		// Imgui updates 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();

		// Scene update
		scene.update(deltaTimer.getElapsedSeconds());
		deltaTimer.start();

		// Scene draw
		scene.draw();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.swapBuffers();

		int millisecondsPerFrame = 1000 / mFpsTarget;
		int aux = fpsTimer.getElapsedMilliseconds();
		if (millisecondsPerFrame > aux) {
			std::this_thread::sleep_for(std::chrono::milliseconds(millisecondsPerFrame - aux));
		}
	}

	Window::setCurrentWindow(nullptr);
}

}