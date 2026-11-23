#pragma once

#include "Module.h"

class ModuleD3D12;
class DebugDrawPass;

/**
 * @brief Owns the DebugDrawPass and records everything emitted with the dd:: functions during the frame.
 *
 * Any module can call dd::xzSquareGrid, dd::axisTriad, ... from its render(); nothing is drawn until this
 * module's render() records them once, with the camera given through setCamera().
 *
 * Must be registered after ModuleD3D12 and ModuleExerciseHub (whose exercises emit the primitives) and before
 * ModuleEditor, so the debug primitives end up under the UI.
 *
 * The primitives are drawn into whatever render target is bound when render() runs, and they use the depth
 * test, so the module that emits them must have left a D32_FLOAT depth buffer bound (ModuleD3D12::getDsvHandle()).
 *
 * TODO: replace setCamera() with a camera module once there is one.
 */
class ModuleDebugDraw : public Module
{
public:

    /**
     * @param d3d12 Provides the device, the command queue and the frame's command list.
     */
    explicit ModuleDebugDraw(ModuleD3D12* d3d12);
    ~ModuleDebugDraw() override;

    /** @brief Creates the debug draw pass. */
    bool init() override;

    /** @brief Records the primitives emitted this frame. Does nothing if setCamera() wasn't called this frame. */
    void render() override;

    /** @brief Waits for the GPU and destroys the debug draw pass. */
    bool cleanUp() override;

    /**
     * @brief Camera used to draw this frame's primitives.
     * Valid for one frame: it has to be called again every frame, from a module's render().
     */
    void setCamera(const Matrix& view, const Matrix& proj);

private:
    ModuleD3D12* d3d12 = nullptr;
    DebugDrawPass* debugDrawPass = nullptr;

    Matrix view;
    Matrix proj;
    bool hasCamera = false;
};
