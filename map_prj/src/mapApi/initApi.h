/**
 * @file initApi.h
 * @brief ���ļ��ṩ��ͼ�����ʼ�����ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-22
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

/*
��Ļ���Ӵ�������֮��Ĺ�ϵ��
һ����ʾ����Ӧһ����Ļ������Ӵ����������
��һ������������attach����һ�Ӵ���
*/

#ifndef _INIT_API_h_ 
#define _INIT_API_h_ 

#include "../define/mbaseType.h"
#include "common.h"
#include "../mapApp/appHead.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn void initScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height,\n
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
void initScreenParam(f_int32_t rotate_type, f_float32_t phi_width, f_float32_t phi_height, 
                     f_int32_t width, f_int32_t height);

/**
 * @fn void setScreenWindowSize(f_int32_t width, f_int32_t height)
 *  @brief ��ʼ�����ڳߴ�.
 *  @param[in] width        ���ڵĿ��(��λ������). 
 *  @param[in] height       ���ڵĸ߶�(��λ������). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void setScreenWindowSize(f_int32_t width, f_int32_t height);

/**
 * @fn void getScreenWindowSize(f_int32_t width, f_int32_t height)
 *  @brief ��ȡ���ڳߴ�.
 *  @param[in] width        ���ڵĿ��(��λ������). 
 *  @param[in] height       ���ڵĸ߶�(��λ������). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenWindowSize(f_int32_t *pwidth, f_int32_t *pheight);                     

/**
 * @fn VOIDPtr createPaintUnit(void)
 *  @brief ����ͼ�λ��Ƶ�Ԫ���.
 *  @exception mallocError �������ʱ�ڴ�����ʧ��
 *  @return VOIDPtr.
 *  @retval ͼ�λ��Ƶ�Ԫ���.
 *  @retval NULL    ʧ��.
 * @see 
 * @note
*/
VOIDPtr createPaintUnit(void);

/**
 * @fn f_int32_t createPaintViewWindow(VOIDPtr *pviewwind)
 *  @brief ����ͼ�λ��Ƶ��Ӵ�.
 *  @param[in] pviewwind �Ӵ����.
 *  @exception mallocError �������ʱ�ڴ�����ʧ��
 *  @return f_int32_t,�����ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 �Ӵ��������ʧ��.
 * @see 
 * @note
*/
f_int32_t createPaintViewWindow(VOIDPtr *pviewwind);

/**
 * @fn f_int32_t createScene(VOIDPtr *scene)
 *  @brief ���������ʼ��.
 *  @param[in] scene �������.
 *  @exception mallocError �������ʱ�ڴ�����ʧ��
 *  @return f_int32_t,��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ���������ʼ��ʧ��.
 *  @retval -2 ��ͼ�����ʼ��ʧ��.
 * @see 
 * @note
*/
f_int32_t createScene(VOIDPtr *scene);

/**
 * @fn void setClearColor(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha)
 *  @brief ������������ɫ.
 *  @param[in] red,green,blue,alpha ��ɫ��rgba������0.0f~1.0f. 
 *  @exception void 
 *  @return void
 * @see 
 * @note �����������������������������ӿ���������ֻ����Ӧ�ò�����һ�ξͿ����ˣ�������ÿ������������
*/
void setClearColor(f_float32_t red, f_float32_t green, f_float32_t blue, f_float32_t alpha);

/**
 * @fn void initGL(void)
 *  @brief OpenGL������ʼ��.
 *  @param[in] void
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void initGL(void);

/**
 * @fn f_int32_t initProjectMode(VOIDPtr scene, f_int32_t pmode)
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
f_int32_t initProjectMode(VOIDPtr scene, f_int32_t pmode);

/**
 * @fn void initMctParam(sMCTPARAM param)
 *  @brief ī���в�����ʼ��.
 *  @param[in] param ͶӰ����.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void initMctParam(sMCTPARAM param);

/**
 * @fn void initLbtParam(sLBTPARAM param)
 *  @brief �����ز�����ʼ��.
 *  @param[in] param ͶӰ����.
 *  @exception void
 *  @return void.
 * @see 
 * @note 
*/
void initLbtParam(sLBTPARAM param);

/**
 * @fn f_int32_t initData(VOIDPtr scene, f_char_t *data_path)
 *  @brief ���ݳ�ʼ��.
 *  @param[in] scene �ӿ����ڵĳ������.
 *  @param[in] data_path �ӿڶ�Ӧ����·���ĸ�Ŀ¼.
 *  @exception void
 *  @return f_int32_t,��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ʧ��.
 * @see 
 * @note 
*/
f_int32_t initData(f_char_t *data_path);

/**
 * @fn BOOL createFont(VOIDPtr scene, f_char_t *font_path, f_char_t *font_file, \n
                       f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4])
 *  @brief ���崴��.
 *  @param[in] scene �ӿ����ڵĳ������.
 *  @param[in] font_path �ֿ��ļ�·���������ļ�����.
 *  @param[in] font_file �ֿ��ļ���.
 *  @param[in] font_size �����С.
 *  @param[in] edge_size �����Ե��С.
 *  @param[in] color_font ������ɫ.
 *  @param[in] color_edge �߿���ɫ.
 *  @exception void
 *  @return BOOL,���崴���ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
 * @see 
 * @note 
*/
BOOL createFont(f_char_t *font_path, f_char_t *font_file, 
                f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4]);

/**
 * @fn f_int32_t initMemoryPool(void)
 *  @brief �ڴ�س�ʼ��.
 *  @exception void
 *  @return f_int32_t,��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ʧ��.
 * @see 
 * @note 
*/
f_int32_t initMemoryPool(void);

/**
 * @fn f_int32_t attachViewwndScene2Painunit(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene)
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
f_int32_t attachViewwndScene2Painunit(VOIDPtr punit, VOIDPtr pviewwnd, VOIDPtr pscene);

/**
 * @fn f_int32_t attachFunc2Scene(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, 
 *                                USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer,
 *                                FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt)
 *  @brief �ҽӳ����Ĺ��ܺ���.
 *  @param[in] scene �������.
 *  @param[in] draw2dMap 2D��ͼ���Ƶ���ں���.
 *  @param[in] draw3dMap 3D��ͼ���Ƶ���ں���.
 *  @param[in] drawScreenUserLayer �û���Ļͼ����Ƶ���ں���.
 *  @param[in] drawGeoUserLayer    �û�����ͼ����Ƶ���ں���.
 *  @param[in] screenPt2GeoPt ��Ļ���굽��������ת������ں���.
 *  @param[in] geoPt2ScreenPt �������굽��Ļ����ת������ں���.
 *  @exception void
 *  @return f_int32_t, �ҽӳɹ���־.
 *  @retval 0  �ҽӳɹ�.
 *  @retval -1 �ҽ�ʧ��.
 * @see 
 * @note
*/
f_int32_t attachFunc2Scene(VOIDPtr scene, FUNCTION draw2dMap, FUNCTION draw3dMap, 
                           USERLAYERFUNCTION drawScreenUserLayer, USERLAYERFUNCTION drawGeoUserLayer,
                           FUNCTION screenPt2GeoPt, FUNCTION geoPt2ScreenPt);

/*
*	��ʼ��ģ�ͻ�����Ϣ
*/
BOOL InitModel();

void AttachpScene2Model(sGLRENDERSCENE* pModelScene);

BOOL TestModelAppMake(int scene);

void TestModelAppSet(int scene);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
