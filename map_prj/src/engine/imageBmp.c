#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "memoryPool.h"
#include "imageBmp.h"
#include "vqtfile.h"

typedef struct 
{
	f_uint8_t* data;
	f_int32_t  width;
	f_int32_t  height;
	f_int32_t  bit;
}IMAGE_BMP;

static f_int32_t bmpImageLoad(f_int8_t* filename, IMAGE_BMP* bmpTex)
{
	f_int32_t ret = 0;
	f_uint8_t bmpHead[64] = "";	
	FILE* fp = NULL;
	f_uint8_t* p = NULL;
	f_uint32_t width = 0, height = 0, dataOffset = 0, bitCount = 0, dataLen = 0;
	if(NULL == filename || bmpTex == NULL)
	{
		return -1;		
	}
	fp = fopen((const char *)filename, "rb");
	if(NULL == fp)
	{
		printf("open %s failed!please check file!\n",filename);
		return -2;
	}
	ret = fread(bmpHead, 1, 54, fp);
	if(54 != ret)
	{
		fclose(fp); fp = NULL;
		return -3;
	}
	/*校验BM标志*/
	if( (0x42 != bmpHead[0]) || (0x4D != bmpHead[1]) )
	{
		fclose(fp); fp = NULL;
		return -4;		
	}
	/*读位图数据偏移、宽度、高度、深度*/
	p = &(bmpHead[10]);
	dataOffset = *((f_uint32_t*)p);
	p = &(bmpHead[18]);
	width = *((f_uint32_t*)p);	    
	p = &(bmpHead[22]);
	height = *((f_uint32_t*)p);	    
	p = &(bmpHead[28]);
	bitCount = *((f_uint32_t*)p);	    
#ifndef _LITTLE_ENDIAN_
	dataOffset = ConvertL2B_uint(dataOffset);	
	width      = ConvertL2B_uint(width);
	height     = ConvertL2B_uint(height);
	bitCount   = ConvertL2B_uint(bitCount);
#endif
	
	/*计算数据长度*/
	dataLen = width*height*bitCount/8;	
	/*分配数据空间*/
	if(NULL == bmpTex->data)
	{
		//一次申请的内存大于动态内存申请的最大值(3.99MB)，改成传统的malloc和free方式
		bmpTex->data = (f_uint8_t *)malloc(dataLen);
		//bmpTex->data = (f_uint8_t *)NewAlterableMemory(dataLen);
	}
		
	if(NULL == bmpTex->data)
	{
		fclose(fp); fp = NULL;
		return -5;		
	}
	
	fseek(fp, dataOffset, SEEK_SET);	
	if(dataLen != fread(bmpTex->data, 1, dataLen, fp))
	{
		//一次申请的内存大于动态内存申请的最大值(3.99MB)，改成传统的malloc和free方式
		//DeleteAlterableMemory(bmpTex->data); 
		free(bmpTex->data);

		bmpTex->data = NULL;
		fclose(fp); fp = NULL;
		return -6;				
	}
	fclose(fp); fp = NULL;	
	bmpTex->width  = width;
	bmpTex->height = height;
	bmpTex->bit = bitCount;
	
	return 0;	
}

