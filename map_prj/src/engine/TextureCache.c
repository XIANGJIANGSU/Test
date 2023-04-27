/* TextureCache.c - 纹理缓存控制
用于控制放入GPU帧存中的纹理不会太多，导致帧存不足
*/
#include "TextureCache.h"
#ifdef USE_CACHE_TEXTURE

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
#include "esUtil.h"
#else
#include "gl/gl.h"
#include "gl/glu.h"
#include "gl/glext.h"
#endif
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define USE_COMPRESSED_TEXTURE
#define TEX_DYNAMIC_UPDATE


/* 缓存的纹理的最大数量 */
#define NODE_MAX			(256)
#define SIZE_INIT			80
#define SIZE_GROWUP			8

#define TEXW		256
#define TEXH		256

typedef struct tagTEXTURE_CACHE_ITEM
{
	GLuint		texid;
	GLboolean	busy;
} TEXTURE_CACHE_ITEM;

static TEXTURE_CACHE_ITEM g_cacheTex[NODE_MAX] = {{0}};
static int g_nBufferCount = 0;

void texcachePrint()
{
	int		i, busynum=0;
	printf("texcount: %d\n", g_nBufferCount);
	for (i=0; i<g_nBufferCount; ++i)
	{
		if ( g_cacheTex[i].busy )
			busynum++;
	}
	printf("busy count: %d\n", busynum);
}

static void cacheGrowup(int count)
{
#ifdef TEX_DYNAMIC_UPDATE
	int		i;
	GLubyte	*pBits = NULL;
	TEXTURE_CACHE_ITEM	*pItem = NULL;

	assert(g_nBufferCount+count <= NODE_MAX);
	pBits = malloc(TEXW * TEXH * 3);
	memset(pBits, 0, TEXW * TEXH * 3);
	for (i=0; i<count; ++i)
	{
		pItem = &g_cacheTex[g_nBufferCount+i];
		pItem->busy = 0;
		glGenTextures(1, &pItem->texid);
		if ( 0 == pItem->texid )
			printf("cacheGrowup: texture failed,pos=%d\n", i);
		glBindTexture(GL_TEXTURE_2D, pItem->texid);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef USE_COMPRESSED_TEXTURE
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 
					TEXW, TEXH, 0, TEXW*TEXH/2, pBits);
#else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TEXW, TEXH, 0, GL_RGB, GL_UNSIGNED_BYTE, pBits);
#endif
	}
	free(pBits);
	printf("%s:buffered texture created: %d-->%d\n", __FUNCTION__, g_nBufferCount, g_nBufferCount+count);
	g_nBufferCount += count;
#endif
}

void texcacheInit()
{
	cacheGrowup(SIZE_INIT);
}

static int texcacheFindIdle()
{
	int		i;
	for (i=0; i<g_nBufferCount; ++i)
	{
		if ( 0 == g_cacheTex[i].busy )
			return i;
	}
	assert(g_nBufferCount+SIZE_GROWUP <= NODE_MAX);
	cacheGrowup(SIZE_GROWUP);
	return i;
}

static int texcacheFindTexture(GLuint texture)
{
	int		i;
	for (i=0; i<g_nBufferCount; ++i)
	{
		if ( texture == g_cacheTex[i].texid )
			return i;
	}
	return -1;
}

void glextGenTextures( GLsizei n, GLuint *textures )
{
#ifdef TEX_DYNAMIC_UPDATE
	int		i;
	TEXTURE_CACHE_ITEM	*pItem = NULL;
	for (i=0; i<n; i++)
	{
		pItem = &g_cacheTex[texcacheFindIdle()];
		textures[i] = pItem->texid;
		pItem->busy = 1;
	}
#else
	glGenTextures(n, textures);
#endif
}

/* 缓存方式下删除纹理 */
void glextDeleteTextures( GLsizei n, const GLuint *textures)
{
#ifdef TEX_DYNAMIC_UPDATE
	int		i, ind;

	for (i=0; i<n; ++i)
	{
		if ( 0 == textures[i] )
			continue;
		ind = texcacheFindTexture(textures[i]);
		if ( ind < 0 )
		{
			printf("%s: %d\n", __FUNCTION__, textures[i]);
			assert(ind >= 0);
		}
		g_cacheTex[ind].busy = 0;
	}
#else
	glDeleteTextures(n, textures);
#endif
}

static const GLubyte* TexDataFormat(GLenum format, const GLubyte *pixels)
{
	static GLubyte	*pBuf = NULL;
	if ( NULL == pBuf )
	{
		pBuf = malloc(TEXW * TEXH * 3);
		memset(pBuf, 0, TEXW * TEXH * 3);
	}
	if ( GL_RGB == format )
		return pixels;
	
	assert(GL_RGBA == format);
	
	/* rgba --> rgb */
	{
		int			i;
		GLubyte		*pdst;
		pdst = pBuf;
		for (i=0; i<TEXW*TEXH; ++i)
		{
			pdst[0] = pixels[0];
			pdst[1] = pixels[1];
			pdst[2] = pixels[2];
			pdst += 3;
			pixels += 4;
		}
	}

	return pBuf;
}

void glextTexSubImage2D( GLenum target, GLint level,
                           GLint xoffset, GLint yoffset,
                           GLsizei width, GLsizei height,
                           GLenum format, GLenum type,
                           const GLvoid *pixels )
{
#ifdef TEX_DYNAMIC_UPDATE
#ifdef USE_COMPRESSED_TEXTURE
	glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height,
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width*height/2, pixels);
#else
	const GLubyte	*pBits = NULL;
	pBits = TexDataFormat(format, (const GLubyte*)pixels);
	glTexSubImage2D(target, level, xoffset,yoffset, width, height, GL_RGB, type, pBits);
#endif
#endif
}

void glextTexImage2D( GLenum target, GLint level,
                            GLint internalFormat,
                            GLsizei width, GLsizei height,
                            GLint border, GLenum format, GLenum type,
                            const GLvoid *pixels )
{
#ifdef TEX_DYNAMIC_UPDATE
#ifdef USE_COMPRESSED_TEXTURE
//	glCompressedTexSubImage2D(target, level, 0, 0, width, height,
//				GL_COMPRESSED_RGB_S3TC_DXT1_EXT, width*height/2, pixels);
#else
	glextTexSubImage2D(target, level, 0,0, width, height, format, type, pixels);
#endif
#else
	glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
#endif
}


void glextCompressedTexImage2D( GLenum target, GLint level,
                            GLint internalFormat,
                            GLsizei width, GLsizei height,
                            GLint border, GLsizei imageSize,
                            const GLvoid *pixels )
{
#ifdef TEX_DYNAMIC_UPDATE
	glCompressedTexSubImage2D(target, level, 
			0, 0, width, height, internalFormat, imageSize, pixels);
#elif defined(USE_COMPRESSED_TEXTURE)
	glCompressedTexImage2D(target, level, internalFormat,
			width, height, border, imageSize, pixels);
#endif
}



#else
void texcacheInit() {}
#endif  /* #ifdef USE_CACHE_TEXTURE */
