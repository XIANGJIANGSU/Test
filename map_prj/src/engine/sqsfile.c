#include <stdio.h>
#include <stdlib.h>

#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "sqsfile.h"
#include "filepool.h"
#include "memoryPool.h"

#define MAXSQSLEVEL	16
#define SQSFILEKEY	(0x775534fc)

#define VERIFYSQSFILE(sqs) ( (sqs != NULL) && (sqs->skey == SQSFILEKEY) && (sqs->pFileItem != NULL) )

typedef struct tagSQSFile
{
	f_uint64_t skey; /*must be SQSFILEKEY*/
	FILEITEM * pFileItem;
	f_uint64_t ver;
	f_uint64_t rootlevel;              /* 树的起始层级（绝对层级） */
	f_uint64_t rootxidx;
	f_uint64_t rootyidx;
	f_uint64_t level;                  /* 树的级数（深度） */
	f_uint64_t tilesize;               /* 瓦片尺寸（像素数） */
	f_uint64_t mipmapcount;            /* mipmap级数 */
	f_uint64_t picsize;                /* mipmapcount级图像大小（字节数） */
	f_uint64_t tilestart[MAXSQSLEVEL]; /* 每一级瓦片的起始号 */
}sSQSFile; /* 一个SQS文件对应一棵四叉树 */

/*
	得到一个SQS文件中的基本图块数据
	参数：pSqs -- 文件句柄
	      level -- 指定的层，从0开始
		  xidx, yidx -- 基本图块在层中的编号
		  pData -- 返回的数据，如果成功返回
	返回值：
	     1 -- 数据正在调入
	     0 -- 成功
		-1 -- 文件句柄无效
		-2 -- 层号无效或xidx或yidx无效
		-3 -- 内存分配错误
		-4 -- 数据解码错误
*/
static f_int32_t sqsfileGetData(sSQSFile *pSqs, f_int32_t level, f_int32_t xidx, f_int32_t yidx, void ** pData)
{
	f_uint64_t offset;
	f_int32_t ret;
	FILE * pFile;
	    
	if ( (level < 0) || (level >= (f_int32_t)pSqs->level) )
		return -2;
	if ( (xidx < 0) || (xidx >= (1 << (level * 2))))
		return -2;
	if ( (yidx < 0) || (yidx >= (1 << (level * 2))))
		return -2;
	
	*pData = (f_uint8_t *)NewAlterableMemory(pSqs->picsize);
	if (*pData == NULL)
		return -3;
	offset = pSqs->tilestart[level] + yidx * (1 << level) + xidx;
	offset *= pSqs->picsize;
	offset += 32;   /* 32个字节的文件头信息 */
	pFile = filepoolGetFile(pSqs->pFileItem);
	if (pFile == 0)
	{
		DeleteAlterableMemory(*pData);
		*pData = NULL;
		return -1;
	}
	if (fseek(pFile, offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(pSqs->pFileItem);
		DeleteAlterableMemory(*pData);
		*pData = NULL;
		return -1;
	}
	ret = fread(*pData, 1, pSqs->picsize, pFile);
	filepoolReleaseFile(pSqs->pFileItem);
	if (ret <= 0)
	{
		DeleteAlterableMemory(*pData);
		*pData = NULL;
		return -4;
	}
	return 0;
}

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D ;
#endif
#endif

static void sqsfileMakeTexture(f_int32_t tilesize, f_int32_t mipmapcount, f_uint8_t * pBuf, f_int32_t * texture)
{
	f_int32_t size;
	f_int32_t i;
    f_float32_t fLargest;
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	//glEnableEx(GL_TEXTURE_2D);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glGenTextures(1, (GLuint *)texture);			/* Generate OpenGL texture IDs*/
	glBindTexture(GL_TEXTURE_2D, *texture);			/* Bind Our Texture*/
#ifdef WIN32
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapcount-1);

	size = tilesize;
	size *= size;
	size /= 16;
	size *= 8;
#ifdef WIN32
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
		                   tilesize, tilesize, 0, size, pBuf);
#endif	
	/* set mipmap Filtering and load all level*/
	for (i = 1; i < mipmapcount; i++)
	{
		pBuf += size;
		tilesize /= 2;
		size /= 4;
#ifdef WIN32
		glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
							tilesize, tilesize, 0, size, pBuf);
#endif
	}
