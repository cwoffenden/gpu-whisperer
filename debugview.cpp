#include "debugview.h"

#include <cassert>

/**
 * Creates a 4x4 red-only uncompressed 8-bit texture with ideal BC3 values.
 *
 * \param[in] txId pre-generated texture ID to use
 */
void create4x4RedBC3Vals(GLuint const txId) {
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
void create4x4RedBC4Vals(GLuint const txId) {
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

//*****************************************************************************/

static Program prog; /**< Simple quad debug program. */
static GLuint vaoId = 0; /**< VAO fullscreen textured quad.  */
static GLuint vboId = 0; /**< VBO for \c vObjId quad. */

void showDebugView(GLFWwindow* const window, ContextVersion const glVers) {
	if (glVers > VERSION_2_0) {
		createVertFragShaders(vertShaderTexture150, fragShaderTexture150, prog);
	} else {
		createVertFragShaders(vertShaderTexture110, fragShaderTexture110, prog);
	}
	createTexturedQuad(glVers, vaoId, vboId);
	while (!glfwWindowShouldClose(window)) {
		int fbW, fbH;
		glfwGetFramebufferSize(window, &fbW, &fbH);
		glViewport(0, 0, fbW, fbH);
		glClearColor(0.3f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}