/**
 * @file bil.h
 * @brief ���ļ��ṩ��ȡ���θ̵߳��ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-12-30
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _BIL_h_ 
#define _BIL_h_ 

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
 
/**
 * @fn BOOL createBilHandle(f_char_t *data_path)
 *  @brief ����BIL���.
 *  @param[in] data_path BIL����·���������ļ�����.
 *  @exception void
 *  @return BOOL,BIL��������ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
 * @see 
 * @note 
*/
BOOL createBilHandle(f_char_t *data_path);

/*
���ܣ����ݾ�γ�Ȼ�ȡ�߳�ֵ,�����ϵ���߷���������壬8848.86��������ߵ㣻������͵��������ɺ���������11034��
      �������õ����ֵΪ10000,���ֵΪ-9999
���룺
    lon ����
	lat γ��
�����
    pzvalue ��ѯ�õ��ĸ߶�ֵ�������ָ��
����ֵ��
    TRUE  ��ѯ�ɹ�,*pAltΪ��ѯ�õ��ĸ߳�ֵ(
	      >-9999: �����߳�
		   -9999: ԭʼ�߳����ݲ�����)
	FALSE ��ѯʧ��,*pAltΪһ���쳣ֵ(
	      -10001: ԭʼ�߳����ݳ�����Χ
		  -10002: ���ݾ����Ч
		  -10003: ������Ч
		  -10004: ����ռ����)
*/
BOOL getAltByGeoPos(const f_float64_t lon, const f_float64_t lat, f_int16_t *pAlt);


/*
���ܣ����ݾ�γ�ȡ�����뾶����ȡ�����η�Χ(Ҳ���Կ���Բ)�����߳�ֵ
���룺
    lon ����
	lat γ��
	radius �����η�Χ�ڽ�Բ�뾶
�����
    ���ظ߳�ֵ
*/
f_int16_t getMaxAltInRect(const f_float64_t lon, const f_float64_t lat, f_float64_t radius);

/*
���ܣ�����̻߳�ȡ���ܻ���
���룺
    ��
�����
    ��
����ֵ��
	TRUE ����ɹ�
	FALSE ����ʧ��
*/
BOOL deleteBilHandle();

/* ��ʾ��ǰ�ڴ����������ͼ���߳��������Ѵ򿪵�ͼ�������ļ����� */
void printBilFileInMem();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 