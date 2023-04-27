/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: LayerCombineApi.c
**
** ����: ���ļ�����ͼ����Ƶ�api�ļ���
**
** ����ĺ���:  
**
**                            
** ���ע��: 
**
** ���ߣ�
**		LPF��
** 
**
** ������ʷ:
**		2017-9-21 10:42 LPF �������ļ�
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>

#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "LayerCombineApi.h"
#include "../mapApp/initData.h"
#include "TextureCache.h"
#include "tqs2rgb.h"

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D ;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC	glCompressedTexSubImage2D;
#endif
#endif
/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*/

#ifdef USE_DDS_TEXTURE

#ifndef USE_TQS2RGB
#define __glextTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels) \
				glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, \
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width*height/2, pixels)
#else
#define __glextTexSubImage2D	glextTexSubImage2D
#endif

#else
#define __glextTexSubImage2D	glextTexSubImage2D
#endif

/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
static unsigned int TexUsedInEngine[1][2] = {0,-1};	//�洢������ͼ���ID
static int TexChange= 1;					//ͼ���л���ȫ�ֱ���
static unsigned char TexBlack[1*4*1] = {174,174,164,255};		//�հ׵�������������	lpf add 2017��9��22��15:21:16
static unsigned char TexBlackDDSRGBA[1*2*1] = {0,0};		//�հ׵�������������	lpf add 2017��9��22��15:21:16

static int TexId;
/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** �ڲ���������
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/
/*.BH--------------------------------------------------------
**
** ������: setLayDisplay
**
** ����:  ���ݳ������ƾ���еĵ�ͼ��ʾģʽ�����ڲ���ͼ�����ģʽ
**
** �������:  int mode, ͼ�����ģʽ
**	          0: ��Ƭtqs/tqt
**	          1: ��Ƭ + ʸ��vqt (��ʱδʹ��)
**	          2: ����clr
**	          3: ���� + ��Ӱhil (��ʱδʹ��)
**	          4: ʸ����դ��
**
** �����������
**
** ����ֵ����
**          
** ���ע��:  ��Ӧ������SE_setModeMapDisplay->setModeMapDisplay����
**            Ŀǰֻ����ͼ��0,ͼ��1δʹ��
**
**.EH--------------------------------------------------------
*/
void setLayDisplay(sGLRENDERSCENE * pHandle)
{
	switch(pHandle->mdctrl_cmd.mapDisplay_mode)
	{
		case 0:
			TexUsedInEngine[0][0] = 0;
			TexUsedInEngine[0][1] = -1;
			return ;

		case 1:
			TexUsedInEngine[0][0] = 0;
			TexUsedInEngine[0][1] = 1;
			return ;


		case 2:
			TexUsedInEngine[0][0] = 2;
			TexUsedInEngine[0][1] = -1;
			return ;


		case 3:
			TexUsedInEngine[0][0] = 2;
			TexUsedInEngine[0][1] = 3;
			return ;

		case 4:
			TexUsedInEngine[0][0] = 0;
			TexUsedInEngine[0][1] = 1;
			return ;


		default:
			TexUsedInEngine[0][0] = 0;
			TexUsedInEngine[0][1] = -1;
			return ;
			

	}
}

