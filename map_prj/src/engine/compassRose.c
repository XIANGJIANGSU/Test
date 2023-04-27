#include <math.h>
#include <stdio.h>

#include "../define/macrodefine.h"
#include "../mapApp/appHead.h"
#include "../engine/mapRender.h"
#include "compassRose.h"
#include "imageBmp.h"

extern sSCREENPARAM screen_param;

static void cpsListPre(f_float32_t yaw, f_float32_t view_lb_x, f_float32_t view_lb_y,
                       f_float32_t cent_x, f_float32_t cent_y, f_float32_t width, f_float32_t height, GLuint texId)
{
	glDisableEx(GL_DEPTH_TEST);
	glViewport(view_lb_x + cent_x - width/2, view_lb_y + cent_y - height/2, width, height);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	glLoadIdentity();
	glTranslatef(width * 0.5, height * 0.5, 0.0f);
	//glRotatef(yaw, 0.0, 0.0f, 1.0f);
	switch(screen_param.rotate_type)
	{   /* 屏幕旋转 */
	    default:
	    case eROTATE_CW_0:
	        /* 默认模式为正常模式，屏幕不旋转 */
	        glRotatef(yaw, 0.0, 0.0f, 1.0f);
	    break;
	    case eROTATE_CW_90:
	        glRotatef(yaw + 90.0, 0.0, 0.0, 1.0);
	    break;
	    case eROTATE_CW_180:
	        glRotatef(yaw + 180.0, 0.0, 0.0, 1.0);
	    break;
	    case eROTATE_CW_270:
	        glRotatef(yaw + 270.0, 0.0, 0.0, 1.0);
	    break;
	}	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glOrtho(0.0, width, 0.0, height, 1.0f, -1.0f);
	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnableEx(GL_TEXTURE_2D);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glDisableEx(GL_TEXTURE_1D);
#endif
	glBindTexture(GL_TEXTURE_2D, texId);	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

static void cpsListPro()
{
	glDisableEx(GL_TEXTURE_2D);
	glDisableEx(GL_BLEND);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/**
 * @fn f_int32_t cpsListCreate(f_float32_t lb_x, f_float32_t lb_y, f_float32_t rt_x, f_float32_t rt_y)
 *  @brief 创建罗盘的显示列表.
 *  @param[in] lb_x, lb_y, rt_x, rt_y  罗盘的位置(相对于罗盘中心点). 
 *  @exception void.
 *  @return f_int32_t 显示列表ID.
 * @see 
 * @note
*/
f_int32_t cpsListCreate(f_float32_t lb_x, f_float32_t lb_y, f_float32_t rt_x, f_float32_t rt_y)
{
	f_int32_t list = 0;
	list = glGenLists(1);	
	if(0 != list)
	{
		glNewList(list, GL_COMPILE);			
		glBegin(GL_QUADS);		
#if 0		//修改顺序为逆时针,lpf add 2017-6-6 16:25:27
		
		    glTexCoord2f(0.0f, 1.0f); glVertex3f(lb_x, rt_y, 0.0f);
		    glTexCoord2f(1.0f, 1.0f); glVertex3f(rt_x, rt_y, 0.0f);
		    glTexCoord2f(1.0f, 0.0f); glVertex3f(rt_x, lb_y, 0.0f);
		    glTexCoord2f(0.0f, 0.0f); glVertex3f(lb_x, lb_y, 0.0f);
#else
		
		    glTexCoord2f(0.0f, 0.0f); glVertex3f(lb_x, lb_y, 0.0f);

		    glTexCoord2f(1.0f, 0.0f); glVertex3f(rt_x, lb_y, 0.0f);

		    glTexCoord2f(1.0f, 1.0f); glVertex3f(rt_x, rt_y, 0.0f);

		    glTexCoord2f(0.0f, 1.0f); glVertex3f(lb_x, rt_y, 0.0f);

#endif
	        glEnd();
		glEndList();		
	}
    return(list);
}

/**
 * @fn f_int32_t renderCompassRose(f_float32_t yaw, f_float32_t view_lb_x, f_float32_t view_lb_y, \n
                                   f_float32_t cent_x, f_float32_t cent_y, f_float32_t width, f_float32_t height,\n
                                   sColor3f color, f_int32_t list_id, f_int32_t texture_id)
 *  @brief 绘制罗盘.
 *  @param[in] yaw     航向. 
 *  @param[in] cent_x, cent_y     罗盘中心点位置（相对于当前视口的左下角）
 *  @param[in] width, height      罗盘宽高
 *  @param[in] color   颜色. 
 *  @param[in] list_id 罗盘显示列表ID号. 
 *  @param[in] texture_id 罗盘纹理ID号.  
 *  @exception void.
 *  @return f_int32_t 创建成功标志.
 *  @retval  0 创建成功.
 *  @retval -1 创建失败.
 * @see 
 * @note
*/
f_int32_t renderCompassRose(f_float32_t yaw, f_float32_t view_lb_x, f_float32_t view_lb_y,
                            f_float32_t cent_x, f_float32_t cent_y, f_float32_t width, f_float32_t height,
                            sColor3f color, f_int32_t list_id, f_int32_t texture_id)
{
	if(0 == list_id)
	{
		DEBUG_PRINT("0 == list_id");
        return -1;
    }
	
	if(0 != texture_id)
	{    cpsListPre(yaw, view_lb_x, view_lb_y, cent_x, cent_y, width, height, texture_id);}
	else
	    return -2;
	    
	glColor3f(color.red, color.green, color.blue);
	glCallList(list_id);	
	cpsListPro();
	
	return 0;
}

/**
 * @fn f_int32_t createCompassRoseTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
 *  @brief 创建罗盘.
 *  @param[in] textFileName 罗盘纹理图的完全文件名.
 *  @param[out] ptexture_id     罗盘的纹理ID. 
 *  @exception -1:纹理文件加载失败，-2:纹理创建失败.
 *  @return f_int32_t 创建成功标志.
 *  @retval  0 创建成功.
 *  @retval -1 创建失败.
 * @see 
 * @note
*/
f_int32_t createCompassRoseTexture(f_int8_t* textFileName, f_int32_t *ptexture_id)
{
	f_int32_t ret = -1;
    ret = createBmpTexture(textFileName, ptexture_id);
	if(0 != ret)
		return(-1);
		
	return(0);
}
