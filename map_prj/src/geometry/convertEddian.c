#include "convertEddian.h"

f_int16_t ConvertL2B_short(f_int16_t s)
{
	SHORT_CONV d1, d2;
	d1.f = s; 
	d2.c[0] = d1.c[1];
	d2.c[1] = d1.c[0];
	return d2.f;
}

f_uint16_t ConvertL2B_ushort(f_uint16_t s)
{
	UNSIGNED_SHORT_CONV d1, d2;
	d1.f = s; 
	d2.c[0] = d1.c[1];
	d2.c[1] = d1.c[0];
	return d2.f;
}

f_uint32_t ConvertL2B_uint(f_uint32_t i)
{
	INT_CONV d1, d2;
	d1.f = i; 
	d2.c[0] = d1.c[3];
	d2.c[1] = d1.c[2];
	d2.c[2] = d1.c[1];
	d2.c[3] = d1.c[0];
	return d2.f;
}

f_int32_t	ConvertL2B_int(f_int32_t i)
{
	INT_CONV d1, d2;
	d1.f = i; 
	d2.c[0] = d1.c[3];
	d2.c[1] = d1.c[2];
	d2.c[2] = d1.c[1];
	d2.c[3] = d1.c[0];
	return d2.f;
}

f_int64_t ConvertL2B_long(f_int64_t l)
{
	LONG_CONV d1, d2;
	d1.f = l; 
	d2.c[0] = d1.c[3];
	d2.c[1] = d1.c[2];
	d2.c[2] = d1.c[1];
	d2.c[3] = d1.c[0];
	return d2.f;
}

f_uint64_t ConvertL2B_ulong(f_uint64_t ul)
{
	UNSIGNED_LONG_CONV d1, d2;
	d1.f = ul; 
	d2.c[0] = d1.c[3];
	d2.c[1] = d1.c[2];
	d2.c[2] = d1.c[1];
	d2.c[3] = d1.c[0];
	return d2.f;
}

f_float32_t ConvertL2B_float(f_float32_t f)
{
	FLOAT_CONV d1, d2;
	d1.f = f; 
	d2.c[0] = d1.c[3];
	d2.c[1] = d1.c[2];
	d2.c[2] = d1.c[1];
	d2.c[3] = d1.c[0];
	return d2.f;
}

f_float64_t ConvertL2B_double(f_float64_t d)
{
	DOUBLE_CONV d1, d2;
	d1.d = d; 
	d2.c[0] = d1.c[7];
	d2.c[1] = d1.c[6];
	d2.c[2] = d1.c[5];
	d2.c[3] = d1.c[4];
	d2.c[4] = d1.c[3];
	d2.c[5] = d1.c[2];
	d2.c[6] = d1.c[1];
	d2.c[7] = d1.c[0];
	return d2.d;
}

