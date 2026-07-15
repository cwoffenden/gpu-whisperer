/**
 * \file glcommon.h
 * Common GL helpers.
 */
#pragma once

#include "glplatform.h"

/**
 * GL version selected to create the context.
 */
enum ContextVersion {
	VERSION_NONE =  0, /**< No valid context. */
	VERSION_2_0  = 20, /**< Legacy GL without VAO support. */
	VERSION_3_3  = 33, /**< Most compatible pre-compute shader GL */
	VERSION_4_3  = 43, /**< Gl with compute shaders. */
};

/**
 * Vertex attribute IDs.
 */
enum VertexID {
	VERT_POSN_ID = 0, /**< Vertex positions, called <tt>aPosn</tt>. */
	VERT_TEX0_ID = 1, /**< Vertex texture channel 0, called <tt>aTex0</tt>. */
	VERT_TEX1_ID = 2, /**< Vertex texture channel 1, called <tt>aTex1</tt>. */
};

/**
 * GLSL 1.10 compatible vertex shader, designed only to draw a fullscreen
 * textured quad.
 */
GLchar const vertShaderTexture110[] =
	"attribute vec2 aPosn;\n"
	"attribute vec2 aTex0;\n"
	"varying vec2 vTex0;\n"
	"void main() {\n"
	"	vTex0 = aTex0;\n"
	"	gl_Position = vec4(aPosn.x, aPosn.y, 0.0, 1.0);\n"
	"}\n";

/**
 * GLSL 1.50 version of the \c #vertShaderTexture110 quad shader.
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
 * GLSL 1.10 compatible fragment shader, designed only to draw a fullscreen
 * textured quad.
 */
GLchar const fragShaderTexture110[] =
	"uniform sampler2D srcTx;\n"
	"varying vec2 vTex0;\n"
	"void main() {\n"
	"	gl_FragColor = texture2D(srcTx, vTex0);\n"
	"}\n";

/**
 * GLSL 1.50 version of the \c #fragShaderTexture110 quad shader.
 */
GLchar const fragShaderTexture150[] =
	"#version 150\n"
	"precision highp float;\n"
	"uniform sampler2D srcTx;\n"
	"in vec2 vTex0;\n"
	"out vec4 FragColor;\n"
	"void main() {\n"
	"	FragColor = texture(srcTx, vTex0);\n"
	"}\n";

/**
 * Container for a GL program and its shaders.
 */
struct Program {
	/**
	 * Zero initialiser.
	 */
	Program()
		: progId(0)
		, vertId(0)
		, fragId(0) {}
	GLuint progId; /**< Program ID. */
	GLuint vertId; /**< Vertex shader ID. */
	GLuint fragId; /**< Fragment shader ID. */
};

/**
 * Helper to compile a shader from source.
 *
 * \param[in] type shader type
 * \param[in] text shader source
 * \return the shader ID (or zero if compilation failed)
 */
GLuint compileShaderSource(GLenum type, const GLchar* _Nonnull text);

/**
 * Helper to create vertex and fragment shaders. After calling the current
 * program is set to the compiled result.
 *
 * \param[in] vertSrc vertex shader source
 * \param[in] fragSrc fragment shader source
 * \param[out] prog destination for the program, vertex and fragment IDs
 * \return \c true of compilation and linking was successful
 */
bool createVertFragShaders(const GLchar* _Nonnull vertSrc, const GLchar* _Nonnull fragSrc, Program& prog);

/**
 * Clean-up for \c #createVertFragShaders().
 *
 * \param[in,out] prog program, vertex and fragment IDs
 */
void deleteVertFragShaders(Program& prog);

/**
 * Creates a fullscreen textured quad. After calling the VAO \a vaoId and/or its
 * VBO \a vboId remain bound (to ease drawing, since this is the only geometry).
 */
void createTexturedQuad(ContextVersion glVers, GLuint& vaoId, GLuint& vboId);

/**
 * Filter to nearest and clamp to edge the current bound texture.
 */
void filterClampBoilerplate();

/**
 * Tests whether the currently bound texture has dimensions greater than zero
 * (and by extension had valid content uploaded).
 *
 * \return \c true if the texture's first mipmap level has content
 */
bool doesBoundTextureHaveContent();

/**
 * Tests whether the currently bound texture is hardware compressed.
 *
 * \return \c true if the texture's first mipmap level is compressed
 */
bool isBoundTextureCompressed();
