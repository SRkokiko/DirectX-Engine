#include "Globals.h"
#include "ModuleEditor.h"

#include "ImGuiPass.h"
#include "ModuleD3D12.h"

ModuleEditor::ModuleEditor(ModuleD3D12* d3d12, HWND hWnd) : d3d12(d3d12), hWnd(hWnd)
{
}

ModuleEditor::~ModuleEditor()
{
    delete imguiPass;
}

bool ModuleEditor::init()
{
    imguiPass = new ImGuiPass(d3d12->getDevice(), hWnd);
    return true;
}

void ModuleEditor::preRender()
{
    // Once per frame, after ModuleD3D12::preRender() waited on the fence and before any ImGui call
    imguiPass->startFrame();
}

void ModuleEditor::render()
{
    // Registered last, so every module's ImGui calls and draws are already recorded.
    // The list is still open: ModuleD3D12::postRender() closes it
    imguiPass->record(d3d12->getCommandList());
}

bool ModuleEditor::cleanUp()
{
    // The backends own GPU resources (font texture, PSO), so make sure none are in flight
    d3d12->flush();

    delete imguiPass;
    imguiPass = nullptr;
    return true;
}
