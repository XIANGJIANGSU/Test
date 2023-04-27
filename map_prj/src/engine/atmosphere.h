/********************************************************************
	created:	2015/12/15
	created:	15:12:2015   15:27
	filename: 	Atmosphere.c
	file path:	$��ͼ���Ŀ¼$\VxWorks_VecMap\3dMapEngine_prj\src
	file base:	Atmosphere
	file ext:	c
	author:		������
	
	purpose:	���ɴ���(���)Ч��
*********************************************************************/

#ifndef _ANNOTATION_H_
#define _ANNOTATION_H_

#include "../define/mbaseType.h"
#include "../mapApp/appHead.h"
#include "../mapApi/common.h"
#include "mapRender.h"

typedef struct tagTexKeyColor	//��������Ĺؼ�ɫ��λ��
{
	f_float32_t posPercent;		//ɫ��λ�ðٷֱ�
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
