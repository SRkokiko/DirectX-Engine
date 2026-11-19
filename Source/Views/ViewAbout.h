#pragma once

#include "View.h"

/**
 * @brief View that shows an "About" window: engine name, description, author, libraries with versions and license.
 */
class ViewAbout : public View
{
public:

    /** @brief Builds the About window; needs the ImGui frame ModuleEditor started in preRender(). */
    void render() override;
};
