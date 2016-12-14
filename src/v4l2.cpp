#include "v4l2.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <new>

const int32_t V4L2::MAX_BUFFER = 4;

V4L2::V4L2()
	: timestamp_(),
	  m_LastBuffer(),
	  texture_buffer_(),
	  m_Buffers(),
	  m_width(640),
	  m_height(480),
	  m_format(V4L2_PIX_FMT_YUV422P),
	  m_fd()
{
}

V4L2::~V4L2() {
}

void V4L2::InitDevice(uint32_t width, uint32_t height, uint32_t format) {
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(struct v4l2_format));

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = format;
	fmt.fmt.pix.field       = V4L2_FIELD_ALTERNATE;

#if DEBUG_
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>>>>>>>>>>>>>> SET DEVICE width       : result -> %d \n", fmt.fmt.pix.width);
	printf(">>>>>>>>>>>>>>> SET DEVICE height      : result -> %d \n", fmt.fmt.pix.height);
	printf(">>>>>>>>>>>>>>> SET DEVICE pixelformat : result -> %x \n", fmt.fmt.pix.pixelformat);
	printf(">>>>>>>>>>>>>>> SET DEVICE field       : result -> %x \n", fmt.fmt.pix.field);
	printf(">>>>>>>>>>>>>>> !! NOTE !!\n");
	printf(">>>>>>>>>>>>>>> V4L2_PIX_FMT_YUV422P : %x \n", V4L2_PIX_FMT_YUV422P);
	printf(">>>>>>>>>>>>>>> V4L2_PIX_FMT_YUYV    : %x \n", V4L2_PIX_FMT_YUYV);
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
#endif
#if 1
	if (-1 == xioctl(VIDIOC_S_FMT, &fmt)) {
		errno_exit("VIDIOC_S_FMT");
	}
#endif
#if DEBUG_
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>>>>>>>>>>>>>> FIXED width       : result -> %d \n", fmt.fmt.pix.width);
	printf(">>>>>>>>>>>>>>> FIXED height      : result -> %d \n", fmt.fmt.pix.height);
	printf(">>>>>>>>>>>>>>> FIXED pixelformat : result -> %x \n", fmt.fmt.pix.pixelformat);
	printf(">>>>>>>>>>>>>>> FIXED field       : result -> %x \n", fmt.fmt.pix.field);
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
#endif

	/* Note VIDIOC_S_FMT may change width and height. */
	m_width  = fmt.fmt.pix.width;
	m_height = fmt.fmt.pix.height;
	m_format = fmt.fmt.pix.pixelformat;

	/* Buggy driver paranoia. */
	uint32_t min_bytesperline = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min_bytesperline) {
		fmt.fmt.pix.bytesperline = min_bytesperline;
	}

	uint32_t min_sizeimage = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min_sizeimage) {
		fmt.fmt.pix.sizeimage = min_sizeimage;
	}

	InitBuffers();
}

void V4L2::UnInitDevice(void) {
	for (uint32_t i = 0; i < MAX_BUFFER; ++i) {
		if (-1 == munmap(m_Buffers[i].start, m_Buffers[i].length)) {
			errno_exit("munmap");
		}
	}

	delete [] m_Buffers;
}

void V4L2::OpenDevice(const char* device_name) {
	struct stat st;
	if (-1 == stat(device_name, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
		        device_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", device_name);
		exit(EXIT_FAILURE);
	}

	m_fd = open(device_name, O_RDWR /* required */  | O_NONBLOCK, 0);
	if (-1 == m_fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
		        device_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "----- V4L2:: Open '%s', m_fd:%d\n",
	        device_name, m_fd);
}

void V4L2::CloseDevice() {
	if (-1 == close(m_fd)) {
		errno_exit("close");
	}

	m_fd = -1;
}

void V4L2::StopCapturing() {
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(VIDIOC_STREAMOFF, &type)) {
		errno_exit("VIDIOC_STREAMOFF");
	}
}

void V4L2::StartCapturing() {
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(VIDIOC_STREAMON, &type)) {
		errno_exit("VIDIOC_STREAMON");
	}
}

int V4L2::mainloop() {
	fd_set fds;
	struct timeval tv;
	int r;

	while (true) {
		// clear fd_set
		FD_ZERO(&fds);
		FD_SET(m_fd, &fds);

		// Timeout.
		// tv.tv_sec  = select_timeout_;
		tv.tv_sec  = 2;
		tv.tv_usec = 0;

		r = select(m_fd + 1, &fds, NULL, NULL, &tv);
		if (-1 == r) {
			// retry select() when errno is EINTR(= get something signal)
			if (EINTR == errno) {
				perror("select interrupt");
				continue;
			}
			// get other error, exit
			errno_exit("select");
		} else if (0 == r) {
			// select timeout
			perror("select timeout");
			return -1;
		} else {
			// no error & do capture
			if (0 == read_frame()) {
				break;
			}
		}
		// when read_frame() is EAGAIN, retry select().
	}

	return 0;
}

void V4L2::syncmainloop() {
	if (-1 == xioctl(VIDIOC_QBUF, &texture_buffer_)) {
		errno_exit("VIDIOC_QBUF");
	}
}

int32_t V4L2::InitBuffers(void) {
	// request buffer
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));

	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	req.count  = MAX_BUFFER;

	if (-1 == xioctl(VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "this device does not support memory mapping\n");
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	// create buffer containers
	m_Buffers = new(std::nothrow) Buffer[req.count]();
	if (NULL == m_Buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	InitMMap(req.count);

	struct v4l2_buffer buf;
	for (int32_t i = 0; i < MAX_BUFFER; ++i) {
		memset(&buf, 0, sizeof(buf));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(VIDIOC_QBUF, &buf)) {
			errno_exit("VIDIOC_QBUF");
		}
	}

	return 0;
}

int32_t V4L2::InitMMap(uint32_t request_count) {
	struct v4l2_buffer buf;
	for (uint32_t i = 0; i < request_count; ++i) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(VIDIOC_QUERYBUF, &buf)) {
			errno_exit("VIDIOC_QUERYBUF");
		}

		m_Buffers[i].length = buf.length;
		m_Buffers[i].start = mmap(NULL /* start anywhere */ ,
		                          buf.length,
		                          PROT_READ | PROT_WRITE /* required */ ,
		                          MAP_SHARED /* recommended */ ,
		                          m_fd, buf.m.offset);
		if (MAP_FAILED == m_Buffers[i].start) {
			errno_exit("mmap");
		}
	}
	return 0;
}

int32_t V4L2::xioctl(int32_t request, void *arg) {
	int32_t r;

	do {
		r = ioctl(m_fd, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}

int32_t V4L2::read_frame(void) {
	memset(&texture_buffer_, 0, sizeof(struct v4l2_buffer));

	texture_buffer_.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	texture_buffer_.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(VIDIOC_DQBUF, &texture_buffer_)) {
		switch (errno) {
		case EAGAIN:
			return -1;

		case EIO:
			/* Could ignore EIO, see spec. */
			/* fall through */

		default:
			errno_exit("VIDIOC_DQBUF");
		}
	}

	assert(texture_buffer_.index < MAX_BUFFER);

	m_LastBuffer = m_Buffers[texture_buffer_.index].start;

	return 0;
}

void V4L2::errno_exit(const char *s) {
	perror(s);
	abort();
}
