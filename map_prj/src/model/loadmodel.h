/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: loadmodel.h
**
** ����: ���ļ���loadmodel.c ��ͷ�ļ�
**
** ����ĺ���:  
**       LoadAirFeatureFile();                      -----------����ģ�͵�������Ϣ�ļ�
**                            
** ���ע��: 
**
** ���ߣ�
**		LPF��
** 
**
** ������ʷ:
**		2015-4-21  14:48  LPF �����ļ���
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#ifndef _LOADMODEL_H_
#define _LOADMODEL_H_


#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include <vxWorks.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"
#include "../engine/frustum.h"
#include "../engine/mapRender.h"
#include "../mapApp/appHead.h"

#include "loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "ModelreadTxtFile.h"
#include "3DSLoader.h"

/*-----------------------------------------------------------
** �ṹ������
**-----------------------------------------------------------
*/
//---ģ����Ϣ�ṹ��
typedef struct tagModelAll
{	
	//ģ����Ϣ
	int id;                     //ģ��id
	char name[256];             //ģ������
	char model3ds[64];          //ģ�͵�3ds�ļ�����
	double model_size;			//ģ�͵ĳߴ�(������뾶)
	
	//ģ������
	Model3DS	pModelMess;     //ģ�;���������Ϣ

	//��ģ����Ϣ
	int child_num;                  //��ģ�͸���,�ݶ�������8��
	int child_id[8];				//��ģ��id
	LPModel3DS child_3ds[8];		//��ģ�͵�3ds����ָ��

	//����ڵ�
	stList_Head stListHead;
} ModelAll, *LPModelAll;

//---���ɵ���Ϣ�ṹ��
typedef struct tagModelImport
{	
	//ģ����Ϣ
	int id;
	char name[256];
	char model3ds[64];
	double lon;
	double lat;
	double height;
	double pitch;
	double roll;
	double yaw;
	double scale;
	double size;

} ModelImport, *LPModelImport;


/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

#if 0
DWORD WINAPI LoadModelConfigFile(LPVOID lpParameter);
#else
BOOL LoadModelConfigFile();
#endif
BOOL ReadModelConfigFile(const f_char_t* chfileName);
BOOL LoadModelData(LPModelAll pModelData);
BOOL RenderModelFeature(int DisplayType);
BOOL CheckModelRender();
// f_float64_t GetPlaneLat();
// f_float64_t GetPlaneLon();
// f_float64_t GetPlaneHei();
// LPFRUSTUM GetFrustumModel();


f_float64_t GetPlaneLat(sGLRENDERSCENE* pModelScene);
//��ȡ�ɻ��ĵ�ǰγ��
f_float64_t GetPlaneLon(sGLRENDERSCENE* pModelScene);
//��ȡ�ɻ��ĵ�ǰ�߶�
f_float64_t GetPlaneHei(sGLRENDERSCENE* pModelScene);

//��ȡ�Ӿ�����Ϣ
LPFRUSTUM GetFrustumModel(sGLRENDERSCENE* pModelScene);
//��ȡ���ý������
f_float32_t GetView_near(sGLRENDERSCENE* pModelScene);
//��ȡԶ�ý������
f_float32_t GetView_far(sGLRENDERSCENE* pModelScene);


//BOOL ModelConfigOut(int** id);
BOOL ModelConfigOut(int* id);

//BOOL RenderModelFeatureDynamic(LPModelRender pModelDynamic);
BOOL RenderModelFeatureDynamic(LPModelRender pModelDynamic , sGLRENDERSCENE* pModelSceneDynamic);

BOOL InitModelData();
BOOL GetModelInitFlag();


//BOOL ModelFeatureAutoRunSend();
//BOOL ModelFeatureAutoRunRecv();
//void ModelDynamicParamSet();

/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/
extern stList_Head	m_listModelData;			//ģ�ͳ�ʼ������

//extern SphereDataEx g_SphereDataEx;
//extern void* GetFeatureByID(AirFeatureType *outType, LPAirFeatureData pAirData, AirFeatureLayerType lType, f_uint32_t uID);


#endif
