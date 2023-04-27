#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include <vxWorks.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"

#include "loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "ModelreadTxtFile.h"

#ifdef WIN32
//#include "../particle/particleInterface.h"
#endif

//�ṹ�嶨��







// ��������
void RenderModelChild(LPModelRender drawChildModel);
void RenderModelSelf(LPModelRender pRenderModelSelf);
// int userFuncModelRegedit(USER_MODEL_DRAW_FUNC userLayerDrawFunc, void *userLayerParam);
void EnableLight(BOOL bEnable);
void EnableLight1(BOOL bEnable);


// ��������
void RenderModelLLH(LPModelRender pRenderModelLLH , sGLRENDERSCENE* pModelSceneLLH)  
{
	PT_3D ptPos/*, ptUp, pNor, pEast, pNorth*/;

	GLfloat light_ambient[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_position[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_position2[4] = {0.0, 0.0, 0.0, 1.0};

	Matrix44 model_mx, /*model_mx1, */temp_mx;
	Matrix44 MxScale;
	Matrix44 MxWorld;	
	Matrix44 MxRotate;
	Matrix44 MxModelM;		//ģ�͵�ģʽ����

	f_float64_t size = 0.0;
	f_int32_t i = 0;

	// 1.����ģ��״̬ʱ��3ds����Ϊ�գ�������
	if(pRenderModelLLH->m_pModel3ds == NULL && pRenderModelLLH->m_modelDisplaytype <= 10)
	{
		return ;
	}

	if(pRenderModelLLH->m_selectMode == FALSE)
	{
		//����ͶӰ����,�볡�����Ƶ�ͶӰ����һ��
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
	//	glLoadMatrixd(g_SphereRender.m_Render.m_lfProjMatrix.m);
		glLoadMatrixd(Get_PROJECTION(pModelSceneLLH));
	}
	else
	{
#ifdef WIN32		
		//��ѡ��ģʽ����ʱ����Ҫ��LoadName
		glLoadName(pRenderModelLLH->m_modelID);
#endif
	}
	
	//����ģʽ����
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfModelMatrix.m);
//	glLoadMatrixd(Get_MODELVIEW());

	// 2.����ת������,�轫����ϵת����ģ�����ڵ�����ϵ
	if(pRenderModelLLH->m_MatrixChange == TRUE)
	{
		//��������겢�洢
		vector3DMemSet(&ptPos, 
			pRenderModelLLH->m_modelPoxWorld.x, 
			pRenderModelLLH->m_modelPoxWorld.y, 
			pRenderModelLLH->m_modelPoxWorld.z);

		size = pRenderModelLLH->m_modelScale;

		//����ģ��λ�ü�����ת�����󲢴洢����ֵ
		calcWorldMatrix44(ptPos.x,  ptPos.y,  ptPos.z, &MxWorld);
		dMatrix44Copy(&pRenderModelLLH->m_mxWorld, &MxWorld);	

		//����ģ�����Ŵ�С�������ž���,����֮ǰ�õ���ת���������
		dMatrix44MemSet(&MxScale, size, 0, 0, 0,
								   0, size, 0, 0,
								   0, 0, size, 0,
								   (1-size)*MxWorld.m[12], (1-size)*MxWorld.m[13], (1-size)*MxWorld.m[14], 1);
		Maxtrix4x4_Mul(&temp_mx, &MxWorld, &MxScale);

		//����ģ�͵ĺ���/����/���������̬�任����,����֮ǰ�õ���ת���������
		dMatrix44RPYmemSet(&MxRotate, 
			pRenderModelLLH->m_modelAngle.y, 
			-pRenderModelLLH->m_modelAngle.x, 
			pRenderModelLLH->m_modelAngle.z);
		//�洢��̬�任����ֵ
		dMatrix44Copy(&pRenderModelLLH->m_mxRotate, &MxRotate);	
		//�õ����յ�ת������
		Maxtrix4x4_Mul(&model_mx, &MxRotate, &temp_mx);
		//�洢���յ�ת������
		memcpy(&pRenderModelLLH->m_modelMatrix[0], &model_mx.m[0],  sizeof(f_float64_t) * 16);

		pRenderModelLLH->m_MatrixChange = FALSE;
		
		pRenderModelLLH->m_stringWorldChange = TRUE;		//��֤�ַ����ľ���ͬʱ���¼���
	}

#if	FALSE
	glMultMatrixd(pRenderModelLLH->m_modelMatrix);

#else
	// 3.ת�������볡����ģʽ������ˣ�������ģʽ���󣬺����Ϳ�����ģ������ϵ��ֱ�ӻ���ģ��
	{
		Matrix44 mModelMatrix;
		Matrix44 mModelRender;
		f_float64_t * pModelRender = Get_MODELVIEW(pModelSceneLLH);

		dMatrix44MemSet(&mModelMatrix, 
			pRenderModelLLH->m_modelMatrix[0],pRenderModelLLH->m_modelMatrix[1],pRenderModelLLH->m_modelMatrix[2],pRenderModelLLH->m_modelMatrix[3],
			pRenderModelLLH->m_modelMatrix[4],pRenderModelLLH->m_modelMatrix[5],pRenderModelLLH->m_modelMatrix[6],pRenderModelLLH->m_modelMatrix[7],
			pRenderModelLLH->m_modelMatrix[8],pRenderModelLLH->m_modelMatrix[9],pRenderModelLLH->m_modelMatrix[10],pRenderModelLLH->m_modelMatrix[11],
			pRenderModelLLH->m_modelMatrix[12],pRenderModelLLH->m_modelMatrix[13],pRenderModelLLH->m_modelMatrix[14],pRenderModelLLH->m_modelMatrix[15]);

		dMatrix44MemSet(&mModelRender,
			pModelRender[0],pModelRender[1],pModelRender[2],pModelRender[3],
			pModelRender[4],pModelRender[5],pModelRender[6],pModelRender[7],
			pModelRender[8],pModelRender[9],pModelRender[10],pModelRender[11],
			pModelRender[12],pModelRender[13],pModelRender[14],pModelRender[15]		);
//		Maxtrix4x4_Mul(&MxModelM, &mModelMatrix, &g_SphereRender.m_Render.m_lfModelMatrix);
		Maxtrix4x4_Mul(&MxModelM, &mModelMatrix, &mModelRender);
	
		glLoadMatrixd(MxModelM.m);
	}

	
#endif
	// 4.�ж����α�־�Ƿ��
	if(pRenderModelLLH->m_flagshield == FALSE)
	{
		// 0-�������� 1- 3- 6- ��������,����ģʽ�رչ���
		if((pRenderModelLLH->m_modelDisplaytype != 2)
			&&(pRenderModelLLH->m_modelDisplaytype != 4)
			&&(pRenderModelLLH->m_modelDisplaytype != 5)
			&&(pRenderModelLLH->m_modelDisplaytype < 7))		
		{	
			// ��ѯ�����Ƿ�򿪣���Ļ���򿪹���
	//		if(GL_FALSE == glIsEnabled(GL_LIGHTING))
			{
				EnableLight(TRUE);
				EnableLight1(TRUE);			
			}

		}
		else
		{
	//		EnableLight(FALSE);
	//		EnableLight1(FALSE);
		
		}

		// 5.����ģ�ͱ���
		RenderModelSelf(pRenderModelLLH);

		// 6.������ģ��
		if(pRenderModelLLH->m_childNum > 0)
		{
			stList_Head *pstListHead = &(pRenderModelLLH->m_childlist);
			stList_Head *pstTmpList = NULL;

			LPModelRender lpsinglemodel = NULL;

			LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
			{
				if( NULL != pstTmpList )
				{
					lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

					glPushMatrix();

					RenderModelChild(lpsinglemodel);

					glPopMatrix();

				}
			}
		}
	}

	//�ָ�ͶӰ����
	if(pRenderModelLLH->m_selectMode == FALSE)
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	//�ָ�ģʽ����
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
}

//�˺�����������ά��
#if 0
void RenderModelXYZ(
	const LPModel3DS pModel, const f_uint32_t glList,  const BOOL use_buildin_list,
	const f_uint8_t minLevel,	const f_uint8_t maxLevel,
	const f_float64_t x, const f_float64_t y, const f_float64_t z, 
	const f_float64_t pitch, const f_float64_t yaw, const f_float64_t roll,
	const f_float64_t size,
	LPMatrix4x4 pMxWorld,
	LPMatrix4x4 pMxRotate,
	BOOL bShowModel,
	int displaytypeXYZ)  
{
//	PT_3D ptPos, ptUp, pNor, pEast, pNorth;
#ifdef HIGH_PRECISE_MATRIX
	Matrix4x4 model_mx, model_mx1, temp_mx;
	Matrix4x4 MxScale;
#endif

	//PT_3D pt0, pt1, pt2, pt3, pt4;
	if ((pModel == NULL && glList == 0) || pMxRotate == NULL || pMxWorld == NULL)
	{
#ifndef WIN32
		printf("RenderModelXYZ, pModel == NULL && glList == 0 || pMxRotate == NULL || pMxWorld == NULL\n");
#else
#endif
		return;
	}
//	glDisableEx(GL_CULL_FACE);
	glEnableEx(GL_DEPTH_TEST);

#ifndef HIGH_PRECISE_MATRIX
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	CalcWorldMatrix(x,  y,  z, pMxWorld);
	glMultMatrixd(pMxWorld->m);
	glScaled(size, size, size);
	MemSet_RMatrix_RPY(pMxRotate, pitch, -yaw, roll);
	glMultMatrixd(pMxRotate->m);
#else
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfProjMatrix.m);
	glLoadMatrixd(Get_PROJECTION());


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfModelMatrix.m);
	glLoadMatrixd(Get_MODELVIEW());

	EnableLight(TRUE);
	EnableLight1(TRUE);

	CalcWorldMatrix(x,  y,  z, pMxWorld);
	MemSet_Matrix4x4(&MxScale, size, 0, 0, 0,
							   0, size, 0, 0,
							   0, 0, size, 0,
							   (1-size)*pMxWorld->m[12], (1-size)*pMxWorld->m[13], (1-size)*pMxWorld->m[14], 1);
	Maxtrix4x4_Mul(&temp_mx, pMxWorld, &MxScale);
	MemSet_RMatrix_RPY(pMxRotate, pitch, -yaw, roll);
	Maxtrix4x4_Mul(&model_mx, pMxRotate, &temp_mx);
	//Maxtrix4x4_Mul(&model_mx1,&model_mx, &modelview_last);
	//glMultMatrixd(model_mx1.m);
	glMultMatrixd(model_mx.m);
