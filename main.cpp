#include <cassert>
#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#endif
#include <GLFW/glfw3.h>

struct BCBlock {
	union {
		uint8_t bytes[8];
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
				uint32_t bits;
				struct {
					uint8_t x0: 2;
					uint8_t x1: 2;
					uint8_t x2: 2;
					uint8_t x3: 2;
				} y[4];
			} texels;
		} bc1;
		uint64_t data;
	};
};

static_assert(sizeof(BCBlock) == 8, "BC block should be 8 bytes");

static uint32_t fpBits(float val) {
	union {
		float    f;
		uint32_t u;
	} bits = {
		val
	};
	return bits.u;
}

BCBlock block[2] = {};

GLuint txName = 0;

void setup() {
	block[1].bc1.endpt[0].r = 31;
	block[1].bc1.endpt[1].r = 9;
	block[1].bc1.texels.bits = 0xEE44EE44;
	glGenTextures(1, &txName);
	glBindTexture(GL_TEXTURE_2D, txName);
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 4, 4, 0, 16, &block);

	float f32[4][4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, f32);
	uint16_t u16[4][4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, u16);
	uint8_t u08[4][4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, u08);
	GLenum err = glGetError();
	assert(err == 0);

	printf("Floats\n");
	printf("%0.9f\n%0.9f\n%0.9f\n%0.9f\n", f32[0][0], f32[0][1], f32[1][0], f32[1][1]);
	printf("\n");
	printf("Float bits\n");
	printf("0x%08X\n0x%08X\n0x%08X\n0x%08X\n", fpBits(f32[0][0]), fpBits(f32[0][1]), fpBits(f32[1][0]), fpBits(f32[1][1]));
	printf("\n");
	printf("Shorts\n");
	printf("0x%04X\n0x%04X\n0x%04X\n0x%04X\n", u16[0][0], u16[0][1], u16[1][0], u16[1][1]);
	printf("\n");
	printf("Bytes\n");
	printf("0x%02X\n0x%02X\n0x%02X\n0x%02X\n", u08[0][0], u08[0][1], u08[1][0], u08[1][1]);
	printf("\n");
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(512, 512, "Test", NULL, NULL);
	if (!window) {
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	setup();

	while (!glfwWindowShouldClose(window)) {
		draw(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}
