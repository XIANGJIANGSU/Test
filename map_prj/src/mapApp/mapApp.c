#ifdef WIN32
//#include <time.h>
#endif

#include <math.h>
#include "../mapApi/common.h"
#include "../engine/mapRender.h"
#include "../engine/atmosphere.h"
#include "../engine/zeroPitchLine.h"
#include "../engine/compassRose.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"
#include "../projection/coord.h"
#include "../camera/camera3d.h"
#include "flyParam.h"
#include "flyCmd.h"
#include "mapApp.h"
#include "paramGet.h"
#include "../engine/Roam.h"
#include "../models/simpleModels.h"
#include "../engine/mapRender.h"
#include "2dmapSetApi.h"
#include "../engine/TerrainPrefile.h"
#include "../engine/shadeMap.h"
#include "../define/macrodefine.h"
#include "../engine/TerrainMap.h"
#include "../mapApi/paramGetApi.h"
#include "../../../SE_api_prj/src/SEmapApi.h"

extern void EnableLight(BOOL bEnable);
extern void EnableLight0(BOOL bEnable);
extern void EnableLight1(BOOL bEnable);
extern BOOL drawGeoUserLayerPre(VOIDPtr pscene);
extern void drawUserLayerPro(void);
extern void StartRoam(RoamT* pRoam, int isRoam, double* mv, int* viewport, float origYaw, sGLRENDERSCENE *pscene);
extern void setVecLineAreaShowValue(int value,int viewIndex);
extern void setViewXYZType(double x, double y, double z, int isScaleView);

/*
1. 屏幕、绘制单元、视窗及场景之间的关系:
    一个显示器只有一个屏幕，对应多个绘制单元；
    一个绘制单元包含一个视窗和一个场景；
    任一场景可以任意attach到任一视窗中。
*/

sSCREENPARAM screen_param;
/*内部屏幕坐标系下，即屏幕正常摆放(未旋转)时的宽度和高度*/
WDSINZE_2I   window_size;

/*3d漫游时是否穿地*/
int roamScale = 0;

/*地图场景树*/
extern sMAPHANDLE * pMapDataSubTree;			
extern void CalAndSetVecMatrix(int viewPortIndex, double *modelview_matrix, double *prj_matrix, double *neardis, double *fardis);

void Hgt2Measure(sGLRENDERSCENE *pScene, double * measure_dis);
static void updateCameraParamDIY(sGLRENDERSCENE *pScene);
static void calCurMatrix(sGLRENDERSCENE *pScene);
void calTruepos(VOIDPtr scene, LP_PT_3D out_xyz, LP_Geo_Pt_D out_lonlathei);

/*
功能：绘制飞机符号
输入：
	pScene 场景句柄
输出：
    0-成功  -1-失败
*/
int drawPlaneSymbol(sGLRENDERSCENE *pScene);

/**
 * @fn void setScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height,\n
                           f_int32_t width, f_int32_t height)
 *  @brief 初始化显示器的参数.
 *  @param[in] rotate_type  显示器旋方式，0:正常,1:顺时旋转90度,2:顺时针旋转180度,3:顺时针旋转270度. 
 *  @param[in] phi_width    显示器物理尺寸的宽度(单位：厘米). 
 *  @param[in] phi_height   显示器物理尺寸的高度(单位：厘米). 
 *  @param[in] width        显示器分辨率尺寸的宽度(单位：像素). 
 *  @param[in] height       显示器分辨率尺寸的高度(单位：像素). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height, 
                    f_int32_t width, f_int32_t height)
{	
	screen_param.rotate_type  = rotate_type;
	screen_param.phi_width    = phi_width;
	screen_param.phi_height   = phi_height;	
	screen_param.width        = width;
	screen_param.height       = height;
}

/**
 * @fn void getScreenRotateType(f_int32_t *protatetype)
 *  @brief 获取屏幕旋转方式. 0:正常,1:顺时旋转90度,2:顺时针旋转180度,3:顺时针旋转270度.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenRotateType(f_int32_t *protatetype)
{
	if(NULL == protatetype)
	{
	    DEBUG_PRINT("NULL == protatetype");	
	    return;
	}
	
	*protatetype  = screen_param.rotate_type;
}

/**
 * @fn void setScreenWndSize(f_int32_t width, f_int32_t height)
 *  @brief 设置屏幕窗口尺寸(像素分辨率). 
 *  @param[in] width    窗口的宽度(单位：像素). 
 *  @param[in] height   窗口的高度(单位：像素). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setScreenWndSize(f_int32_t width, f_int32_t height)
{
	window_size.width = width;
	window_size.height = height;
}

/**
 * @fn void getScreenWndSize(f_int32_t *pwidth, f_int32_t *pheight)
 *  @brief 获取屏幕窗口尺寸(像素分辨率). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenWndSize(f_int32_t *pwidth, f_int32_t *pheight)
{
	if(NULL == pwidth || NULL == pheight)
	{
	    DEBUG_PRINT("NULL == pwidth || NULL == pheight");	
	    return;
	}
	
	*pwidth  = window_size.width;
	*pheight = window_size.height;
}

/**
 * @fn void map3dRenderFunc(sGLRENDERSCENE *pScene)
 *  @brief 三维地图场景渲染.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void map3dRenderFunc(sGLRENDERSCENE *pScene)	
{
	pScene->func3dMap((VOIDPtr *)pScene);		           
}

/**
 * @fn void map2dRenderFunc(sGLRENDERSCENE *pScene)
 *  @brief 二维地图场景渲染.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void map2dRenderFunc(sGLRENDERSCENE *pScene)	
{
	pScene->func2dMap((VOIDPtr *)pScene);		           
}

/**
 * @fn void userScreenLayerRenderFunc(sGLRENDERSCENE *pScene)
 *  @brief 用户屏幕图层渲染.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void userScreenLayerRenderFunc(sGLRENDERSCENE *pScene)	
{
	pScene->drawScreenUserLayer((VOIDPtr *)pScene);		           
}

/**
 * @fn void userGeoLayerRenderFunc(sGLRENDERSCENE *pScene)
 *  @brief 用户地理图层渲染.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void userGeoLayerRenderFunc(sGLRENDERSCENE *pScene)	
{
	pScene->drawGeoUserLayer((VOIDPtr *)pScene);		           
}

/**
 * @fn void screenPt2GeoPtFunc(sGLRENDERSCENE *pScene)
 *  @brief 屏幕坐标转换成地理坐标.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void screenPt2GeoPtFunc(sGLRENDERSCENE *pScene)	
{
	pScene->funcScreenPt2GeoPt((VOIDPtr *)pScene);		           
}

/**
 * @fn void geoPt2ScreenPtFunc(sGLRENDERSCENE *pScene)
 *  @brief 地理坐标转换成屏幕坐标.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void geoPt2ScreenPtFunc(sGLRENDERSCENE *pScene)	
{
	pScene->funcGeoPt2ScreenPt((VOIDPtr *)pScene);		           
}

/**
 * @fn void drawVPortBkground(f_float32_t r, f_float32_t g, f_float32_t b, f_float32_t alpha)
 *  @brief 用于背景色填充.
 *  @param[in] r,g,b,alpha.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void drawVPortBkground(f_float32_t r, f_float32_t g, f_float32_t b, f_float32_t alpha)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	static f_uint32_t vboid[2] = { 0 };
	if (vboid[0] == 0)
	{
		GLfloat vertexBuf[8] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
		GLushort indicesBuf[4] = { 0, 1, 2, 3 };

		glGenBuffers(2,&vboid);
		glBindBuffer(GL_ARRAY_BUFFER, vboid[0]);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* 4, vertexBuf, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboid[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)* 4, indicesBuf, GL_STATIC_DRAW);
	}

	{
		int prog = 0;
		GLuint vLoc = -1;
		GLuint cLoc = -1;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation(prog, "a_position");
		cLoc = glGetAttribLocation(prog, "a_color");

		glEnableEx(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glVertexAttrib4f(cLoc, r, g, b, alpha);

		glEnableVertexAttribArray(vLoc);
		glBindBuffer(GL_ARRAY_BUFFER, vboid[0]);
		glVertexAttribPointer(vLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboid[1]);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(vLoc);
		glDisableEx(GL_BLEND);
	}
#else
	static f_uint32_t tBkgroundList = 0;

    if((tBkgroundList) != 0)
    {
		glColor4f(r, g, b, alpha);
    	glCallList(tBkgroundList);
	}
	else
	{		
	    tBkgroundList = glGenLists(1);
	    if((tBkgroundList) != 0)
		{
			glNewList(tBkgroundList, GL_COMPILE);
			glBegin(GL_TRIANGLE_STRIP);
			//glColor4f(r, g, b, alpha);
            glVertex2f(-1.0f, -1.0f);
            glVertex2f(1.0f, -1.0f);
            glVertex2f(-1.0f, 1.0f);
            glVertex2f(1.0f, 1.0f);                
			glEnd();	
		    glEndList();

		}
		
	} 
#endif

}


/**
 * @fn void drawVPortBrightness(f_float32_t r, f_float32_t g, f_float32_t b, f_float32_t alpha)
 *  @brief 用于透明框绘制,控制地图亮度.
 *  @param[in] r,g,b,alpha.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void drawVPortBrightness(f_float32_t r, f_float32_t g, f_float32_t b, f_float32_t alpha)
{  
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	static f_uint32_t vboid[2] = { 0 };
	if (vboid[0] == 0)
	{
		GLfloat vertexBuf[8] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
		GLushort indicesBuf[4] = { 0, 1, 2, 3 };

		glGenBuffers(2,&vboid);
		glBindBuffer(GL_ARRAY_BUFFER, vboid[0]);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* 4, vertexBuf, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboid[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)* 4, indicesBuf, GL_STATIC_DRAW);
	}

	{
		int prog = 0;
		GLuint vLoc = -1;
		GLuint cLoc = -1;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation(prog, "a_position");
		cLoc = glGetAttribLocation(prog, "a_color");

		glEnableEx(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glVertexAttrib4f(cLoc, r, g, b, alpha);

		glEnableVertexAttribArray(vLoc);
		glBindBuffer(GL_ARRAY_BUFFER, vboid[0]);
		glVertexAttribPointer(vLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboid[1]);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(vLoc);
		glDisableEx(GL_BLEND);
	}
#else
	static f_uint32_t tBrightnessList = 0;

    if((tBrightnessList) != 0)
    {
		glEnableEx(GL_BLEND);	 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    
		glColor4f(r, g, b, alpha);

    	glCallList(tBrightnessList);

	    glDisableEx(GL_BLEND);

	}
	else
	{		
	    tBrightnessList = glGenLists(1);
	    if((tBrightnessList) != 0)
		{
			glNewList(tBrightnessList, GL_COMPILE);
			glBegin(GL_TRIANGLE_STRIP);
            glVertex2f(-1.0f, -1.0f);
            glVertex2f(1.0f, -1.0f);
            glVertex2f(-1.0f, 1.0f);
            glVertex2f(1.0f, 1.0f);                
			glEnd();	
		    glEndList();

		}
		
	} 
#endif
}

/**
 * @fn sPAINTUNIT *unitPtrValid(VOIDPtr punit)
 *  @brief 判断输入的绘制单元句柄是否有效.
 *  @param[in] punit 单元句柄.
 *  @exception void
 *  @return sPAINTUNIT *, 单元句柄.
 *  @retval 窗口句柄.
 *  @retval NULL 窗口句柄无效.
 * @see 
 * @note
*/
sPAINTUNIT *unitPtrValid(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	if(NULL == punit)
	{
	    DEBUG_PRINT("input handle punit = NULL!");
	}
	else
	    pUnit = (sPAINTUNIT *)punit;
	    
    return(pUnit);	    
}

/**
 * @fn sViewWindow *viewWindowPtrValid(VOIDPtr view_window)
 *  @brief 判断输入的渲染视窗句柄是否有效.
 *  @param[in] view_window 视窗句柄.
 *  @exception void
 *  @return sViewWindow，视窗句柄.
 *  @retval 视窗句柄.
 *  @retval NULL 视窗句柄无效.
 * @see 
 * @note
*/
sViewWindow *viewWindowPtrValid(VOIDPtr view_window)
{
	sViewWindow *pviewwind = NULL;
	if(NULL == view_window)
	{
	    DEBUG_PRINT("input handle view_window = NULL!");
	}
	else
	    pviewwind = (sViewWindow *)view_window;
	    
    return(pviewwind);	    
}

/**
 * @fn sGLRENDERSCENE *scenePtrValid(VOIDPtr render_scene)
 *  @brief 判断输入的场景句柄是否有效.
 *  @param[in] render_scene 场景句柄.
 *  @exception void
 *  @return sGLRENDERSCENE，场景句柄.
 *  @retval 场景句柄.
 *  @retval NULL 场景句柄无效.
 * @see 
 * @note
*/
sGLRENDERSCENE *scenePtrValid(VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
	if(NULL == render_scene)
	{
	    DEBUG_PRINT("input handle render_scene = NULL!");
	}
	else
	    pScene = (sGLRENDERSCENE *)render_scene;
	    
    return(pScene);	    
}

