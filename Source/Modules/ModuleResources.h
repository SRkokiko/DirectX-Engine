#pragma once

#include "Module.h"

class ModuleD3D12;

/**
 * @brief Creates GPU buffers (vertex/index data) and uploads CPU data into them.
 *
 * Uses its own command allocator and list, so it can be used at any time without touching the
 * frame's command list owned by ModuleD3D12.
 */
class ModuleResources : public Module
{
public:

    /**
     * @param d3d12 Provides the device, command queue and flush(); must outlive this module.
     */
    explicit ModuleResources(ModuleD3D12* d3d12);

    /** @brief Creates the command allocator and list used for upload copies. */
    bool init() override;

    /** @brief Releases the command allocator and list. */
    bool cleanUp() override;

    /**
     * @brief Creates a buffer in an UPLOAD heap and fills it with the given data.
     * @param data Source bytes, copied immediately; may be null to leave the buffer uninitialized.
     * @param size Size in bytes.
     * @return The buffer, or null on failure (logged).
     */
    ComPtr<ID3D12Resource> createUploadBuffer(const void* data, size_t size);

    /**
     * @brief Creates a GPU-only (DEFAULT heap) buffer and copies the data into it through a
     * temporary upload buffer. Blocks until the copy has finished on the GPU.
     * @param finalState State the buffer is left in, e.g. D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
     * or D3D12_RESOURCE_STATE_INDEX_BUFFER.
     * @return The buffer, or null on failure (logged).
     */
    ComPtr<ID3D12Resource> createDefaultBuffer(const void* data, size_t size,
        D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_COMMON);

private:
    ModuleD3D12* d3d12 = nullptr;

    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12GraphicsCommandList> commandList;
};
