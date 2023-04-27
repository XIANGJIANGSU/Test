#include "terWarnApi.h"
#include "../mapApp/mapApp.h"
#include "../mapApp/terWarn.h"

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
f_int32_t setTerWarningBegin(VOIDPtr punit, f_int32_t warning_layers)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setTerWarningPre( (VOIDPtr)(pUnit->pScene) , warning_layers) );
}

/**
 * @fn f_int32_t setTerWarningEnd(VOIDPtr punit)
 *  @brief 设置事件参数的后置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setTerWarningEnd(VOIDPtr punit)
{
    sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setTerWarningPro( (VOIDPtr)(pUnit->pScene)) );	
}

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
                f_int32_t red, f_int32_t green, f_int32_t blue)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	terSafeSet((VOIDPtr)(pUnit->pScene), index, height, red, green, blue);
}

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
                     f_int32_t red, f_int32_t green, f_int32_t blue)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	terDangerousSet((VOIDPtr)(pUnit->pScene), index, height, red, green, blue);
}

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
                   f_int32_t red, f_int32_t green, f_int32_t blue)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	terWarningSet((VOIDPtr)(pUnit->pScene), index, height_min, height_max, red, green, blue);
}

