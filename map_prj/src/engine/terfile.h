#ifndef __TERFILE_H
#define __TERFILE_H

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif

VOIDPtr terfileOpen(f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx, const f_char_t * pFileName);
void terfileClose(VOIDPtr terfile);
f_int32_t loadterfile(f_int32_t level, f_int32_t xidx, f_int32_t yidx, VOIDPtr param, f_int16_t *ppTerrain);
f_int32_t imgTerIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);
f_int32_t terfileGetLevels(VOIDPtr tqsfile);

#ifdef __cplusplus
}
#endif

#endif
