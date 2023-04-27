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
1. ��Ļ�����Ƶ�Ԫ���Ӵ�������֮��Ĺ�ϵ:
    һ����ʾ��ֻ��һ����Ļ����Ӧ������Ƶ�Ԫ��
    һ�����Ƶ�Ԫ����һ���Ӵ���һ��������
    ��һ������������attach����һ�Ӵ��С�
*/

sSCREENPARAM screen_param;
/*�ڲ���Ļ����ϵ�£�����Ļ�����ڷ�(δ��ת)ʱ�Ŀ�Ⱥ͸߶�*/
WDSINZE_2I   window_size;

/*3d����ʱ�Ƿ񴩵�*/
int roamScale = 0;

/*��ͼ������*/
extern sMAPHANDLE * pMapDataSubTree;			
extern void CalAndSetVecMatrix(int viewPortIndex, double *modelview_matrix, double *prj_matrix, double *neardis, double *fardis);

void Hgt2Measure(sGLRENDERSCENE *pScene, double * measure_dis);
static void updateCameraParamDIY(sGLRENDERSCENE *pScene);
static void calCurMatrix(sGLRENDERSCENE *pScene);
void calTruepos(VOIDPtr scene, LP_PT_3D out_xyz, LP_Geo_Pt_D out_lonlathei);

/*
���ܣ����Ʒɻ�����
���룺
	pScene �������
�����
    0-�ɹ�  -1-ʧ��
*/
int drawPlaneSymbol(sGLRENDERSCENE *pScene);

