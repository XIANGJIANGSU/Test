#include <math.h>
#include <stdio.h>
#include "tools.h"

#include "../define/macrodefine.h" 
#include "../geometry/vectorMethods.h"
#include "../projection/coord.h"
#include "../engine/osAdapter.h"

/*
���ܣ����ݵ�ǰ��ľ�γ�ȼ������ľ��롢����,�����Բ�ϵ�λ��
���룺lon0, lat0        ��ǰ��ľ�γ��(��λ:��)
      distance          ����(��λ:��)
      course            ����(��λ:��)
�����lon1, lat1        �����ľ�γ��(��λ:��)      
*/
void caculateLonLatOnGreateCircle(f_float64_t lon0, f_float64_t lat0, f_float64_t distance, f_float64_t course, 
                                  f_float64_t *lon1, f_float64_t *lat1)
{
	f_float64_t sin_lat0 = 0.0, cos_lat0 = 0.0, sin_d = 0.0, cos_d = 0.0, sin_tc = 0.0, cos_tc = 0.0, dlon = 0.0, dis_radians = 0.0;
	f_float64_t lon = 0.0, lat = 0.0, sin_lat = 0.0;

	dis_radians = distance / EARTH_RADIUS;
	sin_d = sin(dis_radians);
	cos_d = cos(dis_radians);
	lon0 *= DE2RA;
	lat0 *= DE2RA;
	sin_lat0 = sin(lat0);
	cos_lat0 = cos(lat0);
	course *= -DE2RA;
	sin_tc = sin(course);
	cos_tc = cos(course);	
	
	lat  = asin(sin_lat0 * cos_d + cos_lat0 * sin_d * cos_tc);
	sin_lat = sin(lat);
    dlon = atan2(sin_tc * sin_d * cos_lat0, cos_d - sin_lat0 * sin_lat);
    lon  = lon0 - dlon;
    
    *lon1 = lon * RA2DE;
    *lat1 = lat * RA2DE;
}

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
BOOL caculateLatitudeOnGreatCircle(f_float64_t lon, f_float64_t *lat, f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t cos_lat1 = 0.0, cos_lat2 = 0.0, sin_lon12 = 0.0, sin_lat1 = 0.0, 
		        sin_lat2 = 0.0, sin_lon01 = 0.0, sin_lon02 = 0.0;
	lon  *= DE2RA;
	lon1 *= DE2RA;
	lat1 *= DE2RA;
	lon2 *= DE2RA;
	lat2 *= DE2RA;
	cos_lat1 = cos(lat1);
	if(fabs(cos_lat1) < FLT_EPSILON) /* ���� */
		return(FALSE);
	cos_lat2 = cos(lat2);
	if(fabs(cos_lat2) < FLT_EPSILON) /* ���� */
		return(FALSE);
	sin_lon12 = sin(lon1 - lon2);
	if(fabs(sin_lon12) < FLT_EPSILON) /* ���� */
		return(FALSE);

	sin_lat1 = sin(lat1);	
	sin_lat2 = sin(lat2);
	sin_lon01 = sin(lon - lon1);
	sin_lon02 = sin(lon - lon2);
	
    *lat = atan((sin_lat1 * cos_lat2 * sin_lon02 - sin_lat2 * cos_lat1 * sin_lon01) / (cos_lat1 * cos_lat2 * sin_lon12)) * RA2DE;
	return(TRUE);
}

/*
���ܣ�������������ľ�γ�ȣ����������Ĵ�Բ������
���룺
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      �нǣ���λPI���ȣ�
*/
f_float64_t caculateGreatCircleCourse(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t course = 0.0, sin_lat1 = 0.0, cos_lat1 = 0.0, sin_lat2 = 0.0, cos_lat2 = 0.0;
    f_float64_t sin_lonsub = 0.0, cos_latsub = 0.0;
    
    sin_lat1 = sin(lat1 * DE2RA);
    cos_lat1 = cos(lat1 * DE2RA);
    sin_lat2 = sin(lat2 * DE2RA);
    
    if(fabs(cos_lat1) < FLT_EPSILON)  /* �Ӽ��㿪ʼ */
    {
        if(lat1 > 0.0)	/* ���� */
            course = PI;
        else  /* �ϼ� */
            course = 2.0 * PI;
    }
    else  /* ��ʼ�㲻�ڼ��� */
    {
        lon1 *= DE2RA;
        lat1 *= DE2RA; 
        lon2 *= DE2RA;
        lat2 *= DE2RA;
        sin_lat1 = sin(lat1);
        cos_lat1 = cos(lat1);
        sin_lat2 = sin(lat2);
        cos_lat2 = cos(lat2);
        sin_lonsub = sin(lon1-lon2);
        cos_latsub = cos(lon1-lon2);
        course = -atan2( sin_lonsub * cos_lat2, cos_lat1 * sin_lat2 - sin_lat1 * cos_lat2 * cos_latsub);
    }  
    return(course);  
}

/*
���ܣ�������������ľ�γ�ȣ����������Ĵ�Բ�н�
���룺
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      �нǣ���λPI���ȣ�
*/
f_float64_t caculateGreatCircleAngle(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t angle = 0.0;
    lon1 *= DE2RA;
    lat1 *= DE2RA; 
    lon2 *= DE2RA;
    lat2 *= DE2RA;
    angle = acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1 - lon2));
	return(angle);
}

/*
���ܣ�������������ľ�γ�ȣ������Բ����
���룺
      lon1, lat1 ��1�ľ�γ�ȣ���λ�ȣ�
      lon2, lat2 ��2�ľ�γ�ȣ���λ�ȣ�
�����
      �����Ĵ�Բ���루��λ�ף�     
*/
f_float64_t caculateGreatCircleDis(f_float64_t lon1, f_float64_t lat1, f_float64_t lon2, f_float64_t lat2)
{
    f_float64_t dis = 0.0;
    dis = caculateGreatCircleAngle(lon1, lat1, lon2, lat2) * EARTH_RADIUS;
    return(dis);
}

// ���ִ���Ƿ�ﵽĳһʱ��(s)
BOOL CheckTriggerTime(float* lastTick, float delayTime)
{
    float curTickCnt = tickCountGet();
    float spendTime = 0.0f;

    // �ײ�������ͨ��
    if(*lastTick <= FLT_EPSILON)
    {
        *lastTick = tickCountGet();
        return TRUE;
    }

    // ����ʱ���(s)
    spendTime = 1.0f * (curTickCnt - *lastTick)/(1.0f * getSysClkRate());
    
    // ��ʱ,ͨ������
    if(spendTime >= delayTime)
    {
        *lastTick = tickCountGet();
        return TRUE;
    }

    return FALSE;
}