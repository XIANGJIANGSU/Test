#include "../engine/memoryPool.h"
#include "../engine/mapRender.h"
#include "../engine/sqsfile.h"
#include "../projection/mct.h"
#include "../projection/lamBert.h"
#include "../engine/shadeMap.h"

#include "appHead.h"
#include "mapApp.h"
#include "initData.h"
#include "initScene.h"

static f_float32_t color_red = 0.13f;
static f_float32_t color_green = 0.34f;
static f_float32_t color_blue = 0.83f;
static f_float32_t color_alpha = 0.0f;

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32	
extern void vboFunInit(void);
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture = NULL;	
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f = NULL;
PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f = NULL;
#endif
#endif

/**
 * @fn void initGLSetting(void)
 *  @brief GL������ʼ��.
 *  @param[in] void. 
 *  @exception void 
 *  @return void
 * @see 
 * @note
*/
void initGLSetting(void)
{
	/*ָ��ͼԪ�и����������ɫ�Ĺ���ģʽ,GL_SMOOTH(Ĭ��ֵ)ƽ����ɫ,GL_FLAT�㶨��ɫ*/
	glShadeModel(GL_SMOOTH);
	/*ʹ����ͼԪ��ƽ����ɫ,�����߶γ��־��״*/
	glEnableEx(GL_LINE_SMOOTH);
	/*����������ɫ,����������������,glClear(GL_COLOR_BUFFER_BIT)���������������������������ã��˴�������*/
	//glClearColor(color_red, color_green, color_blue, color_alpha);
	/*������Ȼ������Ĳο�ֵΪ1.0,������������Ȼ������Ĳ���,glClear(GL_DEPTH_BUFFER_BIT)��������Ȼ���������*/
	glClearDepth(1.0f);	
	/*ʹ����Ȳ���*/
	glEnableEx(GL_DEPTH_TEST);	
	/* ָ����ȱȽϺ�����������ֵС�ڻ���ڲο�ֵ����ͨ��.������ʹ����Ȳ���*/
	glDepthFunc(GL_LEQUAL);	
	/* ָ����ɫ����������Ĳ�ֵ����,GL_NICEST��ʾ������*/
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	            
	/* ָ������ε����淽��,�������а�˳ʱ�뻹����ʱ��,Ĭ������ʱ��(GL_CCW),���ڱ������ε�����GL_CULL_FACE*/
	glFrontFace(GL_CCW);
	/* ����ģ����ԣ�Ĭ�ϲ�������ģ����Է�����͸���Ȳ���֮����Ȳ���֮ǰ*/
	glEnableEx(GL_STENCIL_TEST);
	/* ����ģ�建������д�����룺��Ϊfalse��0xff(Ĭ��ֵ)ʱ����д��*/
	glStencilMask(0xff);
	/* �������ģ�建������Ĭ��ֵΪ0*/
	glClearStencil(0);
	/* Ĭ��ֵGL_ALWAYS,0,0xff ģ�建�����е�ֵ�������3���������,��������2���бȽ�,�ȽϺ���Ϊ����1. GL_ALWAYS��ʾ��Զ��Ĭ��ֵ*/
	glStencilFunc(GL_ALWAYS,0,0x01);
	/* ���ģ�建����*/
	glClear(GL_STENCIL_BUFFER_BIT);
	/* �رջ��,alphaֵ����Ч*/
	glDisableEx(GL_BLEND);
	/* �رն�ά���� */
	glDisableEx(GL_TEXTURE_2D);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* �ر�һά���� */
	glDisableEx(GL_TEXTURE_1D);

#ifdef WIN32
	/* ��ȡOpenGL��չ�����ĺ���ָ�룬Ƕ��ʽ�������ȡ */
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
	glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
	glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)wglGetProcAddress("glMultiTexCoord1f");

	/* ��ȡOpenGL��չ�����ĺ���ָ�룬Ƕ��ʽ�������ȡ */                   
	sqsTextureFunInit();

	/* ��ȡOpenGL��չ�����ĺ���ָ�룬Ƕ��ʽ�������ȡ */ 
	vboFunInit();
#endif

#endif

	/* ������������һ������VecShadeģʽ��һ�����ڷ����� */
	InitVecShadedReliefMap();
}


