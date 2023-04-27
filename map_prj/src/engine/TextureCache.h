/* TextureCache.h
纹理缓存，把纹理数据缓存在内存中，用于解决帧存不够用的问题
*/
#ifndef __TEXTURE_CACHE_H__
#define __TEXTURE_CACHE_H__

/* 是否使用纹理缓存 */
//#define USE_CACHE_TEXTURE


#ifndef USE_CACHE_TEXTURE
#define glextGenTextures                glGenTextures
#define glextDeleteTextures             glDeleteTextures
#define glextTexImage2D                 glTexImage2D
#define glextTexSubImage2D              glTexSubImage2D
#define glextCompressedTexImage2D       glCompressedTexImage2D
#else

#ifdef _WIN32
#include <windows.h>
#define tickGet		GetTickCount
#else
#include "vxWorks.h"
#include "tickLib.h"
#endif

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
#include "esUtil.h"
#else
#include "gl/gl.h"
#endif

void glextGenTextures( GLsizei n, GLuint *textures );
void glextDeleteTextures( GLsizei n, const GLuint *textures);
void glextTexImage2D( GLenum target, GLint level,
                            GLint internalFormat,
                            GLsizei width, GLsizei height,
                            GLint border, GLenum format, GLenum type,
                            const GLvoid *pixels );
void glextTexSubImage2D( GLenum target, GLint level,
                           GLint xoffset, GLint yoffset,
                           GLsizei width, GLsizei height,
                           GLenum format, GLenum type,
                           const GLvoid *pixels );

void glextCompressedTexImage2D( GLenum target, GLint level,
                            GLint internalFormat,
                            GLsizei width, GLsizei height,
                            GLint border, GLsizei imageSize,
                            const GLvoid *pixels );
                         
//GLint gluextBuild2DMipmaps(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, 
//						   GLenum format, GLenum type, const void *pixels);
                           
#endif  /*#ifndef USE_CACHE_TEXTURE*/


//#define glextBindTexture    glBindTexture
//#define glextTexParameteri  glTexParameteri
//#define gluextBuild2DMipmaps      gluBuild2DMipmaps

#endif 
