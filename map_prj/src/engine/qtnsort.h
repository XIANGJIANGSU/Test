#ifndef __QTN_SORT_H
#define __QTN_SORT_H
 
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"

typedef struct tagQTNText
{
	f_float64_t sx, sy, sz;	         /*< 注记所在地面上的点的屏幕坐标   */
	f_float64_t nsx, nsy, nsz;		 /*< 注记标志杆顶端所在点的屏幕坐标 */
	f_float32_t dis;                 /*< 距离 */
	f_int32_t   textlen;             /*< 注记的字符数，不包含符号       */
	f_int32_t   code;                /*< 注记识别码 */
	f_int32_t   textwidth;
	f_int32_t   textheight;
	f_uint16_t  text[12 + 8];            /*< 注记的unicode码(最多显示12个字+(距离Km))*/
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