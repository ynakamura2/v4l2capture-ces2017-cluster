#include "image_process.h"

#include <unistd.h>
#include <stdint.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "settings.h"

#define GLERROR_CHECK(x) if (GL_NO_ERROR != glGetError()) { \
	std::cerr << x << std::endl; std::abort(); \
}

#ifdef DEBUG_
#  define DEBUG_LOG(x) std::clog << "----- ImageProcess:: " << x << std::endl
#else
#  define DEBUG_LOG(x) do {} while (0)
#endif

// Shader code for vertex
const char* ImageProcess::kShaderCodeVtx =
	"attribute mediump vec4 a_Position;\n"
	"attribute mediump vec2 a_TexCoord;\n"
	"varying mediump vec2 v_TexCoord;\n"
	"\n"
	"void main(void) {\n"
	"  gl_Position = a_Position;\n"
	"  v_TexCoord  = a_TexCoord;\n"
	"}\n";

// Shader code for fragment
const char* ImageProcess::kShaderCodeFrg = {
	"varying mediump vec2 v_TexCoord;\n"
	"uniform sampler2D tex;\n"
	"uniform mediump float alpha;\n"
	"\n"
	"void main (void) {\n"
	"  /* YUYV to RGBA */\n"
	"  mediump float y, u, v;\n"
	"  mediump vec4 resultcolor;\n"
	"  mediump vec4 raw = texture2D(tex, v_TexCoord);\n"
	"\n"
	"  if (int(mod(gl_FragCoord.x, 2.0)) == 0)\n"
	"    raw.b = raw.r;\n"
	"\n"
	"  y = 1.1643*(raw.b-0.0625);\n"
	"  u = raw.g-0.5;\n"
	"  v = raw.a-0.5;\n"
	"  resultcolor.r = (y+1.5958*(v));\n"
	"  resultcolor.g = (y-0.39173*(u)-0.81290*(v));\n"
	"  resultcolor.b = (y+2.017*(u));\n"
	"  resultcolor.a = 1.0;\n"
	"  gl_FragColor = resultcolor;\n"
	"}\n",
};

// Coord for texture mapping
const GLfloat ImageProcess::kVtxCoord[] = {
	 1.0f, -1.0f,
	 1.0f,  1.0f,
	-1.0f, -1.0f,
	-1.0f,  1.0f,
};

// Coord for texture mapping
const GLfloat ImageProcess::kTexCoord[] = {
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,
};

// Constructor
ImageProcess::ImageProcess(Settings* settings)
	: settings_(settings)
	, shader_vtx_()
{
}

// Destructor
ImageProcess::~ImageProcess() {
	glDeleteShader(shader_vtx_);
	glDeleteShader(shader_frg_);
	glDeleteProgram(shader_program_);
}

int32_t ImageProcess::Init() {
	InitShader();
	GLERROR_CHECK("InitShader");

	InitProgram();
	GLERROR_CHECK("InitProgram");

	InitTexture();
	GLERROR_CHECK("InitProgram");

	return 0;
}

void ImageProcess::InitShader() {
	// Create vertex shader
	shader_vtx_ = CreateGLShader(&kShaderCodeVtx, GL_VERTEX_SHADER);
	// Create fragment shader
	shader_frg_ = CreateGLShader(&kShaderCodeFrg, GL_FRAGMENT_SHADER);
	// Create program
	shader_program_ = CreateGLProgram(shader_vtx_, shader_frg_);
}

void ImageProcess::InitProgram() {
	// Initialize shader params
	GLuint loc_vtxcoord_ = glGetAttribLocation(shader_program_, "a_Position");
	GLuint loc_texcoord_ = glGetAttribLocation(shader_program_, "a_TexCoord");
	GLERROR_CHECK("AttLoc");

	glEnableVertexAttribArray(loc_vtxcoord_);
	glEnableVertexAttribArray(loc_texcoord_);

	glVertexAttribPointer(loc_vtxcoord_, 2, GL_FLOAT, GL_FALSE, 0, kVtxCoord);
	glVertexAttribPointer(loc_texcoord_, 2, GL_FLOAT, GL_FALSE, 0, kTexCoord);
	GLERROR_CHECK("AttrPointer");

	GLint loc_sampler_ = glGetUniformLocation(shader_program_, "tex");
	GLERROR_CHECK("UniLoc");

	glUseProgram(shader_program_);
	glUniform1i(loc_sampler_, 0);
	GLERROR_CHECK("Unili 0");
	glUseProgram(0);
}

void ImageProcess::InitTexture() {
	// Create Texture
	glGenTextures(1, &texture_id_);

	glBindTexture(GL_TEXTURE_2D, texture_id_);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			settings_->GetCaptureWidth() / 2,
			settings_->GetCaptureHeight(),
			0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Setup Draw Property
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, settings_->GetCaptureWidth(), settings_->GetCaptureHeight());
}

// Create shader
GLuint ImageProcess::CreateGLShader(const char** shaderCode, GLenum shaderType) {
	GLuint shader = 0;
	GLint stat = 0;

	shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, shaderCode, 0);
	glCompileShader(shader);
	// Check the compile error
	glGetShaderiv(shader, GL_COMPILE_STATUS, (GLint*)&stat);
	if (GL_FALSE == stat) {
		GLint logLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, (GLint*)&logLen);

		char* log = new char[logLen];
		int ans = 0;
		glGetShaderInfoLog(shader, logLen, NULL, log);
		std::cout << "SHADER INFO:" << log << std::endl;

		glGetShaderiv(shader, GL_COMPILE_STATUS, &ans);
		std::cout << "SHADER BUILD INFO:" << ans << std::endl;

		delete [] log;

		exit(1);
	}

	return shader;
}

GLuint ImageProcess::CreateGLProgram(GLuint shaderVtx, GLuint shaderFrg) {
	GLuint program = 0;
	GLint stat = 0;

	program = glCreateProgram();
	glAttachShader(program, shaderFrg);
	glAttachShader(program, shaderVtx);

	// Link program to two shaders
	glLinkProgram(program);
	// Check the link error
	glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&stat);
	if (GL_FALSE == stat) {
		GLint logLen = 0;
		glGetShaderiv(program, GL_INFO_LOG_LENGTH, (GLint*)&logLen);

		char* log = new char[logLen];
		int ans = 0;
		glGetShaderInfoLog(program, logLen, NULL, log);
		std::cout << "SHADER INFO:" << log << std::endl;

		glGetShaderiv(program, GL_COMPILE_STATUS, &ans);
		std::cout << "SHADER BUILD INFO:" << ans << std::endl;

		delete [] log;

		exit(1);
	}

	glReleaseShaderCompiler();

	return program;
}

void ImageProcess::SetBuffer(void* const capBuffer) {
	// Create glTexture from V4L2 Capture Buffer
	glBindTexture(GL_TEXTURE_2D, texture_id_);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					settings_->GetCaptureWidth() / 2,
					settings_->GetCaptureHeight(),
					GL_RGBA, GL_UNSIGNED_BYTE, capBuffer);
}

void ImageProcess::Draw() {
	// Draw
	glUseProgram(shader_program_);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->GetTextureId());
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glUseProgram(0);
}
