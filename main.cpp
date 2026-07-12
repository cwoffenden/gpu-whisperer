#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <limits>

#include "glplatform.h"
#include "rgba.h"


/**
 * \def DEBUG_DRAW_QUAD
 * Define this to draw the non-compute path to screen.
 */
//#define DEBUG_DRAW_QUAD

/**
 * GL version selected to create the context.
 */
enum GLVersion {
	VERSION_NONE, /**< No valid context. */
	VERSION_2_0,  /**< Legacy GL without VAO support. */
	VERSION_3_3,  /**< Most compatible pre-compute shader GL */
	VERSION_4_3,  /**< Gl with compute shaders. */
} glVers;

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

//static_assert(sizeof(BCBlock) == 8, "BC block should be 8 bytes");

/**
 * Block of 16 float pixels arranged as 4x4.
 */
typedef RGBAf32 RGBAf4x4[4][4];

/**
 * Block of 16 16-bit integer pixels arranged as 4x4.
 */
typedef RGBAu16 RGBAs4x4[4][4];

/**
 * Block of 16 8-bit integer pixels arranged as 4x4.
 */
typedef RGBAu08 RGBAb4x4[4][4];

static uint32_t floatBits(float val) {
	union {
		float    f;
		uint32_t u;
	} bits = {
		val
	};
	return bits.u;
}

void printBits(const RGBAf4x4& block, bool eight = false, ChannelIndex ch = CHANNEL_R, bool newline = true) {
	puts("floats (bits)");
	printf("0: 0x%08X\n1: 0x%08X\n2: 0x%08X\n3: 0x%08X\n",
		floatBits(block[0][0][ch]),
		floatBits(block[0][1][ch]),
		floatBits(block[0][2][ch]),
		floatBits(block[0][3][ch]));
	if (eight) {
		printf("4: 0x%08X\n5: 0x%08X\n6: 0x%08X\n7: 0x%08X\n",
			floatBits(block[1][0][ch]),
			floatBits(block[1][1][ch]),
			floatBits(block[1][2][ch]),
			floatBits(block[1][3][ch]));
	}
	if (newline) {
		puts("");
	}
}

void printVals(const RGBAf4x4& block, bool eight = false, ChannelIndex ch = CHANNEL_R, bool newline = true) {
	puts("floats");
	printf("0: %0.8f\n1: %0.8f\n2: %0.8f\n3: %0.8f\n",
		block[0][0][ch],
		block[0][1][ch],
		block[0][2][ch],
		block[0][3][ch]);
	if (eight) {
		printf("4: %0.8f\n5: %0.8f\n6: %0.8f\n7: %0.8f\n",
			block[1][0][ch],
			block[1][1][ch],
			block[1][2][ch],
			block[1][3][ch]);
	}
	if (newline) {
		puts("");
	}
}

void printVals(const RGBAs4x4& block, bool eight = false, ChannelIndex ch = CHANNEL_R, bool newline = true) {
	puts("shorts");
	printf("0: 0x%04X\n1: 0x%04X\n2: 0x%04X\n3: 0x%04X\n",
		block[0][0][ch],
		block[0][1][ch],
		block[0][2][ch],
		block[0][3][ch]);
	if (eight) {
		printf("4: 0x%04X\n5: 0x%04X\n6: 0x%04X\n7: 0x%04X\n",
			block[1][0][ch],
			block[1][1][ch],
			block[1][2][ch],
			block[1][3][ch]);
	}
	if (newline) {
		puts("");
	}
}

void printVals(const RGBAb4x4& block, bool eight = false, ChannelIndex ch = CHANNEL_R, bool newline = true) {
	puts("bytes");
	printf("0: 0x%02X\n1: 0x%02X\n2: 0x%02X\n3: 0x%02X\n",
		block[0][0][ch],
		block[0][1][ch],
		block[0][2][ch],
		block[0][3][ch]);
	if (eight) {
		printf("4: 0x%02X\n5: 0x%02X\n6: 0x%02X\n7: 0x%02X\n",
			block[1][0][ch],
			block[1][1][ch],
			block[1][2][ch],
			block[1][3][ch]);
	}
	if (newline) {
		puts("");
	}
}

void dumpBoundChannelData(bool eight = false, ChannelIndex ch = CHANNEL_R) {
	RGBAf4x4 f32blk;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, f32blk);
	printBits(f32blk, eight, ch);
	printVals(f32blk, eight, ch);

	RGBAs4x4 u16blk;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_SHORT, u16blk);
	printVals(u16blk, eight, ch);

	RGBAb4x4 u08blk;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, u08blk);
	printVals(u08blk, eight, ch);

	assert(glGetError() == 0);
}

/**
 * Filter to nearest and clamp to edge the current bound texture.
 */