/**
 * @fn f_int32_t projectMdInit(VOIDPtr scene, f_int32_t pmode)
 *  @brief ͶӰ��ʽ��ʼ��.
 *  @param[in] scene �������.
 *  @param[in] pmode ͶӰ��ʽ.
 *  @exception void
 *  @return f_int32_t,��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ʧ��.
 * @see 
 * @note 
*/
f_int32_t projectMdInit(VOIDPtr scene, f_int32_t pmode)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return(-1);
	    
	pScene->project_mode = pmode;
	return 0;
}

/**
 * @fn void mctParamInit(sMCTPARAM param)
 *  @brief ī���в�����ʼ��.
 *  @param[in] param ͶӰ����.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void mctParamInit(sMCTPARAM param)
{
	f_float64_t Factor;
	Set_Mercator_Parameters(param.a  , 
	                        param.f  , 
	                        param.lat_true_scale , 
	                        param.lon_cent       , 
	                        param.fale_east      , 
	                        param.false_north    , 
	                        &Factor);	
}

/**
 * @fn void lbtParamInit(sLBTPARAM param)
 *  @brief �����ز�����ʼ��.
 *  @param[in] param ͶӰ����.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void lbtParamInit(sLBTPARAM param)
{
	Set_Lambert_Parameters(param.a,
						   param.b,
						   param.lat_org,
						   param.lon_org,
						   param.lat_std_1,
						   param.lat_std_2,
						   param.false_east,
						   param.false_north);
}

/**
 * @fn void clearColorSet(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
 *  @brief ������������ɫ.
 *  @param[in] red,green,blue,alpha ��ɫ��rgba������0.0f~1.0f. 
 *  @exception void 
 *  @return void
 * @see 
 * @note �����������������������������ӿ���������ֻ����Ӧ�ò�����һ�ξͿ����ˣ�������ÿ������������
*/
void clearColorSet(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
{
	color_red = red;
	color_green = green;
	color_blue = blue;
	color_alpha = alpha;
	glClearColor(color_red, color_green, color_blue, color_alpha);
}

/**
 * @fn sPAINTUNIT * createUnit(void)
 *  @brief ����ͼ�λ��Ƶ�Ԫ���.
 *  @exception mallocError �������ʱ�ڴ�����ʧ��
 *  @return sPAINTUNIT *.
 *  @retval ͼ�λ��Ƶ�Ԫ���.
 *  @retval NULL    ʧ��.
 * @see 
 * @note
*/
sPAINTUNIT * createUnit(void)
{	
	sPAINTUNIT *unit;
	unit = (sPAINTUNIT *)NewFixedMemory(sizeof(sPAINTUNIT));
	if (unit == NULL)
		return(NULL);
	else
	    memset((VOIDPtr *)unit, 0, sizeof(sPAINTUNIT));

    return(unit);
}

/**
 * @fn f_int32_t createRenderViewWindow(VOIDPtr *pviewwind)
 *  @brief ����ͼ����Ⱦ�Ӵ�.
 *  @param[in] pviewwind �Ӵ����.
 *  @exception mallocError �������ʱ�ڴ�����ʧ��
 *  @return f_int32_t,�����ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 �Ӵ��������ʧ��.
 * @see 
 * @note
*/
f_int32_t createRenderViewWindow(VOIDPtr *pviewwind)
{
    sViewWindow *pWind;
    pWind = (sViewWindow *)NewFixedMemory(sizeof(sViewWindow));
    if (pWind == NULL)
		return(-1);
	else
	    memset((VOIDPtr *)pWind, 0, sizeof(sViewWindow));
	    
	pWind->viewwdparamset_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pWind->viewwdparamset_sem);

	pWind->viewwdparaminput_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pWind->viewwdparaminput_sem);

	*pviewwind = pWind;
    return 0;	
}

/**
 * @fn f_int32_t createRenderScene(VOIDPtr *render_scene)
 *  @brief �����������.
 *  @param[in] render_scene �������.
 *  @exception void
 *  @retval f_int32_t, �����ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1: ��Ⱦ�����������ʧ��.
 *  @retval -2: ��ͼ�����ʼ��ʧ��.
 * @see 
 * @note
*/
static int qtn_scene_i = 0;
f_int32_t createRenderScene(VOIDPtr *render_scene)
{
    sGLRENDERSCENE *pScene;
    f_int32_t ret = -1;
	pScene = (sGLRENDERSCENE *)NewFixedMemory(sizeof(sGLRENDERSCENE));
	if (pScene == NULL)
		return(-1);
	else
	    memset((VOIDPtr *)pScene, 0, sizeof(sGLRENDERSCENE));
	
	pScene->periodparaminput_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pScene->periodparaminput_sem);    
	pScene->periodparamset_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pScene->periodparamset_sem);
	
	pScene->eventparaminput_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pScene->eventparaminput_sem);
	pScene->eventparamset_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pScene->eventparamset_sem);

	pScene->terrainMap_param_used.eventparamset_sem = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pScene->terrainMap_param_used.eventparamset_sem);	
