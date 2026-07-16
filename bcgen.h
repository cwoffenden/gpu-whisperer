/**
 * \file bcgen.h
 * Compressed block generation utilities.
 */
#pragma once

#include "glplatform.h"

/**
 * Creates a BC1 texture grid with endpoints varying between the specified
 * minimum and maximum, on the selected \a channel only. Variance in the first
 * endpoint runs down the Y-axis (being stable in the X).
 *
 * \note The \a fill determines the maximum endpoint values and the eventual
 * texture size: \c GL_RED and \c GL_BLUE are \c 31, and \c GL_GREEN is \c 63
 * (with no support for the cutout alpha variant).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \param[in] fill which channel to use (e.g. \c GL_RED for the red channel)
 * \return the number of 4x4 entries
 */
unsigned createBC1(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, GLenum fill = GL_RED);

/**
 * Creates a BC3 texture grid with endpoints varying between the specified
 * minimum and maximum, on the selected \a channel only. Variance in the first
 * endpoint runs down the Y-axis (being stable in the X).
 *
 * \note The \a fill determines the maximum endpoint values and the eventual
 * texture size: \c GL_RED and \c GL_BLUE are \c 31, \c GL_GREEN is \c 63, and
 * \c GL_ALPHA is \c 255 (but note that the interpolation accuracy of BC3 alpha
 * is lower than its equivalent BC4).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \param[in] fill which channel to use (e.g. \c GL_RED for the red channel)
 * \return the number of 4x4 entries
 */
unsigned createBC3(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, GLenum fill = GL_RED);

/**
 * Creates a red-only BC4 texture grid with endpoints varying between the
 * specified minimum and maximum. Variance in the first endpoint runs down the
 * Y-axis (being stable in the X).
 *
 * \note The maximum endpoint value is \c 255 (noting the same caveat in BC3
 * alpha, that BC4 has a higher interpolation accuracy).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \return the number of 4x4 blocks
 */
unsigned createBC4(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1);

/**
 * Creates a red/green BC5 texture grid with endpoints varying between the
 * specified minimum and maximum. Variance in the first endpoint runs down the
 * Y-axis (being stable in the X).
 *
 * \note For BC5 both \c GL_RED and \c GL_GREEN fills have a maximum endpoint
 * value of \c 255 (unlike the varied BC1 and BC3).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \param[in] fill which channel to use (e.g. \c GL_RED for the red channel)
 * \return the number of 4x4 blocks
 */
unsigned createBC5(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, GLenum fill = GL_RED);