void filterClampBoilerplate() {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

#if 0
/**
 * Creates a 4x4 red-only uncompressed 8-bit texture with ideal BC3 values.
 *
 * \param[in] txId pre-generated texture ID to use
 */
void create4x4RedBC3Vals(GLuint txId) {
	assert(txId);
	glBindTexture(GL_TEXTURE_2D, txId);
	uint8_t const block[16] = {
		0xFF, 0x00, 0xAA, 0x55,
		0x00, 0xAA, 0x55, 0xFF,
		0xAA, 0x55, 0xFF, 0x00,
		0x55, 0xFF, 0x00, 0xAA
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, block);
	filterClampBoilerplate();
}

/**
 * Creates a 4x4 red-only uncompressed 8-bit texture with ideal BC4 values.
 *
 * \param[in] txId pre-generated texture ID to use
 */
void create4x4RedBC4Vals(GLuint txId) {
	assert(txId);
	glBindTexture(GL_TEXTURE_2D, txId);
	uint8_t const block[16] = {
		0xFF, 0x00, 0xDB, 0xB6,
		0x92, 0x6D, 0x49, 0x24,
		0x24, 0x49, 0x6D, 0x92,
		0xB6, 0xDB, 0x00, 0xFF
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, block);
	filterClampBoilerplate();
}
#endif

//**************************** BC Block Generation ****************************/

/**
 * Internal helper to perform the work of filling a BC1 or BC3 colour block.
 * The layout is the same, the decoder type determines the result.
 *
 * \note The texel pattern puts the four derived values into the first row.
 *
 * \param[in] block address of the block to fill
 * \param[in] val0 first endpoint
 * \param[in] val1 second endpoint
 * \param[in] channel choice of \c GL_RED, \c GL_GREEN or \c GL_BLUE channel (or \c GL_RGB for all)
 */
void fillBC1Block(BCBlock* block, unsigned val0, unsigned val1, unsigned channel = GL_RED) {
	assert(block);
	new(block) BCBlock(
		0x00, 0x00,
		0x00, 0x00,
		0xE4, 0x39, // 0123 | 1230
		0x4E, 0x93  // 2301 | 3012
	);
	RGB565* endpt = block->bc1.endpt;
	switch (channel) {
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
 * \param[in] block address of the block to fill
 * \param[in] val0 first endpoint
 * \param[in] val1 second endpoint
 */
void fillBC4Block(BCBlock* block, unsigned val0, unsigned val1) {
	assert(block);
	new(block) BCBlock(
		0x00, 0x00,
		0x88, 0xC6, 0xFA, // 01234567
		0x77, 0x39, 0x05  // 76543210
	);
	block->bc4.endpt[0] = val0;
	block->bc4.endpt[1] = val1;
}

/**
 * Tests whether the currently bound texture is hardware compressed.
 *
 * \return \c true if the texture's first mipmap level is compressed
 */
bool isCurrentBoundCompressed() {
	GLint valid = GL_FALSE;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &valid);
	return valid == GL_TRUE;
}

/**
 * Tests whether the currently bound texture has valid  component data.
 *
 * \param[in] channel which channel to query
 * \return \c true if the texture's first mipmap level has the specified component
 */
bool currentBoundHasData(GLenum channel = GL_RED) {
	GLenum chEnum;
	switch (channel) {
	case GL_RED:
		chEnum = GL_TEXTURE_RED_SIZE;
		break;
	case GL_GREEN:
		chEnum = GL_TEXTURE_GREEN_SIZE;
		break;
	case GL_BLUE:
		chEnum = GL_TEXTURE_BLUE_SIZE;
		break;
	default:
		chEnum = GL_TEXTURE_ALPHA_SIZE;
	}
	GLint size = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, chEnum, &size);
	return size > 0;
}

/**
 * Creates a BC1 texture grid with endpoints varying between the specified
 * minimum and maximum, on the selected \a channel only. Variance in the first
 * endpoint runs down the Y-axis (being stable in the X).
 *
 * \note The \a channel determines the maximum endpoint values and the eventual
 * texture size: \c GL_RED and \c GL_BLUE are \c 31, and \c GL_GREEN is \c 63
 * (with no support for the cutout alpha variant).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \param[in] channel which channel to use (e.g. \c GL_RED for the red channel)
 * \return the number of 4x4 entries
 */
unsigned createBC1(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, unsigned channel = GL_RED) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	assert(channel >= GL_RED && channel <= GL_BLUE);
	GLsizei gridW = (max1 + 1) - min1;
	GLsizei gridH = (max0 + 1) - min0;
	GLsizei count = gridW * gridH;
	GLsizei maxWH = 32 * 32;
	if (channel == GL_GREEN) {
		maxWH = 64 * 64;
	}
	if (count > 0 && count <= maxWH) {
		BCBlock* const blocks = new BCBlock[count];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				fillBC1Block(next++, gridY, gridX, channel);
			}
		}
		glBindTexture(GL_TEXTURE_2D, txId);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			gridW * 4, gridH * 4, 0,
				count * sizeof(BCBlock), blocks);
		filterClampBoilerplate();
		glFlush();
		delete[] blocks;
		if (isCurrentBoundCompressed()) {
			return count;
		}
	}
	return 0;
}

