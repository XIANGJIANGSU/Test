/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: ModelApi.c
**
** ����: ���ļ�����ģ�ͳ�ʼ����ģ�ͻ��ƵĽӿڵ��ú�����
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
**		2016-10-24 9:47 LPF �������ļ�
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#include <unistd.h>
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
#include "../projection/coord.h"
#include "../geometry/vectorMethods.h"
#include "../utility/tools.h"

#include "loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "WhiteCity.h"
#include "PathStore.h"


/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

/* �û��ϲ�ʵ�ֹ��ܺ���ע�� */
typedef struct {
	USER_MODEL_DRAW_FUNC userLayerDrawFunc;   /* �û�ͼ�㺯�� */
	void *userLayerParam;                     /* �û�ͼ����� */
}sFUNCModelREGEDIT;


/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*/
#define MODEL_FLAG		12341234				//ģ�͵ı�Ǻ궨��
#define TRACE_HALF_HEGHT	20				// �켣���µĸ߶�

#define BILLBOARD_CENTER		0x01		/* ���ĵ�*/
#define BILLBOARD_TOP			0x02		/* ���ĵ��ڹ�����ϱ�*/
#define BILLBOARD_BOTTOM		0x03		/* ���ĵ��ڹ�����±�*/
#define BILLBOARD_LEFT			0x04		/* ���ĵ��ڹ�������*/
#define BILLBOARD_RIGHT			0x05		/* ���ĵ��ڹ�����ұ�*/
#define BILLBOARD_LEFT_BOTTOM	0x06		/* ���ĵ��ڹ�������½�*/
#define BILLBOARD_LEFT_TOP		0x07		/* ���ĵ��ڹ�������Ͻ�*/
#define BILLBOARD_RIGHT_BOTTOM	0x08		/* ���ĵ��ڹ�������½�*/
#define BILLBOARD_RIGHT_TOP		0x09		/* ���ĵ��ڹ�������Ͻ�*/


/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
static stList_Head *pModelRenderList = NULL;										//�洢����ģ�͵�����
static f_float32_t s_nColorInterval = 0.5 * 1.0 / FLYTRACE_NODENUM_MAX;	//�켣͸��������
//�ֲ���������
static sFUNCModelREGEDIT funcModelRegedit;
static SEMID g_SetModelParam;

TTFONT g_TTFFont = NULL;		// TTF����
int modelid[SCENE_NUM][81] = {0};

/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** �ڲ���������
**-----------------------------------------------------------
*/
//f_int32_t CreateNew3Number();
//f_int32_t SearchNodeIndex(f_int32_t modelID);
f_int32_t SearchNodeIndex(f_int32_t modelID, LPModelRender *pModelOut);

void SetPT3D(PT_3D vec3din, LP_PT_3D vec3dout);
//void ConvertChildPos(PT_3D F_posXYZ, PT_3D F_anglePYR, PT_3D C_pos);
//void PosPart2World(PT_3D ptCEye, PT_3D ptCCenter, PT_3D ptCUp,
//					 f_float64_t pitch, f_float64_t yaw, f_float64_t roll, LPMatrix4x4 mx);

//void ConvertChildAngle(PT_3D F_anglePYR, PT_3D C_angle,  PT_3D C_angleOut);
void PosPart2World(PT_3D ptCEye, PT_3D ptCCenter, PT_3D ptCUp,
					 f_float64_t pitch, f_float64_t yaw, f_float64_t roll, LPMatrix44 mx);
BOOL search_modelByID(f_int32_t modelID , LPModelRender* pModel_select);
BOOL rebulid_model_colorPicture(LPModelRender model_select,PT_4D colorset);
BOOL set_model_shield(LPModelRender model_select, BOOL flag);
BOOL set_model_user(LPModelRender model_select, BOOL flag, int mode);
void RenderModelType1(sGLRENDERSCENE* pModelScene);
void RenderModelType2(sGLRENDERSCENE* pModelScene);
void RenderModelType3(sGLRENDERSCENE* pModelScene);
void RenderModelType4(sGLRENDERSCENE* pModelScene);
void RenderModelType5(sGLRENDERSCENE* pModelScene);
void RenderModelType6(sGLRENDERSCENE* pModelScene);
void RenderUserFlyTrace(sGLRENDERSCENE* pModelScene);
void RenderUserRectangle(sGLRENDERSCENE* pModelScene);
void RenderUserTxt(sGLRENDERSCENE* pModelScene);
void RenderUserDiy(sGLRENDERSCENE* pModelScene);

void InitModelThread();
#ifdef WIN32
DWORD WINAPI LoadModelDataThreadFunc(LPVOID lpParameter);
#else
void LoadModelDataThreadFunc();
#endif
void worldXYZ2screenXY( PT_3D xyz, double* x, double* y);

BOOL GenerateSingleTree(double lon, double lat , double scale,double yaw);
BOOL RenderPlanes(sGLRENDERSCENE* pModelScene);
/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

/*.BH--------------------------------------------------------
**
** ������: CreateList
**
** ����:  ��ʼ������ͷ�ڵ�
**
** �������:  ͷ�ڵ�ĵ�ַ
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�1,  used in out
**
**.EH--------------------------------------------------------
*/
void CreateList(stList_Head *plist )
{
	LIST_INIT(plist);
}


BOOL InitAnnotationFont(f_char_t* file_path)
{
	if(ttfCreateFontSU(file_path, 24, 1, 0, &g_TTFFont))
	{
		//���崴��ʧ��
		printf("����TTF����ʧ�ܣ�\n");
		return FALSE;
	}
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: BOOL InitModel()
**
** ����:  ��ʼ��ģ�����ݡ���ģ���С�ģ�����弰�����ж�ģ���Ƿ�ɼ��ĵ�������
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  
**
**.EH--------------------------------------------------------
*/
BOOL InitModel()
{
	int i = 0;
	// 0.��ʼ���ź���
	//for(i=0; i<SCENE_NUM; i++)
	{
		g_SetModelParam = createBSem(Q_FIFO_SEM, FULL_SEM);
		giveSem(g_SetModelParam);
	}
	
	// 0.��ʼ��ģ�ͻ�������,ÿ�����������Ӧһ��
	for(i=0;i<SCENE_NUM;i++)
	{
		AttachpScene2Model(pScene[i]);

		CreateList(pModelRenderList);
	}
	
	// 1.��ȡģ����Ϣ
//#ifndef WIN32
	if(FALSE == InitModelData())
	{
		return FALSE;
	}

//#else
	//�޸�Ϊ������ʼ������,Ƕ��ʽ�²��Ժ�ȡ���������	
//	taskSpawn("ModelInit", 105, VX_FP_TASK | VX_NO_STACK_FILL, 0x100000, (FUNCPTR)InitModelData, 
//	           0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//#endif

	//  2.��ȡ��ģ������Ϣ
	if(FALSE == InitWhiteCityData())
	{
		return FALSE;
	}

#if 1
	// 3.����ģ�͵ĺ��߼�¼��Ϣ��ʼ��
	AirLineRead();
#endif
	

	// 4.��ʼ������
	if(FALSE == InitAnnotationFont(ModelFntPath))
	{
		return FALSE;

	}

	// 5.��������
	InitModelThread();
	
	return TRUE;
}

BOOL GetWhiteCityDrawOrNot(sGLRENDERSCENE* pModelScene)
{
	double whitecity_lonlat[2] = {120.4628625881- 16.530377, 22.6010411645 + 8.113};
	//�ӵ�߶ȴ���2000m����
	if (pModelScene->camParam.m_geoptEye.height > 5000)
	{
		return FALSE;
	}
	
	//��Բ�������2000m����
	if (2000 < caculateGreatCircleDis(pModelScene->camParam.m_geoptEye.lon, 
		pModelScene->camParam.m_geoptEye.lat, 
		whitecity_lonlat[0], whitecity_lonlat[1]))
	{
		return FALSE;
	}

	return TRUE;

}

BOOL CheckModelDrawByDisWithEye(sGLRENDERSCENE* pModelScene, int modelID)
{
	PT_3D modelpos = {0};

	//�ӵ�߶ȴ���2000m����
	if (pModelScene->camParam.m_geoptEye.height > 2000)
	{
		return FALSE;
	}

	GetModelPos(modelID,&modelpos);
	if (2000 < caculateGreatCircleDis(modelpos.y, 
		modelpos.x, 
		pModelScene->camParam.m_geoptEye.lon, 
		pModelScene->camParam.m_geoptEye.lat))
	{
		return FALSE;
	}

	return TRUE;

}

void SetAirportModelDisapear(sGLRENDERSCENE* pModelScene)
{
	

//	if (FALSE ==  CheckModelDrawByDisWithEye(pModelScene, modelid[0][7])
//		||(pModelScene->camctrl_param.view_type != eVM_COCKPIT_VIEW))
	if(0)
	{
		SetModelShield(modelid[0][7], TRUE);
		SetModelShield(modelid[0][8], TRUE);

	}
	else
	{
		SetModelShield(modelid[0][7], FALSE);
		SetModelShield(modelid[0][8], FALSE);
	}

}

/*.BH--------------------------------------------------------
**
** ������: BOOL RenderModel(BOOL SelectModel, sGLRENDERSCENE* pModelScene)
**
** ����:  ����ģ�ͣ�����ģʽ��ѡ��ģʽ
**
** �������:  SelectModel;			//TRUE-ѡ��ģ�͵Ļ��Ʒ�ʽ,FALSE-����ģʽ����ģ��
**            pModelScene           //��Ⱦ�������
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�2, used in sphereRender.c
**
**.EH--------------------------------------------------------
*/
static int testa,testb,testc, testd,teste;	//2017-2-16 11:49:31 ������
int testk = 1,testk2 = 1;
BOOL RenderModel(BOOL SelectModel, sGLRENDERSCENE* pModelScene)
{

	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;
	ModelRender	SingleModel = {0};	//�洢��ģ�����ݵĽṹ��

	double z0 = 0.0;
	double radius = 100.0;			//Ĭ��ģ�Ͱ뾶��100m
	double dis = 0.0;

	LPModelRender lpchildmodel = NULL;	//�洢��ģ�����ݵ�ָ��
	PT_3D model_world = {0};			//world x,y,z

	int i = 0;							//���Ƶ�ģ�͸���

#ifdef WIN32
	if(SelectModel == TRUE)
		glLoadName(0);  //�û������ƶ�ջ��ջ��ֵ��ѡ��������	
#endif

	// lpf add �رչ�Դ0,����άʱ�򿪹�,Ŀ�����ڷ�ֹ8860������Ӱ���Դ1
	EnableLight0(FALSE);

	testa = tickCountGet();
	if((testk == 1)&&(GetWhiteCityDrawOrNot(pModelScene) == TRUE))
	{
		// 1.���ư�ģ����
		RenderWhiteCity(pModelScene);					
	}

	testb = tickCountGet();

	// 2.1.�ж�ģ�ͳ�ʼ���Ƿ����
	if(FALSE == GetModelInitFlag())
	{
		return FALSE;
	}

	// 2.2.�жϿ����Ƿ���,ʼ�տ���
	if(CheckModelRender() == FALSE)  
	{
		return FALSE;
	}

	// 2.3.���÷ɻ�ģ�͵Ļ��Ʋ�����λ��Ϊ�ɻ�λ��
	RenderPlanes(pModelScene);			

	// 2.4.�жϻ����Ƿ����,��ʱδʵ��
	//SetAirportModelDisapear(pModelScene);

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 2.5.����ģ�ͻ�������,����ģ�͵�λ��	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
	
			lpsinglemodel->m_selectMode = SelectModel;

			// 2.5.1.���ݸ߶�ģʽ����ģ�͵ľ��Ը߶�
			if(lpsinglemodel->m_heimode == 1)		//��������ϵ�߶�
			{
				z0 = - EARTH_RADIUS;
			}
			else if(lpsinglemodel->m_heimode == 0)	//��ظ߶�
			{
				//�������غ��θ߶�	, lpf 2017-5-8 13:56:14 ��ʱע��
//				if(FALSE == GetZ(lpsinglemodel->m_modelPos.y, lpsinglemodel->m_modelPos.x,&z0))
				{
					z0 = 0.0;	//Ĭ�ϵ��ص�ĺ��θ߶�Ϊ0
				}
			}
			else if(lpsinglemodel->m_heimode == 2)	//���θ߶�
			{
				z0 = 0.0;
			}
			else		//�������,����ѭ��������
			{	
				break;
			}

			// 2.5.2.��γ������ת��Ϊ��������	
			{
				Geo_Pt_D geo_pt;

				geo_pt.lat = lpsinglemodel->m_modelPos.x;
				geo_pt.lon = lpsinglemodel->m_modelPos.y;
				geo_pt.height = z0 + lpsinglemodel->m_modelPos.z;
				
				geoDPt2objDPt(&geo_pt, &model_world);
			}
			
			// 2.5.3.�������е�ֵ�Ƚϣ���һ���洢����������־��1
			if((model_world.x != lpsinglemodel->m_modelPoxWorld.x)
				||(model_world.y != lpsinglemodel->m_modelPoxWorld.y)
				||(model_world.z != lpsinglemodel->m_modelPoxWorld.z))
			{
				lpsinglemodel->m_modelPoxWorld.x = model_world.x;
				lpsinglemodel->m_modelPoxWorld.y = model_world.y;
				lpsinglemodel->m_modelPoxWorld.z = model_world.z;
				lpsinglemodel->m_MatrixChange = TRUE;
			}

		}
	}
	
#if 1
	// 2.6.������ʾ��ʽ�ֱ����
 	glEnableEx(GL_TEXTURE_2D);
	//���ĳЩ��ά����Ʋ����������⣬Ĭ����Disable cull face
 	glDisableEx(GL_CULL_FACE);
	//������ȼ��
 	glEnableEx(GL_DEPTH_TEST);	

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// 2.6.1.��ʾ��ʽ�ǣ�0-������ʾ��1-������6-��ɫ����
	RenderModelType1(pModelScene);
	
	// 2.6.2.��ʾ��ʽ�ǣ�2-�߿�(������С)��11����(��11)-����Ч��,��δʵ��
	RenderModelType2(pModelScene);
	
	// 2.6.3.��ʾ��ʽ�ǣ�3-ģ��+�߿�(1.02����С)
	RenderModelType3(pModelScene);

	// 2.6.4.��ʾ��ʽ�ǣ�4-ģ��+������(������С,�ߴ�3��)
	RenderModelType4(pModelScene);

	// 2.6.5.��ʾ��λ�ǣ�5-�⻷Ч��,��ģ��+��ɫ(1.05����Сָ����ɫ)
	RenderModelType5(pModelScene);

	// 2.6.6.��ʾ��ʽ�ǣ�7-10��δ���壬��0-����ģʽ����
	RenderModelType6(pModelScene);

 	glDisableEx(GL_TEXTURE_2D);

	// 2.7.����Ĭ�ϵ��û�ͼ��,ѡ��ģʽ�²����Ƹ�ͼ��
	if(SelectModel == FALSE)
	{
		// 2.7.1.β��
  		RenderUserFlyTrace(pModelScene);

		// 2.7.2.����
  		RenderUserTxt(pModelScene);

		// 2.7.3.͸��������ͼ��
		RenderUserRectangle(pModelScene);
			
		// 2.7.4.�����û�������ͼ��
		RenderUserDiy(pModelScene);

	}
	
	//�ر�cull face
 	glDisableEx(GL_CULL_FACE);				
 	glDisableEx(GL_DEPTH_TEST);
#endif

#if 0
	// 2.8.�ָ�ģ�͵�m_drawOrNot Ϊ FALSE
	pNode = (LPNode)ModelRenderList.m_pFirst;
	while(pNode != NULL)
	{
		lpsinglemodel = (LPModelRender)pNode->m_pCur;
		
		lpsinglemodel->m_drawOrNot = FALSE;
	
		pNode = pNode->m_pNext;
	}
#endif	
	
	giveSem(g_SetModelParam);	

	testc = tickCountGet();
//	printf("b-a=%d,c-b=%d,c-a=%d, draw model num = %d\n",testb-testa,testc-testb,testc-testa,i);

#if 0
	if(tickCountGet() % 100 == 0)
	{
		printf("draw model num = %d\n",i);
	}	
#endif

	return TRUE;	
}


/*.BH--------------------------------------------------------
**
** ������: InitModelThread
**
** ����:  ��ʼ�����������ж�ģ���Ƿ���Ƶ�����
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿڣ���InitModel�н�������
**
**.EH--------------------------------------------------------
*/
void InitModelThread()
{
	int i = 0;
	char name_task[32] = {0}; 

	//ÿ���ӿ�����һ������
	for(i=0; i< SCENE_NUM; i++)
	{
		sprintf(name_task, "loadmodel%d",i);
		
		spawnTask(name_task, 103, FP_TASK_VX | NO_STACK_FILL_VX, 0x400000, LoadModelDataThreadFunc,  (f_int64_t)pScene[i]);

	}



}


/*.BH--------------------------------------------------------
**
** ������: LoadModelDataThreadFunc
**
** ����:  �ж�ģ���Ƿ���Ƶ�����,����ģ�Ͱ�Χ���Ƿ��ڳ������Ӿ����ڣ�������ƣ������򲻻���
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿڣ���InitModelThread�н�������
**
**.EH--------------------------------------------------------
*/
#ifdef WIN32
DWORD WINAPI LoadModelDataThreadFunc(sGLRENDERSCENE*  pModelScene)
#else
void LoadModelDataThreadFunc(sGLRENDERSCENE*  pModelScene)
#endif
{

	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;

	LPModelRender lpsinglemodel = NULL;
	double radius = 100.0;			//Ĭ��ģ�Ͱ뾶��100m
	double dis = 0.0;
	int i = 0;							//���Ƶ�ģ�͸���

	while(1)
	{
	
#ifdef WIN32

#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
		usleep(1000);
#else
		taskDelay(1);
#endif	
		i = 0;

		// 2.1.�ж�ģ�ͳ�ʼ���Ƿ����
		if(FALSE == GetModelInitFlag())
		{
			continue;
		}

		// 2.2.�жϿ����Ƿ���,ʼ�տ���
		if(CheckModelRender() == FALSE)  
		{
			continue;
		}
				
		takeSem(g_SetModelParam, FOREVER_WAIT);

		// 2.3.0.Ĭ��ģ�͵�m_drawOrNotΪFALSE
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

				lpsinglemodel->m_drawOrNot = FALSE;

			}
		}
		
		// 2.3.1 �жϻ���ģ�͵ĸ�������������Ʊ��	
		pstListHead = &(pModelScene->pScene_ModelList);

		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
				// 2.3.2.ģ�Ͱ�Χ��뾶=ģ�����ű��� * ģ�ͳߴ�
				radius = (lpsinglemodel->m_modelScale) * (lpsinglemodel->m_modelSize);

				// 2.3.3.�ж�ģ�Ͱ�Χ���Ƿ����Ӿ�����
				if (InFrustum(GetView_near(pModelScene), GetView_far(pModelScene),
							lpsinglemodel->m_modelPoxWorld.x, 
							lpsinglemodel->m_modelPoxWorld.y, 
							lpsinglemodel->m_modelPoxWorld.z, radius, GetFrustumModel(pModelScene), &dis) == TRUE)									
				{	
					//testk2��������ʹ��
					if(testk2 == 1)
					{		
						// 2.3.4.���ģ����Ҫ����
						lpsinglemodel->m_drawOrNot = TRUE;				
						i++;			
					}

				}
			}
		}
		
		giveSem(g_SetModelParam);	


		if(tickCountGet() % 1000 == 0)
		{
			//printf("draw model num = %d\n",i);
		}
	
	}

}


/*.BH--------------------------------------------------------
**
** ������: AttachpScene2Model
**
** ����:  �ѳ����������ͷ�ڵ㸳ֵ��modelapi �ľֲ�����
**
** �������:  ��������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�3���û������
**
**.EH--------------------------------------------------------
*/
void AttachpScene2Model(sGLRENDERSCENE* pModelScene)
{
	pModelRenderList = &(pModelScene->pScene_ModelList);

	return;
}


/*.BH--------------------------------------------------------
**
** ������: NewModel
**
** ����:  ����ģ����ʾ������������ӽڵ㣬������ID
**
** �������:  modeltype ģ������id
**
** �����������
**
** ����ֵ��ģ��ID
**          
**
** ���ע��:  �ⲿ�ӿ�3���û������
**
**.EH--------------------------------------------------------
*/
f_int32_t NewModel(f_int32_t modeltype)
{
	LPModelRender pData = NULL;

	LPModelAll pData3ds = NULL;

	f_int32_t i = 0;

	LPModelRender pChildData = NULL;

	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	unsigned char text_char[32] = {0}; 

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.1׼���ṹ�������, ����ģ�͵�ID
	// �����ڴ�ʱ����ڴ��������memset��������ģ�͵���ʾ����Ϊ0-������ʾ
	pData = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));	

	pData->m_modelID = (int)pData;					//ģ��ID,����ָ����ΪID
	pData->m_modeltype = modeltype;				    //ģ������
	pData->m_modelScale = 1.0;						//���ű���Ĭ����1��
	pData->m_modelFlag = MODEL_FLAG;				//ģ�͵ı�Ǹ�ֵ
	pData->m_MatrixChange = TRUE;
	pData->m_heimode = 0;							//ģ�͵ĸ߶�����Ĭ����0,��Ը߶�
	pData->m_drawOrNot = FALSE;						//�Ƿ���Ƹ�ģ�ͣ�Ĭ���ǲ�����

	pData->m_flagModelScreen = TRUE;				//ģ�͵�С��ͼ��ʾĬ�ϴ�
	pData->m_ModelScreenID = 13;					//ģ�͵�С��ͼ����Ĭ���������

	vector4DMemSet(&pData->m_color[0] , 0.0f, 1.0f, 0.0f, 1.0f);	//�߿�Ĭ����ɫ:��ɫ
	vector4DMemSet(&pData->m_color[1] , 1.0f, 0.0f, 0.0f, 1.0f);	//��Ļ��Χ����ɫ:��ɫ
	vector4DMemSet(&pData->m_color[2] , 0.72f, 0.36f, 0.18f,0.50f);	//�⻷ģʽ����ɫ:���ɫ
	vector4DMemSet(&pData->m_color[3] , 1.0f, 0.0f, 0.0f, 1.0f);	//��ɫģʽ����ɫ:��ɫ

	//�������ɴ�ɫ������ID�ı�־λ
	pData->m_colorPictureIsNeedRebuild = 1;
	
	// 1.2 ����3ds���ݣ�����type ѡ��ģ�Ͷ���
	pstListHead = &(m_listModelData);
		
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
	{
		if( NULL != pstTmpList )
		{
			pData3ds = ASDE_LIST_ENTRY(pstTmpList, ModelAll, stListHead);
			if(pData3ds->id == modeltype)
			{	
				break;
			}
		}
	}

	//�ж��Ƿ��������
	if(&(pData3ds->pModelMess) == NULL)
	{	
		DeleteAlterableMemory(pData);
		giveSem(g_SetModelParam);
		return FALSE;	
	}

	//����3ds ����
	pData->m_pModel3ds = &pData3ds->pModelMess;
	//����ģ�ͳߴ�
	pData->m_modelSize = pData3ds->model_size;
	
	//���֧��4������
	memset(pData->m_pString[0],0,sizeof(pData->m_pString[0]));
	memset(pData->m_stringNum,0,sizeof(pData->m_stringNum));
	//ģ��typeת��Ϊ���ַ������洢�ڵ�һ��
	sprintf(text_char, "%d",pData->m_modeltype);
	ttfAnsi2Unicode(pData->m_pString[0], text_char, &pData->m_stringNum[0]);

	sprintf(text_char, "%s", "�л�");
	ttfAnsi2Unicode(pData->m_pString[1], text_char, &pData->m_stringNum[1]);

	sprintf(text_char, "%s", "Σ�յȼ�");
	ttfAnsi2Unicode(pData->m_pString[2], text_char, &pData->m_stringNum[2]);

	sprintf(text_char, "%s", "����");
	ttfAnsi2Unicode(pData->m_pString[3], text_char, &pData->m_stringNum[3]);

	// 1.3 ��ȡ��ģ�͵��ӽڵ�(��ģ��)����
	pData->m_childNum = pData3ds->child_num;
	
	//������ģ�������ͷ�ڵ�
