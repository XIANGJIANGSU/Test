
#include "../define/macrodefine.h"
#include "terWarning.h"
#include "../mapApp/mapApp.h"

#define TERRAINWARNING_COLOR_SAMPLING 256		// 水平地形告警采样数
#define TERRAINWARNING_COLOR_ALPHA	  200       // 水平地形告警色带的透明度

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32	
extern PFNGLACTIVETEXTUREPROC glActiveTexture;	
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;	
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f;
#endif                                     /* 影像数据生成纹理函数 */
#endif

/* 用于生成水平地形告警色带的RGBA数组 */
f_uint8_t chRGBA[TERRAINWARNING_COLOR_SAMPLING * 4];

/*.BH--------------------------------------------------------
**
** 函数名: SetTerrainWarningStripeColor
**
** 描述:  设置水平地形告警颜色带
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口1
**
**.EH--------------------------------------------------------
*/
void setTerrainWarningStripeColor(VOIDPtr scene, sTerWarningSet terwarning_set)
{
	sGLRENDERSCENE * pScene = NULL;	
		
	// 设置水平地形告警色带
	f_float64_t texpixel_permeter = 0.0;
	f_int32_t i, j, layers;
	f_float32_t  height_region = 1.0f;

	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return;


	memset(chRGBA, 0, TERRAINWARNING_COLOR_SAMPLING * 4);		// 默认都是黑色
	layers = terwarning_set.layers;
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 0] = terwarning_set.ter_warning_data[layers - 1].red;
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 1] = terwarning_set.ter_warning_data[layers - 1].green;
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 2] = terwarning_set.ter_warning_data[layers - 1].blue;	
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 3] = TERRAINWARNING_COLOR_ALPHA;
	
	chRGBA[0 * 4 + 0] = terwarning_set.ter_warning_data[0].red;
	chRGBA[0 * 4 + 1] = terwarning_set.ter_warning_data[0].green;
	chRGBA[0 * 4 + 2] = terwarning_set.ter_warning_data[0].blue;	
	chRGBA[0 * 4 + 3] = 0;			
	
	// 每个方格的增量, 从0.0->1.0的间隔
	// 计算每米所对应的纹理坐标间隔为
	// 纹理总长度为256，用去两边的，只剩下254，即每个单元格所对应的纹理范围是 254个格子来显示height_region米的区域
	height_region = terwarning_set.height_max - terwarning_set.height_min;
	pScene->s_nTexCoordPerMeter = (f_float32_t)(254.0f / 256.0f / height_region);		// 每米所需要的纹理坐标间隔
	texpixel_permeter = height_region / 254.0f;				// 像素所对应的m
	// 填充黄色和绿色
	for (i = 1; i < 255; i++)
	{
		for(j = 1; j < layers - 1; j++)
		{
			if ((i - 1) * texpixel_permeter + terwarning_set.height_min < terwarning_set.ter_warning_data[j].height_max)
			{
			    chRGBA[i * 4 + 0] = terwarning_set.ter_warning_data[j].red;
			    chRGBA[i * 4 + 1] = terwarning_set.ter_warning_data[j].green;
			    chRGBA[i * 4 + 2] = terwarning_set.ter_warning_data[j].blue; 
			    chRGBA[i * 4 + 3] = TERRAINWARNING_COLOR_ALPHA;	
				//printf("i = %d, r = %d, g = %d, b = %d\n", i, chRGBA[i * 4 + 0], chRGBA[i * 4 + 1], chRGBA[i * 4 + 2]);
				break;
			}
		}
	}

}


/*.BH--------------------------------------------------------
**
** 函数名: GenTerrainWarningTexture
**
** 描述:  生成水平地形告警颜色带纹理
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口1
**
**.EH--------------------------------------------------------
*/
void GenTerrainWarningTexture(VOIDPtr scene)
{
	sGLRENDERSCENE * pScene = NULL;	

	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return;

	if(0 != (pScene->warn_texid))
	{
	    glDeleteTextures(1, (const GLuint *)&pScene->warn_texid);
		pScene->warn_texid = 0;
	}

#ifdef TER_WARNING_USE_TEXTURE_1D	
	glDisableEx(GL_TEXTURE_1D);
	glGenTextures(1, (GLuint *)&pScene->warn_texid);
	glBindTexture(GL_TEXTURE_1D, pScene->warn_texid);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, TERRAINWARNING_COLOR_SAMPLING, 0, GL_RGBA, GL_UNSIGNED_BYTE, chRGBA);	
	glBindTexture(GL_TEXTURE_1D,0);
#else
	glDisableEx(GL_TEXTURE_2D);
	glGenTextures(1, (GLuint *)&pScene->warn_texid);
	glBindTexture(GL_TEXTURE_2D, pScene->warn_texid);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TERRAINWARNING_COLOR_SAMPLING, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, chRGBA);	
	glBindTexture(GL_TEXTURE_2D,0);
#endif
	/*水平地形告警颜色带纹理生成成功*/
    pScene->is_warn_data_set_success = sSuccess;
	/*水平地形告警颜色带纹理无需重新生成*/
    pScene->is_need_regenerate_warn_texid = 0;
}

