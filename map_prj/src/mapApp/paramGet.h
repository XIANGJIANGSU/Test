/**
 * @file paramGet.h
 * @brief ���ļ��ṩ��ȡ���������ĺ���
 * @author 615��ͼ�Ŷ�
 * @date 2016-08-17
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _PARAM_GET_h_ 
#define _PARAM_GET_h_ 


/*
�����еļ�������ϵ��
1. �ⲿ��Ļ����ϵ����Ļ��ת�������,ԭ��������Ļ�����Ͻ�,��������������,��������������;
2. �ڲ���Ļ����ϵ����Ļ��תǰ(��Ļ�����ڷ�)������,ԭ��������Ļ�����Ͻ�,��������������,��������������;
3. �ⲿ�ӿ�����ϵ����Ļ��ת�������,ԭ��������Ļ�����½�,��������������,��������������;
4. �ڲ��ӿ�����ϵ����Ļ��תǰ(��Ļ�����ڷ�)������,ԭ��������Ļ�����½�,��������������,��������������;
5. ��������ϵ��������ʾģʽ�¾���ָ������;
6. ��������ϵ����γ��;

����ת���ļ���˵����
1. �����ڲ���������ʱ���ǻ����ڲ��ӿ������;
   ��Ļ����ԭ�㶨������Ļ�����½ǣ�
   �ӿ�����ԭ�㶨�����ӿڵ����½ǣ�
   ��������������ʾģʽ�¾���ָ������,����ͶӰ�¿���ֱ�ӻ��ƣ�
   ����������MCT��ʾģʽ�¾���ָMCT����,��MCTͶӰ�¿���ֱ�ӻ��ƣ�
   �߼���������mct�����Ӧ�ģ���mct����ת����0��1֮�䣻
   openGL���������ӿ������Ӧ�ģ����ӿ�����ת����-1��1֮�䣻
2. ����ʱ��
   a. �����������ⲿ��Ļ����,�轫�ⲿ��Ļ����ת�����ڲ���Ļ����,�ٽ��ڲ���Ļ����ת�����ڲ��ӿ�����,Ȼ����л���;
   b. �����������ⲿ�ӿ�����,�轫�ⲿ�ӿ�����ת�����ⲿ��Ļ����,�ٽ��ⲿ��Ļ����ת�����ڲ���Ļ����,
      Ȼ���ڲ���Ļ����ת�����ڲ��ӿ�����,�����л���;
3. ��Ļδ��תʱ���ⲿ��Ļ�������ڲ���Ļ����һ�£��ⲿ�ӿ��������ڲ��ӿ�����һ��;
*/


#include "../define/mbaseType.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * @fn BOOL getSceneInnerViewPort(VOIDPtr scene, f_int32_t *view_port)
 *  @brief ��ȡ�������ڲ��ӿڲ���.
 *  @param[in] scene  �������.
 *  @param[out] view_port  �ӿڲ���.
 *  @exception void
 *  @return �ɹ�����־
 * @see 
 * @note
*/
BOOL getSceneInnerViewPort(VOIDPtr scene, f_int32_t *view_port);

/**
 * @fn BOOL getSceneInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *matrix)
 *  @brief ��ȡ������ģ�Ӿ����ͶӰ����.
 *  @param[in] pscene  ���Ƴ������.
 *  @param[in] matrix_mod  ��������.
 *  @param[out] matrix  ����.
 *  @exception void
 *  @return ��ȡ����ɹ�����־
 * @see 
 * @note
*/
BOOL getSceneInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *matrix);

/* ��ȡ������� */
BOOL getSceneMxworld(VOIDPtr pscene, LPMatrix44 matrix);
/* ��ȡ��ת���� */
BOOL getSceneMxrotate(VOIDPtr pscene, LPMatrix44 matrix);

/* ��ȡ�ɻ��������� */
BOOL getScenePlaneObjPos(VOIDPtr pscene, LP_PT_3D pt);