#endif
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
f_int32_t imgSqsIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param)
{
	sSQSFile *pSqs;
	f_int32_t llevel, lxidx, lyidx;
	pSqs = (sSQSFile *)param;
	if(!VERIFYSQSFILE(pSqs))
	{
		DEBUG_PRINT("pSqs error.");
	    return -1;
	}
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
	if ((f_uint64_t)level < pSqs->rootlevel)
		return -1;
	llevel = level - pSqs->rootlevel;
	lxidx = xidx - pSqs->rootxidx * (1 << llevel);
	lyidx = yidx - pSqs->rootyidx * (1 << llevel);
	
	if ( (llevel < 0) || (llevel >= (f_int32_t)pSqs->level) )
		return -1;
	if ( (lxidx < 0) || (lxidx >= (1 << (llevel * 2))))
		return -1;
	if ( (lyidx < 0) || (lyidx >= (1 << (llevel * 2))))
		return -1;

	return 0;
}

f_int32_t loadsqs(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg)
{
	f_int32_t llevel, lxidx, lyidx;
	sSQSFile *pSqs = (sSQSFile *)param;
	if(!VERIFYSQSFILE(pSqs))
	{
		DEBUG_PRINT("pSqs error.");
	    return -1;
	}
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
	if ((f_uint64_t)level < pSqs->rootlevel)
		return -1;
	llevel = level - pSqs->rootlevel;
	lxidx = xidx - pSqs->rootxidx * (1 << llevel);
	lyidx = yidx - pSqs->rootyidx * (1 << llevel);

	if (sqsfileGetData(pSqs, llevel, lxidx, lyidx, (void **)ppImg) != 0)
	{
		if (ppImg == 0)
		{
			return -1;
		}
		*ppImg = (f_uint8_t *)NewAlterableMemory(TILESIZE * TILESIZE * 3);
		memset(*ppImg, 0xFF, TILESIZE * TILESIZE * 3);
	}
	return 0;
}

f_int32_t sqs2tex(f_uint64_t param , f_uint8_t *pImg)
{
	f_int32_t ret = -1;
	sSQSFile *pSqs;
	pSqs = (sSQSFile *)param;
	if(!VERIFYSQSFILE(pSqs))
	{
		DEBUG_PRINT("pSqs error.");
	    return -1;
	}
	sqsfileMakeTexture(pSqs->tilesize, pSqs->mipmapcount, pImg, &ret);
	return ret;
}