#endif

#ifdef TEST_GRAY
	switch(g_nDisplayMode)
	{
	case DISPLAY_MODE_RGB_MAP:                            // RGB��ʾģʽ
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;
	case DISPLAY_MODE_DREYSCALE_MAP:	// �Ҷ�ͼ��ʾģʽ
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_ALPHA);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

		break;
	default:                            // Ĭ����ʾģʽ
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;
	}
#endif

	glEnableEx(GL_TEXTURE_2D);
	if(use_buildin_list == TRUE)
	{
		if(displaytypeXYZ > 10)
		{
			RenderParticleType(displaytypeXYZ - 11);

		}
		else
		{
			Draw_3DS_GLList(pModel, displaytypeXYZ);
		}
	}
	else
	{
		if (glList != 0) 
		{
			if(bShowModel)
			{
				glCallList(glList);
			}
		}
		else 
		{
			if(bShowModel)
			{
				Draw_3DS(pModel);
			}
		}
	}

	EnableLight(FALSE);
	EnableLight1(FALSE);

	//glDisableEx(GL_DEPTH);
//	glEnableEx(GL_CULL_FACE);
	glDisableEx(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
#endif
/*
 * ���ܣ���������������������
 * ���룺x         ����x����
 *       y         ����y����
 *       z         ����z����
 * �����pMxWorld  �������
 * ���أ���
 *
void CalcWorldMatrix(const f_float64_t x, const f_float64_t y, const f_float64_t z, LPMatrix4x4 pMxWorld)
{
	PT_3D ptPos, ptUp, pNor, pEast, pNorth;
	
	MemSet_3D(&ptPos, x, y, z);
	SetValue_3D(&ptUp, &ptPos);
	Normalize_3D(&ptUp);
	MemSet_3D(&pNor, 0, 0, 1);
	CrossProduct_3D(&pEast, &pNor, &ptUp);		// 
	Normalize_3D(&pEast);
	CrossProduct_3D(&pNorth, &pEast, &ptUp);
	Normalize_3D(&pNorth);
	MemSet_Matrix4x4(pMxWorld, pEast.x,   pEast.y,   pEast.z,  0.0,
							   ptUp.x,    ptUp.y,    ptUp.z,   0.0,
						       pNorth.x,  pNorth.y,  pNorth.z, 0.0,
						       ptPos.x,   ptPos.y,   ptPos.z,  1.0f );
						       
}
*/
//#ifdef HIGH_PRECISE_MATRIX
/*
 * ���ܣ���ȡͶӰ����
 * ���룺��
 * �����ͶӰ����
 * ���أ���
 */
f_float64_t * Get_PROJECTION(sGLRENDERSCENE* pSceneGetMatrix)
{
//	return g_SphereRender.m_Render.m_lfProjMatrix.m;
//	return ((sGLRENDERSCENE*)pScene[0])->m_lfProjMatrix;
	return pSceneGetMatrix->m_lfProjMatrix;
}

/*
 * ���ܣ���ȡģʽ����
 * ���룺��
 * �����ģʽ����
 * ���أ���
 */
f_float64_t * Get_MODELVIEW(sGLRENDERSCENE* pSceneGetMatrix)
{
//	return g_SphereRender.m_Render.m_lfModelMatrix.m;
//	return modelview_out.m;
//	return ((sGLRENDERSCENE*)pScene[0])->m_lfModelMatrix;
	return pSceneGetMatrix->m_lfModelMatrix;
}

//#endif


/*.BH--------------------------------------------------------
**
** ������: RenderModelSelf
**
** ����:  ����任֮��Ļ���ģ�ͺ���
**
** �������:  ģ����Ϣ
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�1, used in RenderModelLLH()
**
**.EH--------------------------------------------------------
*/
void RenderModelSelf(LPModelRender pRenderModelSelf)  
{
	//10����������Ч������δʵ��
	if(pRenderModelSelf->m_modelDisplaytype > 10)
	{
#if 0		
		RenderParticleType(pRenderModelSelf->m_modelDisplaytype - 11);

		{
			static int initparticle = 0;
				
			if(initparticle == 0)
			{
				ParticleSysInit();
				initparticle = 1;
			}
			//������Ч 0-ѩ 1-�� 2-��
			SetSpeEffict(pRenderModelSelf->m_modelDisplaytype - 11);

			ParticleSysUpdate();
				
			ParticleSysRender(1,1, 0,0);
				
		}
#endif
	}
	else
	{
		//����ģ��
		Draw_3DS_GLList(pRenderModelSelf->m_pModel3ds, 
						pRenderModelSelf->m_modelDisplaytype,
						pRenderModelSelf->m_color[pRenderModelSelf->m_colorNum_use],
						pRenderModelSelf->m_colorPicture);
	}
	
}

/*.BH--------------------------------------------------------
**
** ������: RenderModelChild
**
** ����:  ����任֮��Ļ�����ģ�ͺ���
**
** �������:  ģ����Ϣ
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�2, used in RenderModelLLH(), lon ,lat ,hei�ĵ�λ������
**
**.EH--------------------------------------------------------
*/
void RenderModelChild(LPModelRender drawChildModel)  
{
	PT_3D ptPos;
	Matrix44 model_mx, temp_mx;
	Matrix44 MxScale;
	Matrix44 MxWorld;	
	Matrix44 MxRotate;
	f_float64_t size = 0.0;
	
	if(drawChildModel->m_pModel3ds == NULL && drawChildModel->m_modelDisplaytype <= 10)
	{
		return;
	}

	if(drawChildModel->m_MatrixChange == TRUE)
	{
		// 1.����任���洢
		vector3DMemSet(&ptPos, drawChildModel->m_modelPos.y, 
						drawChildModel->m_modelPos.z, 
						drawChildModel->m_modelPos.x);
		dMatrix44MemSet(&MxWorld, 1,   	0,   		0,   		0.0,
								  0,   	1,   		0,   		0.0,
							         0,   	0,   		1,   		0.0,
							         ptPos.x, ptPos.y, ptPos.z,   1.0f );

		size = drawChildModel->m_modelScale;
		dMatrix44MemSet(&MxScale, size, 0, 0, 0,
								   0, size, 0, 0,
								   0, 0, size, 0,
								   (1-size)*MxWorld.m[12], (1-size)*MxWorld.m[13], (1-size)*MxWorld.m[14], 1);
		Maxtrix4x4_Mul(&temp_mx, &MxWorld, &MxScale);
		dMatrix44RPYmemSet(&MxRotate, drawChildModel->m_modelAngle.y, 
									-drawChildModel->m_modelAngle.x,
									 drawChildModel->m_modelAngle.z);
		
		Maxtrix4x4_Mul(&model_mx, &MxRotate, &temp_mx);

		memcpy(&drawChildModel->m_modelMatrix[0],&model_mx.m[0],  sizeof(f_float64_t) * 16);	

		drawChildModel->m_MatrixChange = FALSE;
	}
	
	glMultMatrixd(drawChildModel->m_modelMatrix);

	// 2.����ģ�ͱ���
	RenderModelSelf(drawChildModel);


}


/*
���ܣ��û�����ע��
�����0��ע��ɹ���-1��ע��ʧ��
*
int userFuncModelRegedit(USER_LAYER_DRAW_FUNC userLayerDrawFunc, void *userLayerParam)
{
	  funcModelRegedit.userLayerDrawFunc = NULL;
	  funcModelRegedit.userLayerParam = NULL;
    if(NULL == userLayerDrawFunc)
        return(-1);
        
    funcModelRegedit.userLayerDrawFunc = userLayerDrawFunc;
    funcModelRegedit.userLayerParam = userLayerParam;
    return(0);
}
*/
/*
 *	�����رճ�����Դ��������ԴΪƽ�й⣩
 */
void EnableLight(BOOL bEnable)
{
// 	f_float32_t fPosition[] = {0.0f,0.0f,0.0f,1.0f};	// �����Դ
// 	f_float32_t fAmbient[]  = {1.0f,1.0f,1.0f,1.0f};  // ȫ�ֹ�(������)����RE
// 	f_float32_t fDiffuse[]  = {1.0f, 1.0f, 1.0f,1.0f};	// ����������
// 	f_float32_t fSpecular[] = {1.0f,1.0f,1.0f,1.0f};	// ���淴�����
// 	f_float32_t fPosition1[] = {1.0f,1.0f,1.0f,0.0f};	//ƽ�й�RE

	f_float32_t gAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};//global ambient default
	f_float32_t gAmbient1[] = {1.0f, 1.0f, 1.0f, 1.0f};//global ambient set
	
	if( bEnable )
	{

		/*	//���ò�������
		{
			f_float32_t mat_amb_R = 1.0;//90/255;
			f_float32_t mat_amb_G = 1.0;//74/255;
			f_float32_t mat_amb_B = 1.0;//50/255;

			f_float32_t mat_amb_diff[] = {mat_amb_R, mat_amb_G, mat_amb_B, 1.0f};	//0.3
			f_float32_t mat_specular[] = {1.0f,1.0f,1.0f,1.0f};	//��ʹ�þ��淴��RE
			f_float32_t mat_emission[] = {0.0f,0.0f,0.0f,1.0f};	
			f_float32_t mat_shininess[] = {50.0};
			
			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
			glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);	
		}*/
			
		//��ȫ�ֹ���
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT , gAmbient1);
#else
		{
			int prog = 0;
			GLuint lightColorLoc = -1;
			GLuint lightIntensityLoc = -1;
			glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
			lightColorLoc = glGetAttribLocation(prog, "dLight.base.color");
			lightIntensityLoc = glGetAttribLocation(prog, "dLight.base.AmbientIntensity");

			glUniform3f(lightColorLoc, 1.0,  1.0,  1.0);
			glUniform1f(lightIntensityLoc, gAmbient1[0]);

		}
#endif
// 		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE , GL_FALSE);	//�رձ��涥��ļ���	//test
// 		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, 
//			GL_SEPARATE_SPECULAR_COLOR);				//������ͼ+ ����
		
		glEnableEx(GL_LIGHTING);								// �򿪹���

	
	//	glEnableEx(GL_AUTO_NORMAL);							// �Զ����ɷ���
	//	glEnableEx(GL_NORMALIZE);								// ��������
	}
	else
	{
		//printf("�رչ���\n");
		glDisableEx(GL_LIGHTING );
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
		glDisableEx(GL_COLOR_MATERIAL);
		glDisableEx(GL_AUTO_NORMAL);
		glDisableEx(GL_NORMALIZE);
#endif
	}
}