//	if(pData->m_childNum > 0)
	{
		CreateList(&pData->m_childlist);		
	}

	// 1.4 ������ģ�͵��ӽڵ�(��ģ��),����������
	for(i = 0; i<pData->m_childNum; i++)
	{

		// 1.4.1������ģ�͵ĳ�ʼ��
		// ׼���ṹ�������, ����ģ�͵�ID
		pChildData = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));

		memcpy(pChildData,pData,sizeof(ModelRender));

		pChildData->m_modelID = (int)pChildData;				    //ģ��ID,����ָ����ΪID
		pChildData->m_modeltype = pData3ds->child_id[i];			//ģ������
		//�ӽڵ��pos,angleĬ����0
		pChildData->m_modelPos.x = 0;
		pChildData->m_modelPos.y = 0;
		pChildData->m_modelPos.z = 0;
		
		pChildData->m_modelAngle.x = 0;
		pChildData->m_modelAngle.y = 0;
		pChildData->m_modelAngle.z = 0;

		pChildData->m_modelScale = 1;				//Ĭ��ֵ1����ֵΪ������ʵ��ֵ= ��ֵ*���ڵ�Ŵ���

		pChildData->m_MatrixChange = TRUE; 

		pChildData->m_childNum = 0;				//Ĭ��ֻ��һ����ģ��
		
		// 1.4.2 ����3ds ����
		pChildData->m_pModel3ds = pData3ds->child_3ds[i];

		// 1.4.3���ӽڵ����������
		{
		    /**�ҵ�ͷ���֮��**/
		 	stList_Head *pstListHead = NULL;
			pstListHead = &(pData->m_childlist);
			LIST_ADD(&pChildData->stListHead, pstListHead);	
	 	}
	
	}
	
	// 2.ģ�ͻ�����������ӽڵ�
	 {
	    /**�ҵ�ͷ���֮��**/
	 	stList_Head *pstListHead = NULL;
		pstListHead = (pModelRenderList);
		LIST_ADD(&pData->stListHead, pstListHead);	
	 }
	
	giveSem(g_SetModelParam);

	// 3.����ID
	return pData->m_modelID;
}
/*.BH--------------------------------------------------------
**
** ������: SetModelPos
**
** ����:  ����ģ�͵�lat, lon, height, ��λ: �Ƕ�, �Ƕ�, ��
**
** �������:  ģ�͵�ID��ģ�͵�λ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�4���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelPos(f_int32_t modelID, PT_3D latlonhei)
{
	int nodeID = 0;
//	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;
// 		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	int i = 0;
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		SetPT3D(latlonhei, &(lpsinglemodel->m_modelPos));

		// 3.���þ���ı��־λ
		lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
// 	DeleteAlterableMemory(lpsinglemodel);

	return ret;
}
/*.BH--------------------------------------------------------
**
** ������: SetModelAngle
**
** ����:  ����ģ�͵�yaw,pitch,roll.��λ�ǽǶ�
**
** �������:  ģ�͵�ID��ģ�͵ĽǶ�
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�5���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelAngle(f_int32_t modelID, PT_3D yawpitchroll)
{
	LPModelRender lpsinglemodel = NULL;
// 		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸��������
		SetPT3D(yawpitchroll, &lpsinglemodel->m_modelAngle);

		// 3.���þ���ı��־λ
		lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
// 	DeleteAlterableMemory(lpsinglemodel);

	return ret;
	
}
/*.BH--------------------------------------------------------
**
** ������: SetModelScale
**
** ����:  ����ģ�͵�����ϵ��
**
** �������:  ģ�͵�ID������ϵ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�6���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelScale(f_int32_t modelID, f_float64_t modelscale)
{
	LPModelRender lpsinglemodel = NULL;
// 		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸��������	
		lpsinglemodel->m_modelScale = modelscale;

		// 3.���þ���ı��־λ
		lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
// 	DeleteAlterableMemory(lpsinglemodel);

	return ret;
}
/*.BH--------------------------------------------------------
**
** ������: SetModelDisplay
**
** ����:  ����ģ�͵���ʾ��ʽ
**
** �������:  ģ�͵�ID����ʾ��ʽ ,0��������ʾ��1��������2���߿�(������С,��ɫĬ��Ϊ��ɫ)��
** 3��ģ��+�߿�(1.02����С,��ɫĬ��Ϊ��ɫ)��4��ģ��+������(������С,�ߴ�3��,��ɫĬ��Ϊ��ɫ)��
** 5���⻷Ч��,��ģ��+��ɫ(1.05����С,��ɫĬ��Ϊ���ɫ)��6����ɫ����(��ɫĬ��Ϊ��ɫ)
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�7���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelDisplay(f_int32_t modelID, f_int32_t modeldisplaytype)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸���ʾ��ʽ
		set_model_displaytype(lpsinglemodel, modeldisplaytype);

		// 3.���þ���ı��־λ
//		lpsinglemodel->m_MatrixChange = TRUE;
	}

//	DeleteAlterableMemory(lpsinglemodel);
	giveSem(g_SetModelParam);
	return ret;
	
}

/*.BH--------------------------------------------------------
**
** ������: SetModelColorNumuse
**
** ����:  ����ģ�͵���ʾ��ʽ
**
** �������:  ģ�͵�ID����ʾ��ʽ
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�7���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelColorNumuse(f_int32_t modelID, f_int32_t modelColorNum_use)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸���ʾ��ʽ
		set_model_colorNumUse(lpsinglemodel, modelColorNum_use);

		// 3.���þ���ı��־λ
//		lpsinglemodel->m_MatrixChange = TRUE;
	}

//	DeleteAlterableMemory(lpsinglemodel);

	giveSem(g_SetModelParam);
	return ret;	
}

/*.BH--------------------------------------------------------
**
** ������: SetModelShield
**
** ����:  ����ģ�͵����α�־
**
** �������:  ģ�͵�ID�����α�־flag
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�7���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelShield(f_int32_t modelID, BOOL shieldflag)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸���ʾ��ʽ
		set_model_shield(lpsinglemodel, shieldflag);
	}
	giveSem(g_SetModelParam);
	return ret;
}

/*.BH--------------------------------------------------------
**
** ������: SetModelUser
**
** ����:  ����ģ�͵��û�ͼ����ʾ��־,β�������֡������壬�Զ���
**
** �������:  ģ�͵�ID����ʾ��־flag����ʾͼ��mode
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�7���û������,�������û�ͼ��
**
**.EH--------------------------------------------------------
*/
BOOL SetModelUser(f_int32_t modelID, BOOL userflag, int mode)
{
	int nodeID = 0;
// 	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.���ҽڵ�����
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.�޸���ʾ��ʽ
	set_model_user(lpsinglemodel, userflag, mode);
	giveSem(g_SetModelParam);
	
	return TRUE;
}

BOOL SetModelUserAll(int modelID, BOOL userflag)
{
	int nodeID = 0;
// 	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;
	int i = 0;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.���ҽڵ�����
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.�޸���ʾ��ʽ
	for(i=0;i<3;i++)
	{
		set_model_user(lpsinglemodel, userflag, i);		
	}

	giveSem(g_SetModelParam);
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: SetModelScreenFlag
**
** ����:  ����ģ���Ƿ���С��ͼ����
**
** �������:  ģ�͵�ID����ʾ��־flag
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�7���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelScreenFlag(f_int32_t modelID, BOOL userflag)
{
	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.���ҽڵ�����
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.�޸���ʾ��ʽ
	lpsinglemodel->m_flagModelScreen = userflag;

	giveSem(g_SetModelParam);
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: SetModelScreenFlagType
**
** ����:  ����ģ����С��ͼ�ķ���ID����Ļ������ʾ��־����ʽ
**
** �������:  ģ�͵�ID����ʽtype
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�7���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelScreenFlagType(f_int32_t modelID, int  type)
{
	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.���ҽڵ�����
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.�޸���ʾ��ʽ
	lpsinglemodel->m_ModelScreenID = type;

	giveSem(g_SetModelParam);
	
	return TRUE;
}


/*.BH--------------------------------------------------------
**
** ������: search_modelByID
**
** ����:  ����ģ�͵��ڴ�ָ��
**
** �������:  ģ�͵�ID
**
** ����������ڴ�ָ��
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*
BOOL search_modelByID(f_int32_t modelID , LPModelRender* pModel_select)
{
	int nodeID = 0;
	LPNode pNode = NULL;
//	LPModelRender lpsinglemodel = NULL;
//	int i = 0;
	
	// 1.��λ���ڵ�λ��
	nodeID = SearchNodeIndex(modelID);
	if(nodeID == -1)
		{return FALSE;}

	// 2.��ȡ�ڵ����ݲ��޸�
	pNode = NodeAt(&ModelRenderList, nodeID);
	* pModel_select = (LPModelRender)pNode->m_pCur;

	return TRUE;
}
*/
/*.BH--------------------------------------------------------
**
** ������: set_model_displaytype
**
** ����:  ����ģ�͵���ʾ��ʽ
**
** �������:  ģ��ָ�룬��ʾ��ʽ
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
void set_model_displaytype(LPModelRender model_select,f_int32_t displaytype)
{
	f_int32_t i = 0;
	stList_Head *pstListHead = &(model_select->m_childlist);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;
	
	model_select->m_modelDisplaytype = displaytype;
	
//	for(i=0; i<model_select->m_childNum; i++)
//	{
//		model_select->m_childModel[i].m_modelDisplaytype = displaytype;
//	}


	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			lpsinglemodel->m_modelDisplaytype = displaytype;
		}
	}

}

/*.BH--------------------------------------------------------
**
** ������: set_model_colorNumUse
**
** ����:  ����ģ�͵���ɫʹ�����
**
** �������:  ģ��ָ�룬��ɫ���
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
BOOL set_model_colorNumUse(LPModelRender model_select,f_int32_t color_use)
{
	f_int32_t i = 0;
	stList_Head *pstListHead = &(model_select->m_childlist);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;

	if(color_use >= 4 )
	{
		printf("�Ƿ�����colorNum_use\n");
		return FALSE;
	}
	
	model_select->m_colorNum_use= color_use;
// 	for(i=0; i<model_select->m_childNum; i++)
// 	{
// 		model_select->m_childModel[i].m_colorNum_use= color_use;
// 	}

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			lpsinglemodel->m_colorNum_use = color_use;
			
		}
	}

	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: set_model_shield
**
** ����:  ����ģ�͵����α�־
**
** �������:  ģ��ָ�룬���α�־
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�, ֻ��Ҫ���ø��ڵ㼴��,����Ҫ�����ӽڵ�
**
**.EH--------------------------------------------------------
*/
BOOL set_model_shield(LPModelRender model_select, BOOL flag)
{
	model_select->m_flagshield = flag;

	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: set_model_user
**
** ����:  ����ģ�͵��û�ͼ����ʾ��־
**
** �������:  ģ��ָ�룬�û�ͼ����ʾ��־���ڼ�����ʾͼ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�, ֻ��Ҫ���ø��ڵ㼴��,����Ҫ�����ӽڵ�
**
**.EH--------------------------------------------------------
*/
BOOL set_model_user(LPModelRender model_select, BOOL flag, int mode)
{
	if(mode > 3)
	{
		return FALSE;
	}		
	
	model_select->m_flagUser[mode]= flag;

	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: DestroyModel
**
** ����:  ɾ��ģ�ͽڵ�
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�8���û������
**
**.EH--------------------------------------------------------
*/
BOOL DestroyModel(f_int32_t modelID)
{
	f_int32_t nodeID = 0;
	LPModelRender pData = NULL;
	int i = 0;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;

	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	
	// 1.��λ���ڵ�λ��
	nodeID = SearchNodeIndex(modelID,  &pData);
	if(nodeID == -1)
		{giveSem(g_SetModelParam);return FALSE;}


	// 2.��ȡ�ڵ����ݲ��޸�, ɾ���ӽڵ�����
	pstListHead = &(pData->m_childlist);

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			DestroyModel(lpsinglemodel->m_modelID);
		}
	}
	
	// 3.ɾ���ڵ�
	LIST_REMOVE(&(pData->stListHead));
	
	// �ͷ��ڴ�
	DeleteAlterableMemory(pData);

	giveSem(g_SetModelParam);
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: SetModelHeiMode
**
** ����:  ����ģ�͵ĸ߶�ֵ�Ǿ��Ը߶Ȼ�����Ը߶�
**
** �������:  ģ�͵�ID����Ը߶�0�����Ը߶�1, ���θ߶�2
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�9���û������
**
**.EH--------------------------------------------------------
*/
BOOL SetModelHeiMode(f_int32_t modelID,f_int32_t HeiMode)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸���ʾ��ʽ
		lpsinglemodel->m_heimode = HeiMode;

		// 3.���þ���ı��־λ
		//lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
//	DeleteAlterableMemory(lpsinglemodel);

	return ret;	
	
}

/*.BH--------------------------------------------------------
**
** ������: GetModelPos
**
** ����: ����id��ȡģ�͵�λ��
**
** �������:  ģ��id
**
** ���������ģ��λ��
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�10
**
**.EH--------------------------------------------------------
*/
BOOL GetModelPos(f_int32_t modelid, LP_PT_3D model_pos)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.����߶�
		model_pos->x = lpsinglemodel->m_modelPos.x;
		model_pos->y = lpsinglemodel->m_modelPos.y;
		model_pos->z = lpsinglemodel->m_modelPos.z;
	
	}

//	DeleteAlterableMemory(lpsinglemodel);

	return ret;	
	
}

/*.BH--------------------------------------------------------
**
** ������: GetModelType
**
** ����: ����id��ȡģ�͵�����
**
** �������:  ģ��id
**
** ���������ģ������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�21
**
**.EH--------------------------------------------------------
*/
BOOL GetModelType(f_int32_t modelid, f_int32_t *model_type)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�������
		*model_type = lpsinglemodel->m_modeltype;
	
	}

//	DeleteAlterableMemory(lpsinglemodel);

	return ret;	

}

/*.BH--------------------------------------------------------
**
** ������: SetModelPosAdd
**
** ����: �����ƶ�ģ��λ��
**
** �������:  ģ��id,ģ��λ������ֵ���ƶ��ĸ�����lat :0,lon:1,hei:2
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�11
**
**.EH--------------------------------------------------------
*/
BOOL SetModelPosAdd(f_int32_t modelid, f_float64_t model_dpos, f_int32_t mode)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸��������
		if(mode == 0)
			lpsinglemodel->m_modelPos.x += model_dpos;
		else if(mode == 1)
			lpsinglemodel->m_modelPos.y += model_dpos;
		else if(mode == 2)
			lpsinglemodel->m_modelPos.z += model_dpos;
		else
			return FALSE;

		// 3.���þ���ı��־λ
		lpsinglemodel->m_MatrixChange = TRUE;
	}

//	DeleteAlterableMemory(lpsinglemodel);
	giveSem(g_SetModelParam);
	return ret;
	
}

/*.BH--------------------------------------------------------
**
** ������: SetModelAngleAdd
**
** ����: �����ƶ�ģ�ͽǶ�
**
** �������:  ģ��id,ģ�ͽǶ�����ֵ���ƶ��ĸ�����yaw,0, pitch 1, roll 2
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�20
**
**.EH--------------------------------------------------------
*/
BOOL SetModelAngleAdd(f_int32_t modelid, f_float64_t model_dagnle, f_int32_t mode)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.�޸��������
		if(mode == 0)
			lpsinglemodel->m_modelAngle.x += model_dagnle;
		else if(mode == 1)
			lpsinglemodel->m_modelAngle.y += model_dagnle;
		else if(mode == 2)
			lpsinglemodel->m_modelAngle.z += model_dagnle;
		else
			return FALSE;

		// 3.���þ���ı��־λ
		lpsinglemodel->m_MatrixChange = TRUE;
		if (lpsinglemodel->m_modelAngle.x > 180)
		{
			lpsinglemodel->m_modelAngle.x = -180;
		}
	}

//	DeleteAlterableMemory(lpsinglemodel);
	giveSem(g_SetModelParam);

	return ret;	
}


/*.BH--------------------------------------------------------
**
** ������: SetModelColor
**
** ����: ����ģ�͵ĸ�����ʾģʽ����ɫ
**
** �������:  ģ��id��ģ����ʾģʽ����ɫ
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�12
**
**.EH--------------------------------------------------------
*/
BOOL SetModelColor(int modelid, int color_type, PT_4D colorset)
{
	int nodeID = 0;
//	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;

#if 	0
	// 1.��λ���ڵ�λ��
	nodeID = SearchNodeIndex(modelid);
	if(nodeID == -1)
		{return FALSE;}

	// 2.��ȡ�ڵ����ݲ��޸�
	pNode = NodeAt(&ModelRenderList, nodeID);
	lpsinglemodel = (LPModelRender)pNode->m_pCur;

	lpsinglemodel->m_color[type].x = colorset.x;
	lpsinglemodel->m_color[type].y = colorset.y;
	lpsinglemodel->m_color[type].z = colorset.z;
	lpsinglemodel->m_color[type].w = colorset.w;

#endif	

	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.���ҽڵ�����ָ��
	if(FALSE ==  SearchNodeIndex(modelid, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.������ɫ
	if(FALSE == set_model_color(lpsinglemodel, color_type, colorset))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	giveSem(g_SetModelParam);
	return TRUE;	
}

/*.BH--------------------------------------------------------
**
** ������: SetModelText
**
** ����: ����ģ�͵ĸ��е�����(��4��)
**
** �������:  ģ��id���ı��ַ������к�
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�12
**
**.EH--------------------------------------------------------
*/
BOOL SetModelText(f_int32_t modelID, unsigned char *textString, int line)
{

	LPModelRender lpsinglemodel = NULL;

	if( (NULL == textString) || strlen(textString)>16)
	{
		return FALSE;
	}

	if( (line<0) || (line>3) )
	{
		return FALSE;
	}

	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.���ҽڵ�����ָ��
	if(FALSE ==  SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.����ָ���ж�Ӧ���ı�
	memset(lpsinglemodel->m_pString[line],0,sizeof(lpsinglemodel->m_pString[line]));
	ttfAnsi2Unicode(lpsinglemodel->m_pString[line], textString, &lpsinglemodel->m_stringNum[line]);

	giveSem(g_SetModelParam);
	return TRUE;	
}

/*.BH--------------------------------------------------------
**
** ������: SetModelTraceFlag
**
** ����: ����ģ���Ƿ���ʾβ��
**
** �������:  ģ��id����־TRUE or FALSE
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�12
**
**.EH--------------------------------------------------------
*
BOOL SetModelTraceFlag(int modelid, BOOL flag)
{
	int nodeID = 0;
//	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.���ҽڵ�����ָ��
	if(FALSE ==  SearchNodeIndex(modelid, &lpsinglemodel))
	{
		return FALSE;
	}

	// 2.������ʾβ��
	lpsinglemodel->m_flagTrace = flag;

	return TRUE;	
}
*/


/*.BH--------------------------------------------------------
**
** ������: set_model_color
**
** ����:  ����ģ�͵���ɫ
**
** �������:  ģ��ָ�룬ģ����ɫ��ţ���ɫֵ
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
BOOL set_model_color(LPModelRender model_select,int color_type, PT_4D colorset)
{
	f_int32_t i = 0;
	stList_Head *pstListHead = &(model_select->m_childlist);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;

	if(color_type >= 4 )
	{
		printf("�Ƿ�����colorNum_use\n");
		return FALSE;
	}

	model_select->m_color[color_type].x = colorset.x;
	model_select->m_color[color_type].y = colorset.y;
	model_select->m_color[color_type].z = colorset.z;
	model_select->m_color[color_type].w = colorset.w;

// 	for(i=0; i<model_select->m_childNum; i++)
// 	{
// 		model_select->m_childModel[i].m_color[color_type].x = colorset.x;
// 		model_select->m_childModel[i].m_color[color_type].y = colorset.y;
// 		model_select->m_childModel[i].m_color[color_type].z = colorset.z;
// 		model_select->m_childModel[i].m_color[color_type].w = colorset.w;	
// 	}
	{
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

				lpsinglemodel->m_color[color_type].x = colorset.x;
				lpsinglemodel->m_color[color_type].y = colorset.y;
				lpsinglemodel->m_color[color_type].z = colorset.z;
				lpsinglemodel->m_color[color_type].w = colorset.w;	
				
			}
		}

	}

	//��ɫģʽ�½������������ɵı�־λ��1
	if(color_type == 3)
	{
		model_select->m_colorPictureIsNeedRebuild = 1;
	}
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: rebulid_model_colorPicture
**
** ����:  ����ģ�͵Ĵ�ɫģʽ�µ�����ID
**
** �������:  ģ��ָ�룬ģ����ɫ��ɫֵ
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
BOOL rebulid_model_colorPicture(LPModelRender model_select,PT_4D colorset)
{
	f_int32_t i = 0;
	f_uint16_t width = 1;
	f_uint16_t height = 1;
	f_uint8_t pImageData[4] = {0};

	stList_Head *pstListHead = &(model_select->m_childlist);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;	

	/** ����λͼ */
	pImageData[0] = colorset.x * 255;		//float -> char
	pImageData[1] = colorset.y * 255;
	pImageData[2] = colorset.z * 255;
	pImageData[3] = colorset.w * 255;

	if(model_select->m_colorPicture != 0)
	{
//		glDeleteTextures(1, &model_select->m_colorPicture);

		glDeleteTextures(1, (const GLuint *)&model_select->m_colorPicture);
		model_select->m_colorPicture = 0;
	}

	glGenTextures(1, &model_select->m_colorPicture);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, model_select->m_colorPicture);

	/** �����˲� */
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	/** ��������,����Ϊ1*1�Ķ�ά���� */
//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, 
//					  GL_UNSIGNED_BYTE, pImageData);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);

//	for(i=0; i<model_select->m_childNum; i++)
//	{
//		model_select->m_childModel[i].m_colorPicture = model_select->m_colorPicture;
//	}

// 	while(pNode != NULL)
// 	{
// 		lpsinglemodel = (LPModelRender)pNode->m_pCur;
// 
// 		lpsinglemodel->m_colorPicture = model_select->m_colorPicture;
// 
// 
// 		pNode = pNode->m_pNext;
// 	}

	/** ��ģ�͵���ģ�ʹ�ɫ�����븸ģ��һ�� */
	if(model_select->m_childNum > 0)
	{

		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

				lpsinglemodel->m_colorPicture = model_select->m_colorPicture;		
				
			}
		}

	}


	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: GetModelScale
