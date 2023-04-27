/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: ModelApi.c
**
** 描述: 本文件包含模型初始化和模型绘制的接口调用函数。
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
**		2016-10-24 9:47 LPF 创建此文件
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** 头文件引用
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
** 类型声明
**-----------------------------------------------------------
*/

/* 用户上层实现功能函数注册 */
typedef struct {
	USER_MODEL_DRAW_FUNC userLayerDrawFunc;   /* 用户图层函数 */
	void *userLayerParam;                     /* 用户图层参数 */
}sFUNCModelREGEDIT;


/*-----------------------------------------------------------
** 文字量和宏声明
**-----------------------------------------------------------
*/
#define MODEL_FLAG		12341234				//模型的标记宏定义
#define TRACE_HALF_HEGHT	20				// 轨迹上下的高度

#define BILLBOARD_CENTER		0x01		/* 中心点*/
#define BILLBOARD_TOP			0x02		/* 中心点在公告板上边*/
#define BILLBOARD_BOTTOM		0x03		/* 中心点在公告板下边*/
#define BILLBOARD_LEFT			0x04		/* 中心点在公告板左边*/
#define BILLBOARD_RIGHT			0x05		/* 中心点在公告板右边*/
#define BILLBOARD_LEFT_BOTTOM	0x06		/* 中心点在公告板左下角*/
#define BILLBOARD_LEFT_TOP		0x07		/* 中心点在公告板左上角*/
#define BILLBOARD_RIGHT_BOTTOM	0x08		/* 中心点在公告板右下角*/
#define BILLBOARD_RIGHT_TOP		0x09		/* 中心点在公告板右上角*/


/*-----------------------------------------------------------
** 全局变量定义
**-----------------------------------------------------------
*/
static stList_Head *pModelRenderList = NULL;										//存储绘制模型的链表
static f_float32_t s_nColorInterval = 0.5 * 1.0 / FLYTRACE_NODENUM_MAX;	//轨迹透明度设置
//局部变量定义
static sFUNCModelREGEDIT funcModelRegedit;
static SEMID g_SetModelParam;

TTFONT g_TTFFont = NULL;		// TTF字体
int modelid[SCENE_NUM][81] = {0};

/*-----------------------------------------------------------
** 外部变量声明
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 内部函数声明
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
** 函数定义
**-----------------------------------------------------------
*/

/*.BH--------------------------------------------------------
**
** 函数名: CreateList
**
** 描述:  初始化链表头节点
**
** 输入参数:  头节点的地址
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口1,  used in out
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
		//字体创建失败
		printf("创建TTF字体失败！\n");
		return FALSE;
	}
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: BOOL InitModel()
**
** 描述:  初始化模型数据、白模城市、模型字体及启动判断模型是否可见的调度任务
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  
**
**.EH--------------------------------------------------------
*/
BOOL InitModel()
{
	int i = 0;
	// 0.初始化信号量
	//for(i=0; i<SCENE_NUM; i++)
	{
		g_SetModelParam = createBSem(Q_FIFO_SEM, FULL_SEM);
		giveSem(g_SetModelParam);
	}
	
	// 0.初始化模型绘制链表,每个场景句柄对应一个
	for(i=0;i<SCENE_NUM;i++)
	{
		AttachpScene2Model(pScene[i]);

		CreateList(pModelRenderList);
	}
	
	// 1.读取模型信息
//#ifndef WIN32
	if(FALSE == InitModelData())
	{
		return FALSE;
	}

//#else
	//修改为建立初始化进程,嵌入式下测试后取消这个方法	
//	taskSpawn("ModelInit", 105, VX_FP_TASK | VX_NO_STACK_FILL, 0x100000, (FUNCPTR)InitModelData, 
//	           0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

//#endif

	//  2.读取白模城市信息
	if(FALSE == InitWhiteCityData())
	{
		return FALSE;
	}

#if 1
	// 3.增加模型的航线记录信息初始化
	AirLineRead();
#endif
	

	// 4.初始化字体
	if(FALSE == InitAnnotationFont(ModelFntPath))
	{
		return FALSE;

	}

	// 5.调度任务
	InitModelThread();
	
	return TRUE;
}

