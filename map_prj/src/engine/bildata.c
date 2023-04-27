#ifdef WIN32
#include <string.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include <vxWorks.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../geometry/convertEddian.h"
#include "bildata.h"
#include "memoryPool.h"


/*
功能：初始化高程数据图幅链表和高程数据文件链表
输入：
    pLevel 单层级高程数据管理结构体指针
输出：
	无
返回值：
    0  成功
*/
static int billevelInit(sBILLEVEL * pLevel)
{
	pLevel->bufCount = 0;
	pLevel->bufHeader.pPrev = &pLevel->bufHeader;
	pLevel->bufHeader.pNext = &pLevel->bufHeader;

	pLevel->blockCount = 0;
	pLevel->blockHeader.pPrev = &pLevel->blockHeader;
	pLevel->blockHeader.pNext = &pLevel->blockHeader;
	return 0;
}


/*
功能：清空高程数据图幅链表和高程数据文件链表
输入：
    pLevel 单层级高程数据管理结构体指针
输出：
	无
返回值：
    0  成功
*/
static int billevelDestroy(sBILLEVEL * pLevel)
{
	sBILFILE * pBilFile;
	sBILBlockFILE * pBilBlockFile;
	sBILFILE * pNext;
	sBILBlockFILE * pNextBlockFile;

	/*清空高程数据图幅链表*/
	pBilFile = pLevel->bufHeader.pNext;
	while (pBilFile != &pLevel->bufHeader)
	{
		pNext = pBilFile->pNext;
		free(pBilFile);
		pBilFile = pNext;
	}

	/*清空高程数据文件链表*/
	pBilBlockFile = pLevel->blockHeader.pNext;
	while(pBilBlockFile != &pLevel->blockHeader)
	{
		pNextBlockFile = pBilBlockFile->pNext;
		fclose(pBilBlockFile->pFile);
		free(pBilBlockFile);
		pBilBlockFile = pNextBlockFile;
	}

	return 0;
}

