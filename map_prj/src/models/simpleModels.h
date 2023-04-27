/**
 * @file simpleModels.h
 * @brief ���ļ������˼򵥼�����Ļ��Ʒ���
 * @author 615��ͼ�Ŷ�
 * @date 2017-01-06
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef simpleModels 
#define simpleModels

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
���ܣ����������
���룺width, height, length    ������ĳ��������(��λ:��)
�������
*/
void avicCube(f_float64_t width, f_float64_t height, f_float64_t length);

/*
���ܣ��������
���룺radius    ����İ뾶(��λ:��)
      slices    ����İ���
      stacks    ����Ĳ���
�������
*/
void avicSphere(f_float64_t radius, f_int32_t slices, f_int32_t stacks);

/*
���ܣ��������
���룺baseRadius    ����ĵ���뾶(��λ:��)
      topRadius     ����Ķ���뾶(��λ:��)
      height        ����ĸ߶�(��λ:��)
      slices        ����İ���
      stacks        ����Ĳ���
      needDrawTop   �����Ƿ���Ƶı�־
�������
*/
void avicCylinder(f_float64_t baseRadius, f_float64_t topRadius, f_float64_t height, f_int32_t slices, f_int32_t stacks, BOOL needDrawTop);

/*
���ܣ����岿�ֵĻ���
���룺innerRadius    ������ڰ뾶(��λ:��)
      outerRadius    �������뾶(��λ:��)
      slices         ����İ���
      loops          ����Ĳ���
      startAngle     �������ʼ�Ƕ�(��λ:��)
      sweepAngle     �Ƕȿ��(��λ:��)
�������
*/
void avicPartialDisk(f_float64_t innerRadius, f_float64_t outerRadius, 
                     f_int32_t slices, f_int32_t loops, f_float64_t startAngle, f_float64_t sweepAngle);
                     
/*
���ܣ�Բ�̵Ļ���
���룺innerRadius    Բ�̵��ڰ뾶(��λ:��)
      outerRadius    Բ�̵���뾶(��λ:��)
      slices         Բ�̵İ���
      loops          Բ�̵Ĳ���
�������
*/
void avicDisk(f_float64_t innerRadius, f_float64_t outerRadius, f_int32_t slices, f_int32_t loops);     

/*
���ܣ�����������
���룺edgeLength  �������߳�
�������
*/
void avicPyramid(f_float64_t edgeLength);                

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 