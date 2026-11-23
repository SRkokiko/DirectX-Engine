#include "Globals.h"
#include "ModuleDebugDraw.h"

#include "DebugDrawPass.h"
#include "ModuleD3D12.h"

ModuleDebugDraw::ModuleDebugDraw(ModuleD3D12* d3d12) : d3d12(d3d12)
{
}

ModuleDebugDraw::~ModuleDebugDraw()
{
    delete debugDrawPass;
}

bool ModuleDebugDraw::init()
{
    debugDrawPass = new DebugDrawPass(d3d12->getDevice(), d3d12->getCommandQueue());
    return true;
}

void ModuleDebugDraw::render()
{
    if (!hasCamera)
        return;

    UINT width, height;
    d3d12->getBackBufferSize(width, height);

    debugDrawPass->record(d3d12->getCommandList(), width, height, view, proj);
    hasCamera = false;
}

bool ModuleDebugDraw::cleanUp()
{
    // The pass owns GPU resources (PSOs, vertex buffers), so make sure none are in flight
    d3d12->flush();

    delete debugDrawPass;
    debugDrawPass = nullptr;
    return true;
}

void ModuleDebugDraw::setCamera(const Matrix& view, const Matrix& proj)
{
    this->view = view;
    this->proj = proj;
    hasCamera = true;
}
