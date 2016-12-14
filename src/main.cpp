#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

#include <unistd.h>
#include <signal.h>

#include "context.h"
#include "settings.h"

// Capture start method
void startCaptureVideo(Context* const context) {
	context->Start();
}

// Capture stop method
void stopCaptureVideo(Context* const context) {
	context->Stop();
}

// sigint listener
static volatile int gRun = 0;
static void sigintListener(int signum) {
	(void)signum;
	gRun = 0;
}

int main(int argc, char *argv[]) {
	// Init
	Context* context = new(std::nothrow) Context();
	if (NULL == context) { std::abort(); return -1; }

	Settings* setting = new(std::nothrow) Settings();
	if (NULL == setting) { std::abort(); return -1; }

	const char* window_title = "ivi_sample_capture";

        const char* device_name = "/dev/video0";
	if (argc == 2)
		device_name = argv[1];

	setting->SetWidth(640);
	setting->SetHeight(480);
	setting->SetDeviceName(device_name);

	context->Init(setting, window_title);

	startCaptureVideo(context);

	struct sigaction sigint;

	sigint.sa_handler = sigintListener;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sigint, NULL);

	gRun = 1;
	while (gRun == 1) {
		usleep(200*1000);
	}

	stopCaptureVideo(context);

	context->Destroy();

	return 0;
}