/**
 * @fn void setScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height,\n
                           f_int32_t width, f_int32_t height)
 *  @brief ��ʼ����ʾ���Ĳ���.
 *  @param[in] rotate_type  ��ʾ������ʽ��0:����,1:˳ʱ��ת90��,2:˳ʱ����ת180��,3:˳ʱ����ת270��. 
 *  @param[in] phi_width    ��ʾ������ߴ�Ŀ��(��λ������). 
 *  @param[in] phi_height   ��ʾ������ߴ�ĸ߶�(��λ������). 
 *  @param[in] width        ��ʾ���ֱ��ʳߴ�Ŀ��(��λ������). 
 *  @param[in] height       ��ʾ���ֱ��ʳߴ�ĸ߶�(��λ������). 
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
 *  @brief ��ȡ��Ļ��ת��ʽ. 0:����,1:˳ʱ��ת90��,2:˳ʱ����ת180��,3:˳ʱ����ת270��.
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
 *  @brief ������Ļ���ڳߴ�(���طֱ���). 
 *  @param[in] width    ���ڵĿ��(��λ������). 
 *  @param[in] height   ���ڵĸ߶�(��λ������). 
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
 *  @brief ��ȡ��Ļ���ڳߴ�(���طֱ���). 
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
 *  @brief ��ά��ͼ������Ⱦ.
 *  @param[in] pScene �������.
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
 *  @brief ��ά��ͼ������Ⱦ.
 *  @param[in] pScene �������.
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
 *  @brief �û���Ļͼ����Ⱦ.
 *  @param[in] pScene �������.
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
 *  @brief �û�����ͼ����Ⱦ.
 *  @param[in] pScene �������.
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
 *  @brief ��Ļ����ת���ɵ�������.
 *  @param[in] pScene �������.
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
 *  @brief ��������ת������Ļ����.
 *  @param[in] pScene �������.
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
 *  @brief ���ڱ���ɫ���.
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
 *  @brief ����͸�������,���Ƶ�ͼ����.
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
 *  @brief �ж�����Ļ��Ƶ�Ԫ����Ƿ���Ч.
 *  @param[in] punit ��Ԫ���.
 *  @exception void
 *  @return sPAINTUNIT *, ��Ԫ���.
 *  @retval ���ھ��.
 *  @retval NULL ���ھ����Ч.
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
 *  @brief �ж��������Ⱦ�Ӵ�����Ƿ���Ч.
 *  @param[in] view_window �Ӵ����.
 *  @exception void
 *  @return sViewWindow���Ӵ����.
 *  @retval �Ӵ����.
 *  @retval NULL �Ӵ������Ч.
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
 *  @brief �ж�����ĳ�������Ƿ���Ч.
 *  @param[in] render_scene �������.
 *  @exception void
 *  @return sGLRENDERSCENE���������.
 *  @retval �������.
 *  @retval NULL ���������Ч.
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
 *  @brief �ж�����������Ĳ�������Ƿ���Ч.
 *  @param[in] render_scene �����Ĳ������.
 *  @exception void
 *  @return sGLRENDERSCENE�������Ĳ������.
 *  @retval �����Ĳ������.
 *  @retval NULL �����Ĳ��������Ч.
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
 *  @brief �����Ӵ�������ǰ�ú�������ȡ�������õ��ź���. 
 *  @param[in] view_window �Ӵ����.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
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
 *  @brief �����Ӵ������ĺ��ú������ͷŲ������õ��ź���. 
 *  @param[in] view_window �Ӵ����.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
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
 *  @brief �����Ӵ���͸����.
 *  @param[in] view_window �Ӵ����.
 *  @param[in] alpha ͸����.
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
 *  @brief �����Ӵ��ı���ɫ.
 *  @param[in] view_window �Ӵ����.
 *  @param[in] bkcolor ����ɫ.
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
 *  @brief ������Ⱦ�Ӵ����йز����������ߴ硢���̻��뾶��͸���ȡ���ת����. 
 *  @param[in] pviewwind �Ӵ����.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤������һ���ԣ���Ҫ���ź��������б���.
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
 *  @brief ������Ⱦ�Ӵ��Ĳ���.
 *  @param[in] view_window ��Ⱦ�Ӵ��ľ��.
 *  @param[in] scene ��Ⱦ�����ľ��.
 *  @param[in] llx �Ӵ������½Ǻ��������ʼλ��(��λ:����,�ⲿ��Ļ����ϵ,�������ת�����Ļ���Ͻ�).
 *  @param[in] lly �Ӵ������½����������ʼλ��(��λ:����,�ⲿ��Ļ����ϵ,�������ת�����Ļ���Ͻ�).
 *  @param[in] width �Ӵ���ȣ�����,�ⲿ��Ļ����ϵ��.
 *  @param[in] height �Ӵ��߶ȣ�����,�ⲿ��Ļ����ϵ��.
 *  @exception void
 *  @return f_int32_t.
 *  @retval 0 �ɹ�.
 *  @retval -1 �Ӵ����Ϊ��.
 *  @retval -2 ��ʼλ�ò��ڷ�Χ��.
 *  @retval -3 ��ȡ��߶Ȳ��ڷ�Χ��.
 *  @retval -4 �ⲿ��Ļ����ת�����ڲ���Ļ�������.
 * @see 
 * @note
*/
f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
{
	sViewWindow *pviewwind = NULL;
	sGLRENDERSCENE *pscene = NULL;
	/*�ⲿ��Ļ����ϵ(��ת��)��,��Ļ�Ŀ�Ⱥ͸߶�*/
	f_int32_t screen_width_outerscreen = 0, screen_height_outerscreen = 0;
	/*�ڲ���Ļ����ϵ(�����ڷ�)��,�ӿڵĿ�Ⱥ͸߶�*/
	f_int32_t inner_vport_width_innerscreen = 0, inner_vport_height_innerscreen = 0;
	/*�ⲿ��Ļ����ϵ�£��ڲ��ӿ�ԭ��(���½�)���ⲿ��Ļ������Ӧ�ĵ�*/
	PT_2I inner_vport_lb_outerscreen;
	/*�ڲ���Ļ����ϵ�£��ڲ��ӿ�ԭ��(�õ���ȷ����,��Ļ�����ڷ�ʱ�����½�)������*/
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



	/*�����ⲿ�ӿڵ���ʼλ��(���½�)���꼰��ȡ��߶ȣ������ⲿ�ӿ�����ϵ*/
	pviewwind->viewwd_param_input.outer_view_port.llx = llx;
	pviewwind->viewwd_param_input.outer_view_port.lly = screen_height_outerscreen - lly;
	pviewwind->viewwd_param_input.outer_view_port.width = width;
	pviewwind->viewwd_param_input.outer_view_port.height = height;

	/*�����ڲ��ӿ�ԭ�����ڲ���Ļ����ϵ������*/
	if(outerScreenPt2innerScreenPt(inner_vport_lb_outerscreen, &inner_vport_lb_innerscreen))
	{
		/*�����ڲ��ӿڵ���ʼλ��(���½�)���꼰��ȡ��߶ȣ������ڲ��ӿ�����ϵ*/
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
	
	/*�ж��ڲ��ӿ�ԭ�����ڲ���Ļ����ϵ�������Ƿ񳬳��ڲ���Ļ��Χ*/
	if( inner_vport_lb_innerscreen.x < 0 || inner_vport_lb_innerscreen.x > window_size.width || 
		inner_vport_lb_innerscreen.y < 0 || inner_vport_lb_innerscreen.y > window_size.height)
	{
	    DEBUG_PRINT("lower-left corner of the view window is out of range!");
	    return(-2);
	}
	
	/*�ж��ڲ��ӿ����ڲ���Ļ����ϵ�¿�Ⱥͳ����Ƿ񳬳��ڲ���Ļ��Χ*/
	if( (inner_vport_width_innerscreen <= 0) || 
		(inner_vport_width_innerscreen > (window_size.width - inner_vport_lb_innerscreen.x) ) ||
		(inner_vport_height_innerscreen <= 0) || 
		(inner_vport_height_innerscreen > inner_vport_lb_innerscreen.y) )
	{
	    DEBUG_PRINT("width-height of the view window is out of range!");
	    return(-3);
	}

	/*�����ⲿ�ӿڵ���ʼλ��(���½�)���꼰��ȡ��߶ȣ������ⲿ�ӿ�����ϵ*/
	/*�����ڲ��ӿڵ���ʼλ��(���½�)���꼰��ȡ��߶ȣ������ڲ��ӿ�����ϵ*/
    updateViewPortParam(pviewwind, pscene);

	return(0);
}

/**
 * @fn setViewWndParam(sViewWindow *pviewwind)
 *  @brief ������Ⱦ�Ӵ����йز����������ߴ硢���̻��뾶��͸���ȡ���ת����. 
 *  @param[in] pviewwind �Ӵ����.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤������һ���ԣ���Ҫ���ź��������б���.
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
 *  @brief �����ⲿ����Ĳ���.
 *  @param[in] pviewwind  �Ӵ����.
 *  @param[in] pScene �������.
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
	
	/* �����Ӵ����� */
	setViewWndParam(pviewwind);

	/* �����ⲿ�ӿں��ڲ��ӿڲ��� */
	//updateViewPortParam(pviewwind, pScene); 

	/* �����¼�����,����������Ʋ�����ģʽ���Ʋ���������������Ϣ���� */ 
	setEventParam(pScene);  

	/* �������ڲ���,�����в���,����״̬�²��ڴ˴����·ɻ�λ����Ϣ */ 
	setPeriodParam(pScene); 

	/* �����ӵ�������,���ɻ��߶���ظ߶�С��50mʱ��ǿ�����÷ɻ��߶�Ϊ����߶�+50m */
	/* ������������д����ж�,�����ӽ��Ƿ���Ҫ�����жϸ����ͺ�Ҫ��,û���ر�Ҫ��ͱ��� */
	if(pScene->camctrl_param.view_type != eVM_OVERLOOK_VIEW)
	{
		/* ��ȡ��ǰ��γ������Ƭ�еĸ߶�,��Ƭ������,��ʵ�߶Ȼ��нϴ���� */
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

	/* 2016/06/09 ��Ǫ��
	��֪Ϊʲô��������ĸ߶���8000,5000,2500,2000,1500ʱ��������ı�߶ȣ����������
	�ı�߶Ⱥ󣬻���Ͳ����ˣ��Ҹı��ֵ����С��0.4
	������δ����,��ʱȥ��,����߶���ʾ����ȷ
	*/
	//if(fabs(fmod(pScene->geopt_pos.height, 10.0)) < PRECISION )  
	//    pScene->geopt_pos.height += 1.4;
	    
	/* ��������ת������������ */
	{
	    if(!geoDPt2objDPt( &(pScene->geopt_pos), &(pScene->objpt_pos)) )
		{
	        pScene->objpt_pos.x = 0.0;
	        pScene->objpt_pos.y = 0.0;
	        pScene->objpt_pos.z = 0.0;
		}	
	}

	//���¶�ά����ķ��в����Ϳ��Ʋ���
	setVecMapParam(pScene);

}

/**
 * @fn void drawFuncPre(void)
 *  @brief ���ڱ���ģ�Ӿ����ͶӰ����,��Ϊ����ͼ�λ����趨ͶӰ��ʽ�͵�λ��.
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
 *  @brief �ָ�֮ǰ��ģ�Ӿ����ͶӰ����.
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
 *  @brief ��ȡ��ת���ĵ�OpenGL����.
 *  @param[in] pScene  �������.
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
	
	/* ����Բ�ĵ��ӿ����� */
	x = startx + width * 0.5f;    
	y = starty + height * 0.5f;
	/* ����뾶(��λ������) */
	radiu = height * 0.5f - 10.0f;
	
	left   = startx;
	right  = startx + width;
	bottom = starty;
	top    = starty + height;
	
	/* ʹ��ģ�建�����ķ�������һ��Բ */
	/* ��ʼ��initGLSetting������������ */
//	glClearStencil(0);
//	glEnableEx(GL_STENCIL_TEST);	
//	glClear(GL_STENCIL_BUFFER_BIT);

	/* ��ģ�建�����е�ֵ�������3���������,��������2���бȽ�,�ȽϺ���Ϊ����1. GL_NEVER��ʾ��Զ��ͨ������ */
	glStencilFunc(GL_NEVER, 0x1, 0x1);
	/* ���ݱȽϽ������������޸�ģ�建�����е�ֵ,����1��ʾģ����Բ�ͨ��ʱִ�еĲ���, */
	/* ����2��ʾģ�����ͨ������Ȳ���ʧ��ʱִ�еĲ���, ����3��ʾģ�����ͨ������Ȳ���Ҳͨ��ʱִ�еĲ��� */
	/* GL_REPLACE��ʾ��glStencilFunc�Ĳ���2�滻ģ�建�����е�ֵ */
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);	
	
	/* ���ӿڷ�Χ�ڻ���һ��Բ��Բ�ڵ�ģ�建������ֵΪ1��Բ���ģ�建������ֵΪ0 */
	glViewport(startx, starty, width, height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glOrtho(left,right,bottom, top, 1, -1);

	/* �������������һ��Բ */
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
	/* GL_EQUAL��ʾģ�建�����е�ֵ�����2��ȣ���ͨ������ */
	glStencilFunc(GL_EQUAL, 0x1, 0x1);
	/* GL_KEEP��ʾ���ֵ�ǰģ�建�����е�ֵ */
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);	
	
	/* ��ֹд����Ȼ����� */
	glDepthMask(GL_FALSE);
	/* ����Բ����Ӿ��� */
	glRectf(x-radiu, y+radiu, x+radiu, y-radiu);
	/* ����д����Ȼ����� */
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
    
    /* ������ʱ���̶̹���ת������ģʽʱ�����溽���ת�� */
    /* ע�����̵���ת�ǵ���glRotatef��z����ת���������,ֱ���ú���Ǽ���,��pScene->attitude.yaw�෴*/
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

	/* ������ڸ����µ�����ģʽ�һ�δ���Σ�Ӧ��������λ�ü���������꣬�������ɻ�λ�ã�����ʱ�Ͳ��ý��յ��ķɻ���γ���� */
	if((eROM_inroam == pScene->mdctrl_cmd.rom2d_mode) 
		&&(pScene->is_romed == eROM_notroamed))
	{
		ret = getGeoByOuterViewPt(pScene, pScene->rom_pt_input, &geoPt);
		if(ret)		
		{
			/* ע��:��һ֡�Ż���� */
			pScene->geopt_pos.lon  = geoPt.lon;
			pScene->geopt_pos.lat  = geoPt.lat;
			/*Ϊ�˱�֤�����¸߶�Ԥ����������,�˴������¸߶�*/
			//pScene->geopt_pos.height  = geoPt.height;
			//getViewPtByGeo(pScene,geoPt, &viewPt);
		}
		else
		{
			;//���ı�λ��	
		}

		/* ����״̬Ϊ������ */
		pScene->is_romed = eROM_roamed;
	}	

	/* ������ڸ����µ�����ģʽ�һ�δ���ι�λ��Ӧ���ݽ�֮ǰ��¼�����ν���ʱ�ĳ�ʼλ�ø����ɻ�λ�ã�ֱ�ӷ��أ����ٴ���ǰ֮֡������β��� */
	if((eROM_inroam == pScene->mdctrl_cmd.rom2d_mode) 
		&&(pScene->is_rom_homed == eROM_notroamhomed))
	{
		/* ע��:��һ֡�Ż���� */
		pScene->geopt_pos  = pScene->roam_start_pt_pos;
		/* ����״̬Ϊ�����ι�λ */
		pScene->is_rom_homed = eROM_roamhomed;
	}
}


