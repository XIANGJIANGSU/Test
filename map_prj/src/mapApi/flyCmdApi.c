#include "flyCmdApi.h"
#include "common.h"
#include "../mapApp/mapApp.h"
#include "../mapApp/flyCmd.h"

/**
 * @fn void setEventParamBegin(VOIDPtr punit)
 *  @brief 设置事件参数的前置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setEventParamBegin(VOIDPtr punit)
{
    sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setEventParamPre( (VOIDPtr)(pUnit->pScene)) );
}

/**
 * @fn f_int32_t setEventParamEnd(VOIDPtr punit)
 *  @brief 设置事件参数的后置函数. 
 *  @param[in] punit 绘制单元句柄.
 *  @exception void.
 *  @return f_int32_t.
 * @see 
 * @note
*/
f_int32_t setEventParamEnd(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setEventParamPro( (VOIDPtr)(pUnit->pScene)) );
}

/**
 * @fn void setModeColor(VOIDPtr punit, f_int32_t cormode)
 *  @brief 设置颜色模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] cormode 颜色模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeColor(VOIDPtr punit, f_int32_t cormode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeCor((VOIDPtr)(pUnit->pScene), cormode);
}

/**
 * @fn void setModeDayNight(VOIDPtr punit, f_int32_t dnmode)
 *  @brief 设置昼夜模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] dnmode 昼夜模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDayNight(VOIDPtr punit, f_int32_t dnmode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeDN((VOIDPtr)(pUnit->pScene), dnmode);
}

/**
 * @fn void setModeSVSView(VOIDPtr punit, f_int32_t svsmode)
 *  @brief 设置SVS 显示模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] svsmode SVS 显示模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSVSView(VOIDPtr punit, f_int32_t svsmode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeSVS((VOIDPtr)(pUnit->pScene), svsmode);
}

/**
 * @fn void setMode3DModelDisplay(VOIDPtr punit, f_int32_t isdisplay)
 *  @brief 设置3D 模型是否显示
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] isdisplay 是否显示，0:不显示，1:显示
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode3DModelDisplay(VOIDPtr punit, f_int32_t isdisplay)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setMode3DMD((VOIDPtr)(pUnit->pScene), isdisplay);
}

/**
 * @fn void setModeMove(VOIDPtr punit, f_int32_t mvmode)
 *  @brief 设置运动模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] mvmode 运动模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMove(VOIDPtr punit, f_int32_t mvmode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeMv((VOIDPtr)(pUnit->pScene), mvmode);
}

/**
 * @fn void setModePlaneSymbolDisplay(VOIDPtr punit, f_int32_t isdisplay)
 *  @brief 设置是否显示飞机符号.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] isdisplay 是否显示，0:不显示，1:显示.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModePlaneSymbolDisplay(VOIDPtr punit, f_int32_t isdisplay)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModePSDisplay((VOIDPtr)(pUnit->pScene), isdisplay);
}

/**
 * @fn void setModeFreeze(VOIDPtr punit, f_int32_t frzmode)
 *  @brief 设置冻结模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] frzmode 冻结模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeFreeze(VOIDPtr punit, f_int32_t frzmode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeFrz((VOIDPtr)(pUnit->pScene), frzmode);
}

/**
 * @fn void setModeRoma(VOIDPtr punit, f_int32_t rommode)
 *  @brief 设置漫游模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] rommode 漫游模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeRoma(VOIDPtr punit, f_int32_t rommode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeRm((VOIDPtr)(pUnit->pScene), rommode);
}

/**
 * @fn void setMode2dRoma(VOIDPtr punit, f_int32_t rommode)
 *  @brief 设置漫游模式.俯视下
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] rommode 漫游模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode2dRoma(VOIDPtr punit, f_int32_t rommode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setMode2dRm((VOIDPtr)(pUnit->pScene), rommode);
}


/**
 * @fn void setRomFlag(VOIDPtr punit, f_int32_t romflag)
 *  @brief 设置漫游标志.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] romflag 漫游标志.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setRomFlag(VOIDPtr punit, f_int32_t romflag)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setFlagRm((VOIDPtr)(pUnit->pScene), romflag);
}

/**
 * @fn void setRomHomeFlag(VOIDPtr punit, f_int32_t romhomeflag)
 *  @brief 设置漫游归位标志.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] romflag 漫游归位标志.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setRomHomeFlag(VOIDPtr punit, f_int32_t romhomeflag)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setFlagRmHm((VOIDPtr)(pUnit->pScene), romhomeflag);
}


/**
 * @fn void setModeTerwarn(VOIDPtr punit, f_int32_t terwarnmode)
 *  @brief 设置地形告警模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] rommode 地形告警模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeTerwarn(VOIDPtr punit, f_int32_t terwarnmode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeTwarn((VOIDPtr)(pUnit->pScene), terwarnmode);
}

/**
 * @fn void setModeScene(VOIDPtr punit, f_int32_t scenemode)
 *  @brief 设置场景显示模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] scenemode 场景显示模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeScene(VOIDPtr punit, f_int32_t scenemode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeSce((VOIDPtr)(pUnit->pScene), scenemode);
}

/**
 * @fn void setModeMap23D(VOIDPtr punit, f_int32_t scenemode)
 *  @brief 设置二三维显示模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] scenemode 二三维显示模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMap23D(VOIDPtr punit, f_int32_t scenemode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setMode23D((VOIDPtr)(pUnit->pScene), scenemode);
}

/**
 * @fn void setModeMapDisplay(VOIDPtr punit, f_int32_t scenemode)
 *  @brief 设置地图图层叠加模式
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] scenemode 图层叠加模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMapDisplay(VOIDPtr punit, f_int32_t scenemode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeDisplay((VOIDPtr)(pUnit->pScene), scenemode);
}

/**
 * @fn void setModeMapColse3d(VOIDPtr punit, f_int32_t scenemode)
 *  @brief 设置三维地图是否屏蔽
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] scenemode 0 :不屏蔽, 1: 屏蔽
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMapColse3d(VOIDPtr punit, f_int32_t scenemode)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setModeClose3d((VOIDPtr)(pUnit->pScene), scenemode);
}

/**
 * @fn void setPaintSceneViewRes(VOIDPtr punit, f_float32_t viewres)
 *  @brief 设置场景所对应相机控制参数中的显示分辨率.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] viewres 分辨率（米/像素）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneViewRes(VOIDPtr punit, f_float32_t viewres)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setCamCtlParamViewRes((VOIDPtr)(pUnit->pScene), viewres);
}

/**
 * @fn void setPaintSceneViewAngle(VOIDPtr punit, f_float32_t viewangle)
 *  @brief 设置场景所对应相机控制参数中的平截头视角.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] viewangle 角度（单位：度）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneViewAngle(VOIDPtr punit, f_float32_t viewangle)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setCamCtlParamViewAngle((VOIDPtr)(pUnit->pScene), viewangle);
}

/**
 * @fn void setPaintSceneCmpRd(VOIDPtr punit, f_int32_t compass_radius)
 *  @brief 设置场景所对应相机控制参数中的刻度环半径.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] compass_radius 刻度环半径（像素）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneCmpRd(VOIDPtr punit, f_int32_t compass_radius)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setCamCtlParamCmpRd((VOIDPtr)(pUnit->pScene), compass_radius);
}

/**
 * @fn void setPaintSceneRotCent(VOIDPtr punit, f_int32_t xcenter, f_int32_t ycenter)
 *  @brief 设置场景所对应相机控制参数中的地图旋转中心.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] xcenter、ycenter 地图旋转中心的位置（像素，相对于外部视窗左下角）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneRotCent(VOIDPtr punit, f_int32_t xcenter, f_int32_t ycenter)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setCamCtlParamRtCent((VOIDPtr)(pUnit->pScene), xcenter, ycenter);
}

/**
 * @fn void setPaintSceneViewType(VOIDPtr punit, f_int32_t vtype)
 *  @brief 设置视角模式.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] vtype 视角类型.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneViewType(VOIDPtr punit, f_int32_t vtype)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setCamCtlParamViewType((VOIDPtr)(pUnit->pScene), vtype);
}


/**
 * @fn void setPaintColorAndLayermaskCfgIndex(VOIDPtr punit, f_int32_t index)
 *  @brief 设置选用的颜色和防拥配置文件索引号.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 配置文件索引.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintColorAndLayermaskCfgIndex(VOIDPtr punit, f_int32_t index)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setColorAndLayermaskCfgIndex((VOIDPtr)(pUnit->pScene), index);
}

/**
 * @fn void setPaintAreaBaDraw(VOIDPtr punit, f_int32_t bDraw)
 *  @brief 设置矢量政区面是否绘制.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in]  bDraw 0-不绘制 1-绘制.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintAreaBaDraw(VOIDPtr punit, f_int32_t bDraw)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setAreaBaDraw((VOIDPtr)(pUnit->pScene), bDraw);
}

/**
 * @fn void setPaintSeaMapDraw(VOIDPtr punit, f_int32_t bDraw)
 *  @brief 设置海图是否绘制.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in]  bDraw 0-不绘制 1-绘制.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSeaMapDraw(VOIDPtr punit, f_int32_t bDraw)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setSeaMapDraw((VOIDPtr)(pUnit->pScene), bDraw);
}


/**
 * @fn void setPaintTextDrawLevel(VOIDPtr punit, f_int32_t level)
 *  @brief 设置地名注记和符号显示档位.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] level 0-不显示任何注记和符号 1-只显示省会城市和中等城市的地名注记和符号 2-正常显示所有的注记和符号.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintTextDrawLevel(VOIDPtr punit, f_int32_t level)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setTextDrawLevel((VOIDPtr)(pUnit->pScene), level);
}
