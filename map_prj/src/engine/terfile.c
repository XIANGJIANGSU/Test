#ifdef WIN32
#include "windows.h"
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include "vxWorks.h"
#endif

#include "stdio.h"
#include "stdlib.h"
#include "terfile.h"
#include "filepool.h"
#include "memoryPool.h"

#define MAXTERLAYER	16
#define TERFILEKEY	(0xff5a03e8)
#define VERIFYTERFILE(ter) ( (ter != NULL) && (ter->key == TERFILEKEY) && (ter->pFileItem != NULL) ) 

typedef struct tagTerInfo{
	f_uint64_t offset;                /* 瓦片在文件中的偏移地址 */
	f_uint64_t length;                /* 瓦片数据的长度(字节数) */
}sTerInfo;

typedef struct sTerFile
{
	f_uint32_t key;
	FILEITEM * pFileItem;
	f_uint32_t ver;
	f_int32_t  rootlevel;	
	f_int32_t  rootxidx;
	f_int32_t  rootyidx;
	f_int32_t  level;
	f_int32_t  tilesize;
	f_uint32_t tilestart[MAXTERLAYER]; 
	sTerInfo   terinfo[1];   
}sTERFILE;

VOIDPtr terfileOpen(f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx, const f_char_t * pFileName)
{
	sTERFILE * pTer;
	FILE * pFile;
	f_uint8_t filehead[16], buf[8];
	f_uint32_t ver = 0, level = 0, tilesize = 0, piccount = 0, i = 0;
	
	pFile = fopen(pFileName, "rb");
	if (pFile == NULL)
	{
		DEBUG_PRINT("File open failed 0.");
		return NULL;
	}
	
	if (fread(filehead, 1, 16, pFile) != 16)
	{
		DEBUG_PRINT("File read failed.");
		fclose(pFile);
	    return NULL;
	}

	if ( (filehead[0] != 't') || (filehead[1] != 'e') || (filehead[2] != 'r') )
	{
		DEBUG_PRINT("File head error.");
		fclose(pFile);
	    return NULL;
	}

	ver = filehead[4] + 
		  filehead[5] * 256lu + 
		  filehead[6] * 256lu * 256lu + 
		  filehead[7] * 256lu * 256lu * 256lu;

	level = filehead[8] + 
		    filehead[9] * 256lu + 
		    filehead[10] * 256lu * 256lu + 
		    filehead[11] * 256lu * 256lu * 256lu;
	
	/*layer count must between 1 to MAXTQTLAYER*/
	if ( (level == 0) || (level > MAXTERLAYER) )
	{
		DEBUG_PRINT("level is out of range.");
		fclose(pFile);
	    return NULL;
	}

	tilesize = filehead[12] + 
		       filehead[13] * 256lu + 
		       filehead[14] * 256lu * 256lu + 
		       filehead[15] * 256lu * 256lu * 256lu;
	/*tile size must be 33*/
	if ( tilesize != 33 )
	{
		DEBUG_PRINT("tile size is error.");
		fclose(pFile);
	    return NULL;
	}

	piccount = 0;
	for (i = 0; i < level; i++)
	{
		piccount += 1 << (i * 2);
	}
	
	pTer = (sTERFILE *)NewAlterableMemory(sizeof(sTERFILE) + piccount * sizeof(sTerInfo));
	if (pTer == NULL)
	{
		DEBUG_PRINT("malloc failed.");
		fclose(pFile);
		return NULL;
	}
	memset(pTer, 0, sizeof(sTERFILE));
	pTer->key       = TERFILEKEY;
	pTer->ver       = ver;
	pTer->rootlevel = rootlevel;
	pTer->rootxidx  = rootxidx;
	pTer->rootyidx  = rootyidx;
	pTer->level     = level;
	pTer->tilesize  = tilesize;
	
	piccount = 0;
	for (i = 0; i < level; i++)
	{
		pTer->tilestart[i] = piccount;
		piccount += 1 << (i * 2);
	}
	
	for (i = 0; i < piccount; i++)
	{
		if (fread(buf, 1, 8, pFile) != 8)
		{
			DeleteAlterableMemory(pTer);
			DEBUG_PRINT("ter offset read error.");
		    fclose(pFile);
		    return NULL;
		}
		pTer->terinfo[i].offset = buf[0] + 
							      buf[1] * 256lu + 
							      buf[2] * 256lu * 256lu + 
							      buf[3] * 256lu * 256lu * 256lu;
        pTer->terinfo[i].length = buf[4] + 
							      buf[5] * 256lu + 
							      buf[6] * 256lu * 256lu + 
							      buf[7] * 256lu * 256lu * 256lu;
		if ( 0 == pTer->terinfo[i].length )
			pTer->terinfo[i].offset = 0xffffffff;	
	}

    fclose(pFile);
	pTer->pFileItem = filepoolOpen(pFileName, 1);	
	return (VOIDPtr)pTer;
}

