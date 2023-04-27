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
	f_uint64_t rootlevel;              /* ������ʼ�㼶�����Բ㼶�� */
	f_uint64_t rootxidx;
	f_uint64_t rootyidx;
	f_uint64_t level;                  /* ���ļ�������ȣ� */
	f_uint64_t tilesize;               /* ��Ƭ�ߴ磨�������� */
	f_uint64_t mipmapcount;            /* mipmap���� */
	f_uint64_t picsize;                /* mipmapcount��ͼ���С���ֽ����� */
	f_uint64_t tilestart[MAXSQSLEVEL]; /* ÿһ����Ƭ����ʼ�� */
}sSQSFile; /* һ��SQS�ļ���Ӧһ���Ĳ��� */

/*
	�õ�һ��SQS�ļ��еĻ���ͼ������
	������pSqs -- �ļ����
	      level -- ָ���Ĳ㣬��0��ʼ
		  xidx, yidx -- ����ͼ���ڲ��еı��
		  pData -- ���ص����ݣ�����ɹ�����
	����ֵ��
	     1 -- �������ڵ���
	     0 -- �ɹ�
		-1 -- �ļ������Ч
		-2 -- �����Ч��xidx��yidx��Ч
		-3 -- �ڴ�������
		-4 -- ���ݽ������
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
	offset += 32;   /* 32���ֽڵ��ļ�ͷ��Ϣ */
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
���ܣ��жϽڵ��Ƿ����ļ��ķ�Χ��
���룺    
    level -- ָ���Ĳ㣬��0��ʼ
	xidx, yidx -- ����ͼ���ڲ��еı��
	param  �ļ�������ڵĵ�ַ
�����
    0  ��
    -1 ����
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
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*/
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
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*/
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
	��һ��SQS�ļ���
	������cstrFileName -- �ļ�����
	����ֵ��
	    �ļ����
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
	tsize = (tilesize * tilesize / 16) * 8; /*�ٶ���Ⱥ͸߶��ܹ���4������ѹ����ʽΪDXT1*/
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
	�ر�һ���Ѿ��򿪵�SQS�ļ����ͷ��ļ�ռ�õ���Դ
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
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
	�õ�һ���Ѿ��򿪵�SQS�ļ������Ĳ���
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
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
	�õ�һ���Ѿ��򿪵�SQS�ļ���������ʼ���
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
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
	�õ�һ���Ѿ��򿪵�SQS�ļ��л���ͼ���MIPMAP����
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ����ͼ���MIPMAP����
		-1 -- �ļ������Ч
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
	�õ�һ���Ѿ��򿪵�SQS�ļ��л���ͼ��Ĵ�С
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*/
f_int32_t sqsfileGetTileSize(VOIDPtr sqsfile)
{
	sSQSFile * pSqs;
	pSqs = (sSQSFile *)sqsfile;
	if(!VERIFYSQSFILE(pSqs))
	    return -1;	
	return pSqs->tilesize;
}
