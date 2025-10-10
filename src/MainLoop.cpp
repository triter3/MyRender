#include "MyRender/MainLoop.h"

#include <iostream>
#include <thread>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <ImGuizmo.h>
#include "MyRender/Window.h"
#include "MyRender/utils/Timer.h"

namespace myrender
{

MainLoop* MainLoop::mCurrentLoop = nullptr;

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
		float dt = deltaTimer.getElapsedSeconds();
		deltaTimer.start();
		scene.update(deltaTimer.getElapsedSeconds());

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

		if(mAllowFPStest && mDoFPStest) // Do a frame rate test
		{
			std::cout << "Start test" << std::endl;
			Timer timer;
			double millisPerFrame = 0.0;
			constexpr uint32_t numTries = 10;
			constexpr uint32_t numItrs = 300;
			for(uint32_t j=0; j < numTries; j++)
			{
				timer.start();
				for(uint32_t i=0; i < numItrs; i++)
				{
					scene.draw();
					window.swapBuffers();
				}
				glFinish();
				millisPerFrame += static_cast<double>(timer.getElapsedMilliseconds()) / static_cast<double>(numItrs * numTries);
			}
			std::cout << "Ms per frame: " << millisPerFrame << std::endl;
			if(mFPStestCallback) mFPStestCallback.value()(millisPerFrame);
			mFPStestCallback = std::nullopt;
			mDoFPStest = false;
		}
	}

	Window::setCurrentWindow(nullptr);
}

}