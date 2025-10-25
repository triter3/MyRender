#include "MyRender/MainLoop.h"

#include <iostream>
#include <thread>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <ImGuizmo.h>
#include "MyRender/Window.h"
#include "MyRender/utils/Timer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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

	// window.disableVerticalSync()
	
	mCurrentLoop = this;

	scene.start();

	while (!window.shouldClose()) {
		fpsTimer.start();

		if (mFpsTarget > 0) glfwPollEvents();
		else glfwWaitEvents();

		glm::vec2 windowSizeBeforeCapture;
		glm::ivec2 captureSize;
		float captureScale;
		bool doCapture = false;
		uint32_t captureFboId = 0;
		uint32_t captureRboId;
		uint32_t captureTextureId;
		if(mImageCapturePath)
		{
			windowSizeBeforeCapture = window.getWindowSize();
			float aratio = static_cast<float>(window.getWindowSize().x) / static_cast<float>(window.getWindowSize().y);
			captureSize = glm::ivec2(static_cast<float>(mCaptureImageHeight) * aratio, mCaptureImageHeight);
			captureScale = static_cast<float>(captureSize.y) / static_cast<float>(window.getWindowSize().y);
			scene.resize(captureSize);
			// Create frame buffer
			glGenFramebuffers(1, &captureFboId);
        	glBindFramebuffer(GL_FRAMEBUFFER, captureFboId);

			// Create texture
			glGenTextures(1, &captureTextureId);
			glBindTexture(GL_TEXTURE_2D, captureTextureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, captureSize.x, captureSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, captureTextureId, 0);

			// Create render buffer
            glGenRenderbuffers(1, &captureRboId);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRboId);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, captureSize.x, captureSize.y);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, captureRboId);

			glBindTexture(GL_TEXTURE_2D, 0);
        	glBindRenderbuffer(GL_RENDERBUFFER, 0);
			doCapture = true;
		} 
		else if(window.needResize())
		{
			scene.resize(window.getWindowSize());
		}

		window.update();

		// Imgui updates 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		if(doCapture)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(captureSize.x, captureSize.y);
			io.DisplayFramebufferScale = ImVec2(1.0, 1.0);
		}
		ImGui::NewFrame();
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();

		glBindFramebuffer(GL_FRAMEBUFFER, captureFboId);

		// Scene update
		float dt = deltaTimer.getElapsedSeconds();
		deltaTimer.start();
		scene.update(dt);

		// Scene draw
		scene.draw();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if(doCapture)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, captureFboId);
			const size_t buffSizeInBytes = 3 * captureSize.x * captureSize.y; // Is RGB
			std::vector<uint8_t> imageArray(buffSizeInBytes);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, captureSize.x, captureSize.y, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<void*>(imageArray.data()));
			glPixelStorei(GL_PACK_ALIGNMENT, 4);

			// Flip the image
			std::vector<uint8_t> imageArrayFlip(buffSizeInBytes);
			const size_t rowSize = 3 * captureSize.x;
			for(size_t y=0; y < captureSize.y; y++)
			{
				std::memcpy(reinterpret_cast<void*>(imageArrayFlip.data() + y * rowSize),
							reinterpret_cast<void*>(imageArray.data() + (captureSize.y - 1 - y) * rowSize), rowSize);
			}

			if(!stbi_write_png(mImageCapturePath.value().c_str(), captureSize.x, captureSize.y, 3, 
						       reinterpret_cast<void*>(imageArrayFlip.data()), rowSize))
			{
				std::cout << "Capture failed" << std::endl;
			}
			else
			{
				std::cout << "Image captured" << std::endl;
			}

			scene.resize(window.getWindowSize());

			glDeleteTextures(1, &captureTextureId);
			glDeleteRenderbuffers(1, &captureRboId);
			glDeleteFramebuffers(1, &captureFboId);
			mImageCapturePath = std::nullopt;
		}
		
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