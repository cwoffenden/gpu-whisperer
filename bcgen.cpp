#include "bcgen.h"

#include <cassert>
#include <new>

#include "glcommon.h"

/**
 * \c BCBlock internal union for 16-bit colour endpoints.
 */
union RGB565 {
	uint16_t rgb565;
	struct {
		uint16_t b: 5;
		uint16_t g: 6;
		uint16_t r: 5;
	};
};

static_assert(sizeof(RGB565) == 2, "RGB565 should be 2 bytes");

struct BCBlock {
	union {
		uint8_t raw[8];
		struct {
			RGB565 endpt[2];
			union {
				uint32_t texels;
				struct {
					uint8_t x0: 2;
					uint8_t x1: 2;
					uint8_t x2: 2;
					uint8_t x3: 2;
				} y[4];
			};
		} bc1;
		union {
			struct {
				uint64_t       : 16;
				uint64_t texels: 48;
			};
			struct {
				uint8_t endpt[2];
				struct __attribute__((packed)) {
					uint32_t y0x0: 3;
					uint32_t y0x1: 3;
					uint32_t y0x2: 3;
					uint32_t y0x3: 3;

					uint32_t y1x0: 3;
					uint32_t y1x1: 3;
					uint32_t y1x2: 3;
					uint32_t y1x3: 3;

					uint32_t y2x0: 3;
					uint32_t y2x1: 3;
					uint32_t y2x2: 3;
					uint32_t y2x3: 3;

					uint32_t y3x0: 3;
					uint32_t y3x1: 3;
					uint32_t y3x2: 3;
					uint32_t y3x3: 3;
				};
			};
		} bc4;
		uint64_t data;
	};
	BCBlock() {}
	BCBlock(int raw0, int raw1, int raw2, int raw3, int raw4, int raw5, int raw6, int raw7) {
		raw[0] = raw0;
		raw[1] = raw1;
		raw[2] = raw2;
		raw[3] = raw3;
		raw[4] = raw4;
		raw[5] = raw5;
		raw[6] = raw6;
		raw[7] = raw7;
	}
};

static_assert(sizeof(BCBlock) == 8, "BC block should be 8 bytes");

/**
 * Internal helper to perform the work of filling a BC1 or BC3 colour block.
 * The layout is the same, the decoder type determines the result.
 *
 * \note The texel pattern puts the four derived values into the first row.
 *
 * \param[out] block address of the block to fill
 * \param[in] val0 first endpoint (the target colour determined by \a fill)
 * \param[in] val1 second endpoint (the target colour determined by \a fill)
 * \param[in] fill choice of \c GL_RED, \c GL_GREEN or \c GL_BLUE channel (or \c GL_RGB for all)
 */
void fillBC1Block(BCBlock* _Nonnull block, unsigned val0, unsigned val1, GLenum fill) {
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
 * \param[out] block address of the block to fill
 * \param[in] val0 first endpoint
 * \param[in] val1 second endpoint
 */
void fillBC4Block(BCBlock* _Nonnull block, unsigned val0, unsigned val1) {
	assert(block);
	new(block) BCBlock(
		0x00, 0x00,
		0x88, 0xC6, 0xFA, // 01234567
		0x77, 0x39, 0x05  // 76543210
	);
	block->bc4.endpt[0] = val0;
	block->bc4.endpt[1] = val1;
}

unsigned createBC1(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, GLenum fill) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	assert(fill >= GL_RED && fill <= GL_BLUE);
	GLsizei gridW = (max1 + 1) - min1;
	GLsizei gridH = (max0 + 1) - min0;
	GLsizei count = gridW * gridH;
	GLsizei maxWH = 32 * 32;
	if (fill == GL_GREEN) {
		maxWH = 64 * 64;
	}
	if (count > 0 && count <= maxWH) {
		BCBlock* const blocks = new BCBlock[count];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC1Block(next++, gridY, gridX, fill);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			gridW * 4, gridH * 4, 0,
				count * sizeof(BCBlock), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}

unsigned createBC3(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, GLenum fill) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	assert(fill >= GL_RED && fill <= GL_ALPHA);
	GLsizei gridW = (max1 + 1) - min1;
	GLsizei gridH = (max0 + 1) - min0;
	GLsizei count = gridW * gridH;
	GLsizei maxWH = 32 * 32;
	if (fill == GL_GREEN) {
		maxWH = 64 * 64;
	} else {
		if (fill == GL_ALPHA) {
			maxWH = 256 * 256;
		}
	}
	if (count > 0 && count <= maxWH) {
		BCBlock* const blocks = new BCBlock[count * 2];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				if (fill == GL_ALPHA) {
					// Alpha block with endpoints
					fillBC4Block(next++, gridY, gridX);
					// Colour block set to white
					fillBC1Block(next++, 0xFFFF, 0xFFFF, GL_RGB);
				} else {
					// Alpha block set to solid
					fillBC4Block(next++, 0xFF, 0xFF);
					// Colour block with endpoints
					fillBC1Block(next++, gridY, gridX, fill);
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
			gridW * 4, gridH * 4, 0,
				count * sizeof(BCBlock) * 2 /* colour + alpha */, blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}

unsigned createBC4(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	GLsizei gridW = (max1 + 1) - min1;
	GLsizei gridH = (max0 + 1) - min0;
	GLsizei count = gridW * gridH;
	if (count > 0 && count <= 256 * 256) {
		BCBlock* const blocks = new BCBlock[count];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC4Block(next++, gridY, gridX);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RED_RGTC1,
			gridW * 4, gridH * 4, 0,
				count * sizeof(BCBlock), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isBoundTextureCompressed()) {
			return count;
		}
	}
	return 0;
}
