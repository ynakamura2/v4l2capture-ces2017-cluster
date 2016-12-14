#ifndef CONTEXT_H_
#define CONTEXT_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <pthread.h>

class Settings;
class Capture;

class Context {
 public:
	Context();
	~Context();

	void Init(Settings* settings, const char* window_title);

	void Start();
	void Stop();
	void Destroy();

 private:
	Capture*  capture_;
	pthread_t thread_param_;

 private:
	DISALLOW_COPY_AND_ASSIGN(Context);
};
#endif // CONTEXT_H_
