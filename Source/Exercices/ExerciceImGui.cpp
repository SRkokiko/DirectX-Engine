#include "Globals.h"
#include "ExerciceImGui.h"

#include "Modules/ModuleD3D12.h"

#include "imgui.h"

ExerciceImGui::ExerciceImGui(ModuleD3D12* d3d12) : d3d12(d3d12)
{
}

void ExerciceImGui::render()
{
    ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getCurrentRtvHandle();
    const float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

    // Between ModuleEditor's NewFrame (preRender) and Render (its render, registered last)
    ImGui::ShowDemoWindow();
}