BOOL GetWhiteCityDrawOrNot(sGLRENDERSCENE* pModelScene)
{
	double whitecity_lonlat[2] = {120.4628625881- 16.530377, 22.6010411645 + 8.113};
	//视点高度大于2000m不画
	if (pModelScene->camParam.m_geoptEye.height > 5000)
	{
		return FALSE;
	}
	
	//大圆距离大于2000m不画
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

	//视点高度大于2000m不画
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
** 函数名: BOOL RenderModel(BOOL SelectModel, sGLRENDERSCENE* pModelScene)
**
** 描述:  绘制模型，正常模式和选择模式
**
** 输入参数:  SelectModel;			//TRUE-选择模型的绘制方式,FALSE-正常模式绘制模型
**            pModelScene           //渲染场景句柄
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口2, used in sphereRender.c
**
**.EH--------------------------------------------------------
*/
static int testa,testb,testc, testd,teste;	//2017-2-16 11:49:31 调试用
int testk = 1,testk2 = 1;
BOOL RenderModel(BOOL SelectModel, sGLRENDERSCENE* pModelScene)
{

	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;
	ModelRender	SingleModel = {0};	//存储父模型内容的结构体

	double z0 = 0.0;
	double radius = 100.0;			//默认模型半径是100m
	double dis = 0.0;

	LPModelRender lpchildmodel = NULL;	//存储子模型内容的指针
	PT_3D model_world = {0};			//world x,y,z

	int i = 0;							//绘制的模型个数

#ifdef WIN32
	if(SelectModel == TRUE)
		glLoadName(0);  //置换掉名称堆栈的栈顶值，选择反馈机制	
#endif

	// lpf add 关闭光源0,在三维时打开过,目的是在防止8860板子上影响光源1
	EnableLight0(FALSE);

	testa = tickCountGet();
	if((testk == 1)&&(GetWhiteCityDrawOrNot(pModelScene) == TRUE))
	{
		// 1.绘制白模城市
		RenderWhiteCity(pModelScene);					
	}

	testb = tickCountGet();

	// 2.1.判断模型初始化是否完成
	if(FALSE == GetModelInitFlag())
	{
		return FALSE;
	}

	// 2.2.判断开关是否开启,始终开启
	if(CheckModelRender() == FALSE)  
	{
		return FALSE;
	}

	// 2.3.设置飞机模型的绘制参数，位置为飞机位置
	RenderPlanes(pModelScene);			

	// 2.4.判断机场是否绘制,暂时未实现
	//SetAirportModelDisapear(pModelScene);

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 2.5.遍历模型绘制链表,计算模型的位置	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
	
			lpsinglemodel->m_selectMode = SelectModel;

			// 2.5.1.根据高度模式计算模型的绝对高度
			if(lpsinglemodel->m_heimode == 1)		//世界坐标系高度
			{
				z0 = - EARTH_RADIUS;
			}
			else if(lpsinglemodel->m_heimode == 0)	//离地高度
			{
				//测量当地海拔高度	, lpf 2017-5-8 13:56:14 暂时注释
//				if(FALSE == GetZ(lpsinglemodel->m_modelPos.y, lpsinglemodel->m_modelPos.x,&z0))
				{
					z0 = 0.0;	//默认当地点的海拔高度为0
				}
			}
			else if(lpsinglemodel->m_heimode == 2)	//海拔高度
			{
				z0 = 0.0;
			}
			else		//其他情况,跳出循环不处理
			{	
				break;
			}

			// 2.5.2.经纬度坐标转换为世界坐标	
			{
				Geo_Pt_D geo_pt;

				geo_pt.lat = lpsinglemodel->m_modelPos.x;
				geo_pt.lon = lpsinglemodel->m_modelPos.y;
				geo_pt.height = z0 + lpsinglemodel->m_modelPos.z;
				
				geoDPt2objDPt(&geo_pt, &model_world);
			}
			
			// 2.5.3.与链表中的值比较，不一样存储并矩阵计算标志置1
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
	// 2.6.根据显示方式分别绘制
 	glEnableEx(GL_TEXTURE_2D);
	//解决某些三维面绘制不出来的问题，默认是Disable cull face
 	glDisableEx(GL_CULL_FACE);
	//开启深度检测
 	glEnableEx(GL_DEPTH_TEST);	

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// 2.6.1.显示方式是：0-正常显示，1-闪动，6-纯色纹理
	RenderModelType1(pModelScene);
	
	// 2.6.2.显示方式是：2-线框(正常大小)，11以上(含11)-粒子效果,暂未实现
	RenderModelType2(pModelScene);
	
	// 2.6.3.显示方式是：3-模型+线框(1.02倍大小)
	RenderModelType3(pModelScene);

	// 2.6.4.显示方式是：4-模型+包络线(正常大小,线粗3倍)
	RenderModelType4(pModelScene);

	// 2.6.5.显示方位是：5-光环效果,即模型+纯色(1.05倍大小指定颜色)
	RenderModelType5(pModelScene);

	// 2.6.6.显示方式是：7-10：未定义，按0-正常模式绘制
	RenderModelType6(pModelScene);

 	glDisableEx(GL_TEXTURE_2D);

	// 2.7.绘制默认的用户图层,选择模式下不绘制该图层
	if(SelectModel == FALSE)
	{
		// 2.7.1.尾迹
  		RenderUserFlyTrace(pModelScene);

		// 2.7.2.字体
  		RenderUserTxt(pModelScene);

		// 2.7.3.透明长方体图层
		RenderUserRectangle(pModelScene);
			
		// 2.7.4.绘制用户给定的图层
		RenderUserDiy(pModelScene);

	}
	
	//关闭cull face
 	glDisableEx(GL_CULL_FACE);				
 	glDisableEx(GL_DEPTH_TEST);
#endif

#if 0
	// 2.8.恢复模型的m_drawOrNot 为 FALSE
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
** 函数名: InitModelThread
**
** 描述:  初始化任务，启动判断模型是否绘制的任务
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口，在InitModel中建立任务
**
**.EH--------------------------------------------------------
*/
void InitModelThread()
{
	int i = 0;
	char name_task[32] = {0}; 

	//每个视口启动一个任务
	for(i=0; i< SCENE_NUM; i++)
	{
		sprintf(name_task, "loadmodel%d",i);
		
		spawnTask(name_task, 103, FP_TASK_VX | NO_STACK_FILL_VX, 0x400000, LoadModelDataThreadFunc,  (f_int64_t)pScene[i]);

	}



}


/*.BH--------------------------------------------------------
**
** 函数名: LoadModelDataThreadFunc
**
** 描述:  判断模型是否绘制的任务,计算模型包围球是否在场景的视景体内，在则绘制，不在则不绘制
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口，在InitModelThread中建立任务
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
	double radius = 100.0;			//默认模型半径是100m
	double dis = 0.0;
	int i = 0;							//绘制的模型个数

	while(1)
	{
	
#ifdef WIN32

#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
		usleep(1000);
#else
		taskDelay(1);
#endif	
		i = 0;

		// 2.1.判断模型初始化是否完成
		if(FALSE == GetModelInitFlag())
		{
			continue;
		}

		// 2.2.判断开关是否开启,始终开启
		if(CheckModelRender() == FALSE)  
		{
			continue;
		}
				
		takeSem(g_SetModelParam, FOREVER_WAIT);

		// 2.3.0.默认模型的m_drawOrNot为FALSE
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

				lpsinglemodel->m_drawOrNot = FALSE;

			}
		}
		
		// 2.3.1 判断绘制模型的个数，并赋予绘制标记	
		pstListHead = &(pModelScene->pScene_ModelList);

		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
				// 2.3.2.模型包围球半径=模型缩放比例 * 模型尺寸
				radius = (lpsinglemodel->m_modelScale) * (lpsinglemodel->m_modelSize);

				// 2.3.3.判断模型包围球是否在视景体内
				if (InFrustum(GetView_near(pModelScene), GetView_far(pModelScene),
							lpsinglemodel->m_modelPoxWorld.x, 
							lpsinglemodel->m_modelPoxWorld.y, 
							lpsinglemodel->m_modelPoxWorld.z, radius, GetFrustumModel(pModelScene), &dis) == TRUE)									
				{	
					//testk2变量测试使用
					if(testk2 == 1)
					{		
						// 2.3.4.标记模型需要绘制
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
** 函数名: AttachpScene2Model
**
** 描述:  把场景里的链表头节点赋值给modelapi 的局部变量
**
** 输入参数:  场景变量
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口3，用户层调用
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
** 函数名: NewModel
**
** 描述:  新增模型显示，在链表中添加节点，并返回ID
**
** 输入参数:  modeltype 模型类型id
**
** 输出参数：无
**
** 返回值：模型ID
**          
**
** 设计注记:  外部接口3，用户层调用
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

	// 1.1准备结构体的数据, 生成模型的ID
	// 申请内存时会对内存区域进行memset操作，故模型的显示类型为0-正常显示
	pData = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));	

	pData->m_modelID = (int)pData;					//模型ID,采用指针作为ID
	pData->m_modeltype = modeltype;				    //模型类型
	pData->m_modelScale = 1.0;						//缩放比例默认是1；
	pData->m_modelFlag = MODEL_FLAG;				//模型的标记赋值
	pData->m_MatrixChange = TRUE;
	pData->m_heimode = 0;							//模型的高度类型默认是0,相对高度
	pData->m_drawOrNot = FALSE;						//是否绘制该模型，默认是不绘制

	pData->m_flagModelScreen = TRUE;				//模型的小地图显示默认打开
	pData->m_ModelScreenID = 13;					//模型的小地图符号默认是五角形

	vector4DMemSet(&pData->m_color[0] , 0.0f, 1.0f, 0.0f, 1.0f);	//线框默认颜色:绿色
	vector4DMemSet(&pData->m_color[1] , 1.0f, 0.0f, 0.0f, 1.0f);	//屏幕包围框颜色:红色
	vector4DMemSet(&pData->m_color[2] , 0.72f, 0.36f, 0.18f,0.50f);	//光环模式的颜色:金黄色
	vector4DMemSet(&pData->m_color[3] , 1.0f, 0.0f, 0.0f, 1.0f);	//纯色模式的颜色:红色

	//设置生成纯色的纹理ID的标志位
	pData->m_colorPictureIsNeedRebuild = 1;
	
	// 1.2 查找3ds数据，根据type 选择模型对象
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

	//判断是否存在数据
	if(&(pData3ds->pModelMess) == NULL)
	{	
		DeleteAlterableMemory(pData);
		giveSem(g_SetModelParam);
		return FALSE;	
	}

	//复制3ds 数据
	pData->m_pModel3ds = &pData3ds->pModelMess;
	//复制模型尺寸
	pData->m_modelSize = pData3ds->model_size;
	
	//最多支持4行文字
	memset(pData->m_pString[0],0,sizeof(pData->m_pString[0]));
	memset(pData->m_stringNum,0,sizeof(pData->m_stringNum));
	//模型type转换为宽字符，并存储在第一行
	sprintf(text_char, "%d",pData->m_modeltype);
	ttfAnsi2Unicode(pData->m_pString[0], text_char, &pData->m_stringNum[0]);

	sprintf(text_char, "%s", "敌机");
	ttfAnsi2Unicode(pData->m_pString[1], text_char, &pData->m_stringNum[1]);

	sprintf(text_char, "%s", "危险等级");
	ttfAnsi2Unicode(pData->m_pString[2], text_char, &pData->m_stringNum[2]);

	sprintf(text_char, "%s", "距离");
	ttfAnsi2Unicode(pData->m_pString[3], text_char, &pData->m_stringNum[3]);

	// 1.3 获取该模型的子节点(子模型)个数
	pData->m_childNum = pData3ds->child_num;
	
	//生成子模型链表的头节点
//	if(pData->m_childNum > 0)
	{
		CreateList(&pData->m_childlist);		
	}

	// 1.4 创建该模型的子节点(子模型),并设置属性
	for(i = 0; i<pData->m_childNum; i++)
	{

		// 1.4.1增加子模型的初始化
		// 准备结构体的数据, 生成模型的ID
		pChildData = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));

		memcpy(pChildData,pData,sizeof(ModelRender));

		pChildData->m_modelID = (int)pChildData;				    //模型ID,采用指针作为ID
		pChildData->m_modeltype = pData3ds->child_id[i];			//模型类型
		//子节点的pos,angle默认是0
		pChildData->m_modelPos.x = 0;
		pChildData->m_modelPos.y = 0;
		pChildData->m_modelPos.z = 0;
		
		pChildData->m_modelAngle.x = 0;
		pChildData->m_modelAngle.y = 0;
		pChildData->m_modelAngle.z = 0;

		pChildData->m_modelScale = 1;				//默认值1，此值为比例，实际值= 此值*父节点放大倍数

		pChildData->m_MatrixChange = TRUE; 

		pChildData->m_childNum = 0;				//默认只有一级子模型
		
		// 1.4.2 复制3ds 数据
		pChildData->m_pModel3ds = pData3ds->child_3ds[i];

		// 1.4.3把子节点放入链表中
		{
		    /**挂到头结点之后**/
		 	stList_Head *pstListHead = NULL;
			pstListHead = &(pData->m_childlist);
			LIST_ADD(&pChildData->stListHead, pstListHead);	
	 	}
	
	}
	
	// 2.模型绘制链表中添加节点
	 {
	    /**挂到头结点之后**/
	 	stList_Head *pstListHead = NULL;
		pstListHead = (pModelRenderList);
		LIST_ADD(&pData->stListHead, pstListHead);	
	 }
	
	giveSem(g_SetModelParam);

	// 3.返回ID
	return pData->m_modelID;
}
/*.BH--------------------------------------------------------
**
** 函数名: SetModelPos
**
** 描述:  设置模型的lat, lon, height, 单位: 角度, 角度, 米
**
** 输入参数:  模型的ID，模型的位置
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口4，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelPos(f_int32_t modelID, PT_3D latlonhei)
{
	int nodeID = 0;
//	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;
// 		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	int i = 0;
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		SetPT3D(latlonhei, &(lpsinglemodel->m_modelPos));

		// 3.设置矩阵改变标志位
		lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
// 	DeleteAlterableMemory(lpsinglemodel);

	return ret;
}
/*.BH--------------------------------------------------------
**
** 函数名: SetModelAngle
**
** 描述:  设置模型的yaw,pitch,roll.单位是角度
**
** 输入参数:  模型的ID，模型的角度
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口5，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelAngle(f_int32_t modelID, PT_3D yawpitchroll)
{
	LPModelRender lpsinglemodel = NULL;
// 		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改相关内容
		SetPT3D(yawpitchroll, &lpsinglemodel->m_modelAngle);

		// 3.设置矩阵改变标志位
		lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
// 	DeleteAlterableMemory(lpsinglemodel);

	return ret;
	
}
/*.BH--------------------------------------------------------
**
** 函数名: SetModelScale
**
** 描述:  设置模型的缩放系数
**
** 输入参数:  模型的ID，缩放系数
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口6，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelScale(f_int32_t modelID, f_float64_t modelscale)
{
	LPModelRender lpsinglemodel = NULL;
// 		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改相关内容	
		lpsinglemodel->m_modelScale = modelscale;

		// 3.设置矩阵改变标志位
		lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
// 	DeleteAlterableMemory(lpsinglemodel);

	return ret;
}
/*.BH--------------------------------------------------------
**
** 函数名: SetModelDisplay
**
** 描述:  设置模型的显示方式
**
** 输入参数:  模型的ID，显示方式 ,0：正常显示，1：闪动，2：线框(正常大小,颜色默认为绿色)，
** 3：模型+线框(1.02倍大小,颜色默认为绿色)，4：模型+包络线(正常大小,线粗3倍,颜色默认为红色)，
** 5：光环效果,即模型+纯色(1.05倍大小,颜色默认为金黄色)，6：纯色纹理(颜色默认为红色)
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口7，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelDisplay(f_int32_t modelID, f_int32_t modeldisplaytype)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改显示方式
		set_model_displaytype(lpsinglemodel, modeldisplaytype);

		// 3.设置矩阵改变标志位
//		lpsinglemodel->m_MatrixChange = TRUE;
	}

//	DeleteAlterableMemory(lpsinglemodel);
	giveSem(g_SetModelParam);
	return ret;
	
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelColorNumuse
**
** 描述:  设置模型的显示方式
**
** 输入参数:  模型的ID，显示方式
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口7，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelColorNumuse(f_int32_t modelID, f_int32_t modelColorNum_use)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改显示方式
		set_model_colorNumUse(lpsinglemodel, modelColorNum_use);

		// 3.设置矩阵改变标志位
//		lpsinglemodel->m_MatrixChange = TRUE;
	}

//	DeleteAlterableMemory(lpsinglemodel);

	giveSem(g_SetModelParam);
	return ret;	
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelShield
**
** 描述:  设置模型的屏蔽标志
**
** 输入参数:  模型的ID，屏蔽标志flag
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口7，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelShield(f_int32_t modelID, BOOL shieldflag)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改显示方式
		set_model_shield(lpsinglemodel, shieldflag);
	}
	giveSem(g_SetModelParam);
	return ret;
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelUser
**
** 描述:  设置模型的用户图层显示标志,尾迹、文字、长方体，自定义
**
** 输入参数:  模型的ID，显示标志flag，显示图层mode
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口7，用户层调用,不包含用户图层
**
**.EH--------------------------------------------------------
*/
BOOL SetModelUser(f_int32_t modelID, BOOL userflag, int mode)
{
	int nodeID = 0;
// 	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.查找节点内容
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.修改显示方式
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
	// 1.查找节点内容
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.修改显示方式
	for(i=0;i<3;i++)
	{
		set_model_user(lpsinglemodel, userflag, i);		
	}

	giveSem(g_SetModelParam);
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelScreenFlag
**
** 描述:  设置模型是否在小地图绘制
**
** 输入参数:  模型的ID，显示标志flag
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口7，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelScreenFlag(f_int32_t modelID, BOOL userflag)
{
	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.查找节点内容
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.修改显示方式
	lpsinglemodel->m_flagModelScreen = userflag;

	giveSem(g_SetModelParam);
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelScreenFlagType
**
** 描述:  设置模型在小地图的符号ID，屏幕符号显示标志的样式
**
** 输入参数:  模型的ID，样式type
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口7，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelScreenFlagType(f_int32_t modelID, int  type)
{
	LPModelRender lpsinglemodel = NULL;
	
	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.查找节点内容
	if(FALSE == SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.修改显示方式
	lpsinglemodel->m_ModelScreenID = type;

	giveSem(g_SetModelParam);
	
	return TRUE;
}


/*.BH--------------------------------------------------------
**
** 函数名: search_modelByID
**
** 描述:  查找模型的内存指针
**
** 输入参数:  模型的ID
**
** 输出参数：内存指针
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口
**
**.EH--------------------------------------------------------
*
BOOL search_modelByID(f_int32_t modelID , LPModelRender* pModel_select)
{
	int nodeID = 0;
	LPNode pNode = NULL;
//	LPModelRender lpsinglemodel = NULL;
//	int i = 0;
	
	// 1.定位到节点位置
	nodeID = SearchNodeIndex(modelID);
	if(nodeID == -1)
		{return FALSE;}

	// 2.获取节点内容并修改
	pNode = NodeAt(&ModelRenderList, nodeID);
	* pModel_select = (LPModelRender)pNode->m_pCur;

	return TRUE;
}
*/
/*.BH--------------------------------------------------------
**
** 函数名: set_model_displaytype
**
** 描述:  设置模型的显示方式
**
** 输入参数:  模型指针，显示方式
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口
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
** 函数名: set_model_colorNumUse
**
** 描述:  设置模型的颜色使用序号
**
** 输入参数:  模型指针，颜色序号
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口
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
		printf("非法参数colorNum_use\n");
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
** 函数名: set_model_shield
**
** 描述:  设置模型的屏蔽标志
**
** 输入参数:  模型指针，屏蔽标志
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口, 只需要设置父节点即可,不需要考虑子节点
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
** 函数名: set_model_user
**
** 描述:  设置模型的用户图层显示标志
**
** 输入参数:  模型指针，用户图层显示标志，第几个显示图层
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口, 只需要设置父节点即可,不需要考虑子节点
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
** 函数名: DestroyModel
**
** 描述:  删除模型节点
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口8，用户层调用
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
	
	// 1.定位到节点位置
	nodeID = SearchNodeIndex(modelID,  &pData);
	if(nodeID == -1)
		{giveSem(g_SetModelParam);return FALSE;}


	// 2.获取节点内容并修改, 删除子节点属性
	pstListHead = &(pData->m_childlist);

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			DestroyModel(lpsinglemodel->m_modelID);
		}
	}
	
	// 3.删除节点
	LIST_REMOVE(&(pData->stListHead));
	
	// 释放内存
	DeleteAlterableMemory(pData);

	giveSem(g_SetModelParam);
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelHeiMode
**
** 描述:  设置模型的高度值是绝对高度还是相对高度
**
** 输入参数:  模型的ID，相对高度0，绝对高度1, 海拔高度2
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口9，用户层调用
**
**.EH--------------------------------------------------------
*/
BOOL SetModelHeiMode(f_int32_t modelID,f_int32_t HeiMode)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.获取节点内容
	ret = SearchNodeIndex(modelID, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改显示方式
		lpsinglemodel->m_heimode = HeiMode;

		// 3.设置矩阵改变标志位
		//lpsinglemodel->m_MatrixChange = TRUE;
	}
	giveSem(g_SetModelParam);
//	DeleteAlterableMemory(lpsinglemodel);

	return ret;	
	
}

/*.BH--------------------------------------------------------
**
** 函数名: GetModelPos
**
** 描述: 根据id获取模型的位置
**
** 输入参数:  模型id
**
** 输出参数：模型位置
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口10
**
**.EH--------------------------------------------------------
*/
BOOL GetModelPos(f_int32_t modelid, LP_PT_3D model_pos)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.输出高度
		model_pos->x = lpsinglemodel->m_modelPos.x;
		model_pos->y = lpsinglemodel->m_modelPos.y;
		model_pos->z = lpsinglemodel->m_modelPos.z;
	
	}

//	DeleteAlterableMemory(lpsinglemodel);

	return ret;	
	
}

/*.BH--------------------------------------------------------
**
** 函数名: GetModelType
**
** 描述: 根据id获取模型的类型
**
** 输入参数:  模型id
**
** 输出参数：模型类型
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口21
**
**.EH--------------------------------------------------------
*/
BOOL GetModelType(f_int32_t modelid, f_int32_t *model_type)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.输出类型
		*model_type = lpsinglemodel->m_modeltype;
	
	}

