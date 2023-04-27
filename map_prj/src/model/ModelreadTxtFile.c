/*******************************************************************
********************************************************************
*                                                                  *
*��Ȩ��   �й��������ߵ�����о�����2013��                         *
*                                                                  *
*�������� getfc                                                    *
*������   ��ȡ�ı��ļ��е�����                                     *
*���ߣ�   ��Ǫ��  2013/07/27                                       *
*������ʷ��                                                        *
*         ����     ����         ����˵��                           *
*                                                                  *
*�������� readtxtfile_getFloat                                     *
*������   ���ı��ļ��ж�ȡ������                                   *
*���ߣ�   ��Ǫ��  2013/07/27                                       *
*������ʷ��                                                        *
*         ����     ����         ����˵��                           *
*                                                                  *
*�������� readtxtfile_getInt                                       *
*������   ���ı��ļ��ж�ȡ����                                     *
*���ߣ�   ��Ǫ��  2013/07/27                                       *
*������ʷ��                                                        *
*         ����     ����         ����˵��                           *
*                                                                  *
*�������� readtxtfile_getStr                                       *
*������   ���ı��ļ��ж�ȡ�ַ���                                   *
*���ߣ�   ��Ǫ��  2013/07/27                                       *
*������ʷ��                                                        *
*         ����     ����         ����˵��                           *
*                                                                  *
*�������� readtxtfile_nextline                                     *
*������   ���ı��ļ��ж���һ��                                     *
*���ߣ�   ��Ǫ��  2013/07/27                                       *
*������ʷ��                                                        *
*         ����     ����         ����˵��                           *
*                                                                  *
*�������� resetfc                                                  *
*������   �ļ�λ�ø�λ                                             *
*���ߣ�   ��Ǫ��  2013/07/27                                       *
*������ʷ��                                                        *
*         ����     ����         ����˵��                           *
*                                                                  *
********************************************************************
*******************************************************************/

#include "ModelreadTxtFile.h"
#ifdef WIN32
#include <memory.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"

static char filebuf[4096];
static unsigned int  filebuflen = 0;
static unsigned int  fileindex = 0;

/*
���ܣ��ļ�λ�ø�λ
���룺��
�������
*/
static void resetfc()
{
	filebuflen = 0;
	fileindex = 0;
}

/*
���ܣ���ȡ�ı��ļ��е�����
���룺
    pBinFile �ļ����
�����
    buf ��ȡ���ݵ�ָ��
*/
static int getfc(FILE * pBinFile, char * buf)
{
	if(NULL == pBinFile || NULL == buf)
		return -1;
	if (filebuflen <= 0)
	{
		filebuflen = fread(filebuf, 1, sizeof(filebuf), pBinFile);
		fileindex = 0;
	}
	if (filebuflen<=0)
		return -1;
	filebuflen--;
	*buf = filebuf[fileindex++];
	return 0;
}

/*
���ܣ����ı��ļ��ж�ȡ�ַ���
���룺
    pBinFile �ļ����
    maxlen   ��󳤶�
�����
    buf ��ȡ���ݵ�ָ�� 
*/
int readtxtfile_getStr(FILE * pBinFile, char * buf, int maxlen)
{
	int i;
	int total;
	
	if(NULL == pBinFile || NULL == buf)
		return -1;
	total = 0;
	for (i = 0;i<maxlen-1;)
	{
		int ret;
		ret = getfc(pBinFile, buf);/*fread(buf, 1, 1, pBinFile);*/
		if (ret != 0)
			break;
		if( (*buf == '\t') ||
		    (*buf == ' ') ||
		    (*buf == '\n') ||
		    (*buf == '\r') ||
		    (*buf == ',') ||
		    (*buf == ':') ||
		    // (*buf == '-') ||
		    (*buf == '|') )
		{
			if (total == 0)			
				continue;
			else
				break;
		}else
		{
			buf++;
			total++;
			i++;			
		}
	} 
	*buf = 0;
	return total;
}

/*
���ܣ����ı��ļ��ж���һ��
���룺
    pBinFile �ļ����
�����
    �� 
*/
 int readtxtfile_nextline(FILE * pBinFile)
{
	int ret;
	char buf[1];
	if(NULL == pBinFile)
		return -1;
	while(1)
	{
		ret = getfc(pBinFile, buf);/*ret = fread(buf, 1, 1, pBinFile);*/
		if (ret != 0)
			break;
		if (buf[0] == '\n')
			break;
	}
	return 0;
}

