#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/kd.h>

#include <fcntl.h>

#include "fb_api.h"
//////////////////////////////////////////////////////////////////////////
// fb functions 
//////////////////////////////////////////////////////////////////////////


int vt_set_mode(int graphics)
{
	int fd, r;
	fd = open("/dev/tty0", O_RDWR | O_SYNC);
	if (fd < 0)
		return -1;
	r = ioctl(fd, KDSETMODE, (void*) (graphics ? KD_GRAPHICS : KD_TEXT));
	close(fd);
	return r;
}

DeviceFB *fb_open(const char *path)
{
	DeviceFB *device;
	int fd = open(path, O_RDWR);
	if (fd>0)
	{
		device = (DeviceFB*)calloc(1, sizeof(*device));
		ioctl(fd, FBIOGET_VSCREENINFO, &device->vinfo);
		ioctl(fd, FBIOGET_FSCREENINFO, &device->finfo);
	//	printf("[FB]x:%d y:%d bits_per_pixel:%d\n", device->vinfo.xres, device->vinfo.yres, device->vinfo.bits_per_pixel);
		device->mem= (unsigned char*)mmap(NULL, device->finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		vt_set_mode(1);
		device->fd = fd;
		return device;
	}
	return NULL;
}

int fb_close(DeviceFB *device)
{
	if (device->fd>0)
	{
		if (device->mem)
			munmap(device->mem, device->finfo.smem_len);
		close(device->fd);
		free(device);
		return 0;
	}
	return -1;
}