/**
 * Creates a BC3 texture grid with endpoints varying between the specified
 * minimum and maximum, on the selected \a channel only. Variance in the first
 * endpoint runs down the Y-axis (being stable in the X).
 *
 * \note The \a channel determines the maximum endpoint values and the eventual
 * texture size: \c GL_RED and \c GL_BLUE are \c 31, \c GL_GREEN is \c 63, and
 * \c GL_ALPHA is \c 255 (but note that the interpolations should have more
 * accuracy for BC4 than BC3 alpha).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \param[in] channel which channel to use (e.g. \c GL_RED for the red channel)
 * \return the number of 4x4 entries
 */
unsigned createBC3(GLuint txId, unsigned min0, unsigned max0, unsigned min1, unsigned max1, unsigned channel = GL_RED) {
	assert(txId);
	assert(min0 < 256 && max0 < 256 && min0 <= max0);
	assert(min1 < 256 && max1 < 256 && min1 <= max1);
	assert(channel >= GL_RED && channel <= GL_ALPHA);
	GLsizei gridW = (max1 + 1) - min1;
	GLsizei gridH = (max0 + 1) - min0;
	GLsizei count = gridW * gridH;
	GLsizei maxWH = 32 * 32;
	if (channel == GL_GREEN) {
		maxWH = 64 * 64;
	} else {
		if (channel == GL_ALPHA) {
			maxWH = 256 * 256;
		}
	}
	if (count > 0 && count <= maxWH) {
		BCBlock* const blocks = new BCBlock[count * 2];
		BCBlock* next = blocks;
		for(unsigned gridY = min0; gridY <= max0; gridY++) {
			for(unsigned gridX = min1; gridX <= max1; gridX++) {
				if (channel == GL_ALPHA) {
					// Alpha block with endpoints
					fillBC4Block(next++, gridY, gridX);
					// Colour block set to white
					fillBC1Block(next++, 0xFFFF, 0xFFFF, GL_RGB);
				} else {
					// Alpha block set to solid
					fillBC4Block(next++, 0xFF, 0xFF);
					// Colour block with endpoints
					fillBC1Block(next++, gridY, gridX, channel);
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
		if (isCurrentBoundCompressed()) {
			return count;
		}
	}
	return 0;
}

/**
 * Creates a red-only BC4 texture grid with endpoints varying between the
 * specified minimum and maximum. Variance in the first endpoint (\c red0 in the
 * Khronos RGTC specification) runs down the Y-axis (being stable in the X).
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] min0 minimum first endpoint
 * \param[in] max0 maximum first endpoint (inclusive)
 * \param[in] min1 minimum second endpoint
 * \param[in] max1 maximum second endpoint (inclusive)
 * \return the number of 4x4 blocks
 */
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
		if (isCurrentBoundCompressed()) {
			return count;
		}
	}
	return 0;
}

#if 0
/**
 * Creates a 4x4 compressed BC3 texture with known values for testing
 * (purposefully choosing the \e mode with a single interpolated colour).
 *
 * \param[in] txId pre-generated texture ID to use
 */
void create4x4BC1Red(GLuint txId) {
	unsigned count = createBC1(txId, 15, 15, 31, 31, GL_RED);
	assert(count);
}

/**
 * Creates a 4x4 compressed BC3 texture with known values for testing.
 *
 * \param[in] txId pre-generated texture ID to use
 */
void create4x4BC3Red(GLuint txId) {
	unsigned count = createBC3(txId, 31, 31, 0, 0, GL_RED);
	assert(count);
}

/**
 * Creates a 4x4 compressed BC4 texture with known values for testing.
 *
 * \param[in] txId pre-generated texture ID to use
 */
void create4x4BC4Red(GLuint txId) {
	unsigned count = createBC4(txId, 255, 255, 0, 0);
	assert(count);
}
#endif

//**************************** Buffer Verification ****************************/

/**
 * Matches the GL type for a given data type, e.g. \c GL_BYTE for \c int8_t
 * (with only support for 8- and 16-bit integer types).
 *
 * \tparam T fixed-width integer type
 * \return matching GL data type
 */
template<typename T>
GLenum getDataType() {
	unsigned max(std::numeric_limits<T>::max());
	if (max == INT8_MAX) {
		return GL_BYTE;
	} else {
		if (max == UINT8_MAX) {
			return GL_UNSIGNED_BYTE;
		} else {
			if (max == INT16_MAX) {
				return GL_SHORT;
			} else {
				if (max == UINT16_MAX) {
					return GL_UNSIGNED_SHORT;
				}
			}
		}
	}
	return GL_NONE;
}

/*
 * \note Currently supporting only modern GL-style normalisation.
 * \tparam T fixed-width integer type
 */
template<typename T>
float normalize(T val) {
	unsigned max(std::numeric_limits<T>::max());
	if (max == INT8_MAX) {
		return std::max(val / float(INT8_MAX), -1.0f);
	} else {
		if (max == UINT8_MAX) {
			return val / float(UINT8_MAX);
		} else {
			if (max == INT16_MAX) {
				return std::max(val / float(INT16_MAX), -1.0f);
			} else {
				if (max == UINT16_MAX) {
					return val / float(UINT16_MAX);
				}
			}
		}
	}
	return 0.0f;
}

/**
 * \def SWEEP_BC3
 * Test texture size large enough to extract all BC1 and BC3 variations.
 */
#define SWEEP_BC1 256

/**
 * \def SWEEP_BC3
 * Test texture size large enough to extract all BC4 variations.
 */
#define SWEEP_BC4 1024

/**
 * Creates a test texture with a known pattern sweeping the range of possible
 * values for the storage type, which should result in exact float values that
 * can be verified. Any discrepancy between the known and expected values means
 * the extraction method cannot be used.
 *
 * \note The intended use is for 8-bit 256x256 and 16-bit 1024x1024, which
 * covers BC1, BC3 and BC4's ranges.
 *
 * \tparam T data type, tested with \c uint8_t and \c uint16_t
 * \param[in] txId pre-generated texture ID to use
 * \param[in] size texture dimension to use (e.g \c 256 for BC3)
 * \return \c true if texture creation was successful and \a txId has valid content
 */
template<typename T>
bool createTestSweepRed(GLuint txId, unsigned const size) {
	assert(txId && size);
	T* const pixels = new T[size * size];
	unsigned idxVal = 0;
	for (unsigned y = 0; y < size; y++) {
		for (unsigned x = 0; x < size; x++) {
			T val;
			if ((y & 1)) {
				val = static_cast<T>(((idxVal & 1) ? ~idxVal :  idxVal) >> 1);
			} else {
				val = static_cast<T>(((idxVal & 1) ?  idxVal : ~idxVal) >> 1);
			}
			pixels[idxVal++] = val;
		}
	}
	glBindTexture(GL_TEXTURE_2D, txId);
#ifndef GL_VERSION_3_0
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, size, size, 0, GL_LUMINANCE, getDataType<T>(), pixels);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,       size, size, 0, GL_RED,       getDataType<T>(), pixels);
#endif
	filterClampBoilerplate();
	glFlush();
	delete[] pixels;
	return true;//currentBoundHasData(GL_RED);
}

