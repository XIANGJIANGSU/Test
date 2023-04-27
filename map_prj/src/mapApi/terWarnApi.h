/**
 * @file terWarnApi.h
 * @brief 该文件提供地图引擎地形告警参数（高度、颜色）设置的外部接口函数
 * @author 615地图团队
 * @date 2016-06-20
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _TERWARN_API_h_ 
#define _TERWARN_API_h_ 

#include "common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setTerWarningBegin(VOIDPtr punit, f_int32_t warning_layers)
 *  @brief 设置地形告警参数的前置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] warning_layers 地形告警级数.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setTerWarningBegin(VOIDPtr punit, f_int32_t warning_layers);

/**
 * @fn f_int32_t setTerWarningEnd(VOIDPtr punit)
 *  @brief 设置事件参数的后置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setTerWarningEnd(VOIDPtr punit);

/**
 * @fn void setTerSafe(VOIDPtr punit, f_int32_t index, f_float32_t height, \n
                       f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief 设置安全高度，当地形低于安全高度时，用red、green、blue来渲染. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 层级索引.
 *  @param[in] height 高度，地形高度在 飞机高度+height 以下时用red、green、blue来渲染.
 *  @param[in] red    红色分量.
 *  @param[in] green  绿色分量.
 *  @param[in] blue   蓝色分量.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setTerSafe(VOIDPtr punit, f_int32_t index, f_float32_t height, 
                f_int32_t red, f_int32_t green, f_int32_t blue); 

/**
 * @fn void setTerDangerous(VOIDPtr punit, f_int32_t index, f_float32_t height, \n
                            f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief 设置危险高度，当地形高于危险高度时，用red、green、blue来渲染. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 层级索引.
 *  @param[in] height 高度，地形高度高于 飞机高度+height 时用red、green、blue来渲染.
 *  @param[in] red    红色分量.
 *  @param[in] green  绿色分量.
 *  @param[in] blue   蓝色分量.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setTerDangerous(VOIDPtr punit, f_int32_t index, f_float32_t height,
                     f_int32_t red, f_int32_t green, f_int32_t blue);

/**
 * @fn void setTerWarning(VOIDPtr punit, f_int32_t index, f_float32_t height_min, f_float32_t height_max, \n
                          f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief 设置地形告警参数，当地形在(飞机高度+height_min)～(飞机高度+height_max)区间时，用red、green、blue来渲染. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 层级索引.
 *  @param[in] height_min 高度低值.
 *  @param[in] height_max 高度高值.
 *  @param[in] red    红色分量.
 *  @param[in] green  绿色分量.
 *  @param[in] blue   蓝色分量.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setTerWarning(VOIDPtr punit, f_int32_t index, f_float32_t height_min, f_float32_t height_max,
                   f_int32_t red, f_int32_t green, f_int32_t blue);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 