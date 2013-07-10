#include <stdio.h>
#include "fb_api.h"

int main()
{
	DeviceFB *pDeviceFb = fb_open("/dev/fb0");
	if (pDeviceFb)
	{
		printf("w:%d h:%d bpp:%d\n",
			pDeviceFb->vinfo.xres,
			pDeviceFb->vinfo.yres,
			pDeviceFb->vinfo.bits_per_pixel);
			
		fb_close(pDeviceFb);
	}
	return 0;
}
