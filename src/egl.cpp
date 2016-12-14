#include "egl.h"

#include <EGL/egl.h>

#include <iostream>
#include <cstdlib>

// Constructor
EGL::EGL()
	: egl_display_(0)
	, egl_config_(0)
	, egl_surface_(EGL_NO_SURFACE)
	, egl_context_(EGL_NO_CONTEXT)
{
}

// Destructor
EGL::~EGL() {
}

// Constructor without share context
bool EGL::CreateContext(EGLNativeDisplayType nativedisplay,
                        EGLNativeWindowType nativewindow) {

	// Config attributes for creating context and surface
	EGLint cfgAttr[] = {
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE,			8,
		EGL_GREEN_SIZE,			8,
		EGL_BLUE_SIZE,			8,
		EGL_ALPHA_SIZE,			8,
		EGL_DEPTH_SIZE,			24,
		EGL_NONE
	};
	// Context attributes for creating context
	EGLint ctxAttr[] = {
		EGL_CONTEXT_CLIENT_VERSION,	2,
		EGL_NONE
	};

	EGLint stat = EGL_SUCCESS;
	EGLBoolean ans = EGL_FALSE;
	EGLint majorVersion = 0;
	EGLint minorVersion = 0;
	int configs = 0;

	do {
		// Get EGLDisplay as Wayland Back-end Server
		egl_display_ = eglGetDisplay(nativedisplay);
		stat = eglGetError();
		if ((EGL_NO_DISPLAY == egl_display_) || (EGL_SUCCESS != stat)) {
			std::cout << "ERROR: eglGetDisplay() failed. code=" << stat << std::endl;
			std::abort(); // FIXME
			break;
		}

		// Initialize EGL
		ans = eglInitialize(egl_display_, &majorVersion, &minorVersion);
		stat = eglGetError();
		if ((EGL_TRUE != ans) || (EGL_SUCCESS != stat)) {
			std::cout << "ERROR: eglInitialize() failed. code=" << stat << std::endl;
			std::abort(); // FIXME
			break;
		}

		// Bind GLES
		ans = eglBindAPI(EGL_OPENGL_ES_API);
		stat = eglGetError();
		if ((EGL_TRUE != ans) || (EGL_SUCCESS != stat)) {
			std::cout << "ERROR: eglBindAPI() failed. code=" << stat << std::endl;
			std::abort(); // FIXME
			break;
		}

		// Choose Configuration and get config ID for creating context and surface
		ans = eglChooseConfig(egl_display_, cfgAttr, &egl_config_, 1, &configs);
		stat = eglGetError();
		if ((EGL_TRUE != ans) || (configs != 1) || (EGL_SUCCESS != stat)) {
			std::cout << "ERROR: eglChooseConfig() failed. code=" << stat << std::endl;
			std::abort(); // FIXME
			break;
		}

		// Create Window Surface
		egl_surface_ = eglCreateWindowSurface(egl_display_, egl_config_, nativewindow, NULL);
		stat = eglGetError();
		if ((EGL_NO_SURFACE == egl_surface_) || (EGL_SUCCESS != stat)) {
			std::cout << "ERROR: eglCreateWindowSurface() failed. code=" << stat << std::endl;
			std::abort(); // FIXME
			break;
		}

		// Create Context
		egl_context_ = eglCreateContext(egl_display_, egl_config_, EGL_NO_CONTEXT, ctxAttr);
		stat = eglGetError();
		if ((EGL_NO_CONTEXT == egl_context_) || (EGL_SUCCESS != stat)) {
			std::cout << "ERROR: eglCreateContext() failed. code=" << stat << std::endl;
			std::abort(); // FIXME
			break;
		}

		return true;
	} while(0);

	destroyContext();

	return false;
}

// Destroy Resources
void EGL::destroyContext() {
	if (NULL == egl_display_) {
		return;
	}

	// Dettach the context
	eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	if (NULL != egl_context_) {
		eglDestroyContext(egl_display_, egl_context_);
		egl_context_ = NULL;
	}

	if (NULL != egl_surface_) {
		eglDestroySurface(egl_display_, egl_surface_);
		egl_surface_ = NULL;
	}

	eglTerminate(egl_display_);
	egl_display_ = NULL;

	eglReleaseThread();
}

// Swap Buffers
void EGL::SwapBuffers() {
	eglSwapBuffers(egl_display_, egl_surface_);
}

void EGL::MakeCurrent() {
	// Bind EGLContext and EGLSurface to EGLDisplay
	EGLBoolean ans = eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_);
	if ((EGL_TRUE != ans) || (EGL_SUCCESS != eglGetError())) {
		std::cout << "ERROR: eglMakeCurrent() failed." << std::endl;
	}
}
