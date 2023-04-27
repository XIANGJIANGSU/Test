/**
 * @file flyParam.h
 * @brief ���ļ��ṩ��ͼ���в������õĽӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _FLY_PARAM_h_ 
#define _FLY_PARAM_h_ 

#include "../define/mbaseType.h"
#include "appHead.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
f_int32_t setPeriodParamPre(VOIDPtr render_scene);

/**
 * @fn f_int32_t setPeriodParamPro(VOIDPtr render_scene)
 *  @brief �������ڲ����ĺ��ú������ͷŲ������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setPeriodParamPro(VOIDPtr render_scene);

/**
 * @fn setPeriodParam(sGLRENDERSCENE *pScene)
 *  @brief �������ڲ�������Ҫ�Ƿ��в���. 
 *  @param[in] pScene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤���в�����һ���ԣ���Ҫ���ź��������б���.
*/
void setPeriodParam(sGLRENDERSCENE *pScene);

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
void setPosLonLat(VOIDPtr render_scene, f_float64_t longitude, f_float64_t latitude);


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
void getPosLonLat(VOIDPtr render_scene, f_float64_t* longitude, f_float64_t* latitude);

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
void setPosHeight(VOIDPtr render_scene, f_float64_t height);

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
void setAtdYaw(VOIDPtr render_scene, f_float64_t yaw);


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
void setAtdMag(VOIDPtr render_scene, f_float64_t mag);

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
void setAtdPithch(VOIDPtr render_scene, f_float64_t pitch);

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
void setAtdRoll(VOIDPtr render_scene, f_float64_t roll);

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
void setRomxy(VOIDPtr render_scene, f_int32_t romx, f_int32_t romy, f_int32_t *romflag);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
