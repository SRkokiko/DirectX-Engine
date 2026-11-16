#pragma once

#include "Module.h"

class ModuleD3D12;

/**
 * @brief ExerciceImGui: clears the back buffer and shows the Dear ImGui demo window.
 * The ImGui frame itself is started and recorded by ModuleEditor.
 */
class ExerciceImGui : public Module
{
public:

    /** @param d3d12 Provides the frame's command list. */
    explicit ExerciceImGui(ModuleD3D12* d3d12);

    /** @brief Clears the back buffer and builds the demo window (drawn later by ModuleEditor). */
    void render() override;

private:
    ModuleD3D12* d3d12 = nullptr;
};
