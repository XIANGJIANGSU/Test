#ifndef _FILE_SORT_h_ 
#define _FILE_SORT_h_ 

#include "../define/mbaseType.h"

typedef struct tagQuadTreeFileName{
    f_int32_t  rootlevel;     /* ��ʼ�㼶 */
	f_int32_t  rootxidx;     /* x��������� */
	f_int32_t  rootyidx;     /* y��������� */
	f_int32_t  level;    /* �ܲ㼶�� */
	f_char_t name[32];
	f_char_t pathname[256];	//�ļ�����	//32->256,�������飬��֤����Խ��lpf add 2020��5��28��9:44:26
}sQTFName;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

f_int32_t getSortedFile(f_char_t *path, sQTFName **fName_list, char * filetype);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
