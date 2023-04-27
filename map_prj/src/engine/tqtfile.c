#include <stdio.h>
#include <stdlib.h>


#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "tqtfile.h"
#include "filepool.h"
#include "../../../jpgLib_prj/src/jpeglib.h"
#include "vqtfile.h"
#include "memoryPool.h"
/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */
#include <setjmp.h>
 
#define MAXTQTLAYER	16
#define TQTFILEKEY	(0x74717400)
#define VERIFYTQTFILE(tqt) ( (tqt != NULL) && (tqt->skey == TQTFILEKEY) && (tqt->pFileItem != NULL) ) 

typedef struct tagPicInfo{
	f_uint64_t offset;                /* 瓦片在文件中的偏移地址 */
	f_uint64_t length;                /* 瓦片数据的长度(字节数) */
}sPicInfo;

typedef struct tagTQTFile
{
	f_uint64_t skey; /*must be TQTFILEKEY*/
	FILEITEM * pFileItem;
	f_uint64_t ver;
	f_uint64_t rootlevel;              /* 树的起始层级（绝对层级） */
	f_uint64_t rootxidx;
	f_uint64_t rootyidx;
	f_uint64_t level;                  /* 树的级数（深度） */
	f_uint64_t tilesize;               /* 瓦片尺寸（像素数） */
	f_uint64_t tilestart[MAXTQTLAYER]; /* 每一级瓦片的起始号 */
	sPicInfo   picinfo[1];           /* 瓦片数据的信息 */
}sTQTFile;


static int tqtRGBAChannel = 3;	//RGB or RGBA

//设置png图片的alpha通道值
void setTqtAlpha(f_uint8_t * pData, f_int32_t width, f_int32_t height, char alpha)
{
	int i = 0;
	int sum = width * height;

	for(i=1; i<= sum; i++)
	{
		pData[4*i -1] = alpha;	
//		if(pData[4*i -1] != 255)
//			printf("%d, i = %d",pData[4*i -1],i );
		
	}
//	printf("\n");
}


/*
	打开一个TQT文件，
	参数：cstrFileName -- 文件名称
	返回值：
	     文件句柄
	     
说明：TQT文件格式
    数据类型(填“tqt”)                    无符号字符串(4字节)
    版本信息(ver)                        无符号整型(4字节)
    四叉树层数(layers)                   无符号整型(4字节)
    瓦片尺寸(tile_size)                  无符号整型(4字节)
    瓦片0在文件中的偏移地址(pic_offset)  无符号整型(4字节)
    瓦片0的长度(字节数,若无数据则填FFFF) 无符号整型(4字节)
    瓦片1在文件中的偏移地址(pic_offset)  无符号整型(4字节)
    瓦片1的长度(字节数)                  无符号整型(4字节)
    ..........
    ..........
    瓦片n在文件中的偏移地址(pic_offset)  无符号整型(4字节)
    瓦片n的长度(字节数)                  无符号整型(4字节)
    瓦片0的数据
    瓦片1的数据
    ...........
    瓦片n的数据
*/
VOIDPtr tqtfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName)
{
	FILE * pFile;
	sTQTFile * pTqt;
	f_uint8_t filehead[16], buf[8];
	f_uint32_t ver = 0, level = 0, tilesize = 0, piccount = 0, i = 0;

	pFile = fopen(cstrFileName, "rb");
	if (pFile == NULL)
	{
		DEBUG_PRINT("File open failed 0.");
		printf("%s\n",cstrFileName);
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

	pTqt = (sTQTFile *)NewAlterableMemory(sizeof(sTQTFile) + piccount * sizeof(sPicInfo));
	if (pTqt == NULL)
	{
		DEBUG_PRINT("malloc failed.");
		fclose(pFile);
		return NULL;
	}
	memset(pTqt, 0, sizeof(sTQTFile));
	pTqt->skey = TQTFILEKEY;
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
			DEBUG_PRINT("tqt offset read error.");
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
	关闭一个已经打开的TQT文件，释放文件占用的资源
	参数：tqtfile -- 用tqtfileOpen返回的TQT文件句柄
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
*/
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
	DeleteAlterableMemory(pTqt);
	pTqt = NULL;
	return 0;
}

/*
	得到一个已经打开的TQT文件中树的层数
	参数：tqtfile -- 用tqtfileOpen返回的TQT文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
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
	得到一个已经打开的TQT文件中树的起始层号
	参数：tqtfile -- 用tqtfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 树的起始层号
		-1 -- 文件句柄无效
*/
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
	得到一个已经打开的TQT文件中基本图块的大小
	参数：tqtfile -- 用tqtfileOpen返回的TQT文件句柄
	返回值：
	    >0 -- 基本图块的大小
		-1 -- 文件句柄无效
*/
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
功能：判断节点是否在文件的范围内
输入：    
    level -- 指定的层，从0开始
	xidx, yidx -- 基本图块在层中的编号
	param  文件句柄所在的地址
输出：
    0  在
    -1 不在
*/
f_int32_t imgTqtIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param)
{	
	f_int32_t llevel, lxidx, lyidx;
	sTQTFile *pTqt = (sTQTFile *)param;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
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

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/* add by zqx */
static void put_scanline_someplace(unsigned char *src, int size, unsigned long destAddr)
{
    int i;
	unsigned char *dest = (unsigned char *)destAddr;
	for(i = 0; i < size; i++)
	{
	    *dest++ = *src++;
	}
}

int read_JPEG_tqtfile (FILE * pFile, unsigned char **ppImg, int *width, int *height)
{
  unsigned long destAddr;
  unsigned long bufSize;
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if (pFile == NULL) {
    printf("tqt file handle is null.\n");
    return -1;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    return -1;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, pFile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  bufSize = cinfo.output_height * cinfo.output_width * cinfo.output_components;
  *ppImg = (unsigned char *)NewAlterableMemory(bufSize);
	if (*ppImg == NULL)
		return 0;
  memset(*ppImg, 0, bufSize);
  destAddr = (unsigned long)(*ppImg);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
    put_scanline_someplace(buffer[0], row_stride * 1, destAddr);  
	destAddr += row_stride * 1;
  }
  
  *width = cinfo.output_width;
  *height = cinfo.output_height;

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return 0;
}

//int jpegDecode(FILE * pFile, unsigned char ** ppBuf, int *picWidth, int *picHeight);

/*
	得到一个TQT文件中的基本图块数据
	参数：tqtfile -- 用tqtfileOpen返回的TQT文件句柄
	      layer -- 指定的层，从0开始
		  xidx, yidx -- 基本图块在层中的编号
		  pData -- 返回的数据，如果成功返回，该数据不用时必须用tqtfileFreeTileData释放
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
		-2 -- 层号无效
		-3 -- xidx或yidx无效
		-4 -- 数据解码错误
*/
static f_int32_t tqtfileGetData(sTQTFile *pTqt, f_int32_t level, f_int32_t xidx, f_int32_t yidx, void ** pData)
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

    ret = read_JPEG_tqtfile(pFile, (f_uint8_t **)pData, &width, &height);

//    ret = read_PNG_vqtfile(pFile, (f_uint8_t **)pData, &width, &height, &tqtRGBAChannel);

	filepoolReleaseFile(pTqt->pFileItem);
	if (ret < 0)
	{
		*pData = NULL;
		return -4;
	}
	return 0;
}

