// video_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#define YUV_PATH     "d:\\tmp\\yuv_image\\x.yuv"
#define YUV_HEIGHT   640
#define YUV_WIDTH    480
#define BMP_DATA_SIZE     (YUV_HEIGHT*YUV_WIDTH*3)

#define BMP_PATH     "d:\\tmp\\yuv_image\\x.bmp"

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
#define CAL_YUV_R(y, cb, cr) (unsigned char)(1.1665*(y-16.5) - 0.0483*(cb - 128.5) + 1.6455*(cr - 128.5))%255
#define CAL_YUV_G(y, cb, cr) (unsigned char)(1.1665*(y-16.5) - 0.3922*(cb - 128.5) + 0.8151*(cr - 128.5))%255
#define CAL_YUV_B(y, cb, cr) (unsigned char)(1.1665*(y-16.5) - 2.0218*(cb - 128.5) - 0.0013*(cr - 128.5))%255
char* yuv2rgb888(const char *pYuv, int length)
{
	char *pBuf, *ptr;
	unsigned char r,g,b;
	float cb0, y0, cr0, y1;

	pBuf = (char *)malloc(BMP_DATA_SIZE);
	memset(pBuf, 0x0, BMP_DATA_SIZE);
	ptr = pBuf;
	for (int i=0; i<length;)
	{
		cb0 = pYuv[i++];
		y0  = pYuv[i++];
		cr0 = pYuv[i++];
		y1  = pYuv[i++];
		
		*ptr++ = CAL_YUV_R(y0, cb0, cr0);
		*ptr++ = CAL_YUV_B(y0, cb0, cr0);
		*ptr++ = CAL_YUV_G(y0, cb0, cr0);
		
		*ptr++ = CAL_YUV_R(y1, cb0, cr0);
		*ptr++ = CAL_YUV_B(y1, cb0, cr0);
		*ptr++ = CAL_YUV_G(y1, cb0, cr0);
	}


	return pBuf;
}

int WriteIntoBmp(char *ptr)
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
	bmp_head.bfSize = sizeof(bmp_info) + sizeof(bmp_head) + BMP_DATA_SIZE;
	bmp_head.bfOffBits = sizeof(bmp_info) + sizeof(bmp_head);

	bmp_info.biSize  = sizeof(bmp_info);
	bmp_info.biWidth = YUV_WIDTH;
	bmp_info.biHeight = YUV_HEIGHT;
	bmp_info.biBitCount = 24;
	bmp_info.biSizeImage = BMP_DATA_SIZE;
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
	fwrite(ptr, BMP_DATA_SIZE, 1, fp);
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
		pBmp = yuv2rgb888(pYuv, length);
		if (pBmp)
		{
			WriteIntoBmp(pBmp);
			free(pBmp);

		}
		free(pYuv);
	}

	return 0;
}