//	DeleteAlterableMemory(lpsinglemodel);

	return ret;	

}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelPosAdd
**
** 描述: 增量移动模型位置
**
** 输入参数:  模型id,模型位置增量值，移动哪个分量lat :0,lon:1,hei:2
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口11
**
**.EH--------------------------------------------------------
*/
BOOL SetModelPosAdd(f_int32_t modelid, f_float64_t model_dpos, f_int32_t mode)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改相关内容
		if(mode == 0)
			lpsinglemodel->m_modelPos.x += model_dpos;
		else if(mode == 1)
			lpsinglemodel->m_modelPos.y += model_dpos;
		else if(mode == 2)
			lpsinglemodel->m_modelPos.z += model_dpos;
		else
			return FALSE;

		// 3.设置矩阵改变标志位
		lpsinglemodel->m_MatrixChange = TRUE;
	}

//	DeleteAlterableMemory(lpsinglemodel);
	giveSem(g_SetModelParam);
	return ret;
	
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelAngleAdd
**
** 描述: 增量移动模型角度
**
** 输入参数:  模型id,模型角度增量值，移动哪个分量yaw,0, pitch 1, roll 2
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口20
**
**.EH--------------------------------------------------------
*/
BOOL SetModelAngleAdd(f_int32_t modelid, f_float64_t model_dagnle, f_int32_t mode)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;
	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.修改相关内容
		if(mode == 0)
			lpsinglemodel->m_modelAngle.x += model_dagnle;
		else if(mode == 1)
			lpsinglemodel->m_modelAngle.y += model_dagnle;
		else if(mode == 2)
			lpsinglemodel->m_modelAngle.z += model_dagnle;
		else
			return FALSE;

		// 3.设置矩阵改变标志位
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
** 函数名: SetModelColor
**
** 描述: 设置模型的各种显示模式的颜色
**
** 输入参数:  模型id，模型显示模式，颜色
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口12
**
**.EH--------------------------------------------------------
*/
BOOL SetModelColor(int modelid, int color_type, PT_4D colorset)
{
	int nodeID = 0;
//	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;

#if 	0
	// 1.定位到节点位置
	nodeID = SearchNodeIndex(modelid);
	if(nodeID == -1)
		{return FALSE;}

	// 2.获取节点内容并修改
	pNode = NodeAt(&ModelRenderList, nodeID);
	lpsinglemodel = (LPModelRender)pNode->m_pCur;

	lpsinglemodel->m_color[type].x = colorset.x;
	lpsinglemodel->m_color[type].y = colorset.y;
	lpsinglemodel->m_color[type].z = colorset.z;
	lpsinglemodel->m_color[type].w = colorset.w;

#endif	

	takeSem(g_SetModelParam, FOREVER_WAIT);
	// 1.查找节点内容指针
	if(FALSE ==  SearchNodeIndex(modelid, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.设置颜色
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
** 函数名: SetModelText
**
** 描述: 设置模型的各行的文字(共4行)
**
** 输入参数:  模型id，文本字符串，行号
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口12
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
	// 1.查找节点内容指针
	if(FALSE ==  SearchNodeIndex(modelID, &lpsinglemodel))
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.设置指定行对应的文本
	memset(lpsinglemodel->m_pString[line],0,sizeof(lpsinglemodel->m_pString[line]));
	ttfAnsi2Unicode(lpsinglemodel->m_pString[line], textString, &lpsinglemodel->m_stringNum[line]);

	giveSem(g_SetModelParam);
	return TRUE;	
}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelTraceFlag
**
** 描述: 设置模型是否显示尾迹
**
** 输入参数:  模型id，标志TRUE or FALSE
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口12
**
**.EH--------------------------------------------------------
*
BOOL SetModelTraceFlag(int modelid, BOOL flag)
{
	int nodeID = 0;
//	LPNode pNode = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.查找节点内容指针
	if(FALSE ==  SearchNodeIndex(modelid, &lpsinglemodel))
	{
		return FALSE;
	}

	// 2.设置显示尾迹
	lpsinglemodel->m_flagTrace = flag;

	return TRUE;	
}
*/


/*.BH--------------------------------------------------------
**
** 函数名: set_model_color
**
** 描述:  设置模型的颜色
**
** 输入参数:  模型指针，模型颜色序号，颜色值
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口
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
		printf("非法参数colorNum_use\n");
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

	//纯色模式下将纹理重新生成的标志位置1
	if(color_type == 3)
	{
		model_select->m_colorPictureIsNeedRebuild = 1;
	}
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: rebulid_model_colorPicture
**
** 描述:  生成模型的纯色模式下的纹理ID
**
** 输入参数:  模型指针，模型颜色颜色值
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口
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

	/** 载入位图 */
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

	/** 控制滤波 */
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	/** 创建纹理,纹理为1*1的二维纹理 */
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

	/** 该模型的子模型纯色纹理与父模型一致 */
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
** 函数名: GetModelScale
**
** 描述: 根据id获取模型的缩放系数
**
** 输入参数:  模型id
**
** 输出参数：缩放系数
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口13
**
**.EH--------------------------------------------------------
*/
BOOL GetModelScale(f_int32_t modelid, f_float64_t* model_scale)
{
	LPModelRender lpsinglemodel = NULL;
//		(LPModelRender)NewAlterableMemory(sizeof(ModelRender));		//查找的模型数据指针
	BOOL ret = FALSE;

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);

	if(ret == TRUE)
	{
		// 2.输出高度
		* model_scale = lpsinglemodel->m_modelScale;
	
	}
	return TRUE;
}
#if 0
/*.BH--------------------------------------------------------
**
** 函数名: NewChildModelDynamic
**
** 描述:  动态增加子模型
**
** 输入参数:  父模型的ID,子模型的ID
**
** 输出参数：无
**
** 返回值：TRUE OR FALSE
**          
**
** 设计注记:  外部接口14，用户层调用
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
	// 1.定位到节点位置
	nodeID_parent = SearchNodeIndex(parent_id);
	nodeID_child = SearchNodeIndex(child_id);
	if((nodeID_parent == -1)||(nodeID_child == -1))
		{giveSem(g_SetModelParam);return FALSE;}

	// 2.获取父节点内容
	pNode_parent = NodeAt(&ModelRenderList, nodeID_parent);
	lpsinglemodel_parent = (LPModelRender)pNode_parent->m_pCur;

	pNode_child = NodeAt(&ModelRenderList, nodeID_child);
	lpsinglemodel_child = (LPModelRender)pNode_child->m_pCur;

	// 3.把子节点内容挂接到父节点上
	
	// 3.1先存储既有的子节点信息
/*
	pDataTemp = (LPModelRender)NewAlterableMemory(sizeof(ModelRender) * lpsinglemodel_parent->m_childNum);
	memcpy(pDataTemp, lpsinglemodel_parent->m_childModel, sizeof(ModelRender) * lpsinglemodel_parent->m_childNum);

	DeleteAlterableMemory(lpsinglemodel_parent->m_childModel);

	// 3.2增加子节点的内存空间
	lpsinglemodel_parent->m_childModel = (LPModelRender)NewAlterableMemory(sizeof(ModelRender) * (lpsinglemodel_parent->m_childNum+1));
	
	// 3.3拷贝子节点数据
	memcpy(lpsinglemodel_parent->m_childModel, pDataTemp, sizeof(ModelRender) * lpsinglemodel_parent->m_childNum);
	memcpy(&lpsinglemodel_parent->m_childModel[lpsinglemodel_parent->m_childNum], lpsinglemodel_child, sizeof(ModelRender));	
*/
	// 3.1-3.3,更换为子节点链表
	lpsinglemodel = (LPModelRender)NewAlterableMemory(sizeof(ModelRender));
	memcpy(lpsinglemodel, lpsinglemodel_child, sizeof(ModelRender) );
	
	pNode = (LPNode)NewAlterableMemory(sizeof(Node));
	InitNode(pNode);

	pNode->m_pCur = lpsinglemodel;
	PushBack(&lpsinglemodel_parent->m_childlist, pNode);

	// 3.4增加子节点的个数
	lpsinglemodel_parent->m_childNum += 1;	

	// 4.删除链表中的子节点信息
	if(FALSE == DestroyModel(lpsinglemodel_child->m_modelID))
		{giveSem(g_SetModelParam);return FALSE;}

	// 5.设置子节点的相对pos, angle, scale
	//m_nchildNum -1 代表的是序号

	if(FALSE == SetChildModelPos(lpsinglemodel_parent->m_modelID, lpsinglemodel_parent->m_childNum-1, zero_p))
		{giveSem(g_SetModelParam);return FALSE;}
	if(FALSE == SetChildModelAngle(lpsinglemodel_parent->m_modelID, lpsinglemodel_parent->m_childNum-1, zero_p))
		{giveSem(g_SetModelParam);return FALSE;}
	//放大倍数恢复为1
//	lpsinglemodel_parent->m_childModel[lpsinglemodel_parent->m_childNum-1].m_modelScale = 1;
	if(FALSE == SetChildModelScale(lpsinglemodel_parent->m_modelID, lpsinglemodel_parent->m_childNum-1, 1))
		{giveSem(g_SetModelParam);return FALSE;}
	
	giveSem(g_SetModelParam);
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: DeleteChildModelDynamic
**
** 描述:  动态删除子模型
**
** 输入参数:  父模型的id,子模型的序列号
**
** 输出参数：子模型id
**
** 返回值：TRUE OR FALSE
**          
**
** 设计注记:  外部接口14，用户层调用
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
	// 1.定位到节点位置
	nodeID = SearchNodeIndex(parent_id);
	if(nodeID == -1)
		{giveSem(g_SetModelParam);return FALSE;}

	// 2.获取节点内容
	pNode = NodeAt(&ModelRenderList, nodeID);
	lpsinglemodel = (LPModelRender)pNode->m_pCur;

	// 4.把子节点结构体添加到绘制链表中
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

	// 5.删除子节点
	DeleteNode(&lpsinglemodel->m_childlist, child_number);
	
//	for(i=child_number+1; i <lpsinglemodel->m_childNum; i++)
//	{
//		lpsinglemodel->m_childModel[i-1] = lpsinglemodel->m_childModel[i]; 
//	}

	// 3.减少子节点个数
	lpsinglemodel->m_childNum -= 1;

	// 6.输出子节点的ID
	*child_id = pDataChild->m_modelID; 

	// 7.设置新节点的pos, angle, scale
	if(FALSE == SetModelPos(pDataChild->m_modelID, lpsinglemodel->m_modelPos))
		{giveSem(g_SetModelParam);return FALSE;}
	if(FALSE == SetModelAngle(pDataChild->m_modelID, lpsinglemodel->m_modelAngle))
		{giveSem(g_SetModelParam);return FALSE;}
	//放大倍数也设置为父节点同样大小
	if(FALSE == SetModelScale(pDataChild->m_modelID, lpsinglemodel->m_modelScale))
		{giveSem(g_SetModelParam);return FALSE;}

	giveSem(g_SetModelParam);
	return TRUE;
}
*/
#endif
/*.BH--------------------------------------------------------
**
** 函数名: SetChildModelPos
**
** 描述:  设置子节点的相对位置
**
** 输入参数:  父模型的id,子节点序号，相对 pos
**
** 输出参数：无
**
** 返回值：成功or 失败
**          
**
** 设计注记:  外部接口15，用户层调用
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

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.非法判断
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
				// 3.修改子节点属性 
				SetPT3D(xyz, &(lpsinglemodel_child->m_modelPos) );

				// 4.设置矩阵改变标志
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
** 函数名: SetChildModelAngle
**
** 描述:  设置子节点的相对角度
**
** 输入参数:  父模型的id,子节点序号，PT_3D  angle
**
** 输出参数：无
**
** 返回值：成功or 失败
**          
**
** 设计注记:  外部接口16，用户层调用
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

	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}

	// 2.非法判断
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
				// 3.修改子节点属性 
				SetPT3D(yawpitchroll, &(lpsinglemodel_child->m_modelAngle) );
				
				// 4.设置矩阵改变标志
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
** 函数名: SetChildModelPosAdd
**
** 描述: 增量移动子模型位置
**
** 输入参数:  模型id,模型位置增量值，移动哪个分量lat :0,lon:1,hei:2
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口17
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
	
	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 2.非法判断
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
				// 3.修改子节点属性 赋值
				if(mode == 0)
					lpsinglemodel_child->m_modelPos .x += model_dpos;
				else if(mode == 1)
					lpsinglemodel_child->m_modelPos .y += model_dpos;
				else if(mode == 2)
					lpsinglemodel_child->m_modelPos .z += model_dpos;
				else
					return FALSE;
				
				// 4.设置矩阵改变标志
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
** 函数名: SetChildModelAngleAdd
**
** 描述: 增量移动子模型角度
**
** 输入参数:  模型id,模型角度增量值，移动哪个分量yaw 0, pitch 1,roll 2
**
** 输出参数：
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记: 外部接口18
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
	
	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 2.非法判断
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
				// 3.修改子节点属性 赋值
				if(mode == 0)
					lpsinglemodel_child->m_modelAngle.x += model_dangle;
				else if(mode == 1)
					lpsinglemodel_child->m_modelAngle .y += model_dangle;
				else if(mode == 2)
					lpsinglemodel_child->m_modelAngle .z += model_dangle;
				else
					return FALSE;
				
				// 4.设置矩阵改变标志
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
** 函数名: SetChildModelScale
**
** 描述:  设置子节点的放大倍数
**
** 输入参数:  父模型的id,子节点序号，放大倍数scale
**
** 输出参数：无
**
** 返回值：成功or 失败
**          
**
** 设计注记:  外部接口15，用户层调用
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
	
	// 1.获取节点内容
	ret = SearchNodeIndex(modelid, &lpsinglemodel);
	if(ret == FALSE)
	{
		giveSem(g_SetModelParam);
		return FALSE;
	}
	
	// 2.非法判断
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
				// 3.修改子节点属性 
				lpsinglemodel_child->m_modelScale = scale;
				
				// 4.设置矩阵改变标志
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
** 函数名: SetChildNum
**
** 描述:  设置子节点的个数
**
** 输入参数:  父模型的id，子节点个数
**
** 输出参数：无
**
** 返回值：NONE
**          
**
** 设计注记:  内部接口
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

	// 1.定位到节点位置
	nodeID = SearchNodeIndex(modelid);
	if(nodeID == -1)
		{return FALSE;}

	// 2.获取节点内容
	pNode = NodeAt(&ModelRenderList, nodeID);
	lpsinglemodel = (LPModelRender)pNode->m_pCur;

	// 3.增加子节点属性 
	lpsinglemodel->m_childModel
		= (LPModelRender)NewAlterableMemory(num *  sizeof(ModelRender));
	lpsinglemodel->m_childNum = num;
	
	return TRUE;
}
*/

