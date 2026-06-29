#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "rgba.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

struct BCBlock {
	union {
		uint8_t raw[8];
		struct {
			union {
				uint16_t rgb565;
				struct {
					uint16_t b: 5;
					uint16_t g: 6;
					uint16_t r: 5;
				};
			} endpt[2];
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
	BCBlock() : data(0) {}
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

void bc3RedTest() {
	BCBlock block[2] = {{/*unused alpha*/}, {0x00, 0x00, 0x00, 0x00, 0xE4, 0x39, 0x4E, 0x93}};
	block[1].bc1.endpt[0].r = 31;
	block[1].bc1.endpt[1].r =  0;

	GLuint txName = 0;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 4, 4, 0, 16, block);
	filterClampBoilerplate();

	dumpBoundChannelData(false);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}

void bc4RedTest() {
	BCBlock block[1] = {{0x00, 0x00, 0x88, 0xC6, 0xFA, 0x77, 0x39, 0x05}};
	block[0].bc4.endpt[0] = 255;
	block[0].bc4.endpt[1] =   0;

	GLuint txName = 0;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RED_RGTC1, 4, 4, 0, 8, block);
	filterClampBoilerplate();
	
	dumpBoundChannelData(true);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}

void red8Test() {
	uint8_t const block[16] = {0xFF, 0x00, 0xDB, 0xB6, 0x92, 0x6D, 0x49, 0x24};

	GLuint txName = 0;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, block);
	filterClampBoilerplate();

	dumpBoundChannelData(true);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &txName);
}

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
		glShaderSource (shader, 1, texts, nullptr);
		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled) {
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

	BCBlock block[2] = {{}, {0x00, 0x00, 0x00, 0x00, 0xE4, 0x39, 0x4E, 0x93}};
	block[1].bc1.endpt[0].r = 31;
	block[1].bc1.endpt[1].r =  0;

	GLuint srcTxName = 0;
	glGenTextures(1, &srcTxName);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, srcTxName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 4, 4, 0, 16, block);
	filterClampBoilerplate();

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
	dumpBoundChannelData();
	glBindTexture(GL_TEXTURE_2D, 0);

	glDeleteTextures(1, &srcTxName);
	glDeleteTextures(1, &dstTxName);

	printf("Control 0xAA: 0x%08X (%0.8f)\n", floatBits(0xAA / 255.0f), 0xAA / 255.0f);
	printf("Control 0x55: 0x%08X (%0.8f)\n", floatBits(0x55 / 255.0f), 0x55 / 255.0f);
}

GLchar const vertShaderTexture[] =
	"#version 330 core\n"
	"uniform sampler2D srcTx;"
	"attribute vec2 aPosn;"
	"void main() {\n"
	"	gl_Position = vec4(aPosn.x, aPosn.y, 0.0, 1.0);\n"
	"}\n";

GLchar const fragShaderTexture[] =
	"#version 330 core\n"
	"void main() {\n"
	"	gl_FragColor = vec4(1.0 / 3.0);\n"
	"}\n";

void framebufferTest() {
	GLuint fbTx = 0;
	glGenTextures(1, &fbTx);
	glBindTexture(GL_TEXTURE_2D, fbTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGBA, GL_FLOAT, NULL);
	filterClampBoilerplate();
	glBindTexture(GL_TEXTURE_2D, 0);
	assert(glGetError() == 0);

	GLuint fbuf = 0;
	glCreateFramebuffers(1, &fbuf);
	glBindFramebuffer(GL_FRAMEBUFFER, fbuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTx, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	assert(glGetError() == 0);

	uint8_t const block[16] = {0xFF, 0x00, 0xDB, 0xB6, 0x92, 0x6D, 0x49, 0x24};
	GLuint txName = 0;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, block);
	filterClampBoilerplate();

	float const verts[]= {
		 1.0f,  1.0f,
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
	};
	GLuint quad = 0;
	glGenBuffers(1, &quad);
	glBindBuffer(GL_ARRAY_BUFFER, quad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	assert(glGetError() == 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	assert(glGetError() == 0);
	glEnableVertexAttribArray(0);
	assert(glGetError() == 0);

	GLuint vert = compileShaderText(GL_VERTEX_SHADER,   vertShaderTexture);
	GLuint frag = compileShaderText(GL_FRAGMENT_SHADER, fragShaderTexture);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);
	glUseProgram (prog);
	assert(glGetError() == 0);

	glViewport(0, 0, 4, 4);
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, quad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glFinish();
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	assert(glGetError() == 0);

	glBindTexture(GL_TEXTURE_2D, fbTx);
	dumpBoundChannelData(true);
}

void APIENTRY debugCallback(GLenum /*source*/, GLenum /*type*/, GLuint /*id*/, GLenum /*severity*/, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
	printf("GL debug: %s\n", message);
}

void setup() {
	glDebugMessageCallback(debugCallback, NULL);
	glEnable(GL_DEBUG_OUTPUT);
	//bc3RedTest();
	//bc4RedTest();
	//red8Test();
	//computeTest();
	framebufferTest();
}

void draw(GLFWwindow* window) {
	int fbW, fbH;
	glfwGetFramebufferSize(window, &fbW, &fbH);
	glViewport(0, 0, fbW, fbH);
	glClearColor(0.3f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

int main(int /*argc*/, char* /*argv*/[]) {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //<-- Re-enable this and add VAO support
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	// No need to show the window, but we do need to create it for GLFW to work
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
	if (!window) {
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	setup();

#if 0
	while (!glfwWindowShouldClose(window)) {
		draw(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
#endif

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
