#include "mapApp.h"
#include "paramGet.h"
#include "flyParam.h"

/**
 * @fn f_int32_t setPeriodParamPre(VOIDPtr render_scene)
 *  @brief �������ڲ�����ǰ�ú�������ȡ�������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
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
 *  @brief �������ڲ����ĺ��ú������ͷŲ������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
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
 *  @brief �趨λ����Ϣ�ľ�γ��.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��. 
 *  @param[in] render_scene �������.
 *  @param[in] longitude   ���ȣ���λ���ȣ�.
 *  @param[in] latitude    γ�ȣ���λ���ȣ�.
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
	//������ģʽ�Ž��վ�γ��
	if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
		&&(eROM_outroam == pScene->mdctrl_cmd_input.rom2d_mode))
	{
	    pScene->geopt_pos_input.lon  = longitude;
	    pScene->geopt_pos_input.lat  = latitude;
    }
}


/**
 * @fn void getPosLonLat(VOIDPtr render_scene, f_float64_t* longitude, f_float64_t* latitude)
 *  @brief ��ȡ�����ڲ�λ����Ϣ�ľ�γ��.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] render_scene �������.
 *  @param[out] longitude   ���ȣ���λ���ȣ�.
 *  @param[out] latitude    γ�ȣ���λ���ȣ�.
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
 *  @brief �趨λ����Ϣ�ĸ߶�.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
           -# �ڸ�������£��ӵ�ĸ߶��Ǹ������̷��������. 
 *  @param[in] render_scene �������.
 *  @param[in] height   �߶ȣ���λ���ף�.
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
	/*Ϊ���ڸ�������ʱ,�߶�Ԥ��������߶ȱ仯,�޸��ж�����;*/
	/*����ά���ιر�ʱ,���۸��������Ƿ���,�ɻ��߶Ȱ�ʵ��ֵ����*/
	//if((eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
	//	&&(eROM_outroam == pScene->mdctrl_cmd_input.rom2d_mode))
	if(eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)
	{    
	    pScene->geopt_pos_input.height  = height;
	}
}

/**
 * @fn void setAtdYaw(VOIDPtr render_scene, f_float64_t yaw)
 *  @brief �趨������Ϣ�ĺ����. 
 *  @param[in] render_scene �������.
 *  @param[in] yaw   ����ǣ���λ���ȣ�.
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
 *  @brief �趨������Ϣ�ĴŲ��. 
 *  @param[in] render_scene �������.
 *  @param[in] mag  �Ų�ǣ���λ���ȣ�.
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
 *  @brief �趨������Ϣ�ĸ�����. 
 *  @param[in] render_scene �������.
 *  @param[in] pitch   �����ǣ���λ���ȣ�.
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
 *  @brief �趨������Ϣ�ĺ����. 
 *  @param[in] render_scene �������.
 *  @param[in] roll   ����ǣ���λ���ȣ�.
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
 *  @brief �趨���ε��Ӵ�����(������Ӵ����½�).
 *  @param[in] render_scene �������.
 *  @param[in] romx,romy   ���ε��Ӵ�����.
 *  @param[out] romflag ���α�־.
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
    /* �����������ģʽ��Ӧ��������λ�ü���������꣬�������ɻ�λ�ã�����ʱ�Ͳ��ý��յ��ķɻ���γ���� */
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
	        //���ı�λ��	
	    }
	    *romflag = eROM_roamed;
    }
	#else
	    *romflag = eROM_roamed;



	#endif
}

/**
 * @fn setPeriodParam(sGLRENDERSCENE *pScene)
 *  @brief �������ڲ�������Ҫ�Ƿ��в���. 
 *  @param[in] pScene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤���в�����һ���ԣ���Ҫ���ź��������б���.
*/
void setPeriodParam(sGLRENDERSCENE *pScene)
{
	if(NULL == pScene)
	    return;
	
	takeSem(pScene->periodparamset_sem, FOREVER_WAIT);

	/*�ɲ��ڸ��ӷ����ε�ʱ��Ÿ���*/
	if (pScene->mdctrl_cmd.rom2d_mode == eROM_outroam)
	{
		/*����������(��ͼ�����ɻ����Ŷ�,����)ʱ,�����зɲθ���*/
		if(pScene->mdctrl_cmd.mv_mode != eMV_northupmapstatic)
		{
			pScene->geopt_pos = pScene->geopt_pos_input;
		}else
		{
		    /*��������(��ͼ�����ɻ����Ŷ�,����)ʱ,����״̬�����α�ɷ����Σ������зɲθ��£��÷ɻ��ص�ԭʼ�ɲ�λ��*/
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
        /*�κ�ģʽ���ɻ����ŵ�λ�úͺ��򶼸���*/
        pScene->planesymbol_pos = pScene->geopt_pos_input;
        pScene->planesymbol_yaw = pScene->attitude_input.yaw;
	}else
	{
		/*���ɷ������л�������״̬ʱ����¼��ǰ���ĵ㾭γ����Ϣ���������ι�λ*/
		if (pScene->mdctrl_cmd.rom2d_mode_old == eROM_outroam)
		{
			pScene->roam_start_pt_pos = pScene->geopt_pos;
		}
	}
	/*Ϊ�˱�֤���������¸߶�Ԥ����Ч,�߶�ʼ�������µķɲθ߶�*/
	pScene->geopt_pos.height = pScene->geopt_pos_input.height;

	/*ֻ���ڸ����ӽǺ������ӽ���,����Ӧ��ͼ�˶�ģʽ�Ŀ�������*/
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