void terfileClose(VOIDPtr terfile)
{
	sTERFILE * pTer;
	pTer = (sTERFILE *)terfile;
	if(!VERIFYTERFILE(pTer))
	{
		DEBUG_PRINT("pTer error.");
	    return;
	}
	if (pTer->pFileItem != NULL)
		filepoolClose(pTer->pFileItem);
	memset(pTer, 0, sizeof(sTERFILE));
	DeleteAlterableMemory(pTer);
	pTer = NULL;
}

/*
功能：判断节点是否在文件的范围内
输入：    
    level -- 指定的层，从0开始
	xidx, yidx -- 基本图块在层中的编号
	param  文件句柄所在的地址
输出：
    0  在
    -1 不在
*/
f_int32_t imgTerIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param)
{	
	f_int32_t llevel, lxidx, lyidx;
	sTERFILE *pTqs = (sTERFILE *)param;
	if(!VERIFYTERFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
	if (level < pTqs->rootlevel)
		return -1;
	llevel = level - pTqs->rootlevel;
	lxidx = xidx - pTqs->rootxidx * (1 << llevel);
	lyidx = yidx - pTqs->rootyidx * (1 << llevel);
	
	if ( (llevel < 0) || (llevel >= (f_int32_t)pTqs->level) )
		return -1;
	if ( (lxidx < 0) || (lxidx >= (1 << (llevel * 2))))
		return -1;
	if ( (lyidx < 0) || (lyidx >= (1 << (llevel * 2))))
		return -1;

	return 0;
}

/*
	得到一个已经打开的TQT文件中树的层数
	参数：tqsfile -- 用tqsfileOpen返回的TQT文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
f_int32_t terfileGetLevels(VOIDPtr tqsfile)
{
	sTERFILE *pTqs = (sTERFILE *)tqsfile;
	if(!VERIFYTERFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	return pTqs->level;
}

f_int32_t loadterfile(f_int32_t level, f_int32_t xidx, f_int32_t yidx, VOIDPtr param, f_int16_t *ppTerrain)
{
	FILE * pFile;
	f_int32_t llevel, lxidx, lyidx;
	f_uint64_t offset = 0;
	sTERFILE * pTer;
	pTer = (sTERFILE *)param;
	if(!VERIFYTERFILE(pTer))
	{
		//DEBUG_PRINT("pTer error.");
	    return -1;
	}
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
	
	// 瓦块层级在子树根节点之上,无法载入
	if (level < pTer->rootlevel)
		return -1;
	llevel = level - pTer->rootlevel;
	
	// 子树层级无法接入到当前层级,无法载入
	if( (llevel < 0) || (llevel >= pTer->level) )
	    return -1;
	    
	lxidx = xidx - pTer->rootxidx * (1 << llevel);
	if ( (lxidx < 0) || (lxidx >= (1 << (llevel * 2))))
		return -2;
	lyidx = yidx - pTer->rootyidx * (1 << llevel);	
	if ( (lyidx < 0) || (lyidx >= (1 << (llevel * 2))))
		return -2;
		
    pFile = filepoolGetFile(pTer->pFileItem);
	if (pFile == 0)
		return -3;
		
	offset = pTer->tilestart[llevel] + lyidx * (1 << llevel) + lxidx;
	if (pTer->terinfo[offset].offset == 0xffffffff)
		return -3;

//	printf("terrain offset = %d\n",pTer->terinfo[offset].offset);
	
	if(fseek(pFile, pTer->terinfo[offset].offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(pTer->pFileItem);
		return -3;
	}
	fread(ppTerrain, pTer->tilesize * pTer->tilesize * sizeof(f_int16_t), 1, pFile);
#ifdef _LITTLE_ENDIAN_	
	{
		f_int32_t xx = 0;
		f_int16_t *p = (f_int16_t*)ppTerrain;
		f_uint16_t temp = 0;
		for(; xx < pTer->tilesize * pTer->tilesize; xx++)
		{
			temp=*p;	
			*p = (temp << 8) | (temp >> 8);
			p++;
		}
	}
#endif
	filepoolReleaseFile(pTer->pFileItem);

	return 0;
}

