#include "defines.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <limits>

#include "bcgen.h"
#include "debugview.h"
#include "glcommon.h"
#include "info.h"
#include "rgba.h"

/**
 * GLFW context version.
 */
ContextVersion glVers = VERSION_NONE;

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

//**************************** BC Block Generation ****************************/

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
 * Matches the GL type for a given data type, e.g. \c GL_BYTE for \c int8_t
 * (with only support for 8- and 16-bit integer types).
 *
 * \tparam T fixed-width integer type
 * \return matching GL data type
 */
template<typename T>
static GLenum getDataType() {
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

/**
 * Normalises \a val following the GL rules.
 *
 * \note Currently supporting only modern GL-style normalisation.
 *
 * \tparam T fixed-width integer type
 * \param[in] val integer value to normalise
 * \return \a val in the range of \c -1.0 to \c 1.0
 */
template<typename T>
static float normalize(T const val) {
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
 * Helper to perform the work of calculating the next sweep value.
 *
 * \tparam T data type, tested with \c uint8_t and \c uint16_t
 * \param[in] idx running pixel index (incremented each iteration in the calling code)
 * \param[in] x x-axis coordinate
 * \param[in] y y-axis coordinate
 * \param[in] comp component index, ranging from \0 to \c 3
 * \return a single component in the sweep texture pattern
 */
template<typename T>
static T calcSweepVal(unsigned const idx, unsigned const x, unsigned const y, unsigned const comp) {
	T val;
	if ((y & 1)) {
		if ((x & 1)) {
			val = static_cast<T>((~idx >> 3) + comp);
		} else {
			val = static_cast<T>(( idx >> 3) - comp);
		}
	} else {
		if ((x & 1)) {
			val = static_cast<T>(( idx >> 3) + comp);
		} else {
			val = static_cast<T>((~idx >> 3) - comp);
		}
	}
	return val;
}

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
 * \param[in] size texture dimension to use (e.g: \c 256 for BC3)
 * \return \c true if texture creation was successful and \a txId has valid content
 */
template<typename T>
bool createTestSweep(GLuint const txId, unsigned const size) {
	assert(txId && size);
	T* const pixels = new T[size * size * 4];
	unsigned idxVal = 0;
	for (unsigned y = 0; y < size; y++) {
		for (unsigned x = 0; x < size; x++) {
			for (unsigned comp = 0; comp < 4; comp++) {
				T val = calcSweepVal<T>(idxVal, x, y, comp);
				pixels[idxVal++] = val;
			}
		}
	}
	glBindTexture(GL_TEXTURE_2D, txId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, getDataType<T>(), pixels);
	filterClampBoilerplate();
	glFlush();
	delete[] pixels;
	return doesBoundTextureHaveContent();
}

/**
 * Verifies the read back of \c #createTestSweep() after copying the texture
 * content into a buffer.
 *
 * \tparam T data type, tested with \c uint8_t and \c uint16_t
 * \param[in] rgba start of the RGBA buffer containing \a size \c * \a size entries
 * \param[in] size texture dimension to use (e.g: \c 256 for BC3)
 * \return \c true if verification was successful
 */
template<typename T>
bool verifyTestSweep(RGBAf32* const _Nonnull rgba, unsigned const size) {
	assert(rgba && size);
	unsigned idxVal = 0;
	for (unsigned y = 0; y < size; y++) {
		for (unsigned x = 0; x < size; x++) {
			for (unsigned comp = 0; comp < 4; comp++) {
				T val = calcSweepVal<T>(idxVal, x, y, comp);
				float normVal = normalize<T>(val);
				float rgbaComp = (*rgba)[idxVal];
				if (normVal != rgbaComp) {
					printf("Expected %0.8f (0x%08X), found %0.8f (0x%08X) at %d,%d[%d]\n",
						normVal, val, rgbaComp, floatBits(rgbaComp), x, y, comp);
					return false;
				}
				idxVal++;
			}
		}
	}
	return true;
}

//*****************************************************************************/

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
	GLuint comp = compileShaderSource(GL_COMPUTE_SHADER, computeShaderTexture);
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
	//create4x4BC1Red(srcTxName); // TODO: re-enable

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

Program prog; /**< Simple quad drawing program. */

GLuint fbTxId = 0; /**< Texture ID backing \c fbufId (RGBA). */
GLuint fbufId = 0; /**< Framebuffer ID. */

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
	bool valid = doesBoundTextureHaveContent();
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
 * Clean-up for \c #createFramebuffer() (framebuffer and backing texture).
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
		createVertFragShaders(vertShaderTexture150, fragShaderTexture150, prog);
	} else {
		createVertFragShaders(vertShaderTexture110, fragShaderTexture110, prog);
	}

	GLuint txName = 0;
	glGenTextures(1, &txName);
	createTestSweep<uint8_t>(txName, SWEEP_BC4);
	createTexturedQuad(glVers, vaoId, vboId);
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
 * \param[in] winW window width
 * \param[in] winH window height
 * \param[in] show \c true if the window should be shown (default to hiding)
 * \return either a valid window or \c null
 */
GLFWwindow* createGlfwContext(unsigned winW, unsigned winH, bool show = false) {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	if (!show) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
		glfwWindowHint(GLFW_COCOA_GRAPHICS_SWITCHING, GLFW_FALSE);
	#endif
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
	GLFWwindow* window = glfwCreateWindow(winW, winH, "Test", NULL, NULL);
	if (!window) {
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glVers = VERSION_3_3;
		window = glfwCreateWindow(winW, winH, "Test", NULL, NULL);
		if (!window) {
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			glVers = VERSION_2_0;
			window = glfwCreateWindow(winW, winH, "Test", NULL, NULL);
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
bool drawFramebufferSweepTest(GLuint txId, RGBAf32* const _Nonnull rgba, unsigned const size) {
	assert(rgba && size);
	bool success = false;
	if (createTestSweep<T>(txId, size)) {
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
		success = verifyTestSweep<T>(rgba, size);
		if (success) {
			puts("Success!");
		}
	} else {
		puts("Failed to create sweep texture");
	}
	return success;
}

bool runFramebufferSweepTest(RGBAf32* const _Nonnull rgba, unsigned const size, const char* _Nonnull name) {
	assert(rgba && size && name);
	bool success = false;
	if (createFramebuffer(size, size)) {
		GLuint sweepTx = 0;
		glGenTextures(1, &sweepTx);
		printf("Running %s RGBA unsigned byte readback test\n", name);
		success = drawFramebufferSweepTest<uint8_t>(sweepTx, rgba, size);
		if (success) {
			printf("Running %s RGBA unsigned short readback test\n", name);
			success = drawFramebufferSweepTest<uint16_t>(sweepTx, rgba, size);
		}
		glDeleteTextures(1, &sweepTx);
		deleteFramebuffer();
	}
	return success;
}

void runValidateFramebuffer(GLFWwindow* /*window*/) {
	// Common shaders and fullscreen quad
	if (glVers > VERSION_2_0) {
		createVertFragShaders(vertShaderTexture150, fragShaderTexture150, prog);
	} else {
		createVertFragShaders(vertShaderTexture110, fragShaderTexture110, prog);
	}
	createTexturedQuad(glVers, vaoId, vboId);
	// Buffer large enough for all tests
	RGBAf32* const rgba = new RGBAf32[SWEEP_BC4 * SWEEP_BC4];
	runFramebufferSweepTest(rgba, SWEEP_BC1, "256x256");
	runFramebufferSweepTest(rgba, SWEEP_BC4, "1024x1024");
	delete[] rgba;
}

int main(int argc, char* argv[]) {
	enum Mode {
		MODE_USAGE,
		MODE_INFO,
		MODE_VALIDATE,
		MODE_GENERATE,
		MODE_DEBUG_VIEW,
	} mode = MODE_USAGE;
#ifndef NDEBUG
	mode = MODE_DEBUG_VIEW;
#endif
	for (int n = 1; n < argc; n++) {
		if (strcmp("--mode", argv[n]) == 0) {
			if (n + 1 < argc) {
				switch (argv[n + 1][0]) {
				case 'i':
					mode = MODE_INFO;
					break;
				case 'v':
					mode = MODE_VALIDATE;
					break;
				case 'g':
					mode = MODE_GENERATE;
					break;
				case 'd':
					mode = MODE_DEBUG_VIEW;
					break;
				default:
					mode = MODE_USAGE;
					break;
				}
			}
		} else {
			if (strcmp("--framebuffer", argv[n]) == 0) {
				puts("Using framebuffer mode");
			}
		}
	}

	GLFWwindow* window = createGlfwContext(SWEEP_BC4, SWEEP_BC4, mode == MODE_DEBUG_VIEW);
	switch (mode) {
	case MODE_INFO:
		showInfo(glVers);
		break;
	case MODE_VALIDATE:
		runValidateFramebuffer(window);
		break;
	case MODE_DEBUG_VIEW:
		showDebugView(window, glVers);
		break;
	default:
		showUsage((argc > 0) ? argv[0] : NULL);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
