#pragma once

#include "Globals.h"

class Module
{
public:

	Module()
	{
	}

    virtual ~Module()
    {

    }

	virtual bool init() 
	{
		return true; 
	}

	/**
	 * @brief Called when the window's client area changes size.
	 * Never called with a zero width or height (minimized window).
	 */
	virtual void resize(unsigned width, unsigned height)
	{
	}

	/**
	 * @brief Called once per frame before rendering.
	 * @param dt Duration of the last frame in seconds; multiply any per-frame movement by it.
	 */
	virtual void update(float dt)
	{
	}

    virtual void preRender()
    {
    }

    virtual void postRender()
	{
	}

    virtual void render()
    {
    }

    virtual bool cleanUp()
	{ 
		return true; 
	}
};
