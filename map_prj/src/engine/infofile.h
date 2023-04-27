#ifndef __INFOFILE_H
#define __INFOFILE_H

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif

VOIDPtr infofileOpen(f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx, 
                     f_int32_t level, const f_char_t * pFileName);
void infofileClose(VOIDPtr infofile);
f_int32_t loadinfofile(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                       VOIDPtr param, f_float32_t * ppInfo);

#ifdef __cplusplus
}
#endif

#endif
