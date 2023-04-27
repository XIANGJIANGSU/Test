/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: ModelApi.h
**
** 描述: 本文件包含模型初始化和模型绘制的接口调用函数的头文件。
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
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** 头文件引用
**-----------------------------------------------------------
*/
#ifndef _MODELAPI_H_
#define _MODELAPI_H_


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

// #include "Loadmodel.h"
// #include "Model.h"
// #include "ModelApi.h"
#include "../model/3DSLoader.h"
#include "../mapApp/appHead.h"
/*-----------------------------------------------------------
** 结构体声明
**-----------------------------------------------------------
*/
#define FLYTRACE_NODENUM_MAX	100		// 每个飞机最多存储100个飞行轨迹点
typedef struct tagTraceNode
{
	BOOL	m_bUse;		// 该节点是够被使用
	PT_3D	m_ptUp;		// 相对于飞机向上的点
	PT_3D	m_ptDown;   // 相对于飞机向下的点
} TraceNode, *LPTraceNode;

//模型信息结构体
typedef struct tagModelRender{
	f_int32_t	m_modelFlag;		//标记此机构体是模型
	f_int32_t m_modeltype;		//model type in config.txt
	f_int32_t m_modelID;			//model ID used outside
	BOOL	m_flagshield;			//model 是否屏蔽标志
	
	PT_3D	m_modelPos;			//lat, lon, hei
	PT_3D	m_modelAngle;		//yaw, pitch, roll
	f_float64_t	m_modelScale;	//scale index
	f_float64_t	m_modelSize;	//size
	f_int32_t	m_modelDisplaytype;	//display type
	PT_3D     m_modelPoxWorld;	//x,y,z in world vertex
	f_int32_t m_heimode;			//离地高度0，绝对高度1, 海拔高度2
	PT_4D m_color[4];			//color used in draw,  线框,屏幕框,光环,纯颜色
	f_int32_t m_colorNum_use;		//应该使用的颜色数组序号
	f_int32_t m_colorPicture;		//纯色模式下使用的纹理ID
	f_int32_t m_colorPictureIsNeedRebuild;	//纯色模式下使用的纹理ID 是否需要重新生成:  0 : 否,1 : 是
	LPModel3DS m_pModel3ds;		//model 3ds 内容
	BOOL	m_selectMode;		//select mode Render选择方式的绘制模型
	f_float64_t m_modelMatrix[16];	//旋转移动矩阵
	BOOL	m_MatrixChange;		//旋转矩阵是否改变

	Matrix44	m_mxRotate;			// 局部坐标的姿态旋转矩阵
	Matrix44	m_mxWorld;			// 局部坐标到世界坐标变换矩阵

	f_int32_t m_nodeIndex;		//节点序列号

	BOOL		m_flagUser[4];			//用户图层显示标志,分别代表：尾迹，文字，长方体，用户自定义
//	BOOL		m_flagTrace;							//是否显示轨迹,废弃
	
	TraceNode	m_vFlyTrace[FLYTRACE_NODENUM_MAX];	// 飞机飞行轨迹点
	f_uint16_t 	s_pTraceIndex;						// 记录当前可用的点
	f_uint16_t 	s_pTraceCount;						// 节点计时器	

	f_int32_t m_childNum;						//child model number
//	struct tagModelRender * m_childModel;		//child model struct
	stList_Head 		m_childlist;						//child model list

	f_uint32_t	 m_glLists;							/**外界长方体显示列表ID*/
	
	f_uint16_t  m_pString[4][16];					//模型的UNICOD码字符串,最多支持4行
	f_int32_t   m_stringNum[4];					//每行字符串的字符个数
	f_uint32_t	m_stringList;					/**标识号的长方形杆显示列表ID*/
	Matrix44	m_stringWorld;					//模型标识号的局部坐标系到世界坐标系的变换矩阵
	BOOL		m_stringWorldChange;			//该变换矩阵是否重新计算
	
	BOOL		m_drawOrNot;					//是否绘制该模型，TRUE,绘制,仅作用于父节点

	//与小地图有关
	BOOL	m_flagModelScreen;				//模型符号是否在小地图绘制
	PT_2D	m_ModelScreenXY;				//模型符号在小地图的位置
	f_int32_t m_ModelScreenID;				//模型在小地图的符号ID
	//与小地图有关
	stList_Head	stListHead;					//链表的节点头
}ModelRender,*LPModelRender;