/**
 * @fn sMAPHANDLE *maphandlePtrValid(VOIDPtr map_handle)
 *  @brief 判断输入的数据四叉树句柄是否有效.
 *  @param[in] render_scene 数据四叉树句柄.
 *  @exception void
 *  @return sGLRENDERSCENE，数据四叉树句柄.
 *  @retval 数据四叉树句柄.
 *  @retval NULL 数据四叉树句柄无效.
 * @see 
 * @note
*/
sMAPHANDLE *maphandlePtrValid(VOIDPtr map_handle)
{
	sMAPHANDLE * pMapHandle = NULL;
		
	if(NULL == map_handle)
	{
	    DEBUG_PRINT("input handle map_handle = NULL!");
	}
	else
	    pMapHandle = (sMAPHANDLE *)map_handle;
	    
    return(pMapHandle);	    
}

/**
 * @fn f_int32_t setViewWindowParamPre(VOIDPtr view_window)
 *  @brief 设置视窗参数的前置函数，获取参数设置的信号量. 
 *  @param[in] view_window 视窗句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamPre(VOIDPtr view_window)
{
	sViewWindow *pviewwind = NULL;
    pviewwind = viewWindowPtrValid(view_window);
	if(NULL == pviewwind)
	    return(sFail);
	takeSem(pviewwind->viewwdparaminput_sem, FOREVER_WAIT);
	return(sSuccess);
}

/**
 * @fn f_int32_t setViewWindowParamPro(VOIDPtr view_window)
 *  @brief 设置视窗参数的后置函数，释放参数设置的信号量. 
 *  @param[in] view_window 视窗句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamPro(VOIDPtr view_window)
{
	sViewWindow *pviewwind = NULL;
    pviewwind = viewWindowPtrValid(view_window);
	if(NULL == pviewwind)
	    return(sFail);
	giveSem(pviewwind->viewwdparaminput_sem);
	return(sSuccess);
}

/**
 * @fn void setViewWindowAlpha(VOIDPtr view_window, f_float32_t alpha)
 *  @brief 设置视窗的透明度.
 *  @param[in] view_window 视窗句柄.
 *  @param[in] alpha 透明度.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWindowAlpha(VOIDPtr view_window, f_float32_t alpha)
{
	sViewWindow *pviewwind = NULL;
	pviewwind = viewWindowPtrValid(view_window);
	if(NULL == pviewwind)
	    return;
	
	pviewwind->viewwd_param_input.alpha = alpha;
}

/**
 * @fn void setViewWindowBkcolor(VOIDPtr view_window, sColor3f bkcolor)
 *  @brief 设置视窗的背景色.
 *  @param[in] view_window 视窗句柄.
 *  @param[in] bkcolor 背景色.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWindowBkcolor(VOIDPtr view_window, sColor3f bkcolor)
{
	sViewWindow *pviewwind = NULL;
	pviewwind = viewWindowPtrValid(view_window);
	if(NULL == pviewwind)
	    return;
	
	pviewwind->viewwd_param_input.bkcolor = bkcolor;
}

/**
 * @fn setViewWndParam(sViewWindow *pviewwind)
 *  @brief 设置渲染视窗的有关参数，包括尺寸、量程环半径、透明度、旋转中心. 
 *  @param[in] pviewwind 视窗句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证参数的一致性，需要用信号量来进行保护.
*/
static void updateViewPortParam(sViewWindow *pviewwind, sGLRENDERSCENE *pScene)
{
	if((NULL == pviewwind) || (NULL == pScene))
	    return;
	
	takeSem(pScene->eventparamset_sem, FOREVER_WAIT);
	pScene->outerviewport[0] = pviewwind->viewwd_param_input.outer_view_port.llx;
	pScene->outerviewport[1] = pviewwind->viewwd_param_input.outer_view_port.lly;
	pScene->outerviewport[2] = pviewwind->viewwd_param_input.outer_view_port.width;
	pScene->outerviewport[3] = pviewwind->viewwd_param_input.outer_view_port.height;
	
	pScene->innerviewport[0] = pviewwind->viewwd_param_input.inner_view_port.llx;
	pScene->innerviewport[1] = pviewwind->viewwd_param_input.inner_view_port.lly;
	pScene->innerviewport[2] = pviewwind->viewwd_param_input.inner_view_port.width;
	pScene->innerviewport[3] = pviewwind->viewwd_param_input.inner_view_port.height;
	giveSem(pScene->eventparamset_sem);
}

/**
 * @fn f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
 *  @brief 设置渲染视窗的参数.
 *  @param[in] view_window 渲染视窗的句柄.
 *  @param[in] scene 渲染场景的句柄.
 *  @param[in] llx 视窗的左下角横坐标的起始位置(单位:像素,外部屏幕坐标系,相对于旋转后的屏幕左上角).
 *  @param[in] lly 视窗的左下角纵坐标的起始位置(单位:像素,外部屏幕坐标系,相对于旋转后的屏幕左上角).
 *  @param[in] width 视窗宽度（像素,外部屏幕坐标系）.
 *  @param[in] height 视窗高度（像素,外部屏幕坐标系）.
 *  @exception void
 *  @return f_int32_t.
 *  @retval 0 成功.
 *  @retval -1 视窗句柄为空.
 *  @retval -2 起始位置不在范围内.
 *  @retval -3 宽度、高度不在范围内.
 *  @retval -4 外部屏幕坐标转换成内部屏幕坐标出错.
 * @see 
 * @note
*/
f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
{
	sViewWindow *pviewwind = NULL;
	sGLRENDERSCENE *pscene = NULL;
	/*外部屏幕坐标系(旋转后)下,屏幕的宽度和高度*/
	f_int32_t screen_width_outerscreen = 0, screen_height_outerscreen = 0;
	/*内部屏幕坐标系(正常摆放)下,视口的宽度和高度*/
	f_int32_t inner_vport_width_innerscreen = 0, inner_vport_height_innerscreen = 0;
	/*外部屏幕坐标系下，内部视口原点(左下角)在外部屏幕上所对应的点*/
	PT_2I inner_vport_lb_outerscreen;
	/*内部屏幕坐标系下，内部视口原点(该点是确定的,屏幕正常摆放时的左下角)的坐标*/
	PT_2I inner_vport_lb_innerscreen;

	pviewwind = viewWindowPtrValid(view_window);	
	if(NULL == pviewwind)
	{
		DEBUG_PRINT("the handle of view window is null.");
	    return(-1);
	}
	pscene = scenePtrValid(render_scene);
	if(NULL == pscene)
	{
		DEBUG_PRINT("the handle of render scene is null.");
	    return(-1);
	}

	switch(screen_param.rotate_type)
	{  
	    default:
	    case eROTATE_CW_0:			
			inner_vport_lb_outerscreen.x = llx;
	        inner_vport_lb_outerscreen.y = lly; 	        
			screen_width_outerscreen = window_size.width;
			screen_height_outerscreen = window_size.height;
			inner_vport_width_innerscreen = width;
			inner_vport_height_innerscreen = height;
	    break;
	    
	    case eROTATE_CW_90: 
            inner_vport_lb_outerscreen.x = llx;
	        inner_vport_lb_outerscreen.y = lly - height; 
			screen_width_outerscreen = window_size.height;
			screen_height_outerscreen = window_size.width;
			inner_vport_width_innerscreen = height;
			inner_vport_height_innerscreen = width;
	    break;
	    
	    case eROTATE_CW_180:
            inner_vport_lb_outerscreen.x = llx + width;
	        inner_vport_lb_outerscreen.y = lly - height;  
			screen_width_outerscreen = window_size.width;
			screen_height_outerscreen = window_size.height;
			inner_vport_width_innerscreen = width;
			inner_vport_height_innerscreen = height;
	    break;
	     
	    case eROTATE_CW_270:  
            inner_vport_lb_outerscreen.x = llx + width;
	        inner_vport_lb_outerscreen.y = lly;   
			screen_width_outerscreen = window_size.height;
			screen_height_outerscreen = window_size.width;
			inner_vport_width_innerscreen = height;
			inner_vport_height_innerscreen = width;
	    break;
    }



	/*设置外部视口的起始位置(左下角)坐标及宽度、高度，基于外部视口坐标系*/
	pviewwind->viewwd_param_input.outer_view_port.llx = llx;
	pviewwind->viewwd_param_input.outer_view_port.lly = screen_height_outerscreen - lly;
	pviewwind->viewwd_param_input.outer_view_port.width = width;
	pviewwind->viewwd_param_input.outer_view_port.height = height;

	/*计算内部视口原点在内部屏幕坐标系下坐标*/
	if(outerScreenPt2innerScreenPt(inner_vport_lb_outerscreen, &inner_vport_lb_innerscreen))
	{
		/*设置内部视口的起始位置(左下角)坐标及宽度、高度，基于内部视口坐标系*/
		pviewwind->viewwd_param_input.inner_view_port.llx = inner_vport_lb_innerscreen.x;
		pviewwind->viewwd_param_input.inner_view_port.lly = window_size.height - inner_vport_lb_innerscreen.y;
		pviewwind->viewwd_param_input.inner_view_port.width = inner_vport_width_innerscreen;
		pviewwind->viewwd_param_input.inner_view_port.height = inner_vport_height_innerscreen;
	}
	else
	{
	    DEBUG_PRINT("outerScreenPt2innerScreenPt(outer_vport, &inner_vport_lb) error!");
	    return(-4);
	}
	
	/*判断内部视口原点在内部屏幕坐标系下坐标是否超出内部屏幕范围*/
	if( inner_vport_lb_innerscreen.x < 0 || inner_vport_lb_innerscreen.x > window_size.width || 
		inner_vport_lb_innerscreen.y < 0 || inner_vport_lb_innerscreen.y > window_size.height)
	{
	    DEBUG_PRINT("lower-left corner of the view window is out of range!");
	    return(-2);
	}
	
	/*判断内部视口在内部屏幕坐标系下宽度和长度是否超出内部屏幕范围*/
	if( (inner_vport_width_innerscreen <= 0) || 
		(inner_vport_width_innerscreen > (window_size.width - inner_vport_lb_innerscreen.x) ) ||
		(inner_vport_height_innerscreen <= 0) || 
		(inner_vport_height_innerscreen > inner_vport_lb_innerscreen.y) )
	{
	    DEBUG_PRINT("width-height of the view window is out of range!");
	    return(-3);
	}

	/*更新外部视口的起始位置(左下角)坐标及宽度、高度，基于外部视口坐标系*/
	/*更新内部视口的起始位置(左下角)坐标及宽度、高度，基于内部视口坐标系*/
    updateViewPortParam(pviewwind, pscene);

	return(0);
}

/**
 * @fn setViewWndParam(sViewWindow *pviewwind)
 *  @brief 设置渲染视窗的有关参数，包括尺寸、量程环半径、透明度、旋转中心. 
 *  @param[in] pviewwind 视窗句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证参数的一致性，需要用信号量来进行保护.
*/
void setViewWndParam(sViewWindow *pviewwind)
{
	if(NULL == pviewwind)
	    return;
	
	takeSem(pviewwind->viewwdparamset_sem, FOREVER_WAIT);
	pviewwind->viewwd_param = pviewwind->viewwd_param_input;
	giveSem(pviewwind->viewwdparamset_sem);
}

/**
 * @fn vvoid updateInputParam(sViewWindow *pviewwind, sGLRENDERSCENE *pScene)
 *  @brief 更新外部输入的参数.
 *  @param[in] pviewwind  视窗句柄.
 *  @param[in] pScene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void updateInputParam(sViewWindow *pviewwind, sGLRENDERSCENE *pScene)
{
	double terrainHeight = 0.0;
	static BOOL isHeightAdjusted = FALSE;

	if((NULL == pviewwind) || (NULL == pScene))
	    return;	 
	
	/* 更新视窗参数 */
	setViewWndParam(pviewwind);

	/* 更新外部视口和内部视口参数 */
	//updateViewPortParam(pviewwind, pScene); 

	/* 更新事件参数,包括相机控制参数、模式控制参数、辅助导航信息参数 */ 
	setEventParam(pScene);  

	/* 更新周期参数,即飞行参数,漫游状态下不在此处更新飞机位置信息 */ 
	setPeriodParam(pScene); 

	/* 避免视点进入地下,当飞机高度离地高度小于50m时，强制设置飞机高度为地面高度+50m */
	/* 俯视下无需进行穿地判断,其他视角是否需要穿地判断根据型号要求定,没有特别要求就保留 */
	if(pScene->camctrl_param.view_type != eVM_OVERLOOK_VIEW)
	{
		/* 获取当前经纬度在瓦片中的高度,瓦片抽析过,其实高度会有较大误差 */
		GetZ(pScene, pScene->geopt_pos.lon, pScene->geopt_pos.lat, &terrainHeight);

		if(pScene->geopt_pos.height < terrainHeight +50)
		{
			pScene->geopt_pos.height = (float)(terrainHeight +50);
			if(FALSE == isHeightAdjusted)
			{
			//	printf("[3D]Plane height is too low, the hight is now adusted.\n");
			}
			isHeightAdjusted = TRUE;
		}
		else
		{
			if(TRUE == isHeightAdjusted)
			{
			//	printf("[3D]Plane height is OK now.\n");
			}
			isHeightAdjusted = FALSE;
		}

	}

	/* 2016/06/09 张仟新
	不知为什么，在特殊的高度如8000,5000,2500,2000,1500时，如果不改变高度，画面会闪，
	改变高度后，画面就不闪了，且改变的值不能小于0.4
	该现象未复现,暂时去掉,避免高度显示不正确
	*/
	//if(fabs(fmod(pScene->geopt_pos.height, 10.0)) < PRECISION )  
	//    pScene->geopt_pos.height += 1.4;
	    
	/* 地理坐标转换成物体坐标 */
	{
	    if(!geoDPt2objDPt( &(pScene->geopt_pos), &(pScene->objpt_pos)) )
		{
	        pScene->objpt_pos.x = 0.0;
	        pScene->objpt_pos.y = 0.0;
	        pScene->objpt_pos.z = 0.0;
		}	
	}

	//更新二维引擎的飞行参数和控制参数
	setVecMapParam(pScene);

}

