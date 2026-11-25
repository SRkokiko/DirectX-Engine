#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "Modules/ModuleD3D12.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleExerciseHub.h"
#include "Modules/ModuleDebugDraw.h"
#include "Modules/ModuleEditor.h"
#include "Modules/ModuleTime.h"


Application::Application(int argc, wchar_t** argv, void* hWnd)
{
    // First, so the engine clock covers the init of every other module
    moduleTime = new ModuleTime();
    modules.push_back(moduleTime);

    ModuleD3D12* d3d12 = new ModuleD3D12((HWND)hWnd);

    modules.push_back(d3d12);
    modules.push_back(new ModuleInput((HWND)hWnd));
    ModuleResources* resources = new ModuleResources(d3d12);
    modules.push_back(resources);

    ModuleDebugDraw* debugDraw = new ModuleDebugDraw(d3d12);

    // Hosts every exercise; they are toggled from its "Exercises" menu
    modules.push_back(new ModuleExerciseHub(d3d12, resources, debugDraw));

    // After the exercises that emit dd:: primitives and before the UI: records them once
    modules.push_back(debugDraw);

    // Last: records the UI on top of everything the other modules drew
    modules.push_back(new ModuleEditor(d3d12, (HWND)hWnd));
}

Application::~Application()
{
    cleanUp();

	for(auto it = modules.rbegin(); it != modules.rend(); ++it)
    {
        delete *it;
    }
}
 
bool Application::init()
{
	bool ret = true;

    Timer initTimer;
    initTimer.start();

	for(auto it = modules.begin(); it != modules.end() && ret; ++it)
		ret = (*it)->init();

    moduleTime->markInitDone(initTimer.stop());

	return ret;
}

void Application::update()
{
    moduleTime->beginFrame();

    if (!app->paused)
    {
        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->update();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->preRender();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->render();

        for (auto it = modules.begin(); it != modules.end(); ++it)
            (*it)->postRender();
    }
}

void Application::resize(unsigned width, unsigned height)
{
    if (width == 0 || height == 0)
        return;

    for (Module* module : modules)
        module->resize(width, height);
}

bool Application::cleanUp()
{
	bool ret = true;

	for(auto it = modules.rbegin(); it != modules.rend() && ret; ++it)
		ret = (*it)->cleanUp();

	return ret;
}
