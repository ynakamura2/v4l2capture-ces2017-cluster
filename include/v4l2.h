#ifndef V4L2_H_
#define V4L2_H_

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#include <stdint.h>
#include <stdio.h>
#include <linux/videodev2.h>

class V4L2 {
  private:
	struct Buffer {
		void   *start;
		size_t length;
	};

	static const int32_t MAX_BUFFER;

  public:
	// Constructor
	V4L2();
	// Destructor
	virtual ~V4L2();

	void InitDevice(uint32_t width, uint32_t height, uint32_t format);
	void UnInitDevice();
	void OpenDevice(const char* device_name);
	void CloseDevice();
	void StartCapturing();
	void StopCapturing();
	int mainloop();
	void syncmainloop();

	void* const GetBuffer()  const { return m_LastBuffer; }

	// Fixed width and height are able to get after calling InitDevice()
	int32_t GetFixedWidth()  const { return m_width; }
	int32_t GetFixedHeight() const { return m_height; }

  private:
	int32_t InitBuffers();

	int32_t InitMMap(uint32_t request_count);
	int32_t InitDMA(uint32_t request_count);

	int32_t xioctl(int32_t request, void *arg);
	int32_t read_frame(void);
	void    errno_exit(const char *s);

  private:
	double timestamp_;

	void* m_LastBuffer;

	struct v4l2_buffer texture_buffer_;

	struct Buffer *m_Buffers;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_format;

	int32_t m_fd;

  private:
	DISALLOW_COPY_AND_ASSIGN(V4L2);
};
#endif // V4L2_H_
