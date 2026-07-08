/**
 * \file utils/rgba.h
 * Pixel helpers.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <algorithm>

//***************************** Compiler Helpers ******************************/

/**
 * \def __has_feature
 * Dummy \c __has_feature implementation for when not using Clang (in which case
 * all requested features are reported as unimplemented).
 *
 * \param feature compiler feature to query
 */
#ifndef __has_feature
#ifndef __SNC__
#define __has_feature(feature) 0
#endif
#endif

/**
 * \def _Nonnull
 * Qualifier that marks a pointer as \e never being \c null (see also \c
 * #_Nullable). Valid for both parameters and return values.
 *
 * \note This is a Clang-only feature
 * \sa https://clang.llvm.org/docs/AttributeReference.html#nullability-attributes
 */
#ifndef _Nonnull
#if !__has_feature(nullability)
#define _Nonnull
#endif
#endif

//*****************************************************************************/

/**
 * Channel indices.
 */
enum ChannelIndex {
	CHANNEL_R = 0,  /**< Red channel. */
	CHANNEL_G = 1,  /**< Green channel. */
	CHANNEL_B = 2,  /**< Blue channel. */
	CHANNEL_A = 3,  /**< Alpha channel. */
};

/**
 * Container for a single RGBA 4-tuple or pixel.
 *
 * \note Initialisers and setters operating on fewer than all four channels
 * will default to setting unused RGB channels to zero and saturating the alpha
 * channel to \a S.
 *
 * \tparam T numeric type (e.g. \c float)
 * \tparam S saturation value (e.g. \c 1.0 for \c float, \c 255 for \c uint8_t)
 */
template<typename T, int S = 1>
struct RGBA {
	/**
	 * Initialises the RGBA channels.
	 *
	 * \param[in] rVal red channel value
	 * \param[in] gVal green channel value
	 * \param[in] bVal optional blue channel value
	 * \param[in] aVal optional alpha channel value
	 */
	RGBA(T rVal, T gVal, T bVal = T(0), T aVal = T(S))
		: r(rVal)
		, g(gVal)
		, b(bVal)
		, a(aVal) {}
	/**
	 * Initialises just the red channel.
	 *
	 * \param[in] rVal red channel value
	 */
	RGBA(T rVal = T(0))
		: r(rVal)
		, g(T(0))
		, b(T(0))
		, a(T(S)) {}
	/**
	 * Convenience for setting the red channel.
	 *
	 * \param[in] rVal red channel value
	 * \return \c reference to this object
	 */
	RGBA& operator =(T const rVal) {
		r = rVal;
		g = T(0);
		b = T(0);
		a = T(S);
		return *this;
	}
	/**
	 * Convenience for setting the RGB channels to a single luminance value.
	 * 
	 * \param[in] lVal luminance value
	 * \return \c reference to this object
	 */
	RGBA& mono(T const lVal) {
		r = lVal;
		g = lVal;
		b = lVal;
		a = T(S);
		return *this;
	}
	/**
	 * Sets the channels.
	 *
	 * \param[in] rVal red channel value
	 * \param[in] gVal green channel value
	 * \param[in] bVal optional blue channel value
	 * \param[in] aVal optional alpha channel value
	 * \return \c reference to this object 
	 */
	RGBA& set(T rVal, T gVal, T bVal = T(0), T aVal = T(S)) {
		r = rVal;
		g = gVal;
		b = bVal;
		a = aVal;
		return *this;
	}
	/**
	 * Sets \a len pixels to a single value.
	 */
	static void fill(RGBA* _Nonnull dst, unsigned const len, const RGBA& val) {
		for (unsigned n = len; n > 0; n--) {
			memcpy(dst++, val, sizeof(RGBA));
		}
	}
	/**
	 * Helper to copy a block of pixels onto another, cropping the source or
	 * destination so that one fits into the other with the requested offsets.
	 *
	 * \todo move the code out of the template and to a helper, since the only thing to change is the size of the memcpy
	 * \todo an image wrapper at this point; I know, it wasn't the plan but look at what we're doing...
	 */
	static void blit(const RGBA* _Nonnull const src, unsigned const srcW, unsigned const srcH, RGBA* _Nonnull dst, unsigned const dstW, unsigned const dstH, int const dstX, int const dstY) {
		int srcX = (dstX >= 0) ? 0 : -dstX;
		int srcY = (dstY >= 0) ? 0 : -dstY;
		int boxW = std::min(static_cast<int>(dstW) - dstX, static_cast<int>(srcW) - srcX);
		int boxH = std::min(static_cast<int>(dstH) - dstY, static_cast<int>(srcH) - srcY);
		if (boxW > 0 && boxH > 0) {
			const RGBA* srcOff = src + srcY * srcW + srcX;
			RGBA* dstOff = dst + (((dstY > 0) ? dstY * dstW : 0)) + ((dstX > 0) ? dstX : 0);
			for (int line = boxH; line > 0; line--) {
				memcpy(dstOff, srcOff, sizeof(RGBA) * boxW);
				srcOff += srcW;
				dstOff += dstW;
			}
		}
	}
	/**
	 * Compares this channels' contents to another.
	 *
	 * \param[in] val data to compare with
	 * \return \c true if the contents are exactly the same
	 */
	bool operator ==(const RGBA& val) {
		return r == val.r && g = val.g && b = val.b && a = val.a;
	}
	/**
	 * Returns a pointer to the internal representation.
	 *
	 * \return pointer to the red value at the start of the container
	 */
	operator T* _Nonnull () {
		return &r;
	}
	operator const T* _Nonnull () const {
		return &r;
	}
	T r; /**< Red channel. */
	T g; /**< Green channel. */
	T b; /**< Blue channel. */
	T a; /**< Alpha channel. */
};

