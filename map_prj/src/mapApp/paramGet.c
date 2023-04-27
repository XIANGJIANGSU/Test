#include <math.h>
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../engine/mapRender.h"
#include "../projection/coord.h"
#include "../projection/project.h"
#include "mapApp.h"
#include "paramGet.h"

extern BOOL innerViewDPt2GeoDPtOpenglEs(sGLRENDERSCENE *pScene,
	const PT_2I viewPt, LP_Geo_Pt_D geoPt);
extern BOOL outerScreenDPt2GeoDPtOpengEs(sGLRENDERSCENE *pScene, const PT_2I screenPt, 
	const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);
extern BOOL outerViewDPt2GeoDPtOpengEs(sGLRENDERSCENE *pScene, const PT_2I viewPt, 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);

/**
 * @fn BOOL getSceneInnerViewPort(VOIDPtr scene, f_int32_t *view_port)
 *  @brief 获取场景的内部视口参数.
 *  @param[in] scene  场景句柄.
 *  @param[out] view_port  视口参数.
 *  @exception void
 *  @return 成功与否标志
 * @see 
 * @note
*/
BOOL getSceneInnerViewPort(VOIDPtr scene, f_int32_t *view_port)
{
	sGLRENDERSCENE *pScene = NULL;
	f_int32_t i = 0;
    pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return FALSE;

	for(i = 0; i < 4; i++)
	{
		*view_port = pScene->innerviewport[i];
		view_port++;
	}
    return TRUE;
}

/**
 * @fn BOOL getSceneInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *matrix)
 *  @brief 获取场景的模视矩阵或投影矩阵.
 *  @param[in] pscene  绘制场景句柄.
 *  @param[in] matrix_mod  矩阵类型.
 *  @param[out] matrix  矩阵.
 *  @exception void
 *  @return 获取矩阵成功与否标志
 * @see 
 * @note
*/
BOOL getSceneInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *matrix)
{
	sGLRENDERSCENE *pScene = NULL;
	f_int32_t i = 0;
    pScene = scenePtrValid(pscene);
	if(NULL == pScene)
	    return FALSE;

    switch(matrix_mod)
    {
    	default:
    	case eMatrix_Mdview:
	    for(i = 0; i < 16; i++)
	    {
		    *matrix = pScene->m_lfModelMatrix[i];
		    matrix++;
	    }
	    break;
	    
	    case eMatrix_Mdprj:
	    for(i = 0; i < 16; i++)
	    {
		    *matrix = pScene->m_lfProjMatrix[i];
		    matrix++;
	    }
	    break;
    }
    return TRUE;
}

/* 获取世界矩阵 */
BOOL getSceneMxworld(VOIDPtr pscene, LPMatrix44 matrix)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(pscene);
	if(NULL == pScene)
	    return FALSE;
	memcpy((VOIDPtr)matrix, (VOIDPtr)(pScene->m_mxWorld.m), sizeof(Matrix44));    
	return TRUE;
}

/* 获取旋转矩阵 */
BOOL getSceneMxrotate(VOIDPtr pscene, LPMatrix44 matrix)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(pscene);
	if(NULL == pScene)
	    return FALSE;
	memcpy((VOIDPtr)matrix, (VOIDPtr)(pScene->m_mxRotate.m), sizeof(Matrix44));    
	return TRUE;
}

/* 获取飞机物体坐标 */
BOOL getScenePlaneObjPos(VOIDPtr pscene, LP_PT_3D pt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == pt))
	    return(FALSE);
	pt->x = pScene->objpt_pos.x;
	pt->y = pScene->objpt_pos.y;
	pt->z = pScene->objpt_pos.z;
	return(TRUE);
}

/**
 * @fn BOOL getGeoByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
 *  @brief 根据外部屏幕坐标获取地理坐标.外部屏幕坐标->内部屏幕坐标->内部视口坐标->地理坐标.
 *         其中内部视口坐标->地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影，墨卡托投影只有俯视时能转换.
 *  @param[in] pscene    绘制场景句柄.
 *  @param[in] screenPt  外部屏幕坐标.
 *  @param[out] geoPt    地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == geoPt))
	    return(FALSE);

	if (ePRJ_ball == pScene->project_mode)
	{
		/*球投影俯视和其他视角转换函数不同*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!outerScreenDPt2GeoDPtCorrect(screenPt, pScene->m_lfModelMatrix, pScene->invmat, pScene->innerviewport, geoPt))
				return(FALSE);	
		}else
		{
			//if(!outerScreenDPt2GeoDPt(screenPt, pScene->invmat, pScene->innerviewport, geoPt))
			//	return(FALSE);
			if(!outerScreenDPt2GeoDPtOpengEs(pScene, screenPt, pScene->innerviewport, geoPt))
				return(FALSE);
		}
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*墨卡托投影只有俯视视角，没有其他视角，直接返回转换失败*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!outerScreenDPt2GeoDPtMct(screenPt, pScene->m_lfModelMatrix, pScene->invmat, pScene->innerviewport, geoPt))
				return(FALSE);	
		}else
		{
			return(FALSE);
		}
	}else{
		/*其他投影不支持，直接返回转换失败*/
		return(FALSE);
	}
	

	return(TRUE);
}