static f_int32_t bmpTextureCreate(IMAGE_BMP* bmpTex, f_int32_t *text_id)
{
	GLenum format;
	GLenum type;
	GLint  components;
	f_int32_t textId = 0;
	
	if(NULL == bmpTex)
		return -1;
	if(NULL == bmpTex->data)
		return -2;	
	
    type = GL_UNSIGNED_BYTE;
	format = GL_RGBA;
	components = GL_RGBA;

    /*根据图像位数来生成纹理，符号暂时只认24位或32位的BMP图片*/
    switch(bmpTex->bit)
    {
    	case 24:
#ifdef _LITTLE_ENDIAN_
			/*ES下不支持GL_BGR_EXT GL_BGRA_EXT，这2个bmp纹理创建不正常，会手动转换读到的数据成RGBA的存放顺序，后续待修改*/
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
    		format = GL_BGR_EXT; /*小端CPU使用，表明RGB的存放顺序*/
#endif

#else
    		format = GL_RGB; /*大端CPU使用*/
#endif
			components = GL_RGB;
			break;
    	case 32:
#ifdef _LITTLE_ENDIAN_
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	   		format = GL_BGRA_EXT; 
#endif
#else
	   		format = GL_RGBA;
#endif
			components = GL_RGBA;
			break;   
    	default:
    		return -3;        
    		break;    		
    }	
	/*创建新的纹理对象*/
	glEnableEx(GL_TEXTURE_2D);
	glGenTextures(1, (GLuint *)&(textId));

	if(0 == textId)
	    return -4;
	glBindTexture(GL_TEXTURE_2D, textId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, components, bmpTex->width, bmpTex->height, 0, format, type, bmpTex->data);
    
    /* 纹理生成后释放资源 */
	//一次申请的内存大于动态内存申请的最大值(3.99MB)，改成传统的malloc和free方式
    //DeleteAlterableMemory(bmpTex->data); 
	free(bmpTex->data);

	bmpTex->data = NULL;
	glDisableEx(GL_TEXTURE_2D);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glDisableEx(GL_TEXTURE_1D);
#endif
	
    *text_id = textId;
	return 0;
}

f_int32_t createBmpTexture(f_int8_t* filename, f_int32_t *texture_id)
{
#if 1
	IMAGE_BMP sZplTexture;
	f_int32_t ret = -1, text_id = 0;
	memset((void *)(&sZplTexture), 0, sizeof(IMAGE_BMP));
	ret = bmpImageLoad(filename, &sZplTexture);	
	if(0 != ret)
	{
	    DEBUG_PRINT("zero pitch line's texture file load error.");
	    return -1;
	}
	ret = bmpTextureCreate(&sZplTexture, &text_id);
	if(0 != ret)
	{
	    DEBUG_PRINT("zero pitch line's texture create error.");
	    return -2;
	}
	else
	    *texture_id = text_id;
	    
	return 0;
	#else


	if(FALSE == LoadParticlesTexture((f_char_t* )filename, (f_uint32_t*) texture_id))
	{
		DEBUG_PRINT("zero pitch line's texture file load error.");
		return -1;
	}


	return 0;
	


	#endif
}




