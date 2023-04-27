#include "../engine/zeroPitchLine.h"
#include "../engine/compassRose.h"
#include "../projection/coord.h"
#include "auxNavInfo.h"
#include "appHead.h"
#include "mapApp.h"

/* 0俯仰角线数据初始化 */
/* 0-初始化成功  -1-初始化失败*/
static f_int32_t zplDataInit(sGLRENDERSCENE *pScene, f_char_t *data_path)
{
	f_int32_t ret = 0;
	f_char_t buf[256] = {0};
	f_int32_t texture_id = 0;
	sprintf(buf, "%s/zplT.bmp", data_path);
#ifndef _JM7200_
	/* zplT.bmp尺寸是8192*64，7200支持的纹理最大尺寸为4096，7200下不创建此纹理*/
	ret = createZeroPitchLineTexture(buf, &texture_id);
#endif
	pScene->auxnav_list.zpl_list.texture_id = texture_id;
	return ret;
}

/* 罗盘数据初始化 */
/* 0-初始化成功  -1-初始化失败*/
static f_int32_t cpsDataInit(sGLRENDERSCENE *pScene, f_char_t *data_path)
{
	f_int32_t ret = 0;
	f_char_t buf[256] = {0};
	f_int32_t texture_id = 0;
	sprintf(buf, "%s/cpsT.bmp", data_path);
	ret = createCompassRoseTexture(buf, &texture_id);
	pScene->auxnav_list.cps_list.texture_id = texture_id;
	return ret;
}

/**
 * @fn BOOL auxNavInfoDataInit(VOIDPtr render_scene, f_char_t *data_path)
 *  @brief 辅助导航信息数据初始化.
 *  @param[in] scene 视口所在的场景句柄.
 *  @param[in] data_path 视口对应数据路径的根目录.
 *  @exception void
 *  @return BOOL,初始化成功标志.
 *  @retval TRUE 成功.
 *  @retval FALSE 失败.
 * @see 
 * @note
*/
BOOL auxNavInfoDataInit(VOIDPtr render_scene, f_char_t *data_path)
{
	f_int32_t ret = 0;
    sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return (FALSE);
	    
    ret = zplDataInit(pScene, data_path);
	if(0 != ret)
	{
		return (FALSE);
	}
    ret = cpsDataInit(pScene, data_path);
	if(0 != ret)
	{
		return (FALSE);
	}

	return(TRUE);
}

/**
 * @fn void setZplParam(VOIDPtr render_scene, sZplParam zplprarm)
 *  @brief 设定0俯仰角线是否的参数.
 *  @param[in] punit  绘制单元句柄.
 *  @param[in] zplprarm 0俯仰角线参数.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setZplParam(VOIDPtr render_scene, sZplParam zplprarm)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->auxnavinfo_param_input.zpl_param  = zplprarm;
}

/**
 * @fn void setCpsParam(VOIDPtr render_scene, sCpsParam cpsprarm)
 *  @brief 设定罗盘显示的参数.
 *  @param[in] punit  绘制单元句柄.
 *  @param[in] zplprarm 罗盘参数.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setCpsParam(VOIDPtr render_scene, sCpsParam cpsprarm)
{
	PT_2I outer_screen_pt, outer_view_pt, inner_screen_pt, inner_view_pt;
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	outer_view_pt.x = (f_int32_t)cpsprarm.cent_x;
	outer_view_pt.y = (f_int32_t)cpsprarm.cent_y;
	if(outerViewPt2outerScreenPt(pScene->outerviewport, outer_view_pt, &outer_screen_pt))
    {
    	if(outerScreenPt2innerScreenPt(outer_screen_pt, &inner_screen_pt))
    	{
    		if(innerScreenPt2innerViewPt(pScene->innerviewport, inner_screen_pt, &inner_view_pt))
    		{
    			cpsprarm.cent_x = (f_float32_t)inner_view_pt.x;
	            cpsprarm.cent_y = (f_float32_t)inner_view_pt.y;
    		}
    		else
    		{
	            DEBUG_PRINT("innerScreenPt2innerViewPt error.");
    		}
    	}
    	else
    	{
	        DEBUG_PRINT("outerScreenPt2innerScreenPt error.");
    	}
    }
    else
    {
	    DEBUG_PRINT("outerViewPt2outerScreenPt error.");
    }   
	
	pScene->auxnavinfo_param_input.cps_param  = cpsprarm;
}

/**
 * @fn void setPrefileParam(VOIDPtr render_scene, sPrefileParam prefileprarm)
 *  @brief 设定剖面图是否的参数.
 *  @param[in] punit  绘制单元句柄.
 *  @param[in] prefileprarm 剖面图参数.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setPrefileParam(VOIDPtr render_scene, sPrefileParam  prefileprarm)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->auxnavinfo_param_input.prefile_param = prefileprarm;
}

/**
 * @fn setPrefileDisplay(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief 设置是否显示剖面图
 *  @param[in] render_scene 场景句柄.
 *  @param[in] isdisplay 是否显示，0:不显示，1:显示
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPrefileDisplay(VOIDPtr render_scene, f_int32_t isdisplay)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->auxnavinfo_param_input.prefile_param.is_display = isdisplay;
}


/**
 * @fn setPrefileDetectDistance(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief 设置剖面图水平探测距离
 *  @param[in] render_scene 场景句柄.
 *  @param[in] hDetectDistance 水平探测距离
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPrefileDetectDistance(VOIDPtr render_scene, f_float64_t hDetectDistance)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->auxnavinfo_param_input.prefile_param.s_WorldDetectDistance = hDetectDistance;
}

/**
 * @fn setPrefileTerrainColor(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief 设置剖面图垂直地形剖面图VSD的垂直方向半屏代表实际距离(米)及高度色带相关参数
*   @param[in] punit 绘制单元句柄.
 *  @param[in] color_num 色带数量,范围为[2,6]
 *  @param[in] prefile_height[5] 色带区间的高度,每条色带对应一个区间范围,5个值对应最多6个区间
 *  @param[in] prefile_color[6][4] 色带的颜色RGBA
 *  @param[in] s_winHeiMeter 垂直方向半屏代表实际距离(米)，必须大于0
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPrefileTerrainColor(VOIDPtr render_scene, f_int32_t color_num, f_float32_t prefile_height[5], 
	f_uint8_t prefile_color[6][4], f_int32_t s_winHeiMeter)
{
	int i = 0;
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->auxnavinfo_param_input.prefile_param.color_num = color_num;
	pScene->auxnavinfo_param_input.prefile_param.s_winHeiMeter = s_winHeiMeter;

	for(i=0;i<5;i++)
	{
		pScene->auxnavinfo_param_input.prefile_param.prefile_height[i] = prefile_height[i];

		pScene->auxnavinfo_param_input.prefile_param.prefile_color[i][0] = prefile_color[i][0];
		pScene->auxnavinfo_param_input.prefile_param.prefile_color[i][1] = prefile_color[i][1];
		pScene->auxnavinfo_param_input.prefile_param.prefile_color[i][2] = prefile_color[i][2];
		pScene->auxnavinfo_param_input.prefile_param.prefile_color[i][3] = prefile_color[i][3];
	}

	pScene->auxnavinfo_param_input.prefile_param.prefile_color[5][0] = prefile_color[5][0];
	pScene->auxnavinfo_param_input.prefile_param.prefile_color[5][1] = prefile_color[5][1];
	pScene->auxnavinfo_param_input.prefile_param.prefile_color[5][2] = prefile_color[5][2];
	pScene->auxnavinfo_param_input.prefile_param.prefile_color[5][3] = prefile_color[5][3];
}