/**
 * @fn void drawFuncPre(void)
 *  @brief 用于保存模视矩阵和投影矩阵,并为后续图形绘制设定投影方式和单位阵.
 *  @param[in] void.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void drawFuncPre(void)
{  
	glMatrixMode(GL_PROJECTION);						
	glPushMatrix();
	glLoadIdentity();	
	glOrtho (-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);	
	glMatrixMode(GL_MODELVIEW);							
	glPushMatrix();
	glLoadIdentity();
}

/**
 * @fn void drawFuncPro(void)
 *  @brief 恢复之前的模视矩阵和投影矩阵.
 *  @param[in] void.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void drawFuncPro(void)
{  
    glMatrixMode(GL_MODELVIEW);							
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);						
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);	
}

/**
 * @fn static void getRotateOpenglPt(sGLRENDERSCENE *pScene)
 *  @brief 获取旋转中心的OpenGL坐标.
 *  @param[in] pScene  场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
static void getRotateOpenglPt(sGLRENDERSCENE *pScene)
{
	PT_2D viewPt, openGlPt;
		
	if(NULL == pScene)
        return;

	viewPt.x = pScene->camctrl_param.rotate_center.x;
	viewPt.y = pScene->camctrl_param.rotate_center.y;
	if(innerViewDPt2openGlDPt(&viewPt, pScene->innerviewport, &openGlPt))
	{
		pScene->rotate_opengl_pt.x = openGlPt.x;
		pScene->rotate_opengl_pt.y = openGlPt.y;
	}
	else
	{
		pScene->rotate_opengl_pt.x = 0.0;
		pScene->rotate_opengl_pt.y = 0.0;
	}
}

static void renderStencil(f_float32_t startx, f_float32_t starty, f_float32_t width, f_float32_t height)
{
    f_float64_t left, right, bottom, top;
    f_float32_t x, y, radiu;
	
	/* 计算圆心的视口坐标 */
	x = startx + width * 0.5f;    
	y = starty + height * 0.5f;
	/* 计算半径(单位是像素) */
	radiu = height * 0.5f - 10.0f;
	
	left   = startx;
	right  = startx + width;
	bottom = starty;
	top    = starty + height;
	
	/* 使用模板缓冲区的方法绘制一个圆 */
	/* 初始化initGLSetting函数中已启用 */
//	glClearStencil(0);
//	glEnableEx(GL_STENCIL_TEST);	
//	glClear(GL_STENCIL_BUFFER_BIT);

	/* 将模板缓冲区中的值先与参数3进行与操作,结果与参数2进行比较,比较函数为参数1. GL_NEVER表示永远不通过测试 */
	glStencilFunc(GL_NEVER, 0x1, 0x1);
	/* 根据比较结果来决定如何修改模板缓冲区中的值,参数1表示模板测试不通过时执行的操作, */
	/* 参数2表示模板测试通过但深度测试失败时执行的操作, 参数3表示模板测试通过且深度测试也通过时执行的操作 */
	/* GL_REPLACE表示用glStencilFunc的参数2替换模板缓冲区中的值 */
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);	
	
	/* 在视口范围内绘制一个圆，圆内的模板缓冲区中值为1，圆外的模板缓冲区中值为0 */
	glViewport(startx, starty, width, height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glOrtho(left,right,bottom, top, 1, -1);

	/* 用三角形扇组成一个圆 */
	glBegin(GL_TRIANGLE_FAN);
		glVertex2d(x, y);
		glVertex2d(x-radiu, y);
		{
			f_int32_t i = 1;
			for(i = 1; i <= 360; i++)
			{				
				glVertex2d(x-radiu*cos(i*DE2RA), y+radiu*sin(i*DE2RA));
			}
		}
	glEnd();
	
	glColor3f(0.0f, 0.0f, 0.0f);
	/* GL_EQUAL表示模板缓冲区中的值与参数2相等，才通过测试 */
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	/* GL_KEEP表示保持当前模板缓冲区中的值 */
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	
	
	/* 禁止写入深度缓冲区 */
	glDepthMask(GL_FALSE);
	/* 绘制圆的外接矩形 */
	glRectf(x-radiu, y+radiu, x+radiu, y-radiu);
	/* 允许写入深度缓冲区 */
	glDepthMask(GL_TRUE);

	glColor3f(1.0f, 1.0f, 1.0f);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();	
}

static void renderCps(sGLRENDERSCENE *pScene)
{
	f_float32_t yaw = 0.0f;
	if(NULL == pScene)
	    return;	

    if(0 == pScene->auxnav_list.cps_list.displaylist_id)
    {
    	f_float32_t lbx = 0.0f, lby = 0.0f, rtx = 0.0f, rty = 0.0f;
    	lbx = -0.5f * pScene->auxnavinfo_param.cps_param.width;
    	lby = -0.5f * pScene->auxnavinfo_param.cps_param.height;
    	rtx = -lbx;
    	rty = -lby;
        pScene->auxnav_list.cps_list.displaylist_id = cpsListCreate(lbx, lby, rtx, rty);
    }
    
    /* 北朝上时罗盘固定不转，其它模式时罗盘随航向角转动 */
    /* 注意罗盘的旋转是调用glRotatef饶z轴旋转航向角来的,直接用航向角即可,与pScene->attitude.yaw相反*/
	if(eMV_headup == pScene->mdctrl_cmd.mv_mode)
        yaw = 360.0 - pScene->attitude.yaw; 
    renderCompassRose(yaw, 
                      pScene->innerviewport[0], pScene->innerviewport[1], 
                      pScene->auxnavinfo_param.cps_param.cent_x,
                      pScene->auxnavinfo_param.cps_param.cent_y,
                      pScene->auxnavinfo_param.cps_param.width,
                      pScene->auxnavinfo_param.cps_param.height,
                      pScene->auxnavinfo_param.cps_param.color, 
                      pScene->auxnav_list.cps_list.displaylist_id, 
                      pScene->auxnav_list.cps_list.texture_id);                    
}

extern int ConvertScreenXyToGeo(int viewOutPutIndex, int viewPortIndex, float screenX, float screenY, float *pLon, float *pLat);
void updataRoamParam(sGLRENDERSCENE *pScene)
{
	BOOL ret = FALSE;
	Geo_Pt_D geoPt;
	float lon_vec = 0.0f;
	float lat_vec = 0.0f;
	//PT_2I viewPt;

	/* 如果处于俯视下的漫游模式且还未漫游，应根据漫游位置计算地理坐标，并赋给飞机位置，漫游时就不用接收到的飞机经纬度了 */
	if((eROM_inroam == pScene->mdctrl_cmd.rom2d_mode) 
		&&(pScene->is_romed == eROM_notroamed))
	{
		ret = getGeoByOuterViewPt(pScene, pScene->rom_pt_input, &geoPt);
		if(ret)		
		{
			/* 注意:下一帧才会更新 */
			pScene->geopt_pos.lon  = geoPt.lon;
			pScene->geopt_pos.lat  = geoPt.lat;
			/*为了保证漫游下高度预警功能正常,此处不更新高度*/
			//pScene->geopt_pos.height  = geoPt.height;
			//getViewPtByGeo(pScene,geoPt, &viewPt);
		}
		else
		{
			;//不改变位置	
		}

		/* 设置状态为已漫游 */
		pScene->is_romed = eROM_roamed;
	}	

	/* 如果处于俯视下的漫游模式且还未漫游归位，应根据将之前记录的漫游进入时的初始位置赋给飞机位置，直接返回，不再处理当前帧之后的漫游操作 */
	if((eROM_inroam == pScene->mdctrl_cmd.rom2d_mode) 
		&&(pScene->is_rom_homed == eROM_notroamhomed))
	{
		/* 注意:下一帧才会更新 */
		pScene->geopt_pos  = pScene->roam_start_pt_pos;
		/* 设置状态为已漫游归位 */
		pScene->is_rom_homed = eROM_roamhomed;
	}
}


/**
 * @fn void mapSceneRender(VOIDPtr render_wnd, VOIDPtr render_scene)
 *  @brief 地图场景渲染.
 *  @param[in] render_viewwnd  视窗句柄.
 *  @param[in] render_scene 场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void mapSceneRender(VOIDPtr render_viewwnd, VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
	sViewWindow *pViewWnd = NULL;

	/* 判断输入的渲染场景句柄是否有效,若无效则直接返回 */
	pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;	    
	
	/* 判断输入的绘制视窗句柄是否有效,若无效则直接返回 */
	pViewWnd = viewWindowPtrValid(render_viewwnd);
	if(NULL == pViewWnd)
	    return;	 
	
	/* 解冻状态下才更新引擎内部的五类参数 */
	if(eFRZ_outfreeze == pScene->mdctrl_cmd_input.frz_mode)    
	    updateInputParam(pViewWnd, pScene);    
		    
	glViewport(pViewWnd->viewwd_param.inner_view_port.llx, pViewWnd->viewwd_param.inner_view_port.lly,
	           pViewWnd->viewwd_param.inner_view_port.width, pViewWnd->viewwd_param.inner_view_port.height);

	/* 设置旋转中心的OpenGL坐标,只有在俯视模式下,设置旋转中心才有效；其他模式默认为(0,0) */ 
	if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
	{           
	    getRotateOpenglPt(pScene); 
    }
    else
    {
    	pScene->rotate_opengl_pt.x = 0.0;
	    pScene->rotate_opengl_pt.y = 0.0;
    }	                                	        
	
	/* 设置绿色模式,即仅启动颜色缓冲区中的G通道和alpha通道,效果就是只能看到绿色和透明度 */
	if(eCOR_green == pScene->mdctrl_cmd.cor_mode) 
	    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);

#if 1
	{
		/* 开启裁剪测试,用于限制绘制区域 */
		glEnableEx(GL_SCISSOR_TEST);
		/* 设置裁剪区域,以左下角为坐标原点 */
		glScissor(pViewWnd->viewwd_param.inner_view_port.llx, pViewWnd->viewwd_param.inner_view_port.lly,
	           pViewWnd->viewwd_param.inner_view_port.width, pViewWnd->viewwd_param.inner_view_port.height);
		/* 清除裁剪区域深度缓冲区,深度值默认为1.0f */
 		glClear(GL_DEPTH_BUFFER_BIT);
		/* 关闭裁剪测试 */
		glDisableEx(GL_SCISSOR_TEST);	
	}
