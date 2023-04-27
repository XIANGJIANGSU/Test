/**
 * @file flyCmdApi.h
 * @brief ���ļ��ṩ��ͼ������п���������ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-05-11
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _FLYCMD_API_h_ 
#define _FLYCMD_API_h_ 

#include "common.h"
#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn f_int32_t setEventParamBegin(VOIDPtr punit)
 *  @brief �����¼�������ǰ�ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setEventParamBegin(VOIDPtr punit);

/**
 * @fn f_int32_t setEventParamEnd(VOIDPtr punit)
 *  @brief �����¼������ĺ��ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return f_int32_t.
 * @see 
 * @note
*/
f_int32_t setEventParamEnd(VOIDPtr punit);

/**
 * @fn void setModeColor(VOIDPtr punit, f_int32_t cormode)
 *  @brief ������ɫģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] cormode ��ɫģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeColor(VOIDPtr punit, f_int32_t cormode);

/**
 * @fn void setModeDayNight(VOIDPtr punit, f_int32_t dnmode)
 *  @brief ������ҹģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] dnmode ��ҹģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeDayNight(VOIDPtr punit, f_int32_t dnmode);

/**
 * @fn void setModeSVSView(VOIDPtr punit, f_int32_t svsmode)
 *  @brief ����SVS ��ʾģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] svsmode SVS ��ʾģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeSVSView(VOIDPtr punit, f_int32_t svsmode);

/**
 * @fn void setMode3DModelDisplay(VOIDPtr punit, f_int32_t isdisplay)
 *  @brief ����3D ģ���Ƿ���ʾ
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode3DModelDisplay(VOIDPtr punit, f_int32_t isdisplay);

/**
 * @fn void setModeMove(VOIDPtr punit, f_int32_t mvmode)
 *  @brief �����˶�ģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] mvmode �˶�ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMove(VOIDPtr punit, f_int32_t mvmode);

/**
 * @fn void setModePlaneSymbolDisplay(VOIDPtr punit, f_int32_t isdisplay)
 *  @brief �����Ƿ���ʾ�ɻ�����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModePlaneSymbolDisplay(VOIDPtr punit, f_int32_t isdisplay);

/**
 * @fn void setModeFreeze(VOIDPtr punit, f_int32_t frzmode)
 *  @brief ���ö���ģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] frzmode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeFreeze(VOIDPtr punit, f_int32_t frzmode);

/**
 * @fn void setModeRoma(VOIDPtr punit, f_int32_t rommode)
 *  @brief ��������ģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] rommode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeRoma(VOIDPtr punit, f_int32_t rommode);

/**
 * @fn void setModeRoma(VOIDPtr punit, f_int32_t rommode)
 *  @brief ��������ģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] rommode ����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setMode2dRoma(VOIDPtr punit, f_int32_t rommode);

/**
 * @fn void setRomFlag(VOIDPtr punit, f_int32_t romflag)
 *  @brief �������α�־.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] romflag ���α�־.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setRomFlag(VOIDPtr punit, f_int32_t romflag);


/**
 * @fn void setRomHomeFlag(VOIDPtr punit, f_int32_t romhomeflag)
 *  @brief �������ι�λ��־.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] romflag ���ι�λ��־.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setRomHomeFlag(VOIDPtr punit, f_int32_t romhomeflag);

/**
 * @fn void setModeTerwarn(VOIDPtr punit, f_int32_t terwarnmode)
 *  @brief ���õ��θ澯ģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] rommode ���θ澯ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeTerwarn(VOIDPtr punit, f_int32_t terwarnmode);

/**
 * @fn void setModeScene(VOIDPtr punit, f_int32_t scenemode)
 *  @brief ���ó�����ʾģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] scenemode ������ʾģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeScene(VOIDPtr punit, f_int32_t scenemode);
/**
 * @fn void setModeMap23D(VOIDPtr punit, f_int32_t scenemode)
 *  @brief ���ö���ά��ʾģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] scenemode ����ά��ʾģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMap23D(VOIDPtr punit, f_int32_t scenemode);

/**
 * @fn void setModeMapDisplay(VOIDPtr punit, f_int32_t scenemode)
 *  @brief ���õ�ͼͼ�����ģʽ
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] scenemode ͼ�����ģʽ.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMapDisplay(VOIDPtr punit, f_int32_t scenemode);

/**
 * @fn void setModeMapColse3d(VOIDPtr punit, f_int32_t scenemode)
 *  @brief ������ά��ͼ�Ƿ�����
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] scenemode 0 :������, 1: ����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setModeMapColse3d(VOIDPtr punit, f_int32_t scenemode);

/**
 * @fn void setPaintSceneViewRes(VOIDPtr punit, f_float32_t viewres)
 *  @brief ���ó�������Ӧ������Ʋ����е���ʾ�ֱ���.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] viewres �ֱ��ʣ���/���أ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneViewRes(VOIDPtr punit, f_float32_t viewres);

/**
 * @fn void setPaintSceneViewAngle(VOIDPtr punit, f_float32_t viewangle)
 *  @brief ���ó�������Ӧ������Ʋ����е�ƽ��ͷ�ӽ�.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] viewangle �Ƕȣ���λ���ȣ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneViewAngle(VOIDPtr punit, f_float32_t viewangle);

/**
 * @fn void setPaintSceneCmpRd(VOIDPtr punit, f_int32_t compass_radius)
 *  @brief ���ó�������Ӧ������Ʋ����еĿ̶Ȼ��뾶.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] compass_radius �̶Ȼ��뾶�����أ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneCmpRd(VOIDPtr punit, f_int32_t compass_radius);

/**
 * @fn void setPaintSceneRotCent(VOIDPtr punit, f_int32_t xcenter, f_int32_t ycenter)
 *  @brief ���ó�������Ӧ������Ʋ����еĵ�ͼ��ת����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] xcenter��ycenter ��ͼ��ת���ĵ�λ�ã����أ�������ⲿ�Ӵ����½ǣ�.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneRotCent(VOIDPtr punit, f_int32_t xcenter, f_int32_t ycenter);

/**
 * @fn void setPaintSceneViewType(VOIDPtr punit, f_int32_t vtype)
 *  @brief �����ӽ�ģʽ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] vtype �ӽ�����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSceneViewType(VOIDPtr punit, f_int32_t vtype);


/**
 * @fn void setPaintColorAndLayermaskCfgIndex(VOIDPtr punit, f_int32_t index)
 *  @brief ����ѡ�õ���ɫ�ͷ�ӵ�����ļ�������.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] index �����ļ�����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintColorAndLayermaskCfgIndex(VOIDPtr punit, f_int32_t index);

/**
 * @fn void setPaintAreaBaDraw(VOIDPtr punit, f_int32_t bDraw)
 *  @brief ����ʸ���������Ƿ����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in]  bDraw 0-������ 1-����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintAreaBaDraw(VOIDPtr punit, f_int32_t bDraw);

/**
 * @fn void setPaintSeaMapDraw(VOIDPtr punit, f_int32_t bDraw)
 *  @brief ���ú�ͼ�Ƿ����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in]  bDraw 0-������ 1-����.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintSeaMapDraw(VOIDPtr punit, f_int32_t bDraw);

/**
 * @fn void setPaintTextDrawLevel(VOIDPtr punit, f_int32_t level)
 *  @brief ���õ���ע�Ǻͷ�����ʾ��λ.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] level 0-����ʾ�κ�ע�Ǻͷ��� 1-ֻ��ʾʡ����к��еȳ��еĵ���ע�Ǻͷ��� 2-������ʾ���е�ע�Ǻͷ���.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setPaintTextDrawLevel(VOIDPtr punit, f_int32_t level);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
