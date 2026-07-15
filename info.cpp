#include "info.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "glplatform.h"

/**
 * Helper to search through \a texFmt to find a match for \a fmt1st \e and
 * optionally, if supplied, \a fmt2nd.
 *
 * \note this \c find requires \e both enums if the second one is supplied
 * (unlike the extension-based \c find).
 *
 * \param[in] numFmt number of entries in \a texFmt
 * \param[in] texFmt array of \c GL_COMPRESSED_TEXTURE_FORMATS entries
 * \param[in] fmt1st format match to find
 * \param[in] fmt2nd optional format to match (or \c GL_INVALID_ENUM to only match on \a fmt1st)
 * \return \c true if the supplied format(s) are found
 */
static bool find(GLint numFmt, const GLint* texFmt, GLint fmt1st, GLint fmt2nd = GL_INVALID_ENUM) {
	for (GLint n1 = 0; n1 < numFmt; n1++) {
		if (texFmt[n1] == fmt1st) {
			if (fmt2nd == GL_INVALID_ENUM) {
				return true;
			}
			for (GLint n2 = 0; n2 < numFmt; n2++) {
				if (texFmt[n2] == fmt2nd) {
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * Helper to search through \a extStr to find matching GL extension \a ext1 \e
 * or the alternative \a ext2.
 *
 * \note this \c find will match on \e either extension name (unlike the
 * enum-based \c find)
 *
 * \param[in] extStr GL extensions string (see \c GL_EXTENSIONS)
 * \param[in] extEnd pointer to the end of \a extStr (passed in to save calculating it each time)
 * \param[in] ext1 extension to match, e.g.: \c EXT_texture_sRGB
 * \param[in] ext2 optional extension to match (defaulting to \c null to search for only \a ext1)
 * \return \c true if \e either of the extensions were found
 */
static bool find(const char* extStr, const char* extEnd, const char* ext1, const char* ext2 = NULL) {
	size_t const ext1Len =          strlen(ext1);
	size_t const ext2Len = (ext2) ? strlen(ext2) : 0;
	for (const char* ext = extStr; ext < extEnd;) {
		if (size_t extLen = strcspn(ext, " ")) {
			/*
			 * Nix the "GL_" part to just have the extension name (WebGL, for
			 * example, only has the 'official' name without the prefix, whereas
			 * desktop is prefixed).
			 */
			if (extLen > 3 && strncmp(ext, "GL_", 3) == 0) {
				ext    += 3;
				extLen -= 3;
			}
			if (ext1Len == extLen) {
				if (strncmp(ext, ext1, ext1Len) == 0) {
					return true;
				}
			}
			if (ext2Len == extLen) {
				if (strncmp(ext, ext2, ext2Len) == 0) {
					return true;
				}
			}
			ext += extLen + 1;
		} else {
			break;
		}
	}
	return false;
}

/**
 * Ask GL what texture formats are supported (which will probably be incorrect,
 * with known issues documented in the source).
 *
 * \param[out] dxt1 DXT1/BC1 support
 * \param[out] dxt5 DXT5/BC3 support
 * \param[out] rgtc RGTC/BC4/BC5 support
 * \param[out] latc LATC support (legacy)
 */
static void queryFormatSupport(bool& dxt1, bool& dxt5, bool& rgtc, bool& latc) {
	dxt1 = false;
	dxt5 = false;
	rgtc = false;
	latc = false;
	/*
	 * Initial testing is via the exposed compressed texture formats. This
	 * seems to be quite complete and work fine for ES, not so for desktop GL
	 * (which requires the extensions checking). Each supported enum usually
	 * corresponds directly to that type (since that's what's passed into the
	 * texture creation call).
	 */
	GLint numFmt = 0;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFmt);
	if (numFmt) {
		if (GLint* texFmt = static_cast<GLint*>(calloc(numFmt, sizeof(GLint)))) {
			glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, texFmt);
			if (find(numFmt, texFmt, GL_COMPRESSED_RGB_S3TC_DXT1_EXT)) {
				dxt1 = true;
				if (find(numFmt, texFmt, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)) {
					dxt5 = true;
				}
			}
			if (find(numFmt, texFmt, GL_COMPRESSED_RED_RGTC1)) {
				rgtc = true;
			}
			if (find(numFmt, texFmt, GL_COMPRESSED_LUMINANCE_LATC1_EXT)) {
				latc = true;
			}
			free(texFmt);
		}
	}
	/*
	 * After the compressed formats testing we then look at extensions, only
	 * this is completely unreliable and probably won't work. The GL_EXTENSIONS
	 * usage changed from GL3 onwards, and where the older concatenated string
	 * could hold 100+, the newer individual extension list is considerably
	 * smaller (and any calling APIs like GLFW, although they handle the API
	 * differences they're still missing the required content). Mac GL is a good
	 * concrete example, RGTC is supported in hardware, and the ARB extension is
	 * exposed for a 'legacy' context, but not for GL3.
	 */
	if (!dxt1) {
		if (glfwExtensionSupported("GL_EXT_texture_compression_s3tc")) {
			dxt1 = true;
			dxt5 = true;
		}
	}
	if (!rgtc) {
		if (glfwExtensionSupported("GL_EXT_texture_compression_rgtc") ||
			glfwExtensionSupported("GL_ARB_texture_compression_rgtc"))
		{
			rgtc = true;
		}
	}
	if (!latc) {
		if (glfwExtensionSupported("GL_EXT_texture_compression_latc") ||
			glfwExtensionSupported("GL_NV_texture_compression_latc"))
		{
			latc = true;
		}
	}
}

/**
 * Helper to extract the filename from a path.
 *
 * \param[in] path full path
 * \return the file at the end of the path (or an empty string if there is no file)
 */
static const char* extractFilename(const char* path) {
	if (path) {
		const char* found  = strrchr(path, '/');
		if (!found) {
			 found = strrchr(path, '\\');
		}
		if (found && strlen(found) > 0) {
			return found + 1;
		}
	}
	return path;
}

//*****************************************************************************/

void showInfo() {
	bool dxt1, dxt5, rgtc, latc;
	queryFormatSupport(dxt1, dxt5, rgtc, latc);
	printf("GL version: %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
	printf("GL reports DXT1/BC1 support: %s\n", dxt1 ? "yes" : "no");
	printf("GL reports DXT5/BC3 support: %s\n", dxt5 ? "yes" : "no");
	printf("GL reports RGTC/BC4 support: %s\n", rgtc ? "yes" : "no");
	printf("GL reports LATC support: %s\n", latc ? "yes" : "no");
}

void showUsage(const char* path) {
	const char* prog = extractFilename(path);
	if (!prog) {
		 prog = "gpu-whisperer";
	}
	printf("Usage: %s --mode mode [--framebuffer]\n", prog);
	printf("\t--mode options are:\n");
	printf("\t\tinfo basic renderer info\n");
	printf("\t\tvalidate tests the buffer precision\n");
	printf("\t\tgenerate extract texture data\n");
	printf("\t\tdebug show the internal textures\n");
	printf("\t--framebuffer always use the framebuffer fallback\n");
}
