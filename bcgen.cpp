#include "bcgen.h"

#include <cassert>
#include <cstring>
#include <new>

#include "glcommon.h"

namespace /*anonymous*/ {
//***************************** Block Containers ******************************/

/**
 * \c BCBlock internal union for 16-bit colour endpoints.
 */
union RGB565 {
	uint16_t rgb565;   /**< Combined 16-bit RGB value. */
	struct {
	#ifdef XP_BIG_ENDIAN
		uint16_t r: 5; /**< Red value. */
		uint16_t g: 6; /**< Green value. */
		uint16_t b: 5; /**< Blue value. */
	#else
		uint16_t b: 5; /**< Blue value. */
		uint16_t g: 6; /**< Green value. */
		uint16_t r: 5; /**< Red value. */
	#endif
	};
};

static_assert(sizeof(RGB565) == 2, "RGB565 should be 2 bytes");

/**
 * \def BC_RAW_CTOR
 * Helper to fill the \c raw[8] part of a BC block's constructor.
 */
#ifndef BC_RAW_CTOR
#if __cplusplus >= 201103L
#define BC_RAW_CTOR(name)\
name(uint8_t raw0, uint8_t raw1, uint8_t raw2, uint8_t raw3,   \
     uint8_t raw4, uint8_t raw5, uint8_t raw6, uint8_t raw7)   \
	: raw { raw0, raw1, raw2, raw3, raw4, raw5, raw6, raw7 } {}
#else
#define BC_RAW_CTOR(name)\
name(uint8_t raw0, uint8_t raw1, uint8_t raw2, uint8_t raw3,   \
     uint8_t raw4, uint8_t raw5, uint8_t raw6, uint8_t raw7) { \
	raw[0] = raw0; raw[1] = raw1; raw[2] = raw2; raw[3] = raw3;\
	raw[4] = raw4; raw[5] = raw5; raw[6] = raw6; raw[7] = raw7;\
}
#endif
#endif

/**
 * Container for a BC1 block (or BC3 colour).
 */
union BC1Block {
	/**
	 * Raw byte data.
	 */
	uint8_t raw[8];
	struct {
		/**
		 * The two RGB endpoints.
		 */
		RGB565 endpt[2];
		union {
			/**
			 * Raw texel data.
			 */
			uint32_t texels __unused;
		#ifndef NDEBUG
			/**
			 * Texel rows.
			 */
			struct {
				uint8_t x0: 2; /**< Texel <tt>x = 0</tt>. */
				uint8_t x1: 2; /**< Texel <tt>x = 1</tt>. */
				uint8_t x2: 2; /**< Texel <tt>x = 2</tt>. */
				uint8_t x3: 2; /**< Texel <tt>x = 3</tt>. */
			} y[4];
		#endif
		};
	};
	/**
	 * The block as a single 128-bit value.
	 */
	uint64_t data __unused;
	/**
	 * An uninitialised block.
	 */
	BC1Block() {}
	/**
	 * Initialises all eight bytes.
	 */
	BC_RAW_CTOR(BC1Block)
};

static_assert(sizeof(BC1Block) == 8, "BC1 block should be 8 bytes");

/**
 * Container for a BC4 block (or BC3 alpha, or BC5 channel).
 */
union BC4Block {
	/**
	 * Raw byte data.
	 */
	uint8_t raw[8];
	struct {
		uint64_t /* endpt[2] */ : 16;
		/**
		 * Raw texel data.
		 */
		uint64_t texels: 48 __unused;
	};
	struct {
		/**
		 * The two red-channel endpoints.
		 */
		uint8_t endpt[2];
	#ifndef NDEBUG
		struct __attribute__((packed)) {
			uint32_t y0x0: 3; /**< Texel <tt>x = 0, y = 0</tt>. */
			uint32_t y0x1: 3; /**< Texel <tt>x = 1, y = 0</tt>. */
			uint32_t y0x2: 3; /**< Texel <tt>x = 2, y = 0</tt>. */
			uint32_t y0x3: 3; /**< Texel <tt>x = 3, y = 0</tt>. */

