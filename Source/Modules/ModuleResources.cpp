#include "Globals.h"
#include "ModuleResources.h"
#include "ModuleD3D12.h"

ModuleResources::ModuleResources(ModuleD3D12* d3d12) : d3d12(d3d12)
{
}

bool ModuleResources::init()
{
    ID3D12Device5* device = d3d12->getDevice();

    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator))))
    {
        LOG("Failed to create resources command allocator");
        return false;
    }

    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList))))
    {
        LOG("Failed to create resources command list");
        return false;
    }

    // Lists are created open; close it so the first Reset() is valid
    commandList->Close();
    return true;
}

bool ModuleResources::cleanUp()
{
    commandList.Reset();
    commandAllocator.Reset();
    return true;
}

ComPtr<ID3D12Resource> ModuleResources::createUploadBuffer(const void* data, size_t size)
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> buffer;
    if (FAILED(d3d12->getDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer))))
    {
        LOG("Failed to create upload buffer");
        return nullptr;
    }

    if (data)
    {
        // Empty read range: the CPU won't read from this buffer
        D3D12_RANGE readRange = { 0, 0 };
        void* mapped = nullptr;
        if (FAILED(buffer->Map(0, &readRange, &mapped)))
        {
            LOG("Failed to map upload buffer");
            return nullptr;
        }

        memcpy(mapped, data, size);
        buffer->Unmap(0, nullptr);
    }

    return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::createDefaultBuffer(const void* data, size_t size, D3D12_RESOURCE_STATES finalState)
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> buffer;
    if (FAILED(d3d12->getDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer))))
    {
        LOG("Failed to create default buffer");
        return nullptr;
    }

    ComPtr<ID3D12Resource> upload = createUploadBuffer(data, size);
    if (!upload)
        return nullptr;

    // The previous copy was flushed, so the allocator is no longer in use by the GPU
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), nullptr);

    commandList->CopyBufferRegion(buffer.Get(), 0, upload.Get(), 0, size);

    if (finalState != D3D12_RESOURCE_STATE_COMMON)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = buffer.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = finalState;
        commandList->ResourceBarrier(1, &barrier);
    }

    commandList->Close();

    ID3D12CommandList* lists[] = { commandList.Get() };
    d3d12->getCommandQueue()->ExecuteCommandLists(1, lists);

    // The upload buffer must stay alive until the copy has finished
    d3d12->flush();

    return buffer;
}
