#ifndef _HlTools 
#define _HlTools

#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
���ܣ����ݵ�ǰ��ľ�γ�ȼ������ľ��롢����,�����Բ�ϵ�λ��
���룺lon0, lat0        ��ǰ��ľ�γ��(��λ:��)
      distance          ����(��λ:��)
      course            ����(��λ:��)
�����lon1, lat1        �����ľ�γ��(��λ:��)      
*/
void caculateLonLatOnGreateCircle(f_float64_t lon0, f_float64_t lat0, f_float64_t distance, f_float64_t course, 
                                  f_float64_t *lon1, f_float64_t *lat1);

/*
���ܣ����ݵľ��ȣ�����������Բ�ϵ�γ��
���룺
      lon        ��Ҫ����γ������Ӧ�ľ��ȣ���λ�ȣ�
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      γ�ȣ���λ�ȣ�
	  ����ɹ���־, TRUE �ɹ�,FALSE ʧ��
˵������cos(lat1) == 0,��(lon1,lat1)������,���´�Բ�϶����ϱ��������㣻
      ͬ����cos(lat2) == 0,��(lon2,lat2)������,���´�Բ�϶����ϱ��������㣻
	  ��lon1 == lon2,���ԲҲ���ϱ��������㣻
	  ��������������ܸ��ݾ����������Բ�ϵ�γ�ȡ�
*/
BOOL caculateLatitudeOnGreatCircle(f_float64_t lon, f_float64_t *lat, f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);

/*
���ܣ�������������ľ�γ�ȣ����������Ĵ�Բ������
���룺
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      �нǣ���λPI���ȣ�
*/
f_float64_t caculateGreatCircleCourse(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);

/*
���ܣ�������������ľ�γ�ȣ������Բ����
���룺
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      �����Ĵ�Բ���루��λ�ף�     
*/
f_float64_t caculateGreatCircleDis(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);

/*
���ܣ�������������ľ�γ�ȣ����������Ĵ�Բ�н�
���룺
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      �нǣ���λPI���ȣ�
*/
f_float64_t caculateGreatCircleAngle(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
