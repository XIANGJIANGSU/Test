/********************************************************************
	created:	2015/12/15
	created:	15:12:2015   15:27
	filename: 	Atmosphere.c
	file path:	$地图软件目录$\VxWorks_VecMap\3dMapEngine_prj\src
	file base:	Atmosphere
	file ext:	c
	author:		张钰鹏
	
	purpose:	生成大气(天空)效果
*********************************************************************/

#ifndef _ANNOTATION_H_
#define _ANNOTATION_H_

#include "../define/mbaseType.h"
#include "../mapApp/appHead.h"
#include "../mapApi/common.h"
#include "mapRender.h"

typedef struct tagTexKeyColor	//渐变纹理的关键色点位置
{
	f_float32_t posPercent;		//色点位置百分比
	f_int32_t colorR;
	f_int32_t colorG;
	f_int32_t colorB;
} sTexKeyColor;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void initAtmosphereTex(VOIDPtr map_andle);
void renderAtmosphere(sGLRENDERSCENE * pHandle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