/**
 * @fn BOOL getOuterScreenPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I screenPt)
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
BOOL getOuterScreenPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I screenPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == screenPt))
	    return(FALSE);

	if (ePRJ_ball == pScene->project_mode)
	{
		/*球投影*/
		if(!geoDPt2OuterScreenDPt(&geoPt, pScene->matrix, pScene->innerviewport, screenPt))
			return(FALSE);
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*墨卡托投影*/
		if(!geoDPt2OuterScreenDPtMct(&geoPt, pScene->matrix, pScene->innerviewport, screenPt))
			return(FALSE);
	}else{
		/*其他投影不支持，直接返回转换失败*/
		return(FALSE);
	}

	return(TRUE);
}

/**
 * @fn BOOL getGeoByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
 *  @brief 根据外部视口坐标获取地理坐标.外部视口坐标->外部屏幕坐标->内部屏幕坐标->内部视口坐标->地理坐标.
 *         其中内部视口坐标->地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影，墨卡托投影只有俯视时能转换.
 *  @param[in] pscene    绘制场景句柄.
 *  @param[in] viewPt    外部视口坐标.
 *  @param[out] geoPt    地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == geoPt))
	    return(FALSE);

	if (ePRJ_ball == pScene->project_mode)
	{
		/*球投影俯视和其他视角转换函数不同*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!outerViewDPt2geoDPtCorrect(viewPt, pScene->m_lfModelMatrix, pScene->invmat, pScene->outerviewport, pScene->innerviewport, geoPt))
				return(FALSE);
		}else
		{
			//if(!outerViewDPt2geoDPt(viewPt, pScene->invmat, pScene->outerviewport, pScene->innerviewport, geoPt))
			//	return(FALSE);	
			if(!outerViewDPt2GeoDPtOpengEs(pScene, viewPt, pScene->outerviewport, pScene->innerviewport, geoPt))
				return(FALSE);
		}
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*墨卡托投影只有俯视视角，没有其他视角，直接返回转换失败*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!outerViewDPt2geoDPtMct(viewPt, pScene->m_lfModelMatrix, pScene->invmat, pScene->outerviewport, pScene->innerviewport, geoPt))
				return(FALSE);
		}else
		{
			return(FALSE);
		}
	}else{
		/*其他投影不支持，直接返回转换失败*/
		return(FALSE);
	}

	return(TRUE);
}



/**
 * @fn BOOL getOuterViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
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
BOOL getOuterViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == viewPt))
	    return(FALSE);
	if (ePRJ_ball == pScene->project_mode)
	{
		/*球投影*/
		if(!geoDPt2OuterViewDPt(&geoPt, pScene->matrix, pScene->outerviewport, pScene->innerviewport, viewPt))
			return(FALSE);
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*墨卡托投影*/
		if(!geoDPt2OuterViewDPtMct(&geoPt, pScene->matrix, pScene->outerviewport, pScene->innerviewport, viewPt))
			return(FALSE);
	}else{
		/*其他投影不支持，直接返回转换失败*/
		return(FALSE);
	}

	return(TRUE);
}


/**
 * @fn BOOL getGeoByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
 *  @brief 根据内部视口坐标获取地理坐标(内部视口坐标z值强制为0).内部视口坐标->地理坐标.
 *         其中内部视口坐标->地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影，墨卡托投影只有俯视时能转换.
 *  @param[in] pscene    绘制场景句柄.
 *  @param[in] viewPt    外部视口坐标.
 *  @param[out] geoPt    地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
{
	PT_2D viewPt_2D;
	PT_3D viewPt_3D;
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == geoPt))
	    return(FALSE);

	viewPt_2D.x = (f_float64_t)viewPt.x;
	viewPt_2D.y = (f_float64_t)viewPt.y;

	viewPt_3D.x = (f_float64_t)viewPt.x;
	viewPt_3D.y = (f_float64_t)viewPt.y;
	viewPt_3D.y = 0.0;

	if (ePRJ_ball == pScene->project_mode)
	{
		/*球投影俯视和其他视角转换函数不同*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!innerViewDPt2geoDPtCorrect(&viewPt_2D, pScene->m_lfModelMatrix, pScene->invmat, pScene->innerviewport, geoPt))
				return(FALSE);
		}else{
			//if(!innerViewDPt2geoDPt(&viewPt_3D, pScene->invmat, pScene->innerviewport, geoPt))
			//	return(FALSE);
			if(!innerViewDPt2GeoDPtOpenglEs(pScene, viewPt, geoPt))
				return(FALSE);
		}
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*墨卡托投影只有俯视视角，没有其他视角，直接返回转换失败*/
		if(!innerViewDPt2geoDPtMct(&viewPt_3D, pScene->invmat, pScene->innerviewport, geoPt))
			return(FALSE);
	}else{
		/*其他投影不支持，直接返回转换失败*/
		return(FALSE);
	}

	return(TRUE);
}

