/**
 * @file main.c
 * @author dma
 * @brief BMPͼƬ����ת����ͼ
 * @note
 * TODO
 * ֧���������롢����ļ�·��
 * ֧��C�����������
 * 
 * @version 0.1
 * @date 2020-12-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

#include "argparse.h"

//λͼ�ļ�ͷ
typedef struct __attribute__((packed)) BMPFILEHEAD
{
	unsigned short bfType;//�ļ����ͣ�������0x424D��Ҳ�����ַ�BM
	unsigned int bfSize;//�ļ���С������ͷ
	unsigned short bfReserved1;//������
	unsigned short bfReserved2;//������
	unsigned int bfOffBits;//�ļ�ͷ��ʵ�ʵ�ͼ�����ݵ�ƫ���ֽ���
}BMPFILEHEAD;

//λͼ��Ϣͷ
typedef struct __attribute__((packed)) BMPINFOHEAD
{
	unsigned int biSize;//����ṹ��ĳ��ȣ�Ϊ40�ֽ�
	int biWidth;//ͼ��Ŀ��
	int biHeight;//ͼ��ĳ���
	unsigned short biPlanes;//������1
	unsigned short biBitCount;//��ʾ��ɫʱҪ�õ���λ�������õ�ֵΪ 1���ڰ׶�ɫͼ��,4��16 ɫͼ��,8��256 ɫ��,24�����ɫͼ�����µ�.bmp ��ʽ֧�� 32 λɫ�����ﲻ�����ۣ�
	unsigned int biCompression;//ָ��λͼ�Ƿ�ѹ������Ч��ֵΪ BI_RGB��BI_RLE8��BI_RLE4��BI_BITFIELDS������һЩWindows����õĳ�������ʱֻ����BI_RGB��ѹ���������
	unsigned int biSizeImage;//ָ��ʵ�ʵ�λͼ����ռ�õ��ֽ���
	int biXPelsPerMeter;//ָ��Ŀ���豸��ˮƽ�ֱ���
	int biYPelsPerMeter;//ָ��Ŀ���豸�Ĵ�ֱ�ֱ���
	unsigned int biClrUsed;//ָ����ͼ��ʵ���õ�����ɫ���������ֵΪ�㣬���õ�����ɫ��Ϊ 2 �� biBitCount �η�
	unsigned int biClrImportant;//ָ����ͼ������Ҫ����ɫ���������ֵΪ�㣬����Ϊ���е���ɫ������Ҫ��
}BMPINFOHEAD;

//��ɫ��
//typedef struct RGBQUAD
//{
//	unsigned char rgbBlue;//����ɫ����ɫ����
//	unsigned char rgbGreen;//����ɫ����ɫ����
//	unsigned char rgbRed;//����ɫ�ĺ�ɫ����
//	unsigned char rgbReserved;//����ֵ
//};


//ȫ�ֱ���
FILE *rfp;
FILE *wfp;
BMPFILEHEAD BFH;
BMPINFOHEAD BIH;

unsigned char *bmp_data = NULL;//��ɫBMP�ļ�����ָ��
unsigned char *bit_data = NULL;//�ڰ�ͼ���ļ�����ָ��
uint32_t bit_data_size = 0;


//����
int32_t invert_color = 0; // ��ɫ
int32_t luminance = 128; // ����


//argparse
struct argparse argparse;

static const char *const usages[] = {
	"main.exe [options]",
	NULL,
};

struct argparse_option options[] = {
	OPT_HELP(),
	OPT_GROUP("Basic options"),
	OPT_BOOLEAN('i', "invert", &invert_color, "invert color,  default FALSE", NULL, 0, 0),
	OPT_INTEGER('l', "luminance", &luminance, "set luminance, default 128", NULL, 0, 0),
	OPT_END(),
};


//����
int ReadBMPHead(FILE *fp);
unsigned int ReadPoint(int x, int y);
void DrawBITPoint(unsigned int x, unsigned int y);
void CleanBITPoint(unsigned int x, unsigned int y);
unsigned char ReadBITPoint(unsigned int x, unsigned int y);
void BMP2BIT();//24ɫλͼת�ڰ�ͼ


int main(int argc, const char **argv)
{
	char filename[512];
	int i;
	int ret = 0;

	argparse_init(&argparse, options, usages, 0);
	argparse_parse(&argparse, argc, argv);

	if(luminance < 0 || luminance > 255)
	{
		printf("luminance is out of range! \n");
		return 0;
	}
	luminance *= 3; // �������

	rfp = fopen(".\\img\\0000.bmp", "rb");
	if(rfp == NULL)
	{
		printf("open file \".\\img\\0000.bmp\" error \n");
		system("pause");
		return 0;
	}
	ret = ReadBMPHead(rfp);
	if(ret)
	{
		return 0;
	}
	fclose(rfp);

	bmp_data = (unsigned char *)malloc(BIH.biSizeImage);
	
	// ��ͼ��߶�Ϊ׼������15*8���ص�ͼƬ����Ҫ15 * (8 / 8)�ֽ��ڴ棬���ص�ͼƬ��Ҫ15 * (9 / 8)����ȡ�� = 15 * 2�ֽ��ڴ�
	bit_data_size = BIH.biWidth * ((BIH.biHeight - 1) / 8 + 1);
	bit_data = (unsigned char *)malloc(bit_data_size);

	if((wfp = fopen(".\\img.bin", "wb")) == NULL)
	{
		printf("�����ļ�����\n");
		system("pause");
		return 0;
	}

	printf("\n\n");
	i = 0;
	while (1)
	{
		memset(filename, 0, 512);
		sprintf(filename, ".\\img\\%04d.bmp", i);

		if((rfp = fopen(filename, "rb")) == NULL)
		{
			printf("\n���ļ� %s �����ȫ���ļ��Ѵ������\n", filename);
			system("pause");
			break;
			//continue;
		}
		printf("\r���ڴ����ļ��� %s ", filename);
		fflush(stdout);

		fseek(rfp, BFH.bfOffBits, SEEK_SET);
		fread(bmp_data, BIH.biSizeImage, 1, rfp);
		fclose(rfp);

		BMP2BIT();
		fwrite(bit_data, bit_data_size, 1, wfp);

		i++;
	}

	fclose(wfp);

	free(bmp_data);
	bmp_data = NULL;
	free(bit_data);
	bit_data = NULL;

	return 0;
}

int ReadBMPHead(FILE *fp)
{
	fseek(fp, 0, SEEK_SET);

	fread(&BFH, sizeof(BFH), 1, fp); //��ȡBMP�ļ�ͷ
	fread(&BIH, sizeof(BIH), 1, fp);//��ȡBMP��Ϣͷ��40�ֽڣ�ֱ���ýṹ���

	printf("\nBMP�ļ�ͷ\n");
	printf("bfType = %x\n", BFH.bfType);
	printf("bfSize = %d\n", BFH.bfSize);
	printf("bfReserved1 = %d\n", BFH.bfReserved1);
	printf("bfReserved2 = %d\n", BFH.bfReserved2);
	printf("bfOffBits = %d\n", BFH.bfOffBits);

	printf("\nBMP��Ϣͷ\n");
	printf("biSize = %d\n", BIH.biSize);
	printf("biWidth = %d\n", BIH.biWidth);
	printf("biHeight = %d\n", BIH.biHeight);
	printf("biPlanes = %d\n", BIH.biPlanes);
	printf("biBitCount = %d\n", BIH.biBitCount);
	printf("biCompression = %d\n", BIH.biCompression);
	printf("biSizeImage = %d\n", BIH.biSizeImage);
	printf("biXPelsPerMeter = %d\n", BIH.biXPelsPerMeter);
	printf("biYPelsPerMeter = %d\n", BIH.biYPelsPerMeter);
	printf("biClrUsed = %d\n", BIH.biClrUsed);
	printf("biClrImportant = %d\n", BIH.biClrImportant);
	
	
//	if((BFH.bfType != 0x424D) || (BIH.biClrImportant != 0))
	if((BFH.bfType != 0x4D42))
	{
		printf("\n����BMP�ļ���\n");
		return 1;
	}

	if (BIH.biBitCount != 24 || ((BIH.biClrImportant != 0) && (BIH.biClrImportant != 16777216)))
	{
		printf("\n����24λBMP�ļ���\n");
		return 2;
	}

	return 0;
}

//�����0��ʼ����
unsigned int ReadPoint(int x, int y)
{
	unsigned int pos = 0;
	unsigned int line_size = 0;
	unsigned int RGB = 0;
	//unsigned int BGR = 0;

	if (bmp_data == NULL)
	{
		// printf("��ͼ������\n");
		return 1;
	}
	
	if (x >= BIH.biWidth || y >= BIH.biHeight)
	{
		//printf("��ȡ�����ص㳬��ͼ��Χ\n");
		return 1;
	}

	//a=14,b=5
	//��ͨȡ��a/b=2
	//��һȡ��(a+b-1)/b=3

	// (((width*biBitCount+7)/8+3)/4*4
	// (((width*biBitCount)+31)/32)*4
	// (((width*biBitCount)+31)>>5)<<2
	line_size = (((BIH.biWidth * BIH.biBitCount)+31)>>5)<<2;
	pos = (BIH.biHeight - y - 1) * line_size + x * 3;

	//BGR |= bmp_data[pos] << 16;
	//BGR |= bmp_data[pos + 1] << 8;
	//BGR |= bmp_data[pos + 2];

	RGB = *(unsigned int *)(&bmp_data[pos]) & 0x00FFFFFF;

	return RGB;
}

inline void DrawBITPoint(unsigned int x, unsigned int y)
{
	//���ⳬ����Ļ��Χ
	//if(x < BIH.biWidth && y < BIH.biHeight)
	{
		//bit_data[x + BIH.biWidth * (y >> 3)] = bit_data[x + BIH.biWidth * (y >> 3)] | (0x80 >> (y & 0x07));//MSB
		bit_data[x + BIH.biWidth * (y >> 3)] = bit_data[x + BIH.biWidth * (y >> 3)] | (0x01 << (y & 0x07));//LSB
	}
}

inline void CleanBITPoint(unsigned int x, unsigned int y)
{
	//���ⳬ����Ļ��Χ
	//if(x < BIH.biWidth && y < BIH.biHeight)
	{
		//bit_data[x + BIH.biWidth * (y >> 3)] = bit_data[x + BIH.biWidth * (y >> 3)] & (~(0x80 >> (y & 0x07)));//MSB
		bit_data[x + BIH.biWidth * (y >> 3)] = bit_data[x + BIH.biWidth * (y >> 3)] & (~(0x01 << (y & 0x07)));//LSB
	}
}

inline unsigned char ReadBITPoint(unsigned int x, unsigned int y)
{
	//���ⳬ����Ļ��Χ
	//if(x < BIH.biWidth && y < BIH.biHeight)
	{
		//��ȡ����
		//if(bit_data[x + BIH.biWidth * (y >> 3)] & (0x80 >> (y & 0x07)))//MSB
		if(bit_data[x + BIH.biWidth * (y >> 3)] & (0x01 << (y & 0x07)))//LSB
		{
			return 1;
		}
	}
	return 0;
}

//��ͼת�ڰ�
void BMP2BIT()
{
	int x, y;
	uint32_t RGB;
	
	memset(bit_data, 0, bit_data_size);
	for (y=0; y<BIH.biHeight; y++)
	{
		for (x=0; x<BIH.biWidth; x++)
		{
			RGB = ReadPoint(x,y);
			if (((RGB >> 16) & 0x0000FF) + ((RGB >> 8) & 0x0000FF) + (RGB & 0x0000FF) < luminance)
			{
				invert_color ? CleanBITPoint(x, y) : DrawBITPoint(x, y);
			}
			else
			{
				invert_color ? DrawBITPoint(x, y) : CleanBITPoint(x, y);
			}
		}
	}
}
