#include "Globals.h"
#include "Exercise1.h"

#include "Modules/ModuleD3D12.h"

Exercise1::Exercise1(ModuleD3D12* d3d12) : d3d12(d3d12)
{
}

void Exercise1::render()
{
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

    const float clearColor[4] = { 1.0f, 0.41f, 0.71f, 1.0f };
    commandList->ClearRenderTargetView(d3d12->getCurrentRtvHandle(), clearColor, 0, nullptr);
}
