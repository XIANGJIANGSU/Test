/**
 * @file flyParamApi.h
 * @brief ���ļ��ṩ��ͼ������в������õ��ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-05-11
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _FLYPARAM_API_h_ 
#define _FLYPARAM_API_h_ 

#include "common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn void setPeriodParamBegin(VOIDPtr punit)
 *  @brief �������ڲ�����ǰ�ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setPeriodParamBegin(VOIDPtr punit);

/**
 * @fn f_int32_t setPeriodParamEnd(VOIDPtr render_scene)
 *  @brief �������ڲ����ĺ��ú���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setPeriodParamEnd(VOIDPtr punit);

/**
 * @fn void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude)
 *  @brief �趨λ����Ϣ�ľ�γ��.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] longitude   ���ȣ���λ���ȣ���Χ-180~180��.
 *  @param[in] latitude    γ�ȣ���λ���ȣ���Χ-90~90��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude);

/**
 * @fn void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude)
 *  @brief ��ȡλ����Ϣ�ľ�γ��.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] longitude   ���ȣ���λ���ȣ���Χ-180~180��.
 *  @param[in] latitude    γ�ȣ���λ���ȣ���Χ-90~90��.
 *  @exception void
 *  @return void
 * @see
 * @note
*/
void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude);

/**
 * @fn void setPositionHeight(VOIDPtr punit, f_float64_t height)
 *  @brief �趨λ����Ϣ�ĸ߶�.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] height   �߶ȣ���λ���ף�.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setPositionHeight(VOIDPtr punit, f_float64_t height);

/**
 * @fn void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw)
 *  @brief �趨������Ϣ�ĺ����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] yaw   ����ǣ���λ���ȣ���Χ-180~180��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw);

/**
 * @fn void setAttitudeMag(VOIDPtr punit, f_float64_t mag)
 *  @brief �趨������Ϣ�ĴŲ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] mag   �Ų�ǣ���λ���ȣ���Χ-180~180��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeMag(VOIDPtr punit, f_float64_t mag);

/**
 * @fn void setAttitudePitch(VOIDPtr punit, f_float64_t pitch)
 *  @brief �趨������Ϣ�ĸ�����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] pitch   �����ǣ���λ���ȣ���Χ-90~90��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudePitch(VOIDPtr punit, f_float64_t pitch);

/**
 * @fn void setAttitudeRoll(VOIDPtr punit, f_float64_t roll)
 *  @brief �趨������Ϣ�ĺ����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] roll   ����ǣ���λ���ȣ���Χ-180~180��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeRoll(VOIDPtr punit, f_float64_t roll);

/**
 * @fn void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
 *  @brief �趨���ε��Ӵ�����(������Ӵ����½�).
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] romx,romy   ���ε��Ӵ�����.
 *  @param[out] romflag   ���α�־.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
