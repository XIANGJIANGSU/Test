/**
 * @file mapApi.h
 * @brief 该文件提供地图引擎图形渲染的外部接口函数
 * @author 615地图团队
 * @date 2016-04-22
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _MAP_API_h_ 
#define _MAP_API_h_ 

#include "../define/mbaseType.h"
#include "common.h"
#include "../mapApp/appHead.h"
#include "../engine/mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setViewWindowParamBegin(VOIDPtr punit)
 *  @brief 设置视窗参数的前置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamBegin(VOIDPtr punit);

/**
 * @fn f_int32_t setViewWindowParamEnd(VOIDPtr punit)
 *  @brief 设置视窗参数的后置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamEnd(VOIDPtr punit);

/**
 * @fn f_int32_t setViewWndSize(VOIDPtr punit, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
 *  @brief 设置视窗尺寸.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] llx、lly视窗的左下角起始位置（单位：像素）.
 *  @param[in] width、height视窗的宽度、高度（单位：像素）.
 *  @exception ptrError 输入句柄为空
 *  @return f_int32_t,成功标志.
 *  @retval 0 成功.
 *  @retval -1 视窗句柄为空.
 *  @retval -2 起始位置不在范围内.
 *  @retval -3 宽度、高度不在范围内.
 * @see 
 * @note
*/
f_int32_t setViewWndSize(VOIDPtr punit, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height);

/**
 * @fn void setViewWndAlpha(VOIDPtr punit, f_float32_t alpha)
 *  @brief 设置视窗的透明度.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] alpha 透明度.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWndAlpha(VOIDPtr punit, f_float32_t alpha);

/**
 * @fn void setViewWndBkColor(VOIDPtr punit, sColor3f bkcolor)
 *  @brief 设置视窗的背景色.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] bkcolor 背景色.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWndBkColor(VOIDPtr punit, sColor3f bkcolor);

/**
 * @fn void map2dRender(VOIDPtr scene)
 *  @brief 2D地图绘制.
 *  @param[in] pscene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map2dRender(VOIDPtr pscene);

/**
 * @fn void map3dRender(VOIDPtr scene)
 *  @brief 3D地图绘制.
 *  @param[in] pscene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map3dRender(VOIDPtr pscene);

/**
 * @fn void mapRender(VOIDPtr punit)
 *  @brief 地图场景渲染.
 *  @param[in] punit  绘制单元句柄.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void mapRender(VOIDPtr punit);

/*
 * 功能：通过已加载好瓦片数据计算瓦片中某点对应的高度
 * 输入：lfLon 经度
 *       lfLat 纬度
 * 输出：pZ    高度
 * 返回：计算成功，返回TRUE(1)；计算失败，返回FALSE(0)
 * 注释：zyp 2015-12-30 11:56:13
 */
BOOL GetZ(sGLRENDERSCENE *pScene, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ);
//输入变量是sMAPHANDLE *
BOOL GetZex(sMAPHANDLE * pHandle, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
