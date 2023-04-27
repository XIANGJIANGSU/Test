#include "tqs2rgb.h"
#include "memoryPool.h"


static int FixFlags( int flags )
{
	// grab the flag bits
	int method = 0;
	int fit = 0;
	int metric = 0;
	int extra = 0;

	method = flags & ( kDxt1 | kDxt3 | kDxt5 );
	fit = flags & ( kColourIterativeClusterFit | kColourClusterFit | kColourRangeFit );
	metric = flags & ( kColourMetricPerceptual | kColourMetricUniform );
	extra = flags & kWeightColourByAlpha;

	// set defaults
	if( method != kDxt3 && method != kDxt5 )
		method = kDxt1;
	if( fit != kColourRangeFit )
		fit = kColourClusterFit;
	if( metric != kColourMetricUniform )
		metric = kColourMetricPerceptual;

	// done
	return method | fit | metric | extra;
}

static int Unpack565( u8 const* packed, u8* colour )
{
	// build the packed value
	int value = ( int )packed[0] | ( ( int )packed[1] << 8 );

	// get the components in the stored range
	u8 red = ( u8 )( ( value >> 11 ) & 0x1f );
	u8 green = ( u8 )( ( value >> 5 ) & 0x3f );
	u8 blue = ( u8 )( value & 0x1f );

	// scale up to 8 bits
	colour[0] = ( red << 3 ) | ( red >> 2 );
	colour[1] = ( green << 2 ) | ( green >> 4 );
	colour[2] = ( blue << 3 ) | ( blue >> 2 );
	colour[3] = 255;

	// return the value
	return value;
}

void DecompressColour( u8* rgba, void const* block, BOOL isDxt1 )
{
	// get the block bytes
	u8 const* bytes =  (u8 const*)block ;
	int i;
	// unpack the endpoints
	u8 codes[16];
	int a = Unpack565( bytes, codes );
	int b = Unpack565( bytes + 2, codes + 4 );

	// generate the midpoints
	for(i = 0; i < 3; ++i )
	{
		int c = codes[i];
		int d = codes[4 + i];

		if( isDxt1 && a <= b )
		{
			codes[8 + i] = ( u8 )( ( c + d )/2 );
			codes[12 + i] = 0;
		}
		else
		{
			codes[8 + i] = ( u8 )( ( 2*c + d )/3 );
			codes[12 + i] = ( u8 )( ( c + 2*d )/3 );
		}
	}

	// fill in alpha for the intermediate values
	codes[8 + 3] = 255;
	codes[12 + 3] = ( isDxt1 && a <= b ) ? 0 : 255;

	{
		// unpack the indices
		u8 indices[16];
		int i, j;
		for(i = 0; i < 4; ++i )
		{
			u8* ind = indices + 4*i;
			u8 packed = bytes[4 + i];

			ind[0] = packed & 0x3;
			ind[1] = ( packed >> 2 ) & 0x3;
			ind[2] = ( packed >> 4 ) & 0x3;
			ind[3] = ( packed >> 6 ) & 0x3;
		}

		// store out the colours
		for(i = 0; i < 16; ++i )
		{
			u8 offset = 4*indices[i];
			for( j = 0; j < 4; ++j )
				rgba[4*i + j] = codes[offset + j];
		}
	}
}

static void Decompress( u8* rgba, void const* block, int flags )
{
	// get the block locations
	void const* colourBlock = block;

	// fix any bad flags
	flags = FixFlags( flags );

	// decompress colour
	DecompressColour( rgba, colourBlock, ( flags & kDxt1 ) != 0 );	
}

unsigned char* ConvertDDS2RGB(u8* pImage, const int size)
{
	int x = 0, y = 0, px = 0, py = 0, i = 0, j = 0;
	unsigned char * newImage = NULL;
	unsigned char targetRgba[16*4];
	unsigned char* row;
	int width, height;
	int bytesPerBlock;
	unsigned char * sourceBlock = NULL;

	//newImage = (unsigned char*)malloc( sizeof(unsigned char) * size * size * 3 );
	newImage = (unsigned char*)NewAlterableMemory( sizeof(unsigned char) * size * size * 3 );
	if(NULL == newImage)
	{
		return NULL;
	}
	width = size;
	height = size;
	bytesPerBlock = 8;
	sourceBlock = pImage;

	// DDS压缩的纹理4*4为一组压缩
	for( y = 0; y < height; y += 4 )	
	{
		// process a row of blocks
		for( x = 0; x < width; x += 4 )	
		{
			// 从DDS纹理解压缩到RGBA decompress back					
			Decompress( targetRgba, sourceBlock, 1 );	// 解析当前组RGBA

			// write the data into the target rows
			for( py = 0, i = 0; py < 4; ++py )		// 对于当前组的每一行
			{
				//u8* row = ( u8* )targetRows.Get()[y + py] + x*4;
				row = (unsigned char *)((unsigned long)newImage + ((y + py) * width + x) * 3); // &(pBuf3[((y + py) * width + x) * 3]);
				for( px = 0; px < 4; ++px, ++i )	// 对于当前组的每一列
				{	
					for( j = 0; j < 3; ++j )		// 对于当前像素
						*row++ = targetRgba[4*i + j];
				}
			}
			// advance
			sourceBlock += bytesPerBlock;
		}
	}
	return newImage;
}