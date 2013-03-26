#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "sdk/include/libv4l2.h"
#include "sdk/include/libv4lconvert.h"

#include <linux/videodev2.h>
#include <linux/fb.h>
#include <linux/kd.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
	void   *start;
	size_t length;
};

static void xioctl(int fh, int request, void *arg)
{
	int r;

	do {
		r = v4l2_ioctl(fh, request, arg);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	if (r == -1) {
		fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static int vt_set_mode(int graphics)
{
	int fd, r;
	fd = open("/dev/tty0", O_RDWR | O_SYNC);
	if (fd < 0)
		return -1;
	r = ioctl(fd, KDSETMODE, (void*) (graphics ? KD_GRAPHICS : KD_TEXT));
	close(fd);
	return r;
}
static struct fb_fix_screeninfo finfo;
static struct fb_var_screeninfo vinfo;
int main(int argc, char **argv)
{
	struct v4l2_format              fmt;
	struct v4l2_buffer              buf;
	struct v4l2_requestbuffers      req;
	enum v4l2_buf_type              type;
	fd_set                          fds;
	struct timeval                  tv;
	int                             r, fd = -1;
	unsigned int                    i, n_buffers;
	char                            *dev_name = "/dev/video0";
	char                            out_name[256];
	FILE                            *fout;
	struct buffer                   *buffers;

	struct v4lconvert_data          *v4l2_conv;

	struct v4l2_format              dest_fmt;
	unsigned char *pOutBuf, *pOutBufTmp;
	int out_length, yuv_length;
	int ret;

	int fb_fd;
	unsigned char *fb_buf;

	fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd>0)
	{
		ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
		ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
		printf("[FB]x:%d y:%d bits_per_pixel:%d\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);
		fb_buf = (char*)mmap(NULL, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
		vt_set_mode(1);
	}


	fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		perror("Cannot open device");
		exit(EXIT_FAILURE);
	}

	v4l2_conv = v4lconvert_create(fd);

	CLEAR(fmt);
	CLEAR(dest_fmt);
#if 1
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_G_FMT, &fmt);
//	fmt.fmt.pix.width       = 240;
//	fmt.fmt.pix.height      = 320;

	xioctl(fd, VIDIOC_S_FMT, &fmt);
	yuv_length = fmt.fmt.pix.width* fmt.fmt.pix.height*2;

	dest_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//xioctl(fd, VIDIOC_G_FMT, &dest_fmt);
	dest_fmt.fmt.pix.width       = 320;
	dest_fmt.fmt.pix.height      = 240;
	dest_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
	dest_fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	out_length = fmt.fmt.pix.width*fmt.fmt.pix.height*2;
	pOutBuf = (unsigned char*)malloc(out_length);
#else
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 640;
	fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	xioctl(fd, VIDIOC_S_FMT, &fmt);
	if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
		printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
		exit(EXIT_FAILURE);
	}
	if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
		printf("Warning: driver is sending image at %dx%d\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height);
#endif

	fprintf(stderr, "[video]w:%d h:%d pixelformat:%x\n", fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);

	CLEAR(req);
	req.count = 2;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	xioctl(fd, VIDIOC_REQBUFS, &req);

	buffers = calloc(req.count, sizeof(*buffers));
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		xioctl(fd, VIDIOC_QUERYBUF, &buf);

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start) {
			perror("mmap");
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < n_buffers; ++i) {
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		xioctl(fd, VIDIOC_QBUF, &buf);
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	xioctl(fd, VIDIOC_STREAMON, &type);
	for (i = 0; i < 5; i++) {
		do {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(fd + 1, &fds, NULL, NULL, &tv);
		} while ((r == -1 && (errno = EINTR)));
		if (r == -1) {
			perror("select");
			return errno;
		}

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		xioctl(fd, VIDIOC_DQBUF, &buf);

		//if (buf.bytesused != )
		printf("buf.bytesused:%d\n", buf.bytesused);
		if (buf.bytesused != yuv_length)
			goto _end;
		
		pOutBufTmp = pOutBuf;
		ret = v4lconvert_convert(v4l2_conv, &fmt, &dest_fmt, buffers[buf.index].start, buf.bytesused,
			pOutBufTmp, out_length);
		if (ret>0)
		{
			printf("ret:%d\n", ret);
			memcpy(fb_buf, pOutBuf, out_length);
#if 0
			sprintf(out_name, "out%03d.ppm", i);
			fout = fopen(out_name, "w");
			if (!fout) {
				perror("Cannot open image");
				exit(EXIT_FAILURE);
			}
			fprintf(fout, "P6\n%d %d 255\n",
				fmt.fmt.pix.width, fmt.fmt.pix.height);
			// convert
			fwrite(pOutBuf, out_length, 1, fout);
			fclose(fout);
#endif
		}
		else 
			printf("error:%s\n", v4lconvert_get_error_message(v4l2_conv));

_end:
		xioctl(fd, VIDIOC_QBUF, &buf);
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_STREAMOFF, &type);
	for (i = 0; i < n_buffers; ++i)
		v4l2_munmap(buffers[i].start, buffers[i].length);
	v4lconvert_destroy(v4l2_conv);
	v4l2_close(fd);
	if (fb_buf)
		munmap(fb_buf, finfo.smem_len);
	close(fb_fd);
	return 0;
}
