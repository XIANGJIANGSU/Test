#ifndef _INIT_SCENE_h_
#define _INIT_SCENE_h_

#include "../mapApi/common.h"
#include "../define/mbaseType.h"
#include "appHead.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn void initGLSetting(void)
 *  @brief GL������ʼ��.
 *  @param[in] void. 
 *  @exception void 
 *  @return void
 * @see 
 * @note
*/
void initGLSetting(void);

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
f_int32_t projectMdInit(VOIDPtr scene, f_int32_t pmode);

/**
 * @fn void mctParamInit(sMCTPARAM param)
 *  @brief ī���в�����ʼ��.
 *  @param[in] param ͶӰ����.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void mctParamInit(sMCTPARAM param);

/**
 * @fn void lbtParamInit(sLBTPARAM param)
 *  @brief �����ز�����ʼ��.
 *  @param[in] param ͶӰ����.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void lbtParamInit(sLBTPARAM param);

/**
 * @fn void clearColorSet(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
 *  @brief ������������ɫ.
 *  @param[in] red,green,blue,alpha ��ɫ��rgba������0.0f~1.0f. 
 *  @exception void 
 *  @return void
 * @see 
 * @note �����������������������������ӿ���������ֻ����Ӧ�ò�����һ�ξͿ����ˣ�������ÿ������������
*/
void clearColorSet(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha);

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
sPAINTUNIT * createUnit(void);

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
f_int32_t createRenderViewWindow(VOIDPtr *paintwind);

/**
 * @fn f_int32_t createRenderScene(VOIDPtr *render_scene)
 *  @brief �����������.
 *  @param[in] render_scene �������.
 *  @exception void
 *  @return f_int32_t, �����ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1: ��Ⱦ�����������ʧ��.
 *  @retval -2: ��ͼ�����ʼ��ʧ��.
 * @see 
 * @note
*/
f_int32_t createRenderScene(VOIDPtr *render_scene);

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
f_int32_t memPoolInit(void);

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
f_int32_t paintunitViewwndSceneAttach(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene);

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
                          FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
