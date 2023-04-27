#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "tqsfile.h"
#include "filepool.h"
#include "vqtfile.h"
#include "memoryPool.h"
/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */
#include <setjmp.h>
#include "tqs2rgb.h"

#define MAXTQTLAYER	16
#define TQTFILEKEY	(0x747a7411)
#define VERIFYTQTFILE(tqs) ( (tqs != NULL) && (tqs->skey == TQTFILEKEY) && (tqs->pFileItem != NULL) ) 

#define CLRCHANNEL		3

/* ����һ��DDSͼƬ�Ĵ�С */
#define PICSIZE		(TILESIZE * TILESIZE * 3 / 6)

#define PICSIZE_RGBA		(TILESIZE * TILESIZE * 8 / 8)	// RJB,RJBA1:4/8 or RJBA3,RJBA5:8/8 


typedef struct tagPicInfo{
	f_uint64_t offset;                /* ��Ƭ���ļ��е�ƫ�Ƶ�ַ */
	f_uint64_t length;                /* ��Ƭ���ݵĳ���(�ֽ���) */
}sPicInfo;

typedef struct tagTQTFile
{
	f_uint64_t skey; /*must be TQTFILEKEY*/
	FILEITEM * pFileItem;
	f_uint64_t ver;
	f_uint64_t rootlevel;              /* ������ʼ�㼶�����Բ㼶�� */
	f_uint64_t rootxidx;
	f_uint64_t rootyidx;
	f_uint64_t level;                  /* ���ļ�������ȣ� */
	f_uint64_t tilesize;               /* ��Ƭ�ߴ磨�������� */
	f_uint64_t tilestart[MAXTQTLAYER]; /* ÿһ����Ƭ����ʼ�� */
	sPicInfo   picinfo[1];           /* ��Ƭ���ݵ���Ϣ */
}sTQTFile;