#endif
	
	/* 先绘制一个背景底色,用显示列表的方式,绘制一个视口大小的矩形 */
	drawFuncPre();
	drawVPortBkground(pViewWnd->viewwd_param.bkcolor.red, pViewWnd->viewwd_param.bkcolor.green, pViewWnd->viewwd_param.bkcolor.blue, 1.0f); 
	drawFuncPro(); 	   

	// 漫游状态
	if(eROM_inroam == pScene->mdctrl_cmd.rom_mode)
	{
		double roamPosHeight;
		GetPtEye(&pScene->roam, &pScene->objpt_pos);
		pScene->camParam.m_ptEye = pScene->objpt_pos;
		objDPt2geoDPt(&(pScene->camParam.m_ptEye), &(pScene->camParam.m_geoptEye));

		/*判断是否穿地，如果穿地则不再缩放*/
		/* 获取当前经纬度在瓦片中的高度,瓦片抽析过,其实高度会有较大误差 */
		GetZ(pScene, pScene->camParam.m_geoptEye.lon, pScene->camParam.m_geoptEye.lat, &roamPosHeight);
		if (roamPosHeight > pScene->camParam.m_geoptEye.height-50.0)
		{
			roamScale = 1;
		}else{
			roamScale = 0;
		}

		pScene->geopt_pos = pScene->camParam.m_geoptEye;
		calcWorldMatrix44(pScene->objpt_pos.x, pScene->objpt_pos.y, pScene->objpt_pos.z, &(pScene->m_mxWorld) );

		{
			double measure = 0.0;	
			Hgt2Measure(pScene, &measure);
			if(measure < 1.0)
			{
				measure = 6371000;
			}
			{
				double yaw = 0.0;//GetYawByMatrix(&pScene->roam.mv);
				yaw = 360.0 - pScene->roam.thisYaw;
			}
			
			pScene->roam_res = measure / pScene->camctrl_param.compass_radius;

			//对漫游视角下的qtn 使用的分辨率进行最小值限制，
			//目的是保证在座舱下漫游时地名注记不会由于量程过小而减少。
			if(measure < 100000.0)
				measure = 100000.0;

			pScene->roam_res_for_QTN = measure / pScene->camctrl_param.compass_radius;			
		}
	}
	else
	{
		//俯视下接收量程
	    if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
	    {
			pScene->roam_res_for_QTN = pScene->roam_res = pScene->camctrl_param.view_res;
		}
		else
		{
			//其他视角下采用100 km
			pScene->roam_res_for_QTN = pScene->roam_res = 100000.0 / pScene->camctrl_param.compass_radius;
		}

		/* 球面投影下,根据不同视角计算相机参数 */
		if(ePRJ_ball == pScene->project_mode)
		{	
			/*计算视点的局部坐标到世界坐标矩阵*/
			// 俯视视角下，非漫游状态，默认的视点为飞机所在经纬度下，海拔高度为0那一点；
			//             漫游状态，视点为屏幕上传进来那一点的对应经纬度下，海拔高度为0那一点。
			// 响应用户的“量程”输入值，刻度环半径对应的距离 与 刻度环半径的像素数 之比，可以算出每像素对应的实际距离，
			// 再根据视场角，计算出视点的海拔高度
			// 俯视视角下，不绘制飞机	
			if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
			{
				f_float64_t hgt, angle;
				Geo_Pt_D geopt;
				Obj_Pt_D objpt;
				geopt.lon = pScene->geopt_pos.lon;
				geopt.lat = pScene->geopt_pos.lat;
				geopt.height = 0.0;
				geoDPt2objDPt(&geopt, &objpt);	// 飞机经纬度转至球坐标
				// 1.计算视点在地面上的世界矩阵(通过该矩阵可将相机坐标系下的局部坐标变换至球坐标)
				calcWorldMatrix44(objpt.x, objpt.y, objpt.z, &(pScene->m_mxWorld));
				// 2.从量程档换算出视点海拔高度
				angle = pScene->camctrl_param.view_res * pScene->camctrl_param.compass_radius / EARTH_RADIUS;
				hgt = (/*pScene->viewport[3]*/window_size.height * 0.5 * EARTH_RADIUS * sin(angle)) / 
					  (pScene->camctrl_param.compass_radius * tan((pScene->camctrl_param.view_angle * 0.5) * DE2RA)) 
					  + EARTH_RADIUS * cos(angle) - EARTH_RADIUS;
				// 3.设置视点局部坐标(相对于视点中心点)(抬升到量程档对应高空位置)
				vector3DMemSet(&(pScene->camParam.m_ptEye), 0.0, 0.0, hgt);
			}
			else
			{
				// 非俯视视角下，非漫游状态，默认的视点为飞机所在经纬高；计算视点的局部坐标到世界坐标矩阵；
				calcWorldMatrix44(pScene->objpt_pos.x, pScene->objpt_pos.y, pScene->objpt_pos.z, &(pScene->m_mxWorld) );
			}

			//lpf add temp 2018年8月28日15:26:15 for 45度\75度
			if(pScene->camctrl_param.view_type == eVM_SCALE_VIEW)
			{
				calTruepos(pScene, (LP_PT_3D)(&(pScene->camParam.m_ptEye)), (LP_Geo_Pt_D)(&(pScene->camParam.m_geoptEye)));
				calcWorldMatrix44(pScene->camParam.m_ptEye.x, 
					pScene->camParam.m_ptEye.y, 
					pScene->camParam.m_ptEye.z, &(pScene->m_mxWorld) );
			}
			
			/*根据视角类型,飞机的姿态,视点的局部坐标到世界坐标矩阵,计算相机参数(4个参数)及姿态旋转矩阵(m_mxRotate)*/
			setCameraStyle((f_uint8_t)(pScene->camctrl_param.view_type),
						   (LP_PT_3D)(&(pScene->camParam.m_ptEye)),		// 视点相对地面点位置
						   (LP_PT_3D)(&(pScene->camParam.m_ptCenter)),	// 中心点(地面点)位置 
						   (LP_PT_3D)(&(pScene->camParam.m_ptUp)),		// Y轴
						   (LP_PT_3D)(&(pScene->camParam.m_ptTo)), 
						   pScene->attitude.pitch, pScene->attitude.yaw, pScene->attitude.roll,
						   (LPMatrix44)(&(pScene->m_mxRotate)), (LPMatrix44)(&(pScene->m_mxWorld)) );	    

			/*将视点的物体坐标转换成经纬度坐标*/
			if(!objDPt2geoDPt(&(pScene->camParam.m_ptEye), &(pScene->camParam.m_geoptEye)))
			{
				pScene->camParam.m_geoptEye.lon = 0.0;
				pScene->camParam.m_geoptEye.lat = 0.0;
				pScene->camParam.m_geoptEye.height = 0.0;
			}

		}
	}


	if(ePRJ_ball == pScene->project_mode)
	{
		/* 球面投影下,计算相机的远近裁截面,处理画面旋转,设置并存储模式矩阵和投影矩阵 */
		updateCameraParamDIY(pScene);	
	}
	else
	{
		/* 墨卡托投影下,只有俯视视角,计算相机参数,设置并存储模式矩阵和投影矩阵,函数实现在vmapApp.c中 */
	 	CalAndSetVecMatrix(pScene->scene_index, &(pScene->m_lfModelMatrix[0]), &(pScene->m_lfProjMatrix[0]),
	 		&pScene->camParam.m_lfNear, &pScene->camParam.m_lfFar);
	}
#if 0
	{
		f_float32_t eyePos[3];
		eyePos[0] = pScene->camParam.m_ptEye.x;
		eyePos[1] = pScene->camParam.m_ptEye.y;
		eyePos[2] = pScene->camParam.m_ptEye.z;
		glFogfv(GL_FOG, GL_FOG_EYE_POS, eyePos);
	}
#endif

	if(pScene->mdctrl_cmd.svs_mode == 1)
	{	/* 每一帧都要打开光照，且开启光照要放在设置投影矩阵之后，保证嵌入式下光照正常，张钰鹏2017-4-28 */
		EnableLight(FALSE);
	}

	EnableLight0(TRUE);

	/* 更新远近裁截面 */
	pScene->camctrl_param.view_near = pScene->camParam.m_lfNear;
	pScene->camctrl_param.view_far = pScene->camParam.m_lfFar;

	/* 计算模式矩阵和投影矩阵的乘积及其逆矩阵 */
	calCurMatrix(pScene);
	
	/* 计算视景体参数 */
	CalculateFrustum(&(pScene->frustum), pScene);

	/* 三维场景渲染 */         		
	if((NULL != pScene->func3dMap) )
	{   
		if(ePRJ_ball == pScene->project_mode)
		{
			//drawFuncPre();	           
			map3dRenderFunc(pScene);
			//drawFuncPro();
		}
	}

	/* 零俯仰角线纹理大小为8192*64，超过7200支持的最大纹理尺寸，故7200下不绘制零俯仰角线 */
#ifndef _JM7200_
    /* 零俯仰角线显示开关打开才绘制（俯视模式不绘制）,零俯仰角线与场景的模式矩阵相关，必需在场景的当前环境中绘制*/
    if(pScene->auxnavinfo_param.zpl_param.is_display) 
    {
		/* 如果零俯仰角线纹理未生成，重新生成零俯仰角线纹理 */
    	if(0 == pScene->auxnav_list.zpl_list.displaylist_id)
    	    pScene->auxnav_list.zpl_list.displaylist_id = zplListCreate();

        if((eVM_OVERLOOK_VIEW != pScene->camctrl_param.view_type))
        {
            renderZeroPitchLine(pScene->m_mxWorld, pScene->auxnavinfo_param.zpl_param.color, 
                                pScene->auxnav_list.zpl_list.displaylist_id, pScene->auxnav_list.zpl_list.texture_id);		
        }
    }
#endif 


#if 0
    if((eROM_inroam != pScene->mdctrl_cmd.rom_mode) && (eVM_COCKPIT_VIEW != pScene->camctrl_param.view_type))
    {
        glLineWidth(1.0f);
        glBegin(GL_LINES);
#if 0	
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3d(pScene->camParam.m_ptCenter.x - 2000.0, pScene->camParam.m_ptCenter.y, pScene->camParam.m_ptCenter.z);
        glVertex3d(pScene->camParam.m_ptCenter.x + 2000.0, pScene->camParam.m_ptCenter.y, pScene->camParam.m_ptCenter.z);
        glVertex3d(pScene->camParam.m_ptCenter.x, pScene->camParam.m_ptCenter.y - 2000.0, pScene->camParam.m_ptCenter.z);
        glVertex3d(pScene->camParam.m_ptCenter.x, pScene->camParam.m_ptCenter.y + 2000.0, pScene->camParam.m_ptCenter.z);
        glVertex3d(pScene->camParam.m_ptCenter.x, pScene->camParam.m_ptCenter.y, pScene->camParam.m_ptCenter.z - 2000.0);
        glVertex3d(pScene->camParam.m_ptCenter.x, pScene->camParam.m_ptCenter.y, pScene->camParam.m_ptCenter.z + 2000.0);
#endif      
        glColor3f(1.0f, 0.0f, 0.0f);
	    glVertex3d(pScene->objpt_pos.x - 100.0, pScene->objpt_pos.y, pScene->objpt_pos.z);
        glVertex3d(pScene->objpt_pos.x + 100.0, pScene->objpt_pos.y, pScene->objpt_pos.z);
		glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3d(pScene->objpt_pos.x, pScene->objpt_pos.y - 100.0, pScene->objpt_pos.z);
        glVertex3d(pScene->objpt_pos.x, pScene->objpt_pos.y + 100.0, pScene->objpt_pos.z);
		glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3d(pScene->objpt_pos.x, pScene->objpt_pos.y, pScene->objpt_pos.z - 100.0);
        glVertex3d(pScene->objpt_pos.x, pScene->objpt_pos.y, pScene->objpt_pos.z + 100.0);

        glEnd();
    }
	    
    if((eVM_COCKPIT_VIEW != pScene->camctrl_param.view_type)&&(eROM_inroam != pScene->mdctrl_cmd_input.rom_mode))
    {
	    /* 飞机模型及飞机雷达模型的三个角度设置需重新考虑，感觉目前的设置是不对的。待后续改进2017-3-8 , 张仟新*/
        static f_int32_t plane_id = 0;
        f_float32_t color[4] = {0.0f, 1.0f, 0.0f, 0.8f};
	    f_float64_t pitch = 0.0, roll = 0.0, yaw = 0.0;
        if((eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type))
		    pitch = 0.0;
	    else
		    pitch = pScene->attitude.pitch;
	    roll = pScene->attitude.roll;
	    yaw  = pScene->attitude.yaw;
        drawCylinder(&plane_id, pScene->objpt_pos, pitch, roll, yaw, 0, 20.0, 400.0, 32, 32, color);  
    }		
#endif

	/* 二维场景渲染,俯视视角且是二维模式 */
	if((NULL != pScene->func2dMap) 
		&& (pScene->mdctrl_cmd.map2dor3d == 0) 
 		&& ((eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type) 
	 		|| (eVM_SCALE_VIEW == pScene->camctrl_param.view_type))
		
		)
	{   	
		//drawFuncPre();       
		if (pScene->mdctrl_cmd.rom_mode == 1)
		{
			if (pScene->roam.thisPitch >= -45.0)
			{
				//关闭字体显示
				setFontVisable(0);
			}
		}
		else
		{
			if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
			{
				//俯视下正常绘制矢量线、面
				setVecLineAreaShowValue(1,pScene->scene_index);
				//俯视下正常绘制点
				setViewXYZType(pScene->geopt_pos.lon, pScene->geopt_pos.lat, 
					pScene->geopt_pos.height, FALSE);
			}
			else if (eVM_SCALE_VIEW == pScene->camctrl_param.view_type)
			{
				//45°/75°视角下不绘制矢量线、面
				setVecLineAreaShowValue(0,pScene->scene_index);
				//45°/75°视角下控制点的绘制，与视点位置距离大于1500000才进行绘制
				setViewXYZType(pScene->geopt_pos.lon, pScene->geopt_pos.lat, 
					pScene->geopt_pos.height, TRUE);
			}
		}

	    map2dRenderFunc(pScene);
	    //drawFuncPro();

		//控制ttfDrawString正常绘制文字信息
		setFontVisable(1);
	}

	
//	glDisableEx(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,0,0x01);//代替glDisableEx；
	
#if 1	
	drawFuncPre();   
	//俯视且需显示时才绘制罗盘
	if((pScene->auxnavinfo_param.cps_param.is_display) && 
	   (eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type) )
	{
	    renderCps(pScene);
	}
	drawFuncPro();
#endif   
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

#ifdef USE_MODEL
	//绘制模型
	if(1 == pScene->mdctrl_cmd.is_3dplane_display)
	{
		RenderModel(FALSE, pScene);
	}
