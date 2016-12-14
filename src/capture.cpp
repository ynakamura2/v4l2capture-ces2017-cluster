#include "capture.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/videodev2.h>

#include <iostream>
#include <string>
#include <cstdlib>

#include "v4l2.h"
#include "settings.h"
#include "wayland.h"
#include "egl.h"
#include "image_process.h"

#ifdef DEBUG_
#  define DEBUG_LOG(x) std::clog << "----- Capture:: " << x << std::endl
#else
#  define DEBUG_LOG(x) do {} while (0)
#endif

Capture::Capture()
	: capture_status_(CAPTURE_STOP)
	, vcThreadParam()
	, window_title_(NULL)
	, vcConfig()
	, vcWayland(new Wayland())
	, vcV4L2(new V4L2())
	, vcEGL(new EGL())
{
	// Initialize pthread_mutex_t
	pthread_mutex_init(&mutex_capture_status_, 0);
}

Capture::~Capture() {
	delete vcEGL;
	delete vcWayland;
	delete vcV4L2;
	delete vcImageProcess;
	vcEGL       = NULL;
	vcWayland   = NULL;
	vcV4L2      = NULL;
	vcImageProcess = NULL;
}

void Capture::Init(Settings* const Config,
                   const char* window_title) {
	vcConfig = Config;
	window_title_ = window_title;
}

void Capture::StartCapturing() {
	MutexLock(&mutex_capture_status_);
	capture_status_ = CAPTURE_START;
	MutexUnLock(&mutex_capture_status_);
}

void Capture::StopCapturing() {
	MutexLock(&mutex_capture_status_);
	capture_status_ = CAPTURE_STOP;
	MutexUnLock(&mutex_capture_status_);
}

void Capture::CloseDevice() {
	MutexLock(&mutex_capture_status_);
	capture_status_ = CAPTURE_DESTROY;
	MutexUnLock(&mutex_capture_status_);
}

Capture::E_CAPTURE_STATUS Capture::GetCaptureStatus() {
	MutexLock(&mutex_capture_status_);
	E_CAPTURE_STATUS status = capture_status_;
	MutexUnLock(&mutex_capture_status_);
	return status;
}

void Capture::CaptureMain() {
	DEBUG_LOG("thread start");

	InitContext();
	InitV4L2();

	bool is_capturing  = false;

#ifdef DEBUG_
	// double time_start_total, time_end_total;
#endif
	// MAIN LOOP :
	// When CloseDevice() is called, It will be finish.
	DEBUG_LOG("Wait for Start");
	while (CAPTURE_DESTROY != GetCaptureStatus()) {
#ifdef DEBUG_
		// GetTime(time_start_total);
#endif
		// Check the state of having already Captured
		if (!is_capturing) {
			// Wait for START
			if (CAPTURE_START == GetCaptureStatus()) {
				// Start V4L2 Streaming
				DEBUG_LOG("StartCapturing");
				vcV4L2->StartCapturing();
				is_capturing = true;
			} else {
				usleep(50 * 1000);
				continue;
			}
		} else {
			// Capturing and Create GL Texture
			DoCapturing();

			// Watch status STOP
			if (CAPTURE_STOP == GetCaptureStatus()) {
				DEBUG_LOG("StopCapturing");
				vcV4L2->StopCapturing();
				is_capturing = false;
			}
		}

#ifdef DEBUG_
		// GetTime(time_end_total);
		// printf("----- Capture:: fps >>> %lf[frame/sec]\n",
		// 	   1.0 / (time_end_total - time_start_total));
#endif
	}

	CloseV4L2();

	void* p_ret = NULL;
	pthread_join(vcThreadParam, &p_ret);

	vcEGL->destroyContext();
	vcWayland->destroyContext();

	DEBUG_LOG("thread end");
	return;
}

void Capture::DoCapturing() {
	// Capturing from Device using V4L2
	vcV4L2->mainloop();

	// Create GL_TEXTURE from the Captured buffer
	vcImageProcess->SetBuffer(vcV4L2->GetBuffer());

	// VIDIOC_QBUF
	vcV4L2->syncmainloop();

	vcImageProcess->Draw();
	vcEGL->SwapBuffers();
	wl_display_roundtrip(vcWayland->GetDisplay());

}

void Capture::GetTime(double &time) {
	struct timeval tv_;
	gettimeofday(&tv_, NULL);
	time = tv_.tv_sec + (tv_.tv_usec * 1e-6);
}

void Capture::InitContext() {
	int32_t ans = 0;
	// Wayland
	ans = vcWayland->CreateContext(vcConfig->GetCaptureWidth(),
	                               vcConfig->GetCaptureHeight(),
	                               window_title_);
	if (0 != ans) {
		std::abort(); // FIXME
	}

	// Create EGL context
	vcEGL->CreateContext(vcWayland->GetDisplay(),
	                     vcWayland->GetNativeWindow());
	vcEGL->MakeCurrent();

	// Create Buffer context
	// NOTE: Must do initialization after EGL context is created.
	vcImageProcess = new ImageProcess(vcConfig);
	vcImageProcess->Init();
}

void Capture::InitV4L2() {
	// Open Video device
	DEBUG_LOG("vcV4L2->OpenDevice()");
	vcV4L2->OpenDevice(vcConfig->GetDeviceName().c_str());

	DEBUG_LOG("vcV4L2->InitDevice()");
	vcV4L2->InitDevice(vcConfig->GetCaptureWidth(),
					   vcConfig->GetCaptureHeight(),
					   V4L2_PIX_FMT_UYVY);
}

void Capture::CloseV4L2() {
	// UnInit V4L2 Device
	DEBUG_LOG("vcV4L2->UnInitDevice()");
	vcV4L2->UnInitDevice();

	// Close V4L2 Device
	DEBUG_LOG("vcV4L2->CloseDevice()");
	vcV4L2->CloseDevice();
}

void Capture::MutexLock(pthread_mutex_t* const mutex_t) {
	if (0 != pthread_mutex_lock(mutex_t)) {
		abort();
	}
}

void Capture::MutexUnLock(pthread_mutex_t* const mutex_t) {
	if (0 != pthread_mutex_unlock(mutex_t)) {
		abort();
	}
}