static PT_3D g_LightPositionVertex;	//����ȫ�ֱ���
/*
���ù�������,ȡ��������
*/
void SetLightPos(double x, double y, double z)
{
	g_LightPositionVertex.x = -x;
	g_LightPositionVertex.y = -y;
	g_LightPositionVertex.z = -z;
}
/*
	��ȡ��������
*/
LP_PT_3D GetLightPos()
{
	return &g_LightPositionVertex;
}
float flight_i = 0;
int Light0Choose = 0;			//��Դ�Ƿ��ƶ�����,0��������1���ƶ���
int Light0Stop = 0;				//��Դ�ƶ�λ���Ƿ���ס���أ�0��������1����ס��
void EnableLight0(BOOL bEnable)
{
	f_float32_t fAmbient0[] = { 1.0f, 1.0f, 1.0f, 1.0f };		// ȫ�ֹ�(������)����RE		//test
	f_float32_t fAmbient1[] = { 0.0f, 0.0f, 0.0f, 1.0f };		// ȫ�ֹ�(������)����RE		//test
	f_float32_t fAmbient2[] = { 0.2f, 0.2f, 0.2f, 1.0f };		// ȫ�ֹ�(������)����RE		//test

	f_float32_t fDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };	// ����������
	f_float32_t fSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };		// ���淴�����

	f_float32_t fPosition1[4] = { 0.0f, 1.0f, 0.0f, 0.0f };		// ƽ�й�RE

	if (Light0Choose == 1)
	{
		fPosition1[0] = 1.0f*cos(flight_i);	// ƽ�й�RE					//test		
		fPosition1[1] = 1.0f*sin(flight_i);	// ƽ�й�RE					//test	
		fPosition1[2] = 0.0f;				// ƽ�й�RE	
	}
	else
	{
		flight_i = 0.0;//(/*180.0 +*/ 120.0) / 180.0 * 3.14;//0.0;//(/*180.0 +*/ 120.0) / 180.0 * 3.14;//3.1f;
		fPosition1[0] = 1.0f*cos(flight_i);	// ƽ�й�RE					//test		
		fPosition1[1] = 1.0f*sin(flight_i);	// ƽ�й�RE					//test	
		fPosition1[2] = 0.0f;				// ƽ�й�RE	


		flight_i = ( 10.0 ) / 180.0 * 3.14;;

		fPosition1[0] = cos(flight_i);	// ƽ�й�RE					//test		
		fPosition1[1] = 0;	// ƽ�й�RE					//test	
		fPosition1[2] = sin(flight_i);				// ƽ�й�RE	


	}

	if (Light0Stop == 0)
		flight_i += 0.005;
	else
	{
		//printf("flight_i = %f\n",flight_i);
	}

	if (flight_i >= 3.14 * 2)
	{
		flight_i = 0;
	}

	// 	f_float32_t gAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};//global ambient default
	// 	f_float32_t gAmbient1[] = {1.0f, 1.0f, 1.0f, 1.0f};//global ambient set
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	if(0)
	{
		int prog = 0;
		GLuint posLoc, colorLoc, amblentIntensity, diffuseIntensity, sprcularIntensrity, specularPow;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		posLoc = glGetUniformLocation(prog, "dLight.direction");
		colorLoc = glGetUniformLocation(prog, "dLight.base.color");
		amblentIntensity = glGetUniformLocation(prog, "dLight.base.AmbientIntensity");
		diffuseIntensity = glGetUniformLocation(prog, "dLight.base.DiffuseIntensity");    // make sure the DiffuseIntensity of d-light pLight -> dLight 20190218 by mqx
		sprcularIntensrity = glGetUniformLocation(prog, "specularIntensity");
		specularPow = glGetUniformLocation(prog, "specularPower");
		glUniform3f(posLoc, fPosition1[0], fPosition1[1], fPosition1[2]);
		glUniform3f(colorLoc, fAmbient0[0], fAmbient0[1], fAmbient0[2]);
		glUniform1f(amblentIntensity, fAmbient0[3]);    //fAmbient0[3]
		glUniform1f(diffuseIntensity, fDiffuse[3]);
		glUniform1f(sprcularIntensrity, fSpecular[3]);
		glUniform1f(specularPow, 0.1);                  // user setting : 0.1
		// other attributes use default values

		glEnableEx(GL_DIRECTLIGHT);
	}
	else
	{
		glDisableEx(GL_DIRECTLIGHT);
	}