/**
 * 32-bit float RGBA pixel saturating at \c 1.0.
 */
typedef RGBA<float, 1> RGBAf32;

/**
 * 8-bit unsigned byte RGBA pixel saturating at \c 255.
 */
typedef RGBA<uint8_t, UINT8_MAX> RGBAu08;

/**
 * 16-bit unsigned byte RGBA pixel saturating at \c 65535.
 */
typedef RGBA<uint16_t, UINT16_MAX> RGBAu16;

/**
 * Helper to square pixel data errors.
 *
 * \param[in] val value to process
 * \return \a val  squared
 * \tparam T numeric type (e.g. \c double)
 */
template<typename T>
static inline T rgbaSquare(T val) {
	return val * val;
}

/**
 * Calculates the accumulated square error difference.
 *
 * \note RGBA values are treated as linear with equal weighting.
 *
 * \todo Make part of the RGBA class
 *
 * \param[in] src source pixel start address
 * \param[in] dst processed destination pixel start address
 * \param[in] len number of pixels to process
 * \return an accumulated error metric calculated for all the pixels
 * \tparam T numeric type for the result (e.g. \c double)
 */
template<typename T = double>
T rgbaCalcErrorLinear(const RGBAf32* _Nonnull src, const RGBAf32* _Nonnull dst, unsigned len) {
	T err = T(0);
	for (; len > 0; len--) {
		err += rgbaSquare<T>(src->r - dst->r);
		err += rgbaSquare<T>(src->g - dst->g);
		err += rgbaSquare<T>(src->b - dst->b);
		err += rgbaSquare<T>(src->a - dst->a);
		src++;
		dst++;
	}
	return err;
}

/**
 * Calculates the accumulated square error difference for a single channel.
 *
 * \note The channel's values are treated as linear with equal weighting.
 *
 * \todo Make part of the RGBA class
 *
 * \param[in] src source pixel start address
 * \param[in] dst processed destination pixel start address
 * \param[in] len number of pixels to process
 * \return an accumulated error metric calculated for all the pixels
 * \tparam T numeric type for the result (e.g. \c double)
 * \tparam I channel index (e.g. \c 0 for red, \c 1 for green)
 */
template<typename T = double, ChannelIndex I = 0>
T rgbaCalcErrorLinearChannel(const RGBAf32* _Nonnull src, const RGBAf32* _Nonnull dst, unsigned len) {
	T err = T(0);
	for (; len > 0; len--) {
		err += rgbaSquare<T>((*src++)[I] - (*dst++)[I]);
	}
	return err;
}

/**
 * Helper to convert from 8-bit integer to 32-bit float pixels.
 *
 * \param[in] src source pixels
 * \param[out] dst destination pixels
 * \param[in] len number of \e pixels to convert
 */
void convert(const RGBAu08* _Nonnull src, RGBAf32* _Nonnull dst, unsigned len);

/**
 * Helper to convert from 32-bit float to 8-bit integer pixels.
 *
 * \param[in] src source pixels
 * \param[out] dst destination pixels
 * \param[in] len number of \e pixels to convert
 */
void convert(const RGBAf32* _Nonnull src, RGBAu08* _Nonnull dst, unsigned len);

/**
 * Helper to convert from 32-bit float to 16-bit integer pixels.
 *
 * \param[in] src source pixels
 * \param[out] dst destination pixels
 * \param[in] len number of \e pixels to convert
 */
void convert(const RGBAf32* _Nonnull src, RGBAu16* _Nonnull dst, unsigned len);
