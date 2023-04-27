#ifndef _mapLoad_h_ 
#define _mapLoad_h_ 

#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef WIN32
void WINAPI qtmapDataLoadRoute(sMAPHANDLE *pHandle);
#else
void qtmapDataLoadRoute(sMAPHANDLE *pHandle);
#endif

/*
功能：获取影像数据加载任务的心跳
输入：
      无
输出：
      无
返回值：
	  任务心跳
*/
unsigned long getImgLoadTaskHeartBeat();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 