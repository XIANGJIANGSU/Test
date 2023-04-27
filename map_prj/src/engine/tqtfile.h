#ifndef __TQTFILE_H
#define __TQTFILE_H

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
VOIDPtr tqtfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName);

/*
	�ر�һ���Ѿ��򿪵�TQT�ļ����ͷ��ļ�ռ�õ���Դ
	������tqtfile -- ��tqtfileOpen���ص�TQT�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*/
f_int32_t tqtfileClose(VOIDPtr tqtfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ������Ĳ���
	������tqtfile -- ��tqtfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*/
f_int32_t tqtfileGetLevels(VOIDPtr tqtfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ���������ʼ���
	������tqtfile -- ��tqtfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
*/
f_int32_t tqtfileGetLevelstart(VOIDPtr tqtfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ��л���ͼ��Ĵ�С
	������tqtfile -- ��tqtfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*/
f_int32_t tqtfileGetTileSize(VOIDPtr tqtfile);

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
f_int32_t imgTqtIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);

f_int32_t loadtqt(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);
f_int32_t tqt2tex(f_uint64_t param , f_uint8_t *pImg);
void setTqtAlpha(f_uint8_t * pData, f_int32_t width, f_int32_t height, char alpha);

int read_JPEG_tqtfile(FILE * pFile, unsigned char **ppImg, int *width, int *height);

#ifdef __cplusplus
}
#endif

#endif /* __TQTFILE_H */
