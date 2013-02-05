#ifdef USE_V4L2

#include "viddev.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libv4l2.h>
#include <linux/videodev2.h>

namespace {

void showPixFormat(const struct v4l2_pix_format& pix_fmt) {
  char fmt_buf[5];
  fmt_buf[0] = (pix_fmt.pixelformat & 0xFF);
  fmt_buf[1] = (pix_fmt.pixelformat >> 8) & 0xFF;
  fmt_buf[2] = (pix_fmt.pixelformat >> 16) & 0xFF;
  fmt_buf[3] = (pix_fmt.pixelformat >> 24) & 0xFF;
  fmt_buf[4] = 0;
  fprintf(stderr, "w=%u h=%u fmt=%s field=%u bypl=%u sz=%u cs=%u priv=%u\n",
          pix_fmt.width, pix_fmt.height, fmt_buf,
          pix_fmt.field, pix_fmt.bytesperline, pix_fmt.sizeimage,
          pix_fmt.colorspace, pix_fmt.priv);
}

void showCurrentInput(int fd) {
  struct v4l2_input input;
  int input_index;
  if (v4l2_ioctl(fd, VIDIOC_G_INPUT, &input_index) < 0) {
    perror("v4l2_ioctl VIDIOC_G_INPUT");
    exit(EXIT_FAILURE);
  }

  memset(&input, 0, sizeof(input));
  input.index = input_index;

  if (v4l2_ioctl(fd, VIDIOC_ENUMINPUT, &input) < 0) {
    perror ("v4l2_ioctl VIDIOC_ENUMINPUT");
    exit(EXIT_FAILURE);
  }

  fprintf(stderr, "Current input: %s @%d\n", input.name, input_index);
}

void adjustStandard(int fd) {
  v4l2_std_id std_id;
  struct v4l2_standard standard;

  if (v4l2_ioctl(fd, VIDIOC_G_STD, &std_id) < 0) {
    /* Note when VIDIOC_ENUMSTD always returns EINVAL this
       is no video device or it falls under the USB exception,
       and VIDIOC_G_STD returning EINVAL is no error. */
    perror("v4l2_ioctl VIDIOC_G_STD");
    exit(EXIT_FAILURE);
  }

  memset (&standard, 0, sizeof(standard));
  standard.index = 0;

  while (v4l2_ioctl(fd, VIDIOC_ENUMSTD, &standard) == 0) {
    if (standard.id & std_id) {
      fprintf(stderr, "Current video standard: %s\n", standard.name);
      errno = 0;
      break;
    }

    standard.index++;
  }

  /* EINVAL indicates the end of the enumeration, which cannot be
     empty unless this device falls under the USB exception. */
  if (errno == EINVAL /* || standard.index == 0 */) {
    perror("v4l2_ioctl VIDIOC_ENUMSTD");
    exit(EXIT_FAILURE);
  }

  v4l2_std_id new_std_id = V4L2_STD_NTSC_M_JP;
  if (v4l2_ioctl(fd, VIDIOC_S_STD, &new_std_id) < 0) {
    perror("v4l2_ioctl VIDIOC_S_STD");
    exit(EXIT_FAILURE);
  }
}

}  // anonymous namespace

VidDev::VidDev(const char* dev) {
  fd_ = v4l2_open(dev, O_RDWR);
  if (fd_ < 0) {
    perror("v4l2_open");
    exit(EXIT_FAILURE);
  }

  struct v4l2_capability cap;
  if (v4l2_ioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) {
    perror("v4l2_ioctl VIDIOC_QUERYCAP");
    exit(EXIT_FAILURE);
  }

  struct v4l2_format format;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  const struct v4l2_pix_format& pix_fmt = format.fmt.pix;
  if (v4l2_ioctl(fd_, VIDIOC_G_FMT, &format) < 0) {
    perror("v4l2_ioctl VIDIOC_G_FMT");
    exit(EXIT_FAILURE);
  }
  showPixFormat(pix_fmt);

  //format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
  if (v4l2_ioctl(fd_, VIDIOC_S_FMT, &format) < 0) {
    perror("v4l2_ioctl VIDIOC_S_FMT");
    exit(EXIT_FAILURE);
  }
  showPixFormat(pix_fmt);

  width_ = pix_fmt.width;
  height_ = pix_fmt.height;

  showCurrentInput(fd_);

  adjustStandard(fd_);

  initBuffers();

  ok_ = true;
}

