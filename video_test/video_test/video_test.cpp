// video_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//#define YUV_PATH     "d:\\tmp\\yuv_image\\x.yuv"
#if 1
#define YUV_PATH     "v:\\mini2440\\rootfs.nfs\\x.yuv"
#define YUV_HEIGHT   240//480	
#define YUV_WIDTH    320//640
#else
#define YUV_PATH     "d:\\tmp\\yuv_image\\yuv422.yuv"
#define YUV_HEIGHT   144  //640
#define YUV_WIDTH    176  //480
#endif

#define YUV_LENGTH        (YUV_HEIGHT*YUV_WIDTH*2)
#define BMP_DATA_SIZE     (YUV_HEIGHT*YUV_WIDTH*3)

#define BMP_PATH     "d:\\tmp\\yuv_image\\x.bmp"
#define BMP_LINE_LENGTH    (YUV_WIDTH*3)
static int g_bmp_data_size = BMP_DATA_SIZE;

#define CLEAR(x) memset((void*)&x, 0, sizeof(x))
//产生随机数，其中范围为1~max
unsigned int Random(int max)
{
	errno_t err;
	unsigned int number;
	err = rand_s(&number);
	if(err != 0)
	{
		return 0;//产生失败，返回0
	}
	return (unsigned int)((double)number / ((double)UINT_MAX + 1) * double(max)) + 1;
}

char *ReadAllFromImage(const char *path, off_t *len)
{
	char *ptr;
	int fd;
	off_t length;
	fd = open(path, O_RDONLY);
	if (fd<0)
		return NULL;

	length = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	ptr = (char*)malloc(length);

	read(fd, ptr, length);
	*len = length;
	return ptr;
}

// UYVY
// Cb0 Y0 Cr0 Y1 |   Cb1 Y1 Cr1 Y2
// R = 1.1665*(Y-16.5) - 0.0483*(Cb - 128.5) + 1.6455*(Cr - 128.5)
// G = 1.1665*(Y-16.5) - 0.3922*(Cb - 128.5) + 0.8151*(Cr - 128.5)
// B = 1.1665*(Y-16.5) - 2.0218*(Cb - 128.5) - 0.0013*(Cr - 128.5)
// 每4个字节为1组 转换为2组RGb数据
// http://wenku.baidu.com/view/eaa93536ee06eff9aef80775.html
#if 0
#define CAL_YUV_R(y, cb, cr) (1.1665*(y-16.5) - 0.0483*(cb - 128.5) + 1.6455*(cr - 128.5))
#define CAL_YUV_G(y, cb, cr) (1.1665*(y-16.5) - 0.3922*(cb - 128.5) + 0.8151*(cr - 128.5))
#define CAL_YUV_B(y, cb, cr) (1.1665*(y-16.5) - 2.0218*(cb - 128.5) - 0.0013*(cr - 128.5))
#else
//#define CAL_YUV_R(y, cb, cr) (y + 1.4075*(cr-128))
//#define CAL_YUV_G(y, cb, cr) (y - 0.3455 * (cb - 128) - (0.7169 * (cr - 128)))
//#define CAL_YUV_B(y, cb, cr) (y + 1.7790 * (cb - 128))

#define CAL_YUV_R(y, u, v) (y+ 0 * u + 1.13983 * v)
#define CAL_YUV_G(y, u, v) (y -0.39465 * u + -0.58060 * v)
#define CAL_YUV_B(y, u, v) (y -0.03211 * u + 0 * v)
#endif

static inline unsigned char clamp(float v)
{
	if (v>255)
		return 255;
	else if (v<0)
		return 0;
	else
		return (unsigned char)v;
}

enum 
{
	YUV2_RGB_888,
	YUV2_RGB_565,
};