#endif

	//绘制高程剖面图
	if(TRUE == pScene->auxnavinfo_param.prefile_param.is_display)
	{	
		RenderTerrainProfile(pScene);
	}

	if(TRUE == pScene->termap_param_input.isShow) 
	{
		RenderTerrainMap(pScene);
	}

	//俯视且需显示时才绘制飞机
	if((pScene->mdctrl_cmd.plane_symbol_display == 1) && 
		(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type) )
	{
		drawPlaneSymbol(pScene);
	}
	
	/* JM7200下与背景色使用同一个显示列表会有问题，此处改用两个不同的显示列表来实现背景框和透明框的绘制 */
	/* 最后在画面最上层绘制一个与视口等同大小的黑色的矩形,通过透明度来控制视口画面的亮度,alpha越小亮度越高 */
    if( (eScene_normal == pScene->mdctrl_cmd.scene_mode) && (pViewWnd->viewwd_param.alpha > 0.0f) )	
    {
    	drawFuncPre();
	    glViewport(pViewWnd->viewwd_param.inner_view_port.llx, pViewWnd->viewwd_param.inner_view_port.lly,
	               pViewWnd->viewwd_param.inner_view_port.width, pViewWnd->viewwd_param.inner_view_port.height); 
#if 0	           
	    /* 视口边框 */
	    glColor3f(0.0f, 0.0f, 0.0f);
	    glLineWidth(2.0f);
	    glBegin(GL_LINE_LOOP);
	        glVertex2f(-1.0f, -1.0f);
	        glVertex2f(1.0f, -1.0f);
	        glVertex2f(1.0f, 1.0f);
	        glVertex2f(-1.0f, 1.0f);
	    glEnd();
#endif 	    
	    /* 设置透明度 */
	    drawVPortBrightness(0.0f, 0.0f, 0.0f, pViewWnd->viewwd_param.alpha); 	
	    drawFuncPro();
    } 
	
	/* 调用用户注册的用户图层绘制函数 */   
	if(NULL!= pScene->funcScreenPt2GeoPt)
	{
	    screenPt2GeoPtFunc(pScene);	
	}
	if(NULL!= pScene->funcGeoPt2ScreenPt)
	{
		geoPt2ScreenPtFunc(pScene);
	}
	if(NULL != pScene->drawGeoUserLayer) 
    {   	           
	    userGeoLayerRenderFunc(pScene);
	}
	if(NULL != pScene->drawScreenUserLayer) 
    {   	           
	    userScreenLayerRenderFunc(pScene);
	}

	/*非冻结时才能进行漫游*/
	if(eFRZ_outfreeze == pScene->mdctrl_cmd_input.frz_mode) 
	{
		/* 更新俯视视角下的漫游参数,实现俯视视角下地图的漫游功能 */
		updataRoamParam(pScene);
	}


	/* 更新非俯视视角下的漫游参数,实现非俯视视角下地图的漫游功能 */
	StartRoam(&pScene->roam, 
		pScene->mdctrl_cmd.rom_mode, 
		pScene->m_lfModelMatrix, 
		pScene->innerviewport,
		pScene->attitude_input.yaw,
		pScene);

	
	///* 清空绘制链表中的所有瓦片节点 */
	//drawListDeleteNode(pScene);		
}

// 计算视椎体远近裁截面
static void calcCamFarNearPlane(sGLRENDERSCENE *pScene)
{
	PT_3D ptTemp;
	f_float64_t lfNear,lfFar, hei;	// 远近裁截面|飞机高
	f_float64_t angle_theta; //地心-视点 与 地表切点-视点 两条连线的夹角
	
	if(NULL == pScene)       
        return;
    
	// 计算视点到目标点距离
    vector3DSub(&ptTemp, &pScene->camParam.m_ptEye, &pScene->camParam.m_ptCenter);
	pScene->camParam.m_lfDistance = vector3DLength(&ptTemp);
	hei = pScene->camParam.m_geoptEye.height;	// 飞机海拔高
	lfNear = 10;    
	if(hei < 0) //直接用else里的算式，在hei<0时可能对负数开方导致溢出，出现-1.#IND0000000的现象
		lfFar = 100000.0;
	else
	{
		angle_theta = asin(EARTH_RADIUS/(EARTH_RADIUS+hei));
		lfFar = (hei + EARTH_RADIUS)* cos(angle_theta);
	}		

    if(eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)/* 在漫游状态下,俯视时,不能采用下面的方法计算远裁截面,应按透视时的远裁截面,否则会露白 */
    {
		// 计算远裁截面
	    if (eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
	    {
			/*
			// 为了在俯视状态下更精确的做视景体判断，尽量避免多余瓦片进入，将俯视下的远裁截面拉近，放在视景体的圆锥体模型刚好与地球交的距离处。
			// 这里主要是为了根据 垂直视场角g_Fovy 和 屏幕尺寸(左右上下) 计算出视景体若建模为一个锥体时，锥体剖面的半顶角theta
			// 若l为从视点出来到屏幕的距离，则有
			// ((Top - Bottom)/2) / l = tan(Fovy/2)
			// sqrt( ((Top - Bottom)/2)^2 + ((Right - Left)/2)^2 ) / l = tan(theta)
			// 化简合并得 => sqrt( (Top - Bottom)^2 + (Right - Left)^2 ) * tan(Fovy/2) / (Top - Bottom) = tan(theta)

			// 又对于地球来说，设视点高度为h, 地球半径为R，地球球心对应的半角度为alpha，由正弦定理
			// (h + R) / sin[pi - (theta + alpha)] = R / sin(theta)
			// 最终远裁截面距离为
			// lfFar = h + R - R*cos(alpha)
			*/
			f_float64_t theta, alpha;
			theta = atan( sqrt( window_size.width * window_size.width + window_size.height * window_size.height ) 
						  * tan(pScene->camctrl_param.view_angle * 0.5 * DE2RA) / window_size.height );
			alpha = atan( (hei / EARTH_RADIUS + 1.0 - sin(theta)) / cos(theta) );
			lfFar = hei + EARTH_RADIUS - EARTH_RADIUS * cos(alpha);	
	    }

		//近裁截面计算
		if( eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type )	// 俯视视角
		{
			pScene->camParam.m_lfNear = hei - 9000 * 2;
			if(pScene->camParam.m_lfNear < 10	)
			{
				pScene->camParam.m_lfNear = 10;
			}
		}
		else if (eVM_FIXED_VIEW ==  pScene->camctrl_param.view_type)
		{
			pScene->camParam.m_lfNear = 10 * 100;
		}
		else if(eVM_FOLLOW_VIEW == pScene->camctrl_param.view_type)
		{	
			pScene->camParam.m_lfNear = 10 * 10;
		}
		else 
		{
			pScene->camParam.m_lfNear = 10;
		}
	}
	else
	{
		//漫游情况下
		if (hei < 9000)
		{
			pScene->camParam.m_lfNear = 10;
		}
		else
		{
			pScene->camParam.m_lfNear = lfFar *0.01;
			if(pScene->camParam.m_lfNear < 5000)
			{
				pScene->camParam.m_lfNear = 10;
			}
		}
	}
	pScene->camParam.m_lfFar = lfFar;	
}

static void updateCameraParam(sGLRENDERSCENE *pScene)
{
    if(NULL == pScene)       
        return;
        
    calcCamFarNearPlane(pScene);        
        
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(pScene->rotate_opengl_pt.x, pScene->rotate_opengl_pt.y, 0.0);
	glScalef(window_size.width * 1.0f / pScene->innerviewport[2], window_size.height * 1.0f / pScene->innerviewport[3], 1.0f);
	gluPerspective((f_float64_t)(pScene->camctrl_param.view_angle), window_size.width * 1.0f / window_size.height, 
				   pScene->camParam.m_lfNear, pScene->camParam.m_lfFar);
	glGetDoublev(GL_PROJECTION_MATRIX, pScene->m_lfProjMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	switch(screen_param.rotate_type)
	{   /* 屏幕旋转 */
	    default:
	    case eROTATE_CW_0:
	        /* 默认模式为正常模式，屏幕不旋转 */
	    break;
	    case eROTATE_CW_90:
	        glRotatef(90.0, 0.0, 0.0, 1.0);
	    break;
	    case eROTATE_CW_180:
	        glRotatef(180.0, 0.0, 0.0, 1.0);
	    break;
	    case eROTATE_CW_270:
	        glRotatef(270.0, 0.0, 0.0, 1.0);
	    break;
	}	
		
	if(eROM_inroam == pScene->mdctrl_cmd.rom_mode)
	{			
		glLoadMatrixd(pScene->roam.mv.m);
	}else
	{		
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
		gluLookAt(pScene->camParam.m_ptEye.x,    pScene->camParam.m_ptEye.y,    pScene->camParam.m_ptEye.z,
				  pScene->camParam.m_ptCenter.x, pScene->camParam.m_ptCenter.y, pScene->camParam.m_ptCenter.z,
				  pScene->camParam.m_ptUp.x,     pScene->camParam.m_ptUp.y,     pScene->camParam.m_ptUp.z );
#endif
	}
	glGetDoublev(GL_MODELVIEW_MATRIX, pScene->m_lfModelMatrix);	
}

//手动计算投影矩阵，模式矩阵，视口矩阵，防抖需求lpf add 2017年11月13日14:15:46
/*
 *	获得摄像机的矩阵形式
 */
static void GetCameraMatrix(LPMatrix44 lpMatrix4x4, LP_PT_3D ptCEye, LP_PT_3D ptCCenter, LP_PT_3D ptCUp)
{
	Matrix44 T, R;
	PT_3D n, u, v;
//	   |v(up) y
//	   |
//     /--------u(right) x
//    / 	
//   /n(look) z
	// 因为up

	// 构建
	vector3DSub(&n, ptCEye, ptCCenter);
	vector3DNormalize(&n);			//n
	vector3DCrossProduct(&u, ptCUp, &n);
	vector3DNormalize(&u);			//u
	vector3DCrossProduct(&v, &n, &u);//v
	vector3DNormalize(&v);

	// 平移矩阵: 将坐标从球坐标系平移至视点坐标系
	dMatrix44MemSet(&T, 
		1, 0, 0, -ptCEye->x,
		0, 1, 0, -ptCEye->y,
		0, 0, 1, -ptCEye->z,
		0, 0, 0, 1);
	// 旋转矩阵: 将坐标从球坐标旋转至视点坐标系
	dMatrix44MemSet(&R, u.x, u.y, u.z, 0,
		v.x, v.y, v.z, 0,
		n.x, n.y, n.z, 0,
		0, 0, 0, 1);
	Maxtrix4x4_Mul(lpMatrix4x4, &R, &T);
	dMaxtrix44_Inversion(lpMatrix4x4);
	return;
}
// 计算投影矩阵(theta:平截头视角(相机视角的张角);aspect:投影屏幕宽高比)
static void GetProjectMatrix(LPMatrix44 lpMatrix44, sGLRENDERSCENE *pScene, double theta, double aspect, double znear, double zfar)
{
	double half_theta = theta / 2;
	double cot_v = 1.0 / tan(half_theta * PI / 180);
	double viewCenter_passX = pScene->rotate_opengl_pt.x;	// 视点屏幕坐标
	double viewCenter_passY = pScene->rotate_opengl_pt.y;
	double scale_xyz[3] = {window_size.width * 1.0f / pScene->innerviewport[2], window_size.height * 1.0f / pScene->innerviewport[3], 1.0f};	// 视口到窗体的缩放比
	
	// 构建投影矩阵(将xyz变换至规范化坐标系[-1,1])
	dMatrix44MemSet(lpMatrix44, 
		scale_xyz[0]*cot_v / aspect, 0,	-viewCenter_passX, 0,	// X
		0, scale_xyz[1]*cot_v,		    -viewCenter_passY,	0,	// Y
		0, 0, scale_xyz[2]*(znear + zfar) / (znear - zfar), scale_xyz[2]*2 * znear * zfar / (znear - zfar),	// Z
		0, 0, -1, 0);

	//矩阵求逆	
	dMaxtrix44_Inversion(lpMatrix44);
	return;
}
// 计算远近裁截面
static void updateCameraParamDIY(sGLRENDERSCENE *pScene)
{
    if(NULL == pScene)       
        return;
    
	// 计算相机的远近裁截面
    calcCamFarNearPlane(pScene);        

	// 计算相机投影矩阵
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		{
			Matrix44 temp;
			GetProjectMatrix(&temp, pScene, 
				(f_float64_t)(pScene->camctrl_param.view_angle), window_size.width * 1.0f / window_size.height, 
				pScene->camParam.m_lfNear, pScene->camParam.m_lfFar);
			glLoadMatrixd(temp.m);
			memcpy(pScene->m_lfProjMatrix, temp.m, sizeof(temp.m));
		}
	}

	// 处理屏幕旋转,计算相机模式矩阵并存储
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		switch(screen_param.rotate_type)
		{   /* 屏幕旋转 */
			default:
			case eROTATE_CW_0:
				/* 默认模式为正常模式，屏幕不旋转 */
			break;
			case eROTATE_CW_90:
				glRotatef(90.0, 0.0, 0.0, 1.0);
			break;
			case eROTATE_CW_180:
				glRotatef(180.0, 0.0, 0.0, 1.0);
			break;
			case eROTATE_CW_270:
				glRotatef(270.0, 0.0, 0.0, 1.0);
			break;
		}	
		
		if(eROM_inroam == pScene->mdctrl_cmd.rom_mode)
		{			
			glLoadMatrixd(pScene->roam.mv.m);
			glGetDoublev(GL_MODELVIEW_MATRIX, pScene->m_lfModelMatrix);	
		}
		else
		{	
			Matrix44 modelview_temp;
			GetCameraMatrix(&modelview_temp,
				&(pScene->camParam.m_ptEye),
				&(pScene->camParam.m_ptCenter),
				&(pScene->camParam.m_ptUp));
			if(pScene->camctrl_param.view_type == eVM_SCALE_VIEW)
			{
				glMultMatrixd(pScene->m_lfModelMatrix);
			}
			else
			{
				glMultMatrixd(modelview_temp.m);
			}
		}
		glGetDoublev(GL_MODELVIEW_MATRIX, pScene->m_lfModelMatrix);	
	}
}


