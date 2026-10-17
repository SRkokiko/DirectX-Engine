#pragma once

#include "Module.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public Module
{
public:

    bool init() override;
    bool cleanUp() override;

    ID3D12Device5* getDevice() const { return device.Get(); }

private:
    ComPtr<IDXGIFactory6> factory;
    ComPtr<IDXGIAdapter4> adapter;
    ComPtr<ID3D12Device5> device;
};
