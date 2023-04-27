
#include "../define/macrodefine.h"
#include "terWarning.h"
#include "../mapApp/mapApp.h"

#define TERRAINWARNING_COLOR_SAMPLING 256		// ˮƽ���θ澯������
#define TERRAINWARNING_COLOR_ALPHA	  200       // ˮƽ���θ澯ɫ����͸����

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32	
extern PFNGLACTIVETEXTUREPROC glActiveTexture;	
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;	
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f;
#endif                                     /* Ӱ���������������� */
#endif

/* ��������ˮƽ���θ澯ɫ����RGBA���� */
f_uint8_t chRGBA[TERRAINWARNING_COLOR_SAMPLING * 4];

/*.BH--------------------------------------------------------
**
** ������: SetTerrainWarningStripeColor
**
** ����:  ����ˮƽ���θ澯��ɫ��
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�1
**
**.EH--------------------------------------------------------
*/
void setTerrainWarningStripeColor(VOIDPtr scene, sTerWarningSet terwarning_set)
{
	sGLRENDERSCENE * pScene = NULL;	
		
	// ����ˮƽ���θ澯ɫ��
	f_float64_t texpixel_permeter = 0.0;
	f_int32_t i, j, layers;
	f_float32_t  height_region = 1.0f;

	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return;


	memset(chRGBA, 0, TERRAINWARNING_COLOR_SAMPLING * 4);		// Ĭ�϶��Ǻ�ɫ
	layers = terwarning_set.layers;
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 0] = terwarning_set.ter_warning_data[layers - 1].red;
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 1] = terwarning_set.ter_warning_data[layers - 1].green;
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 2] = terwarning_set.ter_warning_data[layers - 1].blue;	
	chRGBA[(TERRAINWARNING_COLOR_SAMPLING - 1) * 4 + 3] = TERRAINWARNING_COLOR_ALPHA;
	
	chRGBA[0 * 4 + 0] = terwarning_set.ter_warning_data[0].red;
	chRGBA[0 * 4 + 1] = terwarning_set.ter_warning_data[0].green;
	chRGBA[0 * 4 + 2] = terwarning_set.ter_warning_data[0].blue;	
	chRGBA[0 * 4 + 3] = 0;			
	
	// ÿ�����������, ��0.0->1.0�ļ��
	// ����ÿ������Ӧ������������Ϊ
	// �����ܳ���Ϊ256����ȥ���ߵģ�ֻʣ��254����ÿ����Ԫ������Ӧ������Χ�� 254����������ʾheight_region�׵�����
	height_region = terwarning_set.height_max - terwarning_set.height_min;
	pScene->s_nTexCoordPerMeter = (f_float32_t)(254.0f / 256.0f / height_region);		// ÿ������Ҫ������������
	texpixel_permeter = height_region / 254.0f;				// ��������Ӧ��m
	// ����ɫ����ɫ
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
** ������: GenTerrainWarningTexture
**
** ����:  ����ˮƽ���θ澯��ɫ������
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�1
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
	/*ˮƽ���θ澯��ɫ���������ɳɹ�*/
    pScene->is_warn_data_set_success = sSuccess;
	/*ˮƽ���θ澯��ɫ������������������*/
    pScene->is_need_regenerate_warn_texid = 0;
}

/*.BH--------------------------------------------------------
**
** ������: RenderTerrainWarningStripeColor
**
** ����:  ����ˮƽ���θ澯��ɫ��
**
** �������:  ��
**
** �����������
**
** ����ֵ���ɹ�������TRUE;
**				 ʧ�ܣ�����FALSE;
**          
**
** ���ע��:  �ⲿ�ӿ�2
**
**.EH--------------------------------------------------------
*/
BOOL RenderTerrainWarningStripeColor(sGLRENDERSCENE *pHandle)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	if(NULL == pHandle)
	    return FALSE;
	
	//���ɻ���ǰ�߶���Ϊ�澯�߶�
	pHandle->terwaring_height = pHandle->geopt_pos.height;


	//��������ɫ��
	//glActiveTextureARB(GL_TEXTURE1_ARB);
	glActiveTexture(GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 

//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
#ifdef TER_WARNING_USE_TEXTURE_1D	
	glEnableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, pHandle->warn_texid);	// ��ˮƽ���θ澯����
#else
	glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pHandle->warn_texid);	// ��ˮƽ���θ澯����
#endif
	glMatrixMode(GL_TEXTURE);							// תΪ����������
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

	//���ɻ���ǰ�߶���Ϊ�澯�߶�
	pHandle->terwaring_height = pHandle->geopt_pos.height;

	//��������ɫ��
	glActiveTextureARB(GL_TEXTURE1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

#ifdef TER_WARNING_USE_TEXTURE_1D	
	glEnableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, pHandle->warn_texid);	// �󶨵��θ澯����
#else
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pHandle->warn_texid);	// �󶨵��θ澯����
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
** ������: RenderTerrainWarningStripeColor2
**
** ����:  �ⲿ��غ�������1
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�3
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
	/* JM7200��glTexCoordPointer��֧��GL_SHORT���޸�ΪGL_FLOAT*/
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

	/* ��������Ӱ��֡�ʣ���fterrain�ڶ�ȡ��ʱ�����short����ת����float���� */
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
** ������: RenderTerrainWarningStripeColor3
**
** ����:  �ⲿ��غ�������2
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�4
**
**.EH--------------------------------------------------------
*/
void RenderTerrainWarningStripeColor3()
{

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	//glActiveTextureARB(GL_TEXTURE1_ARB);					// ��������Ԫ1
	glActiveTexture(GL_TEXTURE1);					        // ��������Ԫ1
	glMatrixMode(GL_TEXTURE);								// תΪ����������
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// �ָ������ջΪģ����ͼ����
#ifdef TER_WARNING_USE_TEXTURE_1D		
	glDisableEx(GL_TEXTURE_1D);
#else
    glDisableEx(GL_TEXTURE_2D);
#endif
	//glActiveTextureARB(GL_TEXTURE0_ARB);					// ��������Ԫ0
	glActiveTexture(GL_TEXTURE0);					// ��������Ԫ0
#else

	glActiveTextureARB(GL_TEXTURE1);					// ��������Ԫ1
	glMatrixMode(GL_TEXTURE);								// תΪ����������
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// �ָ������ջΪģ����ͼ����
#ifdef TER_WARNING_USE_TEXTURE_1D		
	glDisableEx(GL_TEXTURE_1D);
#else
	glDisableEx(GL_TEXTURE_2D);
#endif
	glActiveTextureARB(GL_TEXTURE0);					// ��������Ԫ0
#endif
}


