#include "Globals.h"
#include "ModuleD3D12.h"

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

    return true;
}

bool ModuleD3D12::cleanUp()
{
    device.Reset();
    adapter.Reset();
    factory.Reset();

    return true;
}