/*.BH--------------------------------------------------------
**
** ������: bindAndSubRenderTexID
**
** ����:  ���������ͼ�����ģʽ,�жϵ�ǰ֡ʹ�õĵ�ͼ�����Ƿ�����һ֡ʹ�õĲ�һ��,�����ͬ����а������л�����Ĳ���
**
** �������:  sQTMAPNODE * pNode  ��Ƭ�ڵ��ָ��
              int tex_order	      �ڲ�ʹ�õ�ͼ�����ģʽ�����±�(Ŀǰֻʹ��0)
**
** �����������
**
** ����ֵ��TRUE(1)  �л�����ɹ��������л�����
**         FALSE(0) ��ͼ�������ʹ���,������idΪ0��Ĭ������
**          
**
** ���ע��:  �����ʹ��,֧������ͼ�������ID�󶨼��л�,Ŀǰֻʹ��һ��,tex_order����ʼ��Ϊ0
**
**.EH--------------------------------------------------------
*/
f_uint32_t bindAndSubRenderTexID(sQTMAPNODE * pNode, int tex_order)
{
	int texid = 0;	//����ID
	
	/* ���ݵ�ǰ����ʹ�õĵ�ͼ����,ѡ�����滻����ķ�ʽ�滻��ǰ����id�е����� */
	switch(TexUsedInEngine[0][tex_order])
	{
		/* 0��ʾʹ��Ӱ������ */
		case 0:
			/* �����Ƭ�ڵ������idΪ0,��������������id */
			if (pNode->texid == 0)
			{
				//printf("��������0\n");
				glextGenTextures(1, &pNode->texid);
			}

			/* ����Ƭ�ڵ������id */
			texid = pNode->texid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			/* �����Ƭ�ڵ���һ֡ʹ�õĵ�ͼ������Ӱ������,���滻ΪӰ������ */
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->imgdata != NULL)
				{
					//printf("�л����� 2 -> 0\n");

					/* 0,0,0��ʾlevel,xoffset,yoffset,level��ʾʹ��һ�ֱַ���,xoffset,yoffset��ʾ�������ʼλ�� */
					/* TILESIZE,TILESIZE��ʾwidth,height,��ʾ�滻������Ŀ�Ⱥ͸߶� */
					__glextTexSubImage2D(GL_TEXTURE_2D,0, 
						0,0, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pNode->imgdata);
				}
			}	
			
			break;
		case 1:
			if (pNode->vtexid == 0)
			{
//				printf("��������1\n");
//				glextGenTextures(1, &pNode->vtexid);
				{


				#if 0
					//glEnableEx(GL_TEXTURE_2D);
					//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
					glGenTextures(1, &pNode->vtexid);
					glBindTexture(GL_TEXTURE_2D, pNode->vtexid);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  4);
					
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					
					//gluextBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pImg);
					if (pNode->vqtdata != NULL)
					{
						glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, TILESIZE, TILESIZE,  0, GL_RGBA, GL_UNSIGNED_BYTE, pNode->vqtdata);					
					}
					else 
					{
						glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, 1, 1,  0, GL_RGBA, GL_UNSIGNED_BYTE, TexBlack);					

					}
				//#else
						glextGenTextures(1, (GLuint *)&pNode->vtexid);
						glBindTexture(GL_TEXTURE_2D, pNode->vtexid);

					    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

						if (pNode->vqtdata != NULL)
						{
						}
						else 
						{
							printf("��������1\n");

							glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, 1, 1,  0, GL_RGBA, GL_UNSIGNED_BYTE, TexBlack);					


						}

					
				#endif

					texid = 0;

					

				}
			}
			else
			{
				texid = pNode->vtexid;
			}
			glBindTexture(GL_TEXTURE_2D, texid);				
			#if 0
			//��Ҫ�滻����ʱ�����Ӵ���
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if (pNode->vqtdata != NULL)
				{
					//printf("�л����� 3 -> 1\n");
					glTexSubImage2D(GL_TEXTURE_2D,0, 
						0,0, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pNode->vqtdata);

				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_2D,0, 
						0,0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, TexBlack);
				}
			}	
			#endif
			
			break;
		
		/* 2��ʾʹ�õ�ò�������� */
		case 2:
			/* �����Ƭ�ڵ������idΪ0,��������������id */
			if (pNode->texid == 0)
			{
				//printf("��������2\n");
				glextGenTextures(1, &pNode->texid);
			}
			/* ����Ƭ�ڵ������id */
			texid = pNode->texid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			/* �����Ƭ�ڵ���һ֡ʹ�õĵ�ͼ�����ǵ�ò��������,���滻Ϊ��ò�������� */
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->clrdata != NULL)
				{
					//printf("�л����� 0 -> 2\n");

					/* 0,0,0��ʾlevel,xoffset,yoffset,level��ʾʹ��һ�ֱַ���,xoffset,yoffset��ʾ�������ʼλ��*/
					/* TILESIZE,TILESIZE��ʾwidth,height,��ʾ�滻������Ŀ�Ⱥ͸߶�*/
					__glextTexSubImage2D(GL_TEXTURE_2D,0, 
					0,0, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pNode->clrdata);				
				}
			}

			/* �����Ƭ�ڵ��޵�ò��������,������һ��Ĭ����ɫ(174,174,164,255)��1*1��ά���� */
			if(pNode->clrdata == NULL)
			{
				if(TexId == 0)
				{
					glGenTextures(1, &TexId);

					glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
					glBindTexture(GL_TEXTURE_2D, TexId);

					/** �����˲� */
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

					/** �������� */						
					glTexImage2D( GL_TEXTURE_2D,  0, 
					GL_RGBA, 1, 1,  0, GL_RGBA, GL_UNSIGNED_BYTE, TexBlack);		

				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, TexId);			
				}
			}
			break;

		case 3:
			if (pNode->vtexid == 0)
			{
				//printf("��������3\n");
//				glextGenTextures(1, &pNode->vtexid);
				{
					//glEnableEx(GL_TEXTURE_2D);
					//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
					glGenTextures(1, &pNode->vtexid);
					glBindTexture(GL_TEXTURE_2D, pNode->vtexid);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  4);
					
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					
					//gluextBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pImg);
					//glextTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, TILESIZE, TILESIZE,  0, GL_RGBA, GL_UNSIGNED_BYTE, pNode->hildata);
					if (pNode->hildata != NULL)
					{
						glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, TILESIZE, TILESIZE,  0, GL_RGBA, GL_UNSIGNED_BYTE, pNode->hildata);					
					}
					else 
					{
						glTexImage2D( GL_TEXTURE_2D,  0, GL_RGBA, 1, 1,  0, GL_RGBA, GL_UNSIGNED_BYTE, TexBlack);					
						
					}
				}
			}
			texid = pNode->vtexid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			//��Ҫ�滻����ʱ�����Ӵ���
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->hildata != NULL)
				{
					//printf("�л����� 1 -> 3\n");
					glTexSubImage2D(GL_TEXTURE_2D,0, 
						0,0, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pNode->hildata);
				}
				else
				{
					glTexSubImage2D(GL_TEXTURE_2D,0, 
						0,0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, TexBlack);
					}

			}		

			break;

		/* 4��ʾʹ����Ӱhil���� */
		case 4:
			/* �����Ƭ�ڵ������idΪ0,��������������id */
			if (pNode->texid == 0)
			{
				//printf("��������2\n");
				glextGenTextures(1, &pNode->texid);
			}
			/* ����Ƭ�ڵ������id */
			texid = pNode->texid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			/* �����Ƭ�ڵ���һ֡ʹ�õĵ�ͼ��������Ӱhil����,���滻Ϊ��Ӱhil���� */
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->hildata != NULL)
				{
					//printf("�л����� 0 -> 2\n");

					__glextTexSubImage2D(GL_TEXTURE_2D,0, 
					0,0, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pNode->hildata);				
				}

			}			
			break;
		
		/* Ĭ�ϰ�����id 0,��ͬgpu������idΪ0������Ч����һ�� */
		default:
			glBindTexture(GL_TEXTURE_2D, 0);	
			return FALSE;		
	}

	/* ��¼��Ƭ�ڵ���һ֡ʹ�õĵ�ͼ���� */
	pNode->texUsed[tex_order] = TexUsedInEngine[0][tex_order];

	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: SetLayUser
**
** ����:  
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�3, �л�ͼ����ӷ�ʽ
**
**.EH--------------------------------------------------------
*
void SetLayUser()
{
	if(TexChange > 3)
		TexChange = 0;

	//printf("tex = %d\n", TexChange);
	setLayDisplay(TexChange);

	TexChange ++;
}
*/
