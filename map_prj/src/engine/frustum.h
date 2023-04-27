#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

#include "../define/mbaseType.h"
#include "../mapApp/appHead.h"

// typedef struct tagPLANE
// {
// 	f_float64_t A;
// 	f_float64_t B;
// 	f_float64_t C;
// 	f_float64_t D;
// }PLANE, *LPPLANE;

// typedef struct tagFRUSTUM
// {
// 	PLANE plane[6];
// }FRUSTUM, *LPFRUSTUM;

/*计算视景体参数*/
void CalculateFrustum(FRUSTUM* frustum, sGLRENDERSCENE *pScene);

/*判断瓦片是否在视景体内*/
BOOL InFrustum(const f_float32_t view_near, const f_float32_t view_far,
			   const f_float64_t x, const f_float64_t y, const f_float64_t z,
			   const f_float64_t radius,const LPFRUSTUM pFrustum, f_float64_t *pDis);

#endif
