#pragma once

#include "Globals.h"

#include <vector>

class Module;
class ModuleTime;

class Application
{
public:

	Application(int argc, wchar_t** argv, void* hWnd);
	~Application();

	bool         init();
	void         update();
	void         resize(unsigned width, unsigned height);
	bool         cleanUp();

    ModuleTime*                 getTime() const { return moduleTime; }

    // Skips the whole frame (update and render); used while the window is minimized.
    // Not the Play/Stop of the simulation, which is ModuleTime::play()/stop() and keeps rendering.
    bool                        isPaused() const { return paused; }
    bool                        setPaused(bool p) { paused = p; return paused; }

private:
    std::vector<Module*> modules;

    ModuleTime* moduleTime = nullptr;
    bool        paused = false;
};

extern Application* app;
