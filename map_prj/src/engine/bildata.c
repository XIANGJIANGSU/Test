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
���ܣ���ʼ���߳�����ͼ������͸߳������ļ�����
���룺
    pLevel ���㼶�߳����ݹ���ṹ��ָ��
�����
	��
����ֵ��
    0  �ɹ�
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
���ܣ���ո߳�����ͼ������͸߳������ļ�����
���룺
    pLevel ���㼶�߳����ݹ���ṹ��ָ��
�����
	��
����ֵ��
    0  �ɹ�
*/
static int billevelDestroy(sBILLEVEL * pLevel)
{
	sBILFILE * pBilFile;
	sBILBlockFILE * pBilBlockFile;
	sBILFILE * pNext;
	sBILBlockFILE * pNextBlockFile;

	/*��ո߳�����ͼ������*/
	pBilFile = pLevel->bufHeader.pNext;
	while (pBilFile != &pLevel->bufHeader)
	{
		pNext = pBilFile->pNext;
		free(pBilFile);
		pBilFile = pNext;
	}

	/*��ո߳������ļ�����*/
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
���ܣ���ԭʼ�߳������ļ�,���в�ѯָ����γ�ȵĵ����ڵ�ͼ��,��ȡ��ͼ�������и߳�����
���룺
    pLevel    ���㼶�߳����ݹ���ṹ��ָ��
	pPathName ��Ӧ�߳����ݵ�·��
	h         ˮƽ����,������,��λΪ1/128��
	v         ��ֱ����,��γ��,��λΪ1/128��
���������
	pBilBuf   ͼ���߳����ݽṹ��ָ��
����ֵ��
    -1        ����߳������ļ���������ṹ���ڴ�ʧ�ܻ��ļ���ʧ��
	0         ��ȡ�ɹ�
*/
static int bilfileLoadData(sBILLEVEL *pLevel, sBILFILE * pBilFile, const char * pPathName, int h, int v)
{
	/* ע�⣺����ľ�γ�ȵ�λΪ�ȣ�������1/128��*/
	char blockFilename[256], blockFileFullpath[256];
	FILE *pFile;
	int level = 3;
	int lonNum, latNum;
	long seekNum;
	double vDouble, hDouble;
	sBILBlockFILE * pBilBlockFile;
	
	/* ������ѯ�ľ�γ�ȴ���1/128��Ϊ��λ������ת�����Զ�Ϊ��λ�ĸ����� */
	vDouble = v / (128.0 * 3600.0);
	hDouble = h / (128.0 * 3600.0);
	/* �������ѯ�ľ�γ�����ڵ�ԭʼ�߳����ݿ���ļ���,ÿ���ļ���3��*3��ķ�Χ,�ļ�������000000.bil,ǰ3��������γ��/3,��3�������Ǿ���/3 */
	sprintf(blockFilename, "%03d%03d.bil", (int)(vDouble/V_SCALE_DEGREE), (int)(hDouble/H_SCALE_DEGREE) );
	/* ��ȡԭʼ�߳����ݿ��ȫ·���� */
	sprintf(blockFileFullpath, "%s%s", pPathName, blockFilename);

	/* �����ԭʼ�߳����ݿ���ļ���� */
	pBilBlockFile = pLevel->blockHeader.pNext;
	/* 1.��������Ҫ���ļ��Ƿ��Ѿ��б�����ļ�������ļ������� */
	while(pBilBlockFile != &pLevel->blockHeader)
	{
		if (strcmp(blockFilename, pBilBlockFile->blockFileName) == 0)
			break;
		pBilBlockFile = pBilBlockFile->pNext;
	}
	/* 2.���û���ֳɵ��ļ���������Ҫ���µ��ļ� */
	if (pBilBlockFile == &pLevel->blockHeader)
	{
		/* 2.1 ���ļ�����������������ֵ(20)�������������е��ļ���������������½��ļ��������Ԫ�� */
		if (pLevel->blockCount >= MAXBLOCKBILFILE)
		{
			/* ����ԭ���ڱ�β�Ļ�����,�����������ɾ��,�ر�ԭ���ĸ߳����ݿ��ļ�,�����еľ������-1 */
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

		/* 2.2 ��ʼ���µ��ļ�������Ԫ�� */
		if(pBilBlockFile == NULL)
		{
			return -1;
		}
		else
		{
			memset(pBilBlockFile, 0, sizeof(sBILBlockFILE));
		}

		/* 2.3 ���ļ���������ļ��򿪾�� */
		pFile = fopen(blockFileFullpath, "rb");
		if (pFile != NULL)
		{
			pBilBlockFile->pFile = pFile;
			memcpy(pBilBlockFile->blockFileName, blockFilename, strlen(blockFilename));
			pLevel->blockCount ++;
		}
		else
		{
			/* ���ļ���ʧ�ܣ�������һ��ȫΪ�쳣ֵ(-9999,SRTM-3��Ҳ���ø�ֵ��ʾ�쳣ֵ)�����ݻ����� */
			memset(pBilFile->data, INVALID_DEM_VALUE, BILSIZE2 * sizeof(short));
			/*����ӣ�������ڴ�й¶*/
			DeleteAlterableMemory((void*)pBilBlockFile);
			return -1;
		}
	}

	/* 3.�ѵ�ǰ���ļ�������ڱ����ǰ�棬�Ա��´����� */
	if (pBilBlockFile != pLevel->blockHeader.pNext)
	{
		/* 3.1 ����ڵ���ԭ���ı��У��Ȱ��������� */
		if (pBilBlockFile->pPrev != NULL)
		{
			pBilBlockFile->pNext->pPrev = pBilBlockFile->pPrev;
			pBilBlockFile->pPrev->pNext = pBilBlockFile->pNext;
		}
		/* 3.2 ���뵽��ͷ */
		pBilBlockFile->pNext = pLevel->blockHeader.pNext;
		pBilBlockFile->pPrev = &pLevel->blockHeader;
		pLevel->blockHeader.pNext->pPrev = pBilBlockFile;
		pLevel->blockHeader.pNext = pBilBlockFile;
	}

	/* 4.���ļ����ҵ�������Ҫ�����ݣ���д�����ݻ��� */
	/* �����㼶,ÿ���߳����ݿ��ļ��а���12*12���߳�ͼ��,ͼ��֮�䰴���Ͻǿ�ʼ��,����һ��ͼ���ٴ���һ��ͼ��,���д��,��γ��������*/
	/* ͼ���ڴ����½ǿ�ʼ��,���д洢,��������,γ���𽥼�С*/
	/* latNum = (��ǰγ��-���½�γ��)*(γ�ȷ���1������߳�ͼ������)*/
	latNum = (int)( ( vDouble - ((int)(vDouble/V_SCALE_DEGREE) * V_SCALE_DEGREE) ) * pow(2, level-1) );
	/* lonNum = (��ǰ����-���½Ǿ���)*(���ȷ���1������߳�ͼ������)*/
	lonNum = (int)( ( hDouble - ((int)(hDouble/H_SCALE_DEGREE) * H_SCALE_DEGREE) ) * pow(2, level-1) );
	/* �ڸ߳����ݿ��ļ��е�ƫ��seekNum = (lonNum + latNum * ÿ�д��ͼ������-12��)* ÿ��ͼ���̵߳���� * ÿ������ռ�ֽ��� */
	seekNum = (long)( ( lonNum + latNum * H_SCALE_DEGREE * pow(2, level - 1) ) * BILSIZE2 * sizeof(short) );
	/* �Ӹ߳����ݿ��ļ��ж�ȡͼ���߳����� */
	fseek(pBilBlockFile->pFile, seekNum, SEEK_SET);
	fread(pBilFile->data, 1, BILSIZE2 * sizeof(short), pBilBlockFile->pFile);
	/* ԭʼ�߳����ݿ��ļ��ǰ���˴洢,�����С�˻�������,��Ҫ��С��ת��*/
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
���ܣ���ԭʼ�߳������в�ѯָ����γ�ȵĵ����ڵ�ͼ���߳����ݾ��
���룺
    pLevel ���㼶�߳����ݹ���ṹ��ָ��
	h      ˮƽ����,������,��λΪ1/128��
	v      ��ֱ����,��γ��,��λΪ1/128��
�����
	��
����ֵ��
    ��Чָ��  ͼ���߳����ݾ��
	NULL      δ��ѯ�����ڴ�����ʧ��
*/
static sBILFILE * billevelFindData(sBILLEVEL *pLevel, int h, int v)
{
	sBILFILE * pBilFile;
	pBilFile = pLevel->bufHeader.pNext;
	/* 1.�Ȱ���С�����ݵľ�γ�ȷ�Χ�����ҵ�ǰ������С�����ݻ������Ƿ��ж�Ӧ������ */
	while (pBilFile != &pLevel->bufHeader)
	{
		if ( (h >= pBilFile->hstart) && 
			 (h < pBilFile->hend) &&
			 (v >= pBilFile->vstart) &&
			 (v < pBilFile->vend) )
			 break;
		pBilFile = pBilFile->pNext;
	}
	/* 2.��δ�ҵ�������Ҫ�½�һ�����ݻ��� */
	if (pBilFile == &pLevel->bufHeader)
	{
		/* 2.1 �����ݻ�������(100)�Ѿ��ﵽ���������ԭ���ڱ�β�Ļ��������������½�һ�������� */
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
			/*ʵ�ʶ�������1��short*/
			pBilFile = (sBILFILE *)NewAlterableMemory(sizeof(sBILFILE) + sizeof(short) * BILSIZE2);
		}
		/* 2.2 Ϊ����Ҫ�õ����ݻ���ռ��ʼ�� */
		if (pBilFile == NULL)
		{
			return NULL;
		}
		else
		{
			memset(pBilFile, 0, sizeof(sBILFILE));
		}
		
		/* 2.3 ���ļ��������跶Χ�����ݼ������Ϣд�뻺��ռ��� */
		if (bilfileLoadData(pLevel, pBilFile, pLevel->pathname, h, v) != 0)
		{
			DeleteAlterableMemory(pBilFile);
			return NULL;
		}
		/* 2.4 �������½Ǻ����Ͻǵľ��Ⱥ�γ��*/
		pBilFile->hstart = (h / pLevel->hscale) * pLevel->hscale;
		pBilFile->hend = pBilFile->hstart + pLevel->hscale;
		pBilFile->vstart = (v / pLevel->vscale) * pLevel->vscale;
		pBilFile->vend = pBilFile->vstart + pLevel->vscale;
		/* 2.5 �ѻ���ĸ߳�ͼ������1*/
		pLevel->bufCount ++;
	}
	/* 3.�ѵ�ǰ�������ݷ��ڱ����ǰ�棬 �����´����� */
	if (pBilFile != pLevel->bufHeader.pNext)
	{
		/* 3.1 ��������ԭ���ı��У��Ȱ��������� */
		if (pBilFile->pPrev != NULL)
		{
			pBilFile->pNext->pPrev = pBilFile->pPrev;
			pBilFile->pPrev->pNext = pBilFile->pNext;
		}
		/* 3.2 ���뵽��ͷ */
		pBilFile->pNext = pLevel->bufHeader.pNext;
		pBilFile->pPrev = &pLevel->bufHeader;
		pLevel->bufHeader.pNext->pPrev = pBilFile;
		pLevel->bufHeader.pNext = pBilFile;
	}

	return pBilFile;
}

/*
	����һ�����в㼶�߳����ݹ����ܽṹ��ľ����
	������pBILFile -- ����ɹ��򷵻��ļ��������������ʹ��
	����ֵ��
	     0 -- �ɹ����ļ������pBILFile��
		-1 -- �ڴ����ʧ��
*/
int bildataCreate(BILDATA * pBILFile)
{
	sBILDATA *pData;
	int i;

	pData = (sBILDATA *)NewFixedMemory(sizeof(sBILDATA));
	if (pData == NULL)
		return -1;

	pData->key = BILDATAKEY;
	/*������в㼶�߳����ݹ����ܽṹ���и������㼶�߳����ݹ���ṹ��ĳ�ʼ��,Ŀǰ֧��4���㼶*/
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
	ɾ��BILDATA������ͷ���ռ�õ���Դ
	������bildata -- ��bildataCreate���ص�BILDATA���
	����ֵ��
	     0 -- �ɹ�
		-1 -- �����Ч
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
	����ָ���㼶�ĸ߳�����Ŀ¼���߳�ͼ�����ȿ�ȡ��߳�ͼ��γ�ȿ�ȵ����в㼶�߳����ݹ����ܽṹ����
	������bildata -- ��bildataCreate���ص�BILDATA���
		  level -- ��� 0 ~ 3
	      cstrPathName -- ��/��β��Ŀ¼������ʾ�ò���������Ŀ¼
		  hscale -- һ��ͼ���ľ��ȿ��, 1/128�뵥λ
		  vscale -- һ��ͼ����γ�ȿ�ȣ�1/128�뵥λ
    ����ֵ��
	      0 -- �ɹ�
		 -1 -- �����Ч
		 -2 -- �㼶������Ч
*/
int bildataSetDataPath(BILDATA bildata, int level, const char * cstrPathName, int hscale, int vscale)
{
	sBILDATA * pBilData;
	sBILLEVEL * pLevel;

	/*�жϾ��keyֵ�Ƿ���ȷ*/
	CHECKBIL(bildata, -1);
	/*�жϲ㼶�Ƿ�����Ч��Χ*/
	if ( (level < 0) || (level >= BILLEVELS) )
		return -2;

	/*��ȡ���в㼶�߳����ݹ����ܽṹ��ָ��*/
	pBilData = (sBILDATA *)bildata;
	/*��ȡָ���㼶�ĸ߳����ݹ���ṹ��ָ��*/
	pLevel = &pBilData->levels[level];
	/*��ո߳�����ͼ������͸߳������ļ�����*/
	billevelDestroy(pLevel);
	/*��ʼ��ָ���㼶�ĸ߳����ݹ���ṹ��*/
	billevelInit(pLevel);
	/*���߳������ļ�Ŀ¼���߳�ͼ�����ȿ�ȡ��߳�ͼ��γ�ȿ��,���µ�ָ���㼶�ĸ߳����ݹ���ṹ����*/
	strcpy(pLevel->pathname, cstrPathName);
	pLevel->hscale = hscale;
	pLevel->vscale = vscale;

	return 0;
}

/*
	���ܣ���ָ���㼶��ԭʼ�߳������в�ѯָ����γ�ȵĸ߳�
	������bildata -- ��bildataCreate���ص�BILDATA���
		  level -- ��� 0-3
		  h -- ����, 1/128��Ϊ��λ
		  v -- γ��, 1/128��Ϊ��λ
	����ֵ��
		  >-9999: �����߳�
		   -9999: ԭʼ�߳����ݲ�����
		  -10001: ԭʼ�߳����ݳ�����Χ
		  -10002: ���ݾ����Ч
		  -10003: ������Ч
		  -10004: ����ռ����
*/
short bildataGetHeight(BILDATA bildata, int level, int h, int v)
{
	sBILDATA * pBilData;
	sBILLEVEL * pLevel;
	sBILFILE * pData;
	short data;

	/*�жϾ��keyֵ�Ƿ���ȷ*/
	CHECKBIL(bildata, -10002);
	/*�жϲ㼶�Ƿ�����Ч��Χ*/
	if ( (level < 0) || (level >= BILLEVELS) )
		return -10003;
	/* Ϊ�˷�������ļ��㼰�����ļ��������������и�ֵ�ľ�γ�ȶ�������������ֵ */
	if (h < 0)
		h += 180 * 3600 * 128;
	if (v < 0)
		v += 90 * 3600 * 128;

	/*��ȡ���в㼶�߳����ݹ����ܽṹ��ָ��*/
	pBilData = (sBILDATA *)bildata;
	/*��ȡָ���㼶�ĸ߳����ݹ���ṹ��ָ��*/
	pLevel = &pBilData->levels[level];
	/*��ԭʼ�߳��������ҵ��þ�γ�����ڵ�ͼ���߳����ݴ洢����ṹ����*/
	pData = billevelFindData(pLevel, h, v);
	/*ͼ���߳̾��Ϊ��,˵��û��ѯ���߳�ֵ,ֱ�ӷ���*/
	if (pData == NULL)
		return -10004;
	/*ÿ��ͼ������256*256���̵߳�,��ѯָ����γ���ڻ����е�λ��*/
	/*ͼ���еĸ߳����ݴ�ͼ�����Ͻǿ�ʼ���,���д洢*/
	/*hstart vstart�ֱ����ͼ�����½ǵľ��Ⱥ�γ��,hscale vscale�ֱ����ͼ���ľ��ȿ�Ⱥ�γ�ȿ��,��λ��1/128��*/
	h -= pData->hstart;
	v -= pData->vstart;
	h = h * BILSIZE / pLevel->hscale;
	v = v * BILSIZE / pLevel->vscale;
	v = BILSIZE - 1 - v;
	/*ֱ�������õ��߳�ֵ*/
	data = pData->data[v * BILSIZE + h];

	/*�жϸ߳������Ƿ񳬳���Χ*/
	if ((data > 10000) || (data < INVALID_DEM_VALUE))
		return -10001;

	return data;
}

/*
 * ��ʾ��ǰ�ڴ����������ͼ���߳��������Ѵ򿪵�ͼ�������ļ�����
 * */
void bildataPrintCacheCount(BILDATA bildata, int level)
{
	sBILDATA * pBilBlockData;
	sBILLEVEL * pLevel;

	pBilBlockData = (sBILDATA *)bildata;
	pLevel = &pBilBlockData->levels[level];
	
	printf("BilCount=%d,FileCount=%d\n", pLevel->bufCount, pLevel->blockCount);
}
