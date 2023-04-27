/**
 * @file initData.h
 * @brief ���ļ��ṩ��ͼ���ݳ�ʼ���Ľӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _INIT_DATA_h_ 
#define _INIT_DATA_h_ 

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

/* �Ƿ�ʹ��DDS���� */
#define USE_DDS_TEXTURE
//#undef USE_DDS_TEXTURE

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn int initMapData(char *data_path)
 *  @brief ��ͼ���ݳ�ʼ��.
 *  @param[in] render_scene �������.
 *  @param[in] data_path ��ͼ����·���ĸ�Ŀ¼.
 *  @exception void
 *  @return int, ��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1: �������Ϊ��,ʧ��.
 * @see 
 * @note
*/
int initMapData(f_char_t *data_path);

/**
 * @fn BOOL createTtfFont(f_char_t *font_path, f_int32_t font_size, f_int32_t edge_size,\n
                          f_float32_t color_font[4], f_float32_t color_edge[4])
 *  @brief ���崴��.
 *  @param[in] scene �ӿ����ڵĳ������.
 *  @param[in] font_path �ֿ��ļ�·���������ļ�����.
 *  @param[in] font_file �ֿ��ļ���.
 *  @param[in] font_size �����С.
 *  @param[in] edge_size �����Ե��С.
 *  @param[in] color_font ������ɫ.
 *  @param[in] color_edge �߿���ɫ.
 *  @exception void
 *  @return BOOL,���崴���ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
 * @see 
 * @note 
*/
BOOL createTtfFont(f_char_t *font_path, f_char_t *font_file, 
                   f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4]);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
