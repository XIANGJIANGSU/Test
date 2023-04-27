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
 *  @brief GL环境初始化.
 *  @param[in] void. 
 *  @exception void 
 *  @return void
 * @see 
 * @note
*/
void initGLSetting(void)
{
	/*指定图元中各个顶点间颜色的过渡模式,GL_SMOOTH(默认值)平滑着色,GL_FLAT恒定着色*/
	glShadeModel(GL_SMOOTH);
	/*使能线图元的平滑着色,避免线段呈现锯齿状*/
	glEnableEx(GL_LINE_SMOOTH);
	/*设置清屏颜色,并不进行清屏操作,glClear(GL_COLOR_BUFFER_BIT)才是清屏操作，后续会重新设置，此处无意义*/
	//glClearColor(color_red, color_green, color_blue, color_alpha);
	/*设置深度缓冲区的参考值为1.0,并不进行清深度缓冲区的操作,glClear(GL_DEPTH_BUFFER_BIT)才是清深度缓冲区操作*/
	glClearDepth(1.0f);	
	/*使能深度测试*/
	glEnableEx(GL_DEPTH_TEST);	
	/* 指定深度比较函数，如果深度值小于或等于参考值，则通过.必须先使能深度测试*/
	glDepthFunc(GL_LEQUAL);	
	/* 指定颜色和纹理坐标的差值质量,GL_NICEST表示高质量*/
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	            
	/* 指定多边形的正面方向,顶点序列按顺时针还是逆时针,默认是逆时针(GL_CCW),用于背面多边形的消除GL_CULL_FACE*/
	glFrontFace(GL_CCW);
	/* 开启模板测试，默认不开启，模板测试发生在透明度测试之后，深度测试之前*/
	glEnableEx(GL_STENCIL_TEST);
	/* 设置模板缓冲区的写入掩码：当为false即0xff(默认值)时允许写入*/
	glStencilMask(0xff);
	/* 设置清除模板缓冲区的默认值为0*/
	glClearStencil(0);
	/* 默认值GL_ALWAYS,0,0xff 模板缓冲区中的值先与参数3进行与操作,结果与参数2进行比较,比较函数为参数1. GL_ALWAYS表示永远是默认值*/
	glStencilFunc(GL_ALWAYS,0,0x01);
	/* 清除模板缓冲区*/
	glClear(GL_STENCIL_BUFFER_BIT);
	/* 关闭混合,alpha值不生效*/
	glDisableEx(GL_BLEND);
	/* 关闭二维纹理 */
	glDisableEx(GL_TEXTURE_2D);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* 关闭一维纹理 */
	glDisableEx(GL_TEXTURE_1D);

#ifdef WIN32
	/* 获取OpenGL扩展函数的函数指针，嵌入式下无需获取 */
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
	glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
	glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)wglGetProcAddress("glMultiTexCoord1f");

	/* 获取OpenGL扩展函数的函数指针，嵌入式下无需获取 */                   
	sqsTextureFunInit();

	/* 获取OpenGL扩展函数的函数指针，嵌入式下无需获取 */ 
	vboFunInit();
#endif

#endif

	/* 生成两个纹理，一个用于VecShade模式，一个用于方里网 */
	InitVecShadedReliefMap();
}


/**
 * @fn f_int32_t projectMdInit(VOIDPtr scene, f_int32_t pmode)
 *  @brief 投影方式初始化.
 *  @param[in] scene 场景句柄.
 *  @param[in] pmode 投影方式.
 *  @exception void
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
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
 *  @brief 墨卡托参数初始化.
 *  @param[in] param 投影参数.
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
 *  @brief 兰勃特参数初始化.
 *  @param[in] param 投影参数.
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
 *  @brief 设置清屏的颜色.
 *  @param[in] red,green,blue,alpha 颜色的rgba分量，0.0f~1.0f. 
 *  @exception void 
 *  @return void
 * @see 
 * @note 清屏是清除整个缓冲区，不能针对视口清除，因此只需在应用层设置一次就可以了，不需在每个场景都设置
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
 *  @brief 创建图形绘制单元句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return sPAINTUNIT *.
 *  @retval 图形绘制单元句柄.
 *  @retval NULL    失败.
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
 *  @brief 创建图形渲染视窗.
 *  @param[in] pviewwind 视窗句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return f_int32_t,创建成功标志.
 *  @retval 0 成功.
 *  @retval -1 视窗句柄创建失败.
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
 *  @brief 创建场景句柄.
 *  @param[in] render_scene 场景句柄.
 *  @exception void
 *  @retval f_int32_t, 创建成功标志.
 *  @retval 0 成功.
 *  @retval -1: 渲染场景句柄创建失败.
 *  @retval -2: 地图句柄初始化失败.
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
    	/**链表初始化**/
		pstListHead = &(pScene->scene_draw_list);
		LIST_INIT(pstListHead);
    }

	/**设置场景索引号,视口0使用0,视口1使用1,视口2使用2**/
	pScene->scene_index = qtn_scene_i;

	/**设置需要采用的调度qtn数组的ID号,视口0使用0,视口1使用1,视口2使用2**/
	pScene->qtn_nodeID = qtn_scene_i ;
	qtn_scene_i ++;

	*render_scene = pScene;
    return 0;	
}

/**
 * @fn f_int32_t memPoolInit(void)
 *  @brief 内存池初始化.
 *  @exception void
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
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
 *  @brief 挂接绘制单元的视窗句柄和场景句柄.
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] pviewwnd 视窗句柄.
 *  @param[in] pscene 场景句柄.
 *  @exception void
 *  @return f_int32_t, 挂接成功标志.
 *  @retval 0  挂接成功.
 *  @retval -1 挂接失败.
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
 *  @brief 挂接场景的功能函数.
 *  @param[in] scene 场景句柄.
 *  @param[in] draw2dMap 2D地图绘制的入口函数.
 *  @param[in] draw3dMap 3D地图绘制的入口函数.
 *  @param[in] drawScreenUserLayer 用户屏幕图层绘制的入口函数.
 *  @param[in] drawGeoUserLayer 用户地理图层绘制的入口函数.
 *  @param[in] screenPt2GeoPt 屏幕坐标到地理坐标转换的入口函数.
 *  @param[in] geoPt2ScreenPt 地理坐标到屏幕坐标转换的入口函数.
 *  @exception void
 *  @return f_int32_t, 挂接成功标志.
 *  @retval 0  挂接成功.
 *  @retval -1 挂接失败.
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

