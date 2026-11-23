#pragma once

#include "Module.h"

class ModuleD3D12;
class ModuleResources;

/**
 * @brief Exercise 3: draws a 2D triangle whose vertices are given directly in clip space.
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
     * @brief Creates the vertex buffer (and its view), the root signature and the PSO.
     * @return false if any step fails (the failure is logged).
     */
    bool init() override;

    /**
     * @brief Clears the back buffer and draws the triangle into the frame's command list.
     * Runs between ModuleD3D12::preRender() and postRender(), which handle the list reset,
     * the PRESENT <-> RENDER_TARGET barriers and the submission.
     */
    void render() override;

    /** @brief Releases the buffer, root signature and PSO. */
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
};
