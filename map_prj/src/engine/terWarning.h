#ifndef _terWarning_h_ 
#define _terWarning_h_ 

#include "../mapApi/common.h"
#include "../mapApp/appHead.h"
#include "../define/mbaseType.h"
#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void setTerrainWarningStripeColor(VOIDPtr map_handle, sTerWarningSet terwarning_set);
void GenTerrainWarningTexture(VOIDPtr scene);
BOOL RenderTerrainWarningStripeColor(sGLRENDERSCENE *pHandle);
void RenderTerrainWarningStripeColor2(sGLRENDERSCENE *pHandle, sQTMAPNODE *pNode);
void RenderTerrainWarningStripeColor3();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
