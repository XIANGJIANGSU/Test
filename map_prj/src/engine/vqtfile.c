#include <stdio.h>
#include <stdlib.h>

#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "vqtfile.h"
#include "filepool.h"
#include "../../../libpng/src/ReaderWriterPNG.h"
#include "memoryPool.h"
//#include "../../jpgLib_prj/src/jpeglib.h"
#include "tqtfile.h"
#include "vqtfile.h"

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */
//#include <setjmp.h>
 
#define MAXVQTLAYER	16
#define VQTFILEKEY	(0x74717401)
#define VERIFYVQTFILE(tqt) ( (tqt != NULL) && (tqt->skey == VQTFILEKEY) && (tqt->pFileItem != NULL) ) 

typedef struct tagPicInfo{
	f_uint64_t offset;                /* ��Ƭ���ļ��е�ƫ�Ƶ�ַ */
	f_uint64_t length;                /* ��Ƭ���ݵĳ���(�ֽ���) */
}sPicInfo;

typedef struct tagVQTFile
{
	f_uint64_t skey; /*must be TQTFILEKEY*/
	FILEITEM * pFileItem;
	f_uint64_t ver;
	f_uint64_t rootlevel;              /* ������ʼ�㼶�����Բ㼶�� */
	f_uint64_t rootxidx;
	f_uint64_t rootyidx;
	f_uint64_t level;                  /* ���ļ�������ȣ� */
	f_uint64_t tilesize;               /* ��Ƭ�ߴ磨�������� */
	f_uint64_t tilestart[MAXVQTLAYER]; /* ÿһ����Ƭ����ʼ�� */
	sPicInfo   picinfo[1];           /* ��Ƭ���ݵ���Ϣ */
}sVQTFile;

static int VqtRGBAchannel = 4;

/*
	��һ��VQT�ļ���
	������cstrFileName -- �ļ�����
	����ֵ��
	     �ļ����
	     
˵����VQT�ļ���ʽ
    ��������(�vqt��)                    �޷����ַ���(4�ֽ�)
    �汾��Ϣ(ver)                        �޷�������(4�ֽ�)
    �Ĳ�������(layers)                   �޷�������(4�ֽ�)
    ��Ƭ�ߴ�(tile_size)                  �޷�������(4�ֽ�)
    ��Ƭ0���ļ��е�ƫ�Ƶ�ַ(pic_offset)  �޷�������(4�ֽ�)
    ��Ƭ0�ĳ���(�ֽ���,������������FFFF) �޷�������(4�ֽ�)
    ��Ƭ1���ļ��е�ƫ�Ƶ�ַ(pic_offset)  �޷�������(4�ֽ�)
    ��Ƭ1�ĳ���(�ֽ���)                  �޷�������(4�ֽ�)
    ..........
    ..........
    ��Ƭn���ļ��е�ƫ�Ƶ�ַ(pic_offset)  �޷�������(4�ֽ�)
    ��Ƭn�ĳ���(�ֽ���)                  �޷�������(4�ֽ�)
    ��Ƭ0������
    ��Ƭ1������
    ...........
    ��Ƭn������
*/
VOIDPtr vqtfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName)
{
	FILE * pFile;
	sVQTFile * pTqt;
	f_uint8_t filehead[16], buf[8];
	f_uint32_t ver = 0, level = 0, tilesize = 0, piccount = 0, i = 0;

	pFile = fopen(cstrFileName, "rb");
	if (pFile == NULL)
	{
	//	DEBUG_PRINT("File open failed 0.");
	//	printf("%s\n",cstrFileName);
		return NULL;
	}
	if (fread(filehead, 1, 16, pFile) != 16)
	{
		DEBUG_PRINT("File read failed.");
		fclose(pFile);
	    return NULL;
	}

	if ( (filehead[0] != 't') || (filehead[1] != 'q') || (filehead[2] != 't') )
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
	if ( (level == 0) || (level > MAXVQTLAYER) )
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

	pTqt = (sVQTFile *)NewAlterableMemory(sizeof(sVQTFile) + piccount * sizeof(sPicInfo));
	if (pTqt == NULL)
	{
		DEBUG_PRINT("malloc failed.");
		fclose(pFile);
		return NULL;
	}
	memset(pTqt, 0, sizeof(sVQTFile));
	pTqt->skey = VQTFILEKEY;
	pTqt->ver = ver;
	pTqt->rootlevel = rootlevel;
	pTqt->rootxidx  = rootxidx;
	pTqt->rootyidx  = rootyidx;
	pTqt->level = level;
	pTqt->tilesize = tilesize;
	
	piccount = 0;
	for (i = 0; i < level; i++)
	{
		pTqt->tilestart[i] = piccount;
		piccount += 1 << (i * 2);
	}
	
	for (i = 0; i < piccount; i++)
	{
		if (fread(buf, 1, 8, pFile) != 8)
		{
			DeleteAlterableMemory(pTqt);
			DEBUG_PRINT("vqt offset read error.");
		    fclose(pFile);
		    return NULL;
		}
		pTqt->picinfo[i].offset = buf[0] + 
							      buf[1] * 256lu + 
							      buf[2] * 256lu * 256lu + 
							      buf[3] * 256lu * 256lu * 256lu;
        pTqt->picinfo[i].length = buf[4] + 
							      buf[5] * 256lu + 
							      buf[6] * 256lu * 256lu + 
							      buf[7] * 256lu * 256lu * 256lu;
		if ( 0 == pTqt->picinfo[i].length )
			pTqt->picinfo[i].offset = 0xffffffff;	
	}

	fclose(pFile);
	pTqt->pFileItem = filepoolOpen(cstrFileName, 1);
	{
		f_char_t buf[256] = {0};
		sprintf(buf, "File %s opened", cstrFileName);
//	    DEBUG_PRINT(buf);
    }
	return (VOIDPtr)pTqt;
}

