/**
 * @file flyCmd.h
 * @brief ���ļ��ṩ��ͼ���п����������õĽӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-05-11
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _FLYCMD_h_ 
#define _FLYCMD_h_ 

#include "../define/mbaseType.h"
#include "appHead.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setEventParamPre(VOIDPtr render_scene)
 *  @brief �����¼�������ǰ�ú�������ȡ�������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setEventParamPre(VOIDPtr render_scene);

/**
 * @fn f_int32_t setEventParamPro(VOIDPtr render_scene)
 *  @brief �����¼������ĺ��ú������ͷŲ������õ��ź���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setEventParamPro(VOIDPtr render_scene);

/**
 * @fn setEventParam(sGLRENDERSCENE *pScene)
 *  @brief �����¼���������Ҫ�ǿ�������. 
 *  @param[in] pScene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note Ϊ�˱�֤���в�����һ���ԣ���Ҫ���ź��������б���.
*/
void setEventParam(sGLRENDERSCENE *pScene);

/**
 * @fn void setModeCor(VOIDPtr render_scene, f_int32_t cormode)
 *  @brief ������ɫģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] cormode ��ɫģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeCor(VOIDPtr render_scene, f_int32_t cormode);

/**
 * @fn void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
 *  @brief ������ҹģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] dnmode ��ҹģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDN(VOIDPtr render_scene, f_int32_t dnmode);

/**
 * @fn void setModeSVS(VOIDPtr render_scene, f_int32_t svsmode)
 *  @brief ����SVS ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] svsmode SVS ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSVS(VOIDPtr render_scene, f_int32_t svsmode);

/**
 * @fn setMode3DMD(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief �����Ƿ���ʾ3Dģ��
 *  @param[in] render_scene �������.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode3DMD(VOIDPtr render_scene, f_int32_t isdisplay);

/**
 * @fn void setModeMv(VOIDPtr render_scene, f_int32_t mvmode)
 *  @brief �����˶�ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] mvmode �˶�ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMv(VOIDPtr render_scene, f_int32_t mvmode);

/**
 * @fn void setModePSDisplay(VOIDPtr render_scene, f_int32_t isdisplay)
 *  @brief �����Ƿ���ʾ�ɻ�����.
 *  @param[in] render_scene �������.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModePSDisplay(VOIDPtr render_scene, f_int32_t isdisplay);

/**
 * @fn void setModeFrz(VOIDPtr render_scene, f_int32_t frzmode)
 *  @brief ���ö���ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] frzmode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeFrz(VOIDPtr render_scene, f_int32_t frzmode);

/**
 * @fn void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
 *  @brief ��������ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] rommode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeRm(VOIDPtr render_scene, f_int32_t rommode);

/**
 * @fn void setModeRm(VOIDPtr render_scene, f_int32_t rommode)
 *  @brief ��������ģʽ.������
 *  @param[in] render_scene �������.
 *  @param[in] rommode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode2dRm(VOIDPtr render_scene, f_int32_t rommode);

/**
 * @fn void setFlagRm(VOIDPtr render_scene, f_int32_t romflag)
 *  @brief �������α�־.
 *  @param[in] render_scene �������.
 *  @param[in] romflag ���α�־.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setFlagRm(VOIDPtr render_scene, f_int32_t romflag);

/**
 * @fn void setFlagRmHm(VOIDPtr render_scene, f_int32_t romhomeflag)
 *  @brief �������ι�λ��־.
 *  @param[in] render_scene �������.
 *  @param[in] romflag ���ι�λ��־.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setFlagRmHm(VOIDPtr render_scene, f_int32_t romhomeflag);

/**
 * @fn void setModeTwarn(VOIDPtr render_scene, f_int32_t terwarnmode)
 *  @brief ���õ��θ澯ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] terwarnmode ���θ澯ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeTwarn(VOIDPtr render_scene, f_int32_t terwarnmode);

/**
 * @fn void setModeSce(VOIDPtr render_scene, f_int32_t scenemode)
 *  @brief �������ó�����ʾģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] scenemode ������ʾģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSce(VOIDPtr render_scene, f_int32_t scenemode);

/**
 * @fn void setModeDN(VOIDPtr render_scene, f_int32_t dnmode)
 *  @brief ���ö���άģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] dnmode ����άģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode23D(VOIDPtr render_scene, f_int32_t mode23d);
/**
 * @fn void setModeDisplay(VOIDPtr render_scene, f_int32_t modedisplay)
 *  @brief ���õ�ͼ��ͼ����ӷ�ʽ
 *  @param[in] render_scene �������.
 *  @param[in] modedisplay ͼ�����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDisplay(VOIDPtr render_scene, f_int32_t modedisplay);

/**
 * @fn void setModeClose3d(VOIDPtr render_scene, f_int32_t modedisplay)
 *  @brief ������ά��ͼ�Ƿ�����
 *  @param[in] render_scene �������.
 *  @param[in] modedisplay 0 : �����Σ�1 :����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeClose3d(VOIDPtr render_scene, f_int32_t modedisplay);

/**
 * @fn void setCamCtlParamViewRes(VOIDPtr render_scene, f_float32_t viewres)
 *  @brief ����������Ʋ����е���ʾ�ֱ���.
 *  @param[in] render_scene �������.
 *  @param[in] viewres �ֱ��ʣ���/���أ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewRes(VOIDPtr render_scene, f_float32_t viewres);

/**
 * @fn void setCamCtlParamViewAngle(VOIDPtr render_scene, f_float32_t viewangle)
 *  @brief ��������������Ʋ����е�ƽ��ͷ�ӽ�.
 *  @param[in] render_scene �������.
 *  @param[in] viewangle �ӽǣ���λ���ȣ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewAngle(VOIDPtr render_scene, f_float32_t viewangle);

/**
 * @fn void setCamCtlParamCmpRd(VOIDPtr render_scene, f_int32_t radius)
 *  @brief ����������Ʋ����Ŀ̶Ȼ��뾶.
 *  @param[in] render_scene �������.
 *  @param[in] radius �̶Ȼ��뾶�����أ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamCmpRd(VOIDPtr render_scene, f_int32_t radius);

/**
 * @fn void setCamCtlParamRtCent(VOIDPtr render_scene, f_int32_t xcenter, f_int32_t ycenter)
 *  @brief ����������Ʋ����еĵ�ͼ��ת����.
 *  @param[in] render_scene �������.
 *  @param[in] xcenter��ycenter ��ͼ��ת���ĵ�λ�ã����أ�������ⲿ�Ӵ����½ǣ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamRtCent(VOIDPtr render_scene, f_int32_t xcenter, f_int32_t ycenter);

/**
 * @fn void setCamCtlParamViewType(VOIDPtr render_scene, f_int32_t vtype)
 *  @brief �����ӽ�ģʽ.
 *  @param[in] render_scene �������.
 *  @param[in] vtype �ӽ�����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setCamCtlParamViewType(VOIDPtr render_scene, f_int32_t vtype);


/**
 * @fn void setColorAndLayermaskCfgIndex(VOIDPtr render_scene, f_int32_t index)
 *  @brief ����ѡ�õ���ɫ�ͷ�ӵ�����ļ�������
 *  @param[in] render_scene �������.
 *  @param[in] index��0-7�����֧��8��
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setColorAndLayermaskCfgIndex(VOIDPtr render_scene, f_int32_t index);


/**
 * @fn void setAreaBaDraw(VOIDPtr render_scene, f_int32_t bDraw)
 *  @brief ����ʸ���������Ƿ����
 *  @param[in] render_scene �������.
 *  @param[in] bDraw 0-������ 1-����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAreaBaDraw(VOIDPtr render_scene, f_int32_t bDraw);

/**
 * @fn void setSeaMapDraw(VOIDPtr render_scene, f_int32_t bDraw)
 *  @brief ���ú�ͼ�Ƿ����
 *  @param[in] render_scene �������.
 *  @param[in] bDraw 0-������ 1-����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setSeaMapDraw(VOIDPtr render_scene, f_int32_t bDraw);


/**
 * @fn void setTextDrawLevel(VOIDPtr render_scene, f_int32_t level)
 *  @brief ���õ���ע�Ǻͷ�����ʾ��λ
 *  @param[in] render_scene �������.
 *  @param[in] level 0-����ʾ�κ�ע�Ǻͷ��� 1-ֻ��ʾʡ����к��еȳ��еĵ���ע�Ǻͷ��� 2-������ʾ���е�ע�Ǻͷ���
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setTextDrawLevel(VOIDPtr render_scene, f_int32_t level);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
