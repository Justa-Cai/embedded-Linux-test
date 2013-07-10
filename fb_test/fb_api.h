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
 * @brief 设置显示模式
 *
 */
int vt_set_mode(int graphics);

/**
 * @brief 打开设备
 */
DeviceFB *fb_open(const char *path);

/**
 * @brief 关闭设备 
 */
int fb_close(DeviceFB *device);

#endif
