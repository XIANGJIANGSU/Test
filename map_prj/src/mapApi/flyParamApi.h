/**
 * @file flyParamApi.h
 * @brief 该文件提供地图引擎飞行参数设置的外部接口函数
 * @author 615地图团队
 * @date 2016-05-11
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _FLYPARAM_API_h_ 
#define _FLYPARAM_API_h_ 

#include "common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn void setPeriodParamBegin(VOIDPtr punit)
 *  @brief 设置周期参数的前置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setPeriodParamBegin(VOIDPtr punit);

/**
 * @fn f_int32_t setPeriodParamEnd(VOIDPtr render_scene)
 *  @brief 设置周期参数的后置函数. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setPeriodParamEnd(VOIDPtr punit);

/**
 * @fn void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude)
 *  @brief 设定位置信息的经纬度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] longitude   经度（单位：度，范围-180~180）.
 *  @param[in] latitude    纬度（单位：度，范围-90~90）.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude);

/**
 * @fn void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude)
 *  @brief 获取位置信息的经纬度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] longitude   经度（单位：度，范围-180~180）.
 *  @param[in] latitude    纬度（单位：度，范围-90~90）.
 *  @exception void
 *  @return void
 * @see
 * @note
*/
void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude);

/**
 * @fn void setPositionHeight(VOIDPtr punit, f_float64_t height)
 *  @brief 设定位置信息的高度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] height   高度（单位：米）.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setPositionHeight(VOIDPtr punit, f_float64_t height);

/**
 * @fn void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw)
 *  @brief 设定航姿信息的航向角.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] yaw   航向角（单位：度，范围-180~180）.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw);

/**
 * @fn void setAttitudeMag(VOIDPtr punit, f_float64_t mag)
 *  @brief 设定航姿信息的磁差角.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] mag   磁差角（单位：度，范围-180~180）.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeMag(VOIDPtr punit, f_float64_t mag);

/**
 * @fn void setAttitudePitch(VOIDPtr punit, f_float64_t pitch)
 *  @brief 设定航姿信息的俯仰角.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] pitch   俯仰角（单位：度，范围-90~90）.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudePitch(VOIDPtr punit, f_float64_t pitch);

/**
 * @fn void setAttitudeRoll(VOIDPtr punit, f_float64_t roll)
 *  @brief 设定航姿信息的横滚角.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] roll   横滚角（单位：度，范围-180~180）.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeRoll(VOIDPtr punit, f_float64_t roll);

/**
 * @fn void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
 *  @brief 设定漫游的视窗坐标(相对于视窗左下角).
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] romx,romy   漫游的视窗坐标.
 *  @param[out] romflag   漫游标志.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
