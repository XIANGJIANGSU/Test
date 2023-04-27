#include "mapApi.h"
#include "common.h"
#include "../mapApp/mapApp.h"

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
f_int32_t setViewWindowParamBegin(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setViewWindowParamPre( (VOIDPtr)(pUnit->pViewWindow)) );
}

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
f_int32_t setViewWindowParamEnd(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(sFail);

	return( setViewWindowParamPro( (VOIDPtr)(pUnit->pViewWindow)) );
}

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
f_int32_t setViewWndSize(VOIDPtr punit, f_int32_t llx, f_int32_t lly, f_int32_t width, f_int32_t height)
{
	f_int32_t ret = -1;
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return(ret);
	ret = setViewWindowSize((VOIDPtr)(pUnit->pViewWindow), (VOIDPtr)(pUnit->pScene), llx, lly, width, height);
	return(ret);
}

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
void setViewWndAlpha(VOIDPtr punit, f_float32_t alpha)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setViewWindowAlpha((VOIDPtr)(pUnit->pViewWindow), alpha);
}

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
void setViewWndBkColor(VOIDPtr punit, sColor3f bkcolor)
{
	sPAINTUNIT *pUnit = NULL;
	pUnit = unitPtrValid(punit);
	if(NULL == pUnit)
	    return;
	setViewWindowBkcolor((VOIDPtr)(pUnit->pViewWindow), bkcolor);
}

/**
 * @fn void map2dRender(VOIDPtr scene)
 *  @brief 2D��ͼ����.
 *  @param[in] pscene �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map2dRender(VOIDPtr pscene)
{
	render2dMap(pscene);
}

/**
 * @fn void map3dRender(VOIDPtr scene)
 *  @brief 3D��ͼ����.
 *  @param[in] pscene �������.
 *  @exception void
 *  @return void.
 * @see 
 * @note
*/
void map3dRender(VOIDPtr pscene)
{
	render3dMap(pscene);
}

/**
 * @fn void mapRender(VOIDPtr punit)
 *  @brief ��ͼ������Ⱦ.
 *  @param[in] punit  ���Ƶ�Ԫ���.
 *  @exception void
 *  @return void
 * @see 
 * @note
*/
void mapRender(VOIDPtr punit)
{
	sPAINTUNIT *pUnit = (sPAINTUNIT *)punit;
	if(NULL == pUnit)
	{
	    DEBUG_PRINT("NULL == punit.");
		return;
	}
	mapSceneRender((VOIDPtr)(pUnit->pViewWindow), (VOIDPtr)(pUnit->pScene));
}