/*
���ܣ����ı��ļ��ж�ȡ����
���룺
    pBinFile �ļ����
�����
    ��ȡ������ 
*/
 int readtxtfile_getInt(FILE *pBinFile)
{
	char buf[60] = {0};
	if(NULL == pBinFile)
		return 0;
	memset(buf, 0, 60);
	if (readtxtfile_getStr(pBinFile, buf, 60) > 0)
		return atoi(buf);
	else
		return 0;
}

/*
���ܣ����ı��ļ��ж�ȡ������
���룺
    pBinFile �ļ����
�����
    ��ȡ������ 
*/
 float readtxtfile_getFloat(FILE *pBinFile)
{
	char buf[60] = {0};
	if(NULL == pBinFile)
		return 0.0f;
	memset(buf, 0, 60);
	if (readtxtfile_getStr(pBinFile, buf, 60) > 0)
	{	
		if((*buf == '-')){
			return ((-1.0)*(float)(atof(buf+1)));
		}else{
			return ((float)(atof(buf)));
		}
	}
	else
		return 0.0f;
}

//��ȡbmp�ļ�
f_uint8_t* ReadBMP(const f_char_t* path, f_uint16_t* bmp_width, f_uint16_t* bmp_height, f_uint32_t *bmp_size, f_uint8_t* bmp_style)
{
	FILE* pFile = NULL;
	f_uint8_t* pImageData = NULL;
	f_uint32_t lineLength = 0;
	f_uint8_t textureColors = 0;			/**< ���ڽ�ͼ����ɫ��BGR�任��RGB */
	f_int32_t i, j;
	f_int16_t width, height;
	width = height = 0;
	pFile = fopen(path, "rb");
	if(!pFile)
	{
		printf("��ȡͼ��ʧ��!!!_%s\n", path);
		return NULL;
	}
	fseek(pFile, 0x0012, SEEK_SET);
#ifdef _LITTLE_ENDIAN_
	fread(bmp_width, sizeof(f_int16_t), 1, pFile);
#else
	fread(&width, sizeof(f_int16_t), 1, pFile);
	*bmp_width = (f_uint16_t)ConvertL2B_short(width);
#endif
	
	fseek(pFile, 0x0016, SEEK_SET);
#ifdef _LITTLE_ENDIAN_
	fread(bmp_height, sizeof(f_int16_t), 1, pFile);
#else
	fread(&height, sizeof(f_int16_t), 1, pFile);
	*bmp_height = (f_uint16_t)ConvertL2B_short(height);
#endif
	//�������Ŀռ�
	lineLength = (*bmp_width) * 3;	//ÿ�����ݳ��� = ͼ���� * ÿ���ص��ֽ���
	while(lineLength % 4 != 0)		//����LienWidthʹ��Ϊ4�ı���
	{
		++ lineLength;
	}
	(*bmp_size) = lineLength * (*bmp_height);
	//�ƶ���ͼ�����ݴ�
	fseek(pFile, 0x0036, SEEK_SET);
	/** �����ڴ� */
	pImageData = (f_uint8_t*)NewAlterableMemory(*bmp_size);
	/** ����ڴ�����Ƿ�ɹ� */
	if(!pImageData)                        /**< �������ڴ�ʧ���򷵻� */
	{
		fclose(pFile);
		return NULL;
	}
	/** ��ȡͼ������ */
	fread(pImageData, 1, (*bmp_size), pFile);
	/** ��ͼ����ɫ���ݸ�ʽ���н���,��BGRת��ΪRGB */
	for(i = 0; i < (*bmp_height); ++i)
	{
		for(j = 0; j < (*bmp_width); ++j)
		{
			textureColors = pImageData[i * lineLength + j * 3];
			pImageData[i * lineLength + j * 3] = pImageData[i * lineLength + j * 3 + 2];
			pImageData[i * lineLength + j * 3 + 2] = textureColors;
		}
	}
	fclose(pFile);
	//���ö�ȡ���ļ�Ϊ24λRGB��ʽ
	*bmp_style = 3;
	return pImageData;
}
