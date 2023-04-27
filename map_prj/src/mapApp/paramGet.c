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
 *  @brief ��ȡ�������ڲ��ӿڲ���.
 *  @param[in] scene  �������.
 *  @param[out] view_port  �ӿڲ���.
 *  @exception void
 *  @return �ɹ�����־
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
 *  @brief ��ȡ������ģ�Ӿ����ͶӰ����.
 *  @param[in] pscene  ���Ƴ������.
 *  @param[in] matrix_mod  ��������.
 *  @param[out] matrix  ����.
 *  @exception void
 *  @return ��ȡ����ɹ�����־
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

/* ��ȡ������� */
BOOL getSceneMxworld(VOIDPtr pscene, LPMatrix44 matrix)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(pscene);
	if(NULL == pScene)
	    return FALSE;
	memcpy((VOIDPtr)matrix, (VOIDPtr)(pScene->m_mxWorld.m), sizeof(Matrix44));    
	return TRUE;
}

/* ��ȡ��ת���� */
BOOL getSceneMxrotate(VOIDPtr pscene, LPMatrix44 matrix)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(pscene);
	if(NULL == pScene)
	    return FALSE;
	memcpy((VOIDPtr)matrix, (VOIDPtr)(pScene->m_mxRotate.m), sizeof(Matrix44));    
	return TRUE;
}

/* ��ȡ�ɻ��������� */
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
 *  @brief �����ⲿ��Ļ�����ȡ��������.�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ��ī����ͶӰֻ�и���ʱ��ת��.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] screenPt  �ⲿ��Ļ����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
		/*��ͶӰ���Ӻ������ӽ�ת��������ͬ*/
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
		/*ī����ͶӰֻ�и����ӽǣ�û�������ӽǣ�ֱ�ӷ���ת��ʧ��*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!outerScreenDPt2GeoDPtMct(screenPt, pScene->m_lfModelMatrix, pScene->invmat, pScene->innerviewport, geoPt))
				return(FALSE);	
		}else
		{
			return(FALSE);
		}
	}else{
		/*����ͶӰ��֧�֣�ֱ�ӷ���ת��ʧ��*/
		return(FALSE);
	}
	

	return(TRUE);
}

/**
 * @fn BOOL getOuterScreenPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I screenPt)
 *  @brief ���ݵ��������ȡ�ⲿ��Ļ����.�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����<-��������.
 *         �����ڲ��ӿ�����<-��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] geoPt      ��������.
 *  @param[out] screenPt  �ⲿ��Ļ����.
 *  @exception void
 *  @return 1-�ɹ�,0-ʧ��
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
		/*��ͶӰ*/
		if(!geoDPt2OuterScreenDPt(&geoPt, pScene->matrix, pScene->innerviewport, screenPt))
			return(FALSE);
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*ī����ͶӰ*/
		if(!geoDPt2OuterScreenDPtMct(&geoPt, pScene->matrix, pScene->innerviewport, screenPt))
			return(FALSE);
	}else{
		/*����ͶӰ��֧�֣�ֱ�ӷ���ת��ʧ��*/
		return(FALSE);
	}

	return(TRUE);
}

/**
 * @fn BOOL getGeoByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
 *  @brief �����ⲿ�ӿ������ȡ��������.�ⲿ�ӿ�����->�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ��ī����ͶӰֻ�и���ʱ��ת��.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] viewPt    �ⲿ�ӿ�����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
		/*��ͶӰ���Ӻ������ӽ�ת��������ͬ*/
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
		/*ī����ͶӰֻ�и����ӽǣ�û�������ӽǣ�ֱ�ӷ���ת��ʧ��*/
		if (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		{
			if(!outerViewDPt2geoDPtMct(viewPt, pScene->m_lfModelMatrix, pScene->invmat, pScene->outerviewport, pScene->innerviewport, geoPt))
				return(FALSE);
		}else
		{
			return(FALSE);
		}
	}else{
		/*����ͶӰ��֧�֣�ֱ�ӷ���ת��ʧ��*/
		return(FALSE);
	}

	return(TRUE);
}