f_int32_t loadtqt(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg)
{
	f_int32_t llevel, lxidx, lyidx;
	sTQTFile *pTqt = NULL;

	//判断是否为空
	if(param == 0)
	{
		DEBUG_PRINT("pTqt is NULL.");
		printf("%d,%d,%d\n",level, xidx, yidx);
		return -1;
	}

	pTqt = (sTQTFile *)param;
	
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}
	/*全局的level, xidx, yidx转成局部的level, xidx, yidx*/
	if ((f_uint64_t)level < pTqt->rootlevel)
		return -1;
	llevel = level - pTqt->rootlevel;
	lxidx = xidx - pTqt->rootxidx * (1 << llevel);
	lyidx = yidx - pTqt->rootyidx * (1 << llevel);
	if (tqtfileGetData(pTqt, llevel, lxidx, lyidx, (void **)ppImg) != 0)
	{
		if (ppImg == 0)
			return -1;
		*ppImg = (f_uint8_t *)NewAlterableMemory(pTqt->tilesize * pTqt->tilesize * tqtRGBAChannel);	//rgba 文件改为 * 4,lpf add 2017-6-21 10:26:33
//		memset(*ppImg, 0xFF, pTqt->tilesize * pTqt->tilesize * 4);
		memset(*ppImg, 0xFF, pTqt->tilesize * pTqt->tilesize * tqtRGBAChannel);
		
	}
	return 0;
}

f_int32_t tqt2tex(f_uint64_t param , f_uint8_t *pImg)
{
	f_int32_t ret = -1;
	sTQTFile *pTqt = (sTQTFile *)param;
	if(!VERIFYTQTFILE(pTqt))
	{
		DEBUG_PRINT("pTqt error.");
	    return -1;
	}

	if(tqtRGBAChannel == 3)
	{
		//glEnableEx(GL_TEXTURE_2D);
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glextGenTextures(1, (GLuint *)&ret);
		glBindTexture(GL_TEXTURE_2D, ret);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  4);

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//gluextBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pImg);
		glextTexImage2D( GL_TEXTURE_2D,  0, GL_RGB, TILESIZE, TILESIZE,  0, GL_RGB, GL_UNSIGNED_BYTE, pImg);
	 
		return ret;
	}
	else if(tqtRGBAChannel == 4)
	{

		//glEnableEx(GL_TEXTURE_2D);
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glextGenTextures(1, (GLuint *)&ret);
		glBindTexture(GL_TEXTURE_2D, ret);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  4);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		//gluextBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pImg);
		glextTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, TILESIZE, TILESIZE,  0, GL_RGBA, GL_UNSIGNED_BYTE, pImg);
		
		
		
		return ret;
	}
	else
		return -1;
	
}


