#include "initApi.h"
#include "common.h"
#include "../mapApp/initScene.h"
#include "../mapApp/initData.h"
#include "../mapApp/mapApp.h"
#include "../mapApp/bil.h"

/**
 * @fn void initScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height,\n
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
void initScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height, 
                     f_int32_t width, f_int32_t height)
{	
	setScreenParam(rotate_type, phi_width, phi_height, width, height);
}

/**
 * @fn void setScreenWindowSize(f_int32_t width, f_int32_t height)
 *  @brief 初始化显示屏幕的窗口尺寸.
 *  @param[in] width        窗口的宽度(单位：像素). 
 *  @param[in] height       窗口的高度(单位：像素). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setScreenWindowSize(f_int32_t width, f_int32_t height)
{	
	setScreenWndSize(width, height);
}

/**
 * @fn void getScreenWindowSize(f_int32_t width, f_int32_t height)
 *  @brief 获取显示屏幕的窗口尺寸.
 *  @param[in] width        窗口的宽度(单位：像素). 
 *  @param[in] height       窗口的高度(单位：像素). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenWindowSize(f_int32_t *pwidth, f_int32_t *pheight)
{	
	getScreenWndSize(pwidth, pheight);
}

/**
 * @fn VOIDPtr createPaintUnit(void)
 *  @brief 创建图形绘制单元句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return VOIDPtr.
 *  @retval 图形绘制单元句柄.
 *  @retval NULL    失败.
 * @see 
 * @note
*/
VOIDPtr createPaintUnit(void)
{	
    return( (VOIDPtr)(createUnit()) );
}

/**
 * @fn f_int32_t createPaintViewWindow(VOIDPtr *pviewwind)
 *  @brief 创建图形绘制的视窗.
 *  @param[in] pviewwind 视窗句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return f_int32_t,创建成功标志.
 *  @retval 0 成功.
 *  @retval -1 视窗句柄创建失败.
 * @see 
 * @note
*/
f_int32_t createPaintViewWindow(VOIDPtr *pviewwind)
{
	f_int32_t ret = -1;
	ret = createRenderViewWindow(pviewwind);
	return ret;
}

/**
 * @fn f_int32_t createScene(VOIDPtr *scene)
 *  @brief 场景句柄初始化.
 *  @param[in] scene 场景句柄.
 *  @exception mallocError 创建句柄时内存申请失败
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 场景句柄初始化失败.
 *  @retval -2 地图句柄初始化失败.
 * @see 
 * @note
*/
f_int32_t createScene(VOIDPtr *scene)
{
	f_int32_t ret = -1;
	ret = createRenderScene(scene);
	return ret;
}

/**
 * @fn void setClearColor(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
 *  @brief 设置清屏的颜色.
 *  @param[in] red,green,blue,alpha 颜色的rgba分量，0.0f~1.0f. 
 *  @exception void 
 *  @return void
 * @see 
 * @note 清屏是清除整个缓冲区，不能针对视口清除，因此只需在应用层设置一次就可以了，不需在每个场景都设置
*/
void setClearColor(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
{
	clearColorSet(red, green, blue, alpha);
}

/**
 * @fn void initGL(void)
 *  @brief OpenGL环境初始化.
 *  @param[in] void
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void initGL(void)
{
	initGLSetting();
}


/**
 * @fn f_int32_t initProjectMode(VOIDPtr scene, f_int32_t pmode)
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
f_int32_t initProjectMode(VOIDPtr scene, f_int32_t pmode)
{
	return(projectMdInit(scene, pmode));
}

/**
 * @fn void initMctParam(sMCTPARAM param)
 *  @brief 墨卡托参数初始化.
 *  @param[in] param 投影参数.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void initMctParam(sMCTPARAM param)
{
	mctParamInit(param);
}

/**
 * @fn void initLbtParam(sLBTPARAM param)
 *  @brief 兰勃特参数初始化.
 *  @param[in] param 投影参数.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void initLbtParam(sLBTPARAM param)
{
	lbtParamInit(param);
}

/**
 * @fn f_int32_t initData(VOIDPtr scene, f_char_t *data_path)
 *  @brief 数据初始化.
 *  @param[in] scene 视口所在的场景句柄.
 *  @param[in] data_path 视口对应数据路径的根目录.
 *  @exception void
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
 * @see 
 * @note 
*/
f_int32_t initData(f_char_t *data_path)
{
	return( initMapData(data_path) );
}

/**
 * @fn BOOL createFont(VOIDPtr scene, f_char_t *font_path, f_char_t *font_file, \n
                       f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4])
 *  @brief 字体创建.
 *  @param[in] scene 视口所在的场景句柄.
 *  @param[in] font_path 字库文件路径（不含文件名）.
 *  @param[in] font_file 字库文件名.
 *  @param[in] font_size 字体大小.
 *  @param[in] edge_size 字体边缘大小.
 *  @param[in] color_font 字体颜色.
 *  @param[in] color_edge 边框颜色.
 *  @exception void
 *  @return BOOL,字体创建成功标志.
 *  @retval TRUE 成功.
 *  @retval FALSE 失败.
 * @see 
 * @note 
*/
BOOL createFont(f_char_t *font_path, f_char_t *font_file, 
                f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4])
{
	return( createTtfFont(font_path, font_file, font_size, edge_size, color_font, color_edge) );
}

/**
 * @fn f_int32_t initMemoryPool(void)
 *  @brief 内存池初始化.
 *  @exception void
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
 * @see 
 * @note 
*/
f_int32_t initMemoryPool(void)
{
	return( memPoolInit() );
}

/**
 * @fn f_int32_t attachViewwndScene2Painunit(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene)
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
f_int32_t attachViewwndScene2Painunit(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene)
{
    return(paintunitViewwndSceneAttach(punit, pviewwnd, pscene));
}

/**
 * @fn f_int32_t attachFunc2Scene(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, 
 *                                USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer,
 *                                FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt)
 *  @brief 挂接场景的功能函数.
 *  @param[in] scene 场景句柄.
 *  @param[in] draw2dMap 2D地图绘制的入口函数.
 *  @param[in] draw3dMap 3D地图绘制的入口函数.
 *  @param[in] drawScreenUserLayer 用户屏幕图层绘制的入口函数.
 *  @param[in] drawGeoUserLayer    用户地理图层绘制的入口函数.
 *  @param[in] screenPt2GeoPt 屏幕坐标到地理坐标转换的入口函数.
 *  @param[in] geoPt2ScreenPt 地理坐标到屏幕坐标转换的入口函数.
 *  @exception void
 *  @return f_int32_t, 挂接成功标志.
 *  @retval 0  挂接成功.
 *  @retval -1 挂接失败.
 * @see 
 * @note
*/
f_int32_t attachFunc2Scene(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, 
                           USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer,
                           FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt)
{
	return(sceneFuncAttach(scene, draw2dMap, draw3dMap, drawScreenUserLayer, drawGeoUserLayer, screenPt2GeoPt, geoPt2ScreenPt));
}
