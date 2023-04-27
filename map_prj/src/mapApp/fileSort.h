#ifndef _FILE_SORT_h_ 
#define _FILE_SORT_h_ 

#include "../define/mbaseType.h"

typedef struct tagQuadTreeFileName{
    f_int32_t  rootlevel;     /* 起始层级 */
	f_int32_t  rootxidx;     /* x方向的索引 */
	f_int32_t  rootyidx;     /* y方向的索引 */
	f_int32_t  level;    /* 总层级数 */
	f_char_t name[32];
	f_char_t pathname[256];	//文件夹名	//32->256,增大数组，保证不会越界lpf add 2020年5月28日9:44:26
}sQTFName;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

f_int32_t getSortedFile(f_char_t *path, sQTFName **fName_list, char * filetype);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