/*
	��һ��TQT�ļ���
	������cstrFileName -- �ļ�����
	����ֵ��
	     �ļ����
	     
˵����TQT�ļ���ʽ
    ��������(�tqs��)                    �޷����ַ���(4�ֽ�)
    �汾��Ϣ(ver)                        �޷�������(4�ֽ�)
    �Ĳ�������(layers)                   �޷�������(4�ֽ�)
    ��Ƭ�ߴ�(tile_size)                  �޷�������(4�ֽ�)
    ÿ����Ƭ�Ƿ���Ч                     �޷�������(1�ֽ�) * ��Ƭ����
    ��Ƭ0������
    ��Ƭ1������
    ...........
    ��Ƭn������
*/
VOIDPtr tqsfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName)
{
	FILE * pFile;
	sTQTFile * pTqs;
	f_uint8_t filehead[16]/*, buf[8]*/;
	f_uint32_t ver = 0, level = 0, tilesize = 0, piccount = 0, i = 0;
	f_uint32_t	offset;
	GLubyte		*pbPicValid = NULL;

	//printf("tqsfileOpen %s\n", cstrFileName);
	pFile = fopen(cstrFileName, "rb");
	if (pFile == NULL)
	{
		printf("File open failed 0\n");
		printf("tqsfileOpenFailed %s\n", cstrFileName);
		return NULL;
	}
	if (fread(filehead, 1, 16, pFile) != 16)
	{
		DEBUG_PRINT("File read failed.");
		fclose(pFile);
	    return NULL;
	}

	if (   ((filehead[0] != 't') || (filehead[1] != 'q') || (filehead[2] != 's')) 
		&& ((filehead[0] != 'c') || (filehead[1] != 'r') || (filehead[2] != 'h')) )
	{
		char	msg[200];
#ifndef WIN32
		sprintf(msg, "%s:File head error(%c%c%c),%s", __FUNCTION__, 
					filehead[0],filehead[1],filehead[2], cstrFileName);
#endif
		DEBUG_PRINT(msg);
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
	if ( (level == 0) || (level > MAXTQTLAYER) )
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

	piccount = 0;
	for (i = 0; i < level; i++)
	{
		piccount += 1 << (i * 2);
	}
	
	pTqs = (sTQTFile *)NewAlterableMemory(sizeof(sTQTFile) + piccount * sizeof(sPicInfo));
	if (pTqs == NULL)
	{
		DEBUG_PRINT("malloc failed.");
		fclose(pFile);
		return NULL;
	}
	memset(pTqs, 0, sizeof(sTQTFile));
	pTqs->skey = TQTFILEKEY;
	pTqs->ver = ver;
	pTqs->rootlevel = rootlevel;
	pTqs->rootxidx  = rootxidx;
	pTqs->rootyidx  = rootyidx;
	pTqs->level = level;
	pTqs->tilesize = tilesize;
	
	piccount = 0;
	for (i = 0; i < level; i++)
	{
		pTqs->tilestart[i] = piccount;
		piccount += 1 << (i * 2);
	}
	
	pbPicValid = (GLubyte*)malloc(piccount);
	if (fread(pbPicValid, 1, piccount, pFile) != piccount)
	{
		DEBUG_PRINT("tqsfileOpen:read valid flag failed.");
		free(pbPicValid);
		fclose(pFile);
	    return NULL;
	}
	
	offset = 16 + piccount;

	for (i = 0; i < piccount; i++)
	{
		pTqs->picinfo[i].offset = offset;
        pTqs->picinfo[i].length = PICSIZE;
		if ( 0 == pbPicValid[i] )
			pTqs->picinfo[i].offset = 0xffffffff;	
		offset += PICSIZE;
	}
	free(pbPicValid);

	fclose(pFile);
	pTqs->pFileItem = filepoolOpen(cstrFileName, 1);
	{
		f_char_t buf[256] = {0};
		sprintf(buf, "File %s opened", cstrFileName);
//	    DEBUG_PRINT(buf);
    }
	return (VOIDPtr)pTqs;
}

/*
	�ر�һ���Ѿ��򿪵�TQT�ļ����ͷ��ļ�ռ�õ���Դ
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileClose(VOIDPtr tqsfile)
{
	sTQTFile *pTqs = (sTQTFile *)tqsfile;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	
	if (pTqs->pFileItem != NULL)
		filepoolClose(pTqs->pFileItem);
	memset(pTqs, 0, sizeof(sTQTFile));
	DeleteAlterableMemory(pTqs);
	pTqs = NULL;
	return 0;
}

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ������Ĳ���
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileGetLevels(VOIDPtr tqsfile)
{
	sTQTFile *pTqs = (sTQTFile *)tqsfile;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	return pTqs->level;
}

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ���������ʼ���
	������tqsfile -- ��tqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileGetLevelstart(VOIDPtr tqsfile)
{
	sTQTFile *pTqs = (sTQTFile *)tqsfile;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	return pTqs->rootlevel;
}

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ��л���ͼ��Ĵ�С
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileGetTileSize(VOIDPtr tqsfile)
{
	sTQTFile * pTqs = (sTQTFile *)tqsfile;;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	return pTqs->tilesize;
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
f_int32_t imgTqsIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param)
{	
	f_int32_t llevel, lxidx, lyidx;
	sTQTFile *pTqs = (sTQTFile *)param;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*/
	if ((f_uint64_t)level < pTqs->rootlevel)
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
	�õ�һ��TQT�ļ��еĻ���ͼ������
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	      layer -- ָ���Ĳ㣬��0��ʼ
		  xidx, yidx -- ����ͼ���ڲ��еı��
		  pData -- ���ص����ݣ�����ɹ����أ������ݲ���ʱ������tqsfileFreeTileData�ͷ�
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
		-2 -- �����Ч
		-3 -- xidx��yidx��Ч
		-4 -- ���ݽ������
*/
static f_int32_t tqsfileGetData(sTQTFile *pTqs, f_int32_t level, f_int32_t xidx, f_int32_t yidx, void ** pData)
{
	f_uint64_t 	offset;
	f_uint32_t	ret = 0;
	FILE 		* pFile;
	f_uint8_t	*pImgData = NULL;

	
	if ( (level < 0) || (level >= (f_int32_t)pTqs->level) )
		return -2;
	if ( (xidx < 0) || (xidx >= (1 << (level * 2))))
		return -3;
	if ( (yidx < 0) || (yidx >= (1 << (level * 2))))
		return -3;
		
	offset = pTqs->tilestart[level] + yidx * (1 << level) + xidx;
	if (pTqs->picinfo[offset].offset == 0xffffffff)
		return -1;
	if (pData == NULL) 
		return 0;
	else
	    *pData = NULL;
	    
	pFile = filepoolGetFile(pTqs->pFileItem);
	if (pFile == 0)
		return -1;
	
//	printf("tqs offset = %d\n",pTqs->picinfo[offset].offset);
	
	if (fseek(pFile, pTqs->picinfo[offset].offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(pTqs->pFileItem);
		return -1;
	}

    pImgData = (f_uint8_t *)NewAlterableMemory(PICSIZE);
    ret = fread(pImgData, 1, PICSIZE, pFile);
	filepoolReleaseFile(pTqs->pFileItem);
    
	if (ret == PICSIZE )
	{
#ifdef USE_TQS2RGB
		*pData = ConvertDDS2RGB(pImgData, TILESIZE);
		DeleteAlterableMemory(pImgData);
#else
		*pData = pImgData;
#endif
	}
	else
	{
		DeleteAlterableMemory(pImgData);
		*pData = NULL;
		return -4;
	}

	return 0;
}

static f_int32_t tqsfileGetDataRGBA(sTQTFile *pTqs, f_int32_t level, f_int32_t xidx, f_int32_t yidx, void ** pData)
{
	f_uint64_t 	offset;
	f_uint32_t	ret = 0;
	FILE 		* pFile;
	f_uint8_t	*pImgData = NULL;

	
	if ( (level < 0) || (level >= (f_int32_t)pTqs->level) )
		return -2;
	if ( (xidx < 0) || (xidx >= (1 << (level * 2))))
		return -3;
	if ( (yidx < 0) || (yidx >= (1 << (level * 2))))
		return -3;
		
	offset = pTqs->tilestart[level] + yidx * (1 << level) + xidx;
	if (pTqs->picinfo[offset].offset == 0xffffffff)
		return -1;
	if (pData == NULL) 
		return 0;
	else
	    *pData = NULL;
	    
	pFile = filepoolGetFile(pTqs->pFileItem);
	if (pFile == 0)
		return -1;
	
//	printf("tqs offset = %d\n",pTqs->picinfo[offset].offset);
	
	if (fseek(pFile, pTqs->picinfo[offset].offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(pTqs->pFileItem);
		return -1;
	}

    pImgData = (f_uint8_t *)NewAlterableMemory(PICSIZE_RGBA);
    ret = fread(pImgData, 1, PICSIZE_RGBA, pFile);
	filepoolReleaseFile(pTqs->pFileItem);
    
	if (ret == PICSIZE_RGBA )
	{
		*pData = pImgData;
	}
	else
	{
		free(pImgData);
		*pData = NULL;
		return -4;
	}
	return 0;
}


f_int32_t loadtqs(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg)
{
	f_int32_t llevel, lxidx, lyidx;
	sTQTFile *pTqs = NULL;

	//�ж��Ƿ�Ϊ��
	if(param == 0)
	{
	//	DEBUG_PRINT("pTqs is NULL.");
	//	printf("level = %d, x = %d, y = %d\n",level,xidx,yidx);
		return -1;
	}

	pTqs = (sTQTFile *)param;
	
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*/
	if ((f_uint64_t)level < pTqs->rootlevel)
		return -1;
	llevel = level - pTqs->rootlevel;
	lxidx = xidx - pTqs->rootxidx * (1 << llevel);
	lyidx = yidx - pTqs->rootyidx * (1 << llevel);
	if (tqsfileGetData(pTqs, llevel, lxidx, lyidx, (void **)ppImg) != 0)
	{
		if (ppImg == 0)
			return -1;

		// Ӱ���������ʧ��, Ĭ���ÿ�
		*ppImg = (f_uint8_t *)NewAlterableMemory(pTqs->tilesize * pTqs->tilesize * CLRCHANNEL);
		memset(*ppImg, 0xFF, pTqs->tilesize * pTqs->tilesize * CLRCHANNEL);
		return -1;
	}

	return 0;
}

f_int32_t loadtqsRGBA(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg)
{
	f_int32_t llevel, lxidx, lyidx;
	sTQTFile *pTqs = NULL;

	//�ж��Ƿ�Ϊ��
	if(param == 0)
	{
	//	DEBUG_PRINT("pTqs is NULL.");
	//	printf("level = %d, x = %d, y = %d\n",level,xidx,yidx);
		return -1;
	}

	pTqs = (sTQTFile *)param;
	
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*/
	if ((f_uint64_t)level < pTqs->rootlevel)
		return -1;
	llevel = level - pTqs->rootlevel;
	lxidx = xidx - pTqs->rootxidx * (1 << llevel);
	lyidx = yidx - pTqs->rootyidx * (1 << llevel);
	if (tqsfileGetData(pTqs, llevel, lxidx, lyidx, (void **)ppImg) != 0)
	{
		if (ppImg == 0)
			return -1;
		*ppImg = (f_uint8_t *)NewAlterableMemory(pTqs->tilesize * pTqs->tilesize * 4);
		memset(*ppImg, 0xFF, pTqs->tilesize * pTqs->tilesize * 4);
		
	}
	return 0;
}

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC	glCompressedTexSubImage2D = NULL;
void sqsTextureFunInit(void)
{
	glCompressedTexImage2D 
		= (PFNGLCOMPRESSEDTEXIMAGE2DPROC)wglGetProcAddress("glCompressedTexImage2D");

	glCompressedTexSubImage2D 
		= (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)wglGetProcAddress("glCompressedTexSubImage2D");	
}
#endif
#endif
// ����Ӱ������������������
f_int32_t tqs2tex(f_uint64_t param , f_uint8_t *pImg)
{
	f_int32_t ret = -1;
	sTQTFile *pTqs = (sTQTFile *)param;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}

	glextGenTextures(1, (GLuint *)&ret);
	glBindTexture(GL_TEXTURE_2D, ret);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef USE_TQS2RGB
	//gluextBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pImg);
	glextTexImage2D( GL_TEXTURE_2D,  0, GL_RGB, TILESIZE, TILESIZE,  0, GL_RGB, GL_UNSIGNED_BYTE, pImg);
#else

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
 	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
		                   TILESIZE, TILESIZE, 0, PICSIZE, pImg);
#endif

#endif
	return ret;
}

f_int32_t tqs2texRGBA(f_uint64_t param , f_uint8_t *pImg)
{
	f_int32_t ret = -1;
	sTQTFile *pTqs = (sTQTFile *)param;
	if(!VERIFYTQTFILE(pTqs))
	{
		DEBUG_PRINT("pTqs error.");
	    return -1;
	}

	glextGenTextures(1, (GLuint *)&ret);
	glBindTexture(GL_TEXTURE_2D, ret);

//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//gluextBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pImg);
//	glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, TILESIZE, TILESIZE,  0, GL_RGBA, GL_UNSIGNED_BYTE, pImg);

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TILESIZE, TILESIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImg);

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 
		                   TILESIZE, TILESIZE, 0, PICSIZE_RGBA, pImg);
#endif
	return ret;
}