**
** ����: ����id��ȡģ�͵�����ϵ��
**
** �������:  ģ��id
**
** �������������ϵ��
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�13
**
**.EH--------------------------------------------------------
*/
BOOL GetModelScale(f_int32_t modelid, f_float64_t* model_scale)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//���ҵ�ģ������ָ��
	BOOL ret = FALSE;

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.����߶�
		* model_scale = lpsinglemodel->m_modelScale;
	
	}
	return TRUE;
}
#if 0
/*.BH--------------------------------------------------------
**
** ������: NewChildModelDynamic
**
** ����:  ��̬������ģ��
**
** �������:  ��ģ�͵�ID,��ģ�͵�ID
**
** �����������
**
** ����ֵ��TRUE OR FALSE
**          
**
** ���ע��:  �ⲿ�ӿ�14���û������
**
**.EH--------------------------------------------------------
*/
BOOL  NewChildModelDynamic(f_int32_t parent_id,  f_int32_t child_id)
{
	int nodeID_parent = 0;
	int nodeID_child = 0;
	
	LPNode pNode_parent = NULL;
	LPNode pNode_child = NULL;
	LPNode pNode = NULL;
	
	LPModelRender lpsinglemodel_parent = NULL;
	LPModelRender lpsinglemodel_child = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	LPModelRender pDataTemp = NULL;

	PT_3D zero_p = {0};

	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.��λ���ڵ�λ��
	nodeID_parent = SearchNodeIndex(parent_id);
	nodeID_child = SearchNodeIndex(child_id);
	if((nodeID_parent == -1)||(nodeID_child == -1))
		{giveSem(g_SetModelParam);return FALSE;}

	// 2.��ȡ���ڵ�����
	pNode_parent = NodeAt(&ModelRenderList, nodeID_parent);
	lpsinglemodel_parent = (LPModelRender)pNode_parent->m_pCur;

	pNode_child = NodeAt(&ModelRenderList, nodeID_child);
	lpsinglemodel_child = (LPModelRender)pNode_child->m_pCur;

	// 3.���ӽڵ����ݹҽӵ����ڵ���
	
	// 3.1�ȴ洢���е��ӽڵ���Ϣ
/*
	pDataTemp = (LPModelRender)NewAlterableMemory(sizeof(ModelRender) * lpsinglemodel_parent->m_childNum);
	memcpy(pDataTemp, lpsinglemodel_parent->m_childModel, sizeof(ModelRender) * lpsinglemodel_parent->m_childNum);

	DeleteAlterableMemory(lpsinglemodel_parent->m_childModel);

	// 3.2�����ӽڵ���ڴ�ռ�
	lpsinglemodel_parent->m_childModel = (LPModelRender)NewAlterableMemory(sizeof(ModelRender) * (lpsinglemodel_parent->m_childNum+1));
	
	// 3.3�����ӽڵ�����
	memcpy(lpsinglemodel_parent->m_childModel, pDataTemp, sizeof(ModelRender) * lpsinglemodel_parent->m_childNum);
	memcpy(&lpsinglemodel_parent->m_childModel[lpsinglemodel_parent->m_childNum], lpsinglemodel_child, sizeof(ModelRender));	
*/
	// 3.1-3.3,����Ϊ�ӽڵ�����
	lpsinglemodel = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));
	memcpy(lpsinglemodel, lpsinglemodel_child, sizeof(ModelRender) );
	
	pNode = (LPNode)NewAlterableMemory(sizeof(Node));
	InitNode(pNode);

	pNode->m_pCur = lpsinglemodel;
	PushBack(&lpsinglemodel_parent->m_childlist, pNode);

	// 3.4�����ӽڵ�ĸ���
	lpsinglemodel_parent->m_childNum += 1;	

	// 4.ɾ�������е��ӽڵ���Ϣ
	if(FALSE == DestroyModel(lpsinglemodel_child->m_modelID))
		{giveSem(g_SetModelParam);return FALSE;}

	// 5.�����ӽڵ�����pos, angle, scale
	//m_nchildNum -1 ����������

	if(FALSE == SetChildModelPos(lpsinglemodel_parent->m_modelID, lpsinglemodel_parent->m_childNum-1, zero_p))
		{giveSem(g_SetModelParam);return FALSE;}
	if(FALSE == SetChildModelAngle(lpsinglemodel_parent->m_modelID, lpsinglemodel_parent->m_childNum-1, zero_p))
		{giveSem(g_SetModelParam);return FALSE;}
	//�Ŵ����ָ�Ϊ1
//	lpsinglemodel_parent->m_childModel[lpsinglemodel_parent->m_childNum-1].m_modelScale = 1;
	if(FALSE == SetChildModelScale(lpsinglemodel_parent->m_modelID, lpsinglemodel_parent->m_childNum-1, 1))
		{giveSem(g_SetModelParam);return FALSE;}
	
	giveSem(g_SetModelParam);
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: DeleteChildModelDynamic
**
** ����:  ��̬ɾ����ģ��
**
** �������:  ��ģ�͵�id,��ģ�͵����к�
**
** �����������ģ��id
**
** ����ֵ��TRUE OR FALSE
**          
**
** ���ע��:  �ⲿ�ӿ�14���û������
**
**.EH--------------------------------------------------------
*
BOOL DeleteChildModelDynamic(f_int32_t parent_id, f_int32_t child_number, f_int32_t* child_id)
{
	int nodeID = 0;
	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;
	int i = 0;

	LPModelRender pDataChild = NULL;
	LPNode pNodeChild = NULL;

	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.��λ���ڵ�λ��
	nodeID = SearchNodeIndex(parent_id);
	if(nodeID == -1)
		{giveSem(g_SetModelParam);return FALSE;}

	// 2.��ȡ�ڵ�����
	pNode = NodeAt(&ModelRenderList, nodeID);
	lpsinglemodel = (LPModelRender)pNode->m_pCur;

	// 4.���ӽڵ�ṹ����ӵ�����������
	pDataChild = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));
//	memcpy(pDataChild, &lpsinglemodel->m_childModel[child_number], sizeof(ModelRender));

	{
		LPNode pNode_child = (LPNode)lpsinglemodel->m_childlist.m_pFirst;
		LPModelRender lpsinglemodel_child = NULL;

		while(pNode_child != NULL)
		{
			lpsinglemodel_child = (LPModelRender)pNode_child->m_pCur;

			if(i == child_number)
			{
				memcpy(pDataChild, lpsinglemodel_child, sizeof(ModelRender));	
				break;
			}
				
			pNode_child = pNode_child->m_pNext;
			i ++;
		}
	
	}
	
	pNodeChild = (LPNode)NewAlterableMemory(sizeof(Node));
	InitNode(pNodeChild);
	
	pNodeChild->m_pCur = pDataChild;
	PushBack(&ModelRenderList, pNodeChild);	

	// 5.ɾ���ӽڵ�
	DeleteNode(&lpsinglemodel->m_childlist, child_number);
	
//	for(i=child_number+1; i <lpsinglemodel->m_childNum; i++)
//	{
//		lpsinglemodel->m_childModel[i-1] = lpsinglemodel->m_childModel[i]; 
//	}

	// 3.�����ӽڵ����
	lpsinglemodel->m_childNum -= 1;

	// 6.����ӽڵ��ID
	*child_id = pDataChild->m_modelID; 

	// 7.�����½ڵ��pos, angle, scale
	if(FALSE == SetModelPos(pDataChild->m_modelID, lpsinglemodel->m_modelPos))
		{giveSem(g_SetModelParam);return FALSE;}
	if(FALSE == SetModelAngle(pDataChild->m_modelID, lpsinglemodel->m_modelAngle))
		{giveSem(g_SetModelParam);return FALSE;}
	//�Ŵ���Ҳ����Ϊ���ڵ�ͬ����С
	if(FALSE == SetModelScale(pDataChild->m_modelID, lpsinglemodel->m_modelScale))
		{giveSem(g_SetModelParam);return FALSE;}

	giveSem(g_SetModelParam);
	return TRUE;
}
*/
#endif
/*.BH--------------------------------------------------------
**
** ������: SetChildModelPos
**
** ����:  �����ӽڵ�����λ��
**
** �������:  ��ģ�͵�id,�ӽڵ���ţ���� pos
**
** �����������
**
** ����ֵ���ɹ�or ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�15���û������
**
**.EH--------------------------------------------------------
*/
BOOL  SetChildModelPos(f_int32_t modelid, f_int32_t childmodelID, PT_3D xyz)
{
	LPModelRender lpsinglemodel = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel_child = NULL;
	BOOL ret = FALSE;
	int i = 0;

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.�Ƿ��ж�
	if(childmodelID >= lpsinglemodel->m_childNum)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 3.
	pstListHead = &(lpsinglemodel->m_childlist);
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel_child = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			if (i == childmodelID)
			{
				// 3.�޸��ӽڵ����� 
				SetPT3D(xyz, &(lpsinglemodel_child->m_modelPos) );

				// 4.���þ���ı��־
				lpsinglemodel_child->m_MatrixChange = TRUE;
			}


			i ++;

		}
	}

	giveSem(g_SetModelParam);
	return TRUE;	
}

/*.BH--------------------------------------------------------
**
** ������: SetChildModelAngle
**
** ����:  �����ӽڵ����ԽǶ�
**
** �������:  ��ģ�͵�id,�ӽڵ���ţ�PT_3D  angle
**
** �����������
**
** ����ֵ���ɹ�or ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�16���û������
**
**.EH--------------------------------------------------------
*/
BOOL  SetChildModelAngle(f_int32_t modelid, f_int32_t childmodelID, PT_3D yawpitchroll)
{
	LPModelRender lpsinglemodel = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel_child = NULL;
	BOOL ret = FALSE;
	int i = 0;

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.�Ƿ��ж�
	if(childmodelID >= lpsinglemodel->m_childNum)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 3.

	pstListHead = &(lpsinglemodel->m_childlist);
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel_child = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			if (i == childmodelID)
			{
				// 3.�޸��ӽڵ����� 
				SetPT3D(yawpitchroll, &(lpsinglemodel_child->m_modelAngle) );
				
				// 4.���þ���ı��־
				lpsinglemodel_child->m_MatrixChange = TRUE;
	

			}
			
			
			i ++;
			
		}
	}
	
	giveSem(g_SetModelParam);
	return TRUE;	









}

/*.BH--------------------------------------------------------
**
** ������: SetChildModelPosAdd
**
** ����: �����ƶ���ģ��λ��
**
** �������:  ģ��id,ģ��λ������ֵ���ƶ��ĸ�����lat :0,lon:1,hei:2
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�17
**
**.EH--------------------------------------------------------
*/
BOOL SetChildModelPosAdd(f_int32_t modelid, f_int32_t childmodelID, f_float64_t model_dpos, f_int32_t mode)
{
	LPModelRender lpsinglemodel = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel_child = NULL;
	BOOL ret = FALSE;
	int i = 0;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	
	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 2.�Ƿ��ж�
	if(childmodelID >= lpsinglemodel->m_childNum)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 3.
	pstListHead = &(lpsinglemodel->m_childlist);

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel_child = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			if (i == childmodelID)
			{
				// 3.�޸��ӽڵ����� ��ֵ
				if(mode == 0)
					lpsinglemodel_child->m_modelPos .x += model_dpos;
				else if(mode == 1)
					lpsinglemodel_child->m_modelPos .y += model_dpos;
				else if(mode == 2)
					lpsinglemodel_child->m_modelPos .z += model_dpos;
				else
					return FALSE;
				
				// 4.���þ���ı��־
				lpsinglemodel_child->m_MatrixChange = TRUE;
			}
			
			
			i ++;
			
		}
	}
	
	giveSem(g_SetModelParam);
	return TRUE;	
	
}

/*.BH--------------------------------------------------------
**
** ������: SetChildModelAngleAdd
**
** ����: �����ƶ���ģ�ͽǶ�
**
** �������:  ģ��id,ģ�ͽǶ�����ֵ���ƶ��ĸ�����yaw 0, pitch 1,roll 2
**
** ���������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��: �ⲿ�ӿ�18
**
**.EH--------------------------------------------------------
*/
BOOL SetChildModelAngleAdd(f_int32_t modelid, f_int32_t childmodelID, f_float64_t model_dangle, f_int32_t mode)
{
	LPModelRender lpsinglemodel = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel_child = NULL;
	BOOL ret = FALSE;
	int i = 0;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	
	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 2.�Ƿ��ж�
	if(childmodelID >= lpsinglemodel->m_childNum)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 3.
	pstListHead = &(lpsinglemodel->m_childlist);

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel_child = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			if (i == childmodelID)
			{
				// 3.�޸��ӽڵ����� ��ֵ
				if(mode == 0)
					lpsinglemodel_child->m_modelAngle.x += model_dangle;
				else if(mode == 1)
					lpsinglemodel_child->m_modelAngle .y += model_dangle;
				else if(mode == 2)
					lpsinglemodel_child->m_modelAngle .z += model_dangle;
				else
					return FALSE;
				
				// 4.���þ���ı��־
				lpsinglemodel_child->m_MatrixChange = TRUE;
			}
			
			
			i ++;
			
		}
	}
	
	giveSem(g_SetModelParam);
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: SetChildModelScale
**
** ����:  �����ӽڵ�ķŴ���
**
** �������:  ��ģ�͵�id,�ӽڵ���ţ��Ŵ���scale
**
** �����������
**
** ����ֵ���ɹ�or ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�15���û������
**
**.EH--------------------------------------------------------
*/
BOOL  SetChildModelScale(f_int32_t modelid, f_int32_t childmodelID, f_float64_t scale)
{
	LPModelRender lpsinglemodel = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel_child = NULL;
	BOOL ret = FALSE;
	int i = 0;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	
	// 1.��ȡ�ڵ�����
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 2.�Ƿ��ж�
	if(childmodelID >= lpsinglemodel->m_childNum)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 3.
	pstListHead = &(lpsinglemodel->m_childlist);
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel_child = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			if (i == childmodelID)
			{
				// 3.�޸��ӽڵ����� 
				lpsinglemodel_child->m_modelScale = scale;
				
				// 4.���þ���ı��־
				lpsinglemodel_child->m_MatrixChange = TRUE;
			}
			
			
			i ++;
			
		}
	}
	
	giveSem(g_SetModelParam);
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: SetChildNum
**
** ����:  �����ӽڵ�ĸ���
**
** �������:  ��ģ�͵�id���ӽڵ����
**
** �����������
**
** ����ֵ��NONE
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*
BOOL  SetChildNum(f_int32_t modelid, f_int32_t num)
{
	int nodeID = 0;
	LPNode pNode = NULL;
	LPNode pNode2 = NULL;
	LPModelRender lpsinglemodel = NULL;
	LPModelRender pData = NULL;

	// 1.��λ���ڵ�λ��
	nodeID = SearchNodeIndex(modelid);
	if(nodeID == -1)
		{return FALSE;}

	// 2.��ȡ�ڵ�����
	pNode = NodeAt(&ModelRenderList, nodeID);
	lpsinglemodel = (LPModelRender)pNode->m_pCur;

	// 3.�����ӽڵ����� 
	lpsinglemodel->m_childModel
		= (LPModelRender)NewAlterableMemory(num *  sizeof(ModelRender));
	lpsinglemodel->m_childNum = num;
	
	return TRUE;
}
*/

/*.BH--------------------------------------------------------
**
** ������: CreateNew3Number
**
** ����:  ������λû���ظ����������1-999
**
** �������:  ��
**
** �����������
**
** ����ֵ��3λ�������
**          
**
** ���ע��:  �ڲ����ߺ���1
**
**.EH--------------------------------------------------------
*
f_int32_t CreateNew3Number()
{
	LPNode pNode = (LPNode)ModelRenderList.m_pFirst;
	LPModelRender lpsinglemodel = NULL;

     	int RANGE_MIN = 1;
     	int RANGE_MAX = 999;
 	int randnumber = 0;

	BOOL Repeat_FLAG = FALSE;		//Ĭ����û���ظ�������

	while(1)
	{
		// 1. ������λ�������
	       randnumber = (((double) rand() / (double) RAND_MAX) * RANGE_MAX + RANGE_MIN);

		// 2.�ж��Ƿ��ظ�
		while(pNode != NULL)
		{
			lpsinglemodel = (LPModelRender)pNode->m_pCur;
			if( lpsinglemodel->m_modelID == randnumber)
			{
				Repeat_FLAG = TRUE;
			}

			pNode = pNode->m_pNext;
		}

		// 3.����Repeat_FLAG �ж��Ƿ��������������
		if(Repeat_FLAG == FALSE)
		{
			break;
		}
		
	}

	// 4.���������
	return randnumber;
}
*/
/*.BH--------------------------------------------------------
**
** ������: SearchNodeIndex
**
** ����:  ����ģ�������еĽڵ�ţ�����ģ�͵��ڴ��ַ
**
** �������:  ģ�͵�ID
**
** ���������ģ�͵��ڴ�ָ��
**
** ����ֵ��TRUE or FALSE
**          
**
** ���ע��:  �ڲ����ߺ���2
**
**.EH--------------------------------------------------------
*/
f_int32_t SearchNodeIndex(f_int32_t modelID, LPModelRender *pModelOut)
{

#if 0
	LPNode pNode = (LPNode)ModelRenderList.m_pFirst;
	LPModelRender lpsinglemodel =NULL;
	int node_i = 0;

	// 1.���ҽڵ�node_i
	while(pNode != NULL)
	{
		lpsinglemodel = (LPModelRender)pNode->m_pCur;
	
		if( lpsinglemodel->m_modelID == modelID)
		{
			break;
		}
		
		pNode = pNode->m_pNext;
		node_i ++;
	}

	// 2.����node_i
	if(ModelRenderList.m_nSize == node_i)
	{
		return -1;
	}
	else
	{
		return node_i;
	}
#endif
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LPModelRender  pNode = NULL;

	pstListHead = (pModelRenderList);
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			if(pNode->m_modelID == modelID)
			{
				*pModelOut = pNode;
				return TRUE;
//				break;
			}

		}
	}	

	return FALSE;
}

/*.BH--------------------------------------------------------
**
** ������: SetPT3D
**
** ����:  3ά���ݸ�ֵ
**
** �������:  PT_3D
**
** ���������PT_3D
**
** ����ֵ��NONE
**          
**
** ���ע��:  �ڲ����ߺ���3
**
**.EH--------------------------------------------------------
*/
void SetPT3D(PT_3D vec3din, LP_PT_3D vec3dout)
{
	vec3dout->x = vec3din.x;
	vec3dout->y = vec3din.y;
	vec3dout->z = vec3din.z;
}

/*.BH--------------------------------------------------------
**
** ������: CheckModelcollision
**
** ����:  �ж�ģ���Ƿ���������ײ
**
** �������:  PT_3D �������꣬double ��뾶
**
** �����������
**
** ����ֵ��ģ��id ��-1
**          
**
** ���ע��:  �ⲿ�ӿ�9,lpf delete û��ʹ��
**
**.EH--------------------------------------------------------
*/
/*
 *	���Ƿ�������
 *	������
 *			pPtPos �������ά����
 *			pPtSphereCenter ��������
 *			sphere_radius	��뾶
 */
BOOL IsPointInSphere(LP_PT_3D pPtPos, LP_PT_3D pPtSphereCenter, f_float64_t sphere_radius)
{

	PT_3D ptDis;
	vector3DSub(&ptDis, pPtPos, pPtSphereCenter);
	if(sqr(ptDis.x) + sqr(ptDis.y) + sqr(ptDis.z) < sqr(sphere_radius))
		return TRUE;
	else
		return FALSE;
}

f_int32_t CheckModelcollision(PT_3D collisionPoint, f_float64_t collisionRadius)
{
//	LPNode pNode = (LPNode)ModelRenderList.m_pFirst;
	stList_Head *pstListHead = (pModelRenderList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel =NULL;
	f_float64_t pZ = 0;

	// 1.���ҽڵ�
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			//GetZ(lpsinglemodel->m_modelPos.y, lpsinglemodel->m_modelPos.x, &pZ);		//temp lpf delete 2017-6-6 9:10:44

			
// 			EarthToXYZ(lpsinglemodel->m_modelPos.y * DE2RA, 
// 					lpsinglemodel->m_modelPos.x * DE2RA, 
// 					lpsinglemodel->m_modelPos.z + pZ, 
// 					&lpsinglemodel->m_modelPoxWorld.x,
// 					&lpsinglemodel->m_modelPoxWorld.y,
// 					&lpsinglemodel->m_modelPoxWorld.z);

			{
				Geo_Pt_D geo_pt;
				
				geo_pt.lat = lpsinglemodel->m_modelPos.x;
				geo_pt.lon = lpsinglemodel->m_modelPos.y;
				geo_pt.height = pZ+lpsinglemodel->m_modelPos.z;
				
				geoDPt2objDPt(&geo_pt, &lpsinglemodel->m_modelPoxWorld);
			}






			collisionRadius += lpsinglemodel->m_modelScale * 40;	//ģ�ʹ�С��Ϊ40

			if(IsPointInSphere(&lpsinglemodel->m_modelPoxWorld, &collisionPoint, collisionRadius) == TRUE)
			{
				return lpsinglemodel->m_modelID;
			}			
		}
	}
	// 2.����û����ײ
	return -1;
}

