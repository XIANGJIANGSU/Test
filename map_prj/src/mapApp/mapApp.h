#ifndef _MAP_APP_h_ 
#define _MAP_APP_h_ 

#include "../define/mbaseType.h"
#include "appHead.h"
#include "../engine/mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn void setScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height,\n
                           f_int32_t width, f_int32_t height)
 *  @brief 初始化显示器的参数.
 *  @param[in] rotate_type  显示器旋方式，0:正常,1:顺时旋转90度,2:顺时针旋转180度,3:顺时针旋转270度. 
 *  @param[in] phi_width    显示器物理尺寸的宽度(单位：厘米). 
 *  @param[in] phi_height   显示器物理尺寸的高度(单位：厘米). 
 *  @param[in] width        显示器分辨率尺寸的宽度(单位：像素). 
 *  @param[in] height       显示器分辨率尺寸的高度(单位：像素). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height, 
                    f_int32_t width, f_int32_t height);

/**
 * @fn void getScreenRotateType(f_int32_t *protatetype)
 *  @brief 获取屏幕旋转方式. 0:正常,1:顺时旋转90度,2:顺时针旋转180度,3:顺时针旋转270度.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenRotateType(f_int32_t *protatetype);


/**
 * @fn void setScreenWndSize(f_int32_t width, f_int32_t height)
 *  @brief 设置屏幕窗口尺寸(像素分辨率). 
 *  @param[in] width    窗口的宽度(单位：像素). 
 *  @param[in] height   窗口的高度(单位：像素). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setScreenWndSize(f_int32_t width, f_int32_t height);

/**
 * @fn void getScreenWndSize(f_int32_t *pwidth, f_int32_t *pheight)
 *  @brief 获取屏幕窗口尺寸(像素分辨率). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenWndSize(f_int32_t *pwidth, f_int32_t *pheight);                   

/**
 * @fn sPAINTUNIT *unitPtrValid(VOIDPtr punit)
 *  @brief 判断输入的绘制单元句柄是否有效.
 *  @param[in] punit 单元句柄.
 *  @exception void
 *  @return sPAINTUNIT *, 单元句柄.
 *  @retval 绘制单元句柄.
 *  @retval NULL 绘制单元句柄无效.
 * @see 
 * @note
*/
sPAINTUNIT *unitPtrValid(VOIDPtr punit);

/**
 * @fn sViewWindow *viewWindowPtrValid(VOIDPtr view_window)
 *  @brief 判断输入的渲染视窗句柄是否有效.
 *  @param[in] view_window 视窗句柄.
 *  @exception void
 *  @return sViewWindow，视窗句柄.
 *  @retval 视窗句柄.
 *  @retval NULL 视窗句柄无效.
 * @see 
 * @note
*/
sViewWindow *viewWindowPtrValid(VOIDPtr view_window);

/**
 * @fn sGLRENDERSCENE *scenePtrValid(VOIDPtr render_scene)
 *  @brief 判断输入的场景句柄是否有效.
 *  @param[in] render_scene 场景句柄.
 *  @exception void
 *  @return sGLRENDERSCENE，场景句柄.
 *  @retval 场景句柄.
 *  @retval NULL 场景句柄无效.
 * @see 
 * @note
*/
sGLRENDERSCENE *scenePtrValid(VOIDPtr render_scene);
sMAPHANDLE *maphandlePtrValid(VOIDPtr map_handle);

/**
 * @fn f_int32_t setViewWindowParamPre(VOIDPtr view_window)
 *  @brief 设置视窗参数的前置函数，获取参数设置的信号量. 
 *  @param[in] view_window 视窗句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamPre(VOIDPtr view_window);

/**
 * @fn f_int32_t setViewWindowParamPro(VOIDPtr view_window)
 *  @brief 设置视窗参数的后置函数，释放参数设置的信号量. 
 *  @param[in] view_window 视窗句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamPro(VOIDPtr view_window);

/**
 * @fn void setViewWindowAlpha(VOIDPtr view_window, f_float32_t alpha)
 *  @brief 设置视窗的透明度.
 *  @param[in] view_window 视窗句柄.
 *  @param[in] alpha 透明度.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWindowAlpha(VOIDPtr view_window, f_float32_t alpha);

/**
 * @fn void setViewWindowBkcolor(VOIDPtr view_window, sColor3f bkcolor)
 *  @brief 设置视窗的背景色.
 *  @param[in] view_window 视窗句柄.
 *  @param[in] bkcolor 背景色.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWindowBkcolor(VOIDPtr view_window, sColor3f bkcolor);

/**
 * @fn f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
 *  @brief 设置渲染视窗的参数.
 *  @param[in] view_window 渲染视窗的句柄.
 *  @param[in] scene 渲染场景的句柄.
 *  @param[in] llx 视窗的左下角横坐标的起始位置(单位:像素,相对于旋转后的屏幕左上角).
 *  @param[in] lly 视窗的左下角纵坐标的起始位置(单位:像素,相对于旋转后的屏幕左上角).
 *  @param[in] width 视窗宽度（像素）.
 *  @param[in] height 视窗高度（像素）.
 *  @exception void
 *  @return f_int32_t.
 *  @retval 0 成功.
 *  @retval -1 视窗句柄为空.
 *  @retval -2 起始位置不在范围内.
 *  @retval -3 宽度、高度不在范围内.
 *  @retval -4 外部屏幕坐标转换成内部屏幕坐标出错.
 * @see 
 * @note
*/
f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height);

/**
 * @fn setViewWndParam(sViewWindow *pviewwind)
 *  @brief 设置渲染视窗的有关参数，包括尺寸、量程环半径、透明度、旋转中心. 
 *  @param[in] pviewwind 视窗句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证参数的一致性，需要用信号量来进行保护.
*/
void setViewWndParam(sViewWindow *pviewwind);

/**
 * @fn void mapSceneRender(VOIDPtr render_wnd, VOIDPtr render_scene)
 *  @brief 地图场景渲染.
 *  @param[in] render_viewwnd  视窗句柄.
 *  @param[in] render_scene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void mapSceneRender(VOIDPtr render_viewwnd, VOIDPtr render_scene);

/**
 * @fn void render2dMap(VOIDPtr *scene)
 *  @brief 2d地图场景渲染.
 *  @param[in] scene  场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void render2dMap(VOIDPtr scene);

/**
 * @fn void render3dMap(VOIDPtr *scene)
 *  @brief 3d地图场景渲染.
 *  @param[in] scene  场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void render3dMap(VOIDPtr scene);

/*
*	绘制模型的接口函数
*/
extern BOOL RenderModel(BOOL SelectModel , sGLRENDERSCENE* pModelScene);

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

void render3dmap_simple(VOIDPtr scene);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
