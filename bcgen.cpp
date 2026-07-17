#include "bcgen.h"

#include <cassert>
#include <new>

#include "glcommon.h"

/**
 * \c BCBlock internal union for 16-bit colour endpoints.
 */
union RGB565 {
	uint16_t rgb565;   /**< Combined 16-bit RGB value. */
	struct {
		uint16_t b: 5; /**< Blue value. */
		uint16_t g: 6; /**< Green value. */
		uint16_t r: 5; /**< Red value. */
	};
};

static_assert(sizeof(RGB565) == 2, "RGB565 should be 2 bytes");

/**
 * Basic container for a compressed 4x4 block, covering BC1 and BC4 (recalling
 * that a BC3 block is composed of a BC4 followed by BC1, and BC5 is two BC4s).
 */
struct BCBlock {
	union {
		/**
		 * Raw byte data.
		 */
		uint8_t raw[8];
		/**
		 * The block's data viewed as BC1.
		 */
		struct {
			/**
			 * The two BC1 RGB endpoints.
			 */
			RGB565 endpt[2];
			union {
				/**
				 * Raw BC1 texel data.
				 */
				uint32_t __unused texels;
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
		} bc1;
		/**
		 * The block's data viewed as BC4.
		 */
		union {
			struct {
				uint64_t /* endpt[2] */ : 16;
				/**
				 * Raw BC4 texel data.
				 */
				uint64_t __unused texels: 48;
			};
			struct {
				/**
				 * The two BC4 red-channel endpoints.
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
		} bc4;
		/**
		 * The block as a single 128-bit value.
		 */
		uint64_t __unused data;
	};
	BCBlock() {}
	BCBlock(uint8_t raw0, uint8_t raw1, uint8_t raw2, uint8_t raw3,
			uint8_t raw4, uint8_t raw5, uint8_t raw6, uint8_t raw7)
#if __cplusplus >= 201103L
		: raw { raw0, raw1, raw2, raw3, raw4, raw5, raw6, raw7 } {}
#else
	{
		raw[0] = raw0; raw[1] = raw1; raw[2] = raw2; raw[3] = raw3;
		raw[4] = raw4; raw[5] = raw5; raw[6] = raw6; raw[7] = raw7;
	}
#endif
};

static_assert(sizeof(BCBlock) == 8, "BC block should be 8 bytes");

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
static void fillBC1Block(unsigned const val0, unsigned const val1, GLenum const fill, BCBlock* _Nonnull const block) {
	assert(block);
	new(block) BCBlock(
		0x00, 0x00,
		0x00, 0x00,
		0xE4, 0x39, // 0123 | 1230
		0x4E, 0x93  // 2301 | 3012
	);
	RGB565* endpt = block->bc1.endpt;
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
static void fillBC4Block(unsigned const val0, unsigned const val1, BCBlock* _Nonnull const block) {
	assert(block);
	new(block) BCBlock(
		0x00, 0x00,
		0x88, 0xC6, 0xFA, // 01234567
		0x77, 0x39, 0x05  // 76543210
	);
	block->bc4.endpt[0] = val0;
	block->bc4.endpt[1] = val1;
}

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
		BCBlock* const blocks = new BCBlock[count];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC1Block(gridY, gridX, fill, next++);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BCBlock)), blocks);
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
		// The '* 2' scattered being BC4 + BC3
		BCBlock* const blocks = new BCBlock[count * 2];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				if (fill == GL_ALPHA) {
					// Alpha block with endpoints
					fillBC4Block(gridY, gridX, next++);
					// Colour block set to white
					fillBC1Block(0xFFFF, 0xFFFF, GL_RGB, next++);
				} else {
					// Alpha block set to solid
					fillBC4Block(0xFF, 0xFF, next++);
					// Colour block with endpoints
					fillBC1Block(gridY, gridX, fill, next++);
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BCBlock) * 2), blocks);
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
		BCBlock* const blocks = new BCBlock[count];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC4Block(gridY, gridX, next++);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RED_RGTC1,
			GLsizei(gridW * 4), GLsizei(gridH * 4), 0,
				GLsizei(count * sizeof(BCBlock)), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}