VidDev::~VidDev() {
  // TODO(hamaji): Free resource.
}

void VidDev::initBuffers() {
  struct v4l2_requestbuffers reqbuf;
  memset(&reqbuf, 0, sizeof (reqbuf));
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  reqbuf.count = 20;

  if (v4l2_ioctl(fd_, VIDIOC_REQBUFS, &reqbuf) < 0) {
    if (errno == EINVAL)
      fprintf(stderr,
              "Video capturing or mmap streaming is not supported\n");
    else
      perror("v4l2_ioctl VIDIOC_REQBUFS");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "Buffer count=%u type=%u memory=%u\n",
          reqbuf.count, reqbuf.type, reqbuf.memory);

  if (reqbuf.count < 5) {
    fprintf(stderr, "Not enough buffer memory: %u\n", reqbuf.count);
    exit(EXIT_FAILURE);
  }

  buffers_ = (Buffer*)calloc(reqbuf.count, sizeof(*buffers_));
  assert(buffers_ != NULL);

  fprintf(stderr, "mmap:");
  for (size_t i = 0; i < reqbuf.count; i++) {
    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof (buffer));
    buffer.type = reqbuf.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;
    if (v4l2_ioctl(fd_, VIDIOC_QUERYBUF, &buffer) < 0) {
      perror("VIDIOC_QUERYBUF");
      exit(EXIT_FAILURE);
    }

    buffers_[i].length = buffer.length; /* remember for munmap() */
    buffers_[i].start = (char*)mmap(NULL, buffer.length,
                                    PROT_READ | PROT_WRITE, /* recommended */
                                    MAP_SHARED,             /* recommended */
                                    fd_, buffer.m.offset);
    // This document says rmask and bmask should be swapped, but hmm?
    // http://linuxtv.org/downloads/v4l-dvb-apis/packed-rgb.html
    buffers_[i].surface = SDL_CreateRGBSurfaceFrom(buffers_[i].start,
                                                   width_, height_, 16,
                                                   width_ * 2,
                                                   31 << 11, 63 << 5, 31, 0);
    assert(buffers_[i].surface);

    fprintf(stderr, " %lu:%p+%lu", i, buffers_[i].start, buffers_[i].length);
    if (MAP_FAILED == buffers_[i].start) {
      /* If you do not exit here you should unmap() and free()
         the buffers mapped so far. */
      perror("mmap");
      exit(EXIT_FAILURE);
    }
  }
  fprintf(stderr, "\n");

  for (size_t i = 0; i < reqbuf.count; i++) {
    struct v4l2_buffer buffer;
    memset(&buffer, 0, sizeof (buffer));
    buffer.type = reqbuf.type;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    if (v4l2_ioctl(fd_, VIDIOC_QBUF, &buffer) < 0) {
      perror("VIDIOC_QBUF");
      exit(EXIT_FAILURE);
    }
  }

  if (v4l2_ioctl(fd_, VIDIOC_STREAMON, &reqbuf.type) < 0) {
    perror("v4l2_ioctl VIDIOC_STREAMON");
    exit(EXIT_FAILURE);
  }
}

SDL_Surface* VidDev::getNextFrame() {
  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof (buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;

  if (v4l2_ioctl(fd_, VIDIOC_DQBUF, &buffer) < 0) {
    perror("VIDIOC_DQBUF");
    exit(EXIT_FAILURE);
  }

  const Buffer& buf = buffers_[buffer.index];

  if (v4l2_ioctl(fd_, VIDIOC_QBUF, &buffer) < 0) {
    perror("VIDIOC_QBUF");
    exit(EXIT_FAILURE);
  }

  return buf.surface;
}

#endif
