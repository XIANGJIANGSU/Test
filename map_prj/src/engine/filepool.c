#ifndef WIN32
#if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include <vxWorks.h>
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filepool.h"
#include "osAdapter.h"
#include "memoryPool.h"

#ifdef ACOREOS
#include <errno.h>
#endif

typedef struct tagFilePool
{
	struct tagFilePool *pNext;
	struct tagFilePool *pPrev;
	f_char_t * pFileName;
	FILE * pFile;
	f_int32_t filepointer;
	f_int32_t locked;
	f_int32_t read;
}FILEPOOL;

/*ʵ������ͬʱ�򿪵��ļ���Ŀ*/
#define FILEPOOLOPENFILE	(60)

static f_int32_t g_filepoolinited = 0;

static f_int32_t      g_fileopened = 0;
static FILEPOOL poollist;     /*�����Ѿ�ע����ļ�*/
static FILEPOOL poolopened;   /*�����Ѿ��򿪵��ļ�����ΪLRU��*/
/*
2016-11-17 11:27 ��Ǫ��
����ڶ���߳������ļ��ط����ļ�����Ҫ�����ź������б�����
*/
static SEMID    filepoolsem;  /* �ļ��ط��ʵĻ����ź��� */

/* ��ʼ���ļ��� */
void filepoolInit()
{
    filepoolsem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(filepoolsem);		
}

FILEITEM filepoolOpen(const f_char_t * pstrFileName, f_int32_t read)
{
	FILEPOOL * pItem;
	f_int32_t len;
	
	if(NULL == pstrFileName)
	    return 0;
	 
	takeSem(filepoolsem, FOREVER_WAIT);   
	if (g_filepoolinited == 0)
	{
		/*��ʼ���ļ��������ϵͳ*/
		g_filepoolinited = 1;
		memset(&poollist, 0, sizeof(FILEPOOL));
		poollist.pPrev = &poollist;
		poollist.pNext = &poollist;
		memset(&poolopened, 0, sizeof(FILEPOOL));
		poolopened.pPrev = &poolopened;
		poolopened.pNext = &poolopened;
		g_fileopened = 0;
	}
	/*�����ļ������Ŀ*/
	pItem = (FILEPOOL *)NewAlterableMemory(sizeof(FILEPOOL));
	if (pItem == 0)
	{
		giveSem(filepoolsem);
		return 0;
	}
	/*�����ļ���*/
	len = strlen(pstrFileName);
	memset(pItem, 0, sizeof(FILEPOOL));
	pItem->pFileName = (f_char_t *)NewAlterableMemory(len + 1);
	if (pItem->pFileName == 0)
	{
		DeleteAlterableMemory(pItem);
		pItem = NULL;
		giveSem(filepoolsem);
		return 0;
	}
	pItem->locked = 0;
	pItem->read = read;
	memcpy(pItem->pFileName, pstrFileName, len + 1);
	/*��ʱ���ļ�û��ʵ�ʴ򿪣������ӵ��ļ�����ر���*/
	pItem->pPrev = &poollist;
	pItem->pNext = poollist.pNext;
	pItem->pPrev->pNext = pItem;
	pItem->pNext->pPrev = pItem;
	
	giveSem(filepoolsem);
    return (FILEITEM)pItem;
}

/*Ϊ�˱�֤��ģ���ļ��ع��ܵ������ԣ�Ӧ���д˹��ܣ�����*/
void filepoolClose(FILEITEM hfile)
{
	FILEPOOL * pItem = NULL;
	pItem = (FILEPOOL *)hfile;
	if (pItem == 0)
		return ;
	takeSem(filepoolsem, FOREVER_WAIT); 
	if (pItem->pFile != 0)
	{  
		g_fileopened--;
		fclose(pItem->pFile);
		pItem->pFile = NULL;
	}
	/*��ԭ���ı���ȡ�£�Ȼ��ɾ����*/
	pItem->pNext->pPrev = pItem->pPrev;
	pItem->pPrev->pNext = pItem->pNext;
	DeleteAlterableMemory(pItem->pFileName);//2013-09-17 DXJ add
	pItem->pFileName = NULL;
	DeleteAlterableMemory(pItem);
	pItem = NULL;
	giveSem(filepoolsem);
}