/*.BH--------------------------------------------------------
**
** ������: CheckModelcollisionScreen
**
** ����:  �ж�ģ����Ļ�����Ƿ�����Բ��ײ
**
** �������:  PT_3D Բ�������꣬double Բ�뾶
**
** �����������
**
** ����ֵ��ģ��id ��-1
**          
**
** ���ע��:  �ⲿ�ӿ�9
**
**.EH--------------------------------------------------------
*/
f_int32_t CheckModelcollisionScreen(PT_3D collisionPoint, f_float64_t collisionRadius, sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel =NULL;
//	f_float64_t pZ = 0;
	double screenX,screenY;
	double Point2CollisionSqr;
	double CollisionRadiusCal = 0.0;
	double Point2Eye = 0.0;

	PT_3D Eye2Center, Eye2Model;
	double CenterEyeModel = 0.0;

	double FNearModel = 0.0;

	int i = 0, ret = 0;

	double M2Pixl = 0;
	

	//����ѡ������飬�洢:modelID,  model��Ļ��������Ļ�Ӵ���ľ���
	int MaxSecondSelectNum = 	GetNodeNum(pstListHead);
	double * pSecondSelect = (double *)malloc(sizeof(double) * 2 * MaxSecondSelectNum);	
	double MinModelID = 0; double MinModelLength = 0;	//������С��ģ��ID��ģ�;���
	
	memset(pSecondSelect, 0 , sizeof(double) * 2 * MaxSecondSelectNum);
// 	M2Pixl =  
// 		2*g_SphereRender.m_Render.m_lfNear* tan(g_Fovy / 2 * DE2RA) /						//��������ʹ�ã����ó��ԵĲ���
// 	(g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom);		//���ý����ϵ�1�����ض�Ӧ����

	
	// 1.���ҽڵ�
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);


			//��γ��ת��Ļ����
			worldXYZ2screenXY(lpsinglemodel->m_modelPoxWorld, &screenX, &screenY);

			//�����ж�ģ�͵��Ƿ�����ĻԲ��
			//ģ�͵���Բ�ĵ���Ļ����
			Point2CollisionSqr = (screenX - collisionPoint.x)*(screenX - collisionPoint.x) +
							(screenY- collisionPoint.y)*(screenY- collisionPoint.y);

			//�ӵ��ģ�͵��������
			Point2Eye = 
				DistanceTwoPoints(&lpsinglemodel->m_modelPoxWorld, &pModelScene->camParam.m_ptEye);


			//�ӵ㵽���ĵ����ӵ㵽ģ�͵ļн�
			vector3DSub(&Eye2Center, &pModelScene->camParam.m_ptEye, &pModelScene->camParam.m_ptCenter);
			vector3DSub(&Eye2Model, &pModelScene->camParam.m_ptEye,   &lpsinglemodel->m_modelPoxWorld);
			
			CenterEyeModel = getVa2DVbAngle(&Eye2Center, &Eye2Model);

			//�ӵ㵽���ý�����ģ�ͷ����ϵľ���
			{
				double g_near = GetView_near(pModelScene);
//				FNearModel = g_SphereRender.m_Render.m_lfNear / cos(CenterEyeModel * DE2RA);	
				FNearModel = g_near / cos(CenterEyeModel * DE2RA);	
				
			


			//ģ�͵İ뾶�ڽ��ý����ϵ�ͶӰ����
//			CollisionRadiusCal = 
//				lpsinglemodel->m_modelScale * lpsinglemodel->m_modelSize *5*100/g_SphereRender.m_Render.m_lfNear	// 	/ ��ʹ�ô�M2Pixl�����þ������100,���ý�����1����ֵΪ100�����ý�����100����ֵ��1,�ƶϹ�ϵ100/Near��
//				* FNearModel
//				/Point2Eye;		//�ֶ�����4��


			CollisionRadiusCal = 
				lpsinglemodel->m_modelScale * lpsinglemodel->m_modelSize *5*100/g_near	// 	/ ��ʹ�ô�M2Pixl�����þ������100,���ý�����1����ֵΪ100�����ý�����100����ֵ��1,�ƶϹ�ϵ100/Near��
				* FNearModel
				/Point2Eye;		//�ֶ�����4��

			}
			//CollisionRadiusCal = lpsinglemodel->m_modelScale * lpsinglemodel->m_modelSize;

			//ģ�Ͱ뾶�Ƿ���ھ��룬ƽ��ֵ
			if(sqr(CollisionRadiusCal) > Point2CollisionSqr)
			{
				//���Ӷ���ѡ�񣬾��������ģ�ͱ�ѡ��
				//������ĻԲ
				printf("x=%f,y=%f,r=%f\n",screenX, screenY, CollisionRadiusCal);
#ifdef WIN32
//				SendScreenCircle(screenX, screenY, CollisionRadiusCal);
#endif
				pSecondSelect[2 * i] =  lpsinglemodel->m_modelID;
				pSecondSelect[2 * i +1] = Point2CollisionSqr;

				i ++;
			
				//return lpsinglemodel->m_modelID;
		}
		
	}
}

	// 2.����ѡ��, ���������ģ�ͱ�ѡ��
	i = 0;
	MinModelLength = pSecondSelect[2*i+1];
	MinModelID = pSecondSelect[2*i];
	
	for(i = 0; i < MaxSecondSelectNum; i++)
	{
		if(pSecondSelect[2*i] == 0)
		{
			continue;
		}

		if(MinModelLength >= pSecondSelect[2 * i +1] )
		{
			MinModelLength = pSecondSelect[2*i + 1];
			MinModelID = pSecondSelect[2*i];
		}
		
		
	}

	// 3.�жϷ���ֵ
	if(MinModelID == 0)
		ret = -1;
	else
		ret = MinModelID;
	
	// 3. �ͷ�ָ��
	free(pSecondSelect);
	pSecondSelect = NULL;
	
	// 4.����û����ײ������ײ��ģ��ID
	return ret;
}


/*.BH--------------------------------------------------------
**
** ������: ConvertChildPos
**
** ����:  ��ģ�͵����λ��ת��Ϊ����λ��
**
** �������:  PT_3D F_posXYZ, 		��ģ�͵�λ��
**				PT_3D F_anglePYR, 		��ģ�͵ĽǶ�
**				PT_3D C_pos, 			��ģ�͵����λ�ü�����λ��
**
** ���������PT_3D C_pos, 		��ģ�͵ľ���λ�� 
**
** ����ֵ��NONE
**          
**
** ���ע��:  �ڲ����ߺ���4,�������꣬�Ƕȵ�λ�Ƕ�
**
**.EH--------------------------------------------------------
*
void ConvertChildPos(PT_3D F_posXYZ, PT_3D F_anglePYR, PT_3D C_pos)
{
	Matrix44 matrix_convert = {0};
	Matrix44 matrix_1,matrix_2;

	//*�������ĵ�ľֲ����굽�����������*
	calcWorldMatrix44(F_posXYZ.x, F_posXYZ.y, F_posXYZ.z, &matrix_convert);

	// ����ת��
	PosPart2World(C_pos, 
					&matrix_1, 
					&matrix_2, 
					F_anglePYR.x , -F_anglePYR.y, F_anglePYR.z, 
					&matrix_convert);
	
}
*/
/*.BH--------------------------------------------------------
**
** ������: PosPart2World
**
** ����:  ����SetCameraFollow�����޸ĵõ�����ת������
**
** �������:  
**
** ���������PT_3D pos, PT_3D angle 
**
** ����ֵ��NONE
**          
**
** ���ע��:  �ڲ����ߺ���5���������꣬�Ƕȵ�λ�Ƕ�
**
**.EH--------------------------------------------------------
*
void PosPart2World(PT_3D ptCEye, PT_3D ptCCenter, PT_3D ptCUp,
					 f_float64_t pitch, f_float64_t yaw, f_float64_t roll, LPMatrix44 mx)
{
	PT_3D ptEarthCenter;
	PT_3D	ptEye, ptUp, ptUp0, ptTo;
	Matrix44 rmx;
	vector3DMemSet(&ptEarthCenter, 0 ,0 ,0);
	
	vector3DSetValue(&ptEye, &ptCEye);
	vector3DMemSet(&ptUp, 0, 1, 0);

	vector3DMemSet(&ptCCenter, mx->m[12], mx->m[13], mx->m[14]);
	dMatrix44RPYmemSet(&rmx, pitch, yaw, roll);
	dMatrix44Multi(&ptEye, &ptEye, &rmx);
	dMatrix44Multi(&ptUp0, &ptUp, &rmx);

	dMatrix44Multi(&ptCEye, &ptEye, mx);
	dMatrix44Multi(&ptCUp, &ptUp0, mx);

	vector3DSub(&ptCUp, &ptCUp, &ptCCenter);
	vector3FNormalize(&ptCUp);
	vector3DSub(&ptTo, &ptCCenter, &ptCEye);
	vector3FNormalize(&ptTo);

}
*/
/*.BH--------------------------------------------------------
**
** ������: ConvertChildAngle
**
** ����:  ��ģ�͵ĽǶ�ת��Ϊ���ԽǶ�
**
** �������:  PT_3D F_anglePYR, 		��ģ�͵ĽǶ�
**				 PT_3D C_angle,		��ģ�͵���ԽǶ�
**
** ���������PT_3D C_angleOut 		��ģ�͵ľ��ԽǶ�
**
** ����ֵ��NONE
**          
**
** ���ע��:  �ڲ����ߺ���6,��λ�Ƕ�
**
**.EH--------------------------------------------------------
*/
void ConvertChildAngle(PT_3D F_anglePYR, PT_3D C_angle,  LP_PT_3D C_angleOut)
{
	// �Ƕ�ת��
	C_angleOut->x = C_angle.x + F_anglePYR.x;
	C_angleOut->y = C_angle.y + F_anglePYR.y;
	C_angleOut->z = C_angle.z + F_anglePYR.z;

}

/*********************************���ƹ켣��******************************************/

/*
 *	���·��й켣
 */
BOOL UpdataFlyTrace(LPModelRender pmodelselect)
{
//	LPNode pNode = (LPNode)ModelRenderList.m_pFirst;
//	LPModelRender lpsinglemodel = NULL;

//	while(pNode != NULL)
	{
//		lpsinglemodel = (LPModelRender)pNode->m_pCur;

//		if(lpsinglemodel->m_flagTrace == FALSE)
//		{
//			pNode = pNode->m_pNext;
//			continue;
//		}
		if(pmodelselect->m_flagUser[0] == FALSE)
		{
			return FALSE;
		}

		{
			//if (pmodelselect->s_pTraceCount == 4)//ȡ��ÿ4֡��¼һ��
			{
				
				PT_3D ptUp, ptDown;
				f_int16_t index;
				pmodelselect->s_pTraceCount = 0;
//				MemSet_3D(&ptUp, 0, TRACE_HALF_HEGHT, 0);			//����
//				MemSet_3D(&ptDown, 0, -TRACE_HALF_HEGHT, 0);
				vector3DMemSet(&ptUp, TRACE_HALF_HEGHT, 0, 15);			//����, β����ɻ�����ƽ��15m
				vector3DMemSet(&ptDown, -TRACE_HALF_HEGHT, 0, 15);
				
				index = pmodelselect->s_pTraceIndex;
				if (pmodelselect->s_pTraceIndex == FLYTRACE_NODENUM_MAX - 1)
				{
					pmodelselect->s_pTraceIndex = 0;
				}
				else
				{
					pmodelselect->s_pTraceIndex++;
				}
				// ���ƽڵ㵽����
				// ���㵱ǰ�Ľڵ�λ��
					
					pmodelselect->m_vFlyTrace[index].m_bUse = TRUE;
					dMatrix44Multi(&ptUp, &ptUp, &pmodelselect->m_mxRotate);
					dMatrix44Multi(&ptDown, &ptDown, &pmodelselect->m_mxRotate);
					dMatrix44Multi(&pmodelselect->m_vFlyTrace[index].m_ptUp, &ptUp, &pmodelselect->m_mxWorld);
					dMatrix44Multi(&pmodelselect->m_vFlyTrace[index].m_ptDown, &ptDown, &pmodelselect->m_mxWorld);
			}
			pmodelselect->s_pTraceCount++;
		}

		
//		pNode = pNode->m_pNext;
	}

	return TRUE;
}

//����ʹ�õ�ģ�ͻ��ƺ���1����ʾ��ʽΪ��0-ֱ�ӻ���ģ��,1-ģ������,6-��ɫģʽ
void RenderModelType1(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	// �򿪹���,�ƶ�������仯֮��
//	EnableLight(TRUE);
//	EnableLight1(TRUE);
	
	// 1.ѭ��ģ�ͻ�������
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{				
				// 3.�ж���ʾ��ʽ�Ƿ�0-ֱ�ӻ���ģ��,1-ģ������
				if(	(lpsinglemodel->m_modelDisplaytype == 0)
						||(lpsinglemodel->m_modelDisplaytype == 1)
						/*||(lpsinglemodel->m_modelDisplaytype == 6)*/
						/*||((lpsinglemodel->m_modelDisplaytype >= 7)&&(lpsinglemodel->m_modelDisplaytype <= 10)*/
						)
				{
					//����ģ��
					RenderModelFeatureDynamic(lpsinglemodel, pModelScene);					
				}
				else if(lpsinglemodel->m_modelDisplaytype == 6) // 4.�ж���ʾ��ʽ�Ƿ�6-��ɫ����ģʽ
				{
					if(lpsinglemodel->m_colorPictureIsNeedRebuild == 1)
					{
						lpsinglemodel->m_colorPictureIsNeedRebuild = 0;
						rebulid_model_colorPicture(lpsinglemodel,lpsinglemodel->m_color[3]);		
					}
					//����ģ��
					RenderModelFeatureDynamic(lpsinglemodel, pModelScene);					

				}
		
			}
		
		}
	}	
	
	// �رչ���
	EnableLight(FALSE);
	EnableLight1(FALSE);
	
}

//����ʹ�õ�ģ�ͻ��ƺ���2����ʾ��ʽΪ��2-�߿�ģʽ,11����(��11)-����Ч��,��δʵ��
void RenderModelType2(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;	
	LPModelRender lpsinglemodel = NULL;
	
	// 1.ѭ������
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 3.�ж���ʾ��ʽ�Ƿ�2-�߿�ģʽ,11����(��11)-����Ч��,��δʵ��
				if(	(lpsinglemodel->m_modelDisplaytype == 2)
						||(lpsinglemodel->m_modelDisplaytype >= 11)
					)
				{
					RenderModelFeatureDynamic(lpsinglemodel, pModelScene);					
				}
		
			}
		}
	}	
}

//����ʹ�õ�ģ�ͻ��ƺ���3����ʾ��ʽΪ��3-ģ��+�߿�(1.02����С)��
void RenderModelType3(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);

	// �Ȱ�����ʾ��ʽ0����
	// �򿪹���
	EnableLight(TRUE);
	EnableLight1(TRUE);		

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
	
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{				
				// 3.�ж���ʾ��ʽ�Ƿ�3-ģ��+�߿�(1.02����С)
				if(	(lpsinglemodel->m_modelDisplaytype == 3) )
				{
					// 4.������ʾ��ʽ��0����
					set_model_colorNumUse(lpsinglemodel, 0);
					set_model_displaytype(lpsinglemodel, 0);	
					//ֱ�ӵ���ģ�ͻ��ƺ���
					RenderModelLLH(lpsinglemodel,pModelScene);
					//��������ʾģʽΪ3
					set_model_displaytype(lpsinglemodel, 3);								
				}	
			}
		}		
	}	
	// �رչ���
	EnableLight(FALSE);
	EnableLight1(FALSE);
	
	//�ٰ�����ʾ��ʽ2����
	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.�ж���ʾ��ʽ�Ƿ�3-ģ��+�߿�(1.02����С)
				if(	(lpsinglemodel->m_modelDisplaytype == 3) )
				{
					// 4.������ʾ��ʽ��2-�߿�ģʽ����,ģ�ͷŴ�1.02
					set_model_displaytype(lpsinglemodel, 2);				
					lpsinglemodel->m_modelScale *= 1.02;
					lpsinglemodel->m_MatrixChange = TRUE;
					RenderModelLLH(lpsinglemodel,pModelScene);		

					// 5.ģ��Ч����ԭΪ3, ��С��ԭ
					set_model_displaytype(lpsinglemodel, 3);	
					lpsinglemodel->m_modelScale *= (1/1.02);
					lpsinglemodel->m_MatrixChange = TRUE;
									
				}
			}
		}
	}	
	
}

//����ʹ�õ�ģ�ͻ��ƺ���4����ʾ��ʽΪ��4-ģ��+������(������С,�ߴ�3��)��
void RenderModelType4(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);

	//�Ȱ�����ʾ��ʽ2-�߿�ģʽ����,���߿�Ϊԭ����3��	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.0f);
	glDepthMask(GL_FALSE);			
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{				
				// 3.�ж���ʾ��ʽ�Ƿ�4-ģ��+������(������С,�ߴ�3��)
				if(	(lpsinglemodel->m_modelDisplaytype == 4) )
				{
					// 4.������ʾ��ʽ��4����
					//����ʹ�õ���ɫ��ʵ��������������õ����������ɫ�����û��������ʹ�ô˴����õ���ɫ
					glColor4f(lpsinglemodel->m_color[1].x, 
							lpsinglemodel->m_color[1].y,
							lpsinglemodel->m_color[1].z,
							lpsinglemodel->m_color[1].w);
				
					set_model_colorNumUse(lpsinglemodel, 1);
					set_model_displaytype(lpsinglemodel, 4);		
					RenderModelLLH(lpsinglemodel,pModelScene);
									
				}		
			}		
		}
	}	
		
	// �ٰ�����ʾ��ʽ0-����ģʽ����
	glColor3f(1.0f, 1.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0f);	
	
	// �򿪹���
	EnableLight(TRUE);
	EnableLight1(TRUE);	
	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 3.�ж���ʾ��ʽ�Ƿ�4
				if(	(lpsinglemodel->m_modelDisplaytype == 4) )
				{
					// 4.����ģ�ͣ���0-����ģʽ����
					set_model_colorNumUse(lpsinglemodel, 0);
					set_model_displaytype(lpsinglemodel, 0);		
					RenderModelLLH(lpsinglemodel,pModelScene);

					// 5.ģ��Ч����ԭΪ4
					set_model_displaytype(lpsinglemodel, 4);							
				}
			}
		
		}
	}	
	// �رչ���
	EnableLight(FALSE);
	EnableLight1(FALSE);

}

//����ʹ�õ�ģ�ͻ��ƺ���5����ʾ��ʽΪ��5-�⻷Ч��,��ģ��+��ɫ(1.05����Сָ����ɫ)��
void RenderModelType5(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;

	LPModelRender lpsinglemodel = NULL;

	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);

	//�Ȱ�����ʾ��ʽ5����	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);	
			

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 3.�ж���ʾ��ʽ�Ƿ�5
				if(	(lpsinglemodel->m_modelDisplaytype == 5))
				{
					// 4.������ʾ��ʽ��5����
					//����ʹ�õ���ɫ
					glColor4f(lpsinglemodel->m_color[2].x, 
							lpsinglemodel->m_color[2].y,
							lpsinglemodel->m_color[2].z,
							lpsinglemodel->m_color[2].w);	
					set_model_colorNumUse(lpsinglemodel, 2);
					set_model_displaytype(lpsinglemodel, 5);
					//�Ŵ�1.05��
					lpsinglemodel->m_modelScale *= 1.05;
					lpsinglemodel->m_MatrixChange = TRUE;
					RenderModelLLH(lpsinglemodel,pModelScene);
									
				}
		
			}
		
		}
	}	
		
	// �ٰ�����ʾ��ʽ0����
	glDisableEx(GL_BLEND);
	glDepthMask(GL_TRUE);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	// �򿪹���
	EnableLight(TRUE);
	EnableLight1(TRUE);	
	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.�ж���ʾ��ʽ�Ƿ�5
				if(	(lpsinglemodel->m_modelDisplaytype == 5) )
				{
					// 4.����ģ�ͣ���0-����ģʽ����
					set_model_colorNumUse(lpsinglemodel, 0);
					set_model_displaytype(lpsinglemodel, 0);
					//�ָ�������С
					lpsinglemodel->m_modelScale *= (1/1.05);
					lpsinglemodel->m_MatrixChange = TRUE;	
					RenderModelLLH(lpsinglemodel,pModelScene);			

					//ģ��Ч����ԭΪ5
					set_model_displaytype(lpsinglemodel, 5);
									
				}
			}
		}
	}	

	// �رչ���
	EnableLight(FALSE);
	EnableLight1(FALSE);

}



//����ʹ�õ�ģ�ͻ��ƺ���6����ʾ��ʽΪ��7-10��δ���壬��0-����ģʽ����
void RenderModelType6(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;
	
	// �򿪹���,�ƶ�������仯֮��
//	EnableLight(TRUE);
//	EnableLight1(TRUE);
	
	// 1.ѭ������
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
			
				// 3.�ж���ʾ��ʽ�Ƿ�7-10,
				if((lpsinglemodel->m_modelDisplaytype >= 7)&&(lpsinglemodel->m_modelDisplaytype <= 10))
				{
					RenderModelFeatureDynamic(lpsinglemodel,pModelScene);					
				}
	
			}
		
		}	
	}
	
	// �رչ���
//	EnableLight(FALSE);
//	EnableLight1(FALSE);
	
}

/*
	�������й켣��һά�������
*/
static f_uint32_t S_FLYTRACETEXTURE = 0;
void SetFlyTraceColor()
{
	#define flytrace_sampling 	64

	// ���÷��й켣ɫ��
	f_uint8_t chRGBA[flytrace_sampling * 4];
	f_int32_t i;

	memset(chRGBA, 0, flytrace_sampling * 4);		// Ĭ�϶��Ǻ�ɫ

	// ǰ32*4����ɫ�ǻ�ɫ��͸���Ȱ�i*7����������ȫ͸��תΪ��͸��
	for(i = 0 ; i<(flytrace_sampling /2); i++)
	{
		chRGBA[4 * i + 0] = 255;
		chRGBA[4 * i + 1] = 255;
		chRGBA[4 * i + 2] = 0;
		
		chRGBA[4 * i + 3] = (int) ((i * 255/(flytrace_sampling /2)) % 255);

		//printf("%d-",chRGBA[4 * i + 3] );	
	}

	// ��32*4����ɫ�ǻ�ɫ��͸���Ȱ�(64-1-i)*7�������ɲ�͸��תΪ��ȫ͸��
	for(i= (flytrace_sampling /2); i< (flytrace_sampling ); i++)
	{
		chRGBA[4 * i + 0] = 255;
		chRGBA[4 * i + 1] = 255;
		chRGBA[4 * i + 2] = 0;
		
		chRGBA[4 * i + 3] = chRGBA[4 * (flytrace_sampling -1 - i) + 3 ];
		//printf("%d-",chRGBA[4 * i + 3] );
	}
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	// ���ɷ��й켣����һά����������Ϊ64
	glDisableEx(GL_TEXTURE_1D);
	glGenTextures(1, &S_FLYTRACETEXTURE);
	glBindTexture(GL_TEXTURE_1D, S_FLYTRACETEXTURE);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, flytrace_sampling, 0, GL_RGBA, GL_UNSIGNED_BYTE, chRGBA);	
	glBindTexture(GL_TEXTURE_1D,0);
#else

	glDisableEx(GL_TEXTURE_2D);
	glGenTextures(1, &S_FLYTRACETEXTURE);
	glBindTexture(GL_TEXTURE_2D, S_FLYTRACETEXTURE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, flytrace_sampling, 1,  0, GL_RGBA, GL_UNSIGNED_BYTE, chRGBA);	
	glBindTexture(GL_TEXTURE_2D,0);
#endif
};

/*
 *	���Ʒ��й켣
 */
