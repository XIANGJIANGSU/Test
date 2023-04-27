#ifdef WIN32

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)

#include <string.h>
#include <stdlib.h>
#include "libList.h"
#include <ASSERT.H>

#include "mapRender.h"


// function pointers for VBO Extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;	
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;	
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = NULL;

void vboFunInit(void)
{
	glGenBuffersARB 
		= (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");


	glBindBufferARB 
		= (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");


	glBufferDataARB 
		= (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");


	glDeleteBuffersARB 
		= (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");


	glEnableVertexAttribArray 
		= (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");

	glVertexAttribPointer 
		= (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");

	glBufferSubDataARB 
		= (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");

}


#if defined ACOREOS || defined SYLIXOS
void glBindBufferARB(GLenum target, GLuint buffer)
{
    return;
}

void glDeleteBuffersARB(GLsizei n, const GLuint *buffers)
{
    return;
}

void glGenBuffersARB(GLsizei n, GLuint *buffers)
{
    return;
}

void glBufferDataARB (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage)
{
    return;
}

void glBufferSubDataARB (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data)
{
    return;
}
#endif

static GLfloat va[65535*3*4];
static GLfloat ta[65535*2*4];
static GLushort ia[65535*3*4];

int MemoryCount = 0;

typedef struct tagDrawable{
//	GLfloat  color[4];
//	GLint    colorFlag;
	GLenum   type;
	GLuint   vboIds[3];
	GLuint   vertexCount;
	GLuint   indexCount;
	GLfloat vertexArray[index_count * 3] ;
	GLfloat texCoordArray[index_count * 2];
	GLushort indexArray[index_count];
}stDrawable;


typedef struct tagDrawableStatus{
	stDrawable data;
	int busy;		// 1:在用，0:空闲
}stDrawableStatue;



#define vbocachenum 512
static stDrawableStatue VboCache[vbocachenum];
static int g_drawable_num = vbocachenum;

static stDrawable *curDrawableNode = NULL;
static int VboInit = 0;

void initVertexBufferObjects(stDrawable* draw)
{	
	GLuint* vboIds = draw->vboIds;
	GLuint numVertices = index_count;
	GLuint numIndex    = index_count;


//	draw->vertexCount = index_count;
//	draw->indexCount = index_count;

	(numIndex==0)?numIndex++:numIndex;


	glGenBuffersARB(3, vboIds);
	
#if 0
	glBindBufferARB(GL_ARRAY_BUFFER, vboIds[0]);
	glBufferDataARB(GL_ARRAY_BUFFER, numVertices * sizeof(GLfloat)*3, draw->vertexArray, GL_STATIC_DRAW);

	glBindBufferARB(GL_ARRAY_BUFFER, vboIds[1]);
	glBufferDataARB(GL_ARRAY_BUFFER,	numVertices * sizeof(GLfloat)*2, draw->texCoordArray,	GL_STATIC_DRAW);
	
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vboIds[2]);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, numIndex * sizeof(GLushort), draw->indexArray, GL_STATIC_DRAW);
	
#else
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboIds[0]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, numVertices * sizeof(GLfloat)*3, /*draw->vertexArray*/ NULL, GL_STATIC_DRAW_ARB);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboIds[1]);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,	numVertices * sizeof(GLfloat)*2, /*draw->texCoordArray*/ NULL,	GL_STATIC_DRAW_ARB);
	
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIds[2]);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, numIndex * sizeof(GLushort), /*draw->indexArray*/NULL, GL_STATIC_DRAW_ARB);
		
	
#endif


}



//初始化VBO 池
void InitVboCache()
{
	int i = 0;

	for(i=0; i<vbocachenum; i++)
	{
		initVertexBufferObjects(&(VboCache[i].data));
	}
}



GLboolean  glavicIsList (GLuint list)
{
	if((list < vbocachenum)&&(list > 0))
		return GL_TRUE;
	else
		return GL_FALSE;
}







void drawDrawable(stDrawable* draw)
{

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, draw->vboIds[0]);
// 	glEnableVertexAttribArray(aPositionLoc);	
// 	glVertexAttribPointer(aPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, draw->vboIds[1]);
// 	glEnableVertexAttribArray(aTexCoordLoc);	
// 	glVertexAttribPointer(aTexCoordLoc,	2, GL_FLOAT, GL_FALSE, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);



		
	if(draw->type == GL_QUADS)
	{
	//	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, draw->vboIds[2]);
	//	glDrawElements(GL_TRIANGLES, draw->indexCount, GL_UNSIGNED_SHORT, 0);
	}else
	{
		glDrawArrays ( draw->type, 0, draw->vertexCount);
	}

	

//glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

}


void  glavicBegin (GLenum mode)
{
//	memset(&curDrawableNode, 0, sizeof(stDrawable));
	curDrawableNode->type = mode;
}

void  glavicEnd (void)
{
// 	if(NULL != curDrawableNode)
// 	{
// 		createDrawableBuffer(curDrawableNode);
// 		LIST_ADD(&curDrawableNode->node, &(curListNode->listData.head));
// 		curDrawableNode = NULL;
// 	}

	
}

void  glavicCallList (GLuint list)
{

	stDrawable * draw = NULL; 
	
	if(GL_TRUE != glavicIsList(list))
		return;

	draw = &(VboCache[list].data);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	

	drawDrawable(draw);


	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);		
}



GLuint  glavicGenLists (GLsizei range)
{
	int i = 0;
	
	if(1 != range)  //did not support range>1
		return 0;

	if(VboInit == 0)
	{
#ifdef WIN32
		vboFunInit();
#endif
		InitVboCache();
		
		VboInit = 1;
	}
	
		
	for(i = 1; i<vbocachenum; i++)
	{
		if(VboCache[i].busy == 0)
		{	
			VboCache[i].busy = 1;
			return i;
		}
	}

	assert(i > 0);
	return -1;


	
}


void  glavicDeleteLists (GLuint list, GLsizei range)
{
	
	if(1 != range)  //did not support range>1
		return ;


	VboCache[list].busy = 0;
	VboCache[list].data.indexCount = 0;
	VboCache[list].data.vertexCount = 0;
}

void  glavicNewList (GLuint list, GLenum mode)
{
	curDrawableNode =  &(VboCache[list].data);

}

void  glavicEndList (GLuint list)
{

	stDrawable * draw = curDrawableNode;

	GLuint* vboIds = draw->vboIds;
	GLuint numVertices = draw->vertexCount;
	GLuint numIndex    = draw->indexCount;
	(numIndex==0)?numIndex++:numIndex;
#if 0
	glBindBufferARB(GL_ARRAY_BUFFER, vboIds[0]);
	glBufferSubDataARB(GL_ARRAY_BUFFER, 0,numVertices * sizeof(GLfloat)*3, draw->vertexArray);

	glBindBufferARB(GL_ARRAY_BUFFER, vboIds[1]);
	glBufferSubDataARB(GL_ARRAY_BUFFER,	0,numVertices * sizeof(GLfloat)*2, draw->texCoordArray);
	
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vboIds[2]);
	glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER, 0,numIndex * sizeof(GLushort), draw->indexArray);
#else
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboIds[0]);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0,numVertices * sizeof(GLfloat)*3, draw->vertexArray);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboIds[1]);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,	0,numVertices * sizeof(GLfloat)*2, draw->texCoordArray);
	
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIds[2]);
	glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0,numIndex * sizeof(GLushort), draw->indexArray);	
	
	
#endif
	curDrawableNode = NULL;
	
}




void  glavicTexCoord2f (GLfloat s, GLfloat t)
{
	if(NULL == curDrawableNode)
		return;

	curDrawableNode->texCoordArray[curDrawableNode->vertexCount*2] = s;
	curDrawableNode->texCoordArray[curDrawableNode->vertexCount*2+1] = t;

}


void  glavicVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
	int vcount = 0;

	if(NULL == curDrawableNode)
		return;
	vcount = curDrawableNode->vertexCount;
	curDrawableNode->vertexArray[vcount*3] = x;
	curDrawableNode->vertexArray[vcount*3+1] = y;
	curDrawableNode->vertexArray[vcount*3+2] = z;
	curDrawableNode->vertexCount++;
	
	if(GL_QUADS == curDrawableNode->type)
	{
		if(curDrawableNode->vertexCount % 4 == 0)
		{
			curDrawableNode->indexArray[curDrawableNode->indexCount] = vcount-3;
			curDrawableNode->indexArray[curDrawableNode->indexCount+1] = vcount-1;
			curDrawableNode->indexArray[curDrawableNode->indexCount+2] = vcount;
			curDrawableNode->indexCount+=3;
		}else
		{
			curDrawableNode->indexArray[curDrawableNode->indexCount] = vcount;
			curDrawableNode->indexCount++;
		}
	}
}


void glavicVertex3i (GLint x, GLint y, GLint z)
{
	glavicVertex3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
}

#endif

#endif
