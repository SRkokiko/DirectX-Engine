#include "Globals.h"
#include "Exercise3.h"

#include "Modules/ModuleD3D12.h"
#include "Modules/ModuleResources.h"
#include "Modules/ModuleDebugDraw.h"

#include "ReadData.h"

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

Exercise3::Exercise3(ModuleD3D12* d3d12, ModuleResources* resources, ModuleDebugDraw* debugDraw) : d3d12(d3d12), resources(resources), debugDraw(debugDraw)
{
}

bool Exercise3::init()
{
    return createVertexBuffer() && createRootSignature() && createPSO();
}

void Exercise3::render()
{
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

    UINT width, height;
    d3d12->getBackBufferSize(width, height);

    // NOTE: ModuleExerciseHub already cleared the back buffer, but bound it without a depth buffer.
    // Re-bind the same render target together with the DSV (so only this exercise needs a D32_FLOAT PSO) and clear the depth
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getCurrentRtvHandle();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = d3d12->getDsvHandle();
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // The list was reset without a PSO, so everything the draw needs is set here
    commandList->SetGraphicsRootSignature(rootSignature.Get());
    commandList->SetPipelineState(pso.Get());
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    D3D12_VIEWPORT viewport{ 0.0f, 0.0f, float(width), float(height), 0.0f, 1.0f };
    D3D12_RECT scissor{ 0, 0, LONG(width), LONG(height) };
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissor);

    Matrix model = Matrix::Identity;
    Matrix view = Matrix::CreateLookAt(Vector3(0.0f, 10.0f, 10.0f), Vector3::Zero, Vector3::Up);
    float aspect = float(width) / float(height);
    Matrix proj = Matrix::CreatePerspectiveFieldOfView(XM_PIDIV4, aspect, 0.1f, 1000.0f);
    Matrix mvp = (model * view * proj).Transpose(); // SimpleMath is row-major and pre-multiplies; HLSL expects column-major, hence the transpose

    // Must come after SetGraphicsRootSignature: the signature defines the layout, the command list holds the values
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(Matrix) / sizeof(UINT32), &mvp, 0);

    commandList->DrawInstanced(3, 1, 0, 0);

    // Orientation aids: floor grid on the XZ plane and the XYZ axes at the world origin.
    // ModuleDebugDraw draws them at the end of the frame into the render target and depth buffer bound above
    dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
    dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);
    debugDraw->setCamera(view, proj);
}

bool Exercise3::cleanUp()
{
    pso.Reset();
    rootSignature.Reset();
    vertexBuffer.Reset();
    return true;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

bool Exercise3::createVertexBuffer()
{
    // Local (model) space: the vertex shader will bring these to clip space with the MVP matrix
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

bool Exercise3::createRootSignature()
{
    // One root parameter: the MVP matrix as root constants, bound to shader register b0.
    // The flag lets the Input Assembler read the vertex buffer
    CD3DX12_ROOT_PARAMETER rootParameter = {};
    rootParameter.InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0);

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Init(1, &rootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

bool Exercise3::createPSO()
{
    // ReadData throws if a .cso can't be found (next to the exe or in the working directory)
    std::vector<uint8_t> dataVS, dataPS;
    try
    {
        dataVS = DX::ReadData(L"Exercise3VS.cso");
        dataPS = DX::ReadData(L"Exercise3PS.cso");
    }
    catch (const std::exception&)
    {
        LOG("Failed to read the Exercise3 shaders (.cso)");
        return false;
    }

    // The semantic name must match the vertex shader's input: float3 pos : MY_POSITION
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "MY_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { dataVS.data(), dataVS.size() };
    psoDesc.PS = { dataPS.data(), dataPS.size() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = 0xffffffff;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // depth test and write enabled, LESS

    if (FAILED(d3d12->getDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso))))
    {
        LOG("Failed to create the Exercise3 PSO");
        return false;
    }

    return true;
}