/**
 * @fn void mapSceneRender(VOIDPtr render_wnd, VOIDPtr render_scene)
 *  @brief ��ͼ������Ⱦ.
 *  @param[in] render_viewwnd  �Ӵ����.
 *  @param[in] render_scene �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void mapSceneRender(VOIDPtr render_viewwnd, VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
	sViewWindow *pViewWnd = NULL;

	/* �ж��������Ⱦ��������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;	    
	
	/* �ж�����Ļ����Ӵ�����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	pViewWnd = viewWindowPtrValid(render_viewwnd);
	if(NULL == pViewWnd)
	    return;	 
	
	/* �ⶳ״̬�²Ÿ��������ڲ���������� */
	if(eFRZ_outfreeze == pScene->mdctrl_cmd_input.frz_mode)    
	    updateInputParam(pViewWnd, pScene);    
		    
	glViewport(pViewWnd->viewwd_param.inner_view_port.llx, pViewWnd->viewwd_param.inner_view_port.lly,
	           pViewWnd->viewwd_param.inner_view_port.width, pViewWnd->viewwd_param.inner_view_port.height);

	/* ������ת���ĵ�OpenGL����,ֻ���ڸ���ģʽ��,������ת���Ĳ���Ч������ģʽĬ��Ϊ(0,0) */ 
	if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
	{           
	    getRotateOpenglPt(pScene); 
    }
    else
    {
    	pScene->rotate_opengl_pt.x = 0.0;
	    pScene->rotate_opengl_pt.y = 0.0;
    }	                                	        
	
	/* ������ɫģʽ,����������ɫ�������е�Gͨ����alphaͨ��,Ч������ֻ�ܿ�����ɫ��͸���� */
	if(eCOR_green == pScene->mdctrl_cmd.cor_mode) 
	    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);