BOOL RenderFlyTrace(int nonecon, LPModelRender pModelSelect)
{
	f_int32_t  j;
	f_int16_t index = 0 ;

	PT_3D flytrace_tri[4] = {0};


	if(pModelSelect->m_flagUser[0] == FALSE)
	{
		return FALSE;
	}

	if(FALSE == UpdataFlyTrace(pModelSelect))
	{
		return FALSE;
	}


#if 1
	if(pModelSelect->m_flagUser[0] == TRUE)
	{
		index = pModelSelect->s_pTraceIndex - 1;
		if (index == -1)
		{
			index = FLYTRACE_NODENUM_MAX - 1;
		}
		glBegin(GL_QUAD_STRIP);
		for (j = 0; j < FLYTRACE_NODENUM_MAX; ++j)
		{
			if (pModelSelect->m_vFlyTrace[index].m_bUse == FALSE) break;
#if 0	
			glColor4f(1.0, 1.0, 0.0, (FLYTRACE_NODENUM_MAX - j) * s_nColorInterval);
#else

			{
				int part_num = FLYTRACE_NODENUM_MAX * 0.2;
				if(j < part_num)
				{
					//alpha������
					glColor4f(1.0, 1.0, 0.0, 
				( (FLYTRACE_NODENUM_MAX - FLYTRACE_NODENUM_MAX/2) * j / part_num + FLYTRACE_NODENUM_MAX /2) * s_nColorInterval);

				}
				else
				{
					//alpha�ݼ���
					glColor4f(1.0, 1.0, 0.0, (FLYTRACE_NODENUM_MAX + part_num - j) * s_nColorInterval);
				
				}

				// ����������
				
			}

#endif
#if 0
			if(j == FLYTRACE_NODENUM_MAX * 0.1)
			{
				SetPT3D(pModelSelect->m_vFlyTrace[index].m_ptUp, &flytrace_tri[0]);
				SetPT3D(pModelSelect->m_vFlyTrace[index].m_ptDown, &flytrace_tri[1]);
				{
					PT_3D ptUp;
					MemSet_3D(&ptUp, 10, 0, 0);			//����
				
					MultiMatrix4x4(&ptUp, &ptUp, &pModelSelect->m_mxRotate);
					MultiMatrix4x4(&flytrace_tri[2], &ptUp, &pModelSelect->m_mxWorld);
				}
				{
					PT_3D ptUp;
					MemSet_3D(&ptUp, -10, 0, 0);			//����
				
					MultiMatrix4x4(&ptUp, &ptUp, &pModelSelect->m_mxRotate);
					MultiMatrix4x4(&flytrace_tri[3], &ptUp, &pModelSelect->m_mxWorld);
				}



//					flytrace_tri[3].x = (flytrace_tri[0].x + flytrace_tri[1].x)/2.0;
//					flytrace_tri[3].y = (flytrace_tri[0].y + flytrace_tri[1].y)/2.0;
//					flytrace_tri[3].z = (flytrace_tri[0].z + flytrace_tri[1].z)/2.0;

					

			}




					

				

#endif


//				if(j > (FLYTRACE_NODENUM_MAX * 0.1 - 1))
			{
#if 1
				glTexCoord1f(0.0);
				glVertex3f(pModelSelect->m_vFlyTrace[index].m_ptUp.x, 
					pModelSelect->m_vFlyTrace[index].m_ptUp.y, 
					pModelSelect->m_vFlyTrace[index].m_ptUp.z);


				glTexCoord1f(1.0);				
				glVertex3f(pModelSelect->m_vFlyTrace[index].m_ptDown.x, 
					pModelSelect->m_vFlyTrace[index].m_ptDown.y, 
					pModelSelect->m_vFlyTrace[index].m_ptDown.z);
#endif
				
			}

				
			if (index == 0)
			{
				index = FLYTRACE_NODENUM_MAX - 1;
			}
			else
			{
				index--;
			}
		}
		glEnd();


#if 0
		//����������
		glDisableEx(GL_CULL_FACE);

		glColor4f(1.0, 1.0, 0.0, (FLYTRACE_NODENUM_MAX - FLYTRACE_NODENUM_MAX * 0.1) * s_nColorInterval);

		//glBegin(GL_TRIANGLES);
#if 0

		glTexCoord1f(0.0);
		glVertex3f(flytrace_tri[1].x,flytrace_tri[1].y,flytrace_tri[1].z);

		glTexCoord1f(0.0);
		glVertex3f(flytrace_tri[2].x,flytrace_tri[2].y,flytrace_tri[2].z);


		glTexCoord1f(0.5);
		glVertex3f(flytrace_tri[3].x,flytrace_tri[3].y,flytrace_tri[3].z);		


		glTexCoord1f(0.5);
		glVertex3f(flytrace_tri[3].x,flytrace_tri[3].y,flytrace_tri[3].z);

		glTexCoord1f(0.0);
		glVertex3f(flytrace_tri[2].x,flytrace_tri[2].y,flytrace_tri[2].z);


		glTexCoord1f(0.0);
		glVertex3f(flytrace_tri[0].x,flytrace_tri[0].y,flytrace_tri[0].z);		

//#else
		glTexCoord1f(0.5);
		glVertex3d(flytrace_tri[0].x,flytrace_tri[0].y,flytrace_tri[0].z);		

		glTexCoord1f(0.5);
		glVertex3d(flytrace_tri[1].x,flytrace_tri[1].y,flytrace_tri[1].z);


		glColor4f(1.0, 1.0, 0.0,0.0);		
		glTexCoord1f(0.0);
		glVertex3d(flytrace_tri[2].x,flytrace_tri[2].y,flytrace_tri[2].z);

#endif


		//glEnd();
			



		glBegin(GL_QUADS);

				glTexCoord1f(0.0);
				glVertex3f(flytrace_tri[0].x,flytrace_tri[0].y,flytrace_tri[0].z);

				glTexCoord1f(1.0);
				glVertex3f(flytrace_tri[1].x,flytrace_tri[1].y,flytrace_tri[1].z);

				glTexCoord1f(1.0);
				glVertex3f(flytrace_tri[3].x,flytrace_tri[3].y,flytrace_tri[3].z);		

				glTexCoord1f(0.0);
				glVertex3f(flytrace_tri[2].x,flytrace_tri[2].y,flytrace_tri[2].z);	

		glEnd();


#endif


	}
#endif

	return TRUE;
}

//�û���ʹ�õĻ��Ʒ��й켣�ĺ���������ͼ��ĺ������
void RenderUserFlyTrace(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);
		
//	glPushAttrib(GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
	glDisableEx(GL_CULL_FACE);
//	glDisableEx(GL_TEXTURE_2D);
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// 2.���ɷ��й켣��һά����,������һ��
	if(S_FLYTRACETEXTURE == 0)
	{
		SetFlyTraceColor();
	}
	
//	glPolygonMode(GL_FRONT, 0 ? GL_LINE : GL_FILL);	
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glEnableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, S_FLYTRACETEXTURE);	// �󶨷��й켣��һά����
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);//GL_BLEND);// GL_REPLACE);//GL_MODULATE); 
#else
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, S_FLYTRACETEXTURE);	// �󶨷��й켣����
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);//GL_BLEND);// GL_REPLACE);//GL_MODULATE); 

#endif

	// 3.����ͶӰ��ģ�Ӿ���
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(Get_PROJECTION(pModelScene));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(Get_MODELVIEW(pModelScene));
	
	// 4.����ģ��
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 4.1.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 4.2.�жϷ��й켣��ͼ���Ƿ���
				if(lpsinglemodel->m_flagUser[0] == TRUE)
				{
					// 4.3.���Ʒ��й켣
					RenderFlyTrace(0, lpsinglemodel);				
				}		
			}
		}
	}	
	
	// 5.�ָ�����
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
	
	// 6.�ָ�����״̬
//	glPopAttrib();
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_1D, 0);	
	glDisableEx(GL_TEXTURE_1D);
#else
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, 0);	// �󶨵��θ澯����
	glDisableEx(GL_TEXTURE_2D);

#endif
//	glEnableEx(GL_CULL_FACE);

	//�ָ�alphaֵ
	glColor4ub(255,255,255,255);
	glDisableEx(GL_BLEND);
	
}




//����ģ�͵���ӳ�����
void RenderRectangle(LPModelRender pModelRender, f_uint32_t color4ub[4])
{
	float delt_x,delt_z;
	float pass_y,y;

	//��ģ�͵Ĵ�С������x��z�Ͷ���y
	y = pModelRender->m_modelSize;
	delt_x = delt_z =  pModelRender->m_modelSize /2;
	//���õ���y��ֵ
 	pass_y = - pModelRender->m_modelSize * 0.1;

	// 1.������ʾ�б�		
	if(pModelRender->m_glLists == 0)
	{
		pModelRender->m_glLists = glGenLists(1);		
		
		glNewList(pModelRender->m_glLists, GL_COMPILE);
#if 0		
			//����xz��y = 0
			glBegin(GL_QUADS);
				glVertex3f(delt_x,   pass_y, delt_z);	
				glVertex3f(delt_x,   pass_y, -delt_z);	
				glVertex3f(-delt_x,   pass_y, -delt_z);	
				glVertex3f(-delt_x,   pass_y, delt_z);	
			glEnd();

		//����
		glBegin(GL_QUAD_STRIP);
			glVertex3f(delt_x,   y, delt_z);	
			glVertex3f(delt_x,   pass_y, delt_z);	

			
			glVertex3f(-delt_x,   y, delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	


			glVertex3f(-delt_x,   y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	


			glVertex3f(delt_x,   y, -delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	


				glVertex3f(delt_x,   y, delt_z);	
				glVertex3f(delt_x,   pass_y, delt_z);			
				
			glEnd();
			
			//����
			glBegin(GL_QUADS);
				glVertex3f(delt_x,   y, delt_z);	
				glVertex3f(-delt_x,   y, delt_z);	
				glVertex3f(-delt_x,   y, -delt_z);	
				glVertex3f(delt_x,   y, -delt_z);	
			glEnd();
#else
		//���Ƶ���ľ���,2��������
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   pass_y, delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	

			glVertex3f(delt_x,   pass_y, delt_z);
			glVertex3f(-delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	
				
		glEnd();

		//���ƶ���ľ���,2��������
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   y, delt_z);	
			glVertex3f(delt_x,   y, -delt_z);	
			glVertex3f(-delt_x,   y, -delt_z);	

			glVertex3f(delt_x,   y, delt_z);
			glVertex3f(-delt_x,   y, -delt_z);	
			glVertex3f(-delt_x,   y, delt_z);	
				
		glEnd();
	
		//����4��������Σ�ÿ�����������2�����������
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(delt_x,   y, delt_z);	
			glVertex3f(delt_x,   pass_y, delt_z);	

			glVertex3f(delt_x,   y, -delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	
				
			glVertex3f(-delt_x,   y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	
				
			glVertex3f(-delt_x,   y, delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	

			glVertex3f(delt_x,   y, delt_z);	
			glVertex3f(delt_x,   pass_y, delt_z);	

		glEnd();
			

#endif
		glEndList();	
			
	}
	
	// 2.���Ƴ�����		
//	glEnableEx(GL_POLYGON_SMOOTH);
//	glEnableEx(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// 2.1���û��Ƶ���ɫ
	glColor4ub((GLubyte)color4ub[0], (GLubyte)color4ub[1], (GLubyte)color4ub[2], (GLubyte)color4ub[3]);
	//glColor4ub(255,0,0,255);
	
	//2.2������ʾ�б���л���
	glCallList(pModelRender->m_glLists);
	
//	glDisableEx(GL_POLYGON_SMOOTH);
//	glDisableEx(GL_BLEND);
}
void RenderModelRectangle(int nonecon, LPModelRender pModelSelect)
{
//	LPNode pNode = (LPNode)ModelRenderList.m_pFirst;
//	LPModelRender lpsinglemodel = NULL;
	f_int32_t color_rectangle[4] = {0}; 

	if(pModelSelect->m_flagUser[2] == FALSE)
	{
		return;
	}

//	glPushAttrib(GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
//	glDisableEx(GL_CULL_FACE);
//	glDisableEx(GL_TEXTURE_2D);
//	glEnableEx(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
//	while(pNode != NULL)
	{
//		lpsinglemodel = (LPModelRender)pNode->m_pCur;

		//ȥ������Ҫ���Ƶ�ģ��
//		if(lpsinglemodel->m_flagTrace == FALSE)
//		{
//			pNode = pNode->m_pNext;
//			continue;
//		}

		//������ӳ�����
//		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		//ģ����ӳ�������ɫΪ��ɫ,͸����Ϊ100
		color_rectangle[0] = 150;
		color_rectangle[1] = 150;
		color_rectangle[2] = 150;
		color_rectangle[3] = 100;
		//�����õ���ɫ����ģ����ӳ�����
		RenderRectangle(pModelSelect,color_rectangle);

//		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		//color_rectangle[0] = color_rectangle[1] =color_rectangle[2] =;
//		color_rectangle[3] = 255;
//		RenderRectangle(lpsinglemodel,color_rectangle);
		
//		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		
//		pNode = pNode->m_pNext;

	}

//	glPopAttrib();


}
//�û���ʹ�õĻ��Ƴ�����ĺ���������ͼ��ĺ������
void RenderUserRectangle(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);
	
//	glEnableEx(GL_POLYGON_SMOOTH);
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);


	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.�жϳ�����ͼ���Ƿ����
				if(lpsinglemodel->m_flagUser[2] == TRUE)
				{
					// 4.���þ���,ת����ģ�ͻ��Ƶ�����ϵ
					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
					glLoadMatrixd(Get_PROJECTION(pModelScene));
				
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					glLoadMatrixd(Get_MODELVIEW(pModelScene));
					
					glMultMatrixd(lpsinglemodel->m_modelMatrix);
					
					// 5.���Ƴ�����
					RenderModelRectangle(0, lpsinglemodel);
					
					// 6.�ָ�����
					glMatrixMode(GL_PROJECTION);
					glPopMatrix();
					glMatrixMode(GL_MODELVIEW);
					glPopMatrix();					
				}	
			}	
		}
	}	
	
//	glDisableEx(GL_POLYGON_SMOOTH);
	glDepthMask(GL_TRUE);
	//�ָ�alphaֵ
	glColor4ub(255,255,255,255);
	glDisableEx(GL_BLEND);
}


//�ɻ��Ϸ����ַ�
extern TTFONT g_TTFFont;

//test for ���ֿ�
void RenderBillboard_test(
						const f_float64_t x,
						const f_float64_t y,
						const f_float64_t z,
						const f_float64_t sx,
						const f_float64_t sy,
						const unsigned short pString[][16] ,
						int *stringNum,
						TTFONT pFont,
						const f_uint8_t style,
						const f_float64_t yz_angle,
						const f_float64_t xz_angle,
						const f_float64_t xy_angle,
						PT_3D xyz_vertex
						)
{
	Matrix44 MxRotate, model_mx, MxWorld, MxScale, temp_mx;
	PT_3D ptPos;
	double Radius = 0.0;	//������ת�İ뾶
	int lineIndex = 0;
	float nextTextLineHeight = 0.0f; //ÿ��������ײ��ĸ߶�

#if 1
	//�����ӻ�������ϵת����������Ƶ�����ϵ,�Գ����Ӷ�������Ϊ����ϵԭ��
	glPushMatrix();
	{
		double size = 1;
		// 1.����任���洢
		vector3DMemSet(&ptPos, x, y, z);

		//����λ�õ���ת����֤λ��ʼ����ģ�͵����Ϸ�
//		Radius = sqrt(sqr(x) + sqr(y) + sqr(z));
//		ptPos.z = Radius * sin(yz_angle* DE2RA);
//		ptPos.x = Radius * sin(-xy_angle * DE2RA) * cos(-yz_angle* DE2RA);
//		ptPos.y = Radius * cos(xy_angle * DE2RA)  * cos(-yz_angle* DE2RA);

		dMatrix44MemSet(&MxWorld, 1,   	0,   		0,   		0.0,
								    0,   	1,   		0,   		0.0,
							           0,   	0,   		1,   		0.0,
							           ptPos.x, 	ptPos.y, 		ptPos.z,   		1.0f );
		//�������ž���
		size = sx;
		dMatrix44MemSet(&MxScale, size, 0, 0, 0,
								   0, size, 0, 0,
								   0, 0, size, 0,
								   (1-size)*MxWorld.m[12], (1-size)*MxWorld.m[13], (1-size)*MxWorld.m[14], 1);
		Maxtrix4x4_Mul(&temp_mx, &MxWorld, &MxScale);
		
		//������ת����
		dMatrix44RPYmemSet(&MxRotate, yz_angle, xz_angle,  xy_angle);
		Maxtrix4x4_Mul(&model_mx, &MxRotate, &temp_mx);
	}
	
	glMultMatrixd(model_mx.m);

#endif

	switch(style)
	{
	case BILLBOARD_CENTER:  //���ڱ����м�
#if 0
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex3d(-stringNum /2.0, -0.5, 0.0);		// ���½�
		glTexCoord2f(1.0, 1.0); glVertex3d( stringNum /2.0, -0.5, 0.0);		// ���½�
		glTexCoord2f(1.0, 0.0); glVertex3d( stringNum /2.0,  0.5, 0.0);		// ���Ͻ�
		glTexCoord2f(0.0, 0.0); glVertex3d(-stringNum /2.0,  0.5, 0.0);		// ���Ͻ�
		glEnd();
#endif
		break;
	case BILLBOARD_TOP: //���ڱ��Ƶײ�
		/*���ƻ���,��ʱ������*/
#if 0	
		//����������ǳ�С�������������ͬһƽ���Ͻ������λ��ƣ���
		//OpenGL�Ͳ�����ȷ�ж����ߵ����ֵ���ᵼ����Ȳ��ԵĽ������Ԥ�⣬
		//��ʾ����������ʱ������˸��ǰ���������棬���������Ϊz-fighting
		//�˴�Ϊ�˷�ֹ�ֺͱ���֮�����z-fighting
		glEnableEx(GL_POLYGON_OFFSET_FILL);
		//���ú����ƫ�����ļ��㹫ʽ��Offset=DZ*factor+r*units
		//����������ʾ��ǰ����ȸ���һЩ����ʾ��ʱ��ᱻǰ�����ǣ�
		//��Ϊ������ʾ��Ƚ�ǳ���ᱻ���Ƶ���Ļ��ȥ
		glPolygonOffset(0.0f, 1.0f);
		/* ���Ƶװ� */
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3d(-stringNum /2.0, 0.0, 0.0);		// ���½�
			glVertex3d( stringNum /2.0, 0.0, 0.0);		// ���½�
			glVertex3d( stringNum /2.0, 1.1, 0.0);	// ���Ͻ�
			glVertex3d(-stringNum /2.0, 1.1, 0.0);	// ���Ͻ�
		glEnd();

		/* ���Ƶװ���� */
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
			glVertex3d(-stringNum /2.0, 0.0, 0.0);		// ���½�
			glVertex3d( stringNum /2.0, 0.0, 0.0);		// ���½�
			glVertex3d( stringNum /2.0, 1.1, 0.0);	// ���Ͻ�
			glVertex3d(-stringNum /2.0, 1.1, 0.0);	// ���Ͻ�
		glEnd();
		glDisableEx(GL_POLYGON_OFFSET_FILL);
#endif
		/* ����,���ְ׿� */
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		ttfDrawStringPre(pFont);
		/* ���λ���4������ */
		for(lineIndex=0;lineIndex<4;lineIndex++)
		{
			if(stringNum[lineIndex]>0)
			{
				ttfDrawStringZ(pFont, -stringNum[lineIndex] /2.0f, stringNum[lineIndex] /2.0f, nextTextLineHeight, 0, pString[lineIndex], stringNum[lineIndex], &nextTextLineHeight);
			}
		}

		
		ttfDrawStringPro(pFont);

		break;
	default:
		break;
	}

	glPopMatrix();
}


void RenderModelTxt(char* text_out, LPModelRender pModelSelect, sGLRENDERSCENE* pModelScene)
{
	//�ַ���x�����ƫ��ֵΪ0
	float x = 0;

	//�ַ���y�����ƫ�ƣ���ģ�ʹ�С������ϵͳ�仯
//	float y = 10.0;
	float y = (float)pModelSelect->m_modelSize * pModelSelect->m_modelScale/3;	

	//������˵ĳ���,���ַ���y�����ƫ��ֵ��ͬ
	float y_rectange = y;			
	//�ַ���z�����ƫ��ֵΪ0
	float z = 0;
	//�ַ���xy�����ϵ�����ֵ
	f_float64_t sx = 10.0;			
	f_float64_t sy = 10.0;
	//����xzƽ���ϵ�ͶӰ����ֵ
	float delt_x = 0.2;			
	float delt_z = 0.2;
	//�˵ײ���y���ϵ�ƫ��
	float pass_y = 1.0;			

#if 0		
	int num = 0;
	unsigned char text_char[32] = {0}; 
	unsigned short text[16] = {0}; //{28023,21475,24066};
#endif

	PT_3D ptTout;

	double anglep12, anglepout;
	PT_3D ptEyeLonLatHei;
	PT_3D ptCenterLonLatHei;
	PT_3D ptModelLonLatHei;
	double aa = 0.0;
	
	// 1.�Ȼ��Ƴ�����,��Ĭ������ϵ�»���һ��������
	// 1.1������ʾ�б�,��ʼ��ʱ������ʾ�б�
	if(pModelSelect->m_stringList == 0)
	{
		pModelSelect->m_stringList = glGenLists(1);		
			
		glNewList(pModelSelect->m_stringList, GL_COMPILE);

		//���Ƴ�����ĵ���,2��������
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   pass_y, delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	

			glVertex3f(delt_x,   pass_y, delt_z);
			glVertex3f(-delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	
					
		glEnd();

		//���Ƴ�����Ķ���,2��������
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   y_rectange, delt_z);	
			glVertex3f(delt_x,   y_rectange, -delt_z);	
			glVertex3f(-delt_x,   y_rectange, -delt_z);	

			glVertex3f(delt_x,   y_rectange, delt_z);
			glVertex3f(-delt_x,   y_rectange, -delt_z);	
			glVertex3f(-delt_x,   y_rectange, delt_z);	
					
		glEnd();
		
		//���Ƴ������4������,ÿ��������2�����������
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(delt_x,   y_rectange, delt_z);	
			glVertex3f(delt_x,   pass_y, delt_z);	

			glVertex3f(delt_x,   y_rectange, -delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	
					
			glVertex3f(-delt_x,   y_rectange, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	
					
			glVertex3f(-delt_x,   y_rectange, delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	

			glVertex3f(delt_x,   y_rectange, delt_z);	
			glVertex3f(delt_x,   pass_y, delt_z);	

		glEnd();

		glEndList();	
				
	}

#if 0
	//ת��Ϊ���ַ�
	sprintf(text_char, "%d",pModelSelect->m_modeltype);
	ttfAnsi2Unicode(text, text_char, &num);
#endif	
	
#if 1
	// 2.�ָ�����������ϵ���ƶ���ģ�͵���������ϵλ����
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfProjMatrix.m);
	glLoadMatrixd(Get_PROJECTION(pModelScene));
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfModelMatrix.m);
	glLoadMatrixd(Get_MODELVIEW(pModelScene));
#else
	glPushMatrix();
#endif
	// 3.ģ�;������¼���ʱ�����¼���string����ֵ
	if(pModelSelect->m_stringWorldChange == TRUE)
	{
		//��������겢�洢
		Matrix44 model_mx, temp_mx;
		Matrix44 MxScale;
		Matrix44 MxWorld;	
		Matrix44 MxRotate;

		f_float64_t size = 0.0;

		//�Ŵ����趨Ϊ�̶���С��ȡ������ģ�͵ķŴ���
		size = pModelSelect->m_modelScale;	
		size = 2;//5;					
	
		//����ģ�͵�λ��ת������ֵ
		dMatrix44Copy(&MxWorld, &pModelSelect->m_mxWorld );	
		//�������ž�����λ��ת���������
		dMatrix44MemSet(&MxScale, size, 0, 0, 0,
								   0, size, 0, 0,
								   0, 0, size, 0,
								   (1-size)*MxWorld.m[12], (1-size)*MxWorld.m[13], (1-size)*MxWorld.m[14], 1);
		Maxtrix4x4_Mul(&temp_mx, &MxWorld, &MxScale);
		//������̬ת��������֮ǰת���������
		dMatrix44RPYmemSet(&MxRotate, 0, 0, 0);
		Maxtrix4x4_Mul(&model_mx, &MxRotate, &temp_mx);
		
		//�������յ�ת������ֵ
		dMatrix44Copy(&pModelSelect->m_stringWorld, &model_mx );		
		
		pModelSelect->m_stringWorldChange = FALSE;
	}
	//ת�������볡�����Ƶ�ģ�Ӿ�����ˣ��ͽ�����ϵת��������������ڵ�����ϵ��
	glMultMatrixd(pModelSelect->m_stringWorld.m);

#if 0	//lpf delete 2017-2-28 14:36:21
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
//	glEnableEx(GL_POLYGON_SMOOTH);
	glDisableEx(GL_CULL_FACE);
#endif	
	// 4.������ʾ�б�,���Ƴ�����
	glDisableEx(GL_BLEND);
	glColor3ub(10,10,10);
	glCallList(pModelSelect->m_stringList);
	glEnableEx(GL_BLEND);
	
	// 5.�����ַ���ת�Ƕ�,ʹ�þ�γ�ȼ���
	{
		Geo_Pt_D temp;
		objDPt2geoDPt(&(pModelScene->camParam.m_ptEye),
			&temp);

		vector3DMemSet(&ptEyeLonLatHei, temp.lon, temp.lat, temp.height);

		objDPt2geoDPt(&(pModelScene->camParam.m_ptCenter),
			&temp);

		vector3DMemSet(&ptCenterLonLatHei, temp.lon, temp.lat, temp.height);
	}
	
// 	vector3DMul(&ptEyeLonLatHei, &ptEyeLonLatHei, RA2DE);
// 	vector3DMul(&ptCenterLonLatHei, &ptCenterLonLatHei, RA2DE);
	vector3DMemSet(&ptModelLonLatHei, pModelSelect->m_modelPos.y, pModelSelect->m_modelPos.x, pModelSelect->m_modelPos.z);

	vector3DSub(&ptTout,  &ptEyeLonLatHei, &ptModelLonLatHei);
	ptTout.z = 0;
	vector3DNormalize(&ptTout);
	ptTout.y *= -1;

	{

		if(ptTout.y == 0)
		{
			if(ptTout.x > 0)
				anglep12 = 90;
			else
				anglep12 = 270;
		}

		aa = atan(ptTout.x / ptTout.y);

		if(ptTout.x > 0)
		{
			if(ptTout.y > 0)
			{
				anglep12 = aa * RA2DE;
			}
			else
			{
				anglep12 = aa * RA2DE+ 180;
			}
						
		}
		else
		{
			if(ptTout.y > 0)
			{
				anglep12 = aa * RA2DE + 360;
			}
			else
			{
				anglep12 = aa * RA2DE + 180;
			}


		}

	}


//	anglepout  = pModelSelect->m_modelAngle.x + anglep12;		//������ģ����ת
	anglepout  = anglep12 ;									//ֻ�����ӵ�λ����ת
	
	//6.�����ַ�
	glColor3ub(255,255,255);	
	//glColor3ub(0,0,0);
 	RenderBillboard_test(x, y, z, sx, sy, pModelSelect->m_pString, pModelSelect->m_stringNum, g_TTFFont, BILLBOARD_TOP,
		0, anglepout, 0, ptTout);

#if 0
//	glDisableEx(GL_POLYGON_SMOOTH);
	glEnableEx(GL_CULL_FACE);
	glPopAttrib();
#endif
	
#if 1
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
#else
	glPopMatrix();
#endif
	
	
			
}

//�û���ʹ�õĻ������ֵĺ���������ͼ��ĺ������
void RenderUserTxt(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.ѭ������
	pstListHead = &(pModelScene->pScene_ModelList);
	
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
	glEnableEx(GL_POLYGON_SMOOTH);
#else
	glPushAttrib(GL_COLOR_BUFFER_BIT/*|GL_CURRENT_BIT*/|GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
#endif
	glDisableEx(GL_CULL_FACE);

	// ׼��������ƻ������ƶ��������������֮ǰ������ᵼ�±����޷�����
	//ttfDrawStringPre(g_TTFFont);
	//glDisableEx(GL_BLEND);
		
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.�ж�����ͼ���Ƿ����
				if(lpsinglemodel->m_flagUser[1] == TRUE)
				{
					// 4.��������
					RenderModelTxt(NULL, lpsinglemodel, pModelScene);					
				}		
			}
		}
	}	
	
	//�ָ�alphaֵ
	glColor4ub(255,255,255,255);

	//�ָ�������ƻ���,�ƶ��������������֮ǰ������ᵼ�±����޷�����
	//ttfDrawStringPro(g_TTFFont);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glDisableEx(GL_POLYGON_SMOOTH);
#endif
//	glEnableEx(GL_CULL_FACE);
	glPopAttrib();

}

