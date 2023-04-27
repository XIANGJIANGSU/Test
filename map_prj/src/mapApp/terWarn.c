#include "../engine/terWarning.h"
#include "mapApp.h"
#include "terWarn.h"

/**
 * @fn f_int32_t updateTerWarning(sGLRENDERSCENE *pScene)
 *  @brief ���µ��θ澯����. 
 *  @param[in] pScene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤������һ���ԣ���Ҫ���ź��������б���.
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
 *  @brief ���õ��θ澯������ǰ�ú���,��ȡ�������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @param[in] warning_layers ���θ澯����.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
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
 *  @brief �����¼������ĺ��ú������ͷŲ������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
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
 *  @brief ���ð�ȫ�߶ȣ������ε��ڰ�ȫ�߶�ʱ����red��green��blue����Ⱦ. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �㼶����.
 *  @param[in] height �߶ȣ����θ߶��� �ɻ��߶�+height ����ʱ��red��green��blue����Ⱦ.
 *  @param[in] red    ��ɫ����.
 *  @param[in] green  ��ɫ����.
 *  @param[in] blue   ��ɫ����.
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
 *  @brief ����Σ�ո߶ȣ������θ���Σ�ո߶�ʱ����red��green��blue����Ⱦ. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �㼶����.
 *  @param[in] height �߶ȣ����θ߶ȸ��� �ɻ��߶�+height ʱ��red��green��blue����Ⱦ.
 *  @param[in] red    ��ɫ����.
 *  @param[in] green  ��ɫ����.
 *  @param[in] blue   ��ɫ����.
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
 *  @brief ���õ��θ澯��������������(�ɻ��߶�+height_min)��(�ɻ��߶�+height_max)����ʱ����red��green��blue����Ⱦ. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �㼶����.
 *  @param[in] height_min �߶ȵ�ֵ.
 *  @param[in] height_max �߶ȸ�ֵ.
 *  @param[in] red    ��ɫ����.
 *  @param[in] green  ��ɫ����.
 *  @param[in] blue   ��ɫ����.
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