#if 1
	{
		/* �����ü�����,�������ƻ������� */
		glEnableEx(GL_SCISSOR_TEST);
		/* ���òü�����,�����½�Ϊ����ԭ�� */
		glScissor(pViewWnd->viewwd_param.inner_view_port.llx, pViewWnd->viewwd_param.inner_view_port.lly,
	           pViewWnd->viewwd_param.inner_view_port.width, pViewWnd->viewwd_param.inner_view_port.height);
		/* ����ü�������Ȼ�����,���ֵĬ��Ϊ1.0f */
 		glClear(GL_DEPTH_BUFFER_BIT);
		/* �رղü����� */
		glDisableEx(GL_SCISSOR_TEST);	
	}
#endif
	
	/* �Ȼ���һ��������ɫ,����ʾ�б�ķ�ʽ,����һ���ӿڴ�С�ľ��� */
	drawFuncPre();
	drawVPortBkground(pViewWnd->viewwd_param.bkcolor.red, pViewWnd->viewwd_param.bkcolor.green, pViewWnd->viewwd_param.bkcolor.blue, 1.0f); 
	drawFuncPro(); 	   

	// ����״̬
	if(eROM_inroam == pScene->mdctrl_cmd.rom_mode)
	{
		double roamPosHeight;
		GetPtEye(&pScene->roam, &pScene->objpt_pos);
		pScene->camParam.m_ptEye = pScene->objpt_pos;
		objDPt2geoDPt(&(pScene->camParam.m_ptEye), &(pScene->camParam.m_geoptEye));

		/*�ж��Ƿ񴩵أ����������������*/
		/* ��ȡ��ǰ��γ������Ƭ�еĸ߶�,��Ƭ������,��ʵ�߶Ȼ��нϴ���� */
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

			//�������ӽ��µ�qtn ʹ�õķֱ��ʽ�����Сֵ���ƣ�
			//Ŀ���Ǳ�֤������������ʱ����ע�ǲ����������̹�С�����١�
			if(measure < 100000.0)
				measure = 100000.0;

			pScene->roam_res_for_QTN = measure / pScene->camctrl_param.compass_radius;			
		}
	}
	else
	{
		//�����½�������
	    if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
	    {
			pScene->roam_res_for_QTN = pScene->roam_res = pScene->camctrl_param.view_res;
		}
		else
		{
			//�����ӽ��²���100 km
			pScene->roam_res_for_QTN = pScene->roam_res = 100000.0 / pScene->camctrl_param.compass_radius;
		}

		/* ����ͶӰ��,���ݲ�ͬ�ӽǼ���������� */
		if(ePRJ_ball == pScene->project_mode)
		{	
			/*�����ӵ�ľֲ����굽�����������*/
			// �����ӽ��£�������״̬��Ĭ�ϵ��ӵ�Ϊ�ɻ����ھ�γ���£����θ߶�Ϊ0��һ�㣻
			//             ����״̬���ӵ�Ϊ��Ļ�ϴ�������һ��Ķ�Ӧ��γ���£����θ߶�Ϊ0��һ�㡣
			// ��Ӧ�û��ġ����̡�����ֵ���̶Ȼ��뾶��Ӧ�ľ��� �� �̶Ȼ��뾶�������� ֮�ȣ��������ÿ���ض�Ӧ��ʵ�ʾ��룬
			// �ٸ����ӳ��ǣ�������ӵ�ĺ��θ߶�
			// �����ӽ��£������Ʒɻ�	
			if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
			{
				f_float64_t hgt, angle;
				Geo_Pt_D geopt;
				Obj_Pt_D objpt;
				geopt.lon = pScene->geopt_pos.lon;
				geopt.lat = pScene->geopt_pos.lat;
				geopt.height = 0.0;
				geoDPt2objDPt(&geopt, &objpt);	// �ɻ���γ��ת��������
				// 1.�����ӵ��ڵ����ϵ��������(ͨ���þ���ɽ��������ϵ�µľֲ�����任��������)
				calcWorldMatrix44(objpt.x, objpt.y, objpt.z, &(pScene->m_mxWorld));
				// 2.�����̵�������ӵ㺣�θ߶�
				angle = pScene->camctrl_param.view_res * pScene->camctrl_param.compass_radius / EARTH_RADIUS;
				hgt = (/*pScene->viewport[3]*/window_size.height * 0.5 * EARTH_RADIUS * sin(angle)) / 
					  (pScene->camctrl_param.compass_radius * tan((pScene->camctrl_param.view_angle * 0.5) * DE2RA)) 
					  + EARTH_RADIUS * cos(angle) - EARTH_RADIUS;
				// 3.�����ӵ�ֲ�����(������ӵ����ĵ�)(̧�������̵���Ӧ�߿�λ��)
				vector3DMemSet(&(pScene->camParam.m_ptEye), 0.0, 0.0, hgt);
			}
			else
			{
				// �Ǹ����ӽ��£�������״̬��Ĭ�ϵ��ӵ�Ϊ�ɻ����ھ�γ�ߣ������ӵ�ľֲ����굽�����������
				calcWorldMatrix44(pScene->objpt_pos.x, pScene->objpt_pos.y, pScene->objpt_pos.z, &(pScene->m_mxWorld) );
			}

			//lpf add temp 2018��8��28��15:26:15 for 45��\75��
			if(pScene->camctrl_param.view_type == eVM_SCALE_VIEW)
			{
				calTruepos(pScene, (LP_PT_3D)(&(pScene->camParam.m_ptEye)), (LP_Geo_Pt_D)(&(pScene->camParam.m_geoptEye)));
				calcWorldMatrix44(pScene->camParam.m_ptEye.x, 
					pScene->camParam.m_ptEye.y, 
					pScene->camParam.m_ptEye.z, &(pScene->m_mxWorld) );
			}
			
			/*�����ӽ�����,�ɻ�����̬,�ӵ�ľֲ����굽�����������,�����������(4������)����̬��ת����(m_mxRotate)*/
			setCameraStyle((f_uint8_t)(pScene->camctrl_param.view_type),
						   (LP_PT_3D)(&(pScene->camParam.m_ptEye)),		// �ӵ���Ե����λ��
						   (LP_PT_3D)(&(pScene->camParam.m_ptCenter)),	// ���ĵ�(�����)λ�� 
						   (LP_PT_3D)(&(pScene->camParam.m_ptUp)),		// Y��
						   (LP_PT_3D)(&(pScene->camParam.m_ptTo)), 
						   pScene->attitude.pitch, pScene->attitude.yaw, pScene->attitude.roll,
						   (LPMatrix44)(&(pScene->m_mxRotate)), (LPMatrix44)(&(pScene->m_mxWorld)) );	    

			/*���ӵ����������ת���ɾ�γ������*/
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
		/* ����ͶӰ��,���������Զ���ý���,��������ת,���ò��洢ģʽ�����ͶӰ���� */
		updateCameraParamDIY(pScene);	
	}
	else
	{
		/* ī����ͶӰ��,ֻ�и����ӽ�,�����������,���ò��洢ģʽ�����ͶӰ����,����ʵ����vmapApp.c�� */
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
	{	/* ÿһ֡��Ҫ�򿪹��գ��ҿ�������Ҫ��������ͶӰ����֮�󣬱�֤Ƕ��ʽ�¹���������������2017-4-28 */
		EnableLight(FALSE);
	}

	EnableLight0(TRUE);

	/* ����Զ���ý��� */
	pScene->camctrl_param.view_near = pScene->camParam.m_lfNear;
	pScene->camctrl_param.view_far = pScene->camParam.m_lfFar;

	/* ����ģʽ�����ͶӰ����ĳ˻���������� */
	calCurMatrix(pScene);
	
	/* �����Ӿ������ */
	CalculateFrustum(&(pScene->frustum), pScene);

	/* ��ά������Ⱦ */         		
	if((NULL != pScene->func3dMap) )
	{   
		if(ePRJ_ball == pScene->project_mode)
		{
			//drawFuncPre();	           
			map3dRenderFunc(pScene);
			//drawFuncPro();
		}
	}

	/* �㸩�����������СΪ8192*64������7200֧�ֵ��������ߴ磬��7200�²������㸩������ */
#ifndef _JM7200_
    /* �㸩��������ʾ���ش򿪲Ż��ƣ�����ģʽ�����ƣ�,�㸩�������볡����ģʽ������أ������ڳ����ĵ�ǰ�����л���*/
    if(pScene->auxnavinfo_param.zpl_param.is_display) 
    {
		/* ����㸩����������δ���ɣ����������㸩���������� */
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
	    /* �ɻ�ģ�ͼ��ɻ��״�ģ�͵������Ƕ����������¿��ǣ��о�Ŀǰ�������ǲ��Եġ��������Ľ�2017-3-8 , ��Ǫ��*/
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

	/* ��ά������Ⱦ,�����ӽ����Ƕ�άģʽ */
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
				//�ر�������ʾ
				setFontVisable(0);
			}
		}
		else
		{
			if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
			{
				//��������������ʸ���ߡ���
				setVecLineAreaShowValue(1,pScene->scene_index);
				//�������������Ƶ�
				setViewXYZType(pScene->geopt_pos.lon, pScene->geopt_pos.lat, 
					pScene->geopt_pos.height, FALSE);
			}
			else if (eVM_SCALE_VIEW == pScene->camctrl_param.view_type)
			{
				//45��/75���ӽ��²�����ʸ���ߡ���
				setVecLineAreaShowValue(0,pScene->scene_index);
				//45��/75���ӽ��¿��Ƶ�Ļ��ƣ����ӵ�λ�þ������1500000�Ž��л���
				setViewXYZType(pScene->geopt_pos.lon, pScene->geopt_pos.lat, 
					pScene->geopt_pos.height, TRUE);
			}
		}

	    map2dRenderFunc(pScene);
	    //drawFuncPro();

		//����ttfDrawString��������������Ϣ
		setFontVisable(1);
	}

	
