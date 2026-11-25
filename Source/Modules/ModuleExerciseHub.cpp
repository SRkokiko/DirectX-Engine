#include "Globals.h"
#include "ModuleExerciseHub.h"

#include "ModuleD3D12.h"
#include "ModuleResources.h"
#include "Exercices/Exercise1.h"
#include "Exercices/Exercise2.h"
#include "Exercices/Exercise3.h"
#include "Views/View.h"
#include "Views/ViewAbout.h"
#include "Views/ViewImGuiDemo.h"

#include "imgui.h"

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

ModuleExerciseHub::ModuleExerciseHub(ModuleD3D12* d3d12, ModuleResources* resources, ModuleDebugDraw* debugDraw) : d3d12(d3d12), resources(resources), debugDraw(debugDraw)
{
}

ModuleExerciseHub::~ModuleExerciseHub()
{
    cleanUp();
}

bool ModuleExerciseHub::init()
{
    // Order matters: it is the draw order, so later exercises paint over earlier ones.
    // Register exercises that clear the back buffer first, or they erase the ones before them
    add(exercises, "Exercise 1: Clear", [this]() -> Module* { return new Exercise1(d3d12); });
    add(exercises, "Exercise 2: Triangle", [this]() -> Module* { return new Exercise2(d3d12, resources); });
    add(exercises, "Exercise 3: 3D Triangle", [this]() -> Module* { return new Exercise3(d3d12, resources, debugDraw); });

    add(views, "ImGui demo", []() -> Module* { return new ViewImGuiDemo(); });
    add(views, "About", []() -> Module* { return new ViewAbout(); });
    return true;
}

void ModuleExerciseHub::update(float dt)
{
    sync(exercises);
    sync(views);

    forEachInstance([dt](Module& module) { module.update(dt); });
}

void ModuleExerciseHub::preRender()
{
    forEachInstance([](Module& module) { module.preRender(); });
}

void ModuleExerciseHub::render()
{
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getCurrentRtvHandle();
    const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

    // Between ModuleEditor's NewFrame (preRender) and Render (its render, registered last)
    if (ImGui::BeginMainMenuBar())
    {
        drawMenu("Exercises", exercises);
        drawMenu("Views", views);
        ImGui::EndMainMenuBar();
    }

    forEachInstance([](Module& module) { module.render(); });

    // A view closed with its X button: clear the flag so sync() destroys it and the menu check mark goes away
    for (Entry& entry : views)
    {
        const View* view = dynamic_cast<const View*>(entry.instance);
        if (view && !view->isOpen())
            entry.enabled = false;
    }
}

void ModuleExerciseHub::postRender()
{
    forEachInstance([](Module& module) { module.postRender(); });
}

void ModuleExerciseHub::resize(unsigned width, unsigned height)
{
    forEachInstance([=](Module& module) { module.resize(width, height); });
}

bool ModuleExerciseHub::cleanUp()
{
    destroyAll(exercises);
    destroyAll(views);
    return true;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void ModuleExerciseHub::add(std::vector<Entry>& list, const char* name, std::function<Module*()> create, bool enabled)
{
    list.push_back({ name, std::move(create), enabled, nullptr });
}

// Reconciles each entry's `enabled` flag (set by the menu) with its `instance`
void ModuleExerciseHub::sync(std::vector<Entry>& list)
{
    for (Entry& entry : list)
    {
        if (entry.enabled && !entry.instance)
        {
            entry.instance = entry.create();
            if (!entry.instance->init())
            {
                LOG("Failed to init '%s'", entry.name);
                destroy(entry);
                entry.enabled = false;
            }
        }
        else if (!entry.enabled && entry.instance)
        {
            destroy(entry);
        }
    }
}

void ModuleExerciseHub::drawMenu(const char* title, std::vector<Entry>& list)
{
    if (ImGui::BeginMenu(title))
    {
        // MenuItem flips entry.enabled itself when clicked; sync() acts on it next update()
        for (Entry& entry : list)
            ImGui::MenuItem(entry.name, nullptr, &entry.enabled);
        ImGui::EndMenu();
    }
}

void ModuleExerciseHub::destroyAll(std::vector<Entry>& list)
{
    for (Entry& entry : list)
        destroy(entry);
}

void ModuleExerciseHub::destroy(Entry& entry)
{
    if (!entry.instance)
        return;

    // The GPU may still be using the instance's PSO/buffers (frames in flight)
    d3d12->flush();
    entry.instance->cleanUp();
    delete entry.instance;
    entry.instance = nullptr;
}

void ModuleExerciseHub::forEachInstance(const std::function<void(Module&)>& fn)
{
    // Exercises first (they draw into the back buffer), then the views on top
    for (Entry& entry : exercises)
        if (entry.instance)
            fn(*entry.instance);

    for (Entry& entry : views)
        if (entry.instance)
            fn(*entry.instance);
}
