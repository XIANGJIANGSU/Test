#include "paramGetApi.h"
#include "common.h"
#include "../mapApp/mapApp.h"
#include "../mapApp/paramGet.h"
#include "../mapApp/bil.h"

/**
 * @fn BOOL getInnerViewPort(VOIDPtr pscene, f_int32_t *view_port)
 *  @brief 获取场景的内部视口参数(左下角坐标、宽度、高度).
 *  @param[in] pscene  绘制场景句柄.
 *  @param[out] view_port  内部视口参数.
 *  @exception void
 *  @return 获取视口参数成功与否标志
 * @see 
 * @note
*/
BOOL getInnerViewPort(VOIDPtr pscene, f_int32_t *view_port)
{
	return(getSceneInnerViewPort(pscene, view_port));
}

/**
 * @fn BOOL getInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *matrix)
 *  @brief 获取场景的模视矩阵或投影矩阵.
 *  @param[in] pscene  绘制场景句柄.
 *  @param[in] matrix_mod  矩阵类型.
 *  @param[out] matrix  矩阵.
 *  @exception void
 *  @return 获取矩阵成功与否标志
 * @see 
 * @note
*/
BOOL getInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *mdviewmatrix)
{
	return(getSceneInnerMatrix(pscene, matrix_mod, mdviewmatrix));
}

/* 获取世界矩阵 */
BOOL getmxWorld(VOIDPtr pscene, LPMatrix44 matrix)
{
	return(getSceneMxworld(pscene, matrix));
}

/* 获取旋转矩阵 */
BOOL getmxRotate(VOIDPtr pscene, LPMatrix44 matrix)
{
	return(getSceneMxrotate(pscene, matrix));
}

/**
 * @fn BOOL getScenePlaneObjPos(VOIDPtr pscene, LP_PT_3D pt_pos)
 *  @brief 获取飞机位置.
 *  @param[in] pscene  绘制场景句柄.
 *  @param[out] pt_pos  飞机的物体坐标.
 *  @exception void
 *  @return 获取飞机物体坐标成功与否标志
 * @see 
 * @note
*/
BOOL getPlaneObjPos(VOIDPtr pscene, LP_PT_3D pt_pos)
{
	return(getScenePlaneObjPos(pscene, pt_pos));
}

/**
 * @fn BOOL getGeoByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
 *  @brief 根据外部屏幕坐标获取地理坐标.外部屏幕坐标->内部屏幕坐标->内部视口坐标->地理坐标.
 *         其中内部视口坐标->地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene    绘制场景句柄.
 *  @param[in] screenPt  外部屏幕坐标.
 *  @param[out] geoPt    地理坐标.
 *  @exception void
 *  @return 获取地理坐标成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
{
	return(getGeoByOuterScreenPt(pscene, screenPt, geoPt));
}

/**
 * @fn BOOL getGeoByViewPt(VOIDPtr scene, const PT_2I viewPt, LP_Geo_Pt_D geoPt);
 *  @brief 根据外部视口坐标获取地理坐标.外部视口坐标->外部屏幕坐标->内部屏幕坐标->内部视口坐标->地理坐标.
 *         其中内部视口坐标->地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene    绘制场景句柄.
 *  @param[in] viewPt    外部视口坐标.
 *  @param[out] geoPt    地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoByViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
{
	return(getGeoByOuterViewPt(pscene, viewPt, geoPt));
}

/**
 * @fn BOOL getScreenPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I screenPt)
 *  @brief 根据地理坐标获取外部屏幕坐标.外部屏幕坐标<-内部屏幕坐标<-内部视口坐标<-地理坐标.
 *         其中内部视口坐标<-地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] geoPt      地理坐标.
 *  @param[out] screenPt  外部屏幕坐标.
 *  @exception void
 *  @return 1-成功,0-失败
 * @see 
 * @note
*/
BOOL getScreenPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I screenPt)
{
	return(getOuterScreenPtByGeo(pscene, geoPt, screenPt));
}