/* 获取裁剪矩阵及逆矩阵、获取视口参数 */
static void calCurMatrix(sGLRENDERSCENE *pScene)
{
	f_int32_t i = 0;
	f_float64_t dm[16], dinv[16];
	if(NULL == pScene)
	    return;

	dMatrices44Mult(pScene->m_lfProjMatrix, pScene->m_lfModelMatrix, dm);
	dMatrix44Invert(dm, dinv);

	for (i = 0;i < 16; i++)
	{
		pScene->matrix[i] = dm[i];
		pScene->invmat[i] = dinv[i];
	}
}

static void drawModelsPre(PT_3D ptCenter)
{
	PT_3D ptUp, pNor, pEast, pNorth;
    Matrix44 mx4x4;
    
    glPushMatrix();
        
	vector3DSetValue(&ptUp, &ptCenter);
	vector3DNormalize(&ptUp);
	vector3DMemSet(&pNor, 0.0, 0.0, 1.0);
	vector3DCrossProduct(&pEast, &pNor, &ptUp);
	vector3DNormalize(&pEast);
	vector3DCrossProduct(&pNorth, &pEast, &ptUp);
	vector3DNormalize(&pNorth);	
	dMatrix44MemSet(&mx4x4, -pNorth.x, -pNorth.y, -pNorth.z, 0.0,
						     pEast.x,   pEast.y,   pEast.z,   0.0,
						     ptUp.x,    ptUp.y,    ptUp.z,    0.0,
						     ptCenter.x, ptCenter.y, ptCenter.z, 1.0 );	
	glMultMatrixd(mx4x4.m);
}

void drawModelsPro(void)
{
	glPopMatrix();
}

static void drawCylinder(f_uint32_t *listId, PT_3D base_pt_center, f_float64_t pitch, f_float64_t roll, f_float64_t yaw, 
                            f_float64_t base_radius, f_float64_t radius, f_float64_t cylinder_hei, f_int32_t silces, f_int32_t stacks, f_float32_t* pColor)
{
	f_uint32_t list = *listId;
	
	drawModelsPre(base_pt_center);
	
	{
		yaw = 360.0f - (yaw - 90.0f);
		if(yaw < 0.0f)
		    yaw += 360.0f;
		if(yaw > 360.0f)    
		    yaw -= 360.0f;
		    
	    pitch += 90.0;	
	}
        
    glRotatef(roll, 0.0f, 0.0f, 1.0f);
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
	
	
	glColor4f(pColor[0], pColor[1], pColor[2], pColor[3]);
	if(0 == list)
    {
    	list = glGenLists(1);
    	if(0 == list)
    	{
    	    DEBUG_PRINT("glGenLists error.");	
    	    return;
    	}
    	else
    	{
    		glNewList(list, GL_COMPILE);
//    		glDisableEx(GL_CULL_FACE);
            avicCylinder(base_radius, radius, cylinder_hei, silces, stacks, FALSE);
            glEndList();
            *listId = list;
        }
    }
    else
    {
    	glCallList(list);
    }	
    drawModelsPro();	
}	



/*
求一元二次方程ax*x+bx+c=0的解
*/
void CalOneParamDoubleTime(double a, double b, double c, double *k1,double *k2, BOOL * isReal)
{
	double d;

	d = b*b-4*a*c;

	if(d <0)
	{
		*isReal = FALSE;
		*k1 = 0;
		*k2 = 0;

		return;
	}
	
	*k1 = (-b +sqrt(d))/(2*a);
	*k2 = (-b -sqrt(d))/(2*a);
	*isReal = TRUE;

	

}

// C = A*sin(alpha) + B*cos(alpha) 计算角度值，弧度值
void CalSinA(double A, double B,double C, double * alpha)
{
	double s1 = 0.0, s2 = 0.0;
	BOOL flag = FALSE;
	double calA = B*B + A*A;
	double calB = -2.0 * C*B;
	double calC = C*C - A*A;

	// 首先计算cos()
	CalOneParamDoubleTime(calA, calB, calC, &s1, &s2, &flag);

	if(flag == FALSE)
	{
		return;
	}
	
	// 然后计算角度值
	if(s1 > 0)
	{
		*alpha = acos(s1);
	}
	else
	{
		*alpha = acos(s2);

	}



	//test
	{
		double aa = 0.062714237715496 ;
		double temp;
		


		temp = 5.196152 * sin(aa) + cos(aa) - 1 - 2064560/EARTH_RADIUS;
		if((temp) >FLT_EPSILON	) 
		{
			printf("");
		}

		aa = * alpha;
		temp = A * sin(aa) + B * cos(aa) - C;
		if ((temp)>FLT_EPSILON)
		{
			printf("");

		}
		if ((A/EARTH_RADIUS - 5.196152) >FLT_EPSILON	) 
		{
			if ((C /EARTH_RADIUS -1-2064560 /EARTH_RADIUS)>FLT_EPSILON)
			{
				printf("");
			}
		}
		

	}
	return;


}
//根据视点高度反算量程环半径
//	angle = pScene->camctrl_param.view_res * pScene->camctrl_param.compass_radius / EARTH_RADIUS;
//	hgt = (/*pScene->viewport[3]*/window_size.height * 0.5 * EARTH_RADIUS * sin(angle)) / 
//	      (pScene->camctrl_param.compass_radius * tan((pScene->camctrl_param.view_angle * 0.5) * DE2RA)) 
//	      + EARTH_RADIUS * cos(angle) - EARTH_RADIUS;
// C = A*sin() + B*cos()

double getScaleByEyeHeight(double height, double fovy, double vpHeight, double radius)
{
double tan_fovy ,tan_beta,beta,sin_beta,maxDis,dis,sin_alpha,alpha,theta,arc_len,scale;

	if(height < 1.0) return 1.0;   //在很低，比例尺设置为1

	 tan_fovy = tan(fovy*0.5*DE2RA);
	 tan_beta = radius * tan_fovy / (vpHeight * 0.5);
	 beta = atan(tan_beta);
	 sin_beta = sin(beta);

	 maxDis = EARTH_RADIUS / sin_beta;
	 dis = height + EARTH_RADIUS;
	if(dis > maxDis) 
	{
		printf("------------\n");
			dis = maxDis;
	}

	 sin_alpha = dis * sin_beta / EARTH_RADIUS;
	 alpha = asin(sin_alpha);
	alpha = PI- alpha;
	 theta = PI - beta - alpha;

	 arc_len = EARTH_RADIUS * theta;
	 scale = arc_len / radius;
//	printf("beta = %lf, alpha=%lf, theta=%lf\n", sse::RadiansToDegrees(beta), sse::RadiansToDegrees(alpha), sse::RadiansToDegrees(theta));
	return scale;
}




void Hgt2Measure(sGLRENDERSCENE *pScene, double * measure_dis)
{
	double A=0.0, B=0.0, C=0.0;
	double angle = 0.0;
	double measure = 100000.0;
	
	if (pScene->roam.thisPitch >= -30.0)
	{
		C = pScene->camParam.m_geoptEye.height *1000 +  EARTH_RADIUS ; 
	}
	else if (pScene->roam.thisPitch == -90.0)
	{
		C = pScene->camParam.m_geoptEye.height / cos((90.0 + pScene->roam.thisPitch) * RA2DE)  +  EARTH_RADIUS ; 
	}
	else
	{
		C = pScene->camParam.m_geoptEye.height / cos((90.0 + pScene->roam.thisPitch+30.0) * RA2DE)  +  EARTH_RADIUS ; 

	}
	C = pScene->camParam.m_geoptEye.height   +  EARTH_RADIUS ; 

	A = 	(window_size.height * 0.5 * EARTH_RADIUS ) /(pScene->camctrl_param.compass_radius * tan((pScene->camctrl_param.view_angle * 0.5) * DE2RA)) ;
	B =		EARTH_RADIUS ;
	
	CalSinA( A,  B, C, &angle);

	//俯仰角大于-80时，量程采用800 000
	if (pScene->roam.thisPitch > - 65.0)
	{
		
		measure = 400000.0;
	}
	
	if(pScene->roam.thisPitch > - 45.0)
	{
		measure = 600000.0;
	}

	if (pScene->roam.thisPitch == -90.0)
	{
		* measure_dis = angle *  EARTH_RADIUS;
		

	}
	else
	{
		* measure_dis = measure;
		//pScene->mdctrl_cmd.map2dor3d = 1;

		//lpf delete 2017年11月10日8:56:42 , 
		//此处是在演示程序中在pitch 小于45度时，不出现矢量地名，出现三维的地名
		//if (pScene->roam.thisPitch >= -45.0)
		//{
		//	pScene->camctrl_param.view_type = eVM_COCKPIT_VIEW;
		//}
	}

			* measure_dis = angle *  EARTH_RADIUS;

//			printf("%f\n", * measure_dis -  pScene->camctrl_param.compass_radius * getScaleByEyeHeight( pScene->camParam.m_geoptEye.height, 
//				60.0, window_size.height, pScene->camctrl_param.compass_radius));

			


	//printf("%f,,,,%f\n",* measure_dis,pScene->roam.thisPitch);
}

extern BOOL setModeMeasure_dis2d(f_float32_t distance);
extern BOOL setAttitudeYaw2d(f_float64_t yaw);



extern void geo2obj(f_float64_t lon, f_float64_t lat, f_float64_t ht,
                    f_float64_t *x, f_float64_t *y, f_float64_t *z);

void calTruepos(VOIDPtr scene, LP_PT_3D out_xyz, LP_Geo_Pt_D out_lonlathei)
{
	f_float64_t m_lfModelMatrix[16];
	f_float64_t outMatrix[16];
	f_float64_t angle;
	f_float64_t hgt;
	f_float64_t x, y, z;
//	f_float64_t lon, lat, height;
	sGLRENDERSCENE *pScene = NULL;

	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return;	

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	angle = pScene->camctrl_param.view_res * pScene->camctrl_param.compass_radius / EARTH_RADIUS;
	hgt = (/*pScene->viewport[3]*/window_size.height * 0.5 * EARTH_RADIUS * sin(angle)) / 
	      (pScene->camctrl_param.compass_radius * tan((pScene->camctrl_param.view_angle * 0.5) * DE2RA)) 
	      + EARTH_RADIUS * cos(angle) - EARTH_RADIUS;

	geo2obj(
		pScene->geopt_pos_input.lon,
		pScene->geopt_pos_input.lat, 0.0, &x, &y, &z);

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glTranslatef(0.0, 0.0, -hgt);

	glRotated(pScene->attitude.roll , 0.0, 0.0, 1.0);

	//glRotated(pScene->attitude.roll , 0.0, 0.0, 1.0);
	glRotated(-pScene->attitude.pitch , 1.0, 0.0, 0.0);
	glRotated(360.0-pScene->attitude.yaw , 0.0, 1.0, 0.0);

	glRotated(-(180.0-pScene->camParam.m_geoptEye.lat), 1.0, 0.0, 0.0);
	glRotated(-(90.0+pScene->camParam.m_geoptEye.lon), 0.0, 0.0, 1.0);
	glTranslatef(-x, -y, -z);

	glGetDoublev(GL_MODELVIEW_MATRIX, m_lfModelMatrix);
	glPopMatrix();

	memcpy(pScene->m_lfModelMatrix , m_lfModelMatrix, sizeof(double) * 16);

#else
	{
		ESMatrix mat;
		int prog = 0;
		int loc = 0;
		int i = 0,j = 0;

		esMatrixLoadIdentity(&mat);

		esTranslate(&mat, 0.0, 0.0, -hgt);
		esTranslate(&mat, -x, -y, -z);
		esRotate(&mat, -(90.0+pScene->camParam.m_geoptEye.lon), 0.0, 0.0, 1.0);
		esRotate(&mat, -(180.0-pScene->camParam.m_geoptEye.lat), 1.0, 0.0, 0.0);
		esRotate(&mat, 360.0-pScene->attitude.yaw , 0.0, 1.0, 0.0);
		esRotate(&mat, -pScene->attitude.pitch , 1.0, 0.0, 0.0);
		esRotate(&mat, pScene->attitude.roll , 0.0, 0.0, 1.0);


		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		loc = glGetUniformLocation(prog, "u_vMatrix");

		glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mat);

		for (i = 0; i<4; i++)
		{
			for (j = 0; j<4; j++)
			{
				m_lfModelMatrix[i*4+j] = (f_float64_t)mat.m[i][j];
			}	
		}
	}
	glPopMatrix();

