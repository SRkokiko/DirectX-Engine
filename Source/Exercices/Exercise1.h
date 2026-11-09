#pragma once

#include "Module.h"

class ModuleD3D12;

/**
 * @brief Exercise 1: clears the back buffer to a solid color every frame.
 */
class Exercise1 : public Module
{
public:

    /**
     * @param d3d12 Module that owns the device, swap chain and command list we record into.
     */
    explicit Exercise1(ModuleD3D12* d3d12);

    /**
     * @brief Records a clear of the current back buffer into the frame's command list.
     * Runs between ModuleD3D12::preRender() and postRender(), so the buffer is in RENDER_TARGET state.
     */
    void render() override;

private:
    ModuleD3D12* d3d12 = nullptr;
};
