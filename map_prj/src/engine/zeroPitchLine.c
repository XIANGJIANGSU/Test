#include <math.h>
#include <stdio.h>

#include "../define/macrodefine.h"
#include "zeroPitchLine.h"
#include "imageBmp.h"

#define ZPLRADIU 20000.0f
#define ZEROPITCHLINEHEIGHT 1200.0f
#define DELTA_TEXTURE_COORD 0.00537109375f+0.25f

static void zplListPre(GLuint texId)
{
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnableEx(GL_TEXTURE_2D);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glDisableEx(GL_TEXTURE_1D);
	glBindTexture(GL_TEXTURE_2D, texId);	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	
#else
	glBindTexture(GL_TEXTURE_2D, texId);	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
}

static void zplListPro()
{
	glDisableEx(GL_TEXTURE_2D);
	glDisableEx(GL_BLEND);
}

/**
 * @fn f_int32_t zplListCreate(void)
 *  @brief ����0�������ߵ���ʾ�б�.
 *  @param[in] void.   
 *  @exception void.
 *  @return f_int32_t ��ʾ�б�ID.
 * @see 
 * @note
*/
f_int32_t zplListCreate(void)
{
	PT_2F pointArray[361];
	f_int32_t i = 0, list = 0;	
	int h = ZEROPITCHLINEHEIGHT;
	
	for(i = 0; i <= 360; i++)
	{	
		pointArray[i].x = ZPLRADIU*sin(i*DE2RA);
		pointArray[i].y = ZPLRADIU*cos(i*DE2RA);
	}
	
	list = glGenLists(1);	
	if(0 != list)
	{
		glNewList(list, GL_COMPILE);			
			glBegin(GL_TRIANGLE_STRIP);
				for(i = 0; i <= 360; i++)
				{					
					glTexCoord2f(i/360.0f+DELTA_TEXTURE_COORD, 0.99f); glVertex3f(pointArray[i].y, h, pointArray[i].x);
					glTexCoord2f(i/360.0f+DELTA_TEXTURE_COORD, 0.0f); glVertex3f(pointArray[i].y, 0.0f, pointArray[i].x);
				}
			glEnd();
		glEndList();		
	}
    return(list);
}

/**
 * @fn f_int32_t renderZeroPitchLine(Matrix44 mxWorld, f_int32_t list_id, f_int32_t texture_id)
 *  @brief ����0��������.
 *  @param[in] mxWorld ������ģ�Ӿ���. 
 *  @param[in] color   ��ɫ. 
 *  @param[in] list_id 0����������ʾ�б�ID��. 
 *  @param[in] texture_id 0������������ID��.  
 *  @exception void.
 *  @return f_int32_t �����ɹ���־.
 *  @retval  0 �����ɹ�.
 *  @retval -1 ����ʧ��.
 * @see 
 * @note
*/
f_int32_t renderZeroPitchLine(Matrix44 mxWorld, sColor3f color, f_int32_t list_id, f_int32_t texture_id)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	glMultMatrixd((f_float64_t *)(&mxWorld));

	if(0 == list_id)
	{
		DEBUG_PRINT("0 == list_id");
		glPopMatrix();	
        return -1;
    }
	
	if(0 != texture_id)
	    zplListPre(texture_id);
	else
	{
		glPopMatrix();	
	    return -2;
	}
	    
	glColor3f(color.red, color.green, color.blue);
	glCallList(list_id);	
	zplListPro();
	
	glPopMatrix();	
	return 0;
}

/**
 * @fn f_int32_t createZeroPitchLineTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
 *  @brief ����0�������ߵ�����.
 *  @param[in] textFileName 0������������ͼ����ȫ�ļ���.
 *  @param[out] ptexture_id  0�������ߵ�����ID. 
 *  @exception void.
 *  @return f_int32_t �����ɹ���־.
 *  @retval  0 �����ɹ�.
 *  @retval -1 ����ʧ��.
 * @see 
 * @note
*/
f_int32_t createZeroPitchLineTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
{
	f_int32_t ret = -1;
    ret = createBmpTexture(textFileName, ptexture_id);
	if(0 != ret)
		return(-1);
		
	return(0);
}
