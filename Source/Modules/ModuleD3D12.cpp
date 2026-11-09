#include "Globals.h"
#include "ModuleD3D12.h"

ModuleD3D12::ModuleD3D12(HWND hWnd) : hWnd(hWnd)
{
}

bool ModuleD3D12::init()
{
#if defined(_DEBUG)
    // Must be enabled before the device is created
    ComPtr<ID3D12Debug> debugInterface;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
    {
        debugInterface->EnableDebugLayer();
    }
    else
    {
        LOG("D3D12 debug layer unavailable (is the Graphics Tools optional feature installed?)");
    }
#endif

#if defined(_DEBUG)
    UINT factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
    UINT factoryFlags = 0;
#endif

    if (FAILED(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory))))
    {
        LOG("Failed to create DXGI factory");
        return false;
    }

    if (FAILED(factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))))
    {
        LOG("Failed to find a GPU adapter");
        return false;
    }

    if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
    {
        LOG("Failed to create D3D12 device (feature level 12_0 / ID3D12Device5 required)");
        return false;
    }

#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(device.As(&infoQueue)))
    {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))))
    {
        LOG("Failed to create command queue");
        return false;
    }

    return createSwapChain() && createRenderTargets();
}

bool ModuleD3D12::createSwapChain()
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = clientRect.right - clientRect.left;
    desc.Height = clientRect.bottom - clientRect.top;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc = { 1, 0 };
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = BACK_BUFFER_COUNT;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1)) ||
        FAILED(swapChain1.As(&swapChain)))
    {
        LOG("Failed to create swap chain");
        return false;
    }

    return true;
}

bool ModuleD3D12::createRenderTargets()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NumDescriptors = BACK_BUFFER_COUNT;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap))))
    {
        LOG("Failed to create RTV descriptor heap");
        return false;
    }

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
    {
        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]))))
        {
            LOG("Failed to get swap chain buffer %u", i);
            return false;
        }

        device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

    return true;
}

bool ModuleD3D12::cleanUp()
{
    // Release in reverse order of creation (views/buffers before the swap chain)
    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
        backBuffers[i].Reset();
    rtvHeap.Reset();
    swapChain.Reset();
    commandQueue.Reset();

    device.Reset();
    adapter.Reset();
    factory.Reset();

    return true;
}
