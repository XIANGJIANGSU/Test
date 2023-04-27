/*
 *	规定：模型 x轴为模型的侧方，y轴为模型的上方，z轴为飞机的前方， 构成右手坐标系
 *       |z
 *		 |
 *		 |
 *		 |_________x
 *      /
 *     /
 *    /-y
 *  
 */
#ifndef _MODEL_H_
#define _MODEL_H_

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
#include "../../../SE_api_prj/src/mapMain.h"

#include "loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "ModelreadTxtFile.h"
#include "3DSLoader.h"

//void RenderModelLLH(LPModelRender pRenderModelLLH);  
void RenderModelLLH(LPModelRender pRenderModelLLH , sGLRENDERSCENE* pModelSceneLLH)  ;


/*
 *  pModel: 存储模型数据
 *  glList： 用户自定显示列表
 *  use_buildin_list: 是否使用模型内置的现实列表（主要是对m96板子的优化）
 */
//extern void RenderModelXYZ(
//							const LPModel3DS pModel, const f_uint32_t glList, const BOOL use_buildin_list,
//							const f_uint8_t minLevel,	const f_uint8_t maxLevel,
//							const f_float64_t x, const f_float64_t y, const f_float64_t z, 
//							const f_float64_t pitch, const f_float64_t yaw, const f_float64_t roll,
//							const f_float64_t size,
//							LPMatrix4x4 pMxWorld,
//							LPMatrix4x4 pMxRotate,
//							BOOL bShowModel);

/*
 * 功能：由世界坐标计算世界矩阵
 * 输入：x         世界x坐标
 *       y         世界y坐标
 *       z         世界z坐标
 * 输出：pMxWorld  世界矩阵
 * 返回：无
 */
//extern void CalcWorldMatrix(const f_float64_t x, const f_float64_t y, const f_float64_t z, LPMatrix4x4 pMxWorld);

//获取投影矩阵和模式矩阵
f_float64_t * Get_PROJECTION();
f_float64_t * Get_MODELVIEW();

void EnableLight(BOOL bEnable);
void EnableLight0(BOOL bEnable);
void EnableLight1(BOOL bEnable);
/*-----------------------------------------------------------
** 结构体声明
**-----------------------------------------------------------
*/

//typedef List	ModelGroup;

typedef struct tagModelNode
{
	f_char_t		m_chName[64];		// 
	Model3DS	m_Model;			//
	f_uint32_t    m_NewList;			// 显示列表
} ModelNode, *LPModelNode;

extern VOIDPtr pScene[SCENE_NUM];        /* 场景句柄 */


#endif