			uint32_t y1x0: 3; /**< Texel <tt>x = 0, y = 1</tt>. */
			uint32_t y1x1: 3; /**< Texel <tt>x = 1, y = 1</tt>. */
			uint32_t y1x2: 3; /**< Texel <tt>x = 2, y = 1</tt>. */
			uint32_t y1x3: 3; /**< Texel <tt>x = 3, y = 1</tt>. */

			uint32_t y2x0: 3; /**< Texel <tt>x = 0, y = 2</tt>. */
			uint32_t y2x1: 3; /**< Texel <tt>x = 1, y = 2</tt>. */
			uint32_t y2x2: 3; /**< Texel <tt>x = 2, y = 2</tt>. */
			uint32_t y2x3: 3; /**< Texel <tt>x = 3, y = 2</tt>. */

			uint32_t y3x0: 3; /**< Texel <tt>x = 0, y = 3</tt>. */
			uint32_t y3x1: 3; /**< Texel <tt>x = 1, y = 3</tt>. */
			uint32_t y3x2: 3; /**< Texel <tt>x = 2, y = 3</tt>. */
			uint32_t y3x3: 3; /**< Texel <tt>x = 3, y = 3</tt>. */
		};
	#endif
	};
	/**
	 * The block as a single 128-bit value.
	 */
	uint64_t data __unused;
	/**
	 * An uninitialised block.
	 */
	BC4Block() {}
	/**
	 * Initialises all eight bytes.
	 */
	BC_RAW_CTOR(BC4Block)
};

static_assert(sizeof(BC4Block) == 8, "BC4 block should be 8 bytes");

/**
 * Container for a BC3 block.
 */
struct BC3Block {
	/**
	 * The alpha sub-block.
	 */
	BC4Block alpha;
	/**
	 * The colour sub-block.
	 */
	BC1Block color;
	/**
	 * An uninitialised block.
	 */
	BC3Block() {}
};

static_assert(sizeof(BC3Block) == 16, "BC4 block should be 16 bytes");

/**
 * Container for a BC5 block.
 */
struct BC5Block {
	/**
	 * First channel.
	 */
	BC4Block red;
	/**
	 * Second channel.
	 */
	BC4Block green;
	/**
	 * An uninitialised block.
	 */
	BC5Block() {}
};

static_assert(sizeof(BC5Block) == 16, "BC5 block should be 16 bytes");

//********************************** Helpers **********************************/

/**
 * Internal helper to perform the work of filling a BC1 or BC3 colour block.
 * The layout is the same, the decoder type determines the result.
 *
 * \note The texel pattern puts the four derived values into the first row.
 *
 * \param[in] val0 first endpoint (the target colour determined by \a fill)
 * \param[in] val1 second endpoint (the target colour determined by \a fill)
 * \param[in] fill choice of \c GL_RED, \c GL_GREEN or \c GL_BLUE channel (or \c GL_RGB for all)
 * \param[out] block address of the block to fill
 */
void fillBC1Block(unsigned const val0, unsigned const val1, GLenum const fill, BC1Block* _Nonnull const block) {
	assert(block);
	new(block) BC1Block(
		0x00, 0x00,
		0x00, 0x00,
		0xE4, 0x39, // 0123 | 1230
		0x4E, 0x93  // 2301 | 3012
	);
	RGB565* endpt = block->endpt;
	switch (fill) {
	case GL_RED:
		endpt[0].r = val0;
		endpt[1].r = val1;
		break;
	case GL_GREEN:
		endpt[0].g = val0;
		endpt[1].g = val1;
		break;
	case GL_BLUE:
		endpt[0].b = val0;
		endpt[1].b = val1;
		break;
	default:
		endpt[0].rgb565 = val0;
		endpt[1].rgb565 = val1;
	}
#ifdef XP_BIG_ENDIAN
	endpt[0].rgb565 = ((endpt[0].rgb565 & 0x00FF) << 8) | ((endpt[0].rgb565 & 0xFF00) >> 8);
	endpt[1].rgb565 = ((endpt[1].rgb565 & 0x00FF) << 8) | ((endpt[1].rgb565 & 0xFF00) >> 8);
#endif
}

/**
 * Internal helper to perform the work of filling a BC3 alpha or BC4 block (or
 * the two halves of a BC5 block). The layout is the same, the decoder type
 * determines the result.
 *
 * \note The texel pattern puts the eight derived values into the first rows.
 *
 * \param[in] val0 first endpoint
 * \param[in] val1 second endpoint
 * \param[out] block address of the block to fill
 */
void fillBC4Block(unsigned const val0, unsigned const val1, BC4Block* _Nonnull const block) {
	assert(block);
	new(block) BC4Block(
		0x00, 0x00,
		0x88, 0xC6, 0xFA, // 01234567
		0x77, 0x39, 0x05  // 76543210
	);
	block->endpt[0] = val0;
	block->endpt[1] = val1;
}
}