/*
	打开一个SQS文件，
	参数：cstrFileName -- 文件名称
	返回值：
	    文件句柄
*/
VOIDPtr sqsfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName)
{
	FILE * pFile;
	sSQSFile * pSqs;
	f_uint8_t filehead[32];
	f_uint64_t ver;
	f_uint64_t level;
	f_uint64_t tilesize;
	f_uint64_t piccount;
	f_uint64_t mipmapcount;
	f_uint64_t tsize;
	f_uint64_t i;

	pFile = fopen(cstrFileName, "rb");
	if (pFile == NULL)
	{
		DEBUG_PRINT("File open failed 0.");
		return NULL;
	}
	if (fread(filehead, 1, 32, pFile) != 32)
	{
		DEBUG_PRINT("File read failed.");
		fclose(pFile);
	    return NULL;
	}
	if ( (filehead[0] != 0x53) || 
		 (filehead[1] != 0x51) || 
		 (filehead[2] != 0x53) || 
		 (filehead[3] != 0xEC) )
	{
		DEBUG_PRINT("File key is error.");
		fclose(pFile);
	    return NULL;
	}
	ver = filehead[4] + 
		  filehead[5] * 256lu + 
		  filehead[6] * 256lu * 256lu + 
		  filehead[7] * 256lu * 256lu * 256lu;
	/*version must be 0x00000005*/
	if (ver != 5)
	{
		DEBUG_PRINT("File version is error.");
		fclose(pFile);
	    return NULL;
	}

	level = filehead[8] + 
		  filehead[9] * 256lu + 
		  filehead[10] * 256lu * 256lu + 
		  filehead[11] * 256lu * 256lu * 256lu;
	/*level count must between 1 to MAXSQSLEVEL*/
	if ( (level == 0) || (level > MAXSQSLEVEL) )
	{
		DEBUG_PRINT("level is out of range.");
		fclose(pFile);
	    return NULL;
	}

	tilesize = filehead[12] + 
		  filehead[13] * 256lu + 
		  filehead[14] * 256lu * 256lu + 
		  filehead[15] * 256lu * 256lu * 256lu;
	/*tile size must be 128, 256, 512 or 1024*/
	if ( (tilesize != 128) && 
		 (tilesize != 256) && 
		 (tilesize != 512) && 
		 (tilesize != 1024) )
	{
		DEBUG_PRINT("tile size is error.");
		fclose(pFile);
	    return NULL;
	}
	mipmapcount = filehead[16] + 
		  filehead[17] * 256lu + 
		  filehead[18] * 256lu * 256lu + 
		  filehead[19] * 256lu * 256lu * 256lu;
	if ( (mipmapcount < 1) || (mipmapcount > 12))
	{
		DEBUG_PRINT("mipmapcount is error.");
		fclose(pFile);
	    return NULL;
	}
	
	pSqs = (sSQSFile *)NewAlterableMemory(sizeof(sSQSFile));
	if (pSqs == NULL)
	{
		DEBUG_PRINT("malloc failed.");
		fclose(pFile);
		return NULL;
	}
	memset(pSqs, 0, sizeof(sSQSFile));
	pSqs->skey = SQSFILEKEY;
	pSqs->ver = ver;
	pSqs->rootlevel = rootlevel;
	pSqs->rootxidx  = rootxidx;
	pSqs->rootyidx  = rootyidx;
	pSqs->level = level;
	pSqs->tilesize = tilesize;
	pSqs->mipmapcount = mipmapcount;
	tsize = (tilesize * tilesize / 16) * 8; /*假定宽度和高度能够被4整除，压缩方式为DXT1*/
	pSqs->picsize = 0;
	for (i = 0;i<mipmapcount;i++)
	{
		pSqs->picsize += tsize;
		tsize /= 4;
	}

	piccount = 0;
	for (i = 0;i < level;i++)
	{
		pSqs->tilestart[i] = piccount;
		piccount += 1 << (i * 2);
	}
	fclose(pFile);
	pSqs->pFileItem = filepoolOpen(cstrFileName, 1);
	{
		f_char_t buf[256] = {0};
		sprintf(buf, "File %s opened", cstrFileName);
	    DEBUG_PRINT(buf);
    }
	return(VOIDPtr)pSqs;
}

/*
	关闭一个已经打开的SQS文件，释放文件占用的资源
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileClose(VOIDPtr sqsfile)
{
	sSQSFile *pSqs = (sSQSFile *)sqsfile;
	if(!VERIFYSQSFILE(pSqs))
	    return -1;

	if (pSqs->pFileItem != NULL)
		filepoolClose(pSqs->pFileItem);
	memset(pSqs, 0, sizeof(sSQSFile));
	DeleteAlterableMemory(pSqs);
	pSqs = NULL;
	return 0;
}

/*
	得到一个已经打开的SQS文件中树的层数
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetLevels(VOIDPtr sqsfile)
{
	sSQSFile * pSqs;
	pSqs = (sSQSFile *)sqsfile;
	if(!VERIFYSQSFILE(pSqs))
	    return -1;	
	return pSqs->level;
}

/*
	得到一个已经打开的SQS文件中树的起始层号
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 树的起始层号
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetLevelstart(VOIDPtr sqsfile)
{
	sSQSFile * pSqs;
	pSqs = (sSQSFile *)sqsfile;
	if(!VERIFYSQSFILE(pSqs))
	    return -1;	
	return pSqs->rootlevel;
}

/*
	得到一个已经打开的SQS文件中基本图块的MIPMAP层数
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 基本图块的MIPMAP层数
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetMipMapCount(VOIDPtr sqsfile)
{
	sSQSFile * pSqs;
	pSqs = (sSQSFile *)sqsfile;
	if(!VERIFYSQSFILE(pSqs))
	    return -1;	
	return pSqs->mipmapcount;
}

/*
	得到一个已经打开的SQS文件中基本图块的大小
	参数：sqsfile -- 用sqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 基本图块的大小
		-1 -- 文件句柄无效
*/
f_int32_t sqsfileGetTileSize(VOIDPtr sqsfile)
{
	sSQSFile * pSqs;
	pSqs = (sSQSFile *)sqsfile;
	if(!VERIFYSQSFILE(pSqs))
	    return -1;	
	return pSqs->tilesize;
}
