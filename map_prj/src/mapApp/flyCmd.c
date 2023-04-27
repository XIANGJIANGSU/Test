#include "../projection/coord.h"
#include "mapApp.h"
#include "flyCmd.h"

/**
 * @fn f_int32_t setEventParamPre(VOIDPtr render_scene)
 *  @brief �����¼�������ǰ�ú�������ȡ�������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setEventParamPre(VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(sFail);
	takeSem(pScene->eventparaminput_sem, FOREVER_WAIT);
	return(sSuccess);
}

/**
 * @fn f_int32_t setEventParamPro(VOIDPtr render_scene)
 *  @brief �����¼������ĺ��ú������ͷŲ������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setEventParamPro(VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(sFail);
	giveSem(pScene->eventparaminput_sem);
	return(sSuccess);
}

/**
 * @fn void setModeCor(VOIDPtr render_scene, f_int32_t cormode)
 *  @brief ������ɫģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] cormode ��ɫģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeCor(VOIDPtr render_scene, f_int32_t cormode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.cor_mode = cormode;
}

/**
 * @fn void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
 *  @brief ������ҹģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] dnmode ��ҹģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.dn_mode = dnmode;
}

/**
 * @fn void setModeSVS(VOIDPtr render_scene, f_int32_t svsmode)
 *  @brief ����SVS ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] svsmode SVS ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSVS(VOIDPtr render_scene, f_int32_t svsmode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.svs_mode= svsmode;
}

/**
 * @fn setMode3DMD(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief �����Ƿ���ʾ3Dģ��
 *  @param[in] render_scene �������.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode3DMD(VOIDPtr render_scene, f_int32_t isdisplay)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.is_3dplane_display = isdisplay;
}

/**
 * @fn void setModeMv(VOIDPtr render_scene, f_int32_t mvmode)
 *  @brief �����˶�ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] mvmode �˶�ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMv(VOIDPtr render_scene, f_int32_t mvmode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.mv_mode = mvmode;
}


/**
 * @fn void setModePSDisplay(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief �����Ƿ���ʾ�ɻ�����.
 *  @param[in] render_scene �������.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModePSDisplay(VOIDPtr render_scene, f_int32_t isdisplay)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.plane_symbol_display = isdisplay;
}

/**
 * @fn void setModeFrz(VOIDPtr render_scene, f_int32_t frzmode)
 *  @brief ���ö���ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] frzmode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeFrz(VOIDPtr render_scene, f_int32_t frzmode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.frz_mode = frzmode;
}

/**
 * @fn void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
 *  @brief ��������ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] rommode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.rom_mode = rommode;
}

/**
 * @fn void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
 *  @brief ��������ģʽ.������
 *  @param[in] render_scene �������.
 *  @param[in] rommode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode2dRm(VOIDPtr render_scene, f_int32_t rommode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;

	if(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)	
	{
		pScene->mdctrl_cmd_input.rom2d_mode = rommode;
	}
	else
	{
		pScene->mdctrl_cmd_input.rom2d_mode = eROM_outroam;
	}
}

/**
 * @fn void setFlagRm(VOIDPtr render_scene, f_int32_t romflag)
 *  @brief �������α�־.
 *  @param[in] render_scene �������.
 *  @param[in] romflag ���α�־.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setFlagRm(VOIDPtr render_scene, f_int32_t romflag)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->is_romed = romflag;
}

/**
 * @fn void setFlagRmHm(VOIDPtr render_scene, f_int32_t romhomeflag)
 *  @brief �������ι�λ��־.
 *  @param[in] render_scene �������.
 *  @param[in] romflag ���ι�λ��־.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setFlagRmHm(VOIDPtr render_scene, f_int32_t romhomeflag)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->is_rom_homed = romhomeflag;
}

/**
 * @fn void setModeTwarn(VOIDPtr render_scene, f_int32_t terwarnmode)
 *  @brief ���õ��θ澯ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] terwarnmode ���θ澯ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeTwarn(VOIDPtr render_scene, f_int32_t terwarnmode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.terwarn_mode = terwarnmode;
}

/**
 * @fn void setModeSce(VOIDPtr render_scene, f_int32_t scenemode)
 *  @brief �������ó�����ʾģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] scenemode ������ʾģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSce(VOIDPtr render_scene, f_int32_t scenemode)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;

	pScene->mdctrl_cmd_input.scene_mode = scenemode;
}

/**
 * @fn void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
 *  @brief ���ö���άģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] dnmode ����άģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode23D(VOIDPtr render_scene, f_int32_t mode23d)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.map2dor3d = mode23d;
}

/**
 * @fn void setModeDisplay(VOIDPtr render_scene, f_int32_t modedisplay)
 *  @brief ���õ�ͼ��ͼ����ӷ�ʽ
 *  @param[in] render_scene �������.
 *  @param[in] modedisplay ͼ�����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDisplay(VOIDPtr render_scene, f_int32_t modedisplay)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.mapDisplay_mode = modedisplay;
}

/**
 * @fn void setModeClose3d(VOIDPtr render_scene, f_int32_t modedisplay)
 *  @brief ������ά��ͼ�Ƿ�����
 *  @param[in] render_scene �������.
 *  @param[in] modedisplay 0 : �����Σ�1 :����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeClose3d(VOIDPtr render_scene, f_int32_t modedisplay)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.close3d_mode = modedisplay;
}

/**
 * @fn void setCamCtlParamViewRes(VOIDPtr render_scene, f_float32_t viewres)
 *  @brief ����������Ʋ����е���ʾ�ֱ���.
 *  @param[in] render_scene �������.
 *  @param[in] viewres �ֱ��ʣ���/���أ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewRes(VOIDPtr render_scene, f_float32_t viewres)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->camctrl_param_input.view_res = viewres;
}

/**
 * @fn void setCamCtlParamViewAngle(VOIDPtr render_scene, f_float32_t viewangle)
 *  @brief ��������������Ʋ����е�ƽ��ͷ�ӽ�.
 *  @param[in] render_scene �������.
 *  @param[in] viewangle �ӽǣ���λ���ȣ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewAngle(VOIDPtr render_scene, f_float32_t viewangle)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->camctrl_param_input.view_angle = viewangle;
}

/**
 * @fn void setCamCtlParamCmpRd(VOIDPtr render_scene, f_int32_t radius)
 *  @brief ����������Ʋ����еĿ̶Ȼ��뾶.
 *  @param[in] render_scene �������.
 *  @param[in] radius �̶Ȼ��뾶�����أ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamCmpRd(VOIDPtr render_scene, f_int32_t radius)
{
	sGLRENDERSCENE *pscene = NULL;
    pscene = scenePtrValid(render_scene);
	if(NULL == pscene)
	    return;
	    
	pscene->camctrl_param_input.compass_radius = radius;
}

/**
 * @fn void setCamCtlParamRtCent(VOIDPtr render_scene, f_int32_t xcenter, f_int32_t ycenter)
 *  @brief ����������Ʋ����еĵ�ͼ��ת����.
 *  @param[in] render_scene �������.
 *  @param[in] xcenter��ycenter ��ͼ��ת���ĵ�λ�ã����أ�������ⲿ�Ӵ����½ǣ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamRtCent(VOIDPtr render_scene, f_int32_t xcenter, f_int32_t ycenter)
{
	PT_2I outer_screen_pt, outer_view_pt, inner_screen_pt, inner_view_pt;
    sGLRENDERSCENE *pscene = NULL;
	pscene = scenePtrValid(render_scene);
	if(NULL == pscene)
	    return;
	    
	outer_view_pt.x = xcenter;
	outer_view_pt.y = ycenter;
    if(outerViewPt2outerScreenPt(pscene->outerviewport, outer_view_pt, &outer_screen_pt))
    {
    	if(outerScreenPt2innerScreenPt(outer_screen_pt, &inner_screen_pt))
    	{
    		if(innerScreenPt2innerViewPt(pscene->innerviewport, inner_screen_pt, &inner_view_pt))
    		{
    			pscene->camctrl_param_input.rotate_center.x = inner_view_pt.x;
	            pscene->camctrl_param_input.rotate_center.y = inner_view_pt.y;
    		}
    		else
    		{
    		    pscene->camctrl_param_input.rotate_center.x = xcenter;
	            pscene->camctrl_param_input.rotate_center.y = ycenter;
	            DEBUG_PRINT("innerScreenPt2innerViewPt error.");
    		}
    	}
    	else
    	{
    		pscene->camctrl_param_input.rotate_center.x = xcenter;
	        pscene->camctrl_param_input.rotate_center.y = ycenter;
	        DEBUG_PRINT("outerScreenPt2innerScreenPt error.");
    	}
    }
    else
    {
    	pscene->camctrl_param_input.rotate_center.x = xcenter;
	    pscene->camctrl_param_input.rotate_center.y = ycenter;
	    DEBUG_PRINT("outerViewPt2outerScreenPt error.");
    }
}

/**
 * @fn void setCamCtlParamViewType(VOIDPtr render_scene, f_int32_t vtype)
 *  @brief �����ӽ�ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] vtype �ӽ�����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewType(VOIDPtr render_scene, f_int32_t vtype)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->camctrl_param_input.view_type = vtype;
}

/**
 * @fn setEventParam(sGLRENDERSCENE *pScene)
 *  @brief �����¼���������Ҫ�ǿ�������. 
 *  @param[in] pScene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤���в�����һ���ԣ���Ҫ���ź��������б���.
*/
void setEventParam(sGLRENDERSCENE *pScene)
{
	if(NULL == pScene)
	    return;
	
	takeSem(pScene->eventparamset_sem, FOREVER_WAIT);
	/*��¼��һ֡������״̬*/
	pScene->mdctrl_cmd_input.rom2d_mode_old = pScene->mdctrl_cmd.rom2d_mode;
	pScene->mdctrl_cmd     = pScene->mdctrl_cmd_input;
    pScene->camctrl_param  = pScene->camctrl_param_input;
    pScene->auxnavinfo_param = pScene->auxnavinfo_param_input;
	giveSem(pScene->eventparamset_sem);
}

