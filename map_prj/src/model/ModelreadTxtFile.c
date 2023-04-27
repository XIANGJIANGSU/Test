/*******************************************************************
********************************************************************
*                                                                  *
*版权：   中国航空无线电电子研究所，2013年                         *
*                                                                  *
*函数名： getfc                                                    *
*描述：   读取文本文件中的内容                                     *
*作者：   张仟新  2013/07/27                                       *
*更改历史：                                                        *
*         姓名     日期         更改说明                           *
*                                                                  *
*函数名： readtxtfile_getFloat                                     *
*描述：   从文本文件中读取浮点数                                   *
*作者：   张仟新  2013/07/27                                       *
*更改历史：                                                        *
*         姓名     日期         更改说明                           *
*                                                                  *
*函数名： readtxtfile_getInt                                       *
*描述：   从文本文件中读取整数                                     *
*作者：   张仟新  2013/07/27                                       *
*更改历史：                                                        *
*         姓名     日期         更改说明                           *
*                                                                  *
*函数名： readtxtfile_getStr                                       *
*描述：   从文本文件中读取字符串                                   *
*作者：   张仟新  2013/07/27                                       *
*更改历史：                                                        *
*         姓名     日期         更改说明                           *
*                                                                  *
*函数名： readtxtfile_nextline                                     *
*描述：   从文本文件中读掉一行                                     *
*作者：   张仟新  2013/07/27                                       *
*更改历史：                                                        *
*         姓名     日期         更改说明                           *
*                                                                  *
*函数名： resetfc                                                  *
*描述：   文件位置复位                                             *
*作者：   张仟新  2013/07/27                                       *
*更改历史：                                                        *
*         姓名     日期         更改说明                           *
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
功能：文件位置复位
输入：无
输出：无
*/
static void resetfc()
{
	filebuflen = 0;
	fileindex = 0;
}

/*
功能：读取文本文件中的内容
输入：
    pBinFile 文件句柄
输出：
    buf 读取内容的指针
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
功能：从文本文件中读取字符串
输入：
    pBinFile 文件句柄
    maxlen   最大长度
输出：
    buf 读取内容的指针 
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
功能：从文本文件中读掉一行
输入：
    pBinFile 文件句柄
输出：
    无 
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
功能：从文本文件中读取整数
输入：
    pBinFile 文件句柄
输出：
    读取的内容 
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
功能：从文本文件中读取浮点数
输入：
    pBinFile 文件句柄
输出：
    读取的内容 
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

//读取bmp文件
f_uint8_t* ReadBMP(const f_char_t* path, f_uint16_t* bmp_width, f_uint16_t* bmp_height, f_uint32_t *bmp_size, f_uint8_t* bmp_style)
{
	FILE* pFile = NULL;
	f_uint8_t* pImageData = NULL;
	f_uint32_t lineLength = 0;
	f_uint8_t textureColors = 0;			/**< 用于将图像颜色从BGR变换到RGB */
	f_int32_t i, j;
	f_int16_t width, height;
	width = height = 0;
	pFile = fopen(path, "rb");
	if(!pFile)
	{
		printf("读取图像失败!!!_%s\n", path);
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
	//计算分配的空间
	lineLength = (*bmp_width) * 3;	//每行数据长度 = 图像宽度 * 每像素的字节数
	while(lineLength % 4 != 0)		//修正LienWidth使其为4的倍数
	{
		++ lineLength;
	}
	(*bmp_size) = lineLength * (*bmp_height);
	//移动到图像数据处
	fseek(pFile, 0x0036, SEEK_SET);
	/** 分配内存 */
	pImageData = (f_uint8_t*)NewAlterableMemory(*bmp_size);
	/** 检查内存分配是否成功 */
	if(!pImageData)                        /**< 若分配内存失败则返回 */
	{
		fclose(pFile);
		return NULL;
	}
	/** 读取图像数据 */
	fread(pImageData, 1, (*bmp_size), pFile);
	/** 将图像颜色数据格式进行交换,由BGR转换为RGB */
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
	//设置读取的文件为24位RGB格式
	*bmp_style = 3;
	return pImageData;
}
