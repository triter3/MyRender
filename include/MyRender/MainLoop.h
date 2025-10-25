#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include "MyRender/Scene.h"
#include <optional>
#include <functional>

namespace myrender
{

class MainLoop {
public:
	MainLoop() {}
    MainLoop(int fpsTarget) : mFpsTarget(fpsTarget) {}
	void start(Scene& scene);
	void setFpsTarget(int fps) { mFpsTarget = fps; }
	void requestFpsTest(std::function<void(double)> call)
	{
		mDoFPStest = true;
		mFPStestCallback = call;
	}

	void requestImageCapture(const std::string& imagePath, uint32_t imageHeight)
	{
		mImageCapturePath = imagePath;
		mCaptureImageHeight = imageHeight;
	}

	static MainLoop* getCurrent() { return mCurrentLoop; }
private:
	static MainLoop* mCurrentLoop;
	int mFpsTarget = 60;
	static constexpr bool mAllowFPStest = true;
	bool mDoFPStest = false;
	std::optional<std::function<void(double)>> mFPStestCallback;
	std::optional<std::string> mImageCapturePath;
	uint32_t mCaptureImageHeight = 1;
};

}

#endif