/**
 * @fn BOOL getInnerViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
 *  @brief 根据地理坐标获取内部视口坐标(未返回z值).内部视口坐标<-地理坐标.
 *         其中内部视口坐标<-地理坐标要依赖于模式矩阵和投影矩阵.支持球投影和墨卡托投影.
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] geoPt      地理坐标.
 *  @param[out] viewPt    外部视口坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getInnerViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
{
	PT_3D viewPt_3D;
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == viewPt))
	    return(FALSE);

	if (ePRJ_ball == pScene->project_mode)
	{
		/*球投影*/
		if(!geoDPt2InnerViewDPt(&geoPt, pScene->matrix, pScene->innerviewport, &viewPt_3D))
			return(FALSE);
		else{
			viewPt->x = (f_int32_t)viewPt_3D.x;
			viewPt->y = (f_int32_t)viewPt_3D.y;
			//viewPt_3D.z不需要返回
		}
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*墨卡托投影*/
		if(!geoDPt2InnerViewDPtMct(&geoPt, pScene->matrix, pScene->innerviewport, &viewPt_3D))
			return(FALSE);
		else{
			viewPt->x = (f_int32_t)viewPt_3D.x;
			viewPt->y = (f_int32_t)viewPt_3D.y;
			//viewPt_3D.z不需要返回
		}
	}else{
		/*其他投影不支持，直接返回转换失败*/
		return(FALSE);
	}

	return(TRUE);
}


/**
 * @fn BOOL getOuterViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
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
BOOL getOuterViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == viewPt))
	    return(FALSE);
	if(!outerScreenPt2outerViewPt(pScene->outerviewport, screenPt, viewPt))
	    return(FALSE);
	return(TRUE);
}

/**
 * @fn BOOL getOuterScreenPtByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
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
BOOL getOuterScreenPtByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == screenPt))
	    return(FALSE);
	if(!outerViewPt2outerScreenPt(pScene->outerviewport, viewPt, screenPt))
	    return(FALSE);
	return(TRUE);
}


/**
 * @fn BOOL getInnerViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
 *  @brief 根据外部屏幕坐标获取内部视口坐标.外部屏幕坐标->内部屏幕坐标->内部视口坐标.
 *         不依赖于模式矩阵和投影矩阵,但是与屏幕正常摆放的宽度或高度有关
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] screenPt   外部屏幕坐标.
 *  @param[out] viewPt    内部视口坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getInnerViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
{
	PT_2I inner_screen_pt;
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == viewPt))
	    return(FALSE);
	if(!outerScreenPt2innerScreenPt(screenPt, &inner_screen_pt))
	    return(FALSE);
	if(!innerScreenPt2innerViewPt(pScene->innerviewport, inner_screen_pt, viewPt))
	    return(FALSE);

	return(TRUE);
}

/**
 * @fn BOOL getOuterScreenPtByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
 *  @brief 根据内部视口坐标获取外部屏幕坐标.外部屏幕坐标<-内部屏幕坐标<-内部视口坐标.
 *         不依赖于模式矩阵和投影矩阵,但是与屏幕正常摆放的宽度或高度有关
 *  @param[in] pscene     绘制场景句柄.
 *  @param[in] viewPt     内部视口坐标.
 *  @param[out] screenPt  外部屏幕坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getOuterScreenPtByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
{
	PT_2I inner_screen_pt;
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(pscene);
	if((NULL == pScene) || (NULL == screenPt))
	    return(FALSE);
	if(!innerViewPt2innerScreenPt(pScene->innerviewport, viewPt, &inner_screen_pt))
	    return(FALSE);
	if(!innerScreenPt2outerScreenPt(inner_screen_pt, screenPt))
	    return(FALSE);
	return(TRUE);
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
BOOL getObjPtByGeoPt(Geo_Pt_D geo_pt, LP_PT_3D obj_pt)  
{
	if (NULL == obj_pt)
	    return(FALSE);
	if(!geoDPt2objDPt(&geo_pt, obj_pt))
	    return(FALSE);
	return(TRUE);
}

/**
 * @fn BOOL getGeoPtByObjPt(PT_3D obj_pt, LP_Geo_Pt_D geo_pt) 
 *  @brief 根据物体坐标获取地理坐标.
 *         不依赖于模式矩阵和投影矩阵,仅涉及数学运算
 *  @param[in] draw_pt   物体坐标.
 *  @param[out] geo_pt     地理坐标.
 *  @exception void
 *  @return 获取成功与否标志,1-成功,0-失败
 * @see 
 * @note
*/
BOOL getGeoPtByObjPt(PT_3D obj_pt, LP_Geo_Pt_D geo_pt)  
{
	if (NULL == geo_pt)
	    return(FALSE);
	if(!objDPt2geoDPt(&obj_pt,geo_pt))
	    return(FALSE);
	return(TRUE);
}