/**
 * @fn BOOL getOuterViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
 *  @brief �����ⲿ��Ļ�����ȡ�ⲿ�ӿ�����.�ⲿ�ӿ�����<-�ⲿ��Ļ����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] screenPt   �ⲿ��Ļ����.
 *  @param[out] viewPt    �ⲿ�ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getOuterViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt);
/**
 * @fn BOOL getOuterScreenPtByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
 *  @brief �����ⲿ�ӿ������ȡ�ⲿ��Ļ����.�ⲿ�ӿ�����->�ⲿ��Ļ����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] viewPt     �ⲿ�ӿ�����.
 *  @param[out] screenPt  �ⲿ��Ļ����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getOuterScreenPtByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt); 



/**
 * @fn BOOL getInnerViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
 *  @brief �����ⲿ��Ļ�����ȡ�ڲ��ӿ�����.�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] screenPt   �ⲿ��Ļ����.
 *  @param[out] viewPt    �ڲ��ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getInnerViewPtByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt);

/**
 * @fn BOOL getOuterScreenPtByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
 *  @brief �����ڲ��ӿ������ȡ�ⲿ��Ļ����.�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] viewPt     �ڲ��ӿ�����.
 *  @param[out] screenPt  �ⲿ��Ļ����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getOuterScreenPtByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt);

/**
 * @fn BOOL getDrawPtByGeoPt(const Geo_Pt_D geo_pt, LP_PT_3D draw_pt)
 *  @brief ���ݵ��������ȡ��������.
 *         ��������ģʽ�����ͶӰ����,���漰��ѧ����
 *  @param[in] geo_pt     ��������.
 *  @param[out] draw_pt   ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getObjPtByGeoPt(Geo_Pt_D geo_pt, LP_PT_3D obj_pt);

/**
 * @fn BOOL getGeoPtByObjPt(PT_3D obj_pt, LP_Geo_Pt_D geo_pt) 
 *  @brief �������������ȡ��������.
 *         ��������ģʽ�����ͶӰ����,���漰��ѧ����
 *  @param[in] draw_pt   ��������.
 *  @param[out] geo_pt     ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoPtByObjPt(PT_3D obj_pt, LP_Geo_Pt_D geo_pt);


/**
 * @fn BOOL getGeoByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
 *  @brief �����ⲿ��Ļ�����ȡ��������.�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ��ī����ͶӰֻ�и���ʱ��ת��.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] screenPt  �ⲿ��Ļ����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoByOuterScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt); 

/**
 * @fn BOOL getOuterScreenPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I screenPt)
 *  @brief ���ݵ��������ȡ�ⲿ��Ļ����.�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����<-��������.
 *         �����ڲ��ӿ�����<-��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] geoPt      ��������.
 *  @param[out] screenPt  �ⲿ��Ļ����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getOuterScreenPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I screenPt); 



/**
 * @fn BOOL getGeoByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
 *  @brief �����ⲿ�ӿ������ȡ��������.�ⲿ�ӿ�����->�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ��ī����ͶӰֻ�и���ʱ��ת��.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] viewPt    �ⲿ�ӿ�����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoByOuterViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt);

/**
 * @fn BOOL getOuterViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
 *  @brief ���ݵ��������ȡ�ⲿ�ӿ�����.�ⲿ�ӿ�����<-�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����<-��������.
 *         �����ڲ��ӿ�����<-��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] geoPt      ��������.
 *  @param[out] viewPt    �ⲿ�ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getOuterViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt); 




/**
 * @fn BOOL getGeoByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt)
 *  @brief �����ڲ��ӿ������ȡ��������.�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ��ī����ͶӰֻ�и���ʱ��ת��.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] viewPt    �ⲿ�ӿ�����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoByInnerViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_Geo_Pt_D geoPt);

/**
 * @fn BOOL getInnerViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt)
 *  @brief ���ݵ��������ȡ�ڲ��ӿ�����.�ڲ��ӿ�����<-��������.
 *         �����ڲ��ӿ�����<-��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] geoPt      ��������.
 *  @param[out] viewPt    �ⲿ�ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getInnerViewPtByGeo(VOIDPtr pscene, Geo_Pt_D geoPt, LP_PT_2I viewPt); 

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