extern int Particletypetemp, Particletypetemp1;

/*.BH--------------------------------------------------------
**
** ������: BOOL RenderPlanes(sGLRENDERSCENE* pModelScene)
**
** ����:  ���÷ɻ�ģ�͵Ļ��Ʋ���,ʵ�ʲ������л���
**
** �������:  pModelScene           //��Ⱦ�������           
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  
**
**.EH--------------------------------------------------------
*/
BOOL RenderPlanes(sGLRENDERSCENE* pModelScene)
{
	f_int32_t i = 0; 

	ModelRender modelplane;	// �ɻ�ģ�ͻ��Ʊ���
	f_int32_t model_id = pModelScene->airplane_id;	//�ɻ���ģ��ID

	//0.��ȡ��ǰ�ӿڳ�����ģ�ͻ�������
	AttachpScene2Model(pModelScene);

	// 1.�жϷɻ�ģ���Ƿ���ɳ�ʼ��
	if(pModelScene->airplane_id == 0)
	{
		// 1.1.ģ�ͳ�ʼ����ɺ�Ž��зɻ�ģ�ͳ�ʼ��
		if(FALSE == GetModelInitFlag())
		{
			return FALSE;
		}
		else
		{
			// 1.2.���ɷɻ�ģ�ͽڵ�
			{
				static int id_used = 20102;//20103;

				pModelScene->airplane_id = NewModel(id_used);	
				id_used = 20107;
			}

			// 1.3.�ɻ�ģ�ͽڵ�����ʧ�ܣ�ֱ�ӷ��أ����ٻ���
			if(pModelScene->airplane_id == 0)
			{
				return FALSE;
			}

			// 1.4.����ģ�͵�����ϵ��Ϊ10
			if(FALSE == SetModelScale(pModelScene->airplane_id, 10))
			{
				return FALSE;
			}	

			// 1.5.����ģ�͵ĸ߶�ʹ�ú��θ߶�
			if(FALSE == SetModelHeiMode(pModelScene->airplane_id, 2))
			{
				return FALSE;
			}

			// 1.6.���÷ɻ�ģ���Ƿ���С��ͼ��ʾ
			SetModelScreenFlag(pModelScene->airplane_id, TRUE);		

			// 1.7.���÷ɻ�ģ����С��ͼ��ʾ����ʽ
			SetModelScreenFlagType(pModelScene->airplane_id,  17);	
			
		}

		// 1.8.��һ�����ɷɻ�ģ�ͺ�ֱ�ӷ��أ������Ʒɻ�ģ��
		return FALSE;
	}

	// 2.ʵʱ���·��в���(6�����ɶ�),�������κͷ�����ģʽ
	if(pModelScene->mdctrl_cmd.rom_mode == eROM_inroam)
	{
		modelplane.m_modelPos.x = pModelScene->geopt_pos_input.lat;//;g_pAirPlanes[i].m_Entity.m_nLat;
		modelplane.m_modelPos.y = pModelScene->geopt_pos_input.lon;//g_pAirPlanes[i].m_Entity.m_nLon;
		modelplane.m_modelPos.z = pModelScene->geopt_pos_input.height;//g_pAirPlanes[i].m_Entity.m_nHei;
	}
	else
	{
		modelplane.m_modelPos.x = pModelScene->geopt_pos.lat;//;g_pAirPlanes[i].m_Entity.m_nLat;
		modelplane.m_modelPos.y = pModelScene->geopt_pos.lon;//g_pAirPlanes[i].m_Entity.m_nLon;
		modelplane.m_modelPos.z = pModelScene->geopt_pos.height;//g_pAirPlanes[i].m_Entity.m_nHei;
	}

	modelplane.m_modelAngle.x =	360.0 - pModelScene->attitude.yaw;//g_pAirPlanes[i].m_Entity.m_nYaw;	//lpf add ��Ϊyaw �Ѿ���-180--180תΪ0--360���˴����뷶Χ��-180--180��
	modelplane.m_modelAngle.y = pModelScene->attitude.pitch;//g_pAirPlanes[i].m_Entity.m_nPitch;
	modelplane.m_modelAngle.z = pModelScene->attitude.roll;//g_pAirPlanes[i].m_Entity.m_nRoll;

	// 2.1.���÷ɻ�ģ�͵ľ�γ��
	if(FALSE == SetModelPos(pModelScene->airplane_id, modelplane.m_modelPos))
	{
		return FALSE;
	}

	// 2.2.���÷ɻ�ģ�͵ĺ��򡢸��������
	if(FALSE == SetModelAngle(pModelScene->airplane_id, modelplane.m_modelAngle))
	{
		return FALSE;
	}


	// 3.�������α�־
	// ����ģʽ�²���ʾ�ɻ����û�ͼ��(��4�㣺β�������֡������塢�û��Զ���)
	if(eVM_COCKPIT_VIEW == pModelScene->camctrl_param.view_type)	
	{
		// 3.1.����ģʽ�����ηɻ�ģ�͵Ļ���
		SetModelShield(model_id, TRUE);
		
		// 3.2.����ģʽ�·ɻ�ģ�͵ĵ�0/1/2���û�ͼ���������
		SetModelUserAll(model_id, FALSE);

		// 3.3.����ģʽ�·ɻ�ģ�͵ĵ�3���û�ͼ�㲻����
		SetModelUser(model_id,FALSE,3);

	}
	else	//����ģʽ�¾����Ʒɻ�ģ��
	{
		// 3.4.����ģʽ�²����ηɻ�ģ�͵Ļ���
		SetModelShield(model_id, FALSE);

		// 3.5.����ģʽ�·ɻ�ģ�͵ĵ�0/1/2���û�ͼ�������
		SetModelUserAll(model_id, TRUE);

		// 3.6.����ģʽ�·ɻ�ģ�͵ĵ�2/3���û�ͼ���������
		SetModelUser(model_id,FALSE,3);
		SetModelUser(model_id,FALSE,2);

		//SetModelUser(model_id,TRUE,0);
		
	}
	
//	SetModelDisplay(pModelScene->airplane_id, 11+Particletypetemp);

//	SetModelScale(pModelScene->airplane_id, Particletypetemp1);
	
	return TRUE;
	
}

//������������ϵϵ������������ϵΪԭ��,���û�ͼ��ʹ��ʱ����ģ�͵�����ϵԭ��Ϊԭ�㣻
static PT_3D worldBallPoint;
static double worldBallRadius;
void SetWorldBall(PT_3D point, double radius)
{
	vector3DMemSet(&worldBallPoint, point.x, point.y, point.z);
	worldBallRadius = radius;
}
PT_3D getPointByUV(double u, double v)
{
	PT_3D ret;

	ret.x = sin(PI * v) * cos(PI*2*u)*worldBallRadius + worldBallPoint.x;
	ret.y = sin(PI * v) * sin (PI*2*u)*worldBallRadius + worldBallPoint.y;
	ret.z = cos(PI *v)*worldBallRadius + worldBallPoint.z;

	return ret;
}
static int uStepsNum = 50;
static int vStepsNum = 50;
void RenderBall()
{
	double ustep = 1/(double)uStepsNum;
	double vstep = 1/(double)vStepsNum;
	double u = 0, v = 0;
	int i = 0, j = 0;
	PT_3D point;

	//�����¶�������
	for(i=0; i <uStepsNum; i++)
	{
		glBegin(GL_LINE_LOOP);
		{
			point = getPointByUV(0,0);
			glVertex3f(point.x, point.y, point.z);

			point = getPointByUV(u,vstep);
			glVertex3f(point.x, point.y, point.z);
			
			point = getPointByUV(u+ustep,vstep);
			glVertex3f(point.x, point.y, point.z);

			u += ustep;
		}
		glEnd();

	}

	//�����м���ı���
	u = 0, v=vstep;
	for(i=0;i<vStepsNum; i++)
	{
		for(j=0; j<uStepsNum; j++)
		{
			glBegin(GL_LINE_LOOP);
			{
			point = getPointByUV(u,v);
			glVertex3f(point.x, point.y, point.z);

			point = getPointByUV(u+ustep,v);
			glVertex3f(point.x, point.y, point.z);

			point = getPointByUV(u+ustep, v+vstep);
			glVertex3f(point.x, point.y, point.z);

			point = getPointByUV(u, v+vstep);
			glVertex3f(point.x, point.y, point.z);

			u += ustep;
			}
			glEnd();
		}
		v += vstep;
	}

	//�����¶�������
	u = 0;
	for(i=0; i <uStepsNum; i++)
	{
		glBegin(GL_LINE_LOOP);
		{
			point = getPointByUV(0,1);
			glVertex3f(point.x, point.y, point.z);

			point = getPointByUV(u,1-vstep);
			glVertex3f(point.x, point.y, point.z);

			point = getPointByUV(u+ustep,1-vstep);
			glVertex3f(point.x, point.y, point.z);
	
		}
		glEnd();
	}
	
}
void RenderWorldBall(char text_out, LPModelRender pModelSelect)
{
	PT_3D ZERO_Point = {0};
	f_int32_t color_rectangle[4] = {0}; 

	if(pModelSelect->m_flagUser[3] == FALSE)
	{
//		return;
	}
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glPushAttrib(GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
#else
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
#endif

		color_rectangle[0] = 150;
		color_rectangle[1] = 150;
		color_rectangle[2] = 150;
		color_rectangle[3] = 100;

	glColor4ub(
		(GLubyte)color_rectangle[0],
		(GLubyte)color_rectangle[1],
		(GLubyte)color_rectangle[2],
		(GLubyte)color_rectangle[3]);
	
	SetWorldBall(ZERO_Point, pModelSelect->m_modelSize);
	RenderBall();

	glPopAttrib();
	
}


//���ԣ���ά�� lpf 2017-3-16 14:54:25
static f_uint32_t ModelTreeID = 0;				//��������ID
static f_uint32_t ModelTreeListID = 0;			//������ʾ�б�ID

void RenderWorldTree(char text_out, LPModelRender pModelSelect)
{
#ifdef WIN32
	f_char_t filepath[256] = "F:\\��ͼ�������\\DataFile\\model\\tree\\tree.bmp";
#else
	
	f_char_t filepath[256] = "/ahci00/mapData3d/DataFile/model/tree/tree.bmp";
	#endif
	if(pModelSelect->m_flagUser[3] == FALSE)
	{
		return;
	}


	// 1.��ʼ������
	if (ModelTreeID == 0)
	{
		LoadParticlesTexture(filepath, &ModelTreeID);
	}


	// 2.������������棬����ͼ
//	glEnableEx(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//	glEnableEx(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, ModelTreeID);
//	glDisableEx(GL_CULL_FACE);
//	glDepthMask(GL_FALSE);
	
	
	
	glScalef(20,20,20);

	// 3.ʹ����ʾ�б�
	if(ModelTreeListID == 0)
	{
		ModelTreeListID = glGenLists(1);
		glNewList(ModelTreeListID, GL_COMPILE);
		{

			// 2.1.������
		//	glColor4ub(150,150,150,200);		//������ɫ��ϳ̶�
		#if 1
			//��һ����
			glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(	-1,0,0);	

				glTexCoord2f(0.5,0);
				glVertex3f(	0,0,0);	

				glTexCoord2f(0.5,1);
				glVertex3f(	0,0,2);	

				glTexCoord2f(0,1);
				glVertex3f(	-1,0,2);	
			glEnd();	

			//�ڶ�����
			glRotated(120.0,0.0,0.0,1.0);
			glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(	-1,0,0);	

				glTexCoord2f(0.5,0);
				glVertex3f(	0,0,0);	

				glTexCoord2f(0.5,1);
				glVertex3f(	0,0,2);	

				glTexCoord2f(0,1);
				glVertex3f(	-1,0,2);	
			glEnd();	

			//��������
			glRotated(120.0,0.0,0.0,1.0);
			glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex3f(	-1,0,0);	

				glTexCoord2f(0.5,0);
				glVertex3f(	0,0,0);	

				glTexCoord2f(0.5,1);
				glVertex3f(	0,0,2);	

				glTexCoord2f(0,1);
				glVertex3f(	-1,0,2);	
			glEnd();	

			//���ĸ���
		//	glRotated(90.0,0.0,0.0,1.0);
		//	glBegin(GL_QUADS);
		//		glTexCoord2f(0,0);
		//		glVertex3f(	-1,0,0);	

		//		glTexCoord2f(0.5,0);
		//		glVertex3f(	0,0,0);	

		//		glTexCoord2f(0.5,1);
		//		glVertex3f(	0,0,2);	

		//		glTexCoord2f(0,1);
		//		glVertex3f(	-1,0,2);	
		//	glEnd();	
		#else

			glBegin(GL_QUADS);
				//��һ����
				glTexCoord2f(0,0);
				glVertex3f(	1,0,0);	

				glTexCoord2f(1,0);
				glVertex3f(	-1,0,0);	

				glTexCoord2f(1,1);
				glVertex3f(	-1,0,2);	

				glTexCoord2f(0,1);
				glVertex3f(	1,0,2);	

			glEnd();	

			glRotated(90.0,0.0,0.0,1.0);
			glBegin(GL_QUADS);
			//��һ����
			glTexCoord2f(0,0);
			glVertex3f(	1,0,0);	

			glTexCoord2f(1,0);
			glVertex3f(	-1,0,0);	

			glTexCoord2f(1,1);
			glVertex3f(	-1,0,2);	

			glTexCoord2f(0,1);
			glVertex3f(	1,0,2);	

			glEnd();	
			
		#endif
	
		}	
		glEndList();
	}
	else
	{
		glCallList(ModelTreeListID);

	}
	
	
//	glBindTexture(GL_TEXTURE_2D,0);
//	glDisableEx(GL_TEXTURE_2D);
//
//	glDisableEx(GL_BLEND);
//	glEnableEx(GL_CULL_FACE);
//	glDepthMask(GL_TRUE);
	
	
//	glPopAttrib();

}





/*
	����:���ɵ�������ģ��
	����:���ȣ�γ�ȣ�������ƫ���ǣ���λ:��
*/
BOOL GenerateSingleTree(double lon, double lat , double scale,double yaw)
{
	int model_make;
	PT_3D model_pos;
	PT_3D pitchvalue = {0};
	
	// 1.������ģ��
	model_make = NewModel(20109);				//������ģ��
	if(FALSE == model_make)
	{
		return FALSE;
	}

			
	model_pos.x = lat;
	model_pos.y = lon;
	model_pos.z =  2000;
	pitchvalue.x = yaw;
	pitchvalue.y = 270;

	SetModelPos(model_make, model_pos);
	SetModelAngle(model_make, pitchvalue);
	
	SetModelScale(model_make, scale);
	SetModelDisplay(model_make, 1);
			
	SetModelShield(model_make, TRUE);
	SetModelUserAll(model_make, FALSE);
	SetModelUser(model_make, TRUE, 3);

	return TRUE;
}

/*
	��������������������������ģ��Ⱥ
	���룺���������������飬lon, lat, scale��
	���ݣ�������������ģ�ͣ�����ģ����ʾ��ʽ��
	����ģ�ͳ�ʼ��֮��
*/

BOOL GenerateTreeModels()
{
	int tree_num = 900;				//���ĸ���
	int i = 0;
	int j = 0;								
	double tree_data[900][4] = {0};		//��������lon ,lat ,scale
	int tree_num_sqr = sqrt(tree_num);		//���ĸ�������ֵ
	
	// 1.׼�������������
	{
		double lon_begin = 120.8f;
		double lat_begin = 22.98f;

		double lonlat_step = 200.0 * 9.03/1000000;	//���ļ��

		double lon_user, lat_user;

		for(i = 0; i<tree_num_sqr;i++)		
		{
			lon_user = lon_begin + (i) *lonlat_step;	
				
			for(j=0; j<tree_num_sqr; j++)
			{
				lat_user = lat_begin + (j)*lonlat_step;		


				tree_data[i * tree_num_sqr + j][0] = lon_user;
				tree_data[i * tree_num_sqr + j][1] = lat_user;
				
				tree_data[i * tree_num_sqr + j][2] = 5;
				//tree_data[i * tree_num_sqr + j][2] = (i * tree_num_sqr + j) %10 + 1;	//�Ŵ���3-7

				//tree_data[i * tree_num_sqr + j][3] = (i * tree_num_sqr + j) % 360 * 10.0;	//yaw ֵ0 -360
				tree_data[i * tree_num_sqr + j][3] = 0;
			}

		}
	
	}

	// 2.������ģ�͵�ID
	for(i=0; i<tree_num; i++)
	{
		if(FALSE ==  GenerateSingleTree(tree_data[i][0], tree_data[i][1] , tree_data[i][2], tree_data[i][3]))
		{
			return FALSE;
		}
	}

	return TRUE;
}






//�û��Զ����ͼ�㺯��,������β������ӳ����塢����ĺ���
void ModelUserDraw(int none, LPModelRender pModelUser)
{	
//	RenderModelRectangle(0, pModelUser);	//���ڴ˴�ʹ�ã�lpf 2017-2-28 14:13:45
//	RenderModelTxt(NULL, pModelUser);		//���ڴ˴�ʹ�ã�lpf 2017-2-28 14:46:40
//	RenderFlyTrace(0, pModelUser);			//���ڴ˴�ʹ�ã�lpf 2017-3-1 10:29:36

	//���������,����ʹ�ã�����û��ʹ����ʾ�б�
//	RenderWorldBall(0,pModelUser);
	RenderWorldTree(0,pModelUser);
	
}

//�ṩ�����ο���ʹ�õ�ע�ắ���ӿ�,�޸�Ϊ���������ض�����
void RenderUserDiy(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	// 0.�ж���û��ʹ��ע�ắ��
	if(NULL == funcModelRegedit.userLayerDrawFunc)
	{
		return;
	}

	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ModelTreeID);
//	glDisableEx(GL_CULL_FACE);
	//glDepthMask(GL_FALSE);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glEnableEx(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);
#endif
		
	
	// 1.ѭ������
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

		
			// 2.�ж�ģ���Ƿ����
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 2.1.�ж��û��Զ���ͼ���Ƿ����
				if(lpsinglemodel->m_flagUser[3] == TRUE)
				{
					// 3.���þ���
					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
					glLoadMatrixd(Get_PROJECTION(pModelScene));
				
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					glLoadMatrixd(Get_MODELVIEW(pModelScene));
					
					glMultMatrixd(lpsinglemodel->m_modelMatrix);
					
					// 4.ע�ắ������	/* �����û�ͼ�� */
	//				if(NULL != funcModelRegedit.userLayerDrawFunc)
	//				{	
						funcModelRegedit.userLayerDrawFunc(funcModelRegedit.userLayerParam, lpsinglemodel);
	//				}
					
					// 5.�ָ�����
					glMatrixMode(GL_PROJECTION);
					glPopMatrix();
					glMatrixMode(GL_MODELVIEW);
					glPopMatrix();					
				}
				
			
			}
		
		}
	}	
	
	glBindTexture(GL_TEXTURE_2D,0);
	glDisableEx(GL_TEXTURE_2D);
	glDisableEx(GL_BLEND);
//	glEnableEx(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glDisableEx(GL_ALPHA_TEST);
#endif
	
}

/*.BH--------------------------------------------------------
**
** ������: worldXYZ2screenXY
**
** ����: ��������ϵ�µĵ�ת��Ϊ��Ļ�ϵĵ�
**
** �������:  ��������
**
** �����������Ļ����
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ڲ��ӿ�2 
**
**.EH--------------------------------------------------------
*/
void worldXYZ2screenXY( PT_3D xyz, double* x, double* y)
{
	GLint viewport[4];
	GLdouble* modelview = NULL;
	GLdouble* projection = NULL;
	
	/*double winX,winY,winZ;*/
	
	glGetIntegerv(GL_VIEWPORT, viewport);
	
#ifdef HIGH_PRECISE_MATRIX
	modelview = Get_MODELVIEW();
	projection = Get_PROJECTION();
#else
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	//projection = modelview_out.m;
#endif

//	objXyzToScreenXyz3d(xyz.x, xyz.y, xyz.z, &winX, &winY, &winZ, modelview, projection, viewport);
	{
		Geo_Pt_D temp1 ;
		PT_2I temp2;
		temp1.lon = xyz.x; temp1.lat = xyz.y; temp1.height = xyz.z;
		
		geoDPt2OuterScreenDPt(&temp1, modelview, viewport, &temp2);

		*x = temp2.x;
		*y = temp2.y;		
	}





	
}