/*
功能：打开原始高程数据文件,从中查询指定经纬度的点所在的图幅,获取该图幅的所有高程数据
输入：
    pLevel    单层级高程数据管理结构体指针
	pPathName 对应高程数据的路径
	h         水平方向,即经度,单位为1/128秒
	v         垂直方向,即纬度,单位为1/128秒
输入输出：
	pBilBuf   图幅高程数据结构体指针
返回值：
    -1        申请高程数据文件操作管理结构体内存失败或文件打开失败
	0         获取成功
*/
static int bilfileLoadData(sBILLEVEL *pLevel, sBILFILE * pBilFile, const char * pPathName, int h, int v)
{
	/* 注意：这里的经纬度单位为度，而不是1/128秒*/
	char blockFilename[256], blockFileFullpath[256];
	FILE *pFile;
	int level = 3;
	int lonNum, latNum;
	long seekNum;
	double vDouble, hDouble;
	sBILBlockFILE * pBilBlockFile;
	
	/* 将待查询的经纬度从以1/128秒为单位的整形转换成以度为单位的浮点型 */
	vDouble = v / (128.0 * 3600.0);
	hDouble = h / (128.0 * 3600.0);
	/* 计算待查询的经纬度所在的原始高程数据块的文件名,每个文件是3°*3°的范围,文件命名是000000.bil,前3个数字是纬度/3,后3个数字是经度/3 */
	sprintf(blockFilename, "%03d%03d.bil", (int)(vDouble/V_SCALE_DEGREE), (int)(hDouble/H_SCALE_DEGREE) );
	/* 获取原始高程数据块的全路径名 */
	sprintf(blockFileFullpath, "%s%s", pPathName, blockFilename);

	/* 管理打开原始高程数据块的文件句柄 */
	pBilBlockFile = pLevel->blockHeader.pNext;
	/* 1.先搜索需要的文件是否已经有保存的文件句柄在文件链表中 */
	while(pBilBlockFile != &pLevel->blockHeader)
	{
		if (strcmp(blockFilename, pBilBlockFile->blockFileName) == 0)
			break;
		pBilBlockFile = pBilBlockFile->pNext;
	}
	/* 2.如果没有现成的文件链表，则需要打开新的文件 */
	if (pBilBlockFile == &pLevel->blockHeader)
	{
		/* 2.1 若文件句柄数量超过最大数值(20)，则重用链表中的文件句柄链表；否则，则新建文件句柄链表元素 */
		if (pLevel->blockCount >= MAXBLOCKBILFILE)
		{
			/* 重用原来在表尾的缓冲区,将其从链表中删除,关闭原来的高程数据块文件,链表中的句柄数量-1 */
			pBilBlockFile = pLevel->blockHeader.pPrev;
			pBilBlockFile->pPrev->pNext = pBilBlockFile->pNext;
			pBilBlockFile->pNext->pPrev = pBilBlockFile->pPrev;
			pBilBlockFile->pPrev = NULL;
			pBilBlockFile->pNext = NULL;
			fclose(pBilBlockFile->pFile);
			pLevel->blockCount --;
		}
		else
		{
			pBilBlockFile = (sBILBlockFILE *)NewAlterableMemory(sizeof(sBILBlockFILE));
		}

		/* 2.2 初始化新的文件链表句柄元素 */
		if(pBilBlockFile == NULL)
		{
			return -1;
		}
		else
		{
			memset(pBilBlockFile, 0, sizeof(sBILBlockFILE));
		}

		/* 2.3 打开文件，并获得文件打开句柄 */
		pFile = fopen(blockFileFullpath, "rb");
		if (pFile != NULL)
		{
			pBilBlockFile->pFile = pFile;
			memcpy(pBilBlockFile->blockFileName, blockFilename, strlen(blockFilename));
			pLevel->blockCount ++;
		}
		else
		{
			/* 若文件打开失败，则生成一段全为异常值(-9999,SRTM-3中也是用该值表示异常值)的数据缓冲区 */
			memset(pBilFile->data, INVALID_DEM_VALUE, BILSIZE2 * sizeof(short));
			/*必须加，否则会内存泄露*/
			DeleteAlterableMemory((void*)pBilBlockFile);
			return -1;
		}
	}

	/* 3.把当前的文件句柄放在表的最前面，以便下次搜索 */
	if (pBilBlockFile != pLevel->blockHeader.pNext)
	{
		/* 3.1 如果节点在原来的表中，先把它解下来 */
		if (pBilBlockFile->pPrev != NULL)
		{
			pBilBlockFile->pNext->pPrev = pBilBlockFile->pPrev;
			pBilBlockFile->pPrev->pNext = pBilBlockFile->pNext;
		}
		/* 3.2 插入到表头 */
		pBilBlockFile->pNext = pLevel->blockHeader.pNext;
		pBilBlockFile->pPrev = &pLevel->blockHeader;
		pLevel->blockHeader.pNext->pPrev = pBilBlockFile;
		pLevel->blockHeader.pNext = pBilBlockFile;
	}

	/* 4.从文件中找到本次需要的数据，并写入数据缓存 */
	/* 第三层级,每个高程数据块文件中包含12*12个高程图幅,图幅之间按左上角开始存,存完一个图幅再存下一个图幅,按行存放,经纬度逐渐增大*/
	/* 图幅内从左下角开始存,按行存储,经度增大,纬度逐渐减小*/
	/* latNum = (当前纬度-左下角纬度)*(纬度方向1°包含高程图幅个数)*/
	latNum = (int)( ( vDouble - ((int)(vDouble/V_SCALE_DEGREE) * V_SCALE_DEGREE) ) * pow(2, level-1) );
	/* lonNum = (当前经度-左下角经度)*(经度方向1°包含高程图幅个数)*/
	lonNum = (int)( ( hDouble - ((int)(hDouble/H_SCALE_DEGREE) * H_SCALE_DEGREE) ) * pow(2, level-1) );
	/* 在高程数据块文件中的偏移seekNum = (lonNum + latNum * 每行存放图幅个数-12个)* 每个图幅高程点个数 * 每个点所占字节数 */
	seekNum = (long)( ( lonNum + latNum * H_SCALE_DEGREE * pow(2, level - 1) ) * BILSIZE2 * sizeof(short) );
	/* 从高程数据块文件中读取图幅高程数据 */
	fseek(pBilBlockFile->pFile, seekNum, SEEK_SET);
	fread(pBilFile->data, 1, BILSIZE2 * sizeof(short), pBilBlockFile->pFile);
	/* 原始高程数据块文件是按大端存储,如果再小端环境下用,需要大小端转换*/
#ifdef _LITTLE_ENDIAN_
//#define SWAPSHORT(w) (((w) >> 8) | ((w) << 8))
	{
		int i;
		for (i = 0;i<BILSIZE2;i++)
		{
			pBilFile->data[i] = ConvertL2B_short(pBilFile->data[i]);
//			pBilBuf->data[i] = SWAPSHORT(pBilBuf->data[i]);
		}
	}
#endif

	return 0;
}