//	glDisableEx(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,0,0x01);//����glDisableEx��
	
#if 1	
	drawFuncPre();   
	//����������ʾʱ�Ż�������
	if((pScene->auxnavinfo_param.cps_param.is_display) && 
	   (eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type) )
	{
	    renderCps(pScene);
	}
	drawFuncPro();
#endif   
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

#ifdef USE_MODEL
	//����ģ��
	if(1 == pScene->mdctrl_cmd.is_3dplane_display)
	{
		RenderModel(FALSE, pScene);
	}
#endif

	//���Ƹ߳�����ͼ
	if(TRUE == pScene->auxnavinfo_param.prefile_param.is_display)
	{	
		RenderTerrainProfile(pScene);
	}

	if(TRUE == pScene->termap_param_input.isShow) 
	{
		RenderTerrainMap(pScene);
	}

	//����������ʾʱ�Ż��Ʒɻ�
	if((pScene->mdctrl_cmd.plane_symbol_display == 1) && 
		(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type) )
	{
		drawPlaneSymbol(pScene);
	}
	
	/* JM7200���뱳��ɫʹ��ͬһ����ʾ�б�������⣬�˴�����������ͬ����ʾ�б���ʵ�ֱ������͸����Ļ��� */
	/* ����ڻ������ϲ����һ�����ӿڵ�ͬ��С�ĺ�ɫ�ľ���,ͨ��͸�����������ӿڻ��������,alphaԽС����Խ�� */
    if( (eScene_normal == pScene->mdctrl_cmd.scene_mode) && (pViewWnd->viewwd_param.alpha > 0.0f) )	
    {
    	drawFuncPre();
	    glViewport(pViewWnd->viewwd_param.inner_view_port.llx, pViewWnd->viewwd_param.inner_view_port.lly,
	               pViewWnd->viewwd_param.inner_view_port.width, pViewWnd->viewwd_param.inner_view_port.height); 
#if 0	           
	    /* �ӿڱ߿� */
	    glColor3f(0.0f, 0.0f, 0.0f);
	    glLineWidth(2.0f);
	    glBegin(GL_LINE_LOOP);
	        glVertex2f(-1.0f, -1.0f);
	        glVertex2f(1.0f, -1.0f);
	        glVertex2f(1.0f, 1.0f);
	        glVertex2f(-1.0f, 1.0f);
	    glEnd();
#endif 	    
	    /* ����͸���� */
	    drawVPortBrightness(0.0f, 0.0f, 0.0f, pViewWnd->viewwd_param.alpha); 	
	    drawFuncPro();
    } 
	
	/* �����û�ע����û�ͼ����ƺ��� */   
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

	/*�Ƕ���ʱ���ܽ�������*/
	if(eFRZ_outfreeze == pScene->mdctrl_cmd_input.frz_mode) 
	{
		/* ���¸����ӽ��µ����β���,ʵ�ָ����ӽ��µ�ͼ�����ι��� */
		updataRoamParam(pScene);
	}


	/* ���·Ǹ����ӽ��µ����β���,ʵ�ַǸ����ӽ��µ�ͼ�����ι��� */
	StartRoam(&pScene->roam, 
		pScene->mdctrl_cmd.rom_mode, 
		pScene->m_lfModelMatrix, 
		pScene->innerviewport,
		pScene->attitude_input.yaw,
		pScene);

	
	///* ��ջ��������е�������Ƭ�ڵ� */
	//drawListDeleteNode(pScene);		
}

