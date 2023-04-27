/**
 * @file auxNavInfoApi.h
 * @brief 该文件提供辅助导航信息（罗盘、0俯仰角线等）显示参数设置的外部接口函数
 * @author 615地图团队
 * @date 2016-10-24 
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _auxNavInfoAPI_h_ 
#define _auxNavInfoAPI_h_ 

#include "common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn BOOL initAuxNavInfoData(VOIDPtr scene, f_char_t *data_path)
 *  @brief 辅助导航信息数据初始化.
 *  @param[in] scene 视口所在的场景句柄.
 *  @param[in] data_path 视口对应数据路径的根目录.
 *  @exception void
 *  @return BOOL,初始化成功标志.
 *  @retval TRUE 成功.
 *  @retval FALSE 失败.
 * @see 
 * @note 
*/
BOOL initAuxNavInfoData(VOIDPtr scene, f_char_t *data_path);

/**
 * @fn void setAuxNavZplParam(VOIDPtr punit, sZplParam zplprarm)
 *  @brief 设定0俯仰角线显示的参数.
 *  @param[in] punit  绘制单元句柄.
 *  @param[in] zplprarm 0俯仰角线参数.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAuxNavZplParam(VOIDPtr punit, sZplParam zplprarm);

/**
 * @fn void setAuxNavCpsParam(VOIDPtr punit, sCpsParam cpsprarm)
 *  @brief 设定罗盘显示的参数.
 *  @param[in] punit  绘制单元句柄.
 *  @param[in] cpsprarm 罗盘参数.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAuxNavCpsParam(VOIDPtr punit, sCpsParam cpsprarm);

/**
 * @fn void setAuxNavPrefileParam(VOIDPtr punit, sPrefileParam prefileprarm)
 *  @brief 设定剖面图显示的参数.
 *  @param[in] punit  绘制单元句柄.
 *  @param[in] prefileprarm 剖面图参数.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAuxNavPrefileParam(VOIDPtr punit, sPrefileParam prefileprarm);

/**
 * @fn void setAuxNavPrefileDisplay(VOIDPtr punit, f_int32_t isdisplay)
 *  @brief 设置剖面图是否显示
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] isdisplay 是否显示，0:不显示，1:显示
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAuxNavPrefileDisplay(VOIDPtr punit, f_int32_t isdisplay);

/**
 * @fn void setAuxNavPrefileDetectDistance(VOIDPtr punit, f_float64_t hDetectDistance)
 *  @brief 设置剖面图水平探测距离
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] hDetectDistance 水平探测距离
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAuxNavPrefileDetectDistance(VOIDPtr punit, f_float64_t hDetectDistance);

/**
 * @fn void setAuxNavPrefileTerrainColor(VOIDPtr punit, f_int32_t color_num, f_float32_t prefile_height[5], 
	f_uint8_t prefile_color[6][4], f_int32_t s_winHeiMeter)
 *  @brief 设置剖面图垂直地形剖面图VSD的垂直方向半屏代表实际距离(米)及高度色带相关参数
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] color_num 色带数量,范围为[2,6]
 *  @param[in] prefile_height[5] 色带区间的高度,每条色带对应一个区间范围,5个值对应最多6个区间
 *  @param[in] prefile_color[6][4] 色带的颜色RGBA
 *  @param[in] s_winHeiMeter 垂直方向半屏代表实际距离(米)，必须大于0
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAuxNavPrefileTerrainColor(VOIDPtr punit, f_int32_t color_num, f_float32_t prefile_height[5], 
	f_uint8_t prefile_color[6][4], f_int32_t s_winHeiMeter);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
