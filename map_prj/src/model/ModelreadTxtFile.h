/*******************************************************************
********************************************************************
*                                                                  *
*版权：   中国航空无线电电子研究所，2013年                         *
*                                                                  *
*作者：   张仟新  2013/07/27                                       *
*                                                                  *
*更改历史：                                                        *
*        姓名     日期         更改说明                            *
********************************************************************
*******************************************************************/

#ifndef READ_TXTFILE_H
#define READ_TXTFILE_H

#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include "../map_prj/src/define/mbaseType.h"
#else
#include "../define/mbaseType.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int readtxtfile_getStr(FILE *pBinFile, char * buf, int maxlen);
extern int readtxtfile_getInt(FILE *pBinFile);
extern int readtxtfile_nextline(FILE *pBinFile);
extern float readtxtfile_getFloat(FILE *pBinFile);
extern f_uint8_t* ReadBMP(const f_char_t* path, f_uint16_t* bmp_width, f_uint16_t* bmp_height, f_uint32_t *bmp_size, f_uint8_t* bmp_style);

#ifdef __cplusplus
};
#endif

#endif/*READ_TXTFILE_H*/
