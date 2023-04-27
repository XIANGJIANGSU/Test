#ifndef __QTNFILE_H
#define __QTNFILE_H

typedef void * QTNFILE;
#include "../define/mbaseType.h"
#include "../mapApi/common.h"
 
#ifdef __cplusplus
extern "C" {
#endif

/*
	��һ��QTN�ļ���
	������cstrFileName -- �ļ�����
	      pQTNFile -- ����ɹ��򷵻��ļ��������������ʹ��
	����ֵ��
	     0 -- �ɹ����ļ������pQTNFile��
		-1 -- �ļ���ʧ��
		-2 -- �ļ���ʽ����
		-3 -- �ڴ����ʧ��
*/
f_int32_t qtnfileOpen(const f_char_t * cstrFileName, VOIDPtr * pQTNFile);

/*
	�ر�һ���Ѿ��򿪵�QTN�ļ����ͷ��ļ�ռ�õ���Դ
	������qtnfile -- ��qtnfileOpen���ص�QTN�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
*/
f_int32_t qtnfileClose(VOIDPtr qtnfile);

/*
	�õ�һ���Ѿ��򿪵�QTN�ļ������Ĳ���
	������qtnfile -- ��qtnfileOpen���ص�QTN�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
*/
f_int32_t qtnfileGetLayerCount(VOIDPtr qtnfile);

/*
	�õ�һ��QTN�ļ��еĻ���ͼ������
	������qtnfile -- ��qtnfileOpen���ص�QTN�ļ����
	      layer -- ָ���Ĳ㣬��0��ʼ
		  xidx, yidx -- ����ͼ���ڲ��еı��
		  pData -- ���ص�����
	����ֵ��
	     1 -- �������ڵ���
	     0 -- �ɹ�
		-1 -- �ļ������Ч
		-2 -- �����Ч
		-3 -- xidx��yidx��Ч
		-4 -- �ڴ�������
		-5 -- ���ݽ������
*/
f_int32_t qtnfileGetData(VOIDPtr qtnfile, f_int32_t layer, f_int32_t xidx, f_int32_t yidx, void ** pData);

/*
layer:��ʾ�Ĳ��
*/
void qtnSetLayerFunc(VOIDPtr map_handle, f_int32_t layer);

void qtnAddFile(const f_char_t * pFileName, f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx);

void qtnInit(void);
void qtnDeInit(void);


#ifdef __cplusplus
}
#endif

#endif /* __QTNFILE_H */