char* yuv2rgb(const unsigned char *pYuv, int length, int type)
{
	char *pBuf, *ptr;
	unsigned char r,g,b;
	unsigned short v;
	float cb0, y0, cr0, y1;

	pBuf = (char *)malloc(g_bmp_data_size);
	memset(pBuf, 0x0, g_bmp_data_size);
	ptr = pBuf;
	for (int i=0; i<length;)
	{
		y0  = (unsigned char)pYuv[i++];
		cb0 = (unsigned char)pYuv[i++];
		y1  = (unsigned char)pYuv[i++];
		cr0 = (unsigned char)pYuv[i++];
		cb0 = 0;
		cr0 = 0;

		if (type == YUV2_RGB_888)
		{
			*ptr++ = clamp(CAL_YUV_R(y0, cb0, cr0));
			*ptr++ = clamp(CAL_YUV_G(y0, cb0, cr0));
			*ptr++ = clamp(CAL_YUV_B(y0, cb0, cr0));

			*ptr++ = clamp(CAL_YUV_R(y1, cb0, cr0));
			*ptr++ = clamp(CAL_YUV_G(y1, cb0, cr0));
			*ptr++ = clamp(CAL_YUV_B(y1, cb0, cr0));
		}
		else if (type == YUV2_RGB_565)
		{
			//             11             7  6    5         0
			// R5 R4 R3 R2 R1 G6 G5 G4 G3 G2 G1 B5 B4 B3 B2 B1 
			r = clamp(CAL_YUV_R(y0, cb0, cr0));
			g = clamp(CAL_YUV_G(y0, cb0, cr0));
			b = clamp(CAL_YUV_B(y0, cb0, cr0));
			v = 0;
			v = (((r>>3)&0x1f)<<11 | (g>>2&0x2f)<<6 | ((b>>3)&0x1f));
			*ptr++ = v>>8 &0xff;
			*ptr++ = v&0xff;

			r = clamp(CAL_YUV_R(y1, cb0, cr0));
			g = clamp(CAL_YUV_G(y1, cb0, cr0));
			b = clamp(CAL_YUV_B(y1, cb0, cr0));
			v = 0;
			v = (((r>>3)&0x1f)<<11 | (g>>2&0x2f)<<6 | ((b>>3)&0x1f));
			*ptr++ = v>>8 &0xff;
			*ptr++ = v&0xff;

		}
	}


	return pBuf;
}

int WriteIntoBmp(char *ptr, int bitcount)
{
	FILE *fp;
	char *pBuf;

	BITMAPFILEHEADER bmp_head;
	BITMAPINFOHEADER  bmp_info;
	if (access(BMP_PATH, 0)==0)
		remove(BMP_PATH);

	fp = fopen(BMP_PATH, "w+");
	if (fp<0)
		return -1;

	CLEAR(bmp_head);
	CLEAR(bmp_info);

	bmp_head.bfType = 'B' | 'M' <<8;
	bmp_head.bfSize = sizeof(bmp_info) + sizeof(bmp_head) + YUV_WIDTH*YUV_HEIGHT*bitcount/8;
	bmp_head.bfOffBits = sizeof(bmp_info) + sizeof(bmp_head);

	bmp_info.biSize  = sizeof(bmp_info);
	bmp_info.biWidth = YUV_WIDTH;
	bmp_info.biHeight = YUV_HEIGHT;
	bmp_info.biBitCount = bitcount;
	bmp_info.biSizeImage = YUV_WIDTH*YUV_HEIGHT*bitcount/8;
	bmp_info.biXPelsPerMeter = bmp_info.biYPelsPerMeter = 0; // DIP？

	fwrite(&bmp_head, sizeof(bmp_head), 1, fp);
	fwrite(&bmp_info, sizeof(bmp_info), 1, fp);


#if 0
	pBuf = (char *)malloc(BMP_DATA_SIZE);
	memset(pBuf, 0x555555, BMP_DATA_SIZE);
	for (int i=0; i<YUV_HEIGHT; i++)
		for (int j=0; j<YUV_WIDTH; j++)
		{
			pBuf[i*YUV_WIDTH*3 + j*3] = Random(255);
			pBuf[i*YUV_WIDTH*3 + j*3+1] = Random(255);
			pBuf[i*YUV_WIDTH*3 + j*3+2] = Random(255);
		}
		free(pBuf);
		fwrite(pBuf, BMP_DATA_SIZE, 1, fp);
#else
	for (int i=0; i<YUV_HEIGHT; i++)
		fwrite(ptr+(YUV_HEIGHT-i)*YUV_WIDTH*bitcount/8, YUV_WIDTH*bitcount/8, 1, fp);
#endif

	fclose(fp);

	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	int fd;
	char *pYuv, *pBmp;
	off_t length;

	pYuv = ReadAllFromImage(YUV_PATH, &length);
	if (pYuv)
	{
#if 0
		for (int i=YUV_WIDTH*2*(YUV_HEIGHT-20) ; i<YUV_WIDTH*2* YUV_HEIGHT;)
		{
			pYuv[i++] = 0;
			pYuv[i++] = 0;
			pYuv[i++] = 0;
			pYuv[i++] = 0;
		}
#endif
		int type = YUV2_RGB_888;
		int bitcount = 24;
#if 1
		type = YUV2_RGB_565;
		bitcount = 16;
#endif
		g_bmp_data_size = bitcount*YUV_HEIGHT*YUV_WIDTH;

		pBmp = yuv2rgb((const unsigned char *)pYuv, YUV_LENGTH, type);
		if (pBmp)
		{
			WriteIntoBmp(pBmp, bitcount);
			free(pBmp);

		}
		free(pYuv);
	}

	return 0;
}