void ttmodel()
{
	{
		int modelair1;
		int modelair2;
		PT_3D angle = {21.846, 0.0, 0.0};
		PT_3D latlonhei ;
		
		modelair1 = NewModel(10310);
		modelair2 = NewModel(10310);
		
		
		latlonhei.x = 30+33/60.0+48.7/3600.0;			
		latlonhei.y = 103+56/60.0+24.1/3600.0;			
		latlonhei.z = 492.557;			
		
		
		
		SetModelPos(modelair1, latlonhei);
		SetModelScale(modelair1, 3599.9/1089.965);
		SetModelAngle(modelair1, angle);
		
		latlonhei.x = 30+35/60.0+36.7/3600.0;			
		latlonhei.y = 103+57/60.0+14.4/3600.0;			
		latlonhei.z = 495.30;			
		
		angle.x = 201.846;
		
		SetModelPos(modelair2, latlonhei);
		SetModelScale(modelair2, 3599.9/1089.965);
		SetModelAngle(modelair2, angle);
		
	}
}


//lpf add for test model api
BOOL TestModelAppMake(int scene)
{
	int i = 0;
	int j = 0;
	PT_3D latlonhei ;
	PT_3D latlonhei_used ;
		
	latlonhei.x = 30;//22.6;		//120.461,22.60002
	latlonhei.y = 120.38;
	latlonhei.z = 100;//1000;
	
	latlonhei_used.x = latlonhei.x;
	latlonhei_used.y = latlonhei.y;
	latlonhei_used.z = latlonhei.z;
	
	if(GetModelInitFlag() == FALSE)
	{
		
		return FALSE;
	}

#if 0			//���ֲ�ͬ��ģ�����ɷ�ʽ��������ȫ��ģ�ͣ���һ���Ƿɻ�ģ��
	modelid[scene][0] = NewModel(2010301);	
//	modelid[0] = NewModel(10301);
	modelid[scene][1] = NewModel(20201);
	modelid[scene][2] = NewModel(10202);
	modelid[scene][3] = NewModel(10502);
	modelid[scene][4] = NewModel(10301);
	modelid[scene][5] = NewModel(10501);

//	for(i=6;i <9;i++)
//	{
//		modelid[i] = NewModel(10301);
//	}
#if 1
	i = 5; 
	i++;	modelid[scene][i] = NewModel(20202);//,car,car,3.0,0,0,,,,,,,,,,,,,,,,,,������ģ��,,,,,,,,,
	i++;	modelid[scene][i] = NewModel(10501);//,ld,ld,40,0,0
	i++;	modelid[scene][i] = NewModel(10201);//,arrows,arrows,95,0,0
	i++;	modelid[scene][i] = NewModel(20109);//,rectan,rectan,10,0,0,
	i++;	modelid[scene][i] = NewModel(20103);//,h6,h6,34,0,0,,,,type,name,name3ds,size,childnum,childtype1,...,childtypeN
	i++;	modelid[scene][i] = NewModel(10302);//,hangzlou,hangzlou,70,0,0,,,
	i++;	modelid[scene][i] = NewModel(20204);//,cancar,cancar,15,0,0,,
	//i++;	modelid[i] = NewModel(10304);//,ground3,ground3,1000,0,0,,
	i++;	modelid[scene][i] = NewModel(10306);//,block,block,80,0,0,,
	i++;	modelid[scene][i] = NewModel(10307);//,viaduct,viaduct,150,0,0,,
	i++;	modelid[scene][i] = NewModel(10308);//,block2,block2,50,0,0,,,
	i++;	modelid[scene][i] = NewModel(10101);//,tower_picture2,tower_picture2,10.000000,0,0
	i++;	modelid[scene][i] = NewModel(20101);//,A10_convert,A10_convert,25,0,0
	i++;	modelid[scene][i] = NewModel(20104);//,BELL222,BELL222,5,0,0,,,,
	//i++;	modelid[i] = NewModel(20301);//,NYESEY,NYESEY,20,0,0,,,
	i++;	modelid[scene][i] = NewModel(20105);//,Mu28,Mu28,3,0,0,,,
	i++;	modelid[scene][i] = NewModel(10601);//,tiger,tiger,,2.000,0,0,,,
	i++;	modelid[scene][i] = NewModel(20106);//,A340,A340,50,0,0,,,
	i++;	modelid[scene][i] = NewModel(20203);//,doubike,doubike,1,0,0,,,
	i++;	modelid[scene][i] = NewModel(20102);//,J10Plane,J10Plane,1,0,0,,,
	i++;	modelid[scene][i] = NewModel(10200);//,building,building,51,0,0
#endif
	//	modelid[6] = NewModel(2010303);
	
	//��������ֵ
	//	i = 6;
	//	SetModelPos(modelid[i], latlonhei);
	//	SetModelScale(modelid[i], 5);
	
#else
	modelid[scene][0] = NewModel(2010301);	
	//	modelid[0] = NewModel(10301);
	modelid[scene][1] = NewModel(20101);
	modelid[scene][2] = NewModel(20106);
	modelid[scene][3] = NewModel(20104);
	modelid[scene][4] = NewModel(20103);
	modelid[scene][5] = NewModel(20102);
	modelid[scene][6] = NewModel(20105);
	modelid[scene][7] = NewModel(10305);
	modelid[scene][8] = NewModel(10309);
	
#endif

	//����ģ�͸���ֵ
	for(i = 0; i<9;i++)		
	{
		latlonhei_used.x = latlonhei.x + (i) *0.01;	
			
		for(j=0; j<9; j++)
		{
			PT_3D angle = {112.0, 0.0, 0.0};
			latlonhei_used.y = latlonhei.y + (j)*0.01;			
			
			if (i * 9 + j == 7 || i * 9 + j == 8) {
// 				latlonhei_used.x = 30.7040021;
// 				latlonhei_used.y = 103.954621;
// 				latlonhei_used.z = 530;
				latlonhei_used.x = 30.58;
				latlonhei_used.y = 103.952258;
 				latlonhei_used.z = 530;


				SetModelPos(modelid[scene][i*9+j], latlonhei_used);
				SetModelScale(modelid[scene][i*9+j], 1);
				SetModelAngle(modelid[scene][i*9+j], angle);
				if (i * 9 + j == 8)
					SetModelDisplay(modelid[scene][i*9+j],  7);
				else
					SetModelDisplay(modelid[scene][i*9+j],  0);
				//			SetModelUserAll(modelid[i*9+j], TRUE);
				SetModelUser(modelid[scene][i*9+j], FALSE, 0);		//����Ӱ��Ƕ��ʽ�µ�֡��
				SetModelUser(modelid[scene][i*9+j], FALSE, 1);
				SetModelUser(modelid[scene][i*9+j], FALSE, 2);
				SetModelUser(modelid[scene][i*9+j], FALSE, 3);
				continue;
			}
			
			SetModelPos(modelid[scene][i*9+j], latlonhei_used);
			SetModelScale(modelid[scene][i*9+j], 25);
//			SetModelDisplay(modelid[i],  GetModelDisplay());
//			SetModelDisplay(modelid[scene][i*9+j],  i);
			SetModelDisplay(modelid[scene][i*9+j],  6);
		
//			SetModelUserAll(modelid[i*9+j], TRUE);
			SetModelUser(modelid[scene][i*9+j], FALSE, 0);		//����Ӱ��Ƕ��ʽ�µ�֡��
			SetModelUser(modelid[scene][i*9+j], TRUE, 1);
			SetModelUser(modelid[scene][i*9+j], TRUE, 2);
								
		}

		
	}
	
//	SetChildNum(modelid[0], 2);
//	modelid[7] = NewChildModel(modelid[0], 0,2010303);		//����ģ��
//	modelid[8] = NewChildModel(modelid[0], 1,2010302);		//������ģ��
	//SetChildModelPos( modelid[0], 1, PT_3D xyz);
	//SetModelAngleAdd( modelid[0], 45, 1);

//	userFuncModelRegedit(ModelTest, "�ɶ���");
//	userFuncModelRegedit(RenderFlyTrace, NULL);
//	userFuncModelRegedit(RenderModelRectangle, NULL);
	userFuncModelRegedit( (USER_MODEL_DRAW_FUNC)ModelUserDraw, NULL);

	SetModelUser(modelid[scene][0], TRUE, 0);

//	GenerateTreeModels();		// lpf add for ��Ⱥ�����ɺ���2017-4-18 11:01:41
	if(0)
	{
		PT_4D color_picture = {0};

		for(i = 0; i<6;i++)
		{
			//lpf test for  ��ɫģ��
			switch (i)
			{
			case 0: 			
				color_picture.x = 1.0;
				color_picture.y = 0.0;
				color_picture.z = 0.0;
				break;

			case 1: 			
				color_picture.x = 0.0;
				color_picture.y = 1.0;
				color_picture.z = 0.0;
				break;
			case 2: 			
				color_picture.x = 0.0;
				color_picture.y = 0.0;
				color_picture.z = 1.0;
				break;
			case 3: 			
				color_picture.x = 1.0;
				color_picture.y = 1.0;
				color_picture.z = 0.0;
				break;
			case 4: 			
				color_picture.x = 1.0;
				color_picture.y = 0.0;
				color_picture.z = 1.0;
				break;
			case 5: 			
				color_picture.x = 0.0;
				color_picture.y = 1.0;
				color_picture.z = 1.0;
				break;
			}

			SetModelColor(modelid[scene][i], 3, color_picture);
				
		}

	}
	
//	ttmodel();
	return TRUE;

}

BOOL AddModelToScene(int scene)
{
	int i = 0;
	int j = 0;
	PT_3D latlonhei ;
	PT_3D latlonhei_used ;
	PT_3D angle = {0.0, 0.0, 0.0};

	latlonhei.x = 30.675;//22.6;		//120.461,22.60002
	latlonhei.y = 104.051;
	latlonhei.z = 1000;//1000;

	latlonhei_used.x = latlonhei.x;
	latlonhei_used.y = latlonhei.y;
	latlonhei_used.z = latlonhei.z;

	if(GetModelInitFlag() == FALSE)
	{
		return FALSE;
	}

#if 0			//���ֲ�ͬ��ģ�����ɷ�ʽ��������ȫ��ģ�ͣ���һ���Ƿɻ�ģ��
	modelid[scene][0] = NewModel(2010301);	
	//	modelid[0] = NewModel(10301);
	modelid[scene][1] = NewModel(20201);
	modelid[scene][2] = NewModel(10202);
	modelid[scene][3] = NewModel(10502);
	modelid[scene][4] = NewModel(10301);
	modelid[scene][5] = NewModel(10501);

	//	for(i=6;i <9;i++)
	//	{
	//		modelid[i] = NewModel(10301);
	//	}
#if 1
	i = 5; 
	i++;	modelid[scene][i] = NewModel(20202);//,car,car,3.0,0,0,,,,,,,,,,,,,,,,,,������ģ��,,,,,,,,,
	i++;	modelid[scene][i] = NewModel(10501);//,ld,ld,40,0,0
	i++;	modelid[scene][i] = NewModel(10201);//,arrows,arrows,95,0,0
	i++;	modelid[scene][i] = NewModel(20109);//,rectan,rectan,10,0,0,
	i++;	modelid[scene][i] = NewModel(20103);//,h6,h6,34,0,0,,,,type,name,name3ds,size,childnum,childtype1,...,childtypeN
	i++;	modelid[scene][i] = NewModel(10302);//,hangzlou,hangzlou,70,0,0,,,
	i++;	modelid[scene][i] = NewModel(20204);//,cancar,cancar,15,0,0,,
	//i++;	modelid[i] = NewModel(10304);//,ground3,ground3,1000,0,0,,
	i++;	modelid[scene][i] = NewModel(10306);//,block,block,80,0,0,,
	i++;	modelid[scene][i] = NewModel(10307);//,viaduct,viaduct,150,0,0,,
	i++;	modelid[scene][i] = NewModel(10308);//,block2,block2,50,0,0,,,
	i++;	modelid[scene][i] = NewModel(10101);//,tower_picture2,tower_picture2,10.000000,0,0
	i++;	modelid[scene][i] = NewModel(20101);//,A10_convert,A10_convert,25,0,0
	i++;	modelid[scene][i] = NewModel(20104);//,BELL222,BELL222,5,0,0,,,,
	//i++;	modelid[i] = NewModel(20301);//,NYESEY,NYESEY,20,0,0,,,
	i++;	modelid[scene][i] = NewModel(20105);//,Mu28,Mu28,3,0,0,,,
	i++;	modelid[scene][i] = NewModel(10601);//,tiger,tiger,,2.000,0,0,,,
	i++;	modelid[scene][i] = NewModel(20106);//,A340,A340,50,0,0,,,
	i++;	modelid[scene][i] = NewModel(20203);//,doubike,doubike,1,0,0,,,
	i++;	modelid[scene][i] = NewModel(20102);//,J10Plane,J10Plane,1,0,0,,,
	i++;	modelid[scene][i] = NewModel(10200);//,building,building,51,0,0
#endif
	//	modelid[6] = NewModel(2010303);

	//��������ֵ
	//	i = 6;
	//	SetModelPos(modelid[i], latlonhei);
	//	SetModelScale(modelid[i], 5);

#else
	modelid[scene][0] = NewModel(20102);	
	modelid[scene][1] = NewModel(20102);
	modelid[scene][2] = NewModel(20102);
	modelid[scene][3] = NewModel(20102);
	modelid[scene][4] = NewModel(20102);
	modelid[scene][5] = NewModel(20102);
	modelid[scene][6] = NewModel(20102);
	modelid[scene][7] = NewModel(20102);
	modelid[scene][8] = NewModel(20102);

#endif

	//����ģ�͸���ֵ
	for(i = 0; i<9;i++)		
	{
		latlonhei_used.y = latlonhei.y + (i)*0.002;			

		SetModelPos(modelid[scene][i], latlonhei_used);
		SetModelAngle(modelid[scene][i], angle);
		SetModelScale(modelid[scene][i], 10);

		SetModelDisplay(modelid[scene][i],  0);

		SetModelUser(modelid[scene][i], FALSE, 0);		//����Ӱ��Ƕ��ʽ�µ�֡��
		SetModelUser(modelid[scene][i], TRUE, 1);
		SetModelUser(modelid[scene][i], FALSE, 2);

	}


	return TRUE;

}

int modeltypeset = 6;	//lpf add 2017-2-20 14:48:32 �ⲿ����ģ�Ͳ�ͬ����ʾЧ��

void TestModelAppSetDisplay(int value)
{
	modeltypeset = value;
}

void TestModelAppSet(int scene)
{
	int i = 0;
//	PT_3D latlonhei ;
	PT_3D YPR_model;
	PT_4D color_picture = {0};
	
	if (GetModelInitFlag() == FALSE)
	{
		return;
	}
	
	for(i = 0; i<6;i++)
	{
		SetModelDisplay(modelid[scene][i],  modeltypeset);
	}


//	SetChildModelAngleAdd(modelid[scene][0], 1, 10, 0);
//	SetModelAngleAdd( modelid[0], 5, 0);
//	SetModelAngleAdd( modelid[0], 5, 1);
//	SetModelAngleAdd( modelid[0], 5, 2);
	//SetChildModelPosAdd(modelid[0],0,10,1 );

//	YPR_model.x = 90;
//	YPR_model.y =  YPR_model.z = 0;
//	SetModelAngle( modelid[0], YPR_model);

	//����ģ�͵ļ�¼���ݶ�д
	if(0)
	{
		SetModelAirline(&modelid[scene][0], 6);

		SetModelPosAdd(modelid[scene][0], 0.0001, 0);
	}


	//���Դ�ɫ������л�
	if(1)
	{
		PT_4D color_picture = {0};
		static int color_i = 0;

		for(i = 0; i<6;i++)
		{
			//lpf test for  ��ɫģ��
			switch (color_i % 6)
			{
			case 0: 			
				color_picture.x = 1.0;
				color_picture.y = 0.0;
				color_picture.z = 0.0;
				break;

			case 1: 			
				color_picture.x = 0.0;
				color_picture.y = 1.0;
				color_picture.z = 0.0;
				break;
			case 2: 			
				color_picture.x = 0.0;
				color_picture.y = 0.0;
				color_picture.z = 1.0;
				break;
			case 3: 			
				color_picture.x = 1.0;
				color_picture.y = 1.0;
				color_picture.z = 0.0;
				break;
			case 4: 			
				color_picture.x = 1.0;
				color_picture.y = 0.0;
				color_picture.z = 1.0;
				break;
			case 5: 			
				color_picture.x = 0.0;
				color_picture.y = 1.0;
				color_picture.z = 1.0;
				break;
			}

			SetModelColor(modelid[scene][i], 3, color_picture);

				
		}

		color_i ++;

	}


}

/*
���ܣ��û�����ע��
�����0��ע��ɹ���-1��ע��ʧ��
*/
int userFuncModelRegedit(USER_MODEL_DRAW_FUNC userLayerDrawFunc, void *userLayerParam)
{
	  funcModelRegedit.userLayerDrawFunc = NULL;
	  funcModelRegedit.userLayerParam = NULL;
    if(NULL == userLayerDrawFunc)
        return(-1);
        
    funcModelRegedit.userLayerDrawFunc = userLayerDrawFunc;
    funcModelRegedit.userLayerParam = userLayerParam;
    return(0);
}

#if 0
/*.BH--------------------------------------------------------
**
** ������: RenderScreenModelFlag
**
** ����:  ����Ļ�ϻ���С��ͼ����עģ�͵�λ��
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�3, used in Flight3d.c
**
**.EH--------------------------------------------------------
*/





/*
����fovx��ֵ, ��λ�Ƕ�,��ʱȡȫ�ֱ���pPaint_unit��ֵ
*/
double GetFovX()
{
	double bta = 0.0;
	double g_Fovy = ((sPAINTUNIT *)pPaint_unit)->pScene->camctrl_param_input.view_angle;

	double	viewport_width = ((sPAINTUNIT *)pPaint_unit)->pViewWindow->viewwd_param_input.outer_view_port.width; //g_SphereRender.m_Render.m_rcView.right - g_SphereRender.m_Render.m_rcView.left;
	double 	viewport_height =((sPAINTUNIT *)pPaint_unit)->pViewWindow->viewwd_param_input.outer_view_port.height;// g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom;	
	bta = 	2* atan(tan(g_Fovy/2 * DE2RA) * viewport_width/viewport_height) * RA2DE;


	

		






	return bta;
}



/*
����������֮��ľ���,��ά�����
*/
double DistanceTwoPoint2D(PT_2D point1, PT_2D point2)
{
	return sqrt( sqr(point1.x - point2.x) + sqr(point1.y - point2.y)	);
}


/*
��һԪ���η���ax*x+bx+c=0�Ľ�
*/
void CalOneParamDoubleTime(double a, double b, double c, double *k1,double *k2, BOOL * isReal)
{
	double d;

	d = b*b-4*a*c;

	if(d <0)
	{
		*isReal = FALSE;
		*k1 = 0;
		*k2 = 0;

		return;
	}
	
	*k1 = (-b +sqrt(d))/(2*a);
	*k2 = (-b -sqrt(d))/(2*a);
	*isReal = TRUE;

	

}

/*
	����ƽ����ֱ����Բ�Ľ��㣬Բ����ԭ��
	����:ֱ�ߵ�б��K�����ĵ�����pointA,  Բ�İ뾶R
	���:����1������2
	ֱ�߱��ʽ: y = kx + d

*/
void CalLineWithRound(PT_2D pointA, double k, double R, 
						LP_PT_2D point_intersection1, 
						LP_PT_2D point_intersection2,
						BOOL * usefull)
{
	double d;
	BOOL flag;

	double y1,x1;
	double x2,y2;

	// 1.����ֱ�߱��ʽ
	d = pointA.y - k*pointA.x;

	// 2.���㽻���x����x1 ��x2
	CalOneParamDoubleTime(sqr(k)+1, 2*k*d, sqr(d) - sqr(R),
		&x1, &x2, &flag);

	// 3.�ж��Ƿ���Ч���
	if(flag == FALSE)
	{
//		printf("�Ӿ��������û�н���\n");
		*usefull = FALSE;
		return;
	}

	// 4.���㽻���y1,y2
	y1 = k * x1 + d;
	y2 = k * x2 + d;

	// 5.��������ֵ
	point_intersection1->x = x1;
	point_intersection1->y = y1;
	point_intersection2->x = x2;
	point_intersection2->y = y2;
	*usefull = TRUE;

}

/*
//����γ���ڸߵķ����ϵ�ƫ����
//����:�ӳ���angle_lat, ģʽmode, 0: ���������mode : 1, ���lon ֵ
//���:��γ�ȵĸ�ֵ�����ϵ�ƫ����
*/
void GeoCalHiLat(double angle_lat, int mode, double *offset_hi_lat)
{
	double a,b,c;				//��ǰ�ӵ����������
	double k,d;				// y = kx + d
	double alpha, bta;			//�Ƕ�ֵ

	BOOL flag;

	double y1,x1=0,z1,lon1,lat1,hei1;

	double lat_hi, lat_low, lon_hi, lon_low;
	PT_2D point_line;				//�ӵ���ƽ���ϵ�ͶӰ����
	PT_2D point1, point2;			//ֱ����Բ�Ľ���

	double viewport_center_latlonhei[3] = {0};	//�ӵ�ĵ�ǰλ��lat, lon, hei

	double lat_range, lon_range;		//γ�Ⱥ;��ȵĿ��

	// 1.��ȡ��ǰ�ӵ����������
	a = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.x;	//g_SphereRender.m_Render.m_ptEye.x;	
	b =	((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.y;//g_SphereRender.m_Render.m_ptEye.y;		
	c = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.z;	//g_SphereRender.m_Render.m_ptEye.z;	

// 	XYZToEarth(a, b, c, 
// 		&viewport_center_latlonhei[1], 
// 		&viewport_center_latlonhei[0],
// 		&viewport_center_latlonhei[2]);

	viewport_center_latlonhei[0] = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_geoptEye.lat;
	viewport_center_latlonhei[1] = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_geoptEye.lon;
	viewport_center_latlonhei[2] = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_geoptEye.height;





	// 2.����lat_hi ����yz ƽ��,�ӵ���yzƽ���ͶӰ������(sqrt(a*a + b*b), c)
	
		// 2.0.����Ƕ�ֵ
		point_line.x = sqrt(sqr(a) + sqr(b));
		point_line.y = c;
		alpha = atan(point_line.y/point_line.x) *RA2DE;
		bta = angle_lat/2;

		// 2.1.����lat_hi
		if(c < 0)
		{
			// ��γ	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//��γ
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// ѡ���һ����
				z1 = point1.y;
			
			}
			else
			{
				// ѡ��ڶ�����
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi �Ӿ��������û�н���\n");
			return;
		}

		y1 = (z1-d)/k;

		if(mode == 0)		//�������lat �ĸ�ֵ
		{
			// 1.�ж�lat ��ֵ�Ƿ�Խ��90��
			// ����ֱ����YZ ƽ��Ľ���ȷ����γ�ȵ��Ƿ�Խ��+/- 90��

			if((d > EARTH_RADIUS)&&(y1 < 0))
			{
				XYZToEarth(0,0,EARTH_RADIUS, &lon1, &lat1, &hei1);

			}
			else if( (d < -EARTH_RADIUS)&&(y1 < 0))
			{
				XYZToEarth(0,0,-EARTH_RADIUS, &lon1, &lat1, &hei1);

			}
			else
			{
				XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);
			}

			lat_hi= RA2DE*(lat1);

			lat_range = lat_hi - viewport_center_latlonhei[0]*RA2DE;

//			printf("lat_range =%f\n", lat_range*2);


		}
		else				//�����ת���lon ֵ
		{
			XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);
			lat_hi= RA2DE*(lat1);

			if((d > EARTH_RADIUS)&&(y1 < 0))
			{
				lat_range = 180.0 - viewport_center_latlonhei[0]*RA2DE - lat_hi;
			}
			else if( (d < -EARTH_RADIUS)&&(y1 < 0))
			{
				lat_range = viewport_center_latlonhei[0]*RA2DE + lat_hi + 180.0;
			}
			else
			{
				lat_range = lat_hi - viewport_center_latlonhei[0]*RA2DE;
			}

//			printf("lon_range =%f\n", lat_range*2);

		}

		// ��֤�����lat_range����ֵ
 		if(lat_range < 0.0)
 		{
 			lat_range *= -1;
 		}

		*offset_hi_lat = lat_range;
}



