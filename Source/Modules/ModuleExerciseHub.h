#pragma once

#include "Module.h"

#include <functional>
#include <vector>

class ModuleD3D12;
class ModuleResources;

/**
 * @brief Hosts every exercise and view, and lets you enable/disable each one from the "Exercises" and "Views" menus
 * in the main menu bar. Exercises draw into the back buffer; views (e.g. the ImGui demo) only build ImGui windows.
 *
 * Clears the back buffer once per frame, then renders the enabled exercises in registration order. They all share
 * that one back buffer (no depth), so later exercises paint over earlier ones.
 * Exercises are created and destroyed between frames, in update(), never while the frame's command list is open.
 *
 * Must be registered after ModuleD3D12 and ModuleResources and before ModuleEditor, which records the UI last.
 *
 * TODO: render exercises into one shared offscreen texture shown with ImGui::Image in a single window.
 * Needs a render target + SRV (descriptor allocator) and resize handling. The hub's interface shouldn't change.
 */
class ModuleExerciseHub : public Module
{
public:

    /**
     * @param d3d12 Provides the device and the frame's command list.
     * @param resources Handed to the exercises that create buffers.
     */
    ModuleExerciseHub(ModuleD3D12* d3d12, ModuleResources* resources);
    ~ModuleExerciseHub() override;

    /** @brief Registers the exercises and views and enables the default ones. */
    bool init() override;

    /** @brief Applies pending enable/disable changes, then updates the enabled exercises and views. */
    void update() override;

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

    std::vector<Entry> exercises;
    std::vector<Entry> views;
};