typedef void (*USER_MODEL_DRAW_FUNC)(void *userLayerParam, void *userLayerParam1);

/*-----------------------------------------------------------
** 函数声明
**-----------------------------------------------------------
*/

//初始化模型数据、白模城市、模型字体及启动判断模型是否可见的调度任务
BOOL InitModel();

//绘制模型
// 输入参数:  SelectModel; TRUE-选择模型的绘制方式,FALSE-正常模式绘制模型
//            pModelScene  渲染场景句柄
BOOL RenderModel(BOOL SelectModel , sGLRENDERSCENE* pModelScene);

//void RenderScreenModelFlag();

/*用户层调用的API*/

//根据模型类型(modeltype)新增模型，添加到模型绘制链表，并返回模型ID(即模型结构体指针)
f_int32_t NewModel(f_int32_t modeltype);
//根据模型ID(即模型结构体指针)删除模型，并从模型绘制链表中删除
BOOL DestroyModel(f_int32_t modelID);

//根据模型ID(即模型结构体指针)设置模型的lat, lon, height
BOOL SetModelPos(f_int32_t modelID, PT_3D latlonhei);
//根据模型ID(即模型结构体指针)返回模型的lat, lon, height
BOOL GetModelPos(f_int32_t modelid, LP_PT_3D model_pos);

//根据模型ID(即模型结构体指针)设置模型的yaw,pitch,roll
BOOL SetModelAngle(f_int32_t modelID, PT_3D yawpitchroll);

//根据模型ID(即模型结构体指针)设置模型的缩放系数
BOOL SetModelScale(f_int32_t modelID, f_float64_t modelscale);
//根据模型ID(即模型结构体指针)返回模型的缩放系数
BOOL GetModelScale(f_int32_t modelid, f_float64_t* model_scale);

//根据模型ID(即模型结构体指针)设置模型的显示方式
//0：正常显示，1：闪动，2：线框(正常大小,颜色默认为绿色)，
//3：模型+线框(1.02倍大小,颜色默认为绿色)，4：模型+包络线(正常大小,线粗3倍,颜色默认为红色)，
//5：光环效果,即模型+纯色(1.05倍大小,颜色默认为金黄色)，6：纯色纹理(颜色默认为红色)
BOOL SetModelDisplay(f_int32_t modelID, f_int32_t modeldisplaytype);
//根据模型ID(即模型结构体指针)返回模型的显示方式
BOOL GetModelType(f_int32_t modelid, f_int32_t *model_type);

//根据模型ID(即模型结构体指针)设置模型的高度值类型，相对高度0，绝对高度1, 海拔高度2
BOOL SetModelHeiMode(f_int32_t modelID,f_int32_t HeiMode);

//BOOL SetModelTraceFlag(int modelid, BOOL flag);
//根据模型ID(即模型结构体指针)设置模型的屏蔽标志
BOOL SetModelShield(f_int32_t modelID, BOOL shieldflag);

//根据模型ID(即模型结构体指针)设置模型的指定用户图层显示标志
//输入参数: 模型的ID，显示标志flag，显示图层mode:0-尾迹、1-文字、2-长方体，3-自定义
BOOL SetModelUser(f_int32_t modelID, BOOL userflag, int mode);

//根据模型ID(即模型结构体指针)设置模型的所有用户图层显示标志
BOOL SetModelUserAll(f_int32_t modelID, BOOL userflag);

