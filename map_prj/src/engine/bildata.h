#ifndef __BILDATA_H
#define __BILDATA_H

typedef void * BILDATA;

#define INVALID_DEM_VALUE (-9999)
#define BILDATAKEY	0xBDAEDADE
#define BILLEVELS	4
#define MAXBILBUF	100
#define BILSIZE	    256
#define BILSIZE2    (BILSIZE * BILSIZE)
#define H_SCALE_DEGREE 3
#define V_SCALE_DEGREE 3
#define MAXBLOCKBILFILE 20

/*�߳������ļ���������ṹ��*/
typedef struct tagBILBlockFile
{
	struct tagBILBlockFile * pPrev, * pNext;  /*�߳������ļ�����ָ��,�ֱ�ָ��ǰһ���ͺ�һ�������ļ�*/
	FILE *pFile;                              /*�߳������ļ�ָ��*/
	char blockFileName[256];                  /*�߳������ļ�·��*/
}sBILBlockFILE;

/*��ͼ���߳����ݴ洢����ṹ��*/
typedef struct tagBILFile
{
	struct tagBILFile * pPrev, * pNext;       /*�߳�����ͼ������ָ��,�ֱ�ָ��ǰһ���ͺ�һ��ͼ��*/
	int hstart, vstart, hend, vend;           /*�߳�����ͼ�����½Ǿ��ȡ����½�γ�ȡ����½Ǿ��ȡ����½�γ��*/
	short data[1];                            /*��ʵ�߳�����,�����ʵ��ͼ�����ݴ�С��̬�����ڴ�*/
}sBILFILE;

/*���㼶�߳����ݹ���ṹ��*/
typedef struct tagBILLevelData
{
	char pathname[256];           /*���㼶�߳��������ڵ�Ŀ¼*/
	int  hscale;                  /*���㼶�߳�������ÿ��ͼ��(ÿ���ļ��������ͼ)����ľ��ȿ��,1/128��Ϊ��λ*/
	int  vscale;                  /*���㼶�߳�������ÿ��ͼ��(ÿ���ļ��������ͼ)�����γ�ȿ��,1/128��Ϊ��λ*/
	int  bufCount;                /*���и߳������ļ��д�ŵĸ߳�ͼ������*/
	int  blockCount;              /*�߳������ļ��ܸ���*/
	sBILFILE bufHeader;	          /*������ͼ������*/
	sBILBlockFILE blockHeader;    /*�򿪵ĵ����߳������ļ�ָ�����*/
}sBILLEVEL;

/*���в㼶�߳����ݹ����ܽṹ��*/
typedef struct tagBILDATA
{
	unsigned long key;             /*must be BILDATAKEY*/
	sBILLEVEL levels[BILLEVELS];   /*��ͬ�㼶�ĸ߳����ݹ�����,֧��4����ͬ�㼶*/
}sBILDATA;

#ifdef __cplusplus
extern "C" {
#endif

/*
	����һ�����в㼶�߳����ݹ����ܽṹ��ľ����
	������pBILFile -- ����ɹ��򷵻��ļ��������������ʹ��
	����ֵ��
	     0 -- �ɹ����ļ������pBILFile��
		-1 -- �ڴ����ʧ��
*/
int bildataCreate(BILDATA * pBILFile);

/*
	ɾ��BILDATA������ͷ���ռ�õ���Դ
	������bildata -- ��bildataCreate���ص�BILDATA���
	����ֵ��
	     0 -- �ɹ�
		-1 -- �����Ч
*/
int bildataDestroy(BILDATA bildata);

/*
	����ָ���㼶�ĸ߳�����Ŀ¼���߳�ͼ�����ȿ�ȡ��߳�ͼ��γ�ȿ�ȵ����в㼶�߳����ݹ����ܽṹ����
	������bildata -- ��bildataCreate���ص�BILDATA���
		  level -- ��� 0 ~ 3
	      cstrPathName -- ��/��β��Ŀ¼������ʾ�ò���������Ŀ¼
		  hscale -- һ��ͼ���ľ��ȿ��, 1/128�뵥λ
		  vscale -- һ��ͼ����γ�ȿ�ȣ�1/128�뵥λ
    ����ֵ��
	      0 -- �ɹ�
		 -1 -- �����Ч
		 -2 -- �㼶������Ч
*/
int bildataSetDataPath(BILDATA bildata, int level, const char * cstrPathName, int hscale, int vscale);

/*
	���ܣ���ָ���㼶��ԭʼ�߳������в�ѯָ����γ�ȵĸ߳�
	������bildata -- ��bildataCreate���ص�BILDATA���
		  level -- ��� 0-3
		  h -- ����, 1/128��Ϊ��λ
		  v -- γ��, 1/128��Ϊ��λ
	����ֵ��
		  >-9999: �����߳�
		   -9999: ԭʼ�߳����ݲ�����
		  -10001: ԭʼ�߳����ݳ�����Χ
		  -10002: ���ݾ����Ч
		  -10003: ������Ч
		  -10004: ����ռ����
*/
short bildataGetHeight(BILDATA bildata, int level, int h, int v);

/*
 * ��ʾ��ǰ�ڴ����������ͼ���߳��������Ѵ򿪵�ͼ�������ļ�����
 * */
void bildataPrintCacheCount(BILDATA bildata, int level);

#ifdef __cplusplus
}
#endif

#endif /* __BILFILE_H */