#ifndef CAPTURE_H_
#define CAPTURE_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <pthread.h>

class Settings;
class Wayland;
class EGL;
class V4L2;
class ImageProcess;

class Capture {
 private:
	enum E_CAPTURE_STATUS {
		CAPTURE_START,
		CAPTURE_STOP,
		CAPTURE_DESTROY
	};

 public:
	Capture();

	~Capture();

	// Init
	void Init(Settings* const Config,
	          const char* window_title);

	// Update Flags
	void StartCapturing();
	void StopCapturing();
	void CloseDevice();
	void SwitchSource();
	void SwitchWideMode();

	// wrapper: static function pointer for pthread
	static void *ThreadStart(void* const obj) {
		Capture* const c = static_cast<Capture*>(obj);
		c->CaptureMain();
		return NULL;
	}

 private:
	E_CAPTURE_STATUS GetCaptureStatus();

	void CaptureMain();
	void DoCapturing();

	void GetTime(double &time);

	void InitContext();
	void InitV4L2();
	void CloseV4L2();

	inline void MutexLock(pthread_mutex_t* const mutex_t);
	inline void MutexUnLock(pthread_mutex_t* const mutex_t);

 private:
	volatile E_CAPTURE_STATUS capture_status_;

	pthread_t vcThreadParam;

	pthread_mutex_t mutex_capture_status_;

	const char* window_title_;

	Settings *vcConfig;
	Wayland  *vcWayland;
	V4L2     *vcV4L2;
	EGL      *vcEGL;
	ImageProcess *vcImageProcess;

 private:
	DISALLOW_COPY_AND_ASSIGN(Capture);
};
#endif // CAPTURE_H_
