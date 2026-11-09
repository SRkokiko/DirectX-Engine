#pragma once

#include "Module.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public Module
{
public:

    /**
     * @param hWnd Window the swap chain presents to; its client area sets the back buffer size.
     */
    explicit ModuleD3D12(HWND hWnd);

    /**
     * @brief Creates the DXGI factory, adapter, device and command queue, then the swap chain, its
     * render target views and the per-frame command/fence resources.
     * @return false if any step fails (the failure is logged); the engine then shuts down.
     */
    bool init() override;

    /**
     * @brief Opens the frame: waits until the GPU has finished the last frame that used the current
     * back buffer, resets its command allocator and the command list, and records the
     * PRESENT -> RENDER_TARGET barrier. Other modules record their draw commands in render().
     */
    void preRender() override;

    /**
     * @brief Closes the frame: records the RENDER_TARGET -> PRESENT barrier, closes and submits the
     * command list, presents the back buffer and signals the fence so a later preRender()
     * knows when this frame's allocator can be reused.
     */
    void postRender() override;

    /**
     * @brief Releases every D3D12/DXGI object, after waiting for the GPU to finish pending work.
     * Safe to call after a partially failed init().
     */
    bool cleanUp() override;

    /** @brief The D3D12 device.
     *  @return Owned by this module; valid between init() and cleanUp(). */
    ID3D12Device5* getDevice() const { return device.Get(); }

private:
    bool createSwapChain();
    bool createRenderTargets();
    bool createFrameResources();
    void waitForFence(UINT64 value);
    void flush();

private:
    HWND hWnd = nullptr;

    ComPtr<IDXGIFactory6> factory;
    ComPtr<IDXGIAdapter4> adapter;
    ComPtr<ID3D12Device5> device;

    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain4> swapChain;

    // One RTV descriptor per back buffer: descriptor i <-> back buffer i
    static constexpr UINT BACK_BUFFER_COUNT = 2;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12Resource> backBuffers[BACK_BUFFER_COUNT];
    UINT rtvDescriptorSize = 0;

    // One allocator per back buffer
    ComPtr<ID3D12CommandAllocator> commandAllocators[BACK_BUFFER_COUNT];
    ComPtr<ID3D12GraphicsCommandList> commandList;

    // The queue signals fenceCounter after each frame; frameFenceValues[i] is the value that
    // tells us the GPU finished the last frame that used back buffer i
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    UINT64 fenceCounter = 0;
    UINT64 frameFenceValues[BACK_BUFFER_COUNT] = {};
};