/**
 * Verifies the read back of \c #createTestSweepRed() after copying the texture
 * content into a buffer.
 *
 * \tparam T data type, tested with \c uint8_t and \c uint16_t
 * \param[in] rgba start of the RGBA buffer containing \a size \c * \a size entries
 * \param[in] size texture dimension to use (e.g \c 256 for BC3)
 * \return \c true if verification was successful
 */
template<typename T>
bool verifyTestSweepRed(RGBAf32* const rgba, unsigned const size) {
	assert(rgba && size);
	unsigned idxVal = 0;
	for (unsigned y = 0; y < size; y++) {
		for (unsigned x = 0; x < size; x++) {
			T val;
			if ((y & 1)) {
				val = static_cast<T>(((idxVal & 1) ? ~idxVal :  idxVal) >> 1);
			} else {
				val = static_cast<T>(((idxVal & 1) ?  idxVal : ~idxVal) >> 1);
			}
			float valN = normalize<T>(val);
			float redF = rgba[idxVal].r;
			if (valN != redF) {
				printf("Expected %0.8f (0x%08X) found %0.8f (0x%08X) @ %d,%d\n",
					valN, val, redF, floatBits(redF), x, y);
				return false;
			}
			idxVal++;
		}
	}
	return true;
}

#if 0
void bc3RedTest() {
	GLuint txName = 0;
	glGenTextures(1, &txName);
	create4x4BC3Red(txName);

	dumpBoundChannelData(false);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}

void bc4RedTest() {
	GLuint txName = 0;
	glGenTextures(1, &txName);
	create4x4BC3Red(txName);

	dumpBoundChannelData(true);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}

