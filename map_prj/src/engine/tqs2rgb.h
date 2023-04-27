#ifndef __TQS2RGB__
#define __TQS2RGB__

#include "../define/macrodefine.h"
#include "../mapApi/common.h"

#define USE_TQS2RGB
//#undef USE_TQS2RGB

//! Typedef a quantity that is a single unsigned byte.
typedef f_uint8_t u8;

enum
{
	//! Use DXT1 compression.
	kDxt1 = ( 1 << 0 ), 

	//! Use DXT3 compression.
	kDxt3 = ( 1 << 1 ), 

	//! Use DXT5 compression.
	kDxt5 = ( 1 << 2 ), 

	//! Use a very slow but very high quality colour compressor.
	kColourIterativeClusterFit = ( 1 << 8 ),	

	//! Use a slow but high quality colour compressor (the default).
	kColourClusterFit = ( 1 << 3 ),	

	//! Use a fast but low quality colour compressor.
	kColourRangeFit	= ( 1 << 4 ),

	//! Use a perceptual metric for colour error (the default).
	kColourMetricPerceptual = ( 1 << 5 ),

	//! Use a uniform metric for colour error.
	kColourMetricUniform = ( 1 << 6 ),

	//! Weight the colour by alpha during cluster fit (disabled by default).
	kWeightColourByAlpha = ( 1 << 7 )
};

unsigned char* ConvertDDS2RGB(u8* pImage, const int size);

#endif
