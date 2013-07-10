#ifndef _FB_API_H_
#define _FB_API_H_
#include <linux/fb.h>

// FB
typedef struct _device_fb
{
	int fd;
	unsigned char *mem;
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;

}DeviceFB;

/** 
 * @brief ������ʾģʽ
 *
 */
int vt_set_mode(int graphics);

/**
 * @brief ���豸
 */
DeviceFB *fb_open(const char *path);

/**
 * @brief �ر��豸 
 */
int fb_close(DeviceFB *device);

#endif
