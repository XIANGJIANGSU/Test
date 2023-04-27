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
	/*У��BM��־*/
	if( (0x42 != bmpHead[0]) || (0x4D != bmpHead[1]) )
	{
		fclose(fp); fp = NULL;
		return -4;		
	}
	/*��λͼ����ƫ�ơ���ȡ��߶ȡ����*/
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
	
	/*�������ݳ���*/
	dataLen = width*height*bitCount/8;	
	/*�������ݿռ�*/
	if(NULL == bmpTex->data)
	{
		//һ��������ڴ���ڶ�̬�ڴ���������ֵ(3.99MB)���ĳɴ�ͳ��malloc��free��ʽ
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
		//һ��������ڴ���ڶ�̬�ڴ���������ֵ(3.99MB)���ĳɴ�ͳ��malloc��free��ʽ
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

    /*����ͼ��λ������������������ʱֻ��24λ��32λ��BMPͼƬ*/
    switch(bmpTex->bit)
    {
    	case 24:
#ifdef _LITTLE_ENDIAN_
			/*ES�²�֧��GL_BGR_EXT GL_BGRA_EXT����2��bmp�����������������ֶ�ת�����������ݳ�RGBA�Ĵ��˳�򣬺������޸�*/
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
    		format = GL_BGR_EXT; /*С��CPUʹ�ã�����RGB�Ĵ��˳��*/
#endif

#else
    		format = GL_RGB; /*���CPUʹ��*/
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
	/*�����µ��������*/
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
    
    /* �������ɺ��ͷ���Դ */
	//һ��������ڴ���ڶ�̬�ڴ���������ֵ(3.99MB)���ĳɴ�ͳ��malloc��free��ʽ
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




//32bit��bmp�ļ�������Ҫ�ߵ�RGB
f_uint8_t* ReadBMPEX(const f_char_t* path, f_uint16_t* bmp_width, f_uint16_t* bmp_height, f_uint32_t *bmp_size, f_uint8_t* bmp_style)
{
	FILE* pFile = NULL;
	f_uint8_t* pImageData = NULL;
	f_uint32_t lineLength = 0;
	f_uint8_t textureColors = 0;			/**< ���ڽ�ͼ����ɫ��BGR�任��RGB */
	f_int32_t i, j;
	f_int16_t width, height;
	f_char_t type = 0;


	width = height = 0;
	pFile = fopen(path, "rb");
	if(!pFile)
	{
		printf("��ȡͼ��ʧ��!!!_%s\n", path);
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

	//��ȡ�Ƿ�RGBA����RGB
	fseek(pFile, 0x001c, SEEK_SET);
#ifdef _LITTLE_ENDIAN_
	fread(&type, sizeof(f_char_t), 1, pFile);
#else
	fread(&type, sizeof(f_char_t), 1, pFile);
#endif



	//�������Ŀռ�
	lineLength = (*bmp_width) * type /8;  // 3;	//ÿ�����ݳ��� = ͼ���� * ÿ���ص��ֽ���
	while(lineLength % 4 != 0)		//����LienWidthʹ��Ϊ4�ı���
	{
		++ lineLength;
	}
	(*bmp_size) = lineLength * (*bmp_height);
	//�ƶ���ͼ�����ݴ�
	fseek(pFile, 0x0036, SEEK_SET);
	/** �����ڴ� */
	pImageData = (f_uint8_t*)malloc(*bmp_size);
	/** ����ڴ�����Ƿ�ɹ� */
	if(!pImageData)                        /**< �������ڴ�ʧ���򷵻� */
	{
		fclose(pFile);
		return NULL;
	}
	/** ��ȡͼ������ */
	fread(pImageData, 1, (*bmp_size), pFile);
	/** ��ͼ����ɫ���ݸ�ʽ���н���,��BGRת��ΪRGB */

	//RGB��bmp��Ҫ����˳��
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

		//���ö�ȡ���ļ�Ϊ24λRGB��ʽ
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
			
		//���ö�ȡ���ļ�Ϊ32λRGB��ʽ
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
		printf("%s:����λͼʧ��!\n", filepath);
		return FALSE;
	}
	glGenTextures(1, &textureid);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textureid);

	/** �����˲� */
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	/** �������� */
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