void bc3Red8ValTest() {
	GLuint txName = 0;
	glGenTextures(1, &txName);
	create4x4RedBC3Vals(txName);

	dumpBoundChannelData(true);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}

void bc4Red8ValTest() {
	GLuint txName = 0;
	glGenTextures(1, &txName);
	create4x4RedBC4Vals(txName);

	dumpBoundChannelData(true);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}
#endif

#ifdef GL_VERSION_4_3
GLchar const computeShaderTexture[] =
	"#version 430 core\n"
	"layout(binding = 0) uniform sampler2D srcTx;\n"
	"layout(binding = 1, rgba32f) uniform image2D dstTx;\n"
	"layout(local_size_x = 1, local_size_y = 1) in;\n"
	"void main() {\n"
	"	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n"
	"	vec4 rgba = texelFetch(srcTx, pos, 0);\n"
	"	imageStore(dstTx, pos, rgba);\n"
	"}\n";


GLchar const computeShaderImage[] =
	"#version 430 core\n"
	"layout(binding = 0, rgba8) readonly uniform image2D srcTx;\n"
	"layout(binding = 1, rgba32f) uniform image2D dstTx;\n"
	"layout(local_size_x = 1, local_size_y = 1) in;\n"
	"void main() {\n"
	"	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);\n"
	"	vec4 rgba = imageLoad(srcTx, pos);\n"
	"	imageStore(dstTx, pos, rgba);\n"
	"}\n";

void computeTest() {
	GLuint comp = compileShaderText(GL_COMPUTE_SHADER, computeShaderTexture);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, comp);
	glLinkProgram(prog);
	glUseProgram (prog);
	assert(glGetError() == 0);

	GLuint dstTxName = 0;
	glGenTextures(1, &dstTxName);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, dstTxName);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 4, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindImageTexture(1, dstTxName, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	assert(glGetError() == 0);

	GLuint srcTxName = 0;
	glGenTextures(1, &srcTxName);
	glActiveTexture(GL_TEXTURE0);
	create4x4BC1Red(srcTxName);

	/*
	uint8_t red4x4[16] = {0xFF, 0x00, 0xDB, 0xB6, 0x92, 0x6D, 0x49, 0x24};
	GLuint srcTxName = 0;
	glGenTextures(1, &srcTxName);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, srcTxName);
	// needs GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, red4x4);
	filterClampBoilerplate();
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindImageTexture(0, srcTxName, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8);
	 */

	assert(glGetError() == 0);
	glDispatchCompute(4, 4, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	assert(glGetError() == 0);

	glBindTexture(GL_TEXTURE_2D, dstTxName);
	dumpBoundChannelData(true);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, &srcTxName);
	glDeleteTextures(1, &dstTxName);
}
#endif

//**************************** Framebuffer Version ****************************/

/**
 * Helper to compile a shader from source.
 *
 * \param[in] type shader type
 * \param[in] text shader source
 * \return the shader ID (or zero if compilation failed)
 */
static GLuint compileShaderText(GLenum const type, const GLchar* const text) {
	if (GLuint shader = glCreateShader(type)) {
		const char* texts[1] = {text};
		glShaderSource (shader, 1, texts, NULL);
		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled == GL_TRUE) {
			return shader;
		} else {
			GLint logLen;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
			if (logLen > 1) {
				GLchar* logStr = new GLchar[logLen];
				glGetShaderInfoLog(shader, logLen, NULL, logStr);
				printf("Shader compile error: %s\n", logStr);
				delete[] logStr;
			}
			glDeleteShader(shader);
		}
	}
	return 0;
}

/**
 * Vertex attribute IDs.
 */
enum VertexID {
	VERT_POSN_ID = 0, /**< Vertex positions. */
	VERT_TEX0_ID = 1, /**< Vertex texture channel channel 0. */
	VERT_TEX1_ID = 2, /**< Vertex texture channel channel 1. */
};

/**
 * GLSL 1.20 compatible vertex shader, designed only to draw a fullscreen
 * textured quad.
 */
GLchar const vertShaderTexture120[] =
	"#version 120\n"
	"attribute vec2 aPosn;\n"
	"attribute vec2 aTex0;\n"
	"varying vec2 vTex0;\n"
	"void main() {\n"
	"	vTex0 = aTex0;\n"
	"	gl_Position = vec4(aPosn.x, aPosn.y, 0.0, 1.0);\n"
	"}\n";

/**
 * GLSL 1.50 version of the \c #vertShaderTexture120 quad shader.
 */
GLchar const vertShaderTexture150[] =
	"#version 150\n"
	"in vec2 aPosn;\n"
	"in vec2 aTex0;\n"
	"out vec2 vTex0;\n"
	"void main() {\n"
	"	vTex0 = aTex0;\n"
	"	gl_Position = vec4(aPosn.x, aPosn.y, 0.0, 1.0);\n"
	"}\n";

