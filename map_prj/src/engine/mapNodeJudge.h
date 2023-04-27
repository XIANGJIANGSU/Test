#ifndef _mapNodeJudge_h_ 
#define _mapNodeJudge_h_ 

#include "../define/mbaseType.h"
#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

sNODERANGE getNodeRange(sQTMAPNODE* pNode);
f_int32_t isAllChildNodeDataReady(sQTMAPNODE * pNode);
f_int32_t isNodeInFrustum(f_float32_t view_near, f_float32_t view_far, LPFRUSTUM pFrustum, sQTMAPNODE * pNode);
f_int32_t isNodeInFrustumOverlook(f_float32_t view_near, f_float32_t view_far, LPFRUSTUM pFrustum, sQTMAPNODE * pNode);
f_int32_t isNodeExistInMapSubreeList(sMAPHANDLE * pHandle, f_int32_t level, 
                                     f_int32_t xidx, f_int32_t yidx, sQTMAPSUBTREE ** ppTree);
int isNodeTooFar(sQTMAPNODE * pNode, Obj_Pt_D cameraPos);
f_int32_t isNeedCreateChildren(sQTMAPNODE * pNode, sQTMAPNODE * pPlanePosNode, Geo_Pt_D cameraPos, Obj_Pt_D camObjPos, int view_type);
/* 判断节点是否在地球背面(TRUE:在背面, FALSE:不在背面) */
BOOL isNodeAtEarthBack(sGLRENDERSCENE * pHandle, sQTMAPNODE * pNode);
/* 判断相机所在位置的经纬度是否在节点所在的范围内 */
BOOL isCaminNode(sGLRENDERSCENE * pHandle, sQTMAPNODE * pNode);
sNODERANGE getNodeRangeOSG(sQTMAPNODE* pNode);
sNODERANGE getNodeRangeOSG_Overlook(sQTMAPNODE* pNode);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