/*
	�ر�һ���Ѿ��򿪵�TQT�ļ����ͷ��ļ�ռ�õ���Դ
	������tqtfile -- ��tqtfileOpen���ص�TQT�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*
f_int32_t tqtfileClose(VOIDPtr tqtfile)
{
	sTQTFile *pTqt = (sTQTFile *)tqtfile;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	
	if (pTqt->pFileItem != NULL)
		filepoolClose(pTqt->pFileItem);
	memset(pTqt, 0, sizeof(sTQTFile));
	free(pTqt);
	pTqt = NULL;
	return 0;
}

*
	�õ�һ���Ѿ��򿪵�TQT�ļ������Ĳ���
	������tqtfile -- ��tqtfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*
f_int32_t tqtfileGetLevels(VOIDPtr tqtfile)
{
	sTQTFile *pTqt = (sTQTFile *)tqtfile;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	return pTqt->level;
}

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ���������ʼ���
	������tqtfile -- ��tqtfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
*
f_int32_t tqtfileGetLevelstart(VOIDPtr tqtfile)
{
	sTQTFile *pTqt = (sTQTFile *)tqtfile;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	return pTqt->rootlevel;
}

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ��л���ͼ��Ĵ�С
	������tqtfile -- ��tqtfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*
f_int32_t tqtfileGetTileSize(VOIDPtr tqtfile)
{
	sTQTFile * pTqt = (sTQTFile *)tqtfile;;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	return pTqt->tilesize;
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
*
f_int32_t imgTqtIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param)
{	
	f_int32_t llevel, lxidx, lyidx;
	sTQTFile *pTqt = (sTQTFile *)param;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*
	if ((f_uint64_t)level < pTqt->rootlevel)
		return -1;
	llevel = level - pTqt->rootlevel;
	lxidx = xidx - pTqt->rootxidx * (1 << llevel);
	lyidx = yidx - pTqt->rootyidx * (1 << llevel);
	
	if ( (llevel < 0) || (llevel >= (f_int32_t)pTqt->level) )
		return -1;
	if ( (lxidx < 0) || (lxidx >= (1 << (llevel * 2))))
		return -1;
	if ( (lyidx < 0) || (lyidx >= (1 << (llevel * 2))))
		return -1;

	return 0;
}

 */


//����ֵ: -1,����1,��ȷ
int read_PNG_vqtfile (FILE * pFile, unsigned char **ppImg, int *width, int *height, int * rgbaChannel)
{
	pngInfo info;
#if !defined(__ANDROID__) && !defined(ANDROID) && !defined(__linux__)
	if( 0 == pngLoadFPic(pFile, ppImg, &info, rgbaChannel))
	{	
		*width = *height = 0;
		return -1;
	}
	*width = info.Width;
	*height = info.Height;
#endif 

 	return 1;

}

//int jpegDecode(FILE * pFile, unsigned char ** ppBuf, int *picWidth, int *picHeight);

