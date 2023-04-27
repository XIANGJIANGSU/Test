/*
 *	三维模型类型
 *  
 *
 */
#ifndef _OBJECTENTITY_H_
#define _OBJECTENTITY_H_

#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"

#include "Loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "ModelreadTxtFile.h"

typedef struct tagObjectEntity
{
	// 地理坐标
	f_float64_t	m_nLon;		// 经度
	f_float64_t	m_nLat;		// 纬度
	f_float64_t	m_nHei;		// 海拔高度
	// 空间坐标
	f_float64_t	m_nX;		// x,y,z
	f_float64_t	m_nY;
	f_float64_t	m_nZ;
	// 姿态角
	f_float64_t	m_nPitch;	//	俯仰
	f_float64_t	m_nYaw;		//	偏航
	f_float64_t	m_nRoll;	//	滚动

	Matrix44	m_mxRotate;	// 局部坐标的姿态旋转矩阵
	Matrix44	m_mxWorld;	// 局部坐标到世界坐标变换矩阵
} ObjectEntity, *LPObjectEntity;


#endif


