#ifndef _Hlproject_h_ 
#define _Hlproject_h_

#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void avicglOrtho2D(f_float64_t left, f_float64_t right, f_float64_t bottom, f_float64_t top);
void avicglPerspective(f_float64_t fovy, f_float64_t aspect, f_float64_t zNear, f_float64_t zFar);
void avicglLookAt(f_float64_t eyex,    f_float64_t eyey,    f_float64_t eyez, 
                  f_float64_t centerx, f_float64_t centery, f_float64_t centerz, 
                  f_float64_t upx,     f_float64_t upy,     f_float64_t upz);
BOOL avicglProject(f_float64_t objx, f_float64_t objy, f_float64_t objz, 
	               const f_float64_t modelViewMatrix[16], const f_int32_t viewport[4], 
	               f_float64_t *viewx, f_float64_t *viewy, f_float64_t *viewz);
BOOL avicglUnProject(f_float64_t viewx, f_float64_t viewy, f_float64_t viewz,
		             const f_float64_t invModelViewMatrix[16], const f_int32_t viewport[4],
	                 f_float64_t *objx, f_float64_t *objy, f_float64_t *objz);

/*通过模视矩阵计算点的视觉Z坐标*/
BOOL avicgetViewcoordZ(const f_float64_t modelViewMatrix[16], 
	f_float64_t objx, f_float64_t objy, f_float64_t objz, f_float64_t *pZ);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
