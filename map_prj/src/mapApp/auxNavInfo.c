#include "../engine/zeroPitchLine.h"
#include "../engine/compassRose.h"
#include "../projection/coord.h"
#include "auxNavInfo.h"
#include "appHead.h"
#include "mapApp.h"

/* 0�����������ݳ�ʼ�� */
/* 0-��ʼ���ɹ�  -1-��ʼ��ʧ��*/
static f_int32_t zplDataInit(sGLRENDERSCENE *pScene, f_char_t *data_path)
{
	f_int32_t ret = 0;
	f_char_t buf[256] = {0};
	f_int32_t texture_id = 0;
	sprintf(buf, "%s/zplT.bmp", data_path);
#ifndef _JM7200_
	/* zplT.bmp�ߴ���8192*64��7200֧�ֵ��������ߴ�Ϊ4096��7200�²�����������*/
	ret = createZeroPitchLineTexture(buf, &texture_id);
#endif
	pScene->auxnav_list.zpl_list.texture_id = texture_id;
	return ret;
}

/* �������ݳ�ʼ�� */
/* 0-��ʼ���ɹ�  -1-��ʼ��ʧ��*/
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
 *  @brief ����������Ϣ���ݳ�ʼ��.
 *  @param[in] scene �ӿ����ڵĳ������.
 *  @param[in] data_path �ӿڶ�Ӧ����·���ĸ�Ŀ¼.
 *  @exception void
 *  @return BOOL,��ʼ���ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
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
 *  @brief �趨0���������Ƿ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] zplprarm 0�������߲���.
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
 *  @brief �趨������ʾ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] zplprarm ���̲���.
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
 *  @brief �趨����ͼ�Ƿ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] prefileprarm ����ͼ����.
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
 *  @brief �����Ƿ���ʾ����ͼ
 *  @param[in] render_scene �������.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ
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
 *  @brief ��������ͼˮƽ̽�����
 *  @param[in] render_scene �������.
 *  @param[in] hDetectDistance ˮƽ̽�����
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
 *  @brief ��������ͼ��ֱ��������ͼVSD�Ĵ�ֱ�����������ʵ�ʾ���(��)���߶�ɫ����ز���
*   @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] color_num ɫ������,��ΧΪ[2,6]
 *  @param[in] prefile_height[5] ɫ������ĸ߶�,ÿ��ɫ����Ӧһ�����䷶Χ,5��ֵ��Ӧ���6������
 *  @param[in] prefile_color[6][4] ɫ������ɫRGBA
 *  @param[in] s_winHeiMeter ��ֱ�����������ʵ�ʾ���(��)���������0
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