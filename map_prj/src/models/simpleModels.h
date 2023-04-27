/**
 * @file simpleModels.h
 * @brief 该文件定义了简单几何体的绘制方法
 * @author 615地图团队
 * @date 2017-01-06
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef simpleModels 
#define simpleModels

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
功能：立方体绘制
输入：width, height, length    立方体的长×宽×高(单位:米)
输出：无
*/
void avicCube(f_float64_t width, f_float64_t height, f_float64_t length);

/*
功能：球体绘制
输入：radius    球体的半径(单位:米)
      slices    球体的瓣数
      stacks    球体的层数
输出：无
*/
void avicSphere(f_float64_t radius, f_int32_t slices, f_int32_t stacks);

/*
功能：柱体绘制
输入：baseRadius    柱体的底面半径(单位:米)
      topRadius     柱体的顶面半径(单位:米)
      height        柱体的高度(单位:米)
      slices        柱体的瓣数
      stacks        柱体的层数
      needDrawTop   顶面是否绘制的标志
输出：无
*/
void avicCylinder(f_float64_t baseRadius, f_float64_t topRadius, f_float64_t height, f_int32_t slices, f_int32_t stacks, BOOL needDrawTop);

/*
功能：盘体部分的绘制
输入：innerRadius    盘体的内半径(单位:米)
      outerRadius    盘体的外半径(单位:米)
      slices         盘体的瓣数
      loops          盘体的层数
      startAngle     盘体的起始角度(单位:度)
      sweepAngle     角度跨度(单位:度)
输出：无
*/
void avicPartialDisk(f_float64_t innerRadius, f_float64_t outerRadius, 
                     f_int32_t slices, f_int32_t loops, f_float64_t startAngle, f_float64_t sweepAngle);
                     
/*
功能：圆盘的绘制
输入：innerRadius    圆盘的内半径(单位:米)
      outerRadius    圆盘的外半径(单位:米)
      slices         圆盘的瓣数
      loops          圆盘的层数
输出：无
*/
void avicDisk(f_float64_t innerRadius, f_float64_t outerRadius, f_int32_t slices, f_int32_t loops);     

/*
功能：金字塔绘制
输入：edgeLength  金字塔边长
输出：无
*/
void avicPyramid(f_float64_t edgeLength);                

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 