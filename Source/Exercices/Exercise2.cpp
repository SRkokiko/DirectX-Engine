#include "Globals.h"
#include "Exercise2.h"

#include "Modules/ModuleD3D12.h"
#include "Modules/ModuleResources.h"

Exercise2::Exercise2(ModuleD3D12* d3d12, ModuleResources* resources) : d3d12(d3d12), resources(resources)
{
}

bool Exercise2::init()
{
    return createVertexBuffer() && createRootSignature();
}

bool Exercise2::cleanUp()
{
    rootSignature.Reset();
    vertexBuffer.Reset();
    return true;
}

bool Exercise2::createVertexBuffer()
{
    // Clip space: x and y go from -1 to 1, so no World/View/Projection is needed in the vertex shader
    const Vector3 vertices[3] =
    {
        { -1.0f, -1.0f, 0.0f }, // 0: bottom left
        {  0.0f,  1.0f, 0.0f }, // 1: top middle
        {  1.0f, -1.0f, 0.0f }, // 2: bottom right
    };

    vertexBuffer = resources->createDefaultBuffer(vertices, sizeof(vertices), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    if (!vertexBuffer)
        return false;

    // A vertex buffer view needs no descriptor heap: it is just address, total size and stride
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = sizeof(vertices);
    vertexBufferView.StrideInBytes = sizeof(Vector3);

    return true;
}

bool Exercise2::createRootSignature()
{
    // Empty root signature: the shaders use no resources. The flag lets the Input Assembler read the vertex buffer
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> blob;
    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr)))
    {
        LOG("Failed to serialize root signature");
        return false;
    }

    if (FAILED(d3d12->getDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))))
    {
        LOG("Failed to create root signature");
        return false;
    }

    return true;
}