/**
 * GLSL 1.20 compatible fragment shader, designed only to draw a fullscreen
 * textured quad.
 *
 * /todo GL2.0 might only have 1.10 support (previously solved)
 */
GLchar const fragShaderTexture120[] =
	"#version 120\n"
	"uniform sampler2D srcTx;\n"
	"varying vec2 vTex0;\n"
	"void main() {\n"
	"	vec4 tex0rgb = texture2D(srcTx, vTex0);\n"
	"	gl_FragColor = tex0rgb;\n"
	"}\n";

/**
 * GLSL 1.50 version of the \c #fragShaderTexture120 quad shader.
 */
GLchar const fragShaderTexture150[] =
	"#version 150\n"
	"precision highp float;\n"
	"uniform sampler2D srcTx;\n"
	"in vec2 vTex0;\n"
	"out vec4 FragColor;\n"
	"void main() {\n"
	"	vec4 tex0rgb = texture(srcTx, vTex0);\n"
	"	FragColor = tex0rgb;\n"
	"}\n";

GLuint progId = 0; /**< Program ID. */
GLuint vertId = 0; /**< Vertex shader ID. */
GLuint fragId = 0; /**< Fragment shader ID. */

GLuint fbTxId = 0; /**< Texture ID backing \c fbufId (RGBA). */
GLuint fbufId = 0; /**< Framebuffer ID. */

/**
 * Helper to create vertex and fragment shaders. After calling the current
 * program is set to the compiled result.
 *
 * \param[in] vertSrc vertex shader source
 * \param[in] fragSrc fragment shader source
 * \return \c true of compilation and linking was successful
 */
bool createVertFragShaders(const GLchar* vertSrc, const GLchar* fragSrc) {
	assert(progId == 0);
	progId = glCreateProgram();
	if (progId) {
		glBindAttribLocation(progId, VERT_POSN_ID, "aPosn");
		glBindAttribLocation(progId, VERT_TEX0_ID, "aTex0");
		glBindAttribLocation(progId, VERT_TEX1_ID, "aTex1");
		vertId = compileShaderText(GL_VERTEX_SHADER,   vertSrc);
		fragId = compileShaderText(GL_FRAGMENT_SHADER, fragSrc);
		if (vertId && fragId) {
			glAttachShader(progId, vertId);
			glAttachShader(progId, fragId);
			glLinkProgram (progId);
			GLint linked;
			glGetProgramiv(progId, GL_LINK_STATUS, &linked);
			if (linked == GL_TRUE) {
				glUseProgram(progId);
				return true;
			}
		}
	}
	puts("Failed to create shaders");
	return false;
}

/**
 * Cleanup for \c #createVertFragShaders() (program and shaders).
 */
void deleteVertFragShaders() {
	glUseProgram(0);
	glDetachShader (progId, vertId);
	glDetachShader (progId, fragId);
	glDeleteProgram(progId);
	glDeleteShader (vertId);
	glDeleteShader (fragId);
	progId = 0;
	vertId = 0;
	fragId = 0;
}

/**
 * Creates a framebuffer backed by the specified texture type. After calling,
 * any valid framebuffer remains bound.
 *
 * \note Combinations that work: \c GL_RGBA32F with \c GL_FLOAT (the ideal,
 * though some hardware accepts this but the values look to read as half-float),
 * \c GL_RGBA16F with \c GL_FLOAT (the size-specific \c GL_HALF_FLOAT also
 * works with no difference), \c GL_RGBA16 with \c GL_UNSIGNED_SHORT (probably
 * the best compromise if 32-bit float fails), and the generic \c GL_RGBA with
 * types such as \c GL_FLOAT (which appears to give half-floats).
 *
 * \param[in] bufW framebuffer width
 * \param[in] bufH framebuffer height
 * \param[in] format sized texture format (e.g. \c GL_RGBA32F for 32-bit floats)
 * \param[in] type texture data type (e.g. \c GL_FLOAT matching the \c GL_RGBA32F \a format for 32-bit floats)
 * \return \c true if a valid framebuffer was created
 */
bool createFramebuffer(unsigned bufW, unsigned bufH, GLint format = GL_RGBA32F, GLenum type = GL_FLOAT) {
	assert(fbTxId == 0 && fbufId == 0);
	glGenTextures(1, &fbTxId);
	glBindTexture(GL_TEXTURE_2D, fbTxId);
	glTexImage2D(GL_TEXTURE_2D, 0, format, bufW, bufH, 0, GL_RGBA, type, NULL);
	filterClampBoilerplate();
	bool valid = currentBoundHasData();
	glBindTexture(GL_TEXTURE_2D, 0);
	if (valid) {
		glGenFramebuffers(1, &fbufId);
		glBindFramebuffer(GL_FRAMEBUFFER, fbufId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTxId, 0);
		valid = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		if (!valid) {
			puts("Unable to create framebuffer");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	} else {
		puts("Unable to create framebuffer backing texture");
	}
	return valid;
}

/**
 * Cleanup for \c #createFramebuffer() (framebuffer and backing texture).
 */
void deleteFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbufId);
	fbufId = 0;
	glDeleteTextures(1, &fbTxId);
	fbTxId = 0;
}

