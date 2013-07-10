#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include "bitmap.h"
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

#define CLEAR(x) memset((void*)&x, 0, sizeof(x))
#define BMP_PATH "./fb_565.bmp"
int main(int argc, char **argv)
{
	int fd;
	char *fb_buf;
	FILE *fp;
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	BITMAPFILEHEADER bmp_head;
	BITMAPINFOHEADER bmp_info;
	unsigned int     bmp_rgb_mask[4];
	
	if (argc!=2)
	{
		printf("%s /dev/fb0\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if (fd<0) {
		perror("open:");
		return -1;
	}

	ioctl(fd, FBIOGET_VSCREENINFO, &vinfo);
	ioctl(fd, FBIOGET_FSCREENINFO, &finfo);
	printf("[FB]x:%d y:%d\n", vinfo.xres, vinfo.yres);
	fb_buf = (char*)mmap(NULL, finfo.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	vt_set_mode(1);

	// prepare bmp
	if (access(BMP_PATH, 0)==0)
		remove(BMP_PATH);

	fp = fopen(BMP_PATH, "w+");
	if (fp<0)
		return -1;

	CLEAR(bmp_head);
	CLEAR(bmp_info);
	CLEAR(bmp_rgb_mask);

	bmp_head.bfType = 'B' | 'M' <<8;
	bmp_head.bfSize = sizeof(bmp_info) + sizeof(bmp_head) + finfo.smem_len + sizeof(int)*4;
	bmp_head.bfOffBits = sizeof(bmp_info) + sizeof(bmp_head) + sizeof(int)*4;

	bmp_info.biSize  = sizeof(bmp_info);
	bmp_info.biWidth = vinfo.xres;
	bmp_info.biHeight = vinfo.yres;
	bmp_info.biBitCount = 16;
	bmp_info.biCompression =  BI_BITFIELDS;
	bmp_info.biSizeImage = finfo.smem_len;

	fwrite(&bmp_head, sizeof(bmp_head), 1, fp);
	fwrite(&bmp_info, sizeof(bmp_info), 1, fp);

	// 565 mask
	bmp_rgb_mask[0] = 0xF800;
	bmp_rgb_mask[1] = 0x07E0;
	bmp_rgb_mask[2] = 0x001F;
	fwrite(bmp_rgb_mask, sizeof(bmp_rgb_mask), 1, fp);

	// write image data
	for (int i=0; i<bmp_info.biHeight; i++)
		fwrite(fb_buf+(bmp_info.biHeight-i)*bmp_info.biWidth*2, 
		bmp_info.biWidth*2, 1, fp);



	if (fb_buf)
		munmap(fb_buf, finfo.smem_len);
	close(fd);
	fclose(fp);
	return 0;
}