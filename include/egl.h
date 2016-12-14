#ifndef EGL_H_
#define EGL_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <EGL/egl.h>

// EGL wrapper class
class EGL {
 public:
	// Constructor
	EGL();
	// Destructor
	~EGL();
	// Create Context
	bool CreateContext(EGLNativeDisplayType nativedisplay,
	                   EGLNativeWindowType  nativewindow);

	// Get Context in order to share it
	EGLContext GetContext() const { return egl_context_; }
	// Swap egl buffers
	void SwapBuffers();
	// Change current egl context
	void MakeCurrent();
	// Destroy Resources
	void destroyContext();

 private:
	EGLDisplay egl_display_;
	EGLConfig  egl_config_;
	EGLSurface egl_surface_;
	EGLContext egl_context_;

 private:
	DISALLOW_COPY_AND_ASSIGN(EGL);
};
#endif // EGL_H_