GLuint vaoId = 0; /**< VAO fullscreen textured quad.  */
GLuint vboId = 0; /**< VBO for \c vObjId quad. */

/**
 * Creates a fullscreen textured quad. After calling the VAO \c vaoId and/or its
 * VBO \c vboId remain bound (to ease drawing, since this is the only geometry).
 */
void createTexturedQuad() {
	assert(vaoId == 0);
	float const verts[]= {
		 1.0f,  1.0f, 1.0f, 1.0f, // TR
		-1.0f,  1.0f, 0.0f, 1.0f, // TL
		-1.0f, -1.0f, 0.0f, 0.0f, // BL

		-1.0f, -1.0f, 0.0f, 0.0f, // BL
		 1.0f, -1.0f, 1.0f, 0.0f, // BR
		 1.0f,  1.0f, 1.0f, 1.0f, // TR
	};
	/*
	 * Mac with GL2.1 and the GL3 header will fail here. Using the GL2 header
	 * with the APPLE suffix works, as will a GL3 and 4 context, but for this
	 * simple example a VAO isn't used (but it is necessary to have one bound
	 * for newer GL, otherwise the VBO fails).
	 */
#ifdef GL_VERSION_3_0
	if (glVers > VERSION_2_0) {
		glGenVertexArrays(1, &vaoId);
		glBindVertexArray(vaoId);
	}
#endif
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(VERT_POSN_ID, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(VERT_POSN_ID);
	glVertexAttribPointer(VERT_TEX0_ID, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glEnableVertexAttribArray(VERT_TEX0_ID);
	assert(glGetError() == 0);
}

void deleteTexturedQuad() {
	glDeleteBuffers(1, &vboId);
	vaoId = 0;
#ifdef GL_VERSION_3_0
	if (glVers > VERSION_2_0) {
		glDeleteVertexArrays(1, &vaoId);
		vboId = 0;
	}
#endif
}

void initFramebufferTest() {
#ifndef DEBUG_DRAW_QUAD
	createFramebuffer(SWEEP_BC1, SWEEP_BC1);
#endif
	if (glVers > VERSION_2_0) {
		createVertFragShaders(vertShaderTexture150, fragShaderTexture150);
	} else {
		createVertFragShaders(vertShaderTexture120, fragShaderTexture120);
	}

	GLuint txName = 0;
	glGenTextures(1, &txName);
	//create4x4BC1Red(txName);
	createTestSweepRed<uint8_t>(txName, SWEEP_BC1);

	createTexturedQuad();
}

void drawFramebufferTest() {
	assert(vboId);
#ifndef DEBUG_DRAW_QUAD
	glBindFramebuffer(GL_FRAMEBUFFER, fbufId);
	glViewport(0, 0, SWEEP_BC1, SWEEP_BC1);
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
	/*
	 * The quad's VAO, if used, or its VBO remain bound.
	 */
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glFinish();
	assert(glGetError() == 0);
}

void setup() {
	/*
	 * Note so far: these pure 'val' tests are wrong, compared with the control
	 * values, but the float framebuffer ones are correct. They're only slightly
	 * out, but the takeaway is glGetTexImage() from a bound texture gives
	 * different results than drawing to a float framebuffer (which matches
	 * compute shader).
	 */
	//bc3RedTest();
	//bc4RedTest();
	//bc4Red8ValTest();
	//computeTest();
	initFramebufferTest();
	drawFramebufferTest();
	/*
	printf("Control 0xAA: 0x%08X (%0.8f)\n", floatBits(0xAA / 255.0f), 0xAA / 255.0f);
	printf("Control 0x55: 0x%08X (%0.8f)\n", floatBits(0x55 / 255.0f), 0x55 / 255.0f);
	 */
	/*
	printf("Control 0xDB: 0x%08X (%0.8f)\n", floatBits(0xDB / 255.0f), 0xDB / 255.0f);
	printf("Control 0xB6: 0x%08X (%0.8f)\n", floatBits(0xB6 / 255.0f), 0xB6 / 255.0f);
	printf("Control 0x92: 0x%08X (%0.8f)\n", floatBits(0x92 / 255.0f), 0x92 / 255.0f);
	printf("Control 0x6D: 0x%08X (%0.8f)\n", floatBits(0x6D / 255.0f), 0x6D / 255.0f);
	printf("Control 0x49: 0x%08X (%0.8f)\n", floatBits(0x49 / 255.0f), 0x49 / 255.0f);
	printf("Control 0x24: 0x%08X (%0.8f)\n", floatBits(0xB6 / 255.0f), 0x24 / 255.0f);
	 */
}

void draw(GLFWwindow* window) {
	int fbW, fbH;
	glfwGetFramebufferSize(window, &fbW, &fbH);
	glViewport(0, 0, fbW, fbH);
	glClearColor(0.3f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	drawFramebufferTest();
}

#ifdef GL_VERSION_4_3
void APIENTRY debugCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum /*severity*/, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
	printf("GL debug: %s\n", message);
}
#endif

/**
 * Helper to create a GLFW window, and therefore a GL context, with the highest
 * GL version possible (setting the global \c glVers in the process). If no
 * context can be created then the application exits.
 *
 * \param[in] show \c true if the window should be shown (default to hiding)
 * \return either a valid window or \c null
 */
GLFWwindow* createGlfwContext(bool show = false) {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	if (!show) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}
	/*
	 * We try to create a compute shader compatible context, with retries for
	 * various other older GLs. Macs are unhappy with GLFW_OPENGL_PROFILE and
	 * GLFW_OPENGL_FORWARD_COMPAT, so we avoid any other hints.
	 *
	 * 4.3 is the minimum for a compute shader (4.1 the maximum for Mac), 3.3
	 * for the fallback with a framebuffer, 2.0 the oldest supported.
	 */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glVers = VERSION_4_3;
	GLFWwindow* window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
	if (!window) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 5);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glVers = VERSION_3_3;
		window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
		if (!window) {
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			glVers = VERSION_2_0;
			window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
			if (!window) {
				puts("Unable to create a GL context");
				exit(EXIT_FAILURE);
			}
		}
	}
	glfwMakeContextCurrent(window);
