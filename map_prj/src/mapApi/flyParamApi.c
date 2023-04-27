#include "flyParamApi.h"
#include "common.h"
#include "../mapApp/mapApp.h"
#include "../mapApp/flyParam.h"

/**
 * @fn void setPeriodParamBegin(VOIDPtr punit)
 *  @brief �������ڲ�����ǰ�ú���. 
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @exception void.
 *  @return f_int32_t.
 *  @retval sFail  ʧ��.
 *  @retval sSuccess  �ɹ�.
 * @see 
 * @note
*/
f_int32_t setPeriodParamBegin(VOIDPtr punit)
{
    sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setPeriodParamPre( (VOIDPtr)(pUnit->pScene)) );
}

/**
 * @fn f_int32_t setPeriodParamEnd(VOIDPtr render_scene)
 *  @brief �������ڲ����ĺ��ú���. 
 *  @param[in] render_scene �������.
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
f_int32_t setPeriodParamEnd(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setPeriodParamPro( (VOIDPtr)(pUnit->pScene)) );
}

/**
 * @fn void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude)
 *  @brief �趨λ����Ϣ�ľ�γ��.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] longitude   ���ȣ���λ���ȣ���Χ-180~180��.
 *  @param[in] latitude    γ�ȣ���λ���ȣ���Χ-90~90��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setPositionLonlat(VOIDPtr punit, f_float64_t longitude, f_float64_t latitude)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if( (longitude < -180.0) || (longitude > 180.0) || (latitude < -90.0) || (latitude > 90.0) )
	    return;
    setPosLonLat((VOIDPtr)(pUnit->pScene), longitude, latitude);	
}

/**
 * @fn void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude)
 *  @brief ��ȡλ����Ϣ�ľ�γ��.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] longitude   ���ȣ���λ���ȣ���Χ-180~180��.
 *  @param[in] latitude    γ�ȣ���λ���ȣ���Χ-90~90��.
 *  @exception void
 *  @return void
 * @see
 * @note
*/
void getPositionLonlat(VOIDPtr punit, f_float64_t* longitude, f_float64_t* latitude)
{
    sPAINTUNIT *pUnit = NULL;
    pUnit = unitPtrValid(punit);
    if(NULL == pUnit)
        return;
    if( (longitude == NULL) || (latitude == NULL) )
        return;
    getPosLonLat((VOIDPtr)(pUnit->pScene), longitude, latitude);
}

/**
 * @fn void setPositionHeight(VOIDPtr punit, f_float64_t height)
 *  @brief �趨λ����Ϣ�ĸ߶�.
           -# �ڷǵ������ӽǵ�����£���λ�����ӵ��λ��.
           -# �������ӽǵ�����£���λ���ǽ����λ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] height   �߶ȣ���λ���ף�.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setPositionHeight(VOIDPtr punit, f_float64_t height)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setPosHeight((VOIDPtr)(pUnit->pScene), height);	
}

/**
 * @fn void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw)
 *  @brief �趨������Ϣ�ĺ����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] yaw   ����ǣ���λ���ȣ���Χ-180~180��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeYaw(VOIDPtr punit, f_float64_t yaw)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if((yaw < -180.0) || (yaw > 180.0) )
	    return;
	    
	//ICD����ĽǶ�-180��180��������Ϊ0����ʱ��Ϊ����˳ʱ��Ϊ��;
	//opengl�ĽǶ�0��360��������Ϊ0����ʱ�뷽������;
	//��ICD����ĺ����ת����opengl�ĽǶȡ�
	yaw = 360.0 - yaw;
    setAtdYaw((VOIDPtr)(pUnit->pScene), yaw);
}

/**
 * @fn void setAttitudeMag(VOIDPtr punit, f_float64_t mag)
 *  @brief �趨������Ϣ�ĴŲ��.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] mag   �Ų�ǣ���λ���ȣ���Χ-180~180��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeMag(VOIDPtr punit, f_float64_t mag)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if((mag < -180.0) || (mag > 180.0) )
	    return;
	
	//ICD����ĽǶ�-180��180��������Ϊ0����ʱ��Ϊ����˳ʱ��Ϊ��;
	//opengl�ĽǶ�0��360��������Ϊ0����ʱ�뷽������;
	//��ICD����ĴŲ��ת����opengl�ĽǶȡ�
	mag = 360.0 - mag;

    setAtdMag((VOIDPtr)(pUnit->pScene), mag);
}

/**
 * @fn void setAttitudePitch(VOIDPtr punit, f_float64_t pitch)
 *  @brief �趨������Ϣ�ĸ�����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] pitch   �����ǣ���λ���ȣ���Χ-90~90��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudePitch(VOIDPtr punit, f_float64_t pitch)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    if((pitch < -90.0) || (pitch > 90.0) )
	    return;	    
    setAtdPithch((VOIDPtr)(pUnit->pScene), pitch);
}

/**
 * @fn void setAttitudeRoll(VOIDPtr punit, f_float64_t roll)
 *  @brief �趨������Ϣ�ĺ����.
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] roll   ����ǣ���λ���ȣ���Χ-180~180��.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setAttitudeRoll(VOIDPtr punit, f_float64_t roll)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	if((roll < -180.0) || (roll > 180.0) )
	    return;
    setAtdRoll((VOIDPtr)(pUnit->pScene), roll);
}

/**
 * @fn void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
 *  @brief �趨���ε��Ӵ�����(������Ӵ����½�).
 *  @param[in] punit ���Ƶ�Ԫ���.
 *  @param[in] romx,romy   ���ε��Ӵ�����.
 *  @param[out] romflag   ���α�־.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void setRoamxy(VOIDPtr punit, f_int32_t romx, f_int32_t romy, f_int32_t *romflag)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
    setRomxy((VOIDPtr)(pUnit->pScene), romx, romy, romflag);
}
