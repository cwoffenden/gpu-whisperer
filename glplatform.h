/**
 * \file glplatform.h
 * Helper to smooth out GL differences across the various older versions.
 */
#pragma once

/*
 * Notes for Mac: GLFW_INCLUDE_GLCOREARB will include gl3.h, which with a GL2
 * context will fail for calls to glGenVertexArrays, etc., (invalid operation),
 * needing instead the APPLE suffix versions. The APPLE suffix versions fail
 * with a 3+ context, and since these higher GL's need VAOs we'll fail to draw
 * by not creating one.
 *
 * Whatever this is... it works fine on the tested platforms (Mac 10.5 PPC and
 * 26 ARM, with no testing in between, Debian 13 on x64 and ARM).
 */
#ifdef __APPLE__
#include <AvailabilityMacros.h>
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif
#endif
#define GLFW_INCLUDE_GLEXT

#include <GLFW/glfw3.h>

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_RED_RGTC1
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#endif
#ifndef GL_COMPRESSED_RED_GREEN_RGTC2
#define GL_COMPRESSED_RED_GREEN_RGTC2 0x8DBD
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F GL_RGBA32F_ARB
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F GL_RGBA16F_ARB
#endif
#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT GL_HALF_FLOAT_ARB
#endif

#ifndef GL_VERSION_3_0
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
#endif
#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
#endif
#ifndef glBindFramebuffer
#define glBindFramebuffer glBindFramebufferEXT
#endif
#ifndef glDeleteFramebuffers
#define glDeleteFramebuffers glDeleteFramebuffersEXT
#endif
#ifndef glGenFramebuffers
#define glGenFramebuffers glGenFramebuffersEXT
#endif
#ifndef glCheckFramebufferStatus
#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#endif
#ifndef glFramebufferTexture2D
#define glFramebufferTexture2D glFramebufferTexture2DEXT
#endif
#endif
