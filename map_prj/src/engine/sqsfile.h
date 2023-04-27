#ifndef __SQSFILE_H
#define __SQSFILE_H

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	��һ��SQS�ļ���
	������cstrFileName -- �ļ�����
	����ֵ��
	    �ļ����
*/
VOIDPtr sqsfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName);

/*
	�ر�һ���Ѿ��򿪵�SQS�ļ����ͷ��ļ�ռ�õ���Դ
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*/
f_int32_t sqsfileClose(VOIDPtr sqsfile);

/*
	�õ�һ���Ѿ��򿪵�SQS�ļ������Ĳ���
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*/
f_int32_t sqsfileGetLevels(VOIDPtr sqsfile);

/*
	�õ�һ���Ѿ��򿪵�SQS�ļ���������ʼ���
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
*/
f_int32_t sqsfileGetLevelstart(VOIDPtr sqsfile);

/*
	�õ�һ���Ѿ��򿪵�SQS�ļ��л���ͼ��Ĵ�С
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*/
f_int32_t sqsfileGetTileSize(VOIDPtr sqsfile);

/*
	�õ�һ���Ѿ��򿪵�SQS�ļ��л���ͼ���MIPMAP����
	������sqsfile -- ��sqsfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ����ͼ���MIPMAP����
		-1 -- �ļ������Ч
*/
f_int32_t sqsfileGetMipMapCount(VOIDPtr sqsfile);

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
f_int32_t imgSqsIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);

f_int32_t loadsqs(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);
f_int32_t sqs2tex(f_uint64_t param , f_uint8_t *pImg);

#ifdef WIN32
void sqsTextureFunInit(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SQSFILE_H */
