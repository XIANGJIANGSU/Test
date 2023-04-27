/**
 * @file flyParam.h
 * @brief 该文件提供地图飞行参数设置的接口函数
 * @author 615地图团队
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _FLY_PARAM_h_ 
#define _FLY_PARAM_h_ 

#include "../define/mbaseType.h"
#include "appHead.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setPeriodParamPre(VOIDPtr render_scene)
 *  @brief 设置周期参数的前置函数，获取参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setPeriodParamPre(VOIDPtr render_scene);

/**
 * @fn f_int32_t setPeriodParamPro(VOIDPtr render_scene)
 *  @brief 设置周期参数的后置函数，释放参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setPeriodParamPro(VOIDPtr render_scene);

/**
 * @fn setPeriodParam(sGLRENDERSCENE *pScene)
 *  @brief 设置周期参数，主要是飞行参数. 
 *  @param[in] pScene 场景句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证飞行参数的一致性，需要用信号量来进行保护.
*/
void setPeriodParam(sGLRENDERSCENE *pScene);

/**
 * @fn void setPosLonLat(VOIDPtr render_scene, f_float64_t longitude, f_float64_t latitude)
 *  @brief 设定位置信息的经纬度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] longitude   经度（单位：度）.
 *  @param[in] latitude    纬度（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setPosLonLat(VOIDPtr render_scene, f_float64_t longitude, f_float64_t latitude);


/**
 * @fn void getPosLonLat(VOIDPtr render_scene, f_float64_t* longitude, f_float64_t* latitude)
 *  @brief 获取引擎内部位置信息的经纬度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
 *  @param[in] render_scene 场景句柄.
 *  @param[out] longitude   经度（单位：度）.
 *  @param[out] latitude    纬度（单位：度）.
 *  @exception void.
 *  @return void.
 * @see
 * @note
*/
void getPosLonLat(VOIDPtr render_scene, f_float64_t* longitude, f_float64_t* latitude);

/**
 * @fn void setPosHeight(VOIDPtr render_scene, f_float64_t height)
 *  @brief 设定位置信息的高度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
           -# 在俯视情况下，视点的高度是根据量程反算过来的. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] height   高度（单位：米）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setPosHeight(VOIDPtr render_scene, f_float64_t height);

/**
 * @fn void setAtdYaw(VOIDPtr render_scene, f_float64_t yaw)
 *  @brief 设定航姿信息的航向角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] yaw   航向角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdYaw(VOIDPtr render_scene, f_float64_t yaw);


/**
 * @fn void setAtdMag(VOIDPtr render_scene, f_float64_t mag)
 *  @brief 设定航姿信息的磁差角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] mag  磁差角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdMag(VOIDPtr render_scene, f_float64_t mag);

/**
 * @fn void setAtdPithch(VOIDPtr render_scene, f_float64_t pitch)
 *  @brief 设定航姿信息的俯仰角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] pitch   俯仰角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdPithch(VOIDPtr render_scene, f_float64_t pitch);

/**
 * @fn void setAtdRoll(VOIDPtr render_scene, f_float64_t roll)
 *  @brief 设定航姿信息的横滚角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] roll   横滚角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdRoll(VOIDPtr render_scene, f_float64_t roll);

/**
 * @fn void setRomxy(VOIDPtr render_scene, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
 *  @brief 设定漫游的视窗坐标(相对于视窗左下角).
 *  @param[in] render_scene 场景句柄.
 *  @param[in] romx,romy   漫游的视窗坐标.
 *  @param[out] romflag 漫游标志.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setRomxy(VOIDPtr render_scene, f_int32_t romx, f_int32_t romy, f_int32_t *romflag);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
