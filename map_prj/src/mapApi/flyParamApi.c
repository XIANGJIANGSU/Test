#include "flyParamApi.h"
#include "common.h"
#include "../mapApp/mapApp.h"
#include "../mapApp/flyParam.h"

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
f_int32_t setPeriodParamBegin(VOIDPtr punit)
{
    sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setPeriodParamPre( (VOIDPtr)(pUnit->pScene)) );
}

/**
 * @fn f_int32_t setPeriodParamEnd(VOIDPtr render_scene)
 *  @brief 设置周期参数的后置函数. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setPeriodParamEnd(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setPeriodParamPro( (VOIDPtr)(pUnit->pScene)) );
}

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
void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if( (longitude < -180.0) || (longitude > 180.0) || (latitude < -90.0) || (latitude > 90.0) )
	    return;
    setPosLonLat((VOIDPtr)(pUnit->pScene), longitude, latitude);	
}

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
void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude)
{
    sPAINTUNIT *pUnit = NULL;
    pUnit = unitPtrValid(punit);
    if(NULL == pUnit)
        return;
    if( (longitude == NULL) || (latitude == NULL) )
        return;
    getPosLonLat((VOIDPtr)(pUnit->pScene), longitude, latitude);
}

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
void setPositionHeight(VOIDPtr punit, f_float64_t height)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setPosHeight((VOIDPtr)(pUnit->pScene), height);	
}

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
void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if((yaw < -180.0) || (yaw > 180.0) )
	    return;
	    
	//ICD输入的角度-180～180，正北方为0，逆时针为负，顺时针为正;
	//opengl的角度0～360，正北方为0，逆时针方向增加;
	//将ICD输入的航向角转换成opengl的角度。
	yaw = 360.0 - yaw;
    setAtdYaw((VOIDPtr)(pUnit->pScene), yaw);
}

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
void setAttitudeMag(VOIDPtr punit, f_float64_t mag)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if((mag < -180.0) || (mag > 180.0) )
	    return;
	
	//ICD输入的角度-180～180，正北方为0，逆时针为负，顺时针为正;
	//opengl的角度0～360，正北方为0，逆时针方向增加;
	//将ICD输入的磁差角转换成opengl的角度。
	mag = 360.0 - mag;

    setAtdMag((VOIDPtr)(pUnit->pScene), mag);
}

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
void setAttitudePitch(VOIDPtr punit, f_float64_t pitch)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    if((pitch < -90.0) || (pitch > 90.0) )
	    return;	    
    setAtdPithch((VOIDPtr)(pUnit->pScene), pitch);
}

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
void setAttitudeRoll(VOIDPtr punit, f_float64_t roll)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if((roll < -180.0) || (roll > 180.0) )
	    return;
    setAtdRoll((VOIDPtr)(pUnit->pScene), roll);
}

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
void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setRomxy((VOIDPtr)(pUnit->pScene), romx, romy, romflag);
}