/*.BH--------------------------------------------------------
**
** 函数名: RenderTerrainWarningStripeColor
**
** 描述:  绘制水平地形告警颜色带
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：成功，返回TRUE;
**				 失败，返回FALSE;
**          
**
** 设计注记:  外部接口2
**
**.EH--------------------------------------------------------
*/
BOOL RenderTerrainWarningStripeColor(sGLRENDERSCENE *pHandle)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	if(NULL == pHandle)
	    return FALSE;
	
	//将飞机当前高度设为告警高度
	pHandle->terwaring_height = pHandle->geopt_pos.height;


	//开启绘制色带
	//glActiveTextureARB(GL_TEXTURE1_ARB);
	glActiveTexture(GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 

//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
#ifdef TER_WARNING_USE_TEXTURE_1D	
	glEnableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, pHandle->warn_texid);	// 绑定水平地形告警纹理
#else
	glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pHandle->warn_texid);	// 绑定水平地形告警纹理
#endif
	glMatrixMode(GL_TEXTURE);							// 转为纹理矩阵操作
	glLoadIdentity();

#if 1

	
	//glTranslatef(0.5f, 0.0f, 0.0f);
	glTranslatef((255.0/256.0 - pHandle->m_terwarning_set_input.height_max * pHandle->s_nTexCoordPerMeter), 0.0f, 0.0f);
#ifdef TER_WARNING_USE_TEXTURE_1D		
	glScalef(pHandle->s_nTexCoordPerMeter, 1.0f, 1.0f);
#else
	glScalef(pHandle->s_nTexCoordPerMeter, 1.0f, 1.0f);
#endif
	glTranslatef(-(pHandle->terwaring_height), 0.0f, 0.0f);

#else

	glScalef(0.0002f,0.0002f,0.0002f);	


#endif


	glMatrixMode(GL_MODELVIEW);	
#else
	ESMatrix mat;
	int prog = 0;
	int loc = 0;

	if(NULL == pHandle)
		return FALSE;

	//将飞机当前高度设为告警高度
	pHandle->terwaring_height = pHandle->geopt_pos.height;

	//开启绘制色带
	glActiveTextureARB(GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

#ifdef TER_WARNING_USE_TEXTURE_1D	
	glEnableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, pHandle->warn_texid);	// 绑定地形告警纹理
#else
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pHandle->warn_texid);	// 绑定地形告警纹理
#endif

	esMatrixLoadIdentity(&mat);
	//esTranslate(&mat, 0.5f, 0.0f, 0.0f);
	esTranslate(&mat, (255.0/256.0 - pHandle->m_terwarning_set_input.height_max * pHandle->s_nTexCoordPerMeter), 0.0f, 0.0f);
	esScale(&mat, pHandle->s_nTexCoordPerMeter, 1.0, 1.0);
	esTranslate(&mat, -(pHandle->terwaring_height), 0.0f, 0.0f);
	

	glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
	loc = glGetUniformLocation(prog, "u_tMat1");

	glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mat);

#endif
	
	return TRUE;	
}

/*.BH--------------------------------------------------------
**
** 函数名: RenderTerrainWarningStripeColor2
**
** 描述:  外部相关函数整理1
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口3
**
**.EH--------------------------------------------------------
*/
void RenderTerrainWarningStripeColor2(sGLRENDERSCENE *pHandle, sQTMAPNODE *pNode)
{
	if(NULL == pHandle || NULL == pNode || NULL == pNode->fterrain)
	    return;

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)

#if 0	    
#ifdef _JM7200_
	/* JM7200的glTexCoordPointer不支持GL_SHORT，修改为GL_FLOAT*/
	{    
		int i = 0;
		float tt[vertexNum] = {0};
		for (i = 0; i< vertexNum; i++)
		{
			tt[i] = pNode->fterrain[i] * 1.0f;
		}
		glClientActiveTexture(GL_TEXTURE1);                
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, tt);	
	}
#else
	{              
		glClientActiveTexture(GL_TEXTURE1);                
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_SHORT, 0, pNode->fterrain);
	}
#endif
#endif

	/* 上述方法影响帧率，将fterrain在读取的时候就由short类型转换成float类型 */
	{              
		glClientActiveTexture(GL_TEXTURE1);                
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, pNode->fterrain);
	}
#else
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(1, GL_FLOAT, 0, pNode->fterrain);	

#endif
}



/*.BH--------------------------------------------------------
**
** 函数名: RenderTerrainWarningStripeColor3
**
** 描述:  外部相关函数整理2
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口4
**
**.EH--------------------------------------------------------
*/
void RenderTerrainWarningStripeColor3()
{

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	//glActiveTextureARB(GL_TEXTURE1_ARB);					// 操作纹理单元1
	glActiveTexture(GL_TEXTURE1);					        // 操作纹理单元1
	glMatrixMode(GL_TEXTURE);								// 转为纹理矩阵操作
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// 恢复矩阵堆栈为模型视图矩阵
#ifdef TER_WARNING_USE_TEXTURE_1D		
	glDisableEx(GL_TEXTURE_1D);
#else
    glDisableEx(GL_TEXTURE_2D);
#endif
	//glActiveTextureARB(GL_TEXTURE0_ARB);					// 操作纹理单元0
	glActiveTexture(GL_TEXTURE0);					// 操作纹理单元0
#else

	glActiveTextureARB(GL_TEXTURE1);					// 操作纹理单元1
	glMatrixMode(GL_TEXTURE);								// 转为纹理矩阵操作
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// 恢复矩阵堆栈为模型视图矩阵
#ifdef TER_WARNING_USE_TEXTURE_1D		
	glDisableEx(GL_TEXTURE_1D);
#else
	glDisableEx(GL_TEXTURE_2D);
#endif
	glActiveTextureARB(GL_TEXTURE0);					// 操作纹理单元0
#endif
}


