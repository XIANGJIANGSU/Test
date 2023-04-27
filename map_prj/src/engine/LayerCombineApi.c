/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: LayerCombineApi.c
**
** 描述: 本文件包含图层控制的api文件。
**
** 定义的函数:  
**
**                            
** 设计注记: 
**
** 作者：
**		LPF。
** 
**
** 更改历史:
**		2017-9-21 10:42 LPF 创建此文件
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** 头文件引用
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
** 类型声明
**-----------------------------------------------------------
*/
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D ;
extern PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC	glCompressedTexSubImage2D;
#endif
#endif
/*-----------------------------------------------------------
** 文字量和宏声明
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
** 全局变量定义
**-----------------------------------------------------------
*/
static unsigned int TexUsedInEngine[1][2] = {0,-1};	//存储的两个图层的ID
static int TexChange= 1;					//图层切换的全局变量
static unsigned char TexBlack[1*4*1] = {174,174,164,255};		//空白的纹理数据数组	lpf add 2017年9月22日15:21:16
static unsigned char TexBlackDDSRGBA[1*2*1] = {0,0};		//空白的纹理数据数组	lpf add 2017年9月22日15:21:16

static int TexId;
/*-----------------------------------------------------------
** 外部变量声明
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 内部函数声明
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 函数定义
**-----------------------------------------------------------
*/
/*.BH--------------------------------------------------------
**
** 函数名: setLayDisplay
**
** 描述:  根据场景绘制句柄中的底图显示模式设置内部的图层叠加模式
**
** 输入参数:  int mode, 图层叠加模式
**	          0: 卫片tqs/tqt
**	          1: 卫片 + 矢量vqt (暂时未使用)
**	          2: 晕渲clr
**	          3: 晕渲 + 阴影hil (暂时未使用)
**	          4: 矢量化栅格
**
** 输出参数：无
**
** 返回值：无
**          
** 设计注记:  对应引擎层的SE_setModeMapDisplay->setModeMapDisplay函数
**            目前只用了图层0,图层1未使用
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
** 函数名: bindAndSubRenderTexID
**
** 描述:  根据输入的图层叠加模式,判断当前帧使用的底图纹理是否与上一帧使用的不一致,如果不同则进行绑定纹理及切换纹理的操作
**
** 输入参数:  sQTMAPNODE * pNode  瓦片节点的指针
              int tex_order	      内部使用的图层叠加模式数组下标(目前只使用0)
**
** 输出参数：无
**
** 返回值：TRUE(1)  切换纹理成功或无需切换纹理
**         FALSE(0) 底图纹理类型错误,绑定纹理id为0的默认纹理
**          
**
** 设计注记:  引擎层使用,支持两个图层的纹理ID绑定及切换,目前只使用一个,tex_order参数始终为0
**
**.EH--------------------------------------------------------
*/
f_uint32_t bindAndSubRenderTexID(sQTMAPNODE * pNode, int tex_order)
{
	int texid = 0;	//纹理ID
	
	/* 根据当前引擎使用的底图纹理,选择用替换纹理的方式替换当前纹理id中的纹理 */
	switch(TexUsedInEngine[0][tex_order])
	{
		/* 0表示使用影像纹理 */
		case 0:
			/* 如果瓦片节点的纹理id为0,则重新生成纹理id */
			if (pNode->texid == 0)
			{
				//printf("生成纹理0\n");
				glextGenTextures(1, &pNode->texid);
			}

			/* 绑定瓦片节点的纹理id */
			texid = pNode->texid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			/* 如果瓦片节点上一帧使用的底图纹理不是影像纹理,则替换为影像纹理 */
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->imgdata != NULL)
				{
					//printf("切换纹理 2 -> 0\n");

					/* 0,0,0表示level,xoffset,yoffset,level表示使用一种分辨率,xoffset,yoffset表示纹理的起始位置 */
					/* TILESIZE,TILESIZE表示width,height,表示替换子纹理的宽度和高度 */
					__glextTexSubImage2D(GL_TEXTURE_2D,0, 
						0,0, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pNode->imgdata);
				}
			}	
			
			break;
		case 1:
			if (pNode->vtexid == 0)
			{
//				printf("生成纹理1\n");
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
							printf("生成纹理1\n");

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
			//需要替换纹理时，增加处理
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if (pNode->vqtdata != NULL)
				{
					//printf("切换纹理 3 -> 1\n");
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
		
		/* 2表示使用地貌晕渲纹理 */
		case 2:
			/* 如果瓦片节点的纹理id为0,则重新生成纹理id */
			if (pNode->texid == 0)
			{
				//printf("生成纹理2\n");
				glextGenTextures(1, &pNode->texid);
			}
			/* 绑定瓦片节点的纹理id */
			texid = pNode->texid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			/* 如果瓦片节点上一帧使用的底图纹理不是地貌晕渲纹理,则替换为地貌晕渲纹理 */
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->clrdata != NULL)
				{
					//printf("切换纹理 0 -> 2\n");

					/* 0,0,0表示level,xoffset,yoffset,level表示使用一种分辨率,xoffset,yoffset表示纹理的起始位置*/
					/* TILESIZE,TILESIZE表示width,height,表示替换子纹理的宽度和高度*/
					__glextTexSubImage2D(GL_TEXTURE_2D,0, 
					0,0, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, pNode->clrdata);				
				}
			}

			/* 如果瓦片节点无地貌晕渲数据,则生成一个默认颜色(174,174,164,255)的1*1二维纹理 */
			if(pNode->clrdata == NULL)
			{
				if(TexId == 0)
				{
					glGenTextures(1, &TexId);

					glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
					glBindTexture(GL_TEXTURE_2D, TexId);

					/** 控制滤波 */
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

					/** 创建纹理 */						
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
				//printf("生成纹理3\n");
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
			
			//需要替换纹理时，增加处理
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->hildata != NULL)
				{
					//printf("切换纹理 1 -> 3\n");
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

		/* 4表示使用阴影hil纹理 */
		case 4:
			/* 如果瓦片节点的纹理id为0,则重新生成纹理id */
			if (pNode->texid == 0)
			{
				//printf("生成纹理2\n");
				glextGenTextures(1, &pNode->texid);
			}
			/* 绑定瓦片节点的纹理id */
			texid = pNode->texid;
			glBindTexture(GL_TEXTURE_2D, texid);	
			
			/* 如果瓦片节点上一帧使用的底图纹理不是阴影hil纹理,则替换为阴影hil纹理 */
			if(TexUsedInEngine[0][tex_order] != pNode->texUsed[tex_order])
			{	
				if(pNode->hildata != NULL)
				{
					//printf("切换纹理 0 -> 2\n");

					__glextTexSubImage2D(GL_TEXTURE_2D,0, 
					0,0, TILESIZE, TILESIZE, GL_RGBA, GL_UNSIGNED_BYTE, pNode->hildata);				
				}

			}			
			break;
		
		/* 默认绑定纹理id 0,不同gpu的纹理id为0的纹理效果不一样 */
		default:
			glBindTexture(GL_TEXTURE_2D, 0);	
			return FALSE;		
	}

	/* 记录瓦片节点上一帧使用的底图纹理 */
	pNode->texUsed[tex_order] = TexUsedInEngine[0][tex_order];

	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: SetLayUser
**
** 描述:  
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口3, 切换图层叠加方式
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
