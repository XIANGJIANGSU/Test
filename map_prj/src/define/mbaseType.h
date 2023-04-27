/**
 * @file mbaseType.h
 * @brief 该文件提供了一些基本的数据类型
 * @author 615地图团队 张仟新
 * @date 2016-07-05
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _mbaseType_h_ 
#define _mbaseType_h_ 

#ifndef WIN32
//#include "vxTypesOld.h"
#endif

#ifdef ACOREOS
#include <sysTypes.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
#include "esUtil.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

#ifdef _X64

typedef char				f_char_t;
typedef signed char			f_int8_t;
typedef signed short		f_int16_t;
typedef signed int			f_int32_t;
typedef signed long long	f_int64_t;
typedef unsigned char		f_uint8_t;
typedef unsigned short		f_uint16_t;
typedef unsigned int		f_uint32_t;
typedef unsigned long long	f_uint64_t;
typedef float				f_float32_t;
typedef double				f_float64_t;
typedef long double			f_float128_t;

typedef long long			SE_HANDLE;

#else

typedef char				f_char_t;
typedef signed char			f_int8_t;
typedef signed short		f_int16_t;
typedef signed int			f_int32_t;
typedef signed long			f_int64_t;
typedef unsigned char		f_uint8_t;
typedef unsigned short		f_uint16_t;
typedef unsigned int		f_uint32_t;
typedef unsigned long		f_uint64_t;
typedef float				f_float32_t;
typedef double				f_float64_t;
typedef long double			f_float128_t;

typedef int					SE_HANDLE;

#endif


typedef void * VOIDPtr;
typedef void (*FUNCTION)(VOIDPtr pscene);
typedef void (*USERLAYERFUNCTION)(VOIDPtr pscene);


#ifndef TRUE
#define  TRUE  1
#endif

#ifndef FALSE
#define  FALSE 0
#endif

#ifndef ACOREOS

#ifndef BOOL
#define  BOOL  f_int32_t
#endif

#endif

#ifndef NULL
	#define NULL	0
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