/*
功能：从原始高程数据中查询指定经纬度的点所在的图幅高程数据句柄
输入：
    pLevel 单层级高程数据管理结构体指针
	h      水平方向,即经度,单位为1/128秒
	v      垂直方向,即纬度,单位为1/128秒
输出：
	无
返回值：
    有效指针  图幅高程数据句柄
	NULL      未查询到或内存申请失败
*/
static sBILFILE * billevelFindData(sBILLEVEL *pLevel, int h, int v)
{
	sBILFILE * pBilFile;
	pBilFile = pLevel->bufHeader.pNext;
	/* 1.先按照小块数据的经纬度范围，查找当前的链表小块数据缓存中是否有对应的数据 */
	while (pBilFile != &pLevel->bufHeader)
	{
		if ( (h >= pBilFile->hstart) && 
			 (h < pBilFile->hend) &&
			 (v >= pBilFile->vstart) &&
			 (v < pBilFile->vend) )
			 break;
		pBilFile = pBilFile->pNext;
	}
	/* 2.若未找到，则需要新建一个数据缓存 */
	if (pBilFile == &pLevel->bufHeader)
	{
		/* 2.1 若数据缓存数量(100)已经达到最大，则重用原来在表尾的缓冲区；否则，则新建一个缓冲区 */
		if (pLevel->bufCount >= MAXBILBUF)
		{
			pBilFile = pLevel->bufHeader.pPrev;
			pBilFile->pPrev->pNext = pBilFile->pNext;
			pBilFile->pNext->pPrev = pBilFile->pPrev;
			pBilFile->pPrev = NULL;
			pBilFile->pNext = NULL;
			pLevel->bufCount --;
		}
		else
		{
			/*实际多申请了1个short*/
			pBilFile = (sBILFILE *)NewAlterableMemory(sizeof(sBILFILE) + sizeof(short) * BILSIZE2);
		}
		/* 2.2 为即将要用的数据缓存空间初始化 */
		if (pBilFile == NULL)
		{
			return NULL;
		}
		else
		{
			memset(pBilFile, 0, sizeof(sBILFILE));
		}
		
		/* 2.3 打开文件，将所需范围的数据及相关信息写入缓存空间中 */
		if (bilfileLoadData(pLevel, pBilFile, pLevel->pathname, h, v) != 0)
		{
			DeleteAlterableMemory(pBilFile);
			return NULL;
		}
		/* 2.4 计算左下角和右上角的经度和纬度*/
		pBilFile->hstart = (h / pLevel->hscale) * pLevel->hscale;
		pBilFile->hend = pBilFile->hstart + pLevel->hscale;
		pBilFile->vstart = (v / pLevel->vscale) * pLevel->vscale;
		pBilFile->vend = pBilFile->vstart + pLevel->vscale;
		/* 2.5 已缓存的高程图幅自增1*/
		pLevel->bufCount ++;
	}
	/* 3.把当前缓存数据放在表的最前面， 方便下次搜索 */
	if (pBilFile != pLevel->bufHeader.pNext)
	{
		/* 3.1 如果结点在原来的表中，先把它解下来 */
		if (pBilFile->pPrev != NULL)
		{
			pBilFile->pNext->pPrev = pBilFile->pPrev;
			pBilFile->pPrev->pNext = pBilFile->pNext;
		}
		/* 3.2 插入到表头 */
		pBilFile->pNext = pLevel->bufHeader.pNext;
		pBilFile->pPrev = &pLevel->bufHeader;
		pLevel->bufHeader.pNext->pPrev = pBilFile;
		pLevel->bufHeader.pNext = pBilFile;
	}

	return pBilFile;
}

/*
	生成一个所有层级高程数据管理总结构体的句柄，
	参数：pBILFile -- 如果成功则返回文件句柄供后续调用使用
	返回值：
	     0 -- 成功，文件句柄在pBILFile中
		-1 -- 内存分配失败
*/
int bildataCreate(BILDATA * pBILFile)
{
	sBILDATA *pData;
	int i;

	pData = (sBILDATA *)NewFixedMemory(sizeof(sBILDATA));
	if (pData == NULL)
		return -1;

	pData->key = BILDATAKEY;
	/*完成所有层级高程数据管理总结构体中各个单层级高程数据管理结构体的初始化,目前支持4个层级*/
	memset(pData->levels, 0, sizeof(sBILLEVEL) * BILLEVELS);
	for (i = 0;i<BILLEVELS;i++)
	{
		billevelInit(&pData->levels[i]);
	}

	*pBILFile = (BILDATA)pData;

	return 0;
}

#define CHECKBIL(bd, retv) if (((sBILDATA *)bd)->key != BILDATAKEY) return retv;

/*
	删除BILDATA句柄，释放所占用的资源
	参数：bildata -- 用bildataCreate返回的BILDATA句柄
	返回值：
	     0 -- 成功
		-1 -- 句柄无效
*/
int bildataDestroy(BILDATA bildata)
{
	int i;
	sBILDATA * pBilData;
	CHECKBIL(bildata, -1);
	pBilData = (sBILDATA *)bildata;
	for (i = 0;i<BILLEVELS;i++)
		billevelDestroy(&pBilData->levels[i]);
	pBilData->key = 0;
	DeleteAlterableMemory(pBilData);
	return 0;
}

