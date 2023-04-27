/**
 * @file mapApi.h
 * @brief ���ļ��ṩ��ͼ����ͼ����Ⱦ���ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-22
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _MAP_API_h_ 
#define _MAP_API_h_ 

#include "../define/mbaseType.h"
#include "common.h"
#include "../mapApp/appHead.h"
#include "../engine/mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setViewWindowParamBegin(VOIDPtr punit)
 *  @brief �����Ӵ�������ǰ�ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamBegin(VOIDPtr punit);

/**
 * @fn f_int32_t setViewWindowParamEnd(VOIDPtr punit)
 *  @brief �����Ӵ������ĺ��ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setViewWindowParamEnd(VOIDPtr punit);

/**
 * @fn f_int32_t setViewWndSize(VOIDPtr punit, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
 *  @brief �����Ӵ��ߴ�.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] llx��lly�Ӵ������½���ʼλ�ã���λ�����أ�.
 *  @param[in] width��height�Ӵ��Ŀ�ȡ��߶ȣ���λ�����أ�.
 *  @exception ptrError ������Ϊ��
 *  @return f_int32_t,�ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 �Ӵ����Ϊ��.
 *  @retval -2 ��ʼλ�ò��ڷ�Χ��.
 *  @retval -3 ��ȡ��߶Ȳ��ڷ�Χ��.
 * @see 
 * @note
*/
f_int32_t setViewWndSize(VOIDPtr punit, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height);

/**
 * @fn void setViewWndAlpha(VOIDPtr punit, f_float32_t alpha)
 *  @brief �����Ӵ���͸����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] alpha ͸����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWndAlpha(VOIDPtr punit, f_float32_t alpha);

/**
 * @fn void setViewWndBkColor(VOIDPtr punit, sColor3f bkcolor)
 *  @brief �����Ӵ��ı���ɫ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] bkcolor ����ɫ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setViewWndBkColor(VOIDPtr punit, sColor3f bkcolor);

/**
 * @fn void map2dRender(VOIDPtr scene)
 *  @brief 2D��ͼ����.
 *  @param[in] pscene �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map2dRender(VOIDPtr pscene);

/**
 * @fn void map3dRender(VOIDPtr scene)
 *  @brief 3D��ͼ����.
 *  @param[in] pscene �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map3dRender(VOIDPtr pscene);

/**
 * @fn void mapRender(VOIDPtr punit)
 *  @brief ��ͼ������Ⱦ.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void mapRender(VOIDPtr punit);

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


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
