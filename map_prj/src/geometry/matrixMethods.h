/**
 * @file matrixMethods.h
 * @brief ���ļ������˾������������㷨
 * @author 615��ͼ�Ŷ�
 * @date 2016-05-26
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _matrixMethods_h_ 
#define _matrixMethods_h_

#include "../define/macrodefine.h"
#include "../define/mbaseType.h"
#include "vectorMethods.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* ����3x3��λ��(64λ������) */
void dMatrix33LoadIdentity(f_float64_t m[9]);
/*	����������ξ��� */
void dMatrix33Multi(LP_PT_3D pOut, const PT_3D pIn, const LPMatrix33 mx);
/* ���ܣ�����������ת����3��3��*/
void createRotationMatrix33(LPMatrix33 matrix33, f_float64_t sina, f_float64_t cosa, f_float64_t ux, f_float64_t uy, f_float64_t uz);

/************************4x4��ξ���******************************************/
void dMatrix44MemSet(LPMatrix44 pOut, 
                     const f_float64_t in00, const f_float64_t in01, const f_float64_t in02, const f_float64_t in03,
					 const f_float64_t in10, const f_float64_t in11, const f_float64_t in12, const f_float64_t in13,
					 const f_float64_t in20, const f_float64_t in21, const f_float64_t in22, const f_float64_t in23,
					 const f_float64_t in30, const f_float64_t in31, const f_float64_t in32, const f_float64_t in33);
					 
void calcWorldMatrix44(const f_float64_t x, const f_float64_t y, const f_float64_t z, LPMatrix44 pMxWorld);	
void dMatrix44RPYmemSet( LPMatrix44 pOut, const f_float64_t pitch, const f_float64_t yaw, const f_float64_t roll );
void dMatrix44Multi(LP_PT_3D pOut, const LP_PT_3D pIn, const LPMatrix44 mx);

/* ����4x4��λ��(64λ������) */
void dMatrix44Identity(f_float64_t m[16]);
/*	4ά����(64λ������)������ξ��� */
void vec4dMultMatrix(const f_float64_t matrix[16], const f_float64_t in[4], f_float64_t out[4]);
/* 4x4����(64λ������)��� */
void dMatrices44Mult(const f_float64_t in_left[16], const f_float64_t in_right[16], f_float64_t out[16]);
/* 4x4����(64λ������)���� */
BOOL dMatrix44Invert(const f_float64_t m[16], f_float64_t invOut[16]);
/* ���ص�λ���� */
void dMatrix44LoadIdentity(f_float64_t m[16]);		
/* ������ת�Ǻ���ת�ᴴ����ת���� */
void createRotationMatrix44(f_float64_t angle, f_float64_t x, f_float64_t y, f_float64_t z, f_float64_t mMatrix[16]);	 

//���Ҿ���ֵ�������
void dMatrix44Copy(LPMatrix44 pLeft, LPMatrix44 pRight);

//����ת�ú���
void dMaxtrix44_Inversion(LPMatrix44 pInOut);

//�ϵľ�����˵ĺ���
void Maxtrix4x4_Mul(Matrix44* pOut, Matrix44* pIn0, Matrix44 *pIn1);

void dMatrices44MultNoNormal(const f_float64_t in_left[16], const f_float64_t in_right[16], f_float64_t out[16]);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
 
