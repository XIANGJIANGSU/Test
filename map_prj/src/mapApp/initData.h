/**
 * @file initData.h
 * @brief 该文件提供地图数据初始化的接口函数
 * @author 615地图团队
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _INIT_DATA_h_ 
#define _INIT_DATA_h_ 

#include "../mapApi/common.h"
#include "../define/mbaseType.h"

/* 是否使用DDS纹理 */
#define USE_DDS_TEXTURE
//#undef USE_DDS_TEXTURE

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn int initMapData(char *data_path)
 *  @brief 地图数据初始化.
 *  @param[in] render_scene 场景句柄.
 *  @param[in] data_path 地图数据路径的根目录.
 *  @exception void
 *  @return int, 初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1: 场景句柄为空,失败.
 * @see 
 * @note
*/
int initMapData(f_char_t *data_path);

/**
 * @fn BOOL createTtfFont(f_char_t *font_path, f_int32_t font_size, f_int32_t edge_size,\n
                          f_float32_t color_font[4], f_float32_t color_edge[4])
 *  @brief 字体创建.
 *  @param[in] scene 视口所在的场景句柄.
 *  @param[in] font_path 字库文件路径（不含文件名）.
 *  @param[in] font_file 字库文件名.
 *  @param[in] font_size 字体大小.
 *  @param[in] edge_size 字体边缘大小.
 *  @param[in] color_font 字体颜色.
 *  @param[in] color_edge 边框颜色.
 *  @exception void
 *  @return BOOL,字体创建成功标志.
 *  @retval TRUE 成功.
 *  @retval FALSE 失败.
 * @see 
 * @note 
*/
BOOL createTtfFont(f_char_t *font_path, f_char_t *font_file, 
                   f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4]);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
