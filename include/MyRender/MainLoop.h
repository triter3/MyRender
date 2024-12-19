#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include "MyRender/Scene.h"

namespace myrender
{

class MainLoop {
public:
	MainLoop() {}
    MainLoop(int fpsTarget) : mFpsTarget(fpsTarget) {}
	void start(Scene& scene);
	void setFpsTarget(int fps) { mFpsTarget = fps; }
private:
	int mFpsTarget = 60;
};

}

#endif