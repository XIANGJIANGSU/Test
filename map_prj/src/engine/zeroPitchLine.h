#ifndef __ZEROPITCHLINE_H__
#define __ZEROPITCHLINE_H__

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn f_int32_t zplListCreate(void)
 *  @brief ����0�������ߵ���ʾ�б�.
 *  @param[in] void.   
 *  @exception void.
 *  @return f_int32_t ��ʾ�б�ID.
 * @see 
 * @note
*/
f_int32_t zplListCreate(void);

/**
 * @fn f_int32_t createZeroPitchLineTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
 *  @brief ����0�������ߵ�����.
 *  @param[in] textFileName 0������������ͼ����ȫ�ļ���.
 *  @param[out] ptexture_id  0�������ߵ�����ID. 
 *  @exception void.
 *  @return f_int32_t �����ɹ���־.
 *  @retval  0 �����ɹ�.
 *  @retval -1 ����ʧ��.
 * @see 
 * @note
*/
f_int32_t createZeroPitchLineTexture(f_int8_t* textFileName, f_int32_t *ptexture_id);

/**
 * @fn f_int32_t renderZeroPitchLine(Matrix44 mxWorld, f_int32_t list_id, f_int32_t texture_id)
 *  @brief ����0��������.
 *  @param[in] mxWorld ������ģ�Ӿ���. 
 *  @param[in] color   ��ɫ. 
 *  @param[in] list_id 0����������ʾ�б�ID��. 
 *  @param[in] texture_id 0������������ID��.  
 *  @exception void.
 *  @return f_int32_t �����ɹ���־.
 *  @retval  0 �����ɹ�.
 *  @retval -1 ����ʧ��.
 * @see 
 * @note
*/
f_int32_t renderZeroPitchLine(Matrix44 mxWorld, sColor3f color, f_int32_t list_id, f_int32_t texture_id);

#ifdef __cplusplus
}
#endif

#endif