/**
 * @file camera3d.h
 * @brief 该文件定义了相机模型的相关算法
 * @author 615地图团队
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/
#ifndef _camera3d_h_ 
#define _camera3d_h_ 

#include "../define/mbaseType.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void setCameraStyle(f_uint8_t style, LP_PT_3D ptEye, LP_PT_3D ptCenter, LP_PT_3D ptUp, LP_PT_3D ptTo,
					f_float64_t pitch, f_float64_t yaw, f_float64_t roll, LPMatrix44 rmx, LPMatrix44 mx); 
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif 
