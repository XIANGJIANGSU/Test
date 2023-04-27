/**
 * @file paramGetApi.h
 * @brief 该文件提供获取场景参数的外部接口函数
 * @author 615地图团队
 * @date 2016-08-17
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _PARAM_GET_API_h_ 
#define _PARAM_GET_API_h_ 


/*
引擎中的几个坐标系：
1. 外部屏幕坐标系：屏幕旋转后的坐标,原点是在屏幕的左上角,横坐标向左增大,纵坐标向下增大;
2. 内部屏幕坐标系：屏幕旋转前(屏幕正常摆放)的坐标,原点是在屏幕的左上角,横坐标向左增大,纵坐标向下增大;
3. 外部视口坐标系：屏幕旋转后的坐标,原点是在屏幕的左下角,横坐标向左增大,纵坐标向上增大;
4. 内部视口坐标系：屏幕旋转前(屏幕正常摆放)的坐标,原点是在屏幕的左下角,横坐标向左增大,纵坐标向上增大;
5. 物体坐标系：在球显示模式下就是指球坐标;
6. 地理坐标系：经纬高;

坐标转换的几点说明：
1. 引擎内部场景绘制时都是基于内部视口坐标的;
   屏幕坐标原点定义在屏幕的左下角；
   视口坐标原点定义在视口的左下角；
   物体坐标在球显示模式下就是指球坐标,在球投影下可以直接绘制；
   物体坐标在MCT显示模式下就是指MCT坐标,在MCT投影下可以直接绘制；
   逻辑坐标是与mct坐标对应的，将mct坐标转换到0～1之间；
   openGL坐标是与视口坐标对应的，将视口坐标转换到-1～1之间；
2. 绘制时：
   a. 如果输入的是外部屏幕坐标,需将外部屏幕坐标转换成内部屏幕坐标,再将内部屏幕坐标转换成内部视口坐标,然后进行绘制;
   b. 如果输入的是外部视口坐标,需将外部视口坐标转换成外部屏幕坐标,再将外部屏幕坐标转换成内部屏幕坐标,
      然后将内部屏幕坐标转换成内部视口坐标,最后进行绘制;
3. 屏幕未旋转时，外部屏幕坐标与内部屏幕坐标一致，外部视口坐标与内部视口坐标一致;
*/


#include "../define/mbaseType.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


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
BOOL getInnerViewPort(VOIDPtr pscene, f_int32_t *view_port);

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
BOOL getInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *mdviewmatrix); 

/* 获取世界矩阵 */
BOOL getmxWorld(VOIDPtr pscene, LPMatrix44 matrix); 
/* 获取旋转矩阵 */
BOOL getmxRotate(VOIDPtr pscene, LPMatrix44 matrix);   

/**
 * @fn BOOL getPlaneObjPos(VOIDPtr pscene, LP_PT_3D pt_pos)
 *  @brief 获取飞机位置.
 *  @param[in] pscene  绘制场景句柄.
 *  @param[out] pt_pos  飞机的物体坐标.
 *  @exception void
 *  @return 获取飞机物体坐标成功与否标志
 * @see 
 * @note
*/
BOOL getPlaneObjPos(VOIDPtr pscene, LP_PT_3D pt_pos); 


/**
 * @fn BOOL getGeoByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
 *  @brief 根据外部屏幕坐标获取地理坐标.外部屏幕坐标->内部屏幕坐标->内部视口坐标->地理坐标.
 *         其中内部视口坐标->地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene    绘制场景句柄.
 *  @param[in] screenPt  外部屏幕坐标.
 *  @param[out] geoPt    地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoByScreenPt(VOIDPtr scene, const PT_2I screenPt, LP_Geo_Pt_D geoPt);
/**
 * @fn BOOL getScreenPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I screenPt)
 *  @brief 根据地理坐标获取外部屏幕坐标.外部屏幕坐标<-内部屏幕坐标<-内部视口坐标<-地理坐标.
 *         其中内部视口坐标<-地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] geoPt      地理坐标.
 *  @param[out] screenPt  外部屏幕坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getScreenPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I screenPt); 



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
BOOL getGeoByViewPt(VOIDPtr scene, const PT_2I viewPt, LP_Geo_Pt_D geoPt);

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
BOOL getViewPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I viewPt);



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
BOOL getViewPtByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt); 
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
BOOL getScreenPtByViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt);  


/**
 * @fn BOOL getDrawPtByScreenPt(VOIDPtr pscene, const PT_2I s_pt, LP_PT_2F draw_pt)
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
BOOL getDrawPtByScreenPt(VOIDPtr pscene, const PT_2I s_pt, LP_PT_2I draw_pt);  

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
BOOL getScreenPtByDrawPt(VOIDPtr pscene, const PT_2I draw_pt, LP_PT_2I s_pt);


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
BOOL getDrawPtByGeoPt(const Geo_Pt_D geo_pt, LP_PT_3D draw_pt); 

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
BOOL getGeoPtByDrawPt(const PT_3D draw_pt, LP_Geo_Pt_D geo_pt); 

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
