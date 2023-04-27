/**
 * @file auxNavInfo.h
 * @brief ���ļ��ṩ����������Ϣ��ʾ�������õĽӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-10-24
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _auxNavInfo_h_ 
#define _auxNavInfo_h_ 

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
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
BOOL auxNavInfoDataInit(VOIDPtr render_scene, f_char_t *data_path);

/**
 * @fn void setZplParam(VOIDPtr render_scene, sZplParam zplprarm)
 *  @brief �趨0����������ʾ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] zplprarm 0�������߲���.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setZplParam(VOIDPtr render_scene, sZplParam zplprarm);

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
void setCpsParam(VOIDPtr render_scene, sCpsParam cpsprarm);

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
void setPrefileParam(VOIDPtr render_scene, sPrefileParam  prefileprarm);

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
void setPrefileDisplay(VOIDPtr render_scene, f_int32_t isdisplay);

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
void setPrefileDetectDistance(VOIDPtr render_scene, f_float64_t hDetectDistance);

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
	f_uint8_t prefile_color[6][4], f_int32_t s_winHeiMeter);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
