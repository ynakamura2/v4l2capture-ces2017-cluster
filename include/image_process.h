#ifndef IMAGEPROCESS_H_
#define IMAGEPROCESS_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <stdint.h>
#include <GLES2/gl2.h>

#include <string>

class Settings;

// Draw Octahedron class
class ImageProcess {
 private:
	// Shader code for vertex
	static const char* kShaderCodeVtx;
	// Shader code for fragment
	static const char* kShaderCodeFrg;
	// Vertex Coord
	static const GLfloat kVtxCoord[];
	// Texture coord for texture mapping
	static const GLfloat kTexCoord[];

 public:
	// Constructor
	ImageProcess(Settings* settings);
	// Destructor
	~ImageProcess();

	// Initialize
	int32_t Init();

	// Draw method
	void SetBuffer(void* const capBuffer);
	void Draw();

 private:
	void InitShader();
	void InitProgram();
	void InitTexture();

	GLuint CreateGLShader(const char** shaderCode, GLenum shaderType);
	GLuint CreateGLProgram(GLuint shaderVtx, GLuint shaderFrg);

	void DumpFrameBuffer(std::string name, int32_t count,
	                     int32_t dump_width, int32_t dump_height);

	GLuint GetTextureId() const { return texture_id_; }

 private:
	Settings* settings_;

	GLuint texture_id_;
	// Shader ID for vertex
	GLuint shader_vtx_;
	// Shader ID for fragment
	GLuint shader_frg_;
	// Program ID
	GLuint shader_program_;

 private:
	DISALLOW_COPY_AND_ASSIGN(ImageProcess);
};
#endif // IMAGEPROCESS_H_
