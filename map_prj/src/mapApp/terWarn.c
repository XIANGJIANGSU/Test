#include "../engine/terWarning.h"
#include "mapApp.h"
#include "terWarn.h"

/**
 * @fn f_int32_t updateTerWarning(sGLRENDERSCENE *pScene)
 *  @brief 更新地形告警参数. 
 *  @param[in] pScene 场景句柄.
 *  @exception void.
 *  @return void.
 * @see 
 * @note 为了保证参数的一致性，需要用信号量来进行保护.
*/
static f_int32_t updateTerWarning(sGLRENDERSCENE *pScene)
{
	if(NULL == pScene)
	    return(sFail);
	
	takeSem(pScene->eventparamset_sem, FOREVER_WAIT);
	pScene->m_terwarning_set     = pScene->m_terwarning_set_input;
	giveSem(pScene->eventparamset_sem);
	
	setTerrainWarningStripeColor(pScene, pScene->m_terwarning_set);
	return(sSuccess);
}

/**
 * @fn f_int32_t setTerWarningPre(VOIDPtr render_scene, f_int32_t warning_layers)
 *  @brief 设置地形告警参数的前置函数,获取参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @param[in] warning_layers 地形告警级数.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setTerWarningPre(VOIDPtr render_scene, f_int32_t warning_layers)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(sFail);

	pScene->m_terwarning_set_input.layers = warning_layers;
	return(sSuccess);
}

/**
 * @fn f_int32_t setTerWarningPro(VOIDPtr render_scene)
 *  @brief 设置事件参数的后置函数，释放参数设置的信号量. 
 *  @param[in] render_scene 场景句柄.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  失败.
 *  @retval sSuccess  成功.
 * @see 
 * @note
*/
f_int32_t setTerWarningPro(VOIDPtr render_scene)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(sFail);
	
	if(sFail == updateTerWarning(pScene))
	    return(sFail);
	    
	return(sSuccess);
}

/**
 * @fn f_int32_t terSafeSet(VOIDPtr render_scene, f_int32_t index, f_float32_t height, \n
                            f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief 设置安全高度，当地形低于安全高度时，用red、green、blue来渲染. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 层级索引.
 *  @param[in] height 高度，地形高度在 飞机高度+height 以下时用red、green、blue来渲染.
 *  @param[in] red    红色分量.
 *  @param[in] green  绿色分量.
 *  @param[in] blue   蓝色分量.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void terSafeSet(VOIDPtr render_scene, f_int32_t index, f_float32_t height, 
                f_int32_t red, f_int32_t green, f_int32_t blue)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene || index < 0 || index > TER_WARNING_MAX_LAYERS)
	    return;
	    
	pScene->m_terwarning_set_input.height_min = height;
	pScene->m_terwarning_set_input.height_min_index = index;
	pScene->m_terwarning_set_input.ter_warning_data[index].index = index;
	pScene->m_terwarning_set_input.ter_warning_data[index].height_min = -9999.0f;
	pScene->m_terwarning_set_input.ter_warning_data[index].height_max = height;
	pScene->m_terwarning_set_input.ter_warning_data[index].red = red;
	pScene->m_terwarning_set_input.ter_warning_data[index].green = green;
	pScene->m_terwarning_set_input.ter_warning_data[index].blue = blue;
}

/**
 * @fn void terDangerousSet(VOIDPtr punit, f_int32_t index, f_float32_t height, \n
                            f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief 设置危险高度，当地形高于危险高度时，用red、green、blue来渲染. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 层级索引.
 *  @param[in] height 高度，地形高度高于 飞机高度+height 时用red、green、blue来渲染.
 *  @param[in] red    红色分量.
 *  @param[in] green  绿色分量.
 *  @param[in] blue   蓝色分量.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void terDangerousSet(VOIDPtr render_scene, f_int32_t index, f_float32_t height, 
                     f_int32_t red, f_int32_t green, f_int32_t blue)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene || index < 0 || index > TER_WARNING_MAX_LAYERS)
	    return;
	    
	pScene->m_terwarning_set_input.height_max = height;
	pScene->m_terwarning_set_input.height_max_index = index;
	pScene->m_terwarning_set_input.ter_warning_data[index].index = index;
	pScene->m_terwarning_set_input.ter_warning_data[index].height_min = height;
	pScene->m_terwarning_set_input.ter_warning_data[index].height_max = 9999.0f;
	pScene->m_terwarning_set_input.ter_warning_data[index].red = red;
	pScene->m_terwarning_set_input.ter_warning_data[index].green = green;
	pScene->m_terwarning_set_input.ter_warning_data[index].blue = blue;
}

/**
 * @fn void terWarningSet(VOIDPtr render_scene, f_int32_t index, f_float32_t height_min, f_float32_t height_max,
                          f_int32_t red, f_int32_t green, f_int32_t blue)
 *  @brief 设置地形告警参数，当地形在(飞机高度+height_min)～(飞机高度+height_max)区间时，用red、green、blue来渲染. 
 *  @param[in] punit 绘制单元句柄.
 *  @param[in] index 层级索引.
 *  @param[in] height_min 高度低值.
 *  @param[in] height_max 高度高值.
 *  @param[in] red    红色分量.
 *  @param[in] green  绿色分量.
 *  @param[in] blue   蓝色分量.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void terWarningSet(VOIDPtr render_scene, f_int32_t index, f_float32_t height_min, f_float32_t height_max, 
                   f_int32_t red, f_int32_t green, f_int32_t blue)
{
	sGLRENDERSCENE *pScene = NULL;
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene || index < 0 || index > TER_WARNING_MAX_LAYERS)
	    return;

	pScene->m_terwarning_set_input.ter_warning_data[index].index = index;
	pScene->m_terwarning_set_input.ter_warning_data[index].height_min = height_min;
	pScene->m_terwarning_set_input.ter_warning_data[index].height_max = height_max;
	pScene->m_terwarning_set_input.ter_warning_data[index].red = red;
	pScene->m_terwarning_set_input.ter_warning_data[index].green = green;
	pScene->m_terwarning_set_input.ter_warning_data[index].blue = blue;
}