FILE * filepoolGetFile(FILEITEM hfile)
{
	FILEPOOL * pItem;
	pItem = (FILEPOOL *)hfile;
	if (pItem == 0)
		return 0;
	takeSem(filepoolsem, FOREVER_WAIT); 
	if (pItem->pFile != 0)
	{
		/*���ļ�����ƶ�����ǰ��*/
		if (pItem->pPrev != &poolopened)
		{
			pItem->pNext->pPrev = pItem->pPrev;
			pItem->pPrev->pNext = pItem->pNext;
			pItem->pPrev = &poolopened;
			pItem->pNext = poolopened.pNext;
			pItem->pPrev->pNext = pItem;
			pItem->pNext->pPrev = pItem;		
		}
		pItem->locked = 1;
		giveSem(filepoolsem);
		return pItem->pFile;
	}
	if (g_fileopened >= FILEPOOLOPENFILE)
	{
		/*��ʱ���򿪵��ļ��Ѿ�������ˣ��رմ򿪱��е����һ���ļ���
		ΪҪ�򿪵��ļ��ڳ��ռ�*/
		FILEPOOL * pLastOpened;
		pLastOpened = poolopened.pPrev;
		while ( (pLastOpened != &poolopened) && (0 != pLastOpened->locked))
			pLastOpened = pLastOpened->pPrev;
		if (pLastOpened != &poolopened)
		{
			/*������?���ļ����ʱ��Ȼ�ǿյģ����Ǻ�����ļ�����ʧ��*/
			/*��ס��ǰ�ļ�λ��*/
			if(pLastOpened->pFile == 0)
				pLastOpened->filepointer = 0;
			else
			    pLastOpened->filepointer = ftell(pLastOpened->pFile);
			/*�ȹر��ļ�*/
#if 0       /* �öδ��������⣬���顣��Ǫ��20161011 */
			if(pLastOpened->pFile == 0)
			{
				printf("%d file opened88 %d \n", pLastOpened->pFile, g_fileopened);
			}
			else
			{
			    if(0 != fclose(pLastOpened->pFile))
			        printf("pLastOpened->pFile = %d \n", pLastOpened->pFile);
			}
#else
			fclose(pLastOpened->pFile);
#endif
			pLastOpened->pFile = NULL;
			g_fileopened--;
			/*��ԭ���ı���ȡ��*/
			pLastOpened->pNext->pPrev = pLastOpened->pPrev;
			pLastOpened->pPrev->pNext = pLastOpened->pNext;
			/*�ͻص��ļ�����*/
			pLastOpened->pPrev = &poollist;
			pLastOpened->pNext = poollist.pNext;
			pLastOpened->pPrev->pNext = pLastOpened;
			pLastOpened->pNext->pPrev = pLastOpened;
		}
		else
			{
				printf("open file more %d\n", g_fileopened);
			}
	}
	pItem->pFile = fopen(pItem->pFileName, (0 != pItem->read)?"rb":"wb");
	if (pItem->pFile == 0)
	{

#if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
		printf("%s open failed, already opened filenum=%d\n",pItem->pFileName, g_fileopened);
#else
		printf("%s open failed, errono=%d, already opened filenum=%d\n",pItem->pFileName,errno,g_fileopened);
#endif
		giveSem(filepoolsem);
		return 0;
	}
	g_fileopened++;
	/*��ԭ���ı���ȡ��*/
	pItem->pNext->pPrev = pItem->pPrev;
	pItem->pPrev->pNext = pItem->pNext;
	/*�ӵ��򿪵��ļ�����*/
	pItem->pPrev = &poolopened;
	pItem->pNext = poolopened.pNext;
	pItem->pPrev->pNext = pItem;
	pItem->pNext->pPrev = pItem;
	pItem->locked = 1;
	giveSem(filepoolsem);
	return pItem->pFile;
}
/*Ϊ�˱�֤��ģ���ļ��ع��ܵ������ԣ�Ӧ���д˹��ܣ�����*/
f_int32_t filepoolGetOpenedCount()
{
	return g_fileopened;
}
/*Ϊ�˱�֤��ģ���ļ��ع��ܵ������ԣ�Ӧ���д˹��ܣ�����*/
void filepoolReleaseFile(FILEITEM hfile)
{
	FILEPOOL * pItem;
	pItem = (FILEPOOL *)hfile;
	if (pItem == 0)
		return;
	takeSem(filepoolsem, FOREVER_WAIT); 
	pItem->locked = 0;
	giveSem(filepoolsem);
}

