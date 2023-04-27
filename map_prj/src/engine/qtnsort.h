#ifndef __QTN_SORT_H
#define __QTN_SORT_H
 
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"

typedef struct tagQTNText
{
	f_float64_t sx, sy, sz;	         /*< ע�����ڵ����ϵĵ����Ļ����   */
	f_float64_t nsx, nsy, nsz;		 /*< ע�Ǳ�־�˶������ڵ����Ļ���� */
	f_float32_t dis;                 /*< ���� */
	f_int32_t   textlen;             /*< ע�ǵ��ַ���������������       */
	f_int32_t   code;                /*< ע��ʶ���� */
	f_int32_t   textwidth;
	f_int32_t   textheight;
	f_uint16_t  text[12 + 8];            /*< ע�ǵ�unicode��(�����ʾ12����+(����Km))*/
}sQTNText;

#ifdef __cplusplus
extern "C" {
#endif

void qtnQuickSortByDis(sQTNText *list, f_int32_t m, f_int32_t n);
void qtnQuickSortByDepth(sQTNText *list, f_int32_t m, f_int32_t n);

#ifdef __cplusplus
}
#endif

#endif /* __QTN_SORT_H */