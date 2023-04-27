#ifndef _HlTools 
#define _HlTools

#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
功能：根据当前点的经纬度及给定的距离、方向,计算大圆上的位置
输入：lon0, lat0        当前点的经纬度(单位:度)
      distance          距离(单位:米)
      course            方向(单位:度)
输出：lon1, lat1        计算点的经纬度(单位:度)      
*/
void caculateLonLatOnGreateCircle(f_float64_t lon0, f_float64_t lat0, f_float64_t distance, f_float64_t course, 
                                  f_float64_t *lon1, f_float64_t *lat1);

/*
功能：根据的经度，计算两点间大圆上的纬度
输入：
      lon        需要计算纬度所对应的经度（单位度）
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      纬度（单位度）
	  计算成功标志, TRUE 成功,FALSE 失败
说明：若cos(lat1) == 0,则(lon1,lat1)过极点,导致大圆肯定过南北两个极点；
      同样若cos(lat2) == 0,则(lon2,lat2)过极点,导致大圆肯定过南北两个极点；
	  若lon1 == lon2,则大圆也过南北两个极点；
	  这三种情况都不能根据经度来计算大圆上的纬度。
*/
BOOL caculateLatitudeOnGreatCircle(f_float64_t lon, f_float64_t *lat, f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);

/*
功能：根据输入两点的经纬度，计算两点间的大圆航迹角
输入：
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      夹角（单位PI弧度）
*/
f_float64_t caculateGreatCircleCourse(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);

/*
功能：根据输入两点的经纬度，计算大圆距离
输入：
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      两点间的大圆距离（单位米）     
*/
f_float64_t caculateGreatCircleDis(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);

/*
功能：根据输入两点的经纬度，计算两点间的大圆夹角
输入：
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      夹角（单位PI弧度）
*/
f_float64_t caculateGreatCircleAngle(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
