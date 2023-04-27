/**
 * @file flyCmd.h
 * @brief 该文件提供地图飞行控制命令设置的接口函数
 * @author 615地图团队
 * @date 2016-05-11
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _FLYCMD_h_ 
#define _FLYCMD_h_ 

#include "../define/mbaseType.h"
#include "appHead.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setEventParamPre(VOIDPtr render_scene)
 *  @brief 设置事件参数的前置函数，获取参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setEventParamPre(VOIDPtr render_scene);

/**
 * @fn f_int32_t setEventParamPro(VOIDPtr render_scene)
 *  @brief 设置事件参数的后置函数，释放参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setEventParamPro(VOIDPtr render_scene);

/**
 * @fn setEventParam(sGLRENDERSCENE *pScene)
 *  @brief 设置事件参数，主要是控制命令. 
 *  @param[in] pScene 场景句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证飞行参数的一致性，需要用信号量来进行保护.
*/
void setEventParam(sGLRENDERSCENE *pScene);

/**
 * @fn void setModeCor(VOIDPtr render_scene, f_int32_t cormode)
 *  @brief 设置颜色模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] cormode 颜色模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeCor(VOIDPtr render_scene, f_int32_t cormode);

/**
 * @fn void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
 *  @brief 设置昼夜模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] dnmode 昼夜模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDN(VOIDPtr render_scene, f_int32_t dnmode);

/**
 * @fn void setModeSVS(VOIDPtr render_scene, f_int32_t svsmode)
 *  @brief 设置SVS 模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] svsmode SVS 模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSVS(VOIDPtr render_scene, f_int32_t svsmode);

/**
 * @fn setMode3DMD(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief 设置是否显示3D模型
 *  @param[in] render_scene 场景句柄.
 *  @param[in] isdisplay 是否显示，0:不显示，1:显示
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode3DMD(VOIDPtr render_scene, f_int32_t isdisplay);

/**
 * @fn void setModeMv(VOIDPtr render_scene, f_int32_t mvmode)
 *  @brief 设置运动模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] mvmode 运动模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMv(VOIDPtr render_scene, f_int32_t mvmode);

/**
 * @fn void setModePSDisplay(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief 设置是否显示飞机符号.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] isdisplay 是否显示，0:不显示，1:显示.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModePSDisplay(VOIDPtr render_scene, f_int32_t isdisplay);

/**
 * @fn void setModeFrz(VOIDPtr render_scene, f_int32_t frzmode)
 *  @brief 设置冻结模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] frzmode 冻结模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeFrz(VOIDPtr render_scene, f_int32_t frzmode);

/**
 * @fn void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
 *  @brief 设置漫游模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] rommode 漫游模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeRm(VOIDPtr render_scene, f_int32_t rommode);

/**
 * @fn void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
 *  @brief 设置漫游模式.俯视下
 *  @param[in] render_scene 场景句柄.
 *  @param[in] rommode 漫游模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode2dRm(VOIDPtr render_scene, f_int32_t rommode);

/**
 * @fn void setFlagRm(VOIDPtr render_scene, f_int32_t romflag)
 *  @brief 设置漫游标志.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] romflag 漫游标志.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setFlagRm(VOIDPtr render_scene, f_int32_t romflag);

/**
 * @fn void setFlagRmHm(VOIDPtr render_scene, f_int32_t romhomeflag)
 *  @brief 设置漫游归位标志.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] romflag 漫游归位标志.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setFlagRmHm(VOIDPtr render_scene, f_int32_t romhomeflag);

/**
 * @fn void setModeTwarn(VOIDPtr render_scene, f_int32_t terwarnmode)
 *  @brief 设置地形告警模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] terwarnmode 地形告警模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeTwarn(VOIDPtr render_scene, f_int32_t terwarnmode);

/**
 * @fn void setModeSce(VOIDPtr render_scene, f_int32_t scenemode)
 *  @brief 设置设置场景显示模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] scenemode 场景显示模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSce(VOIDPtr render_scene, f_int32_t scenemode);

/**
 * @fn void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
 *  @brief 设置二三维模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] dnmode 二三维模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode23D(VOIDPtr render_scene, f_int32_t mode23d);
/**
 * @fn void setModeDisplay(VOIDPtr render_scene, f_int32_t modedisplay)
 *  @brief 设置地图的图层叠加方式
 *  @param[in] render_scene 场景句柄.
 *  @param[in] modedisplay 图层叠加模式.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDisplay(VOIDPtr render_scene, f_int32_t modedisplay);

/**
 * @fn void setModeClose3d(VOIDPtr render_scene, f_int32_t modedisplay)
 *  @brief 设置三维地图是否屏蔽
 *  @param[in] render_scene 场景句柄.
 *  @param[in] modedisplay 0 : 不屏蔽，1 :屏蔽
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeClose3d(VOIDPtr render_scene, f_int32_t modedisplay);

/**
 * @fn void setCamCtlParamViewRes(VOIDPtr render_scene, f_float32_t viewres)
 *  @brief 设置相机控制参数中的显示分辨率.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] viewres 分辨率（米/像素）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewRes(VOIDPtr render_scene, f_float32_t viewres);

/**
 * @fn void setCamCtlParamViewAngle(VOIDPtr render_scene, f_float32_t viewangle)
 *  @brief 设置设置相机控制参数中的平截头视角.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] viewangle 视角（单位：度）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewAngle(VOIDPtr render_scene, f_float32_t viewangle);

/**
 * @fn void setCamCtlParamCmpRd(VOIDPtr render_scene, f_int32_t radius)
 *  @brief 设置相机控制参数的刻度环半径.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] radius 刻度环半径（像素）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamCmpRd(VOIDPtr render_scene, f_int32_t radius);

/**
 * @fn void setCamCtlParamRtCent(VOIDPtr render_scene, f_int32_t xcenter, f_int32_t ycenter)
 *  @brief 设置相机控制参数中的地图旋转中心.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] xcenter、ycenter 地图旋转中心的位置（像素，相对于外部视窗左下角）.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamRtCent(VOIDPtr render_scene, f_int32_t xcenter, f_int32_t ycenter);

/**
 * @fn void setCamCtlParamViewType(VOIDPtr render_scene, f_int32_t vtype)
 *  @brief 设置视角模式.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] vtype 视角类型.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewType(VOIDPtr render_scene, f_int32_t vtype);


/**
 * @fn void setColorAndLayermaskCfgIndex(VOIDPtr render_scene, f_int32_t index)
 *  @brief 设置选用的颜色和防拥配置文件索引号
 *  @param[in] render_scene 场景句柄.
 *  @param[in] index：0-7，最多支持8种
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setColorAndLayermaskCfgIndex(VOIDPtr render_scene, f_int32_t index);


/**
 * @fn void setAreaBaDraw(VOIDPtr render_scene, f_int32_t bDraw)
 *  @brief 设置矢量政区面是否绘制
 *  @param[in] render_scene 场景句柄.
 *  @param[in] bDraw 0-不绘制 1-绘制
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAreaBaDraw(VOIDPtr render_scene, f_int32_t bDraw);

/**
 * @fn void setSeaMapDraw(VOIDPtr render_scene, f_int32_t bDraw)
 *  @brief 设置海图是否绘制
 *  @param[in] render_scene 场景句柄.
 *  @param[in] bDraw 0-不绘制 1-绘制
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setSeaMapDraw(VOIDPtr render_scene, f_int32_t bDraw);


/**
 * @fn void setTextDrawLevel(VOIDPtr render_scene, f_int32_t level)
 *  @brief 设置地名注记和符号显示档位
 *  @param[in] render_scene 场景句柄.
 *  @param[in] level 0-不显示任何注记和符号 1-只显示省会城市和中等城市的地名注记和符号 2-正常显示所有的注记和符号
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setTextDrawLevel(VOIDPtr render_scene, f_int32_t level);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
