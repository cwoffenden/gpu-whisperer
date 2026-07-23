#include "debugview.h"

#include <cassert>
#include <cstdio>

#include "bcgen.h"
#include "unsupportedimage.h"

namespace /*anonymous*/ {
/**
 * Choice of debug textures to cycle through, potentially showing gaps in format
 * support (and memory layout).
 */
enum DebugTexture {
	DEBUG_UNSUPPORTED = 0,  /**< Placeholder texture to show if generation fails. */
	DEBUG_4x4_RGBx4_RED,    /**< 4x4 8-bit RGB texture with a red-only \c TABLE_4 pattern. */
	DEBUG_4x4_BC1_RED,      /**< BC1 approximating <tt>DEBUG_4x4_RGBx4_RED</tt>. */
	DEBUG_4x4_BC3_RED,      /**< BC3 approximating <tt>DEBUG_4x4_RGBx4_RED</tt>. */
	DEBUG_4x4_RGBx8_RED,    /**< 4x4 8-bit RGB texture with a red-only \c TABLE_8 pattern. */
	DEBUG_4x4_REDx8_RED,    /**< 4x4 8-bit red-only texture with a \c TABLE_8 pattern. */
	DEBUG_4x4_RGx8_RED,     /**< 4x4 8-bit red/green texture with a red-only \c TABLE_8 pattern. */
	DEBUG_4x4_BC4_RED,      /**< BC4 approximating <tt>DEBUG_4x4_RGBx8_RED</tt>. */
	DEBUG_4x4_BC5_RED,      /**< BC5 approximating <tt>DEBUG_4x4_RGBx8_RED</tt>. */
	DEBUG_4x4_RGBx4_GREEN,  /**< 4x4 8-bit RGB texture with a green-only \c TABLE_4 pattern. */
	DEBUG_4x4_BC1_GREEN,    /**< BC1 approximating <tt>DEBUG_4x4_RGBx4_GREEN</tt>. */
	DEBUG_4x4_BC3_GREEN,    /**< BC3 approximating <tt>DEBUG_4x4_RGBx4_GREEN</tt>. */
	DEBUG_4x4_RGBx8_GREEN,  /**< 4x4 8-bit RGB texture with a green-only \c TABLE_8 pattern. */
	DEBUG_4x4_RGx8_GREEN,   /**< 4x4 8-bit red/green texture with a green-only \c TABLE_8 pattern. */
	DEBUG_4x4_BC5_GREEN,    /**< BC5 approximating <tt>DEBUG_4x4_RGBx8_GREEN</tt>. */
	DEBUG_4x4_RGBx4_BLUE,   /**< 4x4 8-bit RGB texture with a blue-only \c TABLE_4 pattern. */
	DEBUG_4x4_BC1_BLUE,     /**< BC1 approximating <tt>DEBUG_4x4_RGBx4_BLUE</tt>. */
	DEBUG_4x4_BC3_BLUE,     /**< BC1 approximating <tt>DEBUG_4x4_RGBx4_BLUE</tt>. */
	DEBUG_4x4_RGBAx8_LUMA,  /**< 4x4 8-bit RGBA texture with a luminance-only \c TABLE_8 pattern. */
	DEBUG_4x4_LUMAx8_LUMA,  /**< 4x4 8-bit luminance-only texture with a \c TABLE_8 pattern. */
	DEBUG_4x4_LAx8_LUMA,    /**< 4x4 8-bit luminance/alpha texture with a luminance-only \c TABLE_8 pattern. */
	DEBUG_4x4_LATC1_LUMA,   /**< LATC1 approximating <tt>DEBUG_4x4_RGBAx8_LUMA</tt>. */
	DEBUG_4x4_LATC2_LUMA,   /**< LATC2 approximating <tt>DEBUG_4x4_RGBAx8_LUMA</tt>. */
	DEBUG_4x4_3DC_LUMA,     /**< 3DC approximating <tt>DEBUG_4x4_RGBAx8_LUMA</tt>. */
	DEBUG_4x4_RGBAx8_ALPHA, /**< 4x4 8-bit RGBA texture with an alpha-only \c TABLE_8 pattern. */
	DEBUG_4x4_LAx8_ALPHA,   /**< 4x4 8-bit luminance/alpha texture with an alpha-only \c TABLE_8 pattern. */
	DEBUG_4x4_BC3_ALPHA,    /**< BC3 approximating <tt>DEBUG_4x4_RGBAx8_ALPHA</tt>. */
	DEBUG_4x4_LATC2_ALPHA,  /**< LATC2 approximating <tt>DEBUG_4x4_RGBAx8_ALPHA</tt>. */
	DEBUG_4x4_3DC_ALPHA,    /**< 3DC approximating <tt>DEBUG_4x4_RGBAx8_ALPHA</tt>. */
	DEBUG_TEXTURE_COUNT     /**< Number of debug textures. */
};

/**
 * Known 4x4 pattern for BC1 and similar 4-value encodings.
 */
uint8_t const TABLE_4[4 * 4] = {
	0xFF, 0x00, 0xAA, 0x55,
	0x00, 0xAA, 0x55, 0xFF,
	0xAA, 0x55, 0xFF, 0x00,
	0x55, 0xFF, 0x00, 0xAA,
};

/**
 * Known 4x4 pattern for BC1 and similar 8-value encodings.
 */
uint8_t const TABLE_8[4 * 4] = {
	0xFF, 0x00, 0xDB, 0xB6,
	0x92, 0x6D, 0x49, 0x24,
	0x24, 0x49, 0x6D, 0x92,
	0xB6, 0xDB, 0x00, 0xFF,
};

/**
 * Returns a descriptive string for a debug texture.
 *
 * \param type one of the \c DebugTexture entries
 * \return a descriptive string
 */
const char* getDebugTextureInfo(unsigned const type) {
	switch (type) {
	case DEBUG_UNSUPPORTED:
		return "Unsupported";
	case DEBUG_4x4_RGBx4_RED:
		return "RGB red, 4-shades (BC3-style)";
	case DEBUG_4x4_BC1_RED:
		return "BC1 red";
	case DEBUG_4x4_BC3_RED:
		return "BC3 red";
	case DEBUG_4x4_RGBx8_RED:
		return "RGB red, 8-shades (BC4-style)";
	case DEBUG_4x4_REDx8_RED:
		return "Red-only, 8-shades (BC4-style)";
	case DEBUG_4x4_RGx8_RED:
		return "RG red, 8-shades (BC4-style)";
	case DEBUG_4x4_BC4_RED:
		return "BC4 red";
	case DEBUG_4x4_BC5_RED:
		return "BC5 red";
	case DEBUG_4x4_RGBx4_GREEN:
		return "RGB green, 4-shades (BC3-style)";
	case DEBUG_4x4_BC1_GREEN:
		return "BC1 green";
	case DEBUG_4x4_BC3_GREEN:
		return "BC3 green";
	case DEBUG_4x4_RGBx8_GREEN:
		return "RGB green, 8-shades (BC4-style)";
	case DEBUG_4x4_RGx8_GREEN:
		return "RG green, 8-shades (BC4-style)";
	case DEBUG_4x4_BC5_GREEN:
		return "BC5 green";
	case DEBUG_4x4_RGBx4_BLUE:
		return "RGB blue, 4-shades (BC3-style)";
	case DEBUG_4x4_BC1_BLUE:
		return "BC1 blue";
	case DEBUG_4x4_BC3_BLUE:
		return "BC3 blue";
	case DEBUG_4x4_RGBAx8_LUMA:
		return "RGBA grey, 8-shades (LATC-style)";
	case DEBUG_4x4_LUMAx8_LUMA:
		return "Luminance, 8-shades (LATC-style)";
	case DEBUG_4x4_LAx8_LUMA:
		 return "Luminance/alpha grey, 8-shades (LATC-style)";
	case DEBUG_4x4_LATC1_LUMA:
		return "LATC1 luminance";
	case DEBUG_4x4_LATC2_LUMA:
		return "LATC2 luminance";
	case DEBUG_4x4_3DC_LUMA:
		return "3DC luminance";
	case DEBUG_4x4_RGBAx8_ALPHA:
		return "RGBA alpha, 8-shades (BC3-style)";
	case DEBUG_4x4_LAx8_ALPHA:
		return "Luminance/alpha alpha, 8-shades (LATC-style)";
	case DEBUG_4x4_BC3_ALPHA:
		return "BC3 alpha";
	case DEBUG_4x4_LATC2_ALPHA:
		return "LATC2 alpha";
	case DEBUG_4x4_3DC_ALPHA:
		return "3DC alpha";
	default:
		return "Unknown";
	}
}

/**
 * Debug texture IDs.
 */
GLuint texture[DEBUG_TEXTURE_COUNT] = {};

//******************************** Test Blocks ********************************/

/**
 * Creates a 4x4 uncompressed 8-bit RGB texture with ideal known BC3 values.
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] fill which colour channel to use (e.g.: <tt>GL_RED</tt>)
 */
void create4x4BC3Vals(GLuint const txId, GLenum const fill) {
	assert(txId);
	assert(fill >= GL_RED && fill <= GL_BLUE);
	glBindTexture(GL_TEXTURE_2D, txId);
	uint8_t const block[4 * 4 * 3 + 2] = {
		0x00, 0x00, // Extra two bytes to offset red and green
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x55, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x55, 0x00, 0x00, 0xFF, 0x00, 0x00,
		0xAA, 0x00, 0x00, 0x55, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x55, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0x00, 0x00
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, block + (GL_BLUE - fill));
	filterClampBoilerplate();
}

/**
 * Creates a 4x4 uncompressed 8-bit RGB texture with ideal known BC4 values.
 *
 * \note This will create a \c GL_BLUE fill but blue is not valid for BC5.
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] fill which colour channel to use (e.g.: <tt>GL_RED</tt>)
 */
void create4x4BC4Vals(GLuint const txId, GLenum const fill) {
	assert(txId);
	assert(fill >= GL_RED && fill <= GL_BLUE);
	glBindTexture(GL_TEXTURE_2D, txId);
	uint8_t const block[4 * 4 * 3 + 2] = {
		0x00, 0x00, // Extra two bytes to offset red and green
		0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDB, 0x00, 0x00, 0xB6, 0x00, 0x00,
		0x92, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x49, 0x00, 0x00, 0x24, 0x00, 0x00,
		0x24, 0x00, 0x00, 0x49, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x92, 0x00, 0x00,
		0xB6, 0x00, 0x00, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_UNSIGNED_BYTE, block + (GL_BLUE - fill));
	filterClampBoilerplate();
}

/**
 * Creates a 4x4 uncompressed 8-bit RGBA texture with ideal known LATC values.
 *
 * \param[in] txId pre-generated texture ID to use
 * \param[in] fill which channel to use (e.g.: <tt>GL_LUMINANCE</tt>)
 */
void create4x4LATCVals(GLuint const txId, GLenum const fill) {
	assert(txId);
	assert(fill == GL_LUMINANCE || fill == GL_ALPHA);
	glBindTexture(GL_TEXTURE_2D, txId);
	uint8_t block[4 * 4 * 4];
	uint8_t* next = block;
	for (unsigned n = 0; n < 16; n++) {
		if (fill == GL_LUMINANCE) {
			*next++ = TABLE_8[n];
			*next++ = TABLE_8[n];
			*next++ = TABLE_8[n];
			*next++ = 0xFF;
		} else {
			*next++ = 0x00;
			*next++ = 0x00;
			*next++ = 0x00;
			*next++ = TABLE_8[n];
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, block);
	filterClampBoilerplate();
}

/**
 * Placeholder texture to denote failure (16x16 Netscape-style broken image).
 * Unlike most of the other this is RGBA, which should ensure universal
 * compatibility.
 *
 * \param[in] txId pre-generated texture ID to use
 */
void createUnsupportedIcon(GLuint const txId) {
	assert(txId);
	glBindTexture(GL_TEXTURE_2D, txId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, UNSUPPORTED_IMAGE_RGBA);
	filterClampBoilerplate();
}

/**
 * Performs the work of creating the possible compressed and uncompressed test
 * textures, all of the \c DebugTexture options.
 */
void createDebugTextures() {
	assert(texture[0] == 0);
	glGenTextures(DEBUG_TEXTURE_COUNT, texture);
	// Unsupported placeholder
	createUnsupportedIcon(texture[DEBUG_UNSUPPORTED]);
	// Red tests
	create4x4BC3Vals(texture[DEBUG_4x4_RGBx4_RED], GL_RED);
	createBC1(texture[DEBUG_4x4_BC1_RED], 31, 31, 0, 0, GL_RED);
	createBC3(texture[DEBUG_4x4_BC3_RED], 31, 31, 0, 0, GL_RED);
	create4x4BC4Vals(texture[DEBUG_4x4_RGBx8_RED], GL_RED);
	createBC4(texture[DEBUG_4x4_BC4_RED], 255, 255, 0, 0);
	createBC5(texture[DEBUG_4x4_BC5_RED], 255, 255, 0, 0, GL_RED);
	// Green tests
	create4x4BC3Vals(texture[DEBUG_4x4_RGBx4_GREEN], GL_GREEN);
	createBC1(texture[DEBUG_4x4_BC1_GREEN], 63, 63, 0, 0, GL_GREEN);
	createBC3(texture[DEBUG_4x4_BC3_GREEN], 63, 63, 0, 0, GL_GREEN);
	create4x4BC4Vals(texture[DEBUG_4x4_RGBx8_GREEN], GL_GREEN);
	createBC5(texture[DEBUG_4x4_BC5_GREEN], 255, 255, 0, 0, GL_GREEN);
	// Blue tests
	create4x4BC3Vals(texture[DEBUG_4x4_RGBx4_BLUE], GL_BLUE);
	createBC1(texture[DEBUG_4x4_BC1_BLUE], 31, 31, 0, 0, GL_BLUE);
	createBC3(texture[DEBUG_4x4_BC3_BLUE], 31, 31, 0, 0, GL_BLUE);
	// Luma tests
	create4x4LATCVals(texture[DEBUG_4x4_RGBAx8_LUMA], GL_LUMINANCE);
	createBC4(texture[DEBUG_4x4_LATC1_LUMA], 255, 255, 0, 0, GL_COMPRESSED_LUMINANCE_LATC1_EXT);
	createBC5(texture[DEBUG_4x4_LATC2_LUMA], 255, 255, 0, 0, GL_LUMINANCE, GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT);
	// Alpha tests
	create4x4LATCVals(texture[DEBUG_4x4_RGBAx8_ALPHA], GL_ALPHA);
	createBC3(texture[DEBUG_4x4_BC3_ALPHA], 255, 255, 0, 0, GL_ALPHA);
	createBC5(texture[DEBUG_4x4_LATC2_ALPHA], 255, 255, 0, 0, GL_ALPHA, GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT);
}

/**
 * Cleans up the generated debug textures.
 */
void deleteDebugTextures() {
	assert(texture[0] != 0);
	glDeleteTextures(DEBUG_TEXTURE_COUNT, texture);
	for (unsigned n = 0; n < DEBUG_TEXTURE_COUNT; n++) {
		texture[n] = 0;
	}
}
}

//*****************************************************************************/

void showDebugView(GLFWwindow* const window, ContextVersion const glVers) {
	Program prog;
	if (glVers > VERSION_2_0) {
		createVertFragShaders(vertShaderTexture150, fragShaderTexture150, prog);
	} else {
		createVertFragShaders(vertShaderTexture110, fragShaderTexture110, prog);
	}
	GLuint vaoId = 0;
	GLuint vboId = 0;
	createTexturedQuad(glVers, vaoId, vboId, true);
	createDebugTextures();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	unsigned showingTxIdx = DEBUG_TEXTURE_COUNT;
	unsigned showingTicks = 0;
	while (!glfwWindowShouldClose(window)) {
		int fbW, fbH;
		glfwGetFramebufferSize(window, &fbW, &fbH);
		glViewport(0, 0, fbW, fbH);
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		if (showingTicks == 0) {
			showingTicks = 120;
			showingTxIdx++;
			if (showingTxIdx >= DEBUG_TEXTURE_COUNT) {
				showingTxIdx  = DEBUG_4x4_RGBx4_RED;
			}
			glBindTexture(GL_TEXTURE_2D, texture[showingTxIdx]);
			const char* info = getDebugTextureInfo(showingTxIdx);
			if (doesBoundTextureHaveContent()) {
				printf("Showing texture: %s\n", info);
			} else {
				glBindTexture(GL_TEXTURE_2D, texture[DEBUG_UNSUPPORTED]);
				if (doesBoundTextureHaveContent()) {
					printf("Fallback for: %s\n", info);
				} else {
					printf("Total texture failure\n");
				}
			}
		} else {
			showingTicks--;
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDisable(GL_BLEND);
	deleteDebugTextures();
	deleteTexturedQuad(glVers, vaoId, vboId);
	deleteVertFragShaders(prog);
}
