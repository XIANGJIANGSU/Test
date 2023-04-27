/**
 * @file vectorMethods.h
 * @brief 该文件定义了向量运算的相关算法
 * @author 615地图团队
 * @date 2016-05-26
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _vectorMethods_h_ 
#define _vectorMethods_h_

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* 3F向量运算 */
/* 赋值*/
void vector3FMemSet(LP_PT_3F pt, const f_float32_t x, const f_float32_t y, const f_float32_t z);
void vector3FSetValue(LP_PT_3F pLeft, LP_PT_3F pRight);
/* 加法*/
void vector3FAdd(LP_PT_3F pOut, const LP_PT_3F pt0, const LP_PT_3F pt1);
/* 减法*/
void vector3FSub(LP_PT_3F pOut, const LP_PT_3F pt0, const LP_PT_3F pt1);
/* 乘法*/
void vector3FMul(LP_PT_3F pOut, const LP_PT_3F pIn, const f_float32_t lf);
/* 除法*/
void vector3FDiv(LP_PT_3F pOut, const LP_PT_3F pIn, const f_float32_t lf);
/* 长度*/
f_float32_t vector3FLength(const LP_PT_3F pt);
/* 单位化*/
void vector3FNormalize(LP_PT_3F pInOut);
/* 点乘*/
f_float32_t vector3FDotProduct(const LP_PT_3F pt0, const LP_PT_3F pt1);
/* 负号*/
void vector3FNegative(LP_PT_3F pInOut);
/* 叉乘*/
void vector3FCrossProduct(LP_PT_3F pOut, const LP_PT_3F pIn0,  const LP_PT_3F pIn1);
/* 求两向量间的夹角 */
f_float32_t getVa2FVbAngle(const LP_PT_3F pt0, const LP_PT_3F pt1);
/*
 * 计算点或向量绕某向量旋转一定弧度的结果
 * 输入: 初始点或向量坐标, 轴向量, 旋转的弧度值
 * 返回: 旋转的结果值(输出)
 */
PT_3F vector3FRotate(const LP_PT_3F vpRotate, const LP_PT_3F vpAxis, const f_float32_t radRotate);

/* 3D向量运算 */
/* 赋值*/
void vector3DMemSet(LP_PT_3D pt, const f_float64_t x, const f_float64_t y, const f_float64_t z); 
void vector3DSetValue(LP_PT_3D pLeft, LP_PT_3D pRight);
/* 加法*/
void vector3DAdd(LP_PT_3D pOut, const LP_PT_3D pt0, const LP_PT_3D pt1);
/* 减法*/
void vector3DSub(LP_PT_3D pOut, const LP_PT_3D pt0, const LP_PT_3D pt1);
/* 乘法*/
void vector3DMul(LP_PT_3D pOut, const LP_PT_3D pIn, const f_float64_t lf);
/* 除法*/
void vector3DDiv(LP_PT_3D pOut, const LP_PT_3D pIn, const f_float64_t lf);
/* 长度*/
f_float64_t vector3DLength(const LP_PT_3D pt);
/* 单位化*/
void vector3DNormalize(LP_PT_3D pInOut);
/* 点乘*/
f_float64_t vector3DDotProduct(const LP_PT_3D pt0, const LP_PT_3D pt1);
/* 负号*/
void vector3DNegative(LP_PT_3D pInOut);
/* 叉乘*/
void vector3DCrossProduct(LP_PT_3D pOut, const LP_PT_3D pIn0,  const LP_PT_3D pIn1);
/* 求两向量间的夹角 */
f_float64_t getVa2DVbAngle(const LP_PT_3D pt0, const LP_PT_3D pt1);

/*
 * 计算点或向量绕某向量旋转一定弧度的结果
 * 输入: 初始点或向量坐标, 轴向量, 旋转的弧度值
 * 返回: 旋转的结果值(输出)
 * 说明：向量旋转法1,与vector3DRotateM2结果一致
 */
PT_3D vector3DRotate(const LP_PT_3D vpRotate, const LP_PT_3D vpAxis, const f_float64_t radRotate);

/*
功能：点或向量rotated_pt绕着旋转轴rotate_axis旋转一个角度angle_rad
输入：rotated_pt   点或向量
      rotate_axis  旋转轴
      angle_rad    旋转角(弧度)
输出：旋转后的点或向量
说明：向量旋转法2, 与vector3DRotate的结果一致
*/
PT_3D vector3DRotateM2(const PT_3D rotated_pt, const PT_3D rotate_axis, const f_float64_t angle_rad);

/* 4D向量运算 */
/* 赋值*/
void vector4DMemSet(LP_PT_4D pt, const f_float64_t x, const f_float64_t y, const f_float64_t z, const f_float64_t w);
void vector4DSetValue(LP_PT_4D pLeft, LP_PT_4D pRight);

double DistanceTwoPoints(const LP_PT_3D pPt1, const LP_PT_3D pPt2);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 