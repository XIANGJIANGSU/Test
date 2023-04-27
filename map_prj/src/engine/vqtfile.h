#ifndef __VQTFILE_H
#define __VQTFILE_H

#include "../define/mbaseType.h"
#include "../mapApi/common.h"
 
#ifdef __cplusplus
extern "C" {
#endif

/*
	��һ��VQT�ļ���
	������cstrFileName -- �ļ�����
	����ֵ��
	       �ļ����
*/
VOIDPtr vqtfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName);

/*
	�ر�һ���Ѿ��򿪵�VQT�ļ����ͷ��ļ�ռ�õ���Դ
	������vqtfile -- ��vqtfileOpen���ص�VQT�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*/
f_int32_t vqtfileClose(VOIDPtr vqtfile);

/*
	�õ�һ���Ѿ��򿪵�VQT�ļ������Ĳ���
	������vqtfile -- ��vqtfileOpen���ص�TQT�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*/
f_int32_t vqtfileGetLevels(VOIDPtr vqtfile);

/*
	�õ�һ���Ѿ��򿪵�VQT�ļ���������ʼ���
	������vqtfile -- ��vqtfileOpen���ص�SQS�ļ����
	����ֵ��
	    >0 -- ������ʼ���
		-1 -- �ļ������Ч
*/
f_int32_t vqtfileGetLevelstart(VOIDPtr vqtfile);

/*
	�õ�һ���Ѿ��򿪵�TQT�ļ��л���ͼ��Ĵ�С
	������vqtfile -- ��vqtfileOpen���ص�VQT�ļ����
	����ֵ��
	    >0 -- ����ͼ��Ĵ�С
		-1 -- �ļ������Ч
*/
f_int32_t vqtfileGetTileSize(VOIDPtr vqtfile);

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
f_int32_t imgVqtIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);

f_int32_t loadvqt(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);
f_int32_t vqt2tex(f_uint64_t param , f_uint8_t *pImg);

int read_PNG_vqtfile (FILE * pFile, unsigned char **ppImg, int *width, int *height, int * rgbaChannel);


#ifdef __cplusplus
}
#endif

#endif /* __VQTFILE_H */
