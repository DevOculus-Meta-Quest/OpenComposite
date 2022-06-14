#include "stdafx.h"
#include "Compositor/compositor.h"

#include "libovr_wrapper.h"

#include <string>

#include "OVR_CAPI_GL.h"
#include "GL/CAPI_GLE.h"

using namespace std;

OVR::GLEContext CustomGLEContext;

// copied from Quakespasm-Rift

static boolean GL_CheckShader(GLuint shader) {
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		char infolog[1024];
		memset(infolog, 0, sizeof(infolog));
		glGetShaderInfoLog(shader, sizeof(infolog), NULL, infolog);
		return false;
	}
	return true;
}

static boolean GL_CheckProgram(GLuint program)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		char infolog[1024];
		memset(infolog, 0, sizeof(infolog));
		glGetProgramInfoLog(program, sizeof(infolog), NULL, infolog);
		return false;
	}
	return true;
}

struct glsl_attrib_binding {
	const char* name;
	GLuint attrib;
};

static GLuint GL_CreateProgram(const GLchar* vertSource, const GLchar* fragSource,
	std::vector<glsl_attrib_binding> bindings = {}) {
	int i;
	GLuint program, vertShader, fragShader;
	
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertSource, NULL);
	glCompileShader(vertShader);
	if (!GL_CheckShader(vertShader)) {
		glDeleteShader(vertShader);
		return 0;
	}

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, NULL);
	glCompileShader(fragShader);
	if (!GL_CheckShader(fragShader)) {
		glDeleteShader(vertShader);
		glDeleteShader(fragShader);
		return 0;
	}

	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glDeleteShader(vertShader);
	glAttachShader(program, fragShader);
	glDeleteShader(fragShader);

	for (int i = 0; i < bindings.size(); ++i) {
		glBindAttribLocation(program, bindings[i].attrib, bindings[i].name);
	}
	glLinkProgram(program);

	if (!GL_CheckProgram(program)) {
		glDeleteProgram(program);
		return 0;
	} else {
		return program;
	}

}

static const GLchar* vertSource = \
"#version 110\n"
"\n"
"void main(void) {\n"
"      gl_Position = vec4(gl_Vertex.xy, 0.0, 1.0);\n"
"      gl_TexCoord[0] = vec4((gl_Vertex.xy+vec2(1,1))/2.0, 0.0, 1.0);\n"
"}\n";

static const GLchar* fragSource = \
"#version 110\n"
"\n"
"uniform sampler2D tex;\n"
"\n"
"void main(void) {\n"
"        vec4 frag = texture2D(tex, gl_TexCoord[0].xy);\n"
"        gl_FragColor = frag;\n"
"}\n";



GLCompositor::GLCompositor(OVR::Sizei size) {
	// TODO does this interfere with the host application?
	// TODO move somewhere more permenent
	OVR::GLEContext::SetCurrentContext(&CustomGLEContext);
	CustomGLEContext.Init();

	// Create the framebuffer we use when copying across textures
	glGenFramebuffers(1, &fboId);

	// Make eye render buffer
	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = size.w;
	desc.Height = size.h;
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	srcSize = size;

	ovrResult result = ovr_CreateTextureSwapChainGL(*ovr::session, &desc, &chain);
	if (!OVR_SUCCESS(result))
		throw string("Cannot create GL texture swap chain");

	program = GL_CreateProgram(vertSource, fragSource);

	if (program == 0) {
		throw string("Cannot create GL shader");
	}

	texLoc = glGetUniformLocation(program, "tex");

	const GLfloat vertices[] = {
		-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f };

	glGenBuffers(1, &quadVbo);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

}

unsigned int GLCompositor::GetFlags() {
	return ovrLayerFlag_TextureOriginAtBottomLeft;
}

void GLCompositor::Invoke(const vr::Texture_t * texture) {
	int currentIndex = 0;
	ovr_GetTextureSwapChainCurrentIndex(*ovr::session, chain, &currentIndex);

	// Why TF does OpenVR pass GLuints as pointers?!? That's really unsafe, although it's
	// very unlikely GLuint is suddenly going to grow.
	GLuint src = (GLuint)texture->handle;

	GLuint texId;
	ovr_GetTextureSwapChainBufferGL(*ovr::session, chain, currentIndex, &texId);

	//glClearColor(1, 0, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

	// https://stackoverflow.com/questions/23981016/best-method-to-copy-texture-to-texture#23994979
	// bind FBO to output texture
	// draw square with source texture using shader
	// this allows to apply postprocessing like contrast
	// using different shader

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, src);

	glUseProgram(program);
	glUniform1i(texLoc, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLCompositor::Invoke(ovrEyeType eye, const vr::Texture_t * texture, const vr::VRTextureBounds_t * bounds,
	vr::EVRSubmitFlags submitFlags, ovrLayerEyeFov &layer) {

	// Copy the texture over
	Invoke(texture);
}

void GLCompositor::InvokeCubemap(const vr::Texture_t * textures) {
	OOVR_ABORT("GLCompositor::InvokeCubemap: Not yet supported!");
}
