#include "../mapApp/auxNavInfo.h"
#include "../mapApp/appHead.h"
#include "../mapApp/mapApp.h"
#include "auxNavInfoApi.h"

/**
 * @fn BOOL initAuxNavInfoData(VOIDPtr scene, f_char_t *data_path)
 *  @brief ����������Ϣ���ݳ�ʼ��.
 *  @param[in] scene �ӿ����ڵĳ������.
 *  @param[in] data_path �ӿڶ�Ӧ����·���ĸ�Ŀ¼.
 *  @exception void
 *  @return BOOL,��ʼ���ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
 * @see 
 * @note 
*/
BOOL initAuxNavInfoData(VOIDPtr scene, f_char_t *data_path)
{
	return( auxNavInfoDataInit(scene, data_path) );
}


/**
 * @fn void setAuxNavZplParam(VOIDPtr punit, sZplParam zplprarm)
 *  @brief �趨0����������ʾ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] zplprarm 0�������߲���.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAuxNavZplParam(VOIDPtr punit, sZplParam zplprarm)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setZplParam((VOIDPtr)(pUnit->pScene), zplprarm);	
}

/**
 * @fn void setAuxNavCpsParam(VOIDPtr punit, sCpsParam cpsprarm)
 *  @brief �趨������ʾ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] cpsprarm ���̲���.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAuxNavCpsParam(VOIDPtr punit, sCpsParam cpsprarm)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setCpsParam((VOIDPtr)(pUnit->pScene), cpsprarm);	
}

/**
 * @fn void setAuxNavPrefileParam(VOIDPtr punit, sPrefileParam prefileprarm)
 *  @brief �趨����ͼ��ʾ�Ĳ���.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @param[in] prefileprarm ����ͼ����.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAuxNavPrefileParam(VOIDPtr punit, sPrefileParam prefileprarm)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setPrefileParam((VOIDPtr)(pUnit->pScene), prefileprarm);	
}

/**
 * @fn void setAuxNavPrefileDisplay(VOIDPtr punit, f_int32_t isdisplay)
 *  @brief ��������ͼ�Ƿ���ʾ
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] isdisplay �Ƿ���ʾ��0:����ʾ��1:��ʾ
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAuxNavPrefileDisplay(VOIDPtr punit, f_int32_t isdisplay)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setPrefileDisplay((VOIDPtr)(pUnit->pScene), isdisplay);
}

/**
 * @fn void setAuxNavPrefileDetectDistance(VOIDPtr punit, f_float64_t hDetectDistance)
 *  @brief ��������ͼˮƽ̽�����
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] hDetectDistance ˮƽ̽�����
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAuxNavPrefileDetectDistance(VOIDPtr punit, f_float64_t hDetectDistance)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setPrefileDetectDistance((VOIDPtr)(pUnit->pScene), hDetectDistance);
}


/**
 * @fn void setAuxNavPrefileTerrainColor(VOIDPtr punit, f_int32_t color_num, f_float32_t prefile_height[5], 
	f_uint8_t prefile_color[6][4], f_int32_t s_winHeiMeter)
 *  @brief ��������ͼ��ֱ��������ͼVSD�Ĵ�ֱ�����������ʵ�ʾ���(��)���߶�ɫ����ز���
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] color_num ɫ������,��ΧΪ[2,6]
 *  @param[in] prefile_height[5] ɫ������ĸ߶�,ÿ��ɫ����Ӧһ�����䷶Χ,5��ֵ��Ӧ���6������
 *  @param[in] prefile_color[6][4] ɫ������ɫRGBA
 *  @param[in] s_winHeiMeter ��ֱ�����������ʵ�ʾ���(��)���������0
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void setAuxNavPrefileTerrainColor(VOIDPtr punit, f_int32_t color_num, f_float32_t prefile_height[5], 
	f_uint8_t prefile_color[6][4], f_int32_t s_winHeiMeter)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setPrefileTerrainColor((VOIDPtr)(pUnit->pScene),  color_num, prefile_height, prefile_color, s_winHeiMeter);
}