#endif
	
	dMatrix44Invert(m_lfModelMatrix, outMatrix);

	out_xyz->x = outMatrix[12]/outMatrix[15];
	out_xyz->y = outMatrix[13]/outMatrix[15];
	out_xyz->z = outMatrix[14]/outMatrix[15];
	
	objDPt2geoDPt(out_xyz, out_lonlathei);
}






/**
 * @fn void render3dMap(VOIDPtr *scene)
 *  @brief 3d地图场景渲染.
 *  @param[in] scene  场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void render3dMap(VOIDPtr scene)
{
	sGLRENDERSCENE *pScene = NULL;

	pScene = scenePtrValid(scene);
	if(NULL == pScene)
		return;	

	//glEnableEx(GL_CULL_FACE);

	/* 打开深度测试 */
	glEnableEx(GL_DEPTH_TEST);

#ifdef WIN32
	//glEnableEx(GL_DEPTH_CLAMP);	//防止视点穿地
#endif

	/* 如果需要绘制三维;当绘制纯二维矢量(底色为纯色),无需绘制三维俯视视角的底图时,该控制变量为1;其他情况该控制变量均为0 */
	if(pScene->mdctrl_cmd.close3d_mode == 0)	
	{
		f_float32_t pitch = 0.0f;

		/* 如果是三维漫游状态,俯仰角为漫游输入的俯仰角 */
		if(eROM_inroam == pScene->mdctrl_cmd.rom_mode)
		{
			pitch = pScene->roam.thisPitch;
		}else{
			/* 如果是非三维漫游状态,若为俯视视角，则俯仰角强制为-90度;若为其他视角,则为实际飞机的俯仰角 */
			if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
			{
				pitch = -90.0f;
			}
			else
			{
				pitch = pScene->attitude.pitch;
			}
		} 
		
		/* 三维场景渲染 */
		qtmapRender(pMapDataSubTree, pScene->mdctrl_cmd.terwarn_mode, pitch, pScene); 
	}

	/* 关闭深度测试 */
	glDisableEx(GL_DEPTH_TEST);

	//glDisableEx(GL_CULL_FACE); 
}

/**
 * @fn void render2dMap(VOIDPtr *scene)
 *  @brief 2d地图场景渲染.
 *  @param[in] scene  场景句柄.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void render2dMap(VOIDPtr scene)
{
	sGLRENDERSCENE *pScene = NULL;
	f_float32_t r, g, b, gray;
	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return;	

    r = 1.0f;    g = 1.0f;  b = 0.0f;
    gray = 0.299 * r + 0.587 * g + 0.114 * b;
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* 灰色模式 */ 
        glColor3f(gray, gray, gray);
    else
        glColor3f(r, g, b);
	glLineWidth(1.0f);
	glBegin(GL_TRIANGLES);
	    glVertex2f(0.75f, 0.75f);
	    glVertex2f(0.75f, 0.85f);
	    glVertex2f(0.85f, 0.75f);
	glEnd();
	
	r = 1.0f;    g = 0.0f;    b = 0.0f;
    gray = 0.299 * r + 0.587 * g + 0.114 * b;
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* 灰色模式 */ 
        glColor3f(gray, gray, gray);
    else
        glColor3f(r, g, b);
	glBegin(GL_LINES);
	    glVertex2f(-0.5f, -0.5f);
	    glVertex2f(0.5f, 0.5f);
	    glVertex2f(-0.5f, -0.5f);
	    glVertex2f(-0.5f, 0.5f);
	glEnd();
	
	r = 0.0f;    g = 1.0f;    b = 0.0f;
    gray = 0.299 * r + 0.587 * g + 0.114 * b;
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* 灰色模式 */ 
        glColor3f(gray, gray, gray);
    else
        glColor3f(r, g, b);
	glBegin(GL_LINES);
	    glVertex2f(-0.5f, 0.5f);
	    glVertex2f(0.5f, -0.5f);
	glEnd();
	
	r = 0.0f;    g = 0.0f;    b = 1.0f;
    gray = 0.299 * r + 0.587 * g + 0.114 * b;
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* 灰色模式 */ 
        glColor3f(gray, gray, gray);
    else
        glColor3f(r, g, b);
	glBegin(GL_LINES);
	    glVertex2f(-0.5f, 0.5f);
	    glVertex2f(0.5f, 0.5f);
	glEnd();
}

/*
 * 功能：判断某个经纬度点是否在瓦片范围内
 * 输入：lfLon 经度
 *       lfLat 纬度
 * 输出：无
 * 返回：在，返回TRUE(1)；不在，返回FALSE(0)
 */
 BOOL InTile(const sQTMAPNODE  *pTile, const f_float64_t lon, const f_float64_t lat)
{
	return ((lon- pTile->hstart - pTile->hscale)*(lon-pTile->hstart)<=0 && (lat-pTile->vstart - pTile->vscale)*(lat-pTile->vstart )<=0);
}


/*
 * 功能：获取指定经纬度点在已加载到内存中的瓦片中的高度值
 * 输入：pScene 渲染场景句柄
 *       lfLon 经度
 *       lfLat 纬度
 * 输出：*pZ 高度
 * 返回：获取成功，返回TRUE(1)；获取失败，返回FALSE(0)
 */
BOOL GetZ(sGLRENDERSCENE *pScene, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ)
{
	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
	sMAPHANDLE * pHandle = pMapDataSubTree;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/* 循环查找LRU链表里的节点 */
	pstListHead = &(pHandle->nodehead.stListHead);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);

			if( pNode->level < iLevel)
				continue;

			/* 判断该经纬度点是否在瓦片范围内 */
			if(InTile(pNode, lfLon, lfLat))
			{
				// linear z;
				double x,y,dx,dy;
				int i,j,k,l;

				/* 每个瓦片包含33*33个点，找到该经纬度点落在瓦片中哪4个点中间 */
				k = VTXCOUNT  +  1;
				dy = fabs(pNode->vscale)/VTXCOUNT;
				dx = fabs(pNode->hscale)/VTXCOUNT;
				y = fabs(lfLat - pNode->vstart);
				x = fabs(lfLon - pNode->hstart);
				y /= dy;
				x /= dx;
				i = (int)y;
				j = (int)x;
				y -= i;
				x -= j;
				l = i*k+j;

	
				if (NULL != pNode->fterrain 
					&& NULL != &(pNode->fterrain[l]) 
					&& NULL != &(pNode->fterrain[l+1]) 
					&& NULL != &(pNode->fterrain[l+k]) 
					&& NULL != &(pNode->fterrain[l+k+1]))
				{
					/* 由4个点的高程值按比例求平均,得到目标点的高度 */
					*pZ = pNode->fterrain[l]*(1-x)*(1-y) + pNode->fterrain[l+1]*x*(1-y) + 
						   pNode->fterrain[l+k]*(1-x)*y + pNode->fterrain[l+k+1]*x*y;
					/* 继续查找比该节点层级更大的节点,获取更准确的高度 */
					iLevel = pNode->level + 1;
				}
			}
		}
	}	

	giveSem(pHandle->rendscenelock);


	/* 查找到的节点不为0,说明高度值查到了 */
	if (iLevel != 0)
		return TRUE;
	else
		return FALSE;
}


/*
 * 功能：获取指定经纬度点在已加载到内存中的瓦片中的高度值,与GetZ函数一样,只是第一个输入参数不一样
 * 输入：pHandle 地图场景树句柄
 *       lfLon 经度
 *       lfLat 纬度
 * 输出：*pZ 高度
 * 返回：获取成功，返回TRUE(1)；获取失败，返回FALSE(0)
 */
BOOL GetZex(sMAPHANDLE * pHandle, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ)
{
	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/*查找LRU链表里有没有待加载的节点*/
	pstListHead = &(pHandle->nodehead.stListHead);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);

			if( pNode->level < iLevel)
				continue;

			if(InTile(pNode, lfLon, lfLat))
			{
				// linear z;
				double x,y,dx,dy;
				int i,j,k,l;

				k = VTXCOUNT  +  1;
				dy = fabs(pNode->vscale)/VTXCOUNT;
				dx = fabs(pNode->hscale)/VTXCOUNT;
				y = fabs(lfLat - pNode->vstart);
				x = fabs(lfLon - pNode->hstart);
				y /= dy;
				x /= dx;
				i = (int)y;
				j = (int)x;
				y -= i;
				x -= j;
				l = i*k+j;

	
				if (NULL != pNode->fterrain 
					&& NULL != &(pNode->fterrain[l]) 
					&& NULL != &(pNode->fterrain[l+1]) 
					&& NULL != &(pNode->fterrain[l+k]) 
					&& NULL != &(pNode->fterrain[l+k+1]))
				{
					*pZ = pNode->fterrain[l]*(1-x)*(1-y) + pNode->fterrain[l+1]*x*(1-y) + 
						   pNode->fterrain[l+k]*(1-x)*y + pNode->fterrain[l+k+1]*x*y;
					
					iLevel = pNode->level + 1;
				}
			}		
		}
	}	

	giveSem(pHandle->rendscenelock);
	
	if (iLevel != 0)
		return TRUE;
	else
		return FALSE;
}

void GetNormalOverlook(const f_float64_t x0, const f_float64_t y0, const f_float64_t z0, 
	const f_float64_t x1, const f_float64_t y1, const f_float64_t z1, double* pNormal)
{
	f_float64_t x,y,z,w;
	x = y0*z1 - z0*y1;
	y = -x0*z1 + z0*x1;
	z = x0*y1 - y0*x1;

	w = sqrt( x*x + y*y + z*z );

	w = 1/w;
	pNormal[0] = x*w;
	pNormal[1] = y*w;
	pNormal[2] = z*w;
}

//射线方程是 p(t) = p0 + t*d0,其中d0、p(t)、p0是三维向量
//平面方程是p*n = d
void Ray2TrangleIntersect(PT_3D n, double d, PT_3D p0, PT_3D d0, double *t)
{
	*t = (d - vector3DDotProduct(&p0, &n))/(vector3DDotProduct(&d0, &n));
 

}

//查找俯视视角下当前绘制链表的瓦片中对应经纬度的高度值
BOOL GetZOverLookOld(sGLRENDERSCENE *pScene, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ)
{
	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
	sMAPHANDLE * pHandle = pMapDataSubTree;
	Geo_Pt_D geoPt;
	PT_3D p1, p2, p3;
	

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/*查找LRU链表里有没有待加载的节点*/
//	pstListHead = &(pHandle->nodehead.stListHead);
	pstListHead = &(pScene->scene_draw_list);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
//			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);

			

			if( pNode->level < iLevel)
				continue;

			if(InTile(pNode, lfLon, lfLat))
			{
				// linear z;
				double x,y,dx,dy;
				int i,j,k,l;
				double  normal[3] = {0};
				double d  = 0.0;
	

				k = VTXCOUNT  +  1;
				dy = fabs(pNode->vscale)/VTXCOUNT;
				dx = fabs(pNode->hscale)/VTXCOUNT;
				y = fabs(lfLat - pNode->vstart);
				x = fabs(lfLon - pNode->hstart);
				y /= dy;
				x /= dx;
				i = (int)y;
				j = (int)x;
				y -= i;
				x -= j;
				l = i*k+j;

	
				if (NULL != pNode->fterrain 
					&& NULL != &(pNode->fterrain[l]) 
					&& NULL != &(pNode->fterrain[l+1]) 
					&& NULL != &(pNode->fterrain[l+k]) 
					&& NULL != &(pNode->fterrain[l+k+1]))
				{


					//判断点在哪个三角形面内
					y = (lfLat - pNode->vstart);
					x = (lfLon - pNode->hstart);
					if(y/x  > 1.0)
					{


						p1.x = pNode->fvertex_overlook[0];//pNode->hstart;
						p1.y = pNode->fvertex_overlook[1];//pNode->vstart;
						p1.z = pNode->fvertex_overlook[2];//pNode->height_lb;

						p2.x = pNode->fvertex_overlook[6];//pNode->hstart + pNode->hscale;
						p2.y = pNode->fvertex_overlook[7];//pNode->vstart + pNode->vscale;
						p2.z = pNode->fvertex_overlook[8];//pNode->height_rt;

						
						p3.x = pNode->fvertex_overlook[9];//pNode->hstart;
						p3.y = pNode->fvertex_overlook[10];//pNode->vstart + pNode->vscale;
						p3.z = pNode->fvertex_overlook[11];//pNode->height_lt;


						
						
					}
					else
					{
						p1.x = pNode->fvertex_overlook[0];//pNode->hstart;
						p1.y = pNode->fvertex_overlook[1];//pNode->vstart;
						p1.z = pNode->fvertex_overlook[2];//pNode->height_lb;

						p2.x = pNode->fvertex_overlook[6];//pNode->hstart + pNode->hscale;
						p2.y = pNode->fvertex_overlook[7];//pNode->vstart + pNode->vscale;
						p2.z = pNode->fvertex_overlook[8];//pNode->height_rt;

						
						p3.x = pNode->fvertex_overlook[3];//pNode->hstart + pNode->hscale;
						p3.y = pNode->fvertex_overlook[4];//pNode->vstart;
						p3.z = pNode->fvertex_overlook[5];//pNode->height_rb;
					}
					

						GetNormalOverlook(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z,
							p3.x - p1.x, p3.y - p1.y, p3.z - p1.z,
							&normal[0]) ;

						d = normal[0] * p1.x + normal[1] * p1.y +normal[2] * p1.z ;
						//计算该点在三角形面内的位置
						//*pZ = (d - normal[0] * lfLon - normal[1] * lfLat )/normal[2];



						//计算射线方程					
						//射线方程是 p(t) = p0 + t*d0,其中d0、p(t)、p0是三维向量
						{
							PT_3D p0 = {0};
							PT_3D d0 = {cos(lfLon * DE2RA) * cos(lfLat * DE2RA), sin(lfLon * DE2RA) * cos(lfLat * DE2RA), sin(lfLat * DE2RA) };
							double t = 0.0;
							PT_3D pt ={0};

							PT_3D n = {normal[0], normal[1], normal[2]};
							

							Ray2TrangleIntersect(n,  d,  p0,  d0, &t);

							vector3DMul(&pt, &d0, t);

							objDPt2geoDPt(&pt, &geoPt);

							*pZ = geoPt.height;
						}


					
					iLevel = pNode->level + 1;
				}			
			}	
		}
	}	

	giveSem(pHandle->rendscenelock);