/*.BH--------------------------------------------------------
**
** 函数名: CreateNew3Number
**
** 描述:  生成三位没有重复的随机数字1-999
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：3位随机数字
**          
**
** 设计注记:  内部工具函数1
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

	BOOL Repeat_FLAG = FALSE;		//默认是没有重复的数字

	while(1)
	{
		// 1. 生成三位随机数字
	       randnumber = (((double) rand() / (double) RAND_MAX) * RANGE_MAX + RANGE_MIN);

		// 2.判断是否重复
		while(pNode != NULL)
		{
			lpsinglemodel = (LPModelRender)pNode->m_pCur;
			if( lpsinglemodel->m_modelID == randnumber)
			{
				Repeat_FLAG = TRUE;
			}

			pNode = pNode->m_pNext;
		}

		// 3.根据Repeat_FLAG 判断是否重新生成随机数
		if(Repeat_FLAG == FALSE)
		{
			break;
		}
		
	}

	// 4.返回随机数
	return randnumber;
}
*/
/*.BH--------------------------------------------------------
**
** 函数名: SearchNodeIndex
**
** 描述:  根据模型链表中的节点号，查找模型的内存地址
**
** 输入参数:  模型的ID
**
** 输出参数：模型的内存指针
**
** 返回值：TRUE or FALSE
**          
**
** 设计注记:  内部工具函数2
**
**.EH--------------------------------------------------------
*/
f_int32_t SearchNodeIndex(f_int32_t modelID, LPModelRender *pModelOut)
{

#if 0
	LPNode pNode = (LPNode)ModelRenderList.m_pFirst;
	LPModelRender lpsinglemodel =NULL;
	int node_i = 0;

	// 1.查找节点node_i
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

	// 2.返回node_i
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
** 函数名: SetPT3D
**
** 描述:  3维数据赋值
**
** 输入参数:  PT_3D
**
** 输出参数：PT_3D
**
** 返回值：NONE
**          
**
** 设计注记:  内部工具函数3
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
** 函数名: CheckModelcollision
**
** 描述:  判断模型是否与检测球碰撞
**
** 输入参数:  PT_3D 球心坐标，double 球半径
**
** 输出参数：无
**
** 返回值：模型id 或-1
**          
**
** 设计注记:  外部接口9,lpf delete 没有使用
**
**.EH--------------------------------------------------------
*/
/*
 *	点是否在球内
 *	参数：
 *			pPtPos 输入点三维坐标
 *			pPtSphereCenter 球心坐标
 *			sphere_radius	球半径
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

	// 1.查找节点
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






			collisionRadius += lpsinglemodel->m_modelScale * 40;	//模型大小设为40

			if(IsPointInSphere(&lpsinglemodel->m_modelPoxWorld, &collisionPoint, collisionRadius) == TRUE)
			{
				return lpsinglemodel->m_modelID;
			}			
		}
	}
	// 2.返回没有碰撞
	return -1;
}

/*.BH--------------------------------------------------------
**
** 函数名: CheckModelcollisionScreen
**
** 描述:  判断模型屏幕坐标是否与检测圆碰撞
**
** 输入参数:  PT_3D 圆中心坐标，double 圆半径
**
** 输出参数：无
**
** 返回值：模型id 或-1
**          
**
** 设计注记:  外部接口9
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
	

	//二次选择的数组，存储:modelID,  model屏幕坐标与屏幕接触点的距离
	int MaxSecondSelectNum = 	GetNodeNum(pstListHead);
	double * pSecondSelect = (double *)malloc(sizeof(double) * 2 * MaxSecondSelectNum);	
	double MinModelID = 0; double MinModelLength = 0;	//距离最小的模型ID，模型距离
	
	memset(pSecondSelect, 0 , sizeof(double) * 2 * MaxSecondSelectNum);
// 	M2Pixl =  
// 		2*g_SphereRender.m_Render.m_lfNear* tan(g_Fovy / 2 * DE2RA) /						//废弃，不使用，采用尝试的参数
// 	(g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom);		//近裁截面上的1个像素对应的米

	
	// 1.查找节点
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);


			//经纬度转屏幕坐标
			worldXYZ2screenXY(lpsinglemodel->m_modelPoxWorld, &screenX, &screenY);

			//首先判断模型点是否在屏幕圆中
			//模型点与圆心的屏幕距离
			Point2CollisionSqr = (screenX - collisionPoint.x)*(screenX - collisionPoint.x) +
							(screenY- collisionPoint.y)*(screenY- collisionPoint.y);

			//视点和模型的世界距离
			Point2Eye = 
				DistanceTwoPoints(&lpsinglemodel->m_modelPoxWorld, &pModelScene->camParam.m_ptEye);


			//视点到中心点与视点到模型的夹角
			vector3DSub(&Eye2Center, &pModelScene->camParam.m_ptEye, &pModelScene->camParam.m_ptCenter);
			vector3DSub(&Eye2Model, &pModelScene->camParam.m_ptEye,   &lpsinglemodel->m_modelPoxWorld);
			
			CenterEyeModel = getVa2DVbAngle(&Eye2Center, &Eye2Model);

			//视点到近裁截面在模型方向上的距离
			{
				double g_near = GetView_near(pModelScene);
//				FNearModel = g_SphereRender.m_Render.m_lfNear / cos(CenterEyeModel * DE2RA);	
				FNearModel = g_near / cos(CenterEyeModel * DE2RA);	
				
			


			//模型的半径在近裁截面上的投影距离
//			CollisionRadiusCal = 
//				lpsinglemodel->m_modelScale * lpsinglemodel->m_modelSize *5*100/g_SphereRender.m_Render.m_lfNear	// 	/ 不使用此M2Pixl，采用经验参数100,近裁截面是1，此值为100，近裁截面是100，此值是1,推断关系100/Near；
//				* FNearModel
//				/Point2Eye;		//手动增加4倍


			CollisionRadiusCal = 
				lpsinglemodel->m_modelScale * lpsinglemodel->m_modelSize *5*100/g_near	// 	/ 不使用此M2Pixl，采用经验参数100,近裁截面是1，此值为100，近裁截面是100，此值是1,推断关系100/Near；
				* FNearModel
				/Point2Eye;		//手动增加4倍

			}
			//CollisionRadiusCal = lpsinglemodel->m_modelScale * lpsinglemodel->m_modelSize;

			//模型半径是否大于距离，平方值
			if(sqr(CollisionRadiusCal) > Point2CollisionSqr)
			{
				//增加二次选择，距离最近的模型被选定
				//绘制屏幕圆
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

	// 2.二次选择, 距离最近的模型被选定
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

	// 3.判断返回值
	if(MinModelID == 0)
		ret = -1;
	else
		ret = MinModelID;
	
	// 3. 释放指针
	free(pSecondSelect);
	pSecondSelect = NULL;
	
	// 4.返回没有碰撞或者碰撞的模型ID
	return ret;
}


/*.BH--------------------------------------------------------
**
** 函数名: ConvertChildPos
**
** 描述:  子模型的相对位置转换为绝对位置
**
** 输入参数:  PT_3D F_posXYZ, 		父模型的位置
**				PT_3D F_anglePYR, 		父模型的角度
**				PT_3D C_pos, 			子模型的相对位置及绝对位置
**
** 输出参数：PT_3D C_pos, 		子模型的绝对位置 
**
** 返回值：NONE
**          
**
** 设计注记:  内部工具函数4,世界坐标，角度单位是度
**
**.EH--------------------------------------------------------
*
void ConvertChildPos(PT_3D F_posXYZ, PT_3D F_anglePYR, PT_3D C_pos)
{
	Matrix44 matrix_convert = {0};
	Matrix44 matrix_1,matrix_2;

	//*计算中心点的局部坐标到世界坐标矩阵*
	calcWorldMatrix44(F_posXYZ.x, F_posXYZ.y, F_posXYZ.z, &matrix_convert);

	// 坐标转换
	PosPart2World(C_pos, 
					&matrix_1, 
					&matrix_2, 
					F_anglePYR.x , -F_anglePYR.y, F_anglePYR.z, 
					&matrix_convert);
	
}
*/
/*.BH--------------------------------------------------------
**
** 函数名: PosPart2World
**
** 描述:  仿照SetCameraFollow函数修改得到坐标转换函数
**
** 输入参数:  
**
** 输出参数：PT_3D pos, PT_3D angle 
**
** 返回值：NONE
**          
**
** 设计注记:  内部工具函数5，世界坐标，角度单位是度
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
** 函数名: ConvertChildAngle
**
** 描述:  子模型的角度转换为绝对角度
**
** 输入参数:  PT_3D F_anglePYR, 		父模型的角度
**				 PT_3D C_angle,		子模型的相对角度
**
** 输出参数：PT_3D C_angleOut 		子模型的绝对角度
**
** 返回值：NONE
**          
**
** 设计注记:  内部工具函数6,单位是度
**
**.EH--------------------------------------------------------
*/
void ConvertChildAngle(PT_3D F_anglePYR, PT_3D C_angle,  LP_PT_3D C_angleOut)
{
	// 角度转换
	C_angleOut->x = C_angle.x + F_anglePYR.x;
	C_angleOut->y = C_angle.y + F_anglePYR.y;
	C_angleOut->z = C_angle.z + F_anglePYR.z;

}

/*********************************绘制轨迹线******************************************/

/*
 *	更新飞行轨迹
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
			//if (pmodelselect->s_pTraceCount == 4)//取消每4帧记录一次
			{
				
				PT_3D ptUp, ptDown;
				f_int16_t index;
				pmodelselect->s_pTraceCount = 0;
//				MemSet_3D(&ptUp, 0, TRACE_HALF_HEGHT, 0);			//纵向
//				MemSet_3D(&ptDown, 0, -TRACE_HALF_HEGHT, 0);
				vector3DMemSet(&ptUp, TRACE_HALF_HEGHT, 0, 15);			//横向, 尾迹向飞机后面平移15m
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
				// 复制节点到数组
				// 计算当前的节点位置
					
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

//程序使用的模型绘制函数1，显示方式为：0-直接绘制模型,1-模型闪动,6-纯色模式
void RenderModelType1(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	// 打开光照,移动到矩阵变化之后
//	EnableLight(TRUE);
//	EnableLight1(TRUE);
	
	// 1.循环模型绘制链表
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{				
				// 3.判断显示方式是否：0-直接绘制模型,1-模型闪动
				if(	(lpsinglemodel->m_modelDisplaytype == 0)
						||(lpsinglemodel->m_modelDisplaytype == 1)
						/*||(lpsinglemodel->m_modelDisplaytype == 6)*/
						/*||((lpsinglemodel->m_modelDisplaytype >= 7)&&(lpsinglemodel->m_modelDisplaytype <= 10)*/
						)
				{
					//绘制模型
					RenderModelFeatureDynamic(lpsinglemodel, pModelScene);					
				}
				else if(lpsinglemodel->m_modelDisplaytype == 6) // 4.判断显示方式是否：6-纯色纹理模式
				{
					if(lpsinglemodel->m_colorPictureIsNeedRebuild == 1)
					{
						lpsinglemodel->m_colorPictureIsNeedRebuild = 0;
						rebulid_model_colorPicture(lpsinglemodel,lpsinglemodel->m_color[3]);		
					}
					//绘制模型
					RenderModelFeatureDynamic(lpsinglemodel, pModelScene);					

				}
		
			}
		
		}
	}	
	
	// 关闭光照
	EnableLight(FALSE);
	EnableLight1(FALSE);
	
}

