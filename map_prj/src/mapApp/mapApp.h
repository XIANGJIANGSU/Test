#ifndef _MAP_APP_h_ 
#define _MAP_APP_h_ 

#include "../define/mbaseType.h"
#include "appHead.h"
#include "../engine/mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
                    f_int32_t width, f_int32_t height);

/**
 * @fn void getScreenRotateType(f_int32_t *protatetype)
 *  @brief ��ȡ��Ļ��ת��ʽ. 0:����,1:˳ʱ��ת90��,2:˳ʱ����ת180��,3:˳ʱ����ת270��.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenRotateType(f_int32_t *protatetype);


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
void setScreenWndSize(f_int32_t width, f_int32_t height);

/**
 * @fn void getScreenWndSize(f_int32_t *pwidth, f_int32_t *pheight)
 *  @brief ��ȡ��Ļ���ڳߴ�(���طֱ���). 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void getScreenWndSize(f_int32_t *pwidth, f_int32_t *pheight);                   

/**
 * @fn sPAINTUNIT *unitPtrValid(VOIDPtr punit)
 *  @brief �ж�����Ļ��Ƶ�Ԫ����Ƿ���Ч.
 *  @param[in] punit ��Ԫ���.
 *  @exception void
 *  @return sPAINTUNIT *, ��Ԫ���.
 *  @retval ���Ƶ�Ԫ���.
 *  @retval NULL ���Ƶ�Ԫ�����Ч.
 * @see 
 * @note
*/
sPAINTUNIT *unitPtrValid(VOIDPtr punit);

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
sViewWindow *viewWindowPtrValid(VOIDPtr view_window);

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
sGLRENDERSCENE *scenePtrValid(VOIDPtr render_scene);
sMAPHANDLE *maphandlePtrValid(VOIDPtr map_handle);

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
f_int32_t setViewWindowParamPre(VOIDPtr view_window);

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
f_int32_t setViewWindowParamPro(VOIDPtr view_window);

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
void setViewWindowAlpha(VOIDPtr view_window, f_float32_t alpha);

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
void setViewWindowBkcolor(VOIDPtr view_window, sColor3f bkcolor);

/**
 * @fn f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
 *  @brief ������Ⱦ�Ӵ��Ĳ���.
 *  @param[in] view_window ��Ⱦ�Ӵ��ľ��.
 *  @param[in] scene ��Ⱦ�����ľ��.
 *  @param[in] llx �Ӵ������½Ǻ��������ʼλ��(��λ:����,�������ת�����Ļ���Ͻ�).
 *  @param[in] lly �Ӵ������½����������ʼλ��(��λ:����,�������ת�����Ļ���Ͻ�).
 *  @param[in] width �Ӵ���ȣ����أ�.
 *  @param[in] height �Ӵ��߶ȣ����أ�.
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
f_int32_t setViewWindowSize(VOIDPtr view_window, VOIDPtr render_scene, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height);

/**
 * @fn setViewWndParam(sViewWindow *pviewwind)
 *  @brief ������Ⱦ�Ӵ����йز����������ߴ硢���̻��뾶��͸���ȡ���ת����. 
 *  @param[in] pviewwind �Ӵ����.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤������һ���ԣ���Ҫ���ź��������б���.
*/
void setViewWndParam(sViewWindow *pviewwind);

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
void mapSceneRender(VOIDPtr render_viewwnd, VOIDPtr render_scene);

/**
 * @fn void render2dMap(VOIDPtr *scene)
 *  @brief 2d��ͼ������Ⱦ.
 *  @param[in] scene  �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void render2dMap(VOIDPtr scene);

/**
 * @fn void render3dMap(VOIDPtr *scene)
 *  @brief 3d��ͼ������Ⱦ.
 *  @param[in] scene  �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void render3dMap(VOIDPtr scene);

/*
*	����ģ�͵Ľӿں���
*/
extern BOOL RenderModel(BOOL SelectModel , sGLRENDERSCENE* pModelScene);

/*
 * ���ܣ�ͨ���Ѽ��غ���Ƭ���ݼ�����Ƭ��ĳ���Ӧ�ĸ߶�
 * ���룺lfLon ����
 *       lfLat γ��
 * �����pZ    �߶�
 * ���أ�����ɹ�������TRUE(1)������ʧ�ܣ�����FALSE(0)
 * ע�ͣ�zyp 2015-12-30 11:56:13
 */
BOOL GetZ(sGLRENDERSCENE *pScene, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ);
//���������sMAPHANDLE *
BOOL GetZex(sMAPHANDLE * pHandle, const f_float64_t lfLon,const f_float64_t lfLat,f_float64_t *pZ);

void render3dmap_simple(VOIDPtr scene);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
