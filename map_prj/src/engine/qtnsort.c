#include <math.h>
#include "qtnsort.h"

static void qtnswap(sQTNText *x, sQTNText *y)
{
    sQTNText temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

static f_int32_t qtnchoose_pivot(f_int32_t i, f_int32_t j)
{
    return((i + j)/2);
}

/*
	注记按距离(注记与视点的距离)由近到远进行快速排序
	参数：list--注记数组指针
	      m--数组起始下标
		  n--数组结束下标
    返回值：无
*/
void qtnQuickSortByDis(sQTNText *list, f_int32_t m, f_int32_t n)
{
    f_float32_t key;
    f_int32_t i, j, k;
	if(m < n)
	{
	    k = qtnchoose_pivot(m, n);
		qtnswap(&list[m], &list[k]);
		key = list[m].dis;
		i = m + 1;
		j = n;
		while(i <= j)
		{
		    while((i <= n) && ((list[i].dis < key) || (fabs(list[i].dis - key) < FLT_EPSILON)))
				i++;
			while((j >= m) && (list[j].dis > key))
				j--;
			if(i < j)
				qtnswap(&list[i], &list[j]);
		}
		qtnswap(&list[m], &list[j]);
		qtnQuickSortByDis(list, m, j-1);
		qtnQuickSortByDis(list, j+1, n);
	}
}

/*
	注记按深度值由小到大进行快速排序
	参数：list--注记数组指针
	      m--数组起始下标
		  n--数组结束下标
    返回值：无
*/
void qtnQuickSortByDepth(sQTNText *list, f_int32_t m, f_int32_t n)
{
    f_float64_t key;
    f_int32_t i, j, k;
	if(m < n)
	{
	    k = qtnchoose_pivot(m, n);
		qtnswap(&list[m], &list[k]);
		key = list[m].nsz;
		i = m + 1;
		j = n;
		while(i <= j)
		{
		    while((i <= n) && ((list[i].nsz < key) || (fabs(list[i].nsz - key) < FLT_EPSILON)))
				i++;
			while((j >= m) && (list[j].nsz > key))
				j--;
			if(i < j)
				qtnswap(&list[i], &list[j]);
		}
		qtnswap(&list[m], &list[j]);
		qtnQuickSortByDis(list, m, j-1);
		qtnQuickSortByDis(list, j+1, n);
	}
}
