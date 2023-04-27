/*
 *	��άģ������
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
	// ��������
	f_float64_t	m_nLon;		// ����
	f_float64_t	m_nLat;		// γ��
	f_float64_t	m_nHei;		// ���θ߶�
	// �ռ�����
	f_float64_t	m_nX;		// x,y,z
	f_float64_t	m_nY;
	f_float64_t	m_nZ;
	// ��̬��
	f_float64_t	m_nPitch;	//	����
	f_float64_t	m_nYaw;		//	ƫ��
	f_float64_t	m_nRoll;	//	����

	Matrix44	m_mxRotate;	// �ֲ��������̬��ת����
	Matrix44	m_mxWorld;	// �ֲ����굽��������任����
} ObjectEntity, *LPObjectEntity;


#endif