//程序使用的模型绘制函数2，显示方式为：2-线框模式,11以上(含11)-粒子效果,暂未实现
void RenderModelType2(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;	
	LPModelRender lpsinglemodel = NULL;
	
	// 1.循环链表
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 3.判断显示方式是否：2-线框模式,11以上(含11)-粒子效果,暂未实现
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

//程序使用的模型绘制函数3，显示方式为：3-模型+线框(1.02倍大小)，
void RenderModelType3(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);

	// 先按照显示方式0绘制
	// 打开光照
	EnableLight(TRUE);
	EnableLight1(TRUE);		

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
	
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{				
				// 3.判断显示方式是否：3-模型+线框(1.02倍大小)
				if(	(lpsinglemodel->m_modelDisplaytype == 3) )
				{
					// 4.按照显示方式是0绘制
					set_model_colorNumUse(lpsinglemodel, 0);
					set_model_displaytype(lpsinglemodel, 0);	
					//直接调用模型绘制函数
					RenderModelLLH(lpsinglemodel,pModelScene);
					//再设置显示模式为3
					set_model_displaytype(lpsinglemodel, 3);								
				}	
			}
		}		
	}	
	// 关闭光照
	EnableLight(FALSE);
	EnableLight1(FALSE);
	
	//再按照显示方式2绘制
	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
			
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.判断显示方式是否：3-模型+线框(1.02倍大小)
				if(	(lpsinglemodel->m_modelDisplaytype == 3) )
				{
					// 4.按照显示方式是2-线框模式绘制,模型放大1.02
					set_model_displaytype(lpsinglemodel, 2);				
					lpsinglemodel->m_modelScale *= 1.02;
					lpsinglemodel->m_MatrixChange = TRUE;
					RenderModelLLH(lpsinglemodel,pModelScene);		

					// 5.模型效果复原为3, 大小复原
					set_model_displaytype(lpsinglemodel, 3);	
					lpsinglemodel->m_modelScale *= (1/1.02);
					lpsinglemodel->m_MatrixChange = TRUE;
									
				}
			}
		}
	}	
	
}

//程序使用的模型绘制函数4，显示方式为：4-模型+包络线(正常大小,线粗3倍)，
void RenderModelType4(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);

	//先按照显示方式2-线框模式绘制,但线宽为原来的3倍	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.0f);
	glDepthMask(GL_FALSE);			
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{				
				// 3.判断显示方式是否：4-模型+包络线(正常大小,线粗3倍)
				if(	(lpsinglemodel->m_modelDisplaytype == 4) )
				{
					// 4.按照显示方式是4绘制
					//设置使用的颜色，实际上如果有纹理用的是纹理的颜色，如果没有纹理，则使用此处设置的颜色
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
		
	// 再按照显示方式0-正常模式绘制
	glColor3f(1.0f, 1.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0f);	
	
	// 打开光照
	EnableLight(TRUE);
	EnableLight1(TRUE);	
	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 3.判断显示方式是否：4
				if(	(lpsinglemodel->m_modelDisplaytype == 4) )
				{
					// 4.绘制模型，按0-正常模式绘制
					set_model_colorNumUse(lpsinglemodel, 0);
					set_model_displaytype(lpsinglemodel, 0);		
					RenderModelLLH(lpsinglemodel,pModelScene);

					// 5.模型效果复原为4
					set_model_displaytype(lpsinglemodel, 4);							
				}
			}
		
		}
	}	
	// 关闭光照
	EnableLight(FALSE);
	EnableLight1(FALSE);

}

//程序使用的模型绘制函数5，显示方式为：5-光环效果,即模型+纯色(1.05倍大小指定颜色)，
void RenderModelType5(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;

	LPModelRender lpsinglemodel = NULL;

	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);

	//先按照显示方式5绘制	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);	
			

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 3.判断显示方式是否：5
				if(	(lpsinglemodel->m_modelDisplaytype == 5))
				{
					// 4.按照显示方式是5绘制
					//设置使用的颜色
					glColor4f(lpsinglemodel->m_color[2].x, 
							lpsinglemodel->m_color[2].y,
							lpsinglemodel->m_color[2].z,
							lpsinglemodel->m_color[2].w);	
					set_model_colorNumUse(lpsinglemodel, 2);
					set_model_displaytype(lpsinglemodel, 5);
					//放大1.05倍
					lpsinglemodel->m_modelScale *= 1.05;
					lpsinglemodel->m_MatrixChange = TRUE;
					RenderModelLLH(lpsinglemodel,pModelScene);
									
				}
		
			}
		
		}
	}	
		
	// 再按照显示方式0绘制
	glDisableEx(GL_BLEND);
	glDepthMask(GL_TRUE);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	// 打开光照
	EnableLight(TRUE);
	EnableLight1(TRUE);	
	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.判断显示方式是否：5
				if(	(lpsinglemodel->m_modelDisplaytype == 5) )
				{
					// 4.绘制模型，按0-正常模式绘制
					set_model_colorNumUse(lpsinglemodel, 0);
					set_model_displaytype(lpsinglemodel, 0);
					//恢复正常大小
					lpsinglemodel->m_modelScale *= (1/1.05);
					lpsinglemodel->m_MatrixChange = TRUE;	
					RenderModelLLH(lpsinglemodel,pModelScene);			

					//模型效果复原为5
					set_model_displaytype(lpsinglemodel, 5);
									
				}
			}
		}
	}	

	// 关闭光照
	EnableLight(FALSE);
	EnableLight1(FALSE);

}



//程序使用的模型绘制函数6，显示方式为：7-10：未定义，按0-正常模式绘制
void RenderModelType6(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	
	LPModelRender lpsinglemodel = NULL;
	
	// 打开光照,移动到矩阵变化之后
//	EnableLight(TRUE);
//	EnableLight1(TRUE);
	
	// 1.循环链表
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);
		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
			
				// 3.判断显示方式是否：7-10,
				if((lpsinglemodel->m_modelDisplaytype >= 7)&&(lpsinglemodel->m_modelDisplaytype <= 10))
				{
					RenderModelFeatureDynamic(lpsinglemodel,pModelScene);					
				}
	
			}
		
		}	
	}
	
	// 关闭光照
//	EnableLight(FALSE);
//	EnableLight1(FALSE);
	
}

/*
	构建飞行轨迹的一维纹理对象
*/
static f_uint32_t S_FLYTRACETEXTURE = 0;
void SetFlyTraceColor()
{
	#define flytrace_sampling 	64

	// 设置飞行轨迹色带
	f_uint8_t chRGBA[flytrace_sampling * 4];
	f_int32_t i;

	memset(chRGBA, 0, flytrace_sampling * 4);		// 默认都是黑色

	// 前32*4的颜色是黄色，透明度按i*7自增，由完全透明转为不透明
	for(i = 0 ; i<(flytrace_sampling /2); i++)
	{
		chRGBA[4 * i + 0] = 255;
		chRGBA[4 * i + 1] = 255;
		chRGBA[4 * i + 2] = 0;
		
		chRGBA[4 * i + 3] = (int) ((i * 255/(flytrace_sampling /2)) % 255);

		//printf("%d-",chRGBA[4 * i + 3] );	
	}

	// 后32*4的颜色是黄色，透明度按(64-1-i)*7自增，由不透明转为完全透明
	for(i= (flytrace_sampling /2); i< (flytrace_sampling ); i++)
	{
		chRGBA[4 * i + 0] = 255;
		chRGBA[4 * i + 1] = 255;
		chRGBA[4 * i + 2] = 0;
		
		chRGBA[4 * i + 3] = chRGBA[4 * (flytrace_sampling -1 - i) + 3 ];
		//printf("%d-",chRGBA[4 * i + 3] );
	}
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	// 生成飞行轨迹纹理，一维纹理，纹理长度为64
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
 *	绘制飞行轨迹
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
					//alpha递增的
					glColor4f(1.0, 1.0, 0.0, 
				( (FLYTRACE_NODENUM_MAX - FLYTRACE_NODENUM_MAX/2) * j / part_num + FLYTRACE_NODENUM_MAX /2) * s_nColorInterval);

				}
				else
				{
					//alpha递减的
					glColor4f(1.0, 1.0, 0.0, (FLYTRACE_NODENUM_MAX + part_num - j) * s_nColorInterval);
				
				}

				// 绘制三角形
				
			}

#endif
#if 0
			if(j == FLYTRACE_NODENUM_MAX * 0.1)
			{
				SetPT3D(pModelSelect->m_vFlyTrace[index].m_ptUp, &flytrace_tri[0]);
				SetPT3D(pModelSelect->m_vFlyTrace[index].m_ptDown, &flytrace_tri[1]);
				{
					PT_3D ptUp;
					MemSet_3D(&ptUp, 10, 0, 0);			//横向
				
					MultiMatrix4x4(&ptUp, &ptUp, &pModelSelect->m_mxRotate);
					MultiMatrix4x4(&flytrace_tri[2], &ptUp, &pModelSelect->m_mxWorld);
				}
				{
					PT_3D ptUp;
					MemSet_3D(&ptUp, -10, 0, 0);			//横向
				
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
		//绘制三角形
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

//用户层使用的绘制飞行轨迹的函数，按照图层的含义绘制
void RenderUserFlyTrace(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);
		
//	glPushAttrib(GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
	glDisableEx(GL_CULL_FACE);
//	glDisableEx(GL_TEXTURE_2D);
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// 2.生成飞行轨迹的一维纹理,仅生成一次
	if(S_FLYTRACETEXTURE == 0)
	{
		SetFlyTraceColor();
	}
	
//	glPolygonMode(GL_FRONT, 0 ? GL_LINE : GL_FILL);	
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glEnableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_1D, S_FLYTRACETEXTURE);	// 绑定飞行轨迹的一维纹理
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);//GL_BLEND);// GL_REPLACE);//GL_MODULATE); 
#else
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, S_FLYTRACETEXTURE);	// 绑定飞行轨迹纹理
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);//GL_BLEND);// GL_REPLACE);//GL_MODULATE); 

#endif

	// 3.设置投影和模视矩阵
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(Get_PROJECTION(pModelScene));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixd(Get_MODELVIEW(pModelScene));
	
	// 4.遍历模型
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 4.1.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 4.2.判断飞行轨迹的图层是否开启
				if(lpsinglemodel->m_flagUser[0] == TRUE)
				{
					// 4.3.绘制飞行轨迹
					RenderFlyTrace(0, lpsinglemodel);				
				}		
			}
		}
	}	
	
	// 5.恢复矩阵
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
	
	// 6.恢复纹理状态
//	glPopAttrib();
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_1D, 0);	
	glDisableEx(GL_TEXTURE_1D);
#else
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture(GL_TEXTURE_2D, 0);	// 绑定地形告警纹理
	glDisableEx(GL_TEXTURE_2D);

#endif
//	glEnableEx(GL_CULL_FACE);

	//恢复alpha值
	glColor4ub(255,255,255,255);
	glDisableEx(GL_BLEND);
	
}