// ������׵��Զ���ý���
static void calcCamFarNearPlane(sGLRENDERSCENE *pScene)
{
	PT_3D ptTemp;
	f_float64_t lfNear,lfFar, hei;	// Զ���ý���|�ɻ���
	f_float64_t angle_theta; //����-�ӵ� �� �ر��е�-�ӵ� �������ߵļн�
	
	if(NULL == pScene)       
        return;
    
	// �����ӵ㵽Ŀ������
    vector3DSub(&ptTemp, &pScene->camParam.m_ptEye, &pScene->camParam.m_ptCenter);
	pScene->camParam.m_lfDistance = vector3DLength(&ptTemp);
	hei = pScene->camParam.m_geoptEye.height;	// �ɻ����θ�
	lfNear = 10;    
	if(hei < 0) //ֱ����else�����ʽ����hei<0ʱ���ܶԸ��������������������-1.#IND0000000������
		lfFar = 100000.0;
	else
	{
		angle_theta = asin(EARTH_RADIUS/(EARTH_RADIUS+hei));
		lfFar = (hei + EARTH_RADIUS)* cos(angle_theta);
	}		

    if(eROM_outroam == pScene->mdctrl_cmd_input.rom_mode)/* ������״̬��,����ʱ,���ܲ�������ķ�������Զ�ý���,Ӧ��͸��ʱ��Զ�ý���,�����¶�� */
    {
		// ����Զ�ý���
	    if (eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
	    {
			/*
			// Ϊ���ڸ���״̬�¸���ȷ�����Ӿ����жϣ��������������Ƭ���룬�������µ�Զ�ý��������������Ӿ����Բ׶��ģ�͸պ�����򽻵ľ��봦��
			// ������Ҫ��Ϊ�˸��� ��ֱ�ӳ���g_Fovy �� ��Ļ�ߴ�(��������) ������Ӿ�������ģΪһ��׶��ʱ��׶������İ붥��theta
			// ��lΪ���ӵ��������Ļ�ľ��룬����
			// ((Top - Bottom)/2) / l = tan(Fovy/2)
			// sqrt( ((Top - Bottom)/2)^2 + ((Right - Left)/2)^2 ) / l = tan(theta)
			// ����ϲ��� => sqrt( (Top - Bottom)^2 + (Right - Left)^2 ) * tan(Fovy/2) / (Top - Bottom) = tan(theta)

			// �ֶ��ڵ�����˵�����ӵ�߶�Ϊh, ����뾶ΪR���������Ķ�Ӧ�İ�Ƕ�Ϊalpha�������Ҷ���
			// (h + R) / sin[pi - (theta + alpha)] = R / sin(theta)
			// ����Զ�ý������Ϊ
			// lfFar = h + R - R*cos(alpha)
			*/
			f_float64_t theta, alpha;
			theta = atan( sqrt( window_size.width * window_size.width + window_size.height * window_size.height ) 
						  * tan(pScene->camctrl_param.view_angle * 0.5 * DE2RA) / window_size.height );
			alpha = atan( (hei / EARTH_RADIUS + 1.0 - sin(theta)) / cos(theta) );
			lfFar = hei + EARTH_RADIUS - EARTH_RADIUS * cos(alpha);	
	    }

		//���ý������
		if( eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type )	// �����ӽ�
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
		//���������
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
	{   /* ��Ļ��ת */
	    default:
	    case eROTATE_CW_0:
	        /* Ĭ��ģʽΪ����ģʽ����Ļ����ת */
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

//�ֶ�����ͶӰ����ģʽ�����ӿھ��󣬷�������lpf add 2017��11��13��14:15:46
/*
 *	���������ľ�����ʽ
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
	// ��Ϊup

	// ����
	vector3DSub(&n, ptCEye, ptCCenter);
	vector3DNormalize(&n);			//n
	vector3DCrossProduct(&u, ptCUp, &n);
	vector3DNormalize(&u);			//u
	vector3DCrossProduct(&v, &n, &u);//v
	vector3DNormalize(&v);

	// ƽ�ƾ���: �������������ϵƽ�����ӵ�����ϵ
	dMatrix44MemSet(&T, 
		1, 0, 0, -ptCEye->x,
		0, 1, 0, -ptCEye->y,
		0, 0, 1, -ptCEye->z,
		0, 0, 0, 1);
	// ��ת����: ���������������ת���ӵ�����ϵ
	dMatrix44MemSet(&R, u.x, u.y, u.z, 0,
		v.x, v.y, v.z, 0,
		n.x, n.y, n.z, 0,
		0, 0, 0, 1);
	Maxtrix4x4_Mul(lpMatrix4x4, &R, &T);
	dMaxtrix44_Inversion(lpMatrix4x4);
	return;
}
// ����ͶӰ����(theta:ƽ��ͷ�ӽ�(����ӽǵ��Ž�);aspect:ͶӰ��Ļ��߱�)
static void GetProjectMatrix(LPMatrix44 lpMatrix44, sGLRENDERSCENE *pScene, double theta, double aspect, double znear, double zfar)
{
	double half_theta = theta / 2;
	double cot_v = 1.0 / tan(half_theta * PI / 180);
	double viewCenter_passX = pScene->rotate_opengl_pt.x;	// �ӵ���Ļ����
	double viewCenter_passY = pScene->rotate_opengl_pt.y;
	double scale_xyz[3] = {window_size.width * 1.0f / pScene->innerviewport[2], window_size.height * 1.0f / pScene->innerviewport[3], 1.0f};	// �ӿڵ���������ű�
	
	// ����ͶӰ����(��xyz�任���淶������ϵ[-1,1])
	dMatrix44MemSet(lpMatrix44, 
		scale_xyz[0]*cot_v / aspect, 0,	-viewCenter_passX, 0,	// X
		0, scale_xyz[1]*cot_v,		    -viewCenter_passY,	0,	// Y
		0, 0, scale_xyz[2]*(znear + zfar) / (znear - zfar), scale_xyz[2]*2 * znear * zfar / (znear - zfar),	// Z
		0, 0, -1, 0);

	//��������	
	dMaxtrix44_Inversion(lpMatrix44);
	return;
}
// ����Զ���ý���
static void updateCameraParamDIY(sGLRENDERSCENE *pScene)
{
    if(NULL == pScene)       
        return;
    
	// ���������Զ���ý���
    calcCamFarNearPlane(pScene);        

	// �������ͶӰ����
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

	// ������Ļ��ת,�������ģʽ���󲢴洢
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		switch(screen_param.rotate_type)
		{   /* ��Ļ��ת */
			default:
			case eROTATE_CW_0:
				/* Ĭ��ģʽΪ����ģʽ����Ļ����ת */
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


/* ��ȡ�ü���������󡢻�ȡ�ӿڲ��� */
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
��һԪ���η���ax*x+bx+c=0�Ľ�
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

// C = A*sin(alpha) + B*cos(alpha) ����Ƕ�ֵ������ֵ
void CalSinA(double A, double B,double C, double * alpha)
{
	double s1 = 0.0, s2 = 0.0;
	BOOL flag = FALSE;
	double calA = B*B + A*A;
	double calB = -2.0 * C*B;
	double calC = C*C - A*A;

	// ���ȼ���cos()
	CalOneParamDoubleTime(calA, calB, calC, &s1, &s2, &flag);

	if(flag == FALSE)
	{
		return;
	}
	
	// Ȼ�����Ƕ�ֵ
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
//�����ӵ�߶ȷ������̻��뾶
//	angle = pScene->camctrl_param.view_res * pScene->camctrl_param.compass_radius / EARTH_RADIUS;
//	hgt = (/*pScene->viewport[3]*/window_size.height * 0.5 * EARTH_RADIUS * sin(angle)) / 
//	      (pScene->camctrl_param.compass_radius * tan((pScene->camctrl_param.view_angle * 0.5) * DE2RA)) 
//	      + EARTH_RADIUS * cos(angle) - EARTH_RADIUS;
// C = A*sin() + B*cos()

double getScaleByEyeHeight(double height, double fovy, double vpHeight, double radius)
{
double tan_fovy ,tan_beta,beta,sin_beta,maxDis,dis,sin_alpha,alpha,theta,arc_len,scale;

	if(height < 1.0) return 1.0;   //�ںܵͣ�����������Ϊ1

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

	//�����Ǵ���-80ʱ�����̲���800 000
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

		//lpf delete 2017��11��10��8:56:42 , 
		//�˴�������ʾ��������pitch С��45��ʱ��������ʸ��������������ά�ĵ���
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
 *  @brief 3d��ͼ������Ⱦ.
 *  @param[in] scene  �������.
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

	/* ����Ȳ��� */
	glEnableEx(GL_DEPTH_TEST);

#ifdef WIN32
	//glEnableEx(GL_DEPTH_CLAMP);	//��ֹ�ӵ㴩��
#endif

	/* �����Ҫ������ά;�����ƴ���άʸ��(��ɫΪ��ɫ),���������ά�����ӽǵĵ�ͼʱ,�ÿ��Ʊ���Ϊ1;��������ÿ��Ʊ�����Ϊ0 */
	if(pScene->mdctrl_cmd.close3d_mode == 0)	
	{
		f_float32_t pitch = 0.0f;

		/* �������ά����״̬,������Ϊ��������ĸ����� */
		if(eROM_inroam == pScene->mdctrl_cmd.rom_mode)
		{
			pitch = pScene->roam.thisPitch;
		}else{
			/* ����Ƿ���ά����״̬,��Ϊ�����ӽǣ�������ǿ��Ϊ-90��;��Ϊ�����ӽ�,��Ϊʵ�ʷɻ��ĸ����� */
			if(eVM_OVERLOOK_VIEW == pScene->camctrl_param.view_type)
			{
				pitch = -90.0f;
			}
			else
			{
				pitch = pScene->attitude.pitch;
			}
		} 
		
		/* ��ά������Ⱦ */
		qtmapRender(pMapDataSubTree, pScene->mdctrl_cmd.terwarn_mode, pitch, pScene); 
	}

	/* �ر���Ȳ��� */
	glDisableEx(GL_DEPTH_TEST);

	//glDisableEx(GL_CULL_FACE); 
}

/**
 * @fn void render2dMap(VOIDPtr *scene)
 *  @brief 2d��ͼ������Ⱦ.
 *  @param[in] scene  �������.
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
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* ��ɫģʽ */ 
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
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* ��ɫģʽ */ 
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
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* ��ɫģʽ */ 
        glColor3f(gray, gray, gray);
    else
        glColor3f(r, g, b);
	glBegin(GL_LINES);
	    glVertex2f(-0.5f, 0.5f);
	    glVertex2f(0.5f, -0.5f);
	glEnd();
	
	r = 0.0f;    g = 0.0f;    b = 1.0f;
    gray = 0.299 * r + 0.587 * g + 0.114 * b;
    if(eCOR_gray == pScene->mdctrl_cmd.cor_mode)/* ��ɫģʽ */ 
        glColor3f(gray, gray, gray);
    else
        glColor3f(r, g, b);
	glBegin(GL_LINES);
	    glVertex2f(-0.5f, 0.5f);
	    glVertex2f(0.5f, 0.5f);
	glEnd();
}

/*
 * ���ܣ��ж�ĳ����γ�ȵ��Ƿ�����Ƭ��Χ��
 * ���룺lfLon ����
 *       lfLat γ��
 * �������
 * ���أ��ڣ�����TRUE(1)�����ڣ�����FALSE(0)
 */
 BOOL InTile(const sQTMAPNODE  *pTile, const f_float64_t lon, const f_float64_t lat)
{
	return ((lon- pTile->hstart - pTile->hscale)*(lon-pTile->hstart)<=0 && (lat-pTile->vstart - pTile->vscale)*(lat-pTile->vstart )<=0);
}


/*
 * ���ܣ���ȡָ����γ�ȵ����Ѽ��ص��ڴ��е���Ƭ�еĸ߶�ֵ
 * ���룺pScene ��Ⱦ�������
 *       lfLon ����
 *       lfLat γ��
 * �����*pZ �߶�
 * ���أ���ȡ�ɹ�������TRUE(1)����ȡʧ�ܣ�����FALSE(0)
 */
BOOL GetZ(sGLRENDERSCENE *pScene, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ)
{
	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
	sMAPHANDLE * pHandle = pMapDataSubTree;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/* ѭ������LRU������Ľڵ� */
	pstListHead = &(pHandle->nodehead.stListHead);
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);

			if( pNode->level < iLevel)
				continue;

			/* �жϸþ�γ�ȵ��Ƿ�����Ƭ��Χ�� */
			if(InTile(pNode, lfLon, lfLat))
			{
				// linear z;
				double x,y,dx,dy;
				int i,j,k,l;

				/* ÿ����Ƭ����33*33���㣬�ҵ��þ�γ�ȵ�������Ƭ����4�����м� */
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
					/* ��4����ĸ߳�ֵ��������ƽ��,�õ�Ŀ���ĸ߶� */
					*pZ = pNode->fterrain[l]*(1-x)*(1-y) + pNode->fterrain[l+1]*x*(1-y) + 
						   pNode->fterrain[l+k]*(1-x)*y + pNode->fterrain[l+k+1]*x*y;
					/* �������ұȸýڵ�㼶����Ľڵ�,��ȡ��׼ȷ�ĸ߶� */
					iLevel = pNode->level + 1;
				}
			}
		}
	}	

	giveSem(pHandle->rendscenelock);


	/* ���ҵ��Ľڵ㲻Ϊ0,˵���߶�ֵ�鵽�� */
	if (iLevel != 0)
		return TRUE;
	else
		return FALSE;
}


