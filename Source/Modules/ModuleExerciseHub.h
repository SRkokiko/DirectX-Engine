#pragma once

#include "Module.h"

#include <functional>
#include <vector>

class ModuleD3D12;
class ModuleResources;
class ModuleDebugDraw;

/**
 * @brief Hosts every exercise and view, and lets you enable/disable each one from the "Exercises" and "Views" menus
 * in the main menu bar. Exercises draw into the back buffer; views (e.g. the ImGui demo) only build ImGui windows.
 *
 * Clears the back buffer once per frame, then renders the enabled exercises in registration order. They all share
 * that one back buffer, so later exercises paint over earlier ones. It is bound without a depth buffer: an exercise
 * that needs depth re-binds the render target with ModuleD3D12::getDsvHandle() and clears it itself (see Exercise3).
 * Exercises are created and destroyed between frames, in update(), never while the frame's command list is open.
 *
 * Must be registered after ModuleD3D12 and ModuleResources and before ModuleEditor, which records the UI last.
 *
 * TODO: render exercises into one shared offscreen texture shown with ImGui::Image in a single window.
 * Needs a render target + SRV (descriptor allocator) and resize handling. The hub's interface shouldn't change.
 * The hub would then also own the depth buffer (sized to that texture, not the window), bind and clear it for every
 * exercise, and the one in ModuleD3D12 would move here. Exercise3 could then drop its own OMSetRenderTargets and
 * ClearDepthStencilView, and ModuleDebugDraw would no longer depend on the exercise having bound the depth buffer.
 * Every geometry exercise would need a D32_FLOAT PSO (Exercise2 doesn't have one yet).
 */
class ModuleExerciseHub : public Module
{
public:

    /**
     * @param d3d12 Provides the device and the frame's command list.
     * @param resources Handed to the exercises that create buffers.
     * @param debugDraw Handed to the exercises that draw debug primitives.
     */
    ModuleExerciseHub(ModuleD3D12* d3d12, ModuleResources* resources, ModuleDebugDraw* debugDraw);
    ~ModuleExerciseHub() override;

    /** @brief Registers the exercises and views and enables the default ones. */
    bool init() override;

    /** @brief Applies pending enable/disable changes, then updates the enabled exercises and views. */
    void update(float dt) override;

    void preRender() override;

    /** @brief Clears the back buffer, builds the menu bar, then renders the enabled exercises and views. */
    void render() override;

    void postRender() override;
    void resize(unsigned width, unsigned height) override;

    /** @brief Waits for the GPU and destroys every active exercise and view. */
    bool cleanUp() override;

private:
    struct Entry
    {
        const char* name;
        std::function<Module*()> create;
        bool enabled;
        Module* instance;
    };

    static void add(std::vector<Entry>& list, const char* name, std::function<Module*()> create, bool enabled = false);
    void sync(std::vector<Entry>& list);
    static void drawMenu(const char* title, std::vector<Entry>& list);
    void destroyAll(std::vector<Entry>& list);
    void destroy(Entry& entry);
    void forEachInstance(const std::function<void(Module&)>& fn);

private:
    ModuleD3D12* d3d12 = nullptr;
    ModuleResources* resources = nullptr;
    ModuleDebugDraw* debugDraw = nullptr;

    std::vector<Entry> exercises;
    std::vector<Entry> views;
};