#else
	if (bEnable)
	{
		glLightfv(GL_LIGHT0, GL_POSITION, fPosition1);		// ����ƽ�йⷽ����߶����Դ
		glLightfv(GL_LIGHT0, GL_AMBIENT, fAmbient0);			// ����ȫ�ֹ���ɫ
		glLightfv(GL_LIGHT0, GL_DIFFUSE, fDiffuse);			// ������������ɫ
		glLightfv(GL_LIGHT0, GL_SPECULAR, fSpecular);

		glEnableEx(GL_LIGHT0);								// ���õƹ�

		//glEnableEx(GL_RESCALE_NORMAL);

	}
	else
	{
		glDisableEx(GL_LIGHT0);
	}
#endif
	//���ù�������
	SetLightPos((f_float64_t)fPosition1[0], (f_float64_t)fPosition1[1], (f_float64_t)fPosition1[2]);

}


/*
	���ù�Դ1�Ŀ����ر�
*/
 void EnableLight1(BOOL bEnable)
{
	f_float32_t fPosition[] = {0.0f,0.0f,0.0f,1.0f};	// �����Դ
	f_float32_t fAmbient[]  = {1.0f,1.0f,1.0f,1.0f};  // ȫ�ֹ�(������)����RE
	f_float32_t fDiffuse[]  = {1.0f, 1.0f, 1.0f,1.0f};	// ����������
	f_float32_t fSpecular[] = {1.0f,1.0f,1.0f,1.0f};	// ���淴�����
	f_float32_t fPosition1[] = {1.0f,1.0f,1.0f,0.0f};	//ƽ�й�RE

// 	f_float32_t gAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};//global ambient default
// 	f_float32_t gAmbient1[] = {1.0f, 1.0f, 1.0f, 1.0f};//global ambient set

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	if (bEnable)
	{
		int prog = 0;
		GLuint posLoc, colorLoc, amblentIntensity, sprcularIntensrity, diffuseIntemsity, specularPow;
		GLuint pLight_atten_const, pLight_atten_linear, pLight_atten_exp;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		posLoc = glGetUniformLocation(prog, "pLight.position");
		colorLoc = glGetUniformLocation(prog, "pLight.base.color");
		amblentIntensity = glGetUniformLocation(prog, "pLight.base.AmbientIntensity");
		diffuseIntemsity = glGetUniformLocation(prog, "pLight.base.DiffuseIntensity");
		sprcularIntensrity = glGetUniformLocation(prog, "specularIntensity");
		specularPow = glGetUniformLocation(prog, "specularPower");
		glUniform3f(posLoc, fPosition[0], fPosition[1], fPosition[2]);
		glUniform3f(colorLoc, fAmbient[0], fAmbient[1], fAmbient[2]);
		glUniform1f(amblentIntensity, fAmbient[3]);
		glUniform1f(diffuseIntemsity, fDiffuse[3]);
		glUniform1f(sprcularIntensrity, fSpecular[3]);
		glUniform1f(specularPow, 0.1);
		// point light need Attenuation values
		pLight_atten_const = glGetUniformLocation(prog, "pLight.atten.Constant");
		pLight_atten_linear = glGetUniformLocation(prog, "pLight.atten.Linear");
		pLight_atten_exp = glGetUniformLocation(prog, "pLight.atten.Exp");
		glUniform1f(pLight_atten_const, 1.0);
		glUniform1f(pLight_atten_linear, 0.0);
		glUniform1f(pLight_atten_exp, 0.01);
		// other attributes use default values

		glEnableEx(GL_POINTLIGHT);
	}
	else
	{
		glDisableEx(GL_POINTLIGHT);
	}
#else
	if( bEnable )
	{	
		glLightfv(GL_LIGHT1,GL_POSITION, fPosition);		// ���ö����Դ
		glLightfv(GL_LIGHT1,GL_AMBIENT,fAmbient);			// ����ȫ�ֹ���ɫ
		glLightfv(GL_LIGHT1,GL_DIFFUSE,fDiffuse);			// ������������ɫ
		glLightfv(GL_LIGHT1,GL_SPECULAR,fSpecular);

		glEnableEx(GL_LIGHT1);								// ���õƹ�
//���������ʱע��
//		glEnableEx(GL_COLOR_MATERIAL);						// ������ɫ����	

		//��ӷ�����������
		glEnableEx(GL_RESCALE_NORMAL);
	}
	else
	{
		glDisableEx(GL_LIGHT1 );
//		glDisableEx(GL_COLOR_MATERIAL);
	}
#endif
}


