#pragma once

#include "Module.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public Module
{
public:

    explicit ModuleD3D12(HWND hWnd);

    bool init() override;
    bool cleanUp() override;

    ID3D12Device5* getDevice() const { return device.Get(); }

private:
    bool createSwapChain();
    bool createRenderTargets();

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
};
