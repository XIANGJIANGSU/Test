#ifndef _CONVERTEDDIAN_H_
#define _CONVERTEDDIAN_H_
#include "../define/mbaseType.h"

typedef union{
	f_int16_t f;
	f_char_t c[2];
}SHORT_CONV;

typedef union{
	f_uint16_t f;
	f_char_t c[2];
}UNSIGNED_SHORT_CONV;

typedef union{
	f_int32_t f;
	f_char_t c[4];
}INT_CONV;

typedef union{
	f_float32_t f;
	f_char_t c[4];
}FLOAT_CONV;

typedef union{
	f_float64_t d;
	f_char_t c[8];
}DOUBLE_CONV;

typedef union{
	f_int64_t f;
	f_char_t c[4];
}LONG_CONV;

typedef union{
	f_uint64_t f;
	f_char_t c[4];
}UNSIGNED_LONG_CONV;

extern f_int16_t	 ConvertL2B_short(f_int16_t s);
extern f_uint16_t  ConvertL2B_ushort(f_uint16_t s);
extern f_int32_t	 ConvertL2B_int(f_int32_t i);
extern f_uint32_t	 ConvertL2B_uint(f_uint32_t i);
extern f_int64_t	 ConvertL2B_long(f_int64_t l);
extern f_float32_t ConvertL2B_float(f_float32_t f);
extern f_float64_t ConvertL2B_double(f_float64_t d);
extern f_uint64_t	 ConvertL2B_ulong(f_uint64_t ul);
#endif

