#ifndef __IMAGE_BMP_H__
#define __IMAGE_BMP_H__

#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif

f_int32_t createBmpTexture(f_int8_t* filename, f_int32_t *texture_id);

f_int32_t createPngTexture(f_int8_t* filename, f_int32_t* texture_id);

f_uint8_t* ReadBMPEX(const f_char_t* path, f_uint16_t* bmp_width, f_uint16_t* bmp_height, f_uint32_t *bmp_size, f_uint8_t* bmp_style);


#ifdef __cplusplus
}
#endif

#endif