//根据模型ID(即模型结构体指针)设置模型的各种显示模式的颜色
BOOL SetModelColor(int modelid, int color_type, PT_4D colorset);

//根据模型ID(即模型结构体指针)设置模型的在小地图屏幕符号显示标志的样式
BOOL SetModelScreenFlagType(f_int32_t modelID, int  type);

//根据模型ID(即模型结构体指针)设置模型的各行文字
BOOL SetModelText(f_int32_t modelID, unsigned char *textString, int line);

//根据模型ID(即模型结构体指针)设置模型的lat, lon, height的增量,让模型移动
//输入参数: 模型id,模型位置增量值,移动哪个分量lat :0,lon:1,hei:2
BOOL SetModelPosAdd(f_int32_t modelid, f_float64_t model_dpos, f_int32_t mode);

//根据模型ID(即模型结构体指针)设置模型的yaw,pitch,roll的增量,让模型姿态发生变化
//输入参数: 模型id,模型位置增量值,移动哪个分量lat :0,lon:1,hei:2
BOOL SetModelAngleAdd(f_int32_t modelid, f_float64_t model_dagnle, f_int32_t mode);


BOOL NewChildModel(f_int32_t modelid,  f_int32_t model_childID, f_int32_t modeltype);
BOOL SetChildModelPos(f_int32_t modelid, f_int32_t childmodelID, PT_3D xyz);
BOOL SetChildModelAngle(f_int32_t modelid, f_int32_t childmodelID, PT_3D yawpitchroll);
BOOL SetChildModelScale(f_int32_t modelid, f_int32_t childmodelID, f_float64_t scale);
BOOL SetChildModelPosAdd(f_int32_t modelid, f_int32_t childmodelID, f_float64_t model_dpos, f_int32_t mode);
BOOL SetChildModelAngleAdd(f_int32_t modelid, f_int32_t childmodelID, f_float64_t model_dangle, f_int32_t mode);
BOOL SetChildNum(f_int32_t modelid, f_int32_t num);

BOOL NewChildModelDynamic(f_int32_t parent_id,  f_int32_t child_id);
BOOL DeleteChildModelDynamic(f_int32_t parent_id, f_int32_t child_number, f_int32_t* child_id);

void AttachpScene2Model(sGLRENDERSCENE* pModelScene);

//用户层调用的碰撞检测API
f_int32_t CheckModelcollisionScreen(PT_3D collisionPoint, f_float64_t collisionRadius, sGLRENDERSCENE* pModelScene);
//f_int32_t CheckModelcollisionScreen(PT_3D collisionPoint, f_float64_t collisionRadius);

BOOL RenderFlyTrace(int nonecon, LPModelRender pModelSelect);
void RenderModelRectangle(int nonecon, LPModelRender pModelSelect);
void RenderModelTxt(char * text_out, LPModelRender pModelSelect, sGLRENDERSCENE* pModelScene);
void ModelUserDraw(int none, LPModelRender pModelUser);

//其他绘图函数中使用的API
void set_model_displaytype(LPModelRender model_select,f_int32_t displaytype);
BOOL set_model_colorNumUse(LPModelRender model_select,f_int32_t color_use);
BOOL set_model_color(LPModelRender model_select,int color_type, PT_4D colorset);
void CreateList(stList_Head *plist );

int userFuncModelRegedit(USER_MODEL_DRAW_FUNC userLayerDrawFunc, void *userLayerParam);

//测试函数
BOOL TestModelAppMake(int scene);
BOOL AddModelToScene(int scene);
void TestModelAppSet(int scene);

double GetWorldMapSizeX();
double GetWorldMapSizeY();

extern void Maxtrix4x4_Mul(Matrix44* pOut, Matrix44* pIn0, Matrix44 *pIn1);

extern int LoadParticlesTexture(const f_char_t* filepath, f_uint32_t* textureID);
/*-----------------------------------------------------------
** 外部变量声明
**-----------------------------------------------------------
*/



#endif
