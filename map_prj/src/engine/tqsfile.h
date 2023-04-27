#ifndef __TQSFILE_H__
#define __TQSFILE_H__

#include "../define/mbaseType.h"
#include "../mapApi/common.h"
 
#ifdef __cplusplus
extern "C" {
#endif

/*
	��һ��TQT�ļ���
	������cstrFileName -- �ļ�����
	����ֵ��
	       �ļ����
*/
VOIDPtr tqsfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName);

/*
	�ر�һ���Ѿ��򿪵�TQT�ļ����ͷ��ļ�ռ�õ���Դ
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileClose(VOIDPtr tqsfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ������Ĳ���
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileGetLevels(VOIDPtr tqsfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ���������ʼ���
	������tqsfile -- ��tqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileGetLevelstart(VOIDPtr tqsfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ��л���ͼ��Ĵ�С
	������tqsfile -- ��tqsfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*/
f_int32_t tqsfileGetTileSize(VOIDPtr tqsfile);

/*
���ܣ��жϽڵ��Ƿ����ļ��ķ�Χ��
���룺    
    level -- ָ���Ĳ㣬��0��ʼ
	xidx, yidx -- ����ͼ���ڲ��еı��
	param  �ļ�������ڵĵ�ַ
�����
    0  ��
    -1 ����
*/
f_int32_t imgTqsIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);

f_int32_t loadtqs(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);
f_int32_t tqs2tex(f_uint64_t param , f_uint8_t *pImg);

f_int32_t tqs2texRGBA(f_uint64_t param , f_uint8_t *pImg);
f_int32_t loadtqsRGBA(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);


#ifdef __cplusplus
}
#endif

#endif /* __TQTFILE_H */
