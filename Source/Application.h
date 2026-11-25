#pragma once

#include "Globals.h"
#include "Clock.h"

#include <vector>

class Module;

class Application
{
public:

	Application(int argc, wchar_t** argv, void* hWnd);
	~Application();

	bool         init();
	void         update();
	void         resize(unsigned width, unsigned height);
	bool         cleanUp();

    Clock*                      getClock() { return &clock; }

    // Skips the whole frame (update and render); used while the window is minimized.
    // Not the Play/Stop of the simulation, which is Clock::play()/stop() and keeps rendering.
    bool                        isPaused() const { return paused; }
    bool                        setPaused(bool p) { paused = p; return paused; }

private:
    std::vector<Module*> modules;

    Clock       clock;   // Not a Module: Application drives it explicitly. Built before the constructor body, so the clock covers the init of every module
    bool        paused = false;
};

extern Application* app;