//	if(-1 == createMapHandle(&(pScene->map_handle)) )
//	{
//	    DeleteAlterableMemory((void *)pScene);
//		pScene = NULL;
//		return -2;
//	}


    {
    	stList_Head *pstListHead = NULL;
    	/**�����ʼ��**/
		pstListHead = &(pScene->scene_draw_list);
		LIST_INIT(pstListHead);
    }

	/**���ó���������,�ӿ�0ʹ��0,�ӿ�1ʹ��1,�ӿ�2ʹ��2**/
	pScene->scene_index = qtn_scene_i;

	/**������Ҫ���õĵ���qtn�����ID��,�ӿ�0ʹ��0,�ӿ�1ʹ��1,�ӿ�2ʹ��2**/
	pScene->qtn_nodeID = qtn_scene_i ;
	qtn_scene_i ++;

	*render_scene = pScene;
    return 0;	
}

/**
 * @fn f_int32_t memPoolInit(void)
 *  @brief �ڴ�س�ʼ��.
 *  @exception void
 *  @return f_int32_t,��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ʧ��.
 * @see 
 * @note 
*/
f_int32_t memPoolInit(void)
{
	if(FALSE == InitMemoryPoolsDefault())
	{
	    DEBUG_PRINT("init memory pools error");
	    return(-1);
	}
	return 0;
}

/**
 * @fn f_int32_t	paintunitViewwndSceneAttach(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene)
 *  @brief �ҽӻ��Ƶ�Ԫ���Ӵ�����ͳ������.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] pviewwnd �Ӵ����.
 *  @param[in] pscene �������.
 *  @exception void
 *  @return f_int32_t, �ҽӳɹ���־.
 *  @retval 0  �ҽӳɹ�.
 *  @retval -1 �ҽ�ʧ��.
 * @see 
 * @note
*/
f_int32_t paintunitViewwndSceneAttach(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene)
{
	sPAINTUNIT *pUnit = NULL;
	sViewWindow *pViewWind = NULL;
	sGLRENDERSCENE *pScene = NULL;
	
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(-1);
	    
	pViewWind = viewWindowPtrValid(pviewwnd);
	if(NULL == pViewWind)
	    return(-1);
	
	pScene = scenePtrValid(pscene);
	if(NULL == pScene)
	    return(-1);
	    
	pUnit->pViewWindow = pViewWind;
	pUnit->pScene  = pScene;    
    return(0);
}

/**
 * @fn sceneFuncAttach(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, \n
 *                     USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer, \n
 *                     FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt)
 *  @brief �ҽӳ����Ĺ��ܺ���.
 *  @param[in] scene �������.
 *  @param[in] draw2dMap 2D��ͼ���Ƶ���ں���.
 *  @param[in] draw3dMap 3D��ͼ���Ƶ���ں���.
 *  @param[in] drawScreenUserLayer �û���Ļͼ����Ƶ���ں���.
 *  @param[in] drawGeoUserLayer �û�����ͼ����Ƶ���ں���.
 *  @param[in] screenPt2GeoPt ��Ļ���굽��������ת������ں���.
 *  @param[in] geoPt2ScreenPt �������굽��Ļ����ת������ں���.
 *  @exception void
 *  @return f_int32_t, �ҽӳɹ���־.
 *  @retval 0  �ҽӳɹ�.
 *  @retval -1 �ҽ�ʧ��.
 * @see 
 * @note
*/
f_int32_t sceneFuncAttach(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, 
                          USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer,
                          FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt)
{
	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return(-1);
	 
	pScene->func2dMap = draw2dMap;   
	pScene->func3dMap = draw3dMap;
	pScene->drawScreenUserLayer = drawScreenUserLayer;
	pScene->drawGeoUserLayer   = drawGeoUserLayer;
	pScene->funcScreenPt2GeoPt = screenPt2GeoPt;
	pScene->funcGeoPt2ScreenPt = geoPt2ScreenPt;
	return(0);
}