/*
	更新指定层级的高程数据目录、高程图幅经度跨度、高程图幅纬度跨度到所有层级高程数据管理总结构体中
	参数：bildata -- 用bildataCreate返回的BILDATA句柄
		  level -- 层号 0 ~ 3
	      cstrPathName -- 以/结尾的目录名，表示该层数据所在目录
		  hscale -- 一个图幅的经度跨度, 1/128秒单位
		  vscale -- 一个图幅的纬度跨度，1/128秒单位
    返回值：
	      0 -- 成功
		 -1 -- 句柄无效
		 -2 -- 层级参数无效
*/
int bildataSetDataPath(BILDATA bildata, int level, const char * cstrPathName, int hscale, int vscale)
{
	sBILDATA * pBilData;
	sBILLEVEL * pLevel;

	/*判断句柄key值是否正确*/
	CHECKBIL(bildata, -1);
	/*判断层级是否在有效范围*/
	if ( (level < 0) || (level >= BILLEVELS) )
		return -2;

	/*获取所有层级高程数据管理总结构体指针*/
	pBilData = (sBILDATA *)bildata;
	/*获取指定层级的高程数据管理结构体指针*/
	pLevel = &pBilData->levels[level];
	/*清空高程数据图幅链表和高程数据文件链表*/
	billevelDestroy(pLevel);
	/*初始化指定层级的高程数据管理结构体*/
	billevelInit(pLevel);
	/*将高程数据文件目录、高程图幅经度跨度、高程图幅纬度跨度,更新到指定层级的高程数据管理结构体中*/
	strcpy(pLevel->pathname, cstrPathName);
	pLevel->hscale = hscale;
	pLevel->vscale = vscale;

	return 0;
}

/*
	功能：从指定层级的原始高程数据中查询指定经纬度的高程
	参数：bildata -- 用bildataCreate返回的BILDATA句柄
		  level -- 层号 0-3
		  h -- 经度, 1/128秒为单位
		  v -- 纬度, 1/128秒为单位
	返回值：
		  >-9999: 正常高程
		   -9999: 原始高程数据不存在
		  -10001: 原始高程数据超出范围
		  -10002: 数据句柄无效
		  -10003: 参数无效
		  -10004: 缓存空间出错
*/
short bildataGetHeight(BILDATA bildata, int level, int h, int v)
{
	sBILDATA * pBilData;
	sBILLEVEL * pLevel;
	sBILFILE * pData;
	short data;

	/*判断句柄key值是否正确*/
	CHECKBIL(bildata, -10002);
	/*判断层级是否在有效范围*/
	if ( (level < 0) || (level >= BILLEVELS) )
		return -10003;
	/* 为了方便后续的计算及数据文件的命名，将带有负值的经纬度都换成正的周期值 */
	if (h < 0)
		h += 180 * 3600 * 128;
	if (v < 0)
		v += 90 * 3600 * 128;

	/*获取所有层级高程数据管理总结构体指针*/
	pBilData = (sBILDATA *)bildata;
	/*获取指定层级的高程数据管理结构体指针*/
	pLevel = &pBilData->levels[level];
	/*在原始高程数据中找到该经纬度所在的图幅高程数据存储管理结构体句柄*/
	pData = billevelFindData(pLevel, h, v);
	/*图幅高程句柄为空,说明没查询到高程值,直接返回*/
	if (pData == NULL)
		return -10004;
	/*每个图幅包含256*256个高程点,查询指定经纬度在缓冲中的位置*/
	/*图幅中的高程数据从图幅左上角开始存放,按行存储*/
	/*hstart vstart分别代表图幅左下角的经度和纬度,hscale vscale分别代表图幅的经度跨度和纬度跨度,单位是1/128秒*/
	h -= pData->hstart;
	v -= pData->vstart;
	h = h * BILSIZE / pLevel->hscale;
	v = v * BILSIZE / pLevel->vscale;
	v = BILSIZE - 1 - v;
	/*直接索引得到高程值*/
	data = pData->data[v * BILSIZE + h];

	/*判断高程数据是否超出范围*/
	if ((data > 10000) || (data < INVALID_DEM_VALUE))
		return -10001;

	return data;
}

/*
 * 显示当前内存中已载入的图幅高程数量及已打开的图幅数据文件数量
 * */
void bildataPrintCacheCount(BILDATA bildata, int level)
{
	sBILDATA * pBilBlockData;
	sBILLEVEL * pLevel;

	pBilBlockData = (sBILDATA *)bildata;
	pLevel = &pBilBlockData->levels[level];
	
	printf("BilCount=%d,FileCount=%d\n", pLevel->bufCount, pLevel->blockCount);
}