// ���㽻���뵱ǰ�ӵ�ľ��룬����ת����γ�ȼ���
void GeoCalCollsion2(double * focaspoint)
{
	double a,b,c;				//��ǰ�ӵ����������
	double k,d;				// y = kx + d
	double alpha, bta;			//�Ƕ�ֵ

	BOOL flag;

	double y1,x1=0,z1,lon1,lat1,hei1;

	double lat_hi, lat_low, lon_hi, lon_low;
	PT_2D point_line;				//�ӵ���ƽ���ϵ�ͶӰ����
	PT_2D point1, point2;			//ֱ����Բ�Ľ���

	double viewport_center_latlonhei[3] = {0};	//�ӵ�ĵ�ǰλ��lat, lon, hei

	double lat_range, lon_range;		//γ�Ⱥ;��ȵĿ��

	// 1.��ȡ��ǰ�ӵ����������
// 	a = 	g_SphereRender.m_Render.m_ptEye.x;
// 	b =	g_SphereRender.m_Render.m_ptEye.y;
// 	c =  	g_SphereRender.m_Render.m_ptEye.z;
// 
// 	XYZToEarth(a, b, c, 
// 		&viewport_center_latlonhei[1], 
// 		&viewport_center_latlonhei[0],
// 		&viewport_center_latlonhei[2]);



	a = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.x;	//g_SphereRender.m_Render.m_ptEye.x;	
	b =	((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.y;//g_SphereRender.m_Render.m_ptEye.y;		
	c = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.z;	//g_SphereRender.m_Render.m_ptEye.z;	
		
	viewport_center_latlonhei[0] = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_geoptEye.lat;
	viewport_center_latlonhei[1] = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_geoptEye.lon;
	viewport_center_latlonhei[2] = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_geoptEye.height;





	// 2.����lat_hi, lat_low, ����yz ƽ��,�ӵ���yzƽ���ͶӰ������(sqrt(a*a + b*b), c)
	
		// 2.0.����Ƕ�ֵ
		point_line.x = sqrt(sqr(a) + sqr(b));
		point_line.y = c;
		alpha = atan(point_line.y/point_line.x) *RA2DE;
		bta =  ((sPAINTUNIT *)pPaint_unit)->pScene->camctrl_param_input.view_angle / 2; //g_Fovy/2;

		// 2.1.����lat_hi
		if(c < 0)
		{
			// ��γ	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//��γ
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// ѡ���һ����
				z1 = point1.y;
			
			}
			else
			{
				// ѡ��ڶ�����
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi �Ӿ��������û�н���\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_hi= RA2DE*(lat1);

		lat_range = lat_hi - viewport_center_latlonhei[0]*RA2DE;

		printf("lat_range =%f\n", lat_range*2);

	// 3.����lon_hi, lon_low������xyƽ��

		// 2.0.����Ƕ�ֵ
		bta = GetFovX()/2;

		// 2.1.����lat_hi �ȼ���lon��ƫ��ֵ
		if(c < 0)
		{
			// ��γ	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//��γ
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// ѡ���һ����
				z1 = point1.y;
			
			}
			else
			{
				// ѡ��ڶ�����
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi �Ӿ��������û�н���\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_hi= RA2DE*(lat1);

		lon_range = lat_hi - viewport_center_latlonhei[0]*RA2DE;

		printf("lon_range =%f\n", lon_range*2);

	// 4.�������

	

}



//�����Ӿ��������Ľ���, ���˳��:lat_hi, lat_low, lon_hi, lon_low;
void GeoCalCollsion(double * focaspoint)
{
	double a,b,c;				//��ǰ�ӵ����������
	double k,d;				// y = kx + d
	double alpha, bta;			//�Ƕ�ֵ

	BOOL flag;

	double y1,x1=0,z1,lon1,lat1,hei1;

	double lat_hi, lat_low, lon_hi, lon_low;
	PT_2D point_line;				//�ӵ���ƽ���ϵ�ͶӰ����
	PT_2D point1, point2;			//ֱ����Բ�Ľ���

	// 1.��ȡ��ǰ�ӵ����������
// 	a = 	g_SphereRender.m_Render.m_ptEye.x;
// 	b =	g_SphereRender.m_Render.m_ptEye.y;
// 	c =  	g_SphereRender.m_Render.m_ptEye.z;
	a = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.x;	//g_SphereRender.m_Render.m_ptEye.x;	
	b =	((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.y;//g_SphereRender.m_Render.m_ptEye.y;		
	c = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.z;	//g_SphereRender.m_Render.m_ptEye.z;	


//	printf("a = %f, b = %f, c = %f, R = %f\n",a,b,c, sqrt(sqr(a) + sqr(b) + sqr(c)));

	// 2.����lat_hi, lat_low, ����yz ƽ��,�ӵ���yzƽ���ͶӰ������(sqrt(a*a + b*b), c)
	
		// 2.0.����Ƕ�ֵ
		point_line.x = sqrt(sqr(a) + sqr(b));
		point_line.y = c;
		alpha = atan(point_line.y/point_line.x) *RA2DE;
		bta = ((sPAINTUNIT *)pPaint_unit)->pScene->camctrl_param_input.view_angle /2;

		// 2.1.����lat_hi
		if(c < 0)
		{
			// ��γ	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//��γ
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// ѡ���һ����
				z1 = point1.y;
			
			}
			else
			{
				// ѡ��ڶ�����
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi �Ӿ��������û�н���\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_hi= RA2DE*(lat1);

		// 2.2.����lat_low
		if(c<0)
		{
			// ��γ	
			k = tan((alpha - bta)* DE2RA + PI);
		}
		else
		{
			//��γ
			k = tan((alpha + bta)* DE2RA);		
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// ѡ���һ����
				z1 = point1.y;
			
			}
			else
			{
				// ѡ��ڶ�����
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_low �Ӿ��������û�н���\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_low = RA2DE*(lat1);

		printf("lat_hi = %f, lat_low = %f, lat_range = %f\n",lat_hi, lat_low, lat_hi - lat_low);		

	// 3.����lon_hi, lon_low������xyƽ��
		// 3.0.����Ƕ�ֵ
		if((a > 0 ) &&(b > 0 ))  
		{
			alpha = atan(b/a)*RA2DE;
		}
		else	if(( a < 0 )&&(b >0) )
		{
			alpha = atan(b/a)*RA2DE + 180.0;		
		}
		else	if ((a < 0)&&(b < 0))
		{
			alpha = atan(b/a)*RA2DE + 180.0;

		}
		else	if((a>0)&&(b<0))
		{
			alpha = atan(b/a)*RA2DE + 360.0;

		}
		
		bta = GetFovX()/2;

		point_line.x = sqrt(sqr(a) + sqr(b) + sqr(c)) * cos(alpha * DE2RA);
		point_line.y = sqrt(sqr(a) + sqr(b) + sqr(c)) * sin(alpha * DE2RA);

		// 3.1. ����lon_hi
		while(1)
		{


			//�������޺͵ڶ�����
			if(	((a < 0)&&(b < 0))	||((a>0)&&(b<0)))
			{
				k = tan((alpha + bta)* DE2RA);

			}
			else
			{
				k = tan((alpha - bta)* DE2RA);
			}


			
			d = point_line.y - k * point_line.x;

			CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

			if(flag == TRUE)
			{
				//������С�ĵ�����Ҫ�ĵ�
				// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
				double distance1 = 0.0, distance2 = 0.0;

				distance1 = DistanceTwoPoint2D(point1, point_line);
				distance2 = DistanceTwoPoint2D(point2, point_line);

				if(distance1 < distance2)
				{
					// ѡ���һ����
					y1 = point1.y;
				
				}
				else
				{
					// ѡ��ڶ�����
					y1 = point2.y;

				}

			}
			else
			{
	//			printf("lon_hi �Ӿ��������û�н���\n");

				bta -= 1;
				continue;
			}

			x1 = (y1-d)/k;
			
			XYZToEarth(x1,y1,0, &lon1, &lat1, &hei1);

			lon_hi= RA2DE*(lon1);

			break;


		}

	// 3.2. ����lon_low

	bta = GetFovX()/2;

	while(1)
	{

		//�������޺͵ڶ�����
		if(	((a < 0)&&(b < 0))	||((a>0)&&(b<0)))
		{
			k = tan((alpha - bta)* DE2RA);

		}
		else
		{
			k = tan((alpha + bta)* DE2RA);
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// ���ݽ�����ͶӰ��ľ����ж�ѡȡ��һ���㣬����С�ĵ��ǽ���
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// ѡ���һ����
				y1 = point1.y;
			
			}
			else
			{
				// ѡ��ڶ�����
				y1 = point2.y;

			}
		}
		else
		{
//			printf("lon_low �Ӿ��������û�н���\n");
			bta -= 1;
			continue;
//			return;
		}

		x1 = (y1-d)/k;
		
		XYZToEarth(x1,y1,0, &lon1, &lat1, &hei1);

		lon_low = RA2DE*(lon1);

		break;


	}

		// 
		printf("lon_hi = %f, lon_low = %f, lon_range = %f\n",lon_hi, lon_low, lon_hi - lon_low);				


	// 4.�������
	* focaspoint = lat_hi;
	* (focaspoint + 1) = lat_low;
	* (focaspoint + 2) = lon_hi;
	* (focaspoint + 3) = lon_low;
	

}


/*
**����:�����ӿ���С��ͼ�ϵľ��ο�
**����:�ɻ���С��ͼ�ϵ�λ�ã����ο򳤺Ϳ��һ�����
**���:
*/
void RenderScreenRectangle(int x_center, int y_center, float halfWidth,float halfheight, float angle)
{
	int  col, row;
	float x1, y1, x2, y2, x3, y3, x4, y4;
	float rx1, ry1, rx2, ry2, rx3, ry3, rx4, ry4;
	float fArc, sina, cosa;
	
	x1 = -halfWidth;    y1 = -halfheight;
	x2 = -halfWidth;	y2 = halfheight;
	x3 = halfWidth;	    y3 = halfheight;
	x4 = halfWidth;	    y4 = -halfheight;
	
	rx1 = x1 ;
	ry1 = y1 ;
	rx2 = x2 ;
	ry2 = y2 ;
	rx3 = x3 ;
	ry3 = y3  ;
	rx4 = x4  ;
	ry4 = y4 ;

	fArc = angle * DE2RA;
    	sina = sin(fArc);
	cosa = cos(fArc);

	rx1 = x1 * cosa + y1 * sina;
	ry1 = y1 * cosa - x1 * sina;

	rx2 = x2 * cosa + y2 * sina;
	ry2 = y2 * cosa - x2 * sina;

	rx3 = x3 * cosa + y3 * sina;
	ry3 = y3 * cosa - x3 * sina;

	rx4 = x4 * cosa + y4 * sina;
	ry4 = y4 * cosa - x4 * sina;

	

//	glDisableEx(GL_CULL_FACE);
	glColor3f(1.0,1.0,0.0);
	glLineWidth(1.0);
	glEnableEx(GL_LINE_SMOOTH);
	glBegin(GL_LINE_LOOP);
		glVertex2f(x_center + rx2, y_center + ry2);
		
		glVertex2f(x_center + rx3, y_center + ry3);										
		
		glVertex2f(x_center + rx4, y_center + ry4);					
		
		glVertex2f(x_center + rx1, y_center + ry1);						
	glEnd();	
	
	glDisableEx(GL_LINE_SMOOTH);
//	glEnableEx(GL_CULL_FACE);
	glLineWidth(1.0);
	
}

// extern sFLYPARAM g_FlyParam ;  //��ǰ���в���_�ӿ���
// extern sCTRPARAM3D g_CtrlParam;
// extern unsigned int g_MeasureSemiPixelNum ;// ���̻��뾶���أ���ά��ͼʹ��

void RenderPlaneRectange(f_float64_t width, f_float64_t height)
{
	int plane_x, plane_y;		
	float plane_halfwidth,plane_halfheight;				//С��ͼ�еľ��ο��С

	float viewport_world_width,viewport_world_height;	//�ӿڵ���������ϵ�µĴ�С

	float viewport_width,viewport_height;					//�ӿڵ����ش�С

	// 1.��ȡ�ӿڵ����ش�С
	viewport_width = g_SphereRender.m_Render.m_rcView.right - g_SphereRender.m_Render.m_rcView.left;
	viewport_height = g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom;	

	// 2.�����ӿڵ������С
	viewport_world_width = g_CtrlParam.base.measure_distance / g_MeasureSemiPixelNum * viewport_width;
	viewport_world_height = g_CtrlParam.base.measure_distance / g_MeasureSemiPixelNum * viewport_height;
	
	// 3.����С��ͼ�ľ��ο��С
	plane_halfwidth =  viewport_world_width /(2*PI*EARTH_RADIUS) *width;
	plane_halfheight = viewport_world_height /(2*PI*EARTH_RADIUS)*height;
	
	// 4.������ο������λ����С��ͼ�е�
	{

		double viewport_center_latlonhei[3] = {0};

		XYZToEarth(
		 	g_SphereRender.m_Render.m_ptEye.x,
			g_SphereRender.m_Render.m_ptEye.y,
		  	g_SphereRender.m_Render.m_ptEye.z,
			&viewport_center_latlonhei[1], 
			&viewport_center_latlonhei[0],
			&viewport_center_latlonhei[2]);


		plane_x = (viewport_center_latlonhei[1]*RA2DE  + 180.0)/360.0*width;
		plane_y= (viewport_center_latlonhei[0]*RA2DE + 90.0)/180.0*height;

	}



	

	
//	RenderScreenRectangle(plane_x, plane_y, plane_halfwidth, plane_halfheight);


{
	double pointLatLon[4] = {0};			//�ĸ�����
	double pointAround[4][2] = {0};			//�ĸ����������������
	double pointAroundMct[4][2] = {0};		//�ĸ���������Mct
	double pointAroundSmal[4][2] = {0};		//�ĸ���������С��ͼ
	int  i = 0;
	double viewport_latlon[2] = {0};			//�ĸ�����ķ�Χ,lat lon
	
	#if 0
	// 1.�����Ӿ��������Ľ��㣬��������
	GeoCalCollsion(pointLatLon);
	GeoCalCollsion2(pointLatLon);
	
	// 2.��γ�ȵõ���Χ����
	viewport_latlon[0] = fabs(pointLatLon[1] - pointLatLon[0]) ;
	viewport_latlon[1] = fabs(pointLatLon[3] - pointLatLon[2]) ;

	#else

	GeoCalHiLat(g_Fovy, 0, &viewport_latlon[0]);
	GeoCalHiLat(GetFovX(), 1,  &viewport_latlon[1]);


	viewport_latlon[0] = viewport_latlon[0] *2;
	viewport_latlon[1] = viewport_latlon[1] *2;


	#endif

	plane_halfheight =  viewport_latlon[0] /(180) *height/2;
	plane_halfwidth = viewport_latlon[1] /(360)*width/2;


	RenderScreenRectangle(plane_x, plane_y, plane_halfwidth, plane_halfheight, g_FlyParam.yaw);	

}




}

/*
���ܣ�����ɻ�����
���룺
    x_center��y_center �ɻ���������λ��
    symbol_index ��������
    angle ��ת��
�����
    ��
*/
void outPutPlaneSymbolEx(int x_center, int y_center, int symbol_index, float angle)
{
	int  col, row;
	float x1, y1, x2, y2, x3, y3, x4, y4;
	float rx1, ry1, rx2, ry2, rx3, ry3, rx4, ry4;
	float fArc, sina, cosa;
	float halfWidth = 20.0f;
	
	x1 = -halfWidth;    y1 = -halfWidth;
	x2 = -halfWidth;	y2 = halfWidth;
	x3 = halfWidth;	    y3 = halfWidth;
	x4 = halfWidth;	    y4 = -halfWidth;
	
	fArc = angle * DE2RA;
    sina = sin(fArc);
	cosa = cos(fArc);

	rx1 = x1 * cosa + y1 * sina;
	ry1 = y1 * cosa - x1 * sina;

	rx2 = x2 * cosa + y2 * sina;
	ry2 = y2 * cosa - x2 * sina;

	rx3 = x3 * cosa + y3 * sina;
	ry3 = y3 * cosa - x3 * sina;

	rx4 = x4 * cosa + y4 * sina;
	ry4 = y4 * cosa - x4 * sina;
	
	row = 7 - symbol_index / 8;
	col = symbol_index % 8;
	
	glBegin(GL_QUADS);
		glTexCoord2f(col/8.0f, row/8.0f);   
	    //glTexCoord2f(0.0f,0.0f);
		glVertex2f(x_center + rx2, y_center + ry2);
		
		glTexCoord2f(col/8.0f+1.0f/8.0f, row/8.0f);  
		//glTexCoord2f(1.0f,0.0f);
		glVertex2f(x_center + rx3, y_center + ry3);										
		
		glTexCoord2f(col/8.0f+1.0f/8.0f, row/8.0f+1.0f/8.0f);  
		//glTexCoord2f(1.0f,1.0f);
		glVertex2f(x_center + rx4, y_center + ry4);					
		
		glTexCoord2f(col/8.0f, row/8.0f+1.0f/8.0f);	  
		//glTexCoord2f(0.0f,1.0f);
		glVertex2f(x_center + rx1, y_center + ry1);						
	glEnd();	
}

extern int m_textureID[2];
void RenderModelWorldFlag()
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

   	glEnableEx(GL_TEXTURE_2D);
	glEnableEx(GL_BLEND);	 
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, m_textureID[0]);
	glColor4f(0.0f, 0.78f, 0.0f,1.0f);
	glDisableEx(GL_CULL_FACE);

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.����ÿ��ģ�͵�λ�û��Ʒ���
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);


			// 2.�ж�ģ���Ƿ���ʾ
			if(lpsinglemodel->m_flagModelScreen == TRUE)
			{

				// 3.����ģ�͵�С��ͼ�������
				outPutPlaneSymbolEx((int)(lpsinglemodel->m_ModelScreenXY.x), 
					(int)(lpsinglemodel->m_ModelScreenXY.y), 
					lpsinglemodel->m_ModelScreenID, 
					180 + lpsinglemodel->m_modelAngle.x);   //����ɻ����� 
				
			}
			
		}
	}
	
	giveSem(g_SetModelParam);
	
	//����Ϊ�˻�ԭĬ�ϵ�����ӳ�䷽ʽ
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_BLEND);
	glDisableEx(GL_TEXTURE_2D);
	glEnableEx(GL_CULL_FACE);
		
}


/*
**����:������Ļ�ϵ�С��ͼ,Ĭ��С��ͼ�����½�Ϊ��������0,0
**����:С��ͼ�Ŀ�Ⱥ͸߶�
*/
static int WorldPicID = 0;		//�����ͼ��ͼƬ������ID
static int WorldPicListID = 0;	//���������ͼ����ʾ�б�ID

void RenderScreenWorldPic(f_float64_t inwidth, f_float64_t inheight)
{
	f_int16_t left, right, bottom, top;
	f_int16_t width, height;

#ifdef WIN32	
	f_char_t filepath[256] = "../../DataFile/model/world/world.bmp";
#else
	f_char_t filepath[256] ="/ahci00/mapData3d/DataFile/model//world/world.bmp";
#endif
	width = g_SphereRender.m_Render.m_rcView.right - g_SphereRender.m_Render.m_rcView.left;
	height = g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom;
	left = g_SphereRender.m_Render.m_rcView.left;
	right = g_SphereRender.m_Render.m_rcView.right;
	bottom = g_SphereRender.m_Render.m_rcView.bottom;
	top = g_SphereRender.m_Render.m_rcView.top;

	// 1.��ȡworld ͼƬ,��������ID
	if (WorldPicID == 0)
	{
		LoadParticlesTexture(filepath, &WorldPicID);
//		LoadTexture(filepath, &WorldPicID, 0);
	}

	// 2.�������½�inwidth * inheight ��С��ͼ
	glPushAttrib(GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT|GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glViewport(left, bottom, width, height);

	glEnableEx(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);		//gl_replace -> gl_blend
	glBindTexture(GL_TEXTURE_2D, WorldPicID);

	glDisableEx(GL_DEPTH_TEST);

	// 3.ʹ����ʾ�б�
	if(WorldPicListID == 0)
	{
		WorldPicListID = glGenLists(1);
		glNewList(WorldPicListID, GL_COMPILE);

		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex2f(0.0f, 0.0f);

			glTexCoord2f(1,0);
			glVertex2f(inwidth, 0); 

			glTexCoord2f(1,1);
			glVertex2f(inwidth,inheight);

			glTexCoord2f(0,1);
			glVertex2f(0.0f, inheight);       
		glEnd();

		glEndList();
	}
	else
	{
		glCallList(WorldPicListID);

	}

	glBindTexture(GL_TEXTURE_2D,0);
	glDisableEx(GL_TEXTURE_2D);

	//����Ϊ�˻�ԭĬ�ϵ�����ӳ�䷽ʽ
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// 3.����ģ�͵ķ���
	RenderModelWorldFlag();

	// 4.����С��ͼ���ӿڷ�Χ��
	RenderPlaneRectange(inwidth,  inheight);
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	

	glEnableEx(GL_DEPTH_TEST);
	glPopAttrib();
}

/*
**����:����ģ����С��ͼ�ϵ�λ��
**����:С��ͼ�Ŀ�Ⱥ͸߶�
**���:ģ�͵�С��ͼλ��
*/
void UpdataModelScreenPos(f_float64_t inwidth, f_float64_t inheight)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.����ÿ��ģ�͵�λ��
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);


			// 2.�ж�ģ���Ƿ���ʾ
			if(lpsinglemodel->m_flagModelScreen == TRUE)
			{

				// 3.����ģ�͵�λ�ã�����lon,lat 
				lpsinglemodel->m_ModelScreenXY.x = (lpsinglemodel->m_modelPos.y  + 180.0)/360.0*inwidth;
				lpsinglemodel->m_ModelScreenXY.y = (lpsinglemodel->m_modelPos.x  + 90.0)/180.0*inheight;
			}
			
		}
	}
	
	giveSem(g_SetModelParam);
		
}

//------С��ͼ�ĳߴ�Ľӿں���----------------------
static double worldmapsize[2];
void SetWorldMapSize(double x ,double y)
{
	worldmapsize[0] = x;
	worldmapsize[1] = y;

}
double GetWorldMapSizeX()
{
	return worldmapsize[0];
}
double GetWorldMapSizeY()
{
	return worldmapsize[1];
}
//------------С��ͼ�ĳߴ�Ľӿں���-----------------

// ����С��ͼ����
void RenderScreenModelFlag()
{	
	f_float64_t world_width = 500.0;		//�����ͼ�Ŀ��
	f_float64_t world_height = 250.0;		//�����ͼ�ĸ߶�

	// 0.׼��ģ�͵���Ļλ������
	UpdataModelScreenPos(world_width, world_height);
	
	// 1.������Ļ�ϵ�С��ͼ
	RenderScreenWorldPic(world_width, world_height);

	// 2.����С��ͼ�ĳߴ�
	SetWorldMapSize(world_width, world_height);
	
}
#endif

