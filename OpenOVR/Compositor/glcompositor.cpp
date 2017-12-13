#include "stdafx.h"
#include "Compositor/compositor.h"

#include "libovr_wrapper.h"

#include <string>

#include "OVR_CAPI_GL.h"
#include "GL/CAPI_GLE.h"

using namespace std;

OVR::GLEContext CustomGLEContext;

GLCompositor::GLCompositor(ovrTextureSwapChain *chains, OVR::Sizei size) {
	this->chains = chains;

	// TODO does this interfere with the host application?
	// TODO move somewhere more permenent
	OVR::GLEContext::SetCurrentContext(&CustomGLEContext);
	CustomGLEContext.Init();

	// Create the framebuffer we use when copying across textures
	glGenFramebuffers(1, &fboId);

	// Make eye render buffers
	for (int ieye = 0; ieye < 2; ++ieye) {
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = size.w;
		desc.Height = size.h;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;

		ovrResult result = ovr_CreateTextureSwapChainGL(*ovr::session, &desc, &chains[ieye]);
		if (!OVR_SUCCESS(result))
			throw string("Cannot create GL texture swap chain");
	}
}

void GLCompositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
	vr::EVRSubmitFlags submitFlags) {

	ovrTextureSwapChain tex = chains[eye];

	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(*ovr::session, tex, &currentIndex);

	// Why TF does OpenVR pass GLuints as pointers?!? That's really unsafe, although it's
	// very unlikely GLuint is suddenly going to grow.
	GLuint src = (GLuint)texture->handle;

	GLuint texId;
	ovr_GetTextureSwapChainBufferGL(*ovr::session, tex, currentIndex, &texId);

	//glClearColor(1, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

	// Bind the source texture as a framebuffer, then copy out into our destination texture
	// https://stackoverflow.com/questions/23981016/best-method-to-copy-texture-to-texture#23994979
	// Since I cannot be convinved to bother with a passthrough shader and this is almost as
	// fast, we'll us it until supersampling becomes an issue.

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src, 0);

	glBindTexture(GL_TEXTURE_2D, texId);

	GLsizei width, height;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	glCopyTexSubImage2D(
		GL_TEXTURE_2D, 0, // 0 == no mipmapping
		0, 0, // Position in the source framebuffer
		0, 0, width, height // Region of the output texture to copy into (in this case, everything)
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
