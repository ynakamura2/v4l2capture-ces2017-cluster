#include "context.h"

#include <pthread.h>

#include <cstdio>
#include <cstdlib>

#include "capture.h"
#include "settings.h"

Context::Context()
	: capture_(new Capture())
	, thread_param_()
{
}

Context::~Context() {
	delete capture_;
}

// Initialize
void Context::Init(Settings* settings,
                   const char* window_title) {
	capture_->Init(settings, window_title);

	// start capture thread
	pthread_attr_t thread_attributes;
	pthread_attr_init(&thread_attributes);
	pthread_attr_setdetachstate(&thread_attributes, PTHREAD_CREATE_JOINABLE);
	int ret = pthread_create(&thread_param_, &thread_attributes,
	                         Capture::ThreadStart, capture_);
	if (0 != ret) {
		fprintf(stderr, "ERROR\n");
	}
}

void Context::Start() {
	capture_->StartCapturing();
}

void Context::Stop() {
	capture_->StopCapturing();
}

void Context::Destroy() {
	capture_->CloseDevice();

	void* p_ret = NULL;
	pthread_join(thread_param_, &p_ret);
}
