#pragma once

#include "View.h"

/**
 * @brief View that builds the Dear ImGui demo window every frame (drawn later by ModuleEditor).
 */
class ViewImGuiDemo : public View
{
public:

    /** @brief Builds the demo window; needs the ImGui frame ModuleEditor started in preRender(). */
    void render() override;
};
