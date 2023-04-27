#include "mapApi.h"
#include "common.h"
#include "../mapApp/mapApp.h"

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
f_int32_t setViewWindowParamBegin(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setViewWindowParamPre( (VOIDPtr)(pUnit->pViewWindow)) );
}

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
f_int32_t setViewWindowParamEnd(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setViewWindowParamPro( (VOIDPtr)(pUnit->pViewWindow)) );
}

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
f_int32_t setViewWndSize(VOIDPtr punit, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
{
	f_int32_t ret = -1;
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(ret);
	ret = setViewWindowSize((VOIDPtr)(pUnit->pViewWindow), (VOIDPtr)(pUnit->pScene), llx, lly, width, height);
	return(ret);
}

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
void setViewWndAlpha(VOIDPtr punit, f_float32_t alpha)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setViewWindowAlpha((VOIDPtr)(pUnit->pViewWindow), alpha);
}

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
void setViewWndBkColor(VOIDPtr punit, sColor3f bkcolor)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setViewWindowBkcolor((VOIDPtr)(pUnit->pViewWindow), bkcolor);
}

/**
 * @fn void map2dRender(VOIDPtr scene)
 *  @brief 2D地图绘制.
 *  @param[in] pscene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map2dRender(VOIDPtr pscene)
{
	render2dMap(pscene);
}

/**
 * @fn void map3dRender(VOIDPtr scene)
 *  @brief 3D地图绘制.
 *  @param[in] pscene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map3dRender(VOIDPtr pscene)
{
	render3dMap(pscene);
}

/**
 * @fn void mapRender(VOIDPtr punit)
 *  @brief 地图场景渲染.
 *  @param[in] punit  绘制单元句柄.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void mapRender(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = (sPAINTUNIT *)punit;
	if(NULL == pUnit)
	{
	    DEBUG_PRINT("NULL == punit.");
		return;
	}
	mapSceneRender((VOIDPtr)(pUnit->pViewWindow), (VOIDPtr)(pUnit->pScene));
}