/**
 * @fn void setColorAndLayermaskCfgIndex(VOIDPtr render_scene, f_int32_t index)
 *  @brief ����ѡ�õ���ɫ�ͷ�ӵ�����ļ�������
 *  @param[in] render_scene �������.
 *  @param[in] index��0-7�����֧��8��
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setColorAndLayermaskCfgIndex(VOIDPtr render_scene, f_int32_t index)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.color_layermask_cfg_index = index;
}


/**
 * @fn void setAreaBaDraw(VOIDPtr render_scene, f_int32_t bDraw)
 *  @brief ����ʸ���������Ƿ����
 *  @param[in] render_scene �������.
 *  @param[in] bDraw 0-������ 1-����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAreaBaDraw(VOIDPtr render_scene, f_int32_t bDraw)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.area_ba_draw = bDraw;
}

/**
 * @fn void setSeaMapDraw(VOIDPtr render_scene, f_int32_t bDraw)
 *  @brief ���ú�ͼ�Ƿ����
 *  @param[in] render_scene �������.
 *  @param[in] bDraw 0-������ 1-����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setSeaMapDraw(VOIDPtr render_scene, f_int32_t bDraw)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.sea_map_draw = bDraw;
}


/**
 * @fn void setTextDrawLevel(VOIDPtr render_scene, f_int32_t level)
 *  @brief ���õ���ע�Ǻͷ�����ʾ��λ
 *  @param[in] render_scene �������.
 *  @param[in] level 0-����ʾ�κ�ע�Ǻͷ��� 1-ֻ��ʾʡ����к��еȳ��еĵ���ע�Ǻͷ��� 2-������ʾ���е�ע�Ǻͷ���
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setTextDrawLevel(VOIDPtr render_scene, f_int32_t level)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	    
	pScene->mdctrl_cmd_input.text_draw_level = level;
}