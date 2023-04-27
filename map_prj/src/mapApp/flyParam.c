#include "mapApp.h"
#include "paramGet.h"
#include "flyParam.h"

/**
 * @fn f_int32_t setPeriodParamPre(VOIDPtr render_scene)
 *  @brief 设置周期参数的前置函数，获取参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setPeriodParamPre(VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(sFail);
	takeSem(pScene->periodparaminput_sem, FOREVER_WAIT);
	return(sSuccess);
}

/**
 * @fn void setPeriodParamPro(VOIDPtr render_scene)
 *  @brief 设置周期参数的后置函数，释放参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setPeriodParamPro(VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(sFail);
	giveSem(pScene->periodparaminput_sem);
	return(sSuccess);
}

/**
 * @fn void setPosLonLat(VOIDPtr render_scene, f_float64_t longitude, f_float64_t latitude)
 *  @brief 设定位置信息的经纬度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] longitude   经度（单位：度）.
 *  @param[in] latitude    纬度（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setPosLonLat(VOIDPtr render_scene, f_float64_t longitude, f_float64_t latitude)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	//非漫游模式才接收经纬度
	if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
		&&(eROM_outroam == pScene->mdctrl_cmd_input.rom2d_mode))
	{
	    pScene->geopt_pos_input.lon  = longitude;
	    pScene->geopt_pos_input.lat  = latitude;
    }
}


/**
 * @fn void getPosLonLat(VOIDPtr render_scene, f_float64_t* longitude, f_float64_t* latitude)
 *  @brief 获取引擎内部位置信息的经纬度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
 *  @param[in] render_scene 场景句柄.
 *  @param[out] longitude   经度（单位：度）.
 *  @param[out] latitude    纬度（单位：度）.
 *  @exception void.
 *  @return void.
 * @see
 * @note
*/
void getPosLonLat(VOIDPtr render_scene, f_float64_t* longitude, f_float64_t* latitude)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
    if(NULL == pScene)
        return;

    *longitude = pScene->geopt_pos.lon;
    *latitude = pScene->geopt_pos.lat;
}

/**
 * @fn void setPosHeight(VOIDPtr render_scene, f_float64_t height)
 *  @brief 设定位置信息的高度.
           -# 在非第三方视角的情况下，该位置是视点的位置.
           -# 在三方视角的情况下，该位置是焦点的位置.
           -# 在俯视情况下，视点的高度是根据量程反算过来的. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] height   高度（单位：米）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setPosHeight(VOIDPtr render_scene, f_float64_t height)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	/*为了在俯视漫游时,高度预警仍能随高度变化,修改判断条件;*/
	/*在三维漫游关闭时,无论俯视漫游是否开启,飞机高度按实际值更新*/
	//if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
	//	&&(eROM_outroam == pScene->mdctrl_cmd_input.rom2d_mode))
	if(eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
	{    
	    pScene->geopt_pos_input.height  = height;
	}
}

/**
 * @fn void setAtdYaw(VOIDPtr render_scene, f_float64_t yaw)
 *  @brief 设定航姿信息的航向角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] yaw   航向角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdYaw(VOIDPtr render_scene, f_float64_t yaw)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode))
	{    
	    pScene->attitude_input.yaw = yaw;
	}
}

/**
 * @fn void setAtdMag(VOIDPtr render_scene, f_float64_t mag)
 *  @brief 设定航姿信息的磁差角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] mag  磁差角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdMag(VOIDPtr render_scene, f_float64_t mag)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode))
	{    
	    pScene->attitude_input.mag = mag;
	}
}

/**
 * @fn void setAtdPithch(VOIDPtr render_scene, f_float64_t pitch)
 *  @brief 设定航姿信息的俯仰角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] pitch   俯仰角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdPithch(VOIDPtr render_scene, f_float64_t pitch)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
		&&(eROM_outroam == pScene->mdctrl_cmd_input.rom2d_mode))
	{    
	    pScene->attitude_input.pitch = pitch;
	}
}

/**
 * @fn void setAtdRoll(VOIDPtr render_scene, f_float64_t roll)
 *  @brief 设定航姿信息的横滚角. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] roll   横滚角（单位：度）.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setAtdRoll(VOIDPtr render_scene, f_float64_t roll)
{
    sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
	if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
		&&(eROM_outroam == pScene->mdctrl_cmd_input.rom2d_mode))
	{    
	    pScene->attitude_input.roll = roll;
	}
}

/**
 * @fn void setRomxy(VOIDPtr render_scene, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
 *  @brief 设定漫游的视窗坐标(相对于视窗左下角).
 *  @param[in] render_scene 场景句柄.
 *  @param[in] romx,romy   漫游的视窗坐标.
 *  @param[out] romflag 漫游标志.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setRomxy(VOIDPtr render_scene, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;
    pScene->rom_pt_input.x = romx;
    pScene->rom_pt_input.y = romy;

	#if 0
    /* 如果处于漫游模式，应根据漫游位置计算地理坐标，并赋给飞机位置，漫游时就不用接收到的飞机经纬度了 */
    if((eROM_inroam == pScene->mdctrl_cmd_input.rom2d_mode) && 
       eROM_notroamed == pScene->mdctrl_cmd_input.is_romed)
    {
    	Geo_Pt_D geoPt;
    	if(getGeoByOuterViewPt(render_scene, pScene->rom_pt_input, &geoPt))
    	{
    	    pScene->geopt_pos_input.lon  = geoPt.lon;
	        pScene->geopt_pos_input.lat  = geoPt.lat;
	    }
	    else
	    {
	        //不改变位置	
	    }
	    *romflag = eROM_roamed;
    }
	#else
	    *romflag = eROM_roamed;



	#endif
}

/**
 * @fn setPeriodParam(sGLRENDERSCENE *pScene)
 *  @brief 设置周期参数，主要是飞行参数. 
 *  @param[in] pScene 场景句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证飞行参数的一致性，需要用信号量来进行保护.
*/
void setPeriodParam(sGLRENDERSCENE *pScene)
{
	if(NULL == pScene)
	    return;
	
	takeSem(pScene->periodparamset_sem, FOREVER_WAIT);

	/*飞参在俯视非漫游的时候才更新*/
	if (pScene->mdctrl_cmd.rom2d_mode == eROM_outroam)
	{
		/*非正北朝上(地图不动飞机符号动,地稳)时,引擎中飞参更新*/
		if(pScene->mdctrl_cmd.mv_mode != eMV_northupmapstatic)
		{
			pScene->geopt_pos = pScene->geopt_pos_input;
		}else
		{
		    /*正北朝上(地图不动飞机符号动,地稳)时,漫游状态由漫游变成非漫游，引擎中飞参更新，让飞机回到原始飞参位置*/
		    if((pScene->mdctrl_cmd.rom2d_mode_old != pScene->mdctrl_cmd.rom2d_mode) && (pScene->mdctrl_cmd.rom2d_mode_old == eROM_inroam))
		    {
		        pScene->geopt_pos = pScene->geopt_pos_input;
		    }

		    if(pScene->set_center_pos)
		    {
		        pScene->geopt_pos.lon = pScene->centerpt_pos.lon;
		        pScene->geopt_pos.lat = pScene->centerpt_pos.lat;
		        //pScene->set_center_pos = 0;
		        //printf("index=%d Lon=%f Lat=%f\n",pScene->scene_index,pScene->geopt_pos.lon,pScene->geopt_pos.lat);
		    }
		}
        /*任何模式，飞机符号的位置和航向都更新*/
        pScene->planesymbol_pos = pScene->geopt_pos_input;
        pScene->planesymbol_yaw = pScene->attitude_input.yaw;
	}else
	{
		/*当由非漫游切换到漫游状态时，记录当前中心点经纬高信息，用于漫游归位*/
		if (pScene->mdctrl_cmd.rom2d_mode_old == eROM_outroam)
		{
			pScene->roam_start_pt_pos = pScene->geopt_pos;
		}
	}
	/*为了保证俯视漫游下高度预警生效,高度始终用最新的飞参高度*/
	pScene->geopt_pos.height = pScene->geopt_pos_input.height;

	/*只有在俯视视角和缩放视角下,才响应地图运动模式的控制命令*/
	if( (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW) 
	 || (pScene->camctrl_param.view_type == eVM_SCALE_VIEW) )
	{
		if(pScene->mdctrl_cmd.mv_mode == eMV_headup)
		{
			pScene->attitude.yaw = pScene->attitude_input.yaw;
		}else{
			pScene->attitude.yaw = pScene->attitude_input.mag;
		}
	}else{
		pScene->attitude.yaw = pScene->attitude_input.yaw;	
	}
	pScene->attitude.pitch = pScene->attitude_input.pitch;
	pScene->attitude.roll = pScene->attitude_input.roll;

	giveSem(pScene->periodparamset_sem);
}
