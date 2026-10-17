#pragma once

#include "Module.h"

class ModuleD3D12 : public Module
{
public:

    bool init() override;
    bool cleanUp() override;
};
