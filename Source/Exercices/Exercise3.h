#pragma once

#include "Module.h"

class ModuleD3D12;
class ModuleResources;
class DebugDrawPass;

/**
 * @brief Exercise 3: draws a 3D triangle whose vertices are given in local space and transformed by an MVP matrix.
 */
class Exercise3 : public Module
{
public:

    /**
     * @param d3d12 Provides the device and the frame's command list.
     * @param resources Used to create the vertex buffer.
     */
    Exercise3(ModuleD3D12* d3d12, ModuleResources* resources);

    /**
     * @brief Creates the vertex buffer (and its view), the root signature, the PSO and the debug draw pass.
     * @return false if any step fails (the failure is logged).
     */
    bool init() override;

    /**
     * @brief Binds the depth buffer, draws the triangle and then the debug grid and axes into the frame's command list.
     * Runs between ModuleD3D12::preRender() and postRender(), which handle the list reset,
     * the PRESENT <-> RENDER_TARGET barriers and the submission.
     */
    void render() override;

    /** @brief Releases the buffer, root signature, PSO and debug draw pass. */
    bool cleanUp() override;

private:
    bool createVertexBuffer();
    bool createRootSignature();
    bool createPSO();

private:
    ModuleD3D12* d3d12 = nullptr;
    ModuleResources* resources = nullptr;

    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pso;

    // TODO: debug draw should live in its own module (like ModuleEditor does for ImGui): it would own the
    // DebugDrawPass, every exercise would only emit dd:: calls in its render(), and the module would record
    // them once at the end of the frame with the camera's view/projection (which needs a camera module first).
    DebugDrawPass* debugDraw = nullptr;
};
