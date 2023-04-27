#ifndef __compassRose_H__
#define __compassRose_H__

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn f_int32_t cpsListCreate(f_float32_t lb_x, f_float32_t lb_y, f_float32_t rt_x, f_float32_t rt_y)
 *  @brief �������̵���ʾ�б�.
 *  @param[in] lb_x, lb_y, rt_x, rt_y  ���̵�λ��(������������ĵ�). 
 *  @exception void.
 *  @return f_int32_t ��ʾ�б�ID.
 * @see 
 * @note
*/
f_int32_t cpsListCreate(f_float32_t lb_x, f_float32_t lb_y, f_float32_t rt_x, f_float32_t rt_y);

/**
 * @fn f_int32_t createCompassRoseTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
 *  @brief ��������.
 *  @param[in] textFileName ��������ͼ����ȫ�ļ���.
 *  @param[out] ptexture_id     ���̵�����ID. 
 *  @exception -1:�����ļ�����ʧ�ܣ�-2:������ʧ��.
 *  @return f_int32_t �����ɹ���־.
 *  @retval  0 �����ɹ�.
 *  @retval -1 ����ʧ��.
 * @see 
 * @note
*/
f_int32_t createCompassRoseTexture(f_int8_t* textFileName, f_int32_t *ptexture_id);

/**
 * @fn f_int32_t renderCompassRose(f_float32_t yaw, f_float32_t view_lb_x, f_float32_t view_lb_y, \n
                                   f_float32_t cent_x, f_float32_t cent_y, f_float32_t width, f_float32_t height,\n
                                   sColor3f color, f_int32_t list_id, f_int32_t texture_id)
 *  @brief ��������.
 *  @param[in] yaw     ����. 
 *  @param[in] cent_x, cent_y     �������ĵ�λ�ã�����ڵ�ǰ�ӿڵ����½ǣ�
 *  @param[in] width, height      ���̿��
 *  @param[in] color   ��ɫ. 
 *  @param[in] list_id ������ʾ�б�ID��. 
 *  @param[in] texture_id ��������ID��.  
 *  @exception void.
 *  @return f_int32_t �����ɹ���־.
 *  @retval  0 �����ɹ�.
 *  @retval -1 ����ʧ��.
 * @see 
 * @note
*/
f_int32_t renderCompassRose(f_float32_t yaw, f_float32_t view_lb_x, f_float32_t view_lb_y,
                            f_float32_t cent_x, f_float32_t cent_y, f_float32_t width, f_float32_t height,
                            sColor3f color, f_int32_t list_id, f_int32_t texture_id);

#ifdef __cplusplus
}
#endif

#endif