#include "rgba.h"

#include <cmath>

// TODO: these can be entirely templatised

/**
 * \def RGBA_MAX_08
 * Maximum value for an 8-bit channel as a float.
 */
#define RGBA_MAX_08 255.0f

/**
 * \def RGBA_MAX_08
 * Maximum value for a 16-bit channel as a float.
 */
#define RGBA_MAX_16 65535.0f

/**
 * Constrains a value between upper and lower bounds.
 *
 * \param[in] val value to constrain
 * \param[in] min lower bound (inclusive)
 * \param[in] max upper bound (inclusive)
 * \return \a val constrained
 * \tparam T numeric type (e.g. \c float)
 */
template<typename T>
static inline T clamp(T const val, T const min, T const max) {
	return std::min(max, std::max(min, val));
}

void convert(const RGBAu08* src, RGBAf32* dst, unsigned const len) {
	for (unsigned n = len; n > 0; n--) {
		dst->r = src->r / RGBA_MAX_08;
		dst->g = src->g / RGBA_MAX_08;
		dst->b = src->b / RGBA_MAX_08;
		dst->a = src->a / RGBA_MAX_08;
		src++;
		dst++;
	}
}

void convert(const RGBAf32* src, RGBAu08* dst, unsigned const len) {
	for (unsigned n = len; n > 0; n--) {
		dst->r = static_cast<uint8_t>(std::round(clamp(src->r * RGBA_MAX_08, 0.0f, RGBA_MAX_08)));
		dst->g = static_cast<uint8_t>(std::round(clamp(src->g * RGBA_MAX_08, 0.0f, RGBA_MAX_08)));
		dst->b = static_cast<uint8_t>(std::round(clamp(src->b * RGBA_MAX_08, 0.0f, RGBA_MAX_08)));
		dst->a = static_cast<uint8_t>(std::round(clamp(src->a * RGBA_MAX_08, 0.0f, RGBA_MAX_08)));
		src++;
		dst++;
	}
}

void convert(const RGBAf32* src, RGBAu16* dst, unsigned const len) {
	for (unsigned n = len; n > 0; n--) {
		dst->r = static_cast<uint16_t>(std::round(clamp(src->r * RGBA_MAX_16, 0.0f, RGBA_MAX_16)));
		dst->g = static_cast<uint16_t>(std::round(clamp(src->g * RGBA_MAX_16, 0.0f, RGBA_MAX_16)));
		dst->b = static_cast<uint16_t>(std::round(clamp(src->b * RGBA_MAX_16, 0.0f, RGBA_MAX_16)));
		dst->a = static_cast<uint16_t>(std::round(clamp(src->a * RGBA_MAX_16, 0.0f, RGBA_MAX_16)));
		src++;
		dst++;
	}
}