/**
 * @fn BOOL getOuterViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
 *  @brief ���ݵ��������ȡ�ⲿ�ӿ�����.�ⲿ�ӿ�����<-�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����<-��������.
 *         �����ڲ��ӿ�����<-��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] geoPt      ��������.
 *  @param[out] viewPt    �ⲿ�ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
		/*��ͶӰ*/
		if(!geoDPt2OuterViewDPt(&geoPt, pScene->matrix, pScene->outerviewport, pScene->innerviewport, viewPt))
			return(FALSE);
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*ī����ͶӰ*/
		if(!geoDPt2OuterViewDPtMct(&geoPt, pScene->matrix, pScene->outerviewport, pScene->innerviewport, viewPt))
			return(FALSE);
	}else{
		/*����ͶӰ��֧�֣�ֱ�ӷ���ת��ʧ��*/
		return(FALSE);
	}

	return(TRUE);
}


/**
 * @fn BOOL getGeoByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
 *  @brief �����ڲ��ӿ������ȡ��������(�ڲ��ӿ�����zֵǿ��Ϊ0).�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ��ī����ͶӰֻ�и���ʱ��ת��.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] viewPt    �ⲿ�ӿ�����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
		/*��ͶӰ���Ӻ������ӽ�ת��������ͬ*/
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
		/*ī����ͶӰֻ�и����ӽǣ�û�������ӽǣ�ֱ�ӷ���ת��ʧ��*/
		if(!innerViewDPt2geoDPtMct(&viewPt_3D, pScene->invmat, pScene->innerviewport, geoPt))
			return(FALSE);
	}else{
		/*����ͶӰ��֧�֣�ֱ�ӷ���ת��ʧ��*/
		return(FALSE);
	}

	return(TRUE);
}

/**
 * @fn BOOL getInnerViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
 *  @brief ���ݵ��������ȡ�ڲ��ӿ�����(δ����zֵ).�ڲ��ӿ�����<-��������.
 *         �����ڲ��ӿ�����<-��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] geoPt      ��������.
 *  @param[out] viewPt    �ⲿ�ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
		/*��ͶӰ*/
		if(!geoDPt2InnerViewDPt(&geoPt, pScene->matrix, pScene->innerviewport, &viewPt_3D))
			return(FALSE);
		else{
			viewPt->x = (f_int32_t)viewPt_3D.x;
			viewPt->y = (f_int32_t)viewPt_3D.y;
			//viewPt_3D.z����Ҫ����
		}
	}else if (ePRJ_mercator == pScene->project_mode)
	{
		/*ī����ͶӰ*/
		if(!geoDPt2InnerViewDPtMct(&geoPt, pScene->matrix, pScene->innerviewport, &viewPt_3D))
			return(FALSE);
		else{
			viewPt->x = (f_int32_t)viewPt_3D.x;
			viewPt->y = (f_int32_t)viewPt_3D.y;
			//viewPt_3D.z����Ҫ����
		}
	}else{
		/*����ͶӰ��֧�֣�ֱ�ӷ���ת��ʧ��*/
		return(FALSE);
	}

	return(TRUE);
}


/**
 * @fn BOOL getOuterViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
 *  @brief �����ⲿ��Ļ�����ȡ�ⲿ�ӿ�����.�ⲿ�ӿ�����<-�ⲿ��Ļ����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] screenPt   �ⲿ��Ļ����.
 *  @param[out] viewPt    �ⲿ�ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
 *  @brief �����ⲿ�ӿ������ȡ�ⲿ��Ļ����.�ⲿ�ӿ�����->�ⲿ��Ļ����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] viewPt     �ⲿ�ӿ�����.
 *  @param[out] screenPt  �ⲿ��Ļ����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
 *  @brief �����ⲿ��Ļ�����ȡ�ڲ��ӿ�����.�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] screenPt   �ⲿ��Ļ����.
 *  @param[out] viewPt    �ڲ��ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
 *  @brief �����ڲ��ӿ������ȡ�ⲿ��Ļ����.�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] viewPt     �ڲ��ӿ�����.
 *  @param[out] screenPt  �ⲿ��Ļ����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
 *  @brief ���ݵ��������ȡ��������.
 *         ��������ģʽ�����ͶӰ����,���漰��ѧ����
 *  @param[in] geo_pt     ��������.
 *  @param[out] draw_pt   ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
 *  @brief �������������ȡ��������.
 *         ��������ģʽ�����ͶӰ����,���漰��ѧ����
 *  @param[in] draw_pt   ��������.
 *  @param[out] geo_pt     ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
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
