/**
 * @file camera3d.h
 * @brief ���ļ����������ģ�͵�����㷨
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
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
