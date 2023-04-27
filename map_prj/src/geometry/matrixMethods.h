/**
 * @file matrixMethods.h
 * @brief 该文件定义了矩阵运算的相关算法
 * @author 615地图团队
 * @date 2016-05-26
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
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

/* 设置3x3单位阵(64位浮点型) */
void dMatrix33LoadIdentity(f_float64_t m[9]);
/*	向量乘以齐次矩阵 */
void dMatrix33Multi(LP_PT_3D pOut, const PT_3D pIn, const LPMatrix33 mx);
/* 功能：设置向量旋转矩阵（3×3）*/
void createRotationMatrix33(LPMatrix33 matrix33, f_float64_t sina, f_float64_t cosa, f_float64_t ux, f_float64_t uy, f_float64_t uz);

/************************4x4齐次矩阵******************************************/
void dMatrix44MemSet(LPMatrix44 pOut, 
                     const f_float64_t in00, const f_float64_t in01, const f_float64_t in02, const f_float64_t in03,
					 const f_float64_t in10, const f_float64_t in11, const f_float64_t in12, const f_float64_t in13,
					 const f_float64_t in20, const f_float64_t in21, const f_float64_t in22, const f_float64_t in23,
					 const f_float64_t in30, const f_float64_t in31, const f_float64_t in32, const f_float64_t in33);
					 
void calcWorldMatrix44(const f_float64_t x, const f_float64_t y, const f_float64_t z, LPMatrix44 pMxWorld);	
void dMatrix44RPYmemSet( LPMatrix44 pOut, const f_float64_t pitch, const f_float64_t yaw, const f_float64_t roll );
void dMatrix44Multi(LP_PT_3D pOut, const LP_PT_3D pIn, const LPMatrix44 mx);

/* 设置4x4单位阵(64位浮点型) */
void dMatrix44Identity(f_float64_t m[16]);
/*	4维向量(64位浮点型)乘以齐次矩阵 */
void vec4dMultMatrix(const f_float64_t matrix[16], const f_float64_t in[4], f_float64_t out[4]);
/* 4x4矩阵(64位浮点型)相乘 */
void dMatrices44Mult(const f_float64_t in_left[16], const f_float64_t in_right[16], f_float64_t out[16]);
/* 4x4矩阵(64位浮点型)求逆 */
BOOL dMatrix44Invert(const f_float64_t m[16], f_float64_t invOut[16]);
/* 加载单位矩阵 */
void dMatrix44LoadIdentity(f_float64_t m[16]);		
/* 根据旋转角和旋转轴创建旋转矩阵 */
void createRotationMatrix44(f_float64_t angle, f_float64_t x, f_float64_t y, f_float64_t z, f_float64_t mMatrix[16]);	 

//把右矩阵赋值给左矩阵
void dMatrix44Copy(LPMatrix44 pLeft, LPMatrix44 pRight);

//矩阵转置函数
void dMaxtrix44_Inversion(LPMatrix44 pInOut);

//老的矩阵相乘的函数
void Maxtrix4x4_Mul(Matrix44* pOut, Matrix44* pIn0, Matrix44 *pIn1);

void dMatrices44MultNoNormal(const f_float64_t in_left[16], const f_float64_t in_right[16], f_float64_t out[16]);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
 
