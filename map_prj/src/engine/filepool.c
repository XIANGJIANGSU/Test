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

/*实际允许同时打开的文件数目*/
#define FILEPOOLOPENFILE	(60)

static f_int32_t g_filepoolinited = 0;

static f_int32_t      g_fileopened = 0;
static FILEPOOL poollist;     /*所有已经注册的文件*/
static FILEPOOL poolopened;   /*所有已经打开的文件，作为LRU表*/
/*
2016-11-17 11:27 张仟新
如果在多个线程中用文件池访问文件，需要采用信号量进行保护。
*/
static SEMID    filepoolsem;  /* 文件池访问的互斥信号量 */

/* 初始化文件池 */
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
		/*初始化文件句柄池子系统*/
		g_filepoolinited = 1;
		memset(&poollist, 0, sizeof(FILEPOOL));
		poollist.pPrev = &poollist;
		poollist.pNext = &poollist;
		memset(&poolopened, 0, sizeof(FILEPOOL));
		poolopened.pPrev = &poolopened;
		poolopened.pNext = &poolopened;
		g_fileopened = 0;
	}
	/*生成文件句柄项目*/
	pItem = (FILEPOOL *)NewAlterableMemory(sizeof(FILEPOOL));
	if (pItem == 0)
	{
		giveSem(filepoolsem);
		return 0;
	}
	/*保存文件名*/
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
	/*此时，文件没有实际打开，把它加到文件句柄池表中*/
	pItem->pPrev = &poollist;
	pItem->pNext = poollist.pNext;
	pItem->pPrev->pNext = pItem;
	pItem->pNext->pPrev = pItem;
	
	giveSem(filepoolsem);
    return (FILEITEM)pItem;
}

/*为了保证本模块文件池功能的完整性，应该有此功能，备用*/
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
	/*从原来的表中取下，然后删除它*/
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
		/*把文件句柄移动到最前面*/
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
		/*此时，打开的文件已经够多的了，关闭打开表中的最后一个文件，
		为要打开的文件腾出空间*/
		FILEPOOL * pLastOpened;
		pLastOpened = poolopened.pPrev;
		while ( (pLastOpened != &poolopened) && (0 != pLastOpened->locked))
			pLastOpened = pLastOpened->pPrev;
		if (pLastOpened != &poolopened)
		{
			/*可能吗?打开文件表此时居然是空的，除非后面打开文件总是失败*/
			/*记住当前文件位置*/
			if(pLastOpened->pFile == 0)
				pLastOpened->filepointer = 0;
			else
			    pLastOpened->filepointer = ftell(pLastOpened->pFile);
			/*先关闭文件*/
#if 0       /* 该段代码有问题，待查。张仟新20161011 */
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
			/*从原来的表中取下*/
			pLastOpened->pNext->pPrev = pLastOpened->pPrev;
			pLastOpened->pPrev->pNext = pLastOpened->pNext;
			/*送回到文件池中*/
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
	/*从原来的表中取下*/
	pItem->pNext->pPrev = pItem->pPrev;
	pItem->pPrev->pNext = pItem->pNext;
	/*加到打开的文件表中*/
	pItem->pPrev = &poolopened;
	pItem->pNext = poolopened.pNext;
	pItem->pPrev->pNext = pItem;
	pItem->pNext->pPrev = pItem;
	pItem->locked = 1;
	giveSem(filepoolsem);
	return pItem->pFile;
}
/*为了保证本模块文件池功能的完整性，应该有此功能，备用*/
f_int32_t filepoolGetOpenedCount()
{
	return g_fileopened;
}
/*为了保证本模块文件池功能的完整性，应该有此功能，备用*/
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

