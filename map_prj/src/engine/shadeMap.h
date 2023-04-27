#ifndef _shadeMap_h_ 
#define _shadeMap_h_ 

#include "../mapApi/common.h"
#include "../mapApp/appHead.h"
#include "../define/mbaseType.h"
#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void RenderShadeMap(sGLRENDERSCENE *pHandle, sQTMAPNODE *pNode);
BOOL RenderShadeMapPre(sGLRENDERSCENE *pHandle);
void RenderShadeMapPro();
void GetGridTextureID();

BOOL GenerateTileNormal(sQTMAPNODE* mapNode);
BOOL GenerateTexCoord(sQTMAPNODE  *pNode);
BOOL GenerateNodePVertexTex(sQTMAPNODE* pNode);
BOOL GenerateVertexColor(sQTMAPNODE* mapNode);

void InitVecShadedReliefMap();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