#ifdef GL_VERSION_4_3
	glDebugMessageCallback(debugCallback, NULL);
	glEnable(GL_DEBUG_OUTPUT);
#endif
	return window;
}

template<typename T>
void runSweepTestRed(GLuint txId, RGBAf32* const rgba, unsigned const size) {
	if (createTestSweepRed<uint8_t>(txId, size)) {
		glViewport(0, 0, size, size);
		glClear(GL_COLOR_BUFFER_BIT);
		/*
		 * The quad's VAO, if used, or its VBO remain bound.
		 */
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glFinish();
		assert(glGetError() == 0);

		glBindTexture(GL_TEXTURE_2D, fbTxId);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, rgba);
		if (verifyTestSweepRed<uint8_t>(rgba, size)) {
			puts("Success!");
		} else {
			puts("Failed...");
		}
	} else {
		puts("Failed to create test sweep");
	}
}

void runValidateFramebuffer(GLFWwindow* /*window*/) {
	// Common shaders and fullscreen quad
	if (glVers > VERSION_2_0) {
		createVertFragShaders(vertShaderTexture150, fragShaderTexture150);
	} else {
		createVertFragShaders(vertShaderTexture120, fragShaderTexture120);
	}
	createTexturedQuad();
	// Buffer large enough for all tests
	RGBAf32* const rgba = new RGBAf32[SWEEP_BC4 * SWEEP_BC4];

	if (createFramebuffer(SWEEP_BC1, SWEEP_BC1)) {
		GLuint sweepTx = 0;
		glGenTextures(1, &sweepTx);
		runSweepTestRed<uint8_t>(sweepTx, rgba, SWEEP_BC1);
		runSweepTestRed<uint16_t>(sweepTx, rgba, SWEEP_BC1);
	}
	delete[] rgba;
}

int main(int argc, char* argv[]) {
	enum Mode {
		MODE_INFO,
		MODE_VALIDATE,
		MODE_GENERATE,
	} mode = MODE_VALIDATE;
	for (int n = 1; n < argc; n++) {
		switch (argv[n][0]) {
		case 'v':
			mode = MODE_VALIDATE;
			break;
		case 'g':
			mode = MODE_GENERATE;
			break;
		default:
			mode = MODE_INFO;
		}
	}
#ifdef DEBUG_DRAW_QUAD
	GLFWwindow* window = createGlfwContext(true);
#else
	GLFWwindow* window = createGlfwContext();
#endif
	if (mode == MODE_VALIDATE) {
		runValidateFramebuffer(window);
	}
	/*
	setup();

#ifdef DEBUG_DRAW_QUAD
	while (!glfwWindowShouldClose(window)) {
		draw(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
#endif
	 */
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
