#include "glcommon.h"

#include <cassert>
#include <cstdio>

GLuint compileShaderSource(GLenum const type, const GLchar* const text) {
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

bool createVertFragShaders(const GLchar* const vertSrc, const GLchar* const fragSrc, Program& prog) {
	assert(prog.progId == 0);
	prog.progId = glCreateProgram();
	if (prog.progId) {
		glBindAttribLocation(prog.progId, VERT_POSN_ID, "aPosn");
		glBindAttribLocation(prog.progId, VERT_TEX0_ID, "aTex0");
		glBindAttribLocation(prog.progId, VERT_TEX1_ID, "aTex1");
		prog.vertId = compileShaderSource(GL_VERTEX_SHADER,   vertSrc);
		prog.fragId = compileShaderSource(GL_FRAGMENT_SHADER, fragSrc);
		if (prog.vertId && prog.fragId) {
			glAttachShader(prog.progId, prog.vertId);
			glAttachShader(prog.progId, prog.fragId);
			glLinkProgram (prog.progId);
			GLint linked;
			glGetProgramiv(prog.progId, GL_LINK_STATUS, &linked);
			if (linked == GL_TRUE) {
				glUseProgram(prog.progId);
				return true;
			}
		}
	}
	return false;
}

void deleteVertFragShaders(Program& prog) {
	glUseProgram(0);
	glDetachShader (prog.progId, prog.vertId);
	glDetachShader (prog.progId, prog.fragId);
	glDeleteProgram(prog.progId);
	glDeleteShader (prog.vertId);
	glDeleteShader (prog.fragId);
	prog.progId = 0;
	prog.vertId = 0;
	prog.fragId = 0;
}

void createTexturedQuad(ContextVersion const glVers, GLuint& vaoId, GLuint& vboId) {
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

void filterClampBoilerplate() {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool doesBoundTextureHaveContent() {
	GLint size = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size);
	return size > 0;
}

bool isBoundTextureCompressed() {
	GLint valid = GL_FALSE;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &valid);
	return valid == GL_TRUE;
}
