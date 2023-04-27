#ifdef WIN32
#include "windows.h"
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
//todo
#else
#include "vxWorks.h"
#endif

#include "stdio.h"
#include "stdlib.h"
#include "infofile.h"
#include "filepool.h"
#include "memoryPool.h"

#define INFOFILEKEY	(0xff5a03f8)

typedef struct sInfoFile
{
	f_uint64_t key;
	f_int32_t  rootlevel;
	f_int32_t  rootxidx;
	f_int32_t  rootyidx;
	f_int32_t  level;
	FILEITEM * pFileItem;
}sINFOFILE;

VOIDPtr infofileOpen(f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx, 
                     f_int32_t level, const f_char_t * pFileName)
{
	sINFOFILE * sInfo;
	FILE * pFile;
	pFile = fopen(pFileName, "rb");
	if (pFile == NULL)
	{
		return NULL;
	}
	sInfo = (sINFOFILE *)NewFixedMemory(sizeof(sINFOFILE));
	if (sInfo == NULL)
	{
		fclose(pFile);
		return NULL;
	}
	fclose(pFile);

	sInfo->rootlevel = rootlevel;
	sInfo->rootxidx  = rootxidx;
	sInfo->rootyidx  = rootyidx;
	sInfo->level     = level;
	sInfo->pFileItem = filepoolOpen(pFileName, 1);
	sInfo->key = INFOFILEKEY;
	return (VOIDPtr)sInfo;
}

void infofileClose(VOIDPtr infofile)
{
	sINFOFILE * sInfo;
	sInfo = (sINFOFILE *)infofile;
	if (sInfo == NULL)
		return ;
	if (sInfo->key != INFOFILEKEY)
		return ;
	if (sInfo->pFileItem == NULL)
	    return;
    filepoolClose(sInfo->pFileItem);
	sInfo->key = 0;
	DeleteAlterableMemory(sInfo);
}

f_int32_t loadinfofile(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                       VOIDPtr param, f_float32_t * ppInfo)
{
	FILE * pFile;
	f_int32_t llevel, lxidx, lyidx, piccount, i;
	f_int64_t offset;
	sINFOFILE * sInfo;
	sInfo = (sINFOFILE *)param;
	if (sInfo == NULL)
		return -1;
	if (sInfo->key != INFOFILEKEY)
		return -1;
	if(sInfo->pFileItem == NULL)
	    return -2;
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
	if (level < sInfo->rootlevel)
		return -1;

	llevel = level - sInfo->rootlevel;
	if (llevel >= sInfo->level)
		return -1;
		
	lxidx = xidx - sInfo->rootxidx * (1 << llevel);
	lyidx = yidx - sInfo->rootyidx * (1 << llevel);	
    pFile = filepoolGetFile(sInfo->pFileItem);
	if (pFile == 0)
		return -2;
	offset = 0;
	piccount = 1 << llevel;
	for (i = 0; i < llevel; i++)
	{
		offset += 1 << ( 2 * i);
	}
	offset += lyidx * piccount + lxidx;
	offset *= 9 * sizeof(f_float32_t);   /* 每个节点共9个参数 */
	if(fseek(pFile, offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(sInfo->pFileItem);
		return -1;
	}
	fread(ppInfo, 9 * sizeof(f_float32_t), 1, pFile);
    filepoolReleaseFile(sInfo->pFileItem);
	return 0;
}

