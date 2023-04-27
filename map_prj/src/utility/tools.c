#include <math.h>
#include <stdio.h>
#include "tools.h"

#include "../define/macrodefine.h" 
#include "../geometry/vectorMethods.h"
#include "../projection/coord.h"
#include "../engine/osAdapter.h"

/*
功能：根据当前点的经纬度及给定的距离、方向,计算大圆上的位置
输入：lon0, lat0        当前点的经纬度(单位:度)
      distance          距离(单位:米)
      course            方向(单位:度)
输出：lon1, lat1        计算点的经纬度(单位:度)      
*/
void caculateLonLatOnGreateCircle(f_float64_t lon0, f_float64_t lat0, f_float64_t distance, f_float64_t course, 
                                  f_float64_t *lon1, f_float64_t *lat1)
{
	f_float64_t sin_lat0 = 0.0, cos_lat0 = 0.0, sin_d = 0.0, cos_d = 0.0, sin_tc = 0.0, cos_tc = 0.0, dlon = 0.0, dis_radians = 0.0;
	f_float64_t lon = 0.0, lat = 0.0, sin_lat = 0.0;

	dis_radians = distance / EARTH_RADIUS;
	sin_d = sin(dis_radians);
	cos_d = cos(dis_radians);
	lon0 *= DE2RA;
	lat0 *= DE2RA;
	sin_lat0 = sin(lat0);
	cos_lat0 = cos(lat0);
	course *= -DE2RA;
	sin_tc = sin(course);
	cos_tc = cos(course);	
	
	lat  = asin(sin_lat0 * cos_d + cos_lat0 * sin_d * cos_tc);
	sin_lat = sin(lat);
    dlon = atan2(sin_tc * sin_d * cos_lat0, cos_d - sin_lat0 * sin_lat);
    lon  = lon0 - dlon;
    
    *lon1 = lon * RA2DE;
    *lat1 = lat * RA2DE;
}

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
BOOL caculateLatitudeOnGreatCircle(f_float64_t lon, f_float64_t *lat, f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t cos_lat1 = 0.0, cos_lat2 = 0.0, sin_lon12 = 0.0, sin_lat1 = 0.0, 
		        sin_lat2 = 0.0, sin_lon01 = 0.0, sin_lon02 = 0.0;
	lon  *= DE2RA;
	lon1 *= DE2RA;
	lat1 *= DE2RA;
	lon2 *= DE2RA;
	lat2 *= DE2RA;
	cos_lat1 = cos(lat1);
	if(fabs(cos_lat1) < FLT_EPSILON) /* 极点 */
		return(FALSE);
	cos_lat2 = cos(lat2);
	if(fabs(cos_lat2) < FLT_EPSILON) /* 极点 */
		return(FALSE);
	sin_lon12 = sin(lon1 - lon2);
	if(fabs(sin_lon12) < FLT_EPSILON) /* 经线 */
		return(FALSE);

	sin_lat1 = sin(lat1);	
	sin_lat2 = sin(lat2);
	sin_lon01 = sin(lon - lon1);
	sin_lon02 = sin(lon - lon2);
	
    *lat = atan((sin_lat1 * cos_lat2 * sin_lon02 - sin_lat2 * cos_lat1 * sin_lon01) / (cos_lat1 * cos_lat2 * sin_lon12)) * RA2DE;
	return(TRUE);
}

/*
功能：根据输入两点的经纬度，计算两点间的大圆航迹角
输入：
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      夹角（单位PI弧度）
*/
f_float64_t caculateGreatCircleCourse(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t course = 0.0, sin_lat1 = 0.0, cos_lat1 = 0.0, sin_lat2 = 0.0, cos_lat2 = 0.0;
    f_float64_t sin_lonsub = 0.0, cos_latsub = 0.0;
    
    sin_lat1 = sin(lat1 * DE2RA);
    cos_lat1 = cos(lat1 * DE2RA);
    sin_lat2 = sin(lat2 * DE2RA);
    
    if(fabs(cos_lat1) < FLT_EPSILON)  /* 从极点开始 */
    {
        if(lat1 > 0.0)	/* 北极 */
            course = PI;
        else  /* 南极 */
            course = 2.0 * PI;
    }
    else  /* 开始点不在极点 */
    {
        lon1 *= DE2RA;
        lat1 *= DE2RA; 
        lon2 *= DE2RA;
        lat2 *= DE2RA;
        sin_lat1 = sin(lat1);
        cos_lat1 = cos(lat1);
        sin_lat2 = sin(lat2);
        cos_lat2 = cos(lat2);
        sin_lonsub = sin(lon1-lon2);
        cos_latsub = cos(lon1-lon2);
        course = -atan2( sin_lonsub * cos_lat2, cos_lat1 * sin_lat2 - sin_lat1 * cos_lat2 * cos_latsub);
    }  
    return(course);  
}

/*
功能：根据输入两点的经纬度，计算两点间的大圆夹角
输入：
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      夹角（单位PI弧度）
*/
f_float64_t caculateGreatCircleAngle(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t angle = 0.0;
    lon1 *= DE2RA;
    lat1 *= DE2RA; 
    lon2 *= DE2RA;
    lat2 *= DE2RA;
    angle = acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1 - lon2));
	return(angle);
}

/*
功能：根据输入两点的经纬度，计算大圆距离
输入：
      lon1, lat1 点1的经纬度（单位度）
      lon2, lat2 点2的经纬度（单位度）
输出：
      两点间的大圆距离（单位米）     
*/
f_float64_t caculateGreatCircleDis(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t dis = 0.0;
    dis = caculateGreatCircleAngle(lon1, lat1, lon2, lat2) * EARTH_RADIUS;
    return(dis);
}

// 检测执行是否达到某一时间(s)
BOOL CheckTriggerTime(float* lastTick, float delayTime)
{
    float curTickCnt = tickCountGet();
    float spendTime = 0.0f;

    // 首测检测总是通过
    if(*lastTick <= FLT_EPSILON)
    {
        *lastTick = tickCountGet();
        return TRUE;
    }

    // 计算时间差(s)
    spendTime = 1.0f * (curTickCnt - *lastTick)/(1.0f * getSysClkRate());
    
    // 超时,通过测试
    if(spendTime >= delayTime)
    {
        *lastTick = tickCountGet();
        return TRUE;
    }

    return FALSE;
}