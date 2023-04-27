#ifndef __ZEROPITCHLINE_H__
#define __ZEROPITCHLINE_H__

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn f_int32_t zplListCreate(void)
 *  @brief 创建0俯仰角线的显示列表.
 *  @param[in] void.   
 *  @exception void.
 *  @return f_int32_t 显示列表ID.
 * @see 
 * @note
*/
f_int32_t zplListCreate(void);

/**
 * @fn f_int32_t createZeroPitchLineTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
 *  @brief 创建0俯仰角线的纹理.
 *  @param[in] textFileName 0俯仰角线纹理图的完全文件名.
 *  @param[out] ptexture_id  0俯仰角线的纹理ID. 
 *  @exception void.
 *  @return f_int32_t 创建成功标志.
 *  @retval  0 创建成功.
 *  @retval -1 创建失败.
 * @see 
 * @note
*/
f_int32_t createZeroPitchLineTexture(f_int8_t* textFileName, f_int32_t *ptexture_id);

/**
 * @fn f_int32_t renderZeroPitchLine(Matrix44 mxWorld, f_int32_t list_id, f_int32_t texture_id)
 *  @brief 绘制0俯仰角线.
 *  @param[in] mxWorld 场景的模视矩阵. 
 *  @param[in] color   颜色. 
 *  @param[in] list_id 0俯仰角线显示列表ID号. 
 *  @param[in] texture_id 0俯仰角线纹理ID号.  
 *  @exception void.
 *  @return f_int32_t 创建成功标志.
 *  @retval  0 创建成功.
 *  @retval -1 创建失败.
 * @see 
 * @note
*/
f_int32_t renderZeroPitchLine(Matrix44 mxWorld, sColor3f color, f_int32_t list_id, f_int32_t texture_id);

#ifdef __cplusplus
}
#endif

#endif