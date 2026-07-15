/**
* \file debugview.h
 * Utilities to show program info.
 */
#pragma once

#include "glcommon.h"

/**
 * Cycles through debug textures. To be manually verified, the textures should
 * show correct RGB ordering and compressed decoding.
 *
 * \note Control will return to the callee after the window has close.
 *
 * \param[in] glVers Context version acquired at start-up
 * \param[in] window GLFW window with a valid context
 */
void showDebugView(GLFWwindow* _Nonnull window, ContextVersion glVers);
