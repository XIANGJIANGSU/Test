#ifndef _mapNodeDel_h_ 
#define _mapNodeDel_h_ 

#include "../define/mbaseType.h"
#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

f_int32_t qtmapnodeDestroyData(sMAPHANDLE * pHandle, sQTMAPNODE * pNode);
void qtmapnodeDeleteFromList(sMAPHANDLE * pHandle, sQTMAPNODE * pNode);
f_int32_t qtmapnodeDestroy(sMAPHANDLE * pHandle, sQTMAPNODE * pNode);
f_int32_t qtmapClearCache(sMAPHANDLE * pHandle, sQTMAPNODE * pNode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 