#if 0
	{
		if(!drawGeoUserLayerPre(pScene))
		{
		    DEBUG_PRINT("drawGeoUserLayerPre error.");
			return FALSE;
		}

			
		glColor3f(1.0,0.0,0.0);
		glLineWidth(5.0);
		glDisableEx(GL_DEPTH_TEST);
		glDisableEx(GL_TEXTURE_2D);
		
		glBegin(GL_LINE_LOOP);
			glVertex3f(p1.x , p1.y, p1.z);		
			glVertex3f(p2.x , p2.y, p2.z);		
			glVertex3f(p3.x , p3.y, p3.z);		
		glEnd();

		{		

			PT_3D d0 = {
					2*EARTH_RADIUS * cos(lfLon * DE2RA) * cos(lfLat * DE2RA), 
					2*EARTH_RADIUS * sin(lfLon * DE2RA) * cos(lfLat * DE2RA), 
					2*EARTH_RADIUS * sin(lfLat * DE2RA) };

			glColor3f(1.0,1.0,0.0);
			glLineWidth(10.0);

			glBegin(GL_LINES);
						glVertex3f(0.0,0.0,0.0);				
						glVertex3f(d0.x , d0.y, d0.z);		
			glEnd();


		}



		glEnableEx(GL_TEXTURE_2D);


		drawUserLayerPro();	
	}

#endif
	
	if (iLevel != 0)
		return TRUE;
	else
		return FALSE;
}

//查找非俯视视角下当前绘制链表的瓦片中对应经纬度的高度值
BOOL GetZFreeLookOld(sGLRENDERSCENE *pScene, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ)
{
	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
	sMAPHANDLE * pHandle = pMapDataSubTree;
	Geo_Pt_D geoPt;
	PT_3D p1, p2, p3;
	

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/*查找LRU链表里有没有待加载的节点*/
//	pstListHead = &(pHandle->nodehead.stListHead);
	pstListHead = &(pScene->scene_draw_list);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
//			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);

			

			if( pNode->level < iLevel)
				continue;

			if(InTile(pNode, lfLon, lfLat))
			{
				// linear z;
				double x,y,dx,dy;
				int i,j,k,l;
				double  normal[3] = {0};
				double d  = 0.0;
	

				k = VTXCOUNT  +  1;
				dy = fabs(pNode->vscale)/VTXCOUNT;
				dx = fabs(pNode->hscale)/VTXCOUNT;
				y = fabs(lfLat - pNode->vstart);
				x = fabs(lfLon - pNode->hstart);
				y /= dy;
				x /= dx;
				i = (int)y;
				j = (int)x;
				y -= i;
				x -= j;
				l = i*k+j;

	
				if (NULL != pNode->fterrain 
					&& NULL != &(pNode->fterrain[l]) 
					&& NULL != &(pNode->fterrain[l+1]) 
					&& NULL != &(pNode->fterrain[l+k]) 
					&& NULL != &(pNode->fterrain[l+k+1]))
				{


					//判断点在哪个三角形面内
					//y = (lfLat - pNode->vstart);
					//x = (lfLon - pNode->hstart);
					if(y/(1.0 - x)  > 1.0)
					{


						p1.x = pNode->fvertex[3 *( l + 1) ];	//pNode->hstart + pNode->hscale;
						p1.y = pNode->fvertex[3 *( l + 1) + 1];	//pNode->vstart;
						p1.z = pNode->fvertex[3 *( l + 1) + 2];	//pNode->height_lb;

						p2.x = pNode->fvertex[3 *( l + k + 1)];		//pNode->hstart + pNode->hscale;
						p2.y = pNode->fvertex[3 *( l + k + 1) + 1];	//pNode->vstart + pNode->vscale;
						p2.z = pNode->fvertex[3 *( l + k + 1) + 2];	//pNode->height_rt;

						
						p3.x = pNode->fvertex[3 *( l + k )];		//pNode->hstart;
						p3.y = pNode->fvertex[3 *( l + k ) + 1];	//pNode->vstart + pNode->vscale;
						p3.z = pNode->fvertex[3 *( l + k ) + 2];	//pNode->height_lt;


						
						
					}
					else
					{
						p1.x = pNode->fvertex[3 *( l + 1) ];	//pNode->hstart + pNode->hscale;
						p1.y = pNode->fvertex[3 *( l + 1) + 1];	//pNode->vstart;
						p1.z = pNode->fvertex[3 *( l + 1) + 2];	//pNode->height_lb;
					
						p2.x = pNode->fvertex[3 * l ];	//pNode->hstart;
						p2.y = pNode->fvertex[3 * l + 1];	//pNode->vstart;
						p2.z = pNode->fvertex[3 * l + 2];	//pNode->height_lb;

						p3.x = pNode->fvertex[3 *( l + k )];		//pNode->hstart;
						p3.y = pNode->fvertex[3 *( l + k ) + 1];	//pNode->vstart + pNode->vscale;
						p3.z = pNode->fvertex[3 *( l + k ) + 2];	//pNode->height_lt;
					}
					

						GetNormalOverlook(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z,
							p3.x - p1.x, p3.y - p1.y, p3.z - p1.z,
							&normal[0]) ;

						d = normal[0] * p1.x + normal[1] * p1.y +normal[2] * p1.z ;
						//计算该点在三角形面内的位置
						//*pZ = (d - normal[0] * lfLon - normal[1] * lfLat )/normal[2];



						//计算射线方程					
						//射线方程是 p(t) = p0 + t*d0,其中d0、p(t)、p0是三维向量
						{
							PT_3D p0 = {0};
							PT_3D d0 = {cos(lfLon * DE2RA) * cos(lfLat * DE2RA), sin(lfLon * DE2RA) * cos(lfLat * DE2RA), sin(lfLat * DE2RA) };
							double t = 0.0;
							PT_3D pt ={0};

							PT_3D n = {normal[0], normal[1], normal[2]};
							

							Ray2TrangleIntersect(n,  d,  p0,  d0, &t);

							vector3DMul(&pt, &d0, t);

							objDPt2geoDPt(&pt, &geoPt);

							*pZ = geoPt.height;
						}


					
					iLevel = pNode->level + 1;
				}				
			}		
		}
	}	

	giveSem(pHandle->rendscenelock);

	if(0)
	{
		if(!drawGeoUserLayerPre(pScene))
		{
		    DEBUG_PRINT("drawGeoUserLayerPre error.");
			return FALSE;
		}

			
		glColor3f(1.0,0.0,0.0);
		glLineWidth(5.0);
		glDisableEx(GL_DEPTH_TEST);
		glDisableEx(GL_TEXTURE_2D);
		
		glBegin(GL_LINE_LOOP);
			glVertex3f(p1.x , p1.y, p1.z);		
			glVertex3f(p2.x , p2.y, p2.z);		
			glVertex3f(p3.x , p3.y, p3.z);		
		glEnd();

		{		

			PT_3D d0 = {
					2*EARTH_RADIUS * cos(lfLon * DE2RA) * cos(lfLat * DE2RA), 
					2*EARTH_RADIUS * sin(lfLon * DE2RA) * cos(lfLat * DE2RA), 
					2*EARTH_RADIUS * sin(lfLat * DE2RA) };

			glColor3f(1.0,1.0,0.0);
			glLineWidth(10.0);

			glBegin(GL_LINES);
						glVertex3f(0.0,0.0,0.0);				
						glVertex3f(d0.x , d0.y, d0.z);		
			glEnd();


		}



		glEnableEx(GL_TEXTURE_2D);


		drawUserLayerPro();	
	}


	
	if (iLevel != 0)
		return TRUE;
	else
		return FALSE;
}

/*绑定tga纹理,只有1份tga纹理,index只能取0*/
extern void textureUse(int index);
/*从tga纹理取出飞机符号的纹理并进行绘制*/
/*
输入：
	x_center，y_center 飞机符号中心位置
	symbol_index 符号索引
	angle 旋转角
*/
extern void outPutPlaneSymbol(float x_center, float y_center, int symbol_index, float angle);

/*
功能：绘制飞机符号
输入：
	pScene 场景句柄
输出：
    0-成功  -1-失败
*/
int drawPlaneSymbol(sGLRENDERSCENE *pScene)
{
	f_int32_t startx = 0.0, starty = 0.0, width = 0.0, height = 0.0;
	PT_3D plane_view_pt;
	Geo_Pt_D plane_pos;
	f_float64_t plane_yaw = 0.0;
	
	if(NULL == pScene)
		return -1;	

	plane_pos.lon = pScene->planesymbol_pos.lon;
	plane_pos.lat = pScene->planesymbol_pos.lat;
	plane_pos.height = pScene->planesymbol_pos.height;

	//获取飞机符号绘制的角度
	switch(screen_param.rotate_type)
	{   
		case eROTATE_CW_0:
			plane_yaw = (360 - pScene->planesymbol_yaw);
			break;
		case eROTATE_CW_90:
			plane_yaw = (270 - pScene->planesymbol_yaw);
			break;
		case eROTATE_CW_180:
			plane_yaw = (180 - pScene->planesymbol_yaw);
			break;
		case eROTATE_CW_270:
			plane_yaw = (90 - pScene->planesymbol_yaw);
			break;
		default:
			plane_yaw = (360 - pScene->planesymbol_yaw);
			break;
	}

	//获取飞机符号内部视口坐标	
	if (ePRJ_mercator == pScene->project_mode)
	{
		geoDPt2InnerViewDPtMct(&plane_pos,pScene->matrix,pScene->innerviewport,&plane_view_pt);
	}else if (ePRJ_ball == pScene->project_mode)
	{
		geoDPt2InnerViewDPt(&plane_pos,pScene->matrix,pScene->innerviewport,&plane_view_pt);
	}

	startx = pScene->innerviewport[0];
	starty = pScene->innerviewport[1];
	width  = pScene->innerviewport[2];
	height = pScene->innerviewport[3];

	//漫游状态不判断飞机是否出边界，否则会导致漫游时飞机符号超出屏幕之后被强制拉回来
	//设置中心点状态下不判断飞机是否出边界，否则会导致设置中心点后进行缩放飞机符号超出屏幕之后被强制拉回来
	if((eROM_outroam == pScene->mdctrl_cmd.rom2d_mode) && (0 == pScene->set_center_pos))
	{
        //飞机符号内部视口坐标到达视口的任意边界，更新地图中心点的坐标为当前飞机符号的坐标,未考虑旋转
        if ( (plane_view_pt.y + 12 > height) || (plane_view_pt.y - 12 < starty) ||
             (plane_view_pt.x + 12 > width) || (plane_view_pt.x - 12 < 0))
        {
            pScene->geopt_pos = pScene->planesymbol_pos;
        }
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); 
	glLoadIdentity(); 

	glOrtho(0, width, 0, height, -1.0f, 1.0f); 
	 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
	glLoadIdentity();
    glViewport(startx, starty, width, height);
	
    /* 初始化点状符合的纹理环境 */
    glEnableEx(GL_TEXTURE_2D);
	textureUse(0);
	
	glEnableEx(GL_BLEND);	 
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glColor3f(0.0f, 0.78f, 0.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glColor3f(0.0f, 1.0f, 0.0f);
    //输出飞机符号,歼击机-16,直升机-23        
    outPutPlaneSymbol(plane_view_pt.x, plane_view_pt.y, 16, plane_yaw);
    
    glDisableEx(GL_BLEND);
	glDisableEx(GL_TEXTURE_2D);
	
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();	
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
        
    return 0;	
}
