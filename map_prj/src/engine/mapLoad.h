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
���ܣ���ȡӰ�����ݼ������������
���룺
      ��
�����
      ��
����ֵ��
	  ��������
*/
unsigned long getImgLoadTaskHeartBeat();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 