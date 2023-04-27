/**
 * @file terWarnApi.h
 * @brief ���ļ��ṩ��ͼ������θ澯�������߶ȡ���ɫ�����õ��ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-06-20
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _TERWARN_API_h_ 
#define _TERWARN_API_h_ 

#include "common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setTerWarningBegin(VOIDPtr punit, f_int32_t warning_layers)
 *  @brief ���õ��θ澯������ǰ�ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] warning_layers ���θ澯����.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setTerWarningBegin(VOIDPtr punit, f_int32_t warning_layers);

/**
 * @fn f_int32_t setTerWarningEnd(VOIDPtr punit)
 *  @brief �����¼������ĺ��ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setTerWarningEnd(VOIDPtr punit);

/**
 * @fn void setTerSafe(VOIDPtr punit, f_int32_t index, f_float32_t height, \n
                       f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief ���ð�ȫ�߶ȣ������ε��ڰ�ȫ�߶�ʱ����red��green��blue����Ⱦ. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �㼶����.
 *  @param[in] height �߶ȣ����θ߶��� �ɻ��߶�+height ����ʱ��red��green��blue����Ⱦ.
 *  @param[in] red    ��ɫ����.
 *  @param[in] green  ��ɫ����.
 *  @param[in] blue   ��ɫ����.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setTerSafe(VOIDPtr punit, f_int32_t index, f_float32_t height, 
                f_int32_t red, f_int32_t green, f_int32_t blue); 

/**
 * @fn void setTerDangerous(VOIDPtr punit, f_int32_t index, f_float32_t height, \n
                            f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief ����Σ�ո߶ȣ������θ���Σ�ո߶�ʱ����red��green��blue����Ⱦ. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �㼶����.
 *  @param[in] height �߶ȣ����θ߶ȸ��� �ɻ��߶�+height ʱ��red��green��blue����Ⱦ.
 *  @param[in] red    ��ɫ����.
 *  @param[in] green  ��ɫ����.
 *  @param[in] blue   ��ɫ����.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setTerDangerous(VOIDPtr punit, f_int32_t index, f_float32_t height,
                     f_int32_t red, f_int32_t green, f_int32_t blue);

/**
 * @fn void setTerWarning(VOIDPtr punit, f_int32_t index, f_float32_t height_min, f_float32_t height_max, \n
                          f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief ���õ��θ澯��������������(�ɻ��߶�+height_min)��(�ɻ��߶�+height_max)����ʱ����red��green��blue����Ⱦ. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �㼶����.
 *  @param[in] height_min �߶ȵ�ֵ.
 *  @param[in] height_max �߶ȸ�ֵ.
 *  @param[in] red    ��ɫ����.
 *  @param[in] green  ��ɫ����.
 *  @param[in] blue   ��ɫ����.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setTerWarning(VOIDPtr punit, f_int32_t index, f_float32_t height_min, f_float32_t height_max,
                   f_int32_t red, f_int32_t green, f_int32_t blue);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 