/*
	�õ�һ��VQT�ļ��еĻ���ͼ������
	������vqtfile -- ��vqtfileOpen���ص�VQT�ļ����
	      layer -- ָ���Ĳ㣬��0��ʼ
		  xidx, yidx -- ����ͼ���ڲ��еı��
		  pData -- ���ص����ݣ�����ɹ����أ������ݲ���ʱ������vqtfileFreeTileData�ͷ�
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
		-2 -- �����Ч
		-3 -- xidx��yidx��Ч
		-4 -- ���ݽ������
*/
static f_int32_t vqtfileGetData(sVQTFile *pTqt, f_int32_t level, f_int32_t xidx, f_int32_t yidx, void ** pData)
{
	f_uint64_t offset;
	f_int32_t width, height, ret;
	FILE * pFile;
	
	if ( (level < 0) || (level >= (f_int32_t)pTqt->level) )
		return -2;
	if ( (xidx < 0) || (xidx >= (1 << (level * 2))))
		return -3;
	if ( (yidx < 0) || (yidx >= (1 << (level * 2))))
		return -3;
		
	offset = pTqt->tilestart[level] + yidx * (1 << level) + xidx;
	if (pTqt->picinfo[offset].offset == 0xffffffff)
		return -1;
	if (pData == NULL) 
		return 0;
	else
	    *pData = NULL;
	    
	pFile = filepoolGetFile(pTqt->pFileItem);
	if (pFile == 0)
		return -1;
	
//	printf("tqt offset = %d\n",pTqt->picinfo[offset].offset);
	
	if (fseek(pFile, pTqt->picinfo[offset].offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(pTqt->pFileItem);
		return -1;
	}
/*	fread(buf, 1, 2, pFile);
	if ( (buf[0] != 0xFF) || (buf[1] != 0xD8) )
		return -1;
	fseek(pFile, pTqt->picinfo[offset].offset, SEEK_SET);*/
//	ret = jpegDecode(pFile, (unsigned char **)pData, &width, &height);

    ret = read_PNG_vqtfile(pFile, (f_uint8_t **)pData, &width, &height, &VqtRGBAchannel);

//	setTqtAlpha( * pData,  width,  height, 19);
	
	filepoolReleaseFile(pTqt->pFileItem);
	if (ret < 0)
	{
		*pData = NULL;
		return -4;
	}
	return 0;
}

f_int32_t loadvqt(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg)
{
	f_int32_t llevel, lxidx, lyidx;
	sVQTFile *pTqt = NULL;

	//�ж��Ƿ�Ϊ��
	if(param == 0)
	{
		return -1;
	}

	pTqt = (sVQTFile *)param;
	
	if(!VERIFYVQTFILE(pTqt))
	{
		DEBUG_PRINT("pVqt error.");
	    return -1;
	}
	/*ȫ�ֵ�level, xidx, yidxת�ɾֲ���level, xidx, yidx*/
	if ((f_uint64_t)level < pTqt->rootlevel)
		return -1;
	llevel = level - pTqt->rootlevel;
	lxidx = xidx - pTqt->rootxidx * (1 << llevel);
	lyidx = yidx - pTqt->rootyidx * (1 << llevel);
	if (vqtfileGetData(pTqt, llevel, lxidx, lyidx, (void **)ppImg) != 0)
	{
		if (ppImg == 0)
			return -1;
		*ppImg = (f_uint8_t *)NewAlterableMemory(pTqt->tilesize * pTqt->tilesize * VqtRGBAchannel);
		memset(*ppImg, 0x00, pTqt->tilesize * pTqt->tilesize * VqtRGBAchannel);
	}
	return 0;
}

f_int32_t vqt2tex(f_uint64_t param , f_uint8_t *pImg)
{
	f_int32_t ret = -1;
	sVQTFile *pVqt = (sVQTFile *)param;
	if(!VERIFYVQTFILE(pVqt))
	{
		DEBUG_PRINT("pVqt error.");
	    return -1;
	}

	if(VqtRGBAchannel == 4)
	{
	//glEnableEx(GL_TEXTURE_2D);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  4);

    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pImg);
	glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, TILESIZE, TILESIZE,  0, GL_RGBA, GL_UNSIGNED_BYTE, pImg);
	
	}
	else if(VqtRGBAchannel == 3)
	{
	//glEnableEx(GL_TEXTURE_2D);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  4);

    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pImg);
		glTexImage2D( GL_TEXTURE_2D,  0, GL_RGB, TILESIZE, TILESIZE,  0, GL_RGB, GL_UNSIGNED_BYTE, pImg);


		
	}
	else
		{return -1;}
 
	return ret;
}