/*
 * ���ܣ���ȡָ����γ�ȵ����Ѽ��ص��ڴ��е���Ƭ�еĸ߶�ֵ,��GetZ����һ��,ֻ�ǵ�һ�����������һ��
 * ���룺pHandle ��ͼ���������
 *       lfLon ����
 *       lfLat γ��
 * �����*pZ �߶�
 * ���أ���ȡ�ɹ�������TRUE(1)����ȡʧ�ܣ�����FALSE(0)
 */
BOOL GetZex(sMAPHANDLE * pHandle, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ)
{
	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/*����LRU��������û�д����صĽڵ�*/
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

//���߷����� p(t) = p0 + t*d0,����d0��p(t)��p0����ά����
//ƽ�淽����p*n = d
void Ray2TrangleIntersect(PT_3D n, double d, PT_3D p0, PT_3D d0, double *t)
{
	*t = (d - vector3DDotProduct(&p0, &n))/(vector3DDotProduct(&d0, &n));
 

}

//���Ҹ����ӽ��µ�ǰ�����������Ƭ�ж�Ӧ��γ�ȵĸ߶�ֵ
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

	/*����LRU��������û�д����صĽڵ�*/
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


					//�жϵ����ĸ�����������
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
						//����õ������������ڵ�λ��
						//*pZ = (d - normal[0] * lfLon - normal[1] * lfLat )/normal[2];



						//�������߷���					
						//���߷����� p(t) = p0 + t*d0,����d0��p(t)��p0����ά����
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

//���ҷǸ����ӽ��µ�ǰ�����������Ƭ�ж�Ӧ��γ�ȵĸ߶�ֵ
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

	/*����LRU��������û�д����صĽڵ�*/
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


					//�жϵ����ĸ�����������
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
						//����õ������������ڵ�λ��
						//*pZ = (d - normal[0] * lfLon - normal[1] * lfLat )/normal[2];



						//�������߷���					
						//���߷����� p(t) = p0 + t*d0,����d0��p(t)��p0����ά����
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

/*��tga����,ֻ��1��tga����,indexֻ��ȡ0*/
extern void textureUse(int index);
/*��tga����ȡ���ɻ����ŵ��������л���*/
/*
���룺
	x_center��y_center �ɻ���������λ��
	symbol_index ��������
	angle ��ת��
*/
extern void outPutPlaneSymbol(float x_center, float y_center, int symbol_index, float angle);

/*
���ܣ����Ʒɻ�����
���룺
	pScene �������
�����
    0-�ɹ�  -1-ʧ��
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

	//��ȡ�ɻ����Ż��ƵĽǶ�
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

	//��ȡ�ɻ������ڲ��ӿ�����	
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

	//����״̬���жϷɻ��Ƿ���߽磬����ᵼ������ʱ�ɻ����ų�����Ļ֮��ǿ��������
	//�������ĵ�״̬�²��жϷɻ��Ƿ���߽磬����ᵼ���������ĵ��������ŷɻ����ų�����Ļ֮��ǿ��������
	if((eROM_outroam == pScene->mdctrl_cmd.rom2d_mode) && (0 == pScene->set_center_pos))
	{
        //�ɻ������ڲ��ӿ����굽���ӿڵ�����߽磬���µ�ͼ���ĵ������Ϊ��ǰ�ɻ����ŵ�����,δ������ת
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
	
    /* ��ʼ����״���ϵ������� */
    glEnableEx(GL_TEXTURE_2D);
	textureUse(0);
	
	glEnableEx(GL_BLEND);	 
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glColor3f(0.0f, 0.78f, 0.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glColor3f(0.0f, 1.0f, 0.0f);
    //����ɻ�����,�߻���-16,ֱ����-23        
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
