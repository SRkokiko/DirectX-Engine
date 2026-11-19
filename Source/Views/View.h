#pragma once

#include "Module.h"

/**
 * @brief Base class for views: ImGui windows that can be closed with their X button.
 *
 * Pass `&open` to ImGui::Begin (or the window's p_open parameter). ModuleExerciseHub polls isOpen() every frame
 * and destroys the view once it is closed; enabling it again from the menu creates a fresh, open instance.
 */
class View : public Module
{
public:

    bool isOpen() const { return open; }

protected:
    // NOTE: we add this indirection and not Entry::enabled directly because the hub's entries live in a std::vector, so a pointer into them could dangle,
    // and this way a view doesn't depend on the hub. The hub polls isOpen() and clears Entry::enabled itself.
    bool open = true;
};
