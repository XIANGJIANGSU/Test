#ifndef _INIT_SCENE_h_
#define _INIT_SCENE_h_

#include "../mapApi/common.h"
#include "../define/mbaseType.h"
#include "appHead.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn void initGLSetting(void)
 *  @brief GL环境初始化.
 *  @param[in] void. 
 *  @exception void 
 *  @return void
 * @see 
 * @note
*/
void initGLSetting(void);

/**
 * @fn f_int32_t projectMdInit(VOIDPtr scene, f_int32_t pmode)
 *  @brief 投影方式初始化.
 *  @param[in] scene 场景句柄.
 *  @param[in] pmode 投影方式.
 *  @exception void
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
 * @see 
 * @note 
*/
f_int32_t projectMdInit(VOIDPtr scene, f_int32_t pmode);

/**
 * @fn void mctParamInit(sMCTPARAM param)
 *  @brief 墨卡托参数初始化.
 *  @param[in] param 投影参数.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void mctParamInit(sMCTPARAM param);

/**
 * @fn void lbtParamInit(sLBTPARAM param)
 *  @brief 兰勃特参数初始化.
 *  @param[in] param 投影参数.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void lbtParamInit(sLBTPARAM param);

/**
 * @fn void clearColorSet(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
 *  @brief 设置清屏的颜色.
 *  @param[in] red,green,blue,alpha 颜色的rgba分量，0.0f~1.0f. 
 *  @exception void 
 *  @return void
 * @see 
 * @note 清屏是清除整个缓冲区，不能针对视口清除，因此只需在应用层设置一次就可以了，不需在每个场景都设置
*/
void clearColorSet(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha);

/**
 * @fn sPAINTUNIT * createUnit(void)
 *  @brief 创建图形绘制单元句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return sPAINTUNIT *.
 *  @retval 图形绘制单元句柄.
 *  @retval NULL    失败.
 * @see 
 * @note
*/
sPAINTUNIT * createUnit(void);

/**
 * @fn f_int32_t createRenderViewWindow(VOIDPtr *pviewwind)
 *  @brief 创建图形渲染视窗.
 *  @param[in] pviewwind 视窗句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return f_int32_t,创建成功标志.
 *  @retval 0 成功.
 *  @retval -1 视窗句柄创建失败.
 * @see 
 * @note
*/
f_int32_t createRenderViewWindow(VOIDPtr *paintwind);

/**
 * @fn f_int32_t createRenderScene(VOIDPtr *render_scene)
 *  @brief 创建场景句柄.
 *  @param[in] render_scene 场景句柄.
 *  @exception void
 *  @return f_int32_t, 创建成功标志.
 *  @retval 0 成功.
 *  @retval -1: 渲染场景句柄创建失败.
 *  @retval -2: 地图句柄初始化失败.
 * @see 
 * @note
*/
f_int32_t createRenderScene(VOIDPtr *render_scene);

/**
 * @fn f_int32_t memPoolInit(void)
 *  @brief 内存池初始化.
 *  @exception void
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
 * @see 
 * @note 
*/
f_int32_t memPoolInit(void);

/**
 * @fn f_int32_t	paintunitViewwndSceneAttach(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene)
 *  @brief 挂接绘制单元的视窗句柄和场景句柄.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] pviewwnd 视窗句柄.
 *  @param[in] pscene 场景句柄.
 *  @exception void
 *  @return f_int32_t, 挂接成功标志.
 *  @retval 0  挂接成功.
 *  @retval -1 挂接失败.
 * @see 
 * @note
*/
f_int32_t paintunitViewwndSceneAttach(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene);

/**
 * @fn sceneFuncAttach(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, \n
 *                     USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer, \n
 *                     FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt)
 *  @brief 挂接场景的功能函数.
 *  @param[in] scene 场景句柄.
 *  @param[in] draw2dMap 2D地图绘制的入口函数.
 *  @param[in] draw3dMap 3D地图绘制的入口函数.
 *  @param[in] drawScreenUserLayer 用户屏幕图层绘制的入口函数.
 *  @param[in] drawGeoUserLayer 用户地理图层绘制的入口函数.
 *  @param[in] screenPt2GeoPt 屏幕坐标到地理坐标转换的入口函数.
 *  @param[in] geoPt2ScreenPt 地理坐标到屏幕坐标转换的入口函数.
 *  @exception void
 *  @return f_int32_t, 挂接成功标志.
 *  @retval 0  挂接成功.
 *  @retval -1 挂接失败.
 * @see 
 * @note
*/
f_int32_t sceneFuncAttach(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, 
                          USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer,
                          FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
