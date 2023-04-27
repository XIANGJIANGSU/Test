/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: loadmodel.h
**
** 描述: 本文件是loadmodel.c 的头文件
**
** 定义的函数:  
**       LoadAirFeatureFile();                      -----------读入模型的配置信息文件
**                            
** 设计注记: 
**
** 作者：
**		LPF。
** 
**
** 更改历史:
**		2015-4-21  14:48  LPF 创建文件；
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 头文件引用
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
** 结构体声明
**-----------------------------------------------------------
*/
//---模型信息结构体
typedef struct tagModelAll
{	
	//模型信息
	int id;                     //模型id
	char name[256];             //模型名称
	char model3ds[64];          //模型的3ds文件名称
	double model_size;			//模型的尺寸(外切球半径)
	
	//模型数据
	Model3DS	pModelMess;     //模型具体数据信息

	//子模型信息
	int child_num;                  //子模型个数,暂定不超过8个
	int child_id[8];				//子模型id
	LPModel3DS child_3ds[8];		//子模型的3ds数据指针

	//链表节点
	stList_Head stListHead;
} ModelAll, *LPModelAll;

//---生成的信息结构体
typedef struct tagModelImport
{	
	//模型信息
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
** 函数声明
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
//读取飞机的当前纬度
f_float64_t GetPlaneLon(sGLRENDERSCENE* pModelScene);
//读取飞机的当前高度
f_float64_t GetPlaneHei(sGLRENDERSCENE* pModelScene);

//读取视景体信息
LPFRUSTUM GetFrustumModel(sGLRENDERSCENE* pModelScene);
//获取近裁截面距离
f_float32_t GetView_near(sGLRENDERSCENE* pModelScene);
//获取远裁截面距离
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
** 外部变量声明
**-----------------------------------------------------------
*/
extern stList_Head	m_listModelData;			//模型初始化链表

//extern SphereDataEx g_SphereDataEx;
//extern void* GetFeatureByID(AirFeatureType *outType, LPAirFeatureData pAirData, AirFeatureLayerType lType, f_uint32_t uID);


#endif