/**
 * @fn BOOL getViewPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I viewPt)
 *  @brief 根据地理坐标获取外部视口坐标.外部视口坐标<-外部屏幕坐标<-内部屏幕坐标<-内部视口坐标<-地理坐标.
 *         其中内部视口坐标<-地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] geoPt      地理坐标.
 *  @param[out] viewPt    外部视口坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getViewPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I viewPt)
{
	return(getOuterViewPtByGeo(pscene, geoPt, viewPt));
}

/**
 * @fn BOOL getViewPtByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
 *  @brief 根据外部屏幕坐标获取外部视口坐标.外部视口坐标<-外部屏幕坐标.
 *         不依赖于模式矩阵和投影矩阵,但是与屏幕正常摆放的宽度或高度有关
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] screenPt   外部屏幕坐标.
 *  @param[out] viewPt    外部视口坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getViewPtByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
{
	return(getOuterViewPtByOuterScreenPt(pscene, screenPt, viewPt));
}

/**
 * @fn BOOL getScreenPtByViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
 *  @brief 根据外部视口坐标获取外部屏幕坐标.外部视口坐标->外部屏幕坐标.
 *         不依赖于模式矩阵和投影矩阵,但是与屏幕正常摆放的宽度或高度有关
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] viewPt     外部视口坐标.
 *  @param[out] screenPt  外部屏幕坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getScreenPtByViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
{
	return(getOuterScreenPtByOuterViewPt(pscene, viewPt, screenPt));
}

/**
 * @fn BOOL getDrawPtByScreenPt(VOIDPtr pscene, const PT_2I s_pt, LP_PT_2I draw_pt)
 *  @brief 根据外部屏幕坐标获取内部视口坐标.外部屏幕坐标->内部屏幕坐标->内部视口坐标.
 *         不依赖于模式矩阵和投影矩阵,但是与屏幕正常摆放的宽度或高度有关
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] s_pt       外部屏幕坐标.
 *  @param[out] draw_pt   内部视口坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getDrawPtByScreenPt(VOIDPtr pscene, const PT_2I s_pt, LP_PT_2I draw_pt)
{
	return(getInnerViewPtByOuterScreenPt(pscene, s_pt, draw_pt));
}

/**
 * @fn BOOL getScreenPtByDrawPt(VOIDPtr pscene, const PT_2I draw_pt, LP_PT_2I s_pt)
 *  @brief 根据内部视口坐标获取外部屏幕坐标.外部屏幕坐标<-内部屏幕坐标<-内部视口坐标.
 *         不依赖于模式矩阵和投影矩阵,但是与屏幕正常摆放的宽度或高度有关
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] draw_pt    内部视口坐标.
 *  @param[out] s_pt      外部屏幕坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getScreenPtByDrawPt(VOIDPtr pscene, const PT_2I draw_pt, LP_PT_2I s_pt)
{
	return(getOuterScreenPtByInnerViewPt(pscene, draw_pt, s_pt));
}

/**
 * @fn BOOL getDrawPtByGeoPt(const Geo_Pt_D geo_pt, LP_PT_3D draw_pt)
 *  @brief 根据地理坐标获取物体坐标.
 *         不依赖于模式矩阵和投影矩阵,仅涉及数学运算
 *  @param[in] geo_pt     地理坐标.
 *  @param[out] draw_pt   物体坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getDrawPtByGeoPt(const Geo_Pt_D geo_pt, LP_PT_3D draw_pt)
{
	return(getObjPtByGeoPt(geo_pt, draw_pt));
}

/**
 * @fn BOOL getGeoPtByDrawPt(const PT_3D draw_pt, LP_Geo_Pt_D geo_pt)
 *  @brief 根据物体坐标获取地理坐标.
 *         不依赖于模式矩阵和投影矩阵,仅涉及数学运算
 *  @param[in] draw_pt    物体坐标.
 *  @param[out] geo_pt    地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoPtByDrawPt(const PT_3D draw_pt, LP_Geo_Pt_D geo_pt)
{
	return(getGeoPtByObjPt(draw_pt, geo_pt));
}

