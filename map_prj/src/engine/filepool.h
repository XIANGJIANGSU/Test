#ifndef __FILEPOOL_H
#define __FILEPOOL_H

#include <stdio.h>
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * FILEITEM;

FILEITEM filepoolOpen(const f_char_t * pstrFileName, f_int32_t read);
void filepoolClose(FILEITEM hfile);
FILE * filepoolGetFile(FILEITEM hfile);
void filepoolReleaseFile(FILEITEM hfile);
f_int32_t filepoolGetOpenedCount();
/* 初始化文件池 */
void filepoolInit();

#ifdef __cplusplus
};
#endif

#endif