//绘制模型的外接长方体
void RenderRectangle(LPModelRender pModelRender, f_uint32_t color4ub[4])
{
	float delt_x,delt_z;
	float pass_y,y;

	//按模型的大小来设置x，z和顶面y
	y = pModelRender->m_modelSize;
	delt_x = delt_z =  pModelRender->m_modelSize /2;
	//设置底面y的值
 	pass_y = - pModelRender->m_modelSize * 0.1;

	// 1.生成显示列表		
	if(pModelRender->m_glLists == 0)
	{
		pModelRender->m_glLists = glGenLists(1);		
		
		glNewList(pModelRender->m_glLists, GL_COMPILE);
#if 0		
			//底面xz面y = 0
			glBegin(GL_QUADS);
				glVertex3f(delt_x,   pass_y, delt_z);	
				glVertex3f(delt_x,   pass_y, -delt_z);	
				glVertex3f(-delt_x,   pass_y, -delt_z);	
				glVertex3f(-delt_x,   pass_y, delt_z);	
			glEnd();

		//侧面
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
			
			//顶面
			glBegin(GL_QUADS);
				glVertex3f(delt_x,   y, delt_z);	
				glVertex3f(-delt_x,   y, delt_z);	
				glVertex3f(-delt_x,   y, -delt_z);	
				glVertex3f(delt_x,   y, -delt_z);	
			glEnd();
#else
		//绘制底面的矩形,2个三角形
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   pass_y, delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	

			glVertex3f(delt_x,   pass_y, delt_z);
			glVertex3f(-delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	
				
		glEnd();

		//绘制顶面的矩形,2个三角形
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   y, delt_z);	
			glVertex3f(delt_x,   y, -delt_z);	
			glVertex3f(-delt_x,   y, -delt_z);	

			glVertex3f(delt_x,   y, delt_z);
			glVertex3f(-delt_x,   y, -delt_z);	
			glVertex3f(-delt_x,   y, delt_z);	
				
		glEnd();
	
		//绘制4个侧面矩形，每个侧面矩形由2个三角形组成
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
	
	// 2.绘制长方体		
//	glEnableEx(GL_POLYGON_SMOOTH);
//	glEnableEx(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// 2.1设置绘制的颜色
	glColor4ub((GLubyte)color4ub[0], (GLubyte)color4ub[1], (GLubyte)color4ub[2], (GLubyte)color4ub[3]);
	//glColor4ub(255,0,0,255);
	
	//2.2调用显示列表进行绘制
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

		//去除不需要绘制的模型
//		if(lpsinglemodel->m_flagTrace == FALSE)
//		{
//			pNode = pNode->m_pNext;
//			continue;
//		}

		//绘制外接长方体
//		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		//模型外接长方体颜色为灰色,透明度为100
		color_rectangle[0] = 150;
		color_rectangle[1] = 150;
		color_rectangle[2] = 150;
		color_rectangle[3] = 100;
		//按设置的颜色绘制模型外接长方体
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
//用户层使用的绘制长方体的函数，按照图层的含义绘制
void RenderUserRectangle(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.循环链表
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

		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.判断长方体图层是否绘制
				if(lpsinglemodel->m_flagUser[2] == TRUE)
				{
					// 4.设置矩阵,转换到模型绘制的坐标系
					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
					glLoadMatrixd(Get_PROJECTION(pModelScene));
				
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					glLoadMatrixd(Get_MODELVIEW(pModelScene));
					
					glMultMatrixd(lpsinglemodel->m_modelMatrix);
					
					// 5.绘制长方体
					RenderModelRectangle(0, lpsinglemodel);
					
					// 6.恢复矩阵
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
	//恢复alpha值
	glColor4ub(255,255,255,255);
	glDisableEx(GL_BLEND);
}


//飞机上方画字符
extern TTFONT g_TTFFont;

//test for 文字框
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
	double Radius = 0.0;	//字体旋转的半径
	int lineIndex = 0;
	float nextTextLineHeight = 0.0f; //每行文字离底部的高度

#if 1
	//长杆子绘制坐标系转换到字体绘制的坐标系,以长杆子顶面中心为坐标系原点
	glPushMatrix();
	{
		double size = 1;
		// 1.矩阵变换并存储
		vector3DMemSet(&ptPos, x, y, z);

		//增加位置的旋转，保证位置始终在模型的正上方
//		Radius = sqrt(sqr(x) + sqr(y) + sqr(z));
//		ptPos.z = Radius * sin(yz_angle* DE2RA);
//		ptPos.x = Radius * sin(-xy_angle * DE2RA) * cos(-yz_angle* DE2RA);
//		ptPos.y = Radius * cos(xy_angle * DE2RA)  * cos(-yz_angle* DE2RA);

		dMatrix44MemSet(&MxWorld, 1,   	0,   		0,   		0.0,
								    0,   	1,   		0,   		0.0,
							           0,   	0,   		1,   		0.0,
							           ptPos.x, 	ptPos.y, 		ptPos.z,   		1.0f );
		//计算缩放矩阵
		size = sx;
		dMatrix44MemSet(&MxScale, size, 0, 0, 0,
								   0, size, 0, 0,
								   0, 0, size, 0,
								   (1-size)*MxWorld.m[12], (1-size)*MxWorld.m[13], (1-size)*MxWorld.m[14], 1);
		Maxtrix4x4_Mul(&temp_mx, &MxWorld, &MxScale);
		
		//计算旋转矩阵
		dMatrix44RPYmemSet(&MxRotate, yz_angle, xz_angle,  xy_angle);
		Maxtrix4x4_Mul(&model_mx, &MxRotate, &temp_mx);
	}
	
	glMultMatrixd(model_mx.m);

#endif

	switch(style)
	{
	case BILLBOARD_CENTER:  //字在标牌中间
#if 0
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex3d(-stringNum /2.0, -0.5, 0.0);		// 左下角
		glTexCoord2f(1.0, 1.0); glVertex3d( stringNum /2.0, -0.5, 0.0);		// 右下角
		glTexCoord2f(1.0, 0.0); glVertex3d( stringNum /2.0,  0.5, 0.0);		// 右上角
		glTexCoord2f(0.0, 0.0); glVertex3d(-stringNum /2.0,  0.5, 0.0);		// 左上角
		glEnd();
#endif
		break;
	case BILLBOARD_TOP: //字在标牌底部
		/*标牌绘制,暂时不绘制*/
#if 0	
		//对于深度相差非常小的情况（例如在同一平面上进行两次绘制），
		//OpenGL就不能正确判定两者的深度值，会导致深度测试的结果不可预测，
		//显示出来的现象时交错闪烁的前后两个画面，这种情况称为z-fighting
		//此处为了防止字和标牌之间出现z-fighting
		glEnableEx(GL_POLYGON_OFFSET_FILL);
		//设置后深度偏移量的计算公式是Offset=DZ*factor+r*units
		//设置正数表示当前的深度更深一些，显示的时候会被前景覆盖，
		//设为负数表示深度较浅，会被绘制到屏幕上去
		glPolygonOffset(0.0f, 1.0f);
		/* 绘制底板 */
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3d(-stringNum /2.0, 0.0, 0.0);		// 左下角
			glVertex3d( stringNum /2.0, 0.0, 0.0);		// 右下角
			glVertex3d( stringNum /2.0, 1.1, 0.0);	// 右上角
			glVertex3d(-stringNum /2.0, 1.1, 0.0);	// 左上角
		glEnd();

		/* 绘制底板外框 */
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
			glVertex3d(-stringNum /2.0, 0.0, 0.0);		// 左下角
			glVertex3d( stringNum /2.0, 0.0, 0.0);		// 右下角
			glVertex3d( stringNum /2.0, 1.1, 0.0);	// 右上角
			glVertex3d(-stringNum /2.0, 1.1, 0.0);	// 左上角
		glEnd();
		glDisableEx(GL_POLYGON_OFFSET_FILL);
#endif
		/* 文字,黑字白框 */
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		ttfDrawStringPre(pFont);
		/* 依次绘制4行文字 */
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
	//字符在x方向的偏移值为0
	float x = 0;

	//字符在y方向的偏移，随模型大小和缩放系统变化
//	float y = 10.0;
	float y = (float)pModelSelect->m_modelSize * pModelSelect->m_modelScale/3;	

	//长方体杆的长度,与字符在y方向的偏移值相同
	float y_rectange = y;			
	//字符在z方向的偏移值为0
	float z = 0;
	//字符在xy方向上的缩放值
	f_float64_t sx = 10.0;			
	f_float64_t sy = 10.0;
	//杆在xz平面上的投影坐标值
	float delt_x = 0.2;			
	float delt_z = 0.2;
	//杆底部在y轴上的偏移
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
	
	// 1.先绘制长杆子,在默认坐标系下绘制一个长方体
	// 1.1生成显示列表,初始化时生成显示列表
	if(pModelSelect->m_stringList == 0)
	{
		pModelSelect->m_stringList = glGenLists(1);		
			
		glNewList(pModelSelect->m_stringList, GL_COMPILE);

		//绘制长方体的底面,2个三角形
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   pass_y, delt_z);	
			glVertex3f(delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, -delt_z);	

			glVertex3f(delt_x,   pass_y, delt_z);
			glVertex3f(-delt_x,   pass_y, -delt_z);	
			glVertex3f(-delt_x,   pass_y, delt_z);	
					
		glEnd();

		//绘制长方体的顶面,2个三角形
		glBegin(GL_TRIANGLES);
			glVertex3f(delt_x,   y_rectange, delt_z);	
			glVertex3f(delt_x,   y_rectange, -delt_z);	
			glVertex3f(-delt_x,   y_rectange, -delt_z);	

			glVertex3f(delt_x,   y_rectange, delt_z);
			glVertex3f(-delt_x,   y_rectange, -delt_z);	
			glVertex3f(-delt_x,   y_rectange, delt_z);	
					
		glEnd();
		
		//绘制长方体的4个侧面,每个侧面由2个三角形组成
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
	//转换为宽字符
	sprintf(text_char, "%d",pModelSelect->m_modeltype);
	ttfAnsi2Unicode(text, text_char, &num);
#endif	
	
#if 1
	// 2.恢复到世界坐标系，移动到模型的世界坐标系位置下
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
	// 3.模型矩阵重新计算时，重新计算string矩阵值
	if(pModelSelect->m_stringWorldChange == TRUE)
	{
		//矩阵计算完并存储
		Matrix44 model_mx, temp_mx;
		Matrix44 MxScale;
		Matrix44 MxWorld;	
		Matrix44 MxRotate;

		f_float64_t size = 0.0;

		//放大倍数设定为固定大小，取消跟随模型的放大倍数
		size = pModelSelect->m_modelScale;	
		size = 2;//5;					
	
		//复用模型的位置转换矩阵值
		dMatrix44Copy(&MxWorld, &pModelSelect->m_mxWorld );	
		//计算缩放矩阵并与位置转换矩阵相乘
		dMatrix44MemSet(&MxScale, size, 0, 0, 0,
								   0, size, 0, 0,
								   0, 0, size, 0,
								   (1-size)*MxWorld.m[12], (1-size)*MxWorld.m[13], (1-size)*MxWorld.m[14], 1);
		Maxtrix4x4_Mul(&temp_mx, &MxWorld, &MxScale);
		//计算姿态转换矩阵并与之前转换矩阵相乘
		dMatrix44RPYmemSet(&MxRotate, 0, 0, 0);
		Maxtrix4x4_Mul(&model_mx, &MxRotate, &temp_mx);
		
		//保存最终的转换矩阵值
		dMatrix44Copy(&pModelSelect->m_stringWorld, &model_mx );		
		
		pModelSelect->m_stringWorldChange = FALSE;
	}
	//转换矩阵与场景绘制的模视矩阵相乘，就将坐标系转换到字体绘制所在的坐标系下
	glMultMatrixd(pModelSelect->m_stringWorld.m);

#if 0	//lpf delete 2017-2-28 14:36:21
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
//	glEnableEx(GL_POLYGON_SMOOTH);
	glDisableEx(GL_CULL_FACE);
#endif	
	// 4.调用显示列表,绘制长杆子
	glDisableEx(GL_BLEND);
	glColor3ub(10,10,10);
	glCallList(pModelSelect->m_stringList);
	glEnableEx(GL_BLEND);
	
	// 5.计算字符旋转角度,使用经纬度计算
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


//	anglepout  = pModelSelect->m_modelAngle.x + anglep12;		//不跟随模型旋转
	anglepout  = anglep12 ;									//只根据视点位置旋转
	
	//6.绘制字符
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

//用户层使用的绘制文字的函数，按照图层的含义绘制
void RenderUserTxt(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	// 1.循环链表
	pstListHead = &(pModelScene->pScene_ModelList);
	
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
	glEnableEx(GL_POLYGON_SMOOTH);
#else
	glPushAttrib(GL_COLOR_BUFFER_BIT/*|GL_CURRENT_BIT*/|GL_DEPTH_BUFFER_BIT | GL_ENABLE_BIT);
#endif
	glDisableEx(GL_CULL_FACE);

	// 准备字体绘制环境，移动到真正字体绘制之前，否则会导致标牌无法绘制
	//ttfDrawStringPre(g_TTFFont);
	//glDisableEx(GL_BLEND);
		
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				// 3.判断文字图层是否绘制
				if(lpsinglemodel->m_flagUser[1] == TRUE)
				{
					// 4.绘制文字
					RenderModelTxt(NULL, lpsinglemodel, pModelScene);					
				}		
			}
		}
	}	
	
	//恢复alpha值
	glColor4ub(255,255,255,255);

	//恢复字体绘制环境,移动到真正字体绘制之前，否则会导致标牌无法绘制
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
** 函数名: BOOL RenderPlanes(sGLRENDERSCENE* pModelScene)
**
** 描述:  设置飞机模型的绘制参数,实际并不进行绘制
**
** 输入参数:  pModelScene           //渲染场景句柄           
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  
**
**.EH--------------------------------------------------------
*/
BOOL RenderPlanes(sGLRENDERSCENE* pModelScene)
{
	f_int32_t i = 0; 

	ModelRender modelplane;	// 飞机模型绘制变量
	f_int32_t model_id = pModelScene->airplane_id;	//飞机的模型ID

	//0.获取当前视口场景下模型绘制链表
	AttachpScene2Model(pModelScene);

	// 1.判断飞机模型是否完成初始化
	if(pModelScene->airplane_id == 0)
	{
		// 1.1.模型初始化完成后才进行飞机模型初始化
		if(FALSE == GetModelInitFlag())
		{
			return FALSE;
		}
		else
		{
			// 1.2.生成飞机模型节点
			{
				static int id_used = 20102;//20103;

				pModelScene->airplane_id = NewModel(id_used);	
				id_used = 20107;
			}

			// 1.3.飞机模型节点生成失败，直接返回，不再绘制
			if(pModelScene->airplane_id == 0)
			{
				return FALSE;
			}

			// 1.4.设置模型的缩放系数为10
			if(FALSE == SetModelScale(pModelScene->airplane_id, 10))
			{
				return FALSE;
			}	

			// 1.5.设置模型的高度使用海拔高度
			if(FALSE == SetModelHeiMode(pModelScene->airplane_id, 2))
			{
				return FALSE;
			}

			// 1.6.设置飞机模型是否在小地图显示
			SetModelScreenFlag(pModelScene->airplane_id, TRUE);		

			// 1.7.设置飞机模型在小地图显示的样式
			SetModelScreenFlagType(pModelScene->airplane_id,  17);	
			
		}

		// 1.8.第一次生成飞机模型后直接返回，不绘制飞机模型
		return FALSE;
	}

	// 2.实时更新飞行参数(6个自由度),区分漫游和非漫游模式
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

	modelplane.m_modelAngle.x =	360.0 - pModelScene->attitude.yaw;//g_pAirPlanes[i].m_Entity.m_nYaw;	//lpf add 因为yaw 已经从-180--180转为0--360，此处输入范围是-180--180；
	modelplane.m_modelAngle.y = pModelScene->attitude.pitch;//g_pAirPlanes[i].m_Entity.m_nPitch;
	modelplane.m_modelAngle.z = pModelScene->attitude.roll;//g_pAirPlanes[i].m_Entity.m_nRoll;

	// 2.1.设置飞机模型的经纬高
	if(FALSE == SetModelPos(pModelScene->airplane_id, modelplane.m_modelPos))
	{
		return FALSE;
	}

	// 2.2.设置飞机模型的航向、俯仰、横滚
	if(FALSE == SetModelAngle(pModelScene->airplane_id, modelplane.m_modelAngle))
	{
		return FALSE;
	}


	// 3.更新屏蔽标志
	// 座舱模式下不显示飞机和用户图层(共4层：尾迹、文字、长方体、用户自定义)
	if(eVM_COCKPIT_VIEW == pModelScene->camctrl_param.view_type)	
	{
		// 3.1.座舱模式下屏蔽飞机模型的绘制
		SetModelShield(model_id, TRUE);
		
		// 3.2.座舱模式下飞机模型的第0/1/2层用户图层均不绘制
		SetModelUserAll(model_id, FALSE);

		// 3.3.座舱模式下飞机模型的第3层用户图层不绘制
		SetModelUser(model_id,FALSE,3);

	}
	else	//其余模式下均绘制飞机模型
	{
		// 3.4.其余模式下不屏蔽飞机模型的绘制
		SetModelShield(model_id, FALSE);

		// 3.5.其余模式下飞机模型的第0/1/2层用户图层均绘制
		SetModelUserAll(model_id, TRUE);

		// 3.6.其余模式下飞机模型的第2/3层用户图层均不绘制
		SetModelUser(model_id,FALSE,3);
		SetModelUser(model_id,FALSE,2);

		//SetModelUser(model_id,TRUE,0);
		
	}
	
//	SetModelDisplay(pModelScene->airplane_id, 11+Particletypetemp);

//	SetModelScale(pModelScene->airplane_id, Particletypetemp1);
	
	return TRUE;
	
}

//绘制世界坐标系系的球，世界坐标系为原点,在用户图层使用时，以模型的坐标系原点为原点；
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

	//绘制下端三角形
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

	//绘制中间的四边形
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

	//绘制下端三角形
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


//测试，三维树 lpf 2017-3-16 14:54:25
static f_uint32_t ModelTreeID = 0;				//树的纹理ID
static f_uint32_t ModelTreeListID = 0;			//树的显示列表ID

void RenderWorldTree(char text_out, LPModelRender pModelSelect)
{
#ifdef WIN32
	f_char_t filepath[256] = "F:\\地图组的资料\\DataFile\\model\\tree\\tree.bmp";
#else
	
	f_char_t filepath[256] = "/ahci00/mapData3d/DataFile/model/tree/tree.bmp";
	#endif
	if(pModelSelect->m_flagUser[3] == FALSE)
	{
		return;
	}


	// 1.初始化纹理
	if (ModelTreeID == 0)
	{
		LoadParticlesTexture(filepath, &ModelTreeID);
	}


	// 2.画两个交叉的面，并贴图
//	glEnableEx(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//	glEnableEx(GL_TEXTURE_2D);
//	glBindTexture(GL_TEXTURE_2D, ModelTreeID);
//	glDisableEx(GL_CULL_FACE);
//	glDepthMask(GL_FALSE);
	
	
	
	glScalef(20,20,20);

	// 3.使用显示列表
	if(ModelTreeListID == 0)
	{
		ModelTreeListID = glGenLists(1);
		glNewList(ModelTreeListID, GL_COMPILE);
		{

			// 2.1.绘制面
		//	glColor4ub(150,150,150,200);		//赋予颜色混合程度
		#if 1
			//第一个面
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

			//第二个面
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

			//第三个面
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

			//第四个面
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
				//第一个面
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
			//第一个面
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
	函数:生成单个的树模型
	输入:经度，纬度，倍数，偏航角，单位:度
*/
BOOL GenerateSingleTree(double lon, double lat , double scale,double yaw)
{
	int model_make;
	PT_3D model_pos;
	PT_3D pitchvalue = {0};
	
	// 1.生成树模型
	model_make = NewModel(20109);				//长方体模型
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
	函数：生成树，生成树的阵列模型群
	输入：单个树的坐标数组，lon, lat, scale；
	内容：根据坐标生成模型，设置模型显示方式；
	放在模型初始化之后；
*/

BOOL GenerateTreeModels()
{
	int tree_num = 900;				//树的个数
	int i = 0;
	int j = 0;								
	double tree_data[900][4] = {0};		//树的数据lon ,lat ,scale
	int tree_num_sqr = sqrt(tree_num);		//树的个数开方值
	
	// 1.准备点的坐标数据
	{
		double lon_begin = 120.8f;
		double lat_begin = 22.98f;

		double lonlat_step = 200.0 * 9.03/1000000;	//树的间隔

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
				//tree_data[i * tree_num_sqr + j][2] = (i * tree_num_sqr + j) %10 + 1;	//放大倍数3-7

				//tree_data[i * tree_num_sqr + j][3] = (i * tree_num_sqr + j) % 360 * 10.0;	//yaw 值0 -360
				tree_data[i * tree_num_sqr + j][3] = 0;
			}

		}
	
	}

	// 2.生成树模型的ID
	for(i=0; i<tree_num; i++)
	{
		if(FALSE ==  GenerateSingleTree(tree_data[i][0], tree_data[i][1] , tree_data[i][2], tree_data[i][3]))
		{
			return FALSE;
		}
	}

	return TRUE;
}






//用户自定义的图层函数,不包含尾迹，外接长方体、字体的函数
void ModelUserDraw(int none, LPModelRender pModelUser)
{	
//	RenderModelRectangle(0, pModelUser);	//不在此处使用，lpf 2017-2-28 14:13:45
//	RenderModelTxt(NULL, pModelUser);		//不在此处使用，lpf 2017-2-28 14:46:40
//	RenderFlyTrace(0, pModelUser);			//不在此处使用，lpf 2017-3-1 10:29:36

	//绘制外接球,可以使用，但是没有使用显示列表
//	RenderWorldBall(0,pModelUser);
	RenderWorldTree(0,pModelUser);
	
}

//提供给二次开发使用的注册函数接口,修改为绘制树的特定函数
void RenderUserDiy(sGLRENDERSCENE* pModelScene)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;
	
	// 0.判断有没有使用注册函数
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
		
	
	// 1.循环链表
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);

		
			// 2.判断模型是否绘制
			if(lpsinglemodel->m_drawOrNot == TRUE )
			{
				
				// 2.1.判断用户自定义图层是否绘制
				if(lpsinglemodel->m_flagUser[3] == TRUE)
				{
					// 3.设置矩阵
					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
					glLoadMatrixd(Get_PROJECTION(pModelScene));
				
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					glLoadMatrixd(Get_MODELVIEW(pModelScene));
					
					glMultMatrixd(lpsinglemodel->m_modelMatrix);
					
					// 4.注册函数绘制	/* 绘制用户图层 */
	//				if(NULL != funcModelRegedit.userLayerDrawFunc)
	//				{	
						funcModelRegedit.userLayerDrawFunc(funcModelRegedit.userLayerParam, lpsinglemodel);
	//				}
					
					// 5.恢复矩阵
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
** 函数名: worldXYZ2screenXY
**
** 描述: 世界坐标系下的点转换为屏幕上的点
**
** 输入参数:  世界坐标
**
** 输出参数：屏幕坐标
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  内部接口2 
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

#if 0			//两种不同的模型生成方式，这种是全部模型，下一种是飞机模型
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
	i++;	modelid[scene][i] = NewModel(20202);//,car,car,3.0,0,0,,,,,,,,,,,,,,,,,,第三个模型,,,,,,,,,
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
	
	//导弹赋初值
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

	//其余模型赋初值
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
				SetModelUser(modelid[scene][i*9+j], FALSE, 0);		//此项影响嵌入式下的帧率
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
			SetModelUser(modelid[scene][i*9+j], FALSE, 0);		//此项影响嵌入式下的帧率
			SetModelUser(modelid[scene][i*9+j], TRUE, 1);
			SetModelUser(modelid[scene][i*9+j], TRUE, 2);
								
		}

		
	}
	
//	SetChildNum(modelid[0], 2);
//	modelid[7] = NewChildModel(modelid[0], 0,2010303);		//导弹模型
//	modelid[8] = NewChildModel(modelid[0], 1,2010302);		//螺旋桨模型
	//SetChildModelPos( modelid[0], 1, PT_3D xyz);
	//SetModelAngleAdd( modelid[0], 45, 1);

//	userFuncModelRegedit(ModelTest, "成都市");
//	userFuncModelRegedit(RenderFlyTrace, NULL);
//	userFuncModelRegedit(RenderModelRectangle, NULL);
	userFuncModelRegedit( (USER_MODEL_DRAW_FUNC)ModelUserDraw, NULL);

	SetModelUser(modelid[scene][0], TRUE, 0);

//	GenerateTreeModels();		// lpf add for 树群的生成函数2017-4-18 11:01:41
	if(0)
	{
		PT_4D color_picture = {0};

		for(i = 0; i<6;i++)
		{
			//lpf test for  纯色模型
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

#if 0			//两种不同的模型生成方式，这种是全部模型，下一种是飞机模型
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
	i++;	modelid[scene][i] = NewModel(20202);//,car,car,3.0,0,0,,,,,,,,,,,,,,,,,,第三个模型,,,,,,,,,
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

	//导弹赋初值
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

	//其余模型赋初值
	for(i = 0; i<9;i++)		
	{
		latlonhei_used.y = latlonhei.y + (i)*0.002;			

		SetModelPos(modelid[scene][i], latlonhei_used);
		SetModelAngle(modelid[scene][i], angle);
		SetModelScale(modelid[scene][i], 10);

		SetModelDisplay(modelid[scene][i],  0);

		SetModelUser(modelid[scene][i], FALSE, 0);		//此项影响嵌入式下的帧率
		SetModelUser(modelid[scene][i], TRUE, 1);
		SetModelUser(modelid[scene][i], FALSE, 2);

	}


	return TRUE;

}

int modeltypeset = 6;	//lpf add 2017-2-20 14:48:32 外部测试模型不同的显示效果

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

	//测试模型的记录数据读写
	if(0)
	{
		SetModelAirline(&modelid[scene][0], 6);

		SetModelPosAdd(modelid[scene][0], 0.0001, 0);
	}


	//测试纯色纹理的切换
	if(1)
	{
		PT_4D color_picture = {0};
		static int color_i = 0;

		for(i = 0; i<6;i++)
		{
			//lpf test for  纯色模型
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
功能：用户函数注册
输出：0：注册成功，-1：注册失败
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
** 函数名: RenderScreenModelFlag
**
** 描述:  在屏幕上绘制小地图，标注模型的位置
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口3, used in Flight3d.c
**
**.EH--------------------------------------------------------
*/





/*
计算fovx的值, 单位是度,暂时取全局变量pPaint_unit的值
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
计算两个点之间的距离,二维情况下
*/
double DistanceTwoPoint2D(PT_2D point1, PT_2D point2)
{
	return sqrt( sqr(point1.x - point2.x) + sqr(point1.y - point2.y)	);
}


/*
求一元二次方程ax*x+bx+c=0的解
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
	计算平面内直线与圆的交点，圆心在原点
	输入:直线的斜率K，过的点坐标pointA,  圆的半径R
	输出:交点1，交点2
	直线表达式: y = kx + d

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

	// 1.完善直线表达式
	d = pointA.y - k*pointA.x;

	// 2.计算交点的x坐标x1 和x2
	CalOneParamDoubleTime(sqr(k)+1, 2*k*d, sqr(d) - sqr(R),
		&x1, &x2, &flag);

	// 3.判断是否有效输出
	if(flag == FALSE)
	{
//		printf("视景体与地球没有交点\n");
		*usefull = FALSE;
		return;
	}

	// 4.计算交点的y1,y2
	y1 = k * x1 + d;
	y2 = k * x2 + d;

	// 5.输出交点的值
	point_intersection1->x = x1;
	point_intersection1->y = y1;
	point_intersection2->x = x2;
	point_intersection2->y = y2;
	*usefull = TRUE;

}

/*
//计算纬度在高的方向上的偏移量
//输入:视场角angle_lat, 模式mode, 0: 正常输出；mode : 1, 输出lon 值
//输出:在纬度的高值方向上的偏移量
*/
void GeoCalHiLat(double angle_lat, int mode, double *offset_hi_lat)
{
	double a,b,c;				//当前视点的世界坐标
	double k,d;				// y = kx + d
	double alpha, bta;			//角度值

	BOOL flag;

	double y1,x1=0,z1,lon1,lat1,hei1;

	double lat_hi, lat_low, lon_hi, lon_low;
	PT_2D point_line;				//视点在平面上的投影坐标
	PT_2D point1, point2;			//直线与圆的交点

	double viewport_center_latlonhei[3] = {0};	//视点的当前位置lat, lon, hei

	double lat_range, lon_range;		//纬度和经度的跨度

	// 1.获取当前视点的世界坐标
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





	// 2.计算lat_hi 考虑yz 平面,视点在yz平面的投影坐标是(sqrt(a*a + b*b), c)
	
		// 2.0.计算角度值
		point_line.x = sqrt(sqr(a) + sqr(b));
		point_line.y = c;
		alpha = atan(point_line.y/point_line.x) *RA2DE;
		bta = angle_lat/2;

		// 2.1.计算lat_hi
		if(c < 0)
		{
			// 南纬	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//北纬
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// 选择第一个点
				z1 = point1.y;
			
			}
			else
			{
				// 选择第二个点
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi 视景体与地球没有交点\n");
			return;
		}

		y1 = (z1-d)/k;

		if(mode == 0)		//正常输出lat 的高值
		{
			// 1.判断lat 高值是否越过90度
			// 根据直线与YZ 平面的交点确定高纬度点是否越过+/- 90度

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
		else				//输出旋转后的lon 值
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

		// 保证输出的lat_range是正值
 		if(lat_range < 0.0)
 		{
 			lat_range *= -1;
 		}

		*offset_hi_lat = lat_range;
}



// 计算交点与当前视点的距离，经度转换到纬度计算
void GeoCalCollsion2(double * focaspoint)
{
	double a,b,c;				//当前视点的世界坐标
	double k,d;				// y = kx + d
	double alpha, bta;			//角度值

	BOOL flag;

	double y1,x1=0,z1,lon1,lat1,hei1;

	double lat_hi, lat_low, lon_hi, lon_low;
	PT_2D point_line;				//视点在平面上的投影坐标
	PT_2D point1, point2;			//直线与圆的交点

	double viewport_center_latlonhei[3] = {0};	//视点的当前位置lat, lon, hei

	double lat_range, lon_range;		//纬度和经度的跨度

	// 1.获取当前视点的世界坐标
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





	// 2.计算lat_hi, lat_low, 考虑yz 平面,视点在yz平面的投影坐标是(sqrt(a*a + b*b), c)
	
		// 2.0.计算角度值
		point_line.x = sqrt(sqr(a) + sqr(b));
		point_line.y = c;
		alpha = atan(point_line.y/point_line.x) *RA2DE;
		bta =  ((sPAINTUNIT *)pPaint_unit)->pScene->camctrl_param_input.view_angle / 2; //g_Fovy/2;

		// 2.1.计算lat_hi
		if(c < 0)
		{
			// 南纬	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//北纬
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// 选择第一个点
				z1 = point1.y;
			
			}
			else
			{
				// 选择第二个点
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi 视景体与地球没有交点\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_hi= RA2DE*(lat1);

		lat_range = lat_hi - viewport_center_latlonhei[0]*RA2DE;

		printf("lat_range =%f\n", lat_range*2);

	// 3.计算lon_hi, lon_low，考虑xy平面

		// 2.0.计算角度值
		bta = GetFovX()/2;

		// 2.1.计算lat_hi 等价于lon的偏移值
		if(c < 0)
		{
			// 南纬	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//北纬
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// 选择第一个点
				z1 = point1.y;
			
			}
			else
			{
				// 选择第二个点
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi 视景体与地球没有交点\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_hi= RA2DE*(lat1);

		lon_range = lat_hi - viewport_center_latlonhei[0]*RA2DE;

		printf("lon_range =%f\n", lon_range*2);

	// 4.输出变量

	

}



//计算视景体与地球的交点, 输出顺序:lat_hi, lat_low, lon_hi, lon_low;
void GeoCalCollsion(double * focaspoint)
{
	double a,b,c;				//当前视点的世界坐标
	double k,d;				// y = kx + d
	double alpha, bta;			//角度值

	BOOL flag;

	double y1,x1=0,z1,lon1,lat1,hei1;

	double lat_hi, lat_low, lon_hi, lon_low;
	PT_2D point_line;				//视点在平面上的投影坐标
	PT_2D point1, point2;			//直线与圆的交点

	// 1.获取当前视点的世界坐标
// 	a = 	g_SphereRender.m_Render.m_ptEye.x;
// 	b =	g_SphereRender.m_Render.m_ptEye.y;
// 	c =  	g_SphereRender.m_Render.m_ptEye.z;
	a = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.x;	//g_SphereRender.m_Render.m_ptEye.x;	
	b =	((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.y;//g_SphereRender.m_Render.m_ptEye.y;		
	c = ((sPAINTUNIT *)pPaint_unit)->pScene->camParam.m_ptEye.z;	//g_SphereRender.m_Render.m_ptEye.z;	


//	printf("a = %f, b = %f, c = %f, R = %f\n",a,b,c, sqrt(sqr(a) + sqr(b) + sqr(c)));

	// 2.计算lat_hi, lat_low, 考虑yz 平面,视点在yz平面的投影坐标是(sqrt(a*a + b*b), c)
	
		// 2.0.计算角度值
		point_line.x = sqrt(sqr(a) + sqr(b));
		point_line.y = c;
		alpha = atan(point_line.y/point_line.x) *RA2DE;
		bta = ((sPAINTUNIT *)pPaint_unit)->pScene->camctrl_param_input.view_angle /2;

		// 2.1.计算lat_hi
		if(c < 0)
		{
			// 南纬	
			k = tan((alpha + bta)* DE2RA + PI);
		}
		else
		{
			//北纬
			k = tan((alpha - bta)* DE2RA);
			
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// 选择第一个点
				z1 = point1.y;
			
			}
			else
			{
				// 选择第二个点
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_hi 视景体与地球没有交点\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_hi= RA2DE*(lat1);

		// 2.2.计算lat_low
		if(c<0)
		{
			// 南纬	
			k = tan((alpha - bta)* DE2RA + PI);
		}
		else
		{
			//北纬
			k = tan((alpha + bta)* DE2RA);		
		}
		
		d = point_line.y - k * point_line.x;

		CalLineWithRound(point_line, k, EARTH_RADIUS,&point1, &point2, &flag);

		if(flag == TRUE)
		{
			// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// 选择第一个点
				z1 = point1.y;
			
			}
			else
			{
				// 选择第二个点
				z1 = point2.y;

			}
		}
		else
		{
			printf("lat_low 视景体与地球没有交点\n");
			return;
		}

		y1 = (z1-d)/k;
		
		XYZToEarth(0,y1,z1, &lon1, &lat1, &hei1);

		lat_low = RA2DE*(lat1);

		printf("lat_hi = %f, lat_low = %f, lat_range = %f\n",lat_hi, lat_low, lat_hi - lat_low);		

	// 3.计算lon_hi, lon_low，考虑xy平面
		// 3.0.计算角度值
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

		// 3.1. 计算lon_hi
		while(1)
		{


			//第三象限和第二象限
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
				//距离最小的点是需要的点
				// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
				double distance1 = 0.0, distance2 = 0.0;

				distance1 = DistanceTwoPoint2D(point1, point_line);
				distance2 = DistanceTwoPoint2D(point2, point_line);

				if(distance1 < distance2)
				{
					// 选择第一个点
					y1 = point1.y;
				
				}
				else
				{
					// 选择第二个点
					y1 = point2.y;

				}

			}
			else
			{
	//			printf("lon_hi 视景体与地球没有交点\n");

				bta -= 1;
				continue;
			}

			x1 = (y1-d)/k;
			
			XYZToEarth(x1,y1,0, &lon1, &lat1, &hei1);

			lon_hi= RA2DE*(lon1);

			break;


		}

	// 3.2. 计算lon_low

	bta = GetFovX()/2;

	while(1)
	{

		//第三象限和第二象限
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
			// 根据交点与投影点的距离判断选取哪一个点，距离小的点是交点
			double distance1 = 0.0, distance2 = 0.0;

			distance1 = DistanceTwoPoint2D(point1, point_line);
			distance2 = DistanceTwoPoint2D(point2, point_line);

			if(distance1 < distance2)
			{
				// 选择第一个点
				y1 = point1.y;
			
			}
			else
			{
				// 选择第二个点
				y1 = point2.y;

			}
		}
		else
		{
//			printf("lon_low 视景体与地球没有交点\n");
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


	// 4.输出变量
	* focaspoint = lat_hi;
	* (focaspoint + 1) = lat_low;
	* (focaspoint + 2) = lon_hi;
	* (focaspoint + 3) = lon_low;
	

}


/*
**函数:绘制视口在小地图上的矩形框
**输入:飞机在小地图上的位置，矩形框长和宽的一半距离
**输出:
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

// extern sFLYPARAM g_FlyParam ;  //当前飞行参数_接口用
// extern sCTRPARAM3D g_CtrlParam;
// extern unsigned int g_MeasureSemiPixelNum ;// 量程环半径像素，三维地图使用

void RenderPlaneRectange(f_float64_t width, f_float64_t height)
{
	int plane_x, plane_y;		
	float plane_halfwidth,plane_halfheight;				//小地图中的矩形框大小

	float viewport_world_width,viewport_world_height;	//视口的世界坐标系下的大小

	float viewport_width,viewport_height;					//视口的像素大小

	// 1.获取视口的像素大小
	viewport_width = g_SphereRender.m_Render.m_rcView.right - g_SphereRender.m_Render.m_rcView.left;
	viewport_height = g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom;	

	// 2.计算视口的世界大小
	viewport_world_width = g_CtrlParam.base.measure_distance / g_MeasureSemiPixelNum * viewport_width;
	viewport_world_height = g_CtrlParam.base.measure_distance / g_MeasureSemiPixelNum * viewport_height;
	
	// 3.计算小地图的矩形框大小
	plane_halfwidth =  viewport_world_width /(2*PI*EARTH_RADIUS) *width;
	plane_halfheight = viewport_world_height /(2*PI*EARTH_RADIUS)*height;
	
	// 4.计算矩形框的中心位置在小地图中的
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
	double pointLatLon[4] = {0};			//四个交点
	double pointAround[4][2] = {0};			//四个交点坐标地理坐标
	double pointAroundMct[4][2] = {0};		//四个交点坐标Mct
	double pointAroundSmal[4][2] = {0};		//四个交点坐标小地图
	int  i = 0;
	double viewport_latlon[2] = {0};			//四个交点的范围,lat lon
	
	#if 0
	// 1.计算视景体与地球的交点，即四至点
	GeoCalCollsion(pointLatLon);
	GeoCalCollsion2(pointLatLon);
	
	// 2.经纬度得到范围长度
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
功能：输出飞机符号
输入：
    x_center，y_center 飞机符号中心位置
    symbol_index 符号索引
    angle 旋转角
输出：
    无
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

	// 1.根据每个模型的位置绘制符号
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);


			// 2.判断模型是否显示
			if(lpsinglemodel->m_flagModelScreen == TRUE)
			{

				// 3.根据模型的小地图坐标绘制
				outPutPlaneSymbolEx((int)(lpsinglemodel->m_ModelScreenXY.x), 
					(int)(lpsinglemodel->m_ModelScreenXY.y), 
					lpsinglemodel->m_ModelScreenID, 
					180 + lpsinglemodel->m_modelAngle.x);   //输出飞机符号 
				
			}
			
		}
	}
	
	giveSem(g_SetModelParam);
	
	//本句为了还原默认的纹理映射方式
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_BLEND);
	glDisableEx(GL_TEXTURE_2D);
	glEnableEx(GL_CULL_FACE);
		
}


/*
**函数:绘制屏幕上的小地图,默认小地图的左下角为纹理坐标0,0
**输入:小地图的宽度和高度
*/
static int WorldPicID = 0;		//世界地图的图片的纹理ID
static int WorldPicListID = 0;	//绘制世界地图的显示列表ID

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

	// 1.读取world 图片,生成纹理ID
	if (WorldPicID == 0)
	{
		LoadParticlesTexture(filepath, &WorldPicID);
//		LoadTexture(filepath, &WorldPicID, 0);
	}

	// 2.绘制左下角inwidth * inheight 的小地图
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

	// 3.使用显示列表
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

	//本句为了还原默认的纹理映射方式
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// 3.绘制模型的符号
	RenderModelWorldFlag();

	// 4.绘制小地图的视口范围框
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
**函数:计算模型在小地图上的位置
**输入:小地图的宽度和高度
**输出:模型的小地图位置
*/
void UpdataModelScreenPos(f_float64_t inwidth, f_float64_t inheight)
{
	stList_Head *pstListHead = &(pModelScene->pScene_ModelList);
	stList_Head *pstTmpList = NULL;
	LPModelRender lpsinglemodel = NULL;

	takeSem(g_SetModelParam, FOREVER_WAIT);

	// 1.计算每个模型的位置
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, ModelRender, stListHead);


			// 2.判断模型是否显示
			if(lpsinglemodel->m_flagModelScreen == TRUE)
			{

				// 3.计算模型的位置，根据lon,lat 
				lpsinglemodel->m_ModelScreenXY.x = (lpsinglemodel->m_modelPos.y  + 180.0)/360.0*inwidth;
				lpsinglemodel->m_ModelScreenXY.y = (lpsinglemodel->m_modelPos.x  + 90.0)/180.0*inheight;
			}
			
		}
	}
	
	giveSem(g_SetModelParam);
		
}

//------小地图的尺寸的接口函数----------------------
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
//------------小地图的尺寸的接口函数-----------------

// 绘制小地图函数
void RenderScreenModelFlag()
{	
	f_float64_t world_width = 500.0;		//世界地图的宽度
	f_float64_t world_height = 250.0;		//世界地图的高度

	// 0.准备模型的屏幕位置数据
	UpdataModelScreenPos(world_width, world_height);
	
	// 1.绘制屏幕上的小地图
	RenderScreenWorldPic(world_width, world_height);

	// 2.设置小地图的尺寸
	SetWorldMapSize(world_width, world_height);
	
}
#endif

