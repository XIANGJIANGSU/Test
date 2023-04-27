/**
 * @file vectorMethods.h
 * @brief ���ļ��������������������㷨
 * @author 615��ͼ�Ŷ�
 * @date 2016-05-26
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _vectorMethods_h_ 
#define _vectorMethods_h_

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* 3F�������� */
/* ��ֵ*/
void vector3FMemSet(LP_PT_3F pt, const f_float32_t x, const f_float32_t y, const f_float32_t z);
void vector3FSetValue(LP_PT_3F pLeft, LP_PT_3F pRight);
/* �ӷ�*/
void vector3FAdd(LP_PT_3F pOut, const LP_PT_3F pt0, const LP_PT_3F pt1);
/* ����*/
void vector3FSub(LP_PT_3F pOut, const LP_PT_3F pt0, const LP_PT_3F pt1);
/* �˷�*/
void vector3FMul(LP_PT_3F pOut, const LP_PT_3F pIn, const f_float32_t lf);
/* ����*/
void vector3FDiv(LP_PT_3F pOut, const LP_PT_3F pIn, const f_float32_t lf);
/* ����*/
f_float32_t vector3FLength(const LP_PT_3F pt);
/* ��λ��*/
void vector3FNormalize(LP_PT_3F pInOut);
/* ���*/
f_float32_t vector3FDotProduct(const LP_PT_3F pt0, const LP_PT_3F pt1);
/* ����*/
void vector3FNegative(LP_PT_3F pInOut);
/* ���*/
void vector3FCrossProduct(LP_PT_3F pOut, const LP_PT_3F pIn0,  const LP_PT_3F pIn1);
/* ����������ļн� */
f_float32_t getVa2FVbAngle(const LP_PT_3F pt0, const LP_PT_3F pt1);
/*
 * ������������ĳ������תһ�����ȵĽ��
 * ����: ��ʼ�����������, ������, ��ת�Ļ���ֵ
 * ����: ��ת�Ľ��ֵ(���)
 */
PT_3F vector3FRotate(const LP_PT_3F vpRotate, const LP_PT_3F vpAxis, const f_float32_t radRotate);

/* 3D�������� */
/* ��ֵ*/
void vector3DMemSet(LP_PT_3D pt, const f_float64_t x, const f_float64_t y, const f_float64_t z); 
void vector3DSetValue(LP_PT_3D pLeft, LP_PT_3D pRight);
/* �ӷ�*/
void vector3DAdd(LP_PT_3D pOut, const LP_PT_3D pt0, const LP_PT_3D pt1);
/* ����*/
void vector3DSub(LP_PT_3D pOut, const LP_PT_3D pt0, const LP_PT_3D pt1);
/* �˷�*/
void vector3DMul(LP_PT_3D pOut, const LP_PT_3D pIn, const f_float64_t lf);
/* ����*/
void vector3DDiv(LP_PT_3D pOut, const LP_PT_3D pIn, const f_float64_t lf);
/* ����*/
f_float64_t vector3DLength(const LP_PT_3D pt);
/* ��λ��*/
void vector3DNormalize(LP_PT_3D pInOut);
/* ���*/
f_float64_t vector3DDotProduct(const LP_PT_3D pt0, const LP_PT_3D pt1);
/* ����*/
void vector3DNegative(LP_PT_3D pInOut);
/* ���*/
void vector3DCrossProduct(LP_PT_3D pOut, const LP_PT_3D pIn0,  const LP_PT_3D pIn1);
/* ����������ļн� */
f_float64_t getVa2DVbAngle(const LP_PT_3D pt0, const LP_PT_3D pt1);

/*
 * ������������ĳ������תһ�����ȵĽ��
 * ����: ��ʼ�����������, ������, ��ת�Ļ���ֵ
 * ����: ��ת�Ľ��ֵ(���)
 * ˵����������ת��1,��vector3DRotateM2���һ��
 */
PT_3D vector3DRotate(const LP_PT_3D vpRotate, const LP_PT_3D vpAxis, const f_float64_t radRotate);

/*
���ܣ��������rotated_pt������ת��rotate_axis��תһ���Ƕ�angle_rad
���룺rotated_pt   �������
      rotate_axis  ��ת��
      angle_rad    ��ת��(����)
�������ת��ĵ������
˵����������ת��2, ��vector3DRotate�Ľ��һ��
*/
PT_3D vector3DRotateM2(const PT_3D rotated_pt, const PT_3D rotate_axis, const f_float64_t angle_rad);

/* 4D�������� */
/* ��ֵ*/
void vector4DMemSet(LP_PT_4D pt, const f_float64_t x, const f_float64_t y, const f_float64_t z, const f_float64_t w);
void vector4DSetValue(LP_PT_4D pLeft, LP_PT_4D pRight);

double DistanceTwoPoints(const LP_PT_3D pPt1, const LP_PT_3D pPt2);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 