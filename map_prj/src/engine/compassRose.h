#ifndef __compassRose_H__
#define __compassRose_H__

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn f_int32_t cpsListCreate(f_float32_t lb_x, f_float32_t lb_y, f_float32_t rt_x, f_float32_t rt_y)
 *  @brief 创建罗盘的显示列表.
 *  @param[in] lb_x, lb_y, rt_x, rt_y  罗盘的位置(相对于罗盘中心点). 
 *  @exception void.
 *  @return f_int32_t 显示列表ID.
 * @see 
 * @note
*/
f_int32_t cpsListCreate(f_float32_t lb_x, f_float32_t lb_y, f_float32_t rt_x, f_float32_t rt_y);

/**
 * @fn f_int32_t createCompassRoseTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
 *  @brief 创建罗盘.
 *  @param[in] textFileName 罗盘纹理图的完全文件名.
 *  @param[out] ptexture_id     罗盘的纹理ID. 
 *  @exception -1:纹理文件加载失败，-2:纹理创建失败.
 *  @return f_int32_t 创建成功标志.
 *  @retval  0 创建成功.
 *  @retval -1 创建失败.
 * @see 
 * @note
*/
f_int32_t createCompassRoseTexture(f_int8_t* textFileName, f_int32_t *ptexture_id);

/**
 * @fn f_int32_t renderCompassRose(f_float32_t yaw, f_float32_t view_lb_x, f_float32_t view_lb_y, \n
                                   f_float32_t cent_x, f_float32_t cent_y, f_float32_t width, f_float32_t height,\n
                                   sColor3f color, f_int32_t list_id, f_int32_t texture_id)
 *  @brief 绘制罗盘.
 *  @param[in] yaw     航向. 
 *  @param[in] cent_x, cent_y     罗盘中心点位置（相对于当前视口的左下角）
 *  @param[in] width, height      罗盘宽高
 *  @param[in] color   颜色. 
 *  @param[in] list_id 罗盘显示列表ID号. 
 *  @param[in] texture_id 罗盘纹理ID号.  
 *  @exception void.
 *  @return f_int32_t 创建成功标志.
 *  @retval  0 创建成功.
 *  @retval -1 创建失败.
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