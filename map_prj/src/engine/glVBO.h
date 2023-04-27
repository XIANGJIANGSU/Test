#include <string.h>
#include <stdlib.h>
#include "libList.h"
#include "../define/mbaseType.h"

void  glavicVertex3f (GLfloat x, GLfloat y, GLfloat z);
void  glavicTexCoord2f (GLfloat s, GLfloat t);

void  glavicNewList (GLuint list, GLenum mode);
void  glavicEndList (GLuint list);
void  glavicDeleteLists (GLuint list, GLsizei range);
GLuint  glavicGenLists (GLsizei range);

GLboolean  glavicIsList (GLuint list);

void  glavicBegin (GLenum mode);
void  glavicEnd (void);

void  glavicCallList (GLuint list);

void glavicVertex3i (GLint x, GLint y, GLint z);



