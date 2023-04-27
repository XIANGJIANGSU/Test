/*
 *	�涨��ģ�� x��Ϊģ�͵Ĳ෽��y��Ϊģ�͵��Ϸ���z��Ϊ�ɻ���ǰ���� ������������ϵ
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
 *  pModel: �洢ģ������
 *  glList�� �û��Զ���ʾ�б�
 *  use_buildin_list: �Ƿ�ʹ��ģ�����õ���ʵ�б���Ҫ�Ƕ�m96���ӵ��Ż���
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
 * ���ܣ���������������������
 * ���룺x         ����x����
 *       y         ����y����
 *       z         ����z����
 * �����pMxWorld  �������
 * ���أ���
 */
//extern void CalcWorldMatrix(const f_float64_t x, const f_float64_t y, const f_float64_t z, LPMatrix4x4 pMxWorld);

//��ȡͶӰ�����ģʽ����
f_float64_t * Get_PROJECTION();
f_float64_t * Get_MODELVIEW();

void EnableLight(BOOL bEnable);
void EnableLight0(BOOL bEnable);
void EnableLight1(BOOL bEnable);
/*-----------------------------------------------------------
** �ṹ������
**-----------------------------------------------------------
*/

//typedef List	ModelGroup;

typedef struct tagModelNode
{
	f_char_t		m_chName[64];		// 
	Model3DS	m_Model;			//
	f_uint32_t    m_NewList;			// ��ʾ�б�
} ModelNode, *LPModelNode;

extern VOIDPtr pScene[SCENE_NUM];        /* ������� */


#endif