//32bit的bmp文件，不需要颠倒RGB
f_uint8_t* ReadBMPEX(const f_char_t* path, f_uint16_t* bmp_width, f_uint16_t* bmp_height, f_uint32_t *bmp_size, f_uint8_t* bmp_style)
{
	FILE* pFile = NULL;
	f_uint8_t* pImageData = NULL;
	f_uint32_t lineLength = 0;
	f_uint8_t textureColors = 0;			/**< 用于将图像颜色从BGR变换到RGB */
	f_int32_t i, j;
	f_int16_t width, height;
	f_char_t type = 0;


	width = height = 0;
	pFile = fopen(path, "rb");
	if(!pFile)
	{
		printf("读取图像失败!!!_%s\n", path);
		return NULL;
	}
	fseek(pFile, 0x0012, SEEK_SET);
#ifdef _LITTLE_ENDIAN_
	fread(bmp_width, sizeof(f_int16_t), 1, pFile);
#else
	fread(&width, sizeof(f_int16_t), 1, pFile);
	*bmp_width = (f_uint16_t)ConvertL2B_short(width);
#endif

	fseek(pFile, 0x0016, SEEK_SET);
#ifdef _LITTLE_ENDIAN_
	fread(bmp_height, sizeof(f_int16_t), 1, pFile);
#else
	fread(&height, sizeof(f_int16_t), 1, pFile);
	*bmp_height = (f_uint16_t)ConvertL2B_short(height);
#endif

	//获取是否RGBA或者RGB
	fseek(pFile, 0x001c, SEEK_SET);
#ifdef _LITTLE_ENDIAN_
	fread(&type, sizeof(f_char_t), 1, pFile);
#else
	fread(&type, sizeof(f_char_t), 1, pFile);
#endif



	//计算分配的空间
	lineLength = (*bmp_width) * type /8;  // 3;	//每行数据长度 = 图像宽度 * 每像素的字节数
	while(lineLength % 4 != 0)		//修正LienWidth使其为4的倍数
	{
		++ lineLength;
	}
	(*bmp_size) = lineLength * (*bmp_height);
	//移动到图像数据处
	fseek(pFile, 0x0036, SEEK_SET);
	/** 分配内存 */
	pImageData = (f_uint8_t*)malloc(*bmp_size);
	/** 检查内存分配是否成功 */
	if(!pImageData)                        /**< 若分配内存失败则返回 */
	{
		fclose(pFile);
		return NULL;
	}
	/** 读取图像数据 */
	fread(pImageData, 1, (*bmp_size), pFile);
	/** 将图像颜色数据格式进行交换,由BGR转换为RGB */

	//RGB的bmp需要纠正顺序
	if (type == 24)
	{
		for(i = 0; i < (*bmp_height); ++i)
		{
			for(j = 0; j < (*bmp_width); ++j)
			{
				textureColors = pImageData[i * lineLength + j * 3];
				pImageData[i * lineLength + j * 3] = pImageData[i * lineLength + j * 3 + 2];
				pImageData[i * lineLength + j * 3 + 2] = textureColors;
			}
		}

		//设置读取的文件为24位RGB格式
		*bmp_style = 3;
	}
	else
	{
// 		for(i = 0; i < (*bmp_height); ++i)
// 		{
// 			for(j = 0; j < (*bmp_width); ++j)
// 			{
// 				textureColors = pImageData[i * lineLength + j * 4];
// 				pImageData[i * lineLength + j * 4] = pImageData[i * lineLength + j * 4 + 2];
// 				pImageData[i * lineLength + j * 4 + 2] = textureColors;
// 
// // 				textureColors = pImageData[i * lineLength + j * 4 + 2];
// // 				pImageData[i * lineLength + j * 4 + 2] = pImageData[i * lineLength + j * 4 + 3];
// // 				pImageData[i * lineLength + j * 4 + 3] = textureColors;
// 			}
// 		}
			
		//设置读取的文件为32位RGB格式
		*bmp_style = 4;
	}
	fclose(pFile);
	return pImageData;
}


int LoadParticlesTexture(const f_char_t* filepath, f_uint32_t* textureID)
{
	f_uint16_t width, height;
	f_uint32_t size;
	f_uint8_t style; 
	f_uint8_t* pImageData = NULL;
/*	f_char_t file_path[1024] = {0};*/
	f_uint32_t textureid = 0;

	if(!filepath)
		return FALSE;
	
	pImageData = ReadBMPEX(filepath, &width, &height, &size, &style);

	if(pImageData == NULL)
	{
		printf("%s:载入位图失败!\n", filepath);
		return FALSE;
	}
	glGenTextures(1, &textureid);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textureid);

	/** 控制滤波 */
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	/** 创建纹理 */
//	if (style == IMAGE_RGB)
//	{
//		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, 
//			GL_UNSIGNED_BYTE, pImageData);
//
//	}
//	else
//	{
//		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, 
//			GL_UNSIGNED_BYTE, pImageData);	
//	}
	if (style == 3)
	{
//		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, 
//			GL_UNSIGNED_BYTE, pImageData);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);

	}
	else
	{
//		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, 
//			GL_UNSIGNED_BYTE, pImageData);	
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
	}

	
	
	
	
	
	
	free(pImageData);
	*textureID = textureid;

	return TRUE;
}


f_int32_t createPngTexture(f_int8_t* filename, f_int32_t* texture_id)
{
	f_uint32_t width, height;
	f_uint32_t style; 
	f_uint8_t* pImageData = NULL;
	FILE* fp = NULL;
	f_int32_t text_id = 0;
	
	fp = fopen((const char *)filename, "rb");	
	
	if(fp == NULL)
		return -1;


	read_PNG_vqtfile (fp, &pImageData, &width, &height, &style);
	fclose(fp); fp = NULL;
	
	if(pImageData == NULL)
	{
		printf("pop pic read failed\n");
		return -1;
	}
	glGenTextures(1,&text_id);											
	
	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D,text_id);							
	*texture_id = text_id;
	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
	
	DeleteAlterableMemory(pImageData);
	return 0;
}
