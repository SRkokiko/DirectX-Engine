#include "Globals.h"
#include "ModuleD3D12.h"

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------


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

    if (!createSwapChain())
        return false;

    if (!createRenderTargets())
        return false;

    if (!createDepthStencil())
        return false;

    return createFrameResources();
}

void ModuleD3D12::resize(UINT width, UINT height)
{
    // Not initialized yet (WM_SIZE can arrive during window creation)
    if (width == 0 || height == 0 || !swapChain)
        return;

    DXGI_SWAP_CHAIN_DESC1 desc;
    swapChain->GetDesc1(&desc);
    if (desc.Width == width && desc.Height == height)
        return;

    // The GPU must be done with the back buffers before releasing them
    flush();

    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
        backBuffers[i].Reset();
    depthStencilBuffer.Reset();

    if (FAILED(swapChain->ResizeBuffers(BACK_BUFFER_COUNT, width, height, DXGI_FORMAT_UNKNOWN, 0)))
    {
        LOG("Failed to resize swap chain to %ux%u", width, height);
        return;
    }

    // Same heap and slots, new buffers: just rewrite the descriptors
    createBackBufferViews();

    // The depth buffer must match the new back buffer size
    createDepthBuffer();
}

void ModuleD3D12::preRender()
{
    UINT index = swapChain->GetCurrentBackBufferIndex();

    // Wait until the GPU is done with the last frame that used this buffer's allocator
    waitForFence(frameFenceValues[index]);

    commandAllocators[index]->Reset();
    commandList->Reset(commandAllocators[index].Get(), nullptr);

    // The swap chain hands us the buffer in PRESENT state; drawing needs RENDER_TARGET
    CD3DX12_RESOURCE_BARRIER toRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffers[index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &toRenderTarget);
}

void ModuleD3D12::postRender()
{
    UINT index = swapChain->GetCurrentBackBufferIndex();

    CD3DX12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        backBuffers[index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &toPresent);
    commandList->Close();

    ID3D12CommandList* lists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, lists);

    swapChain->Present(1, 0);

    frameFenceValues[index] = ++fenceCounter;
    commandQueue->Signal(fence.Get(), fenceCounter);
}

bool ModuleD3D12::cleanUp()
{
    // GPU must be idle before releasing anything it may still be using
    flush();

    if (fenceEvent)
    {
        CloseHandle(fenceEvent);
        fenceEvent = nullptr;
    }
    fence.Reset();

    // Release in reverse order of creation (views/buffers before the swap chain)
    commandList.Reset();
    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
        commandAllocators[i].Reset();
    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
        backBuffers[i].Reset();
    depthStencilBuffer.Reset();
    dsvHeap.Reset();
    rtvHeap.Reset();
    swapChain.Reset();
    commandQueue.Reset();

    device.Reset();
    adapter.Reset();
    factory.Reset();

    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleD3D12::getCurrentRtvHandle() const
{
    // i-th RTV descriptor <-> i-th back buffer
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), swapChain->GetCurrentBackBufferIndex(), rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleD3D12::getDsvHandle() const
{
    return dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void ModuleD3D12::getBackBufferSize(UINT& width, UINT& height) const
{
    DXGI_SWAP_CHAIN_DESC1 desc;
    swapChain->GetDesc1(&desc);
    width = desc.Width;
    height = desc.Height;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------


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

    return createBackBufferViews();
}

// Fetches the swap chain's buffers and (re)writes their RTV descriptors into the heap
bool ModuleD3D12::createBackBufferViews()
{
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

bool ModuleD3D12::createDepthStencil()
{
    // Non shader-visible heap with a single DSV slot: the buffer is recreated in the same slot on resize
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heapDesc.NumDescriptors = 1;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dsvHeap))))
    {
        LOG("Failed to create DSV descriptor heap");
        return false;
    }

    return createDepthBuffer();
}

// (Re)creates the depth texture at the back buffer size and rewrites its DSV descriptor
bool ModuleD3D12::createDepthBuffer()
{
    UINT width, height;
    getBackBufferSize(width, height);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    // arraySize = 1, mipLevels = 1 (0 would allocate a full mip chain)
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    // Clearing with this exact value is the fast path (1.0 = far plane, with the default LESS test)
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    if (FAILED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthStencilBuffer))))
    {
        LOG("Failed to create the depth buffer");
        return false;
    }

    // nullptr desc: default view of the whole texture
    device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, dsvHeap->GetCPUDescriptorHandleForHeapStart());
    return true;
}

bool ModuleD3D12::createFrameResources()
{
    for (UINT i = 0; i < BACK_BUFFER_COUNT; ++i)
    {
        if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]))))
        {
            LOG("Failed to create command allocator %u", i);
            return false;
        }
    }

    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList))))
    {
        LOG("Failed to create command list");
        return false;
    }

    // Lists are created in recording state; preRender() expects to Reset() a closed one
    commandList->Close();

    if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
    {
        LOG("Failed to create fence");
        return false;
    }

    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    return fenceEvent != nullptr;
}

void ModuleD3D12::waitForFence(UINT64 value)
{
    if (fence->GetCompletedValue() < value)
    {
        fence->SetEventOnCompletion(value, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

// Blocks until the GPU has finished everything submitted so far
void ModuleD3D12::flush()
{
    if (commandQueue && fence && fenceEvent)
    {
        commandQueue->Signal(fence.Get(), ++fenceCounter);
        waitForFence(fenceCounter);
    }
}