//******************************** Public API *********************************/

unsigned createBC1(GLuint const txId, unsigned const min0, unsigned const max0, unsigned const min1, unsigned const max1, GLenum const fill) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	assert(fill >= GL_RED && fill <= GL_BLUE);
	unsigned gridW = (max1 + 1) - min1;
	unsigned gridH = (max0 + 1) - min0;
	unsigned count = gridW * gridH;
	unsigned maxWH = 32 * 32;
	if (fill == GL_GREEN) {
		maxWH = 64 * 64;
	}
	if (count > 0 && count <= maxWH) {
		BC1Block* const blocks = new BC1Block[count];
		BC1Block* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC1Block(gridY, gridX, fill, next++);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BC1Block)), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}

unsigned createBC3(GLuint const txId, unsigned const min0, unsigned const max0, unsigned const min1, unsigned const max1, GLenum const fill) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	assert(fill >= GL_RED && fill <= GL_ALPHA);
	unsigned gridW = (max1 + 1) - min1;
	unsigned gridH = (max0 + 1) - min0;
	unsigned count = gridW * gridH;
	unsigned maxWH = 32 * 32;
	if (fill == GL_GREEN) {
		maxWH = 64 * 64;
	} else {
		if (fill == GL_ALPHA) {
			maxWH = 256 * 256;
		}
	}
	if (count > 0 && count <= maxWH) {
		BC3Block* const blocks = new BC3Block[count];
		BC3Block* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				if (fill == GL_ALPHA) {
					// Alpha block with endpoints
					fillBC4Block(gridY, gridX, &next->alpha);
					// Colour block set to white
					fillBC1Block(0xFFFF, 0xFFFF, GL_RGB, &next->color);
				} else {
					// Alpha block set to solid
					fillBC4Block(0xFF, 0xFF, &next->alpha);
					// Colour block with endpoints
					fillBC1Block(gridY, gridX, fill, &next->color);
				}
				next++;
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BC3Block)), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}

unsigned createBC4(GLuint const txId, unsigned const min0, unsigned const max0, unsigned const min1, unsigned const max1) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	unsigned gridW = (max1 + 1) - min1;
	unsigned gridH = (max0 + 1) - min0;
	unsigned count = gridW * gridH;
	if (count > 0 && count <= 256 * 256) {
		BC4Block* const blocks = new BC4Block[count];
		BC4Block* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC4Block(gridY, gridX, next++);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RED_RGTC1,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BC4Block)), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}

unsigned createBC5(GLuint const txId, unsigned const min0, unsigned const max0, unsigned const min1, unsigned const max1, GLenum const fill) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	unsigned gridW = (max1 + 1) - min1;
	unsigned gridH = (max0 + 1) - min0;
	unsigned count = gridW * gridH;
	if (count > 0 && count <= 256 * 256) {
		BC5Block* const blocks = new BC5Block[count];
		BC5Block* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				if (fill == GL_RED) {
					// Red block to endpoints
					fillBC4Block(gridY, gridX, &next->red);
					// Zero unused green block
					memset(&next->green, 0, sizeof(BC4Block));
				} else {
					// Zero unused red block
					memset(&next->red, 0, sizeof(BC4Block));
					// Green block to endpoints
					fillBC4Block(gridY, gridX, &next->green);
				}
				next++;
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RG_RGTC2,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BC5Block)), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}
