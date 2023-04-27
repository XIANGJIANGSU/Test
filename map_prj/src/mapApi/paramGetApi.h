/**
 * @file paramGetApi.h
 * @brief ���ļ��ṩ��ȡ�����������ⲿ�ӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-08-17
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _PARAM_GET_API_h_ 
#define _PARAM_GET_API_h_ 


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
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * @fn BOOL getInnerViewPort(VOIDPtr pscene, f_int32_t *view_port)
 *  @brief ��ȡ�������ڲ��ӿڲ���(���½����ꡢ��ȡ��߶�).
 *  @param[in] pscene  ���Ƴ������.
 *  @param[out] view_port  �ڲ��ӿڲ���.
 *  @exception void
 *  @return ��ȡ�ӿڲ����ɹ�����־
 * @see 
 * @note
*/
BOOL getInnerViewPort(VOIDPtr pscene, f_int32_t *view_port);

/**
 * @fn BOOL getInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *matrix)
 *  @brief ��ȡ������ģ�Ӿ����ͶӰ����.
 *  @param[in] pscene  ���Ƴ������.
 *  @param[in] matrix_mod  ��������.
 *  @param[out] matrix  ����.
 *  @exception void
 *  @return ��ȡ����ɹ�����־
 * @see 
 * @note
*/
BOOL getInnerMatrix(VOIDPtr pscene, f_int32_t matrix_mod, f_float64_t *mdviewmatrix); 

/* ��ȡ������� */
BOOL getmxWorld(VOIDPtr pscene, LPMatrix44 matrix); 
/* ��ȡ��ת���� */
BOOL getmxRotate(VOIDPtr pscene, LPMatrix44 matrix);   

/**
 * @fn BOOL getPlaneObjPos(VOIDPtr pscene, LP_PT_3D pt_pos)
 *  @brief ��ȡ�ɻ�λ��.
 *  @param[in] pscene  ���Ƴ������.
 *  @param[out] pt_pos  �ɻ�����������.
 *  @exception void
 *  @return ��ȡ�ɻ���������ɹ�����־
 * @see 
 * @note
*/
BOOL getPlaneObjPos(VOIDPtr pscene, LP_PT_3D pt_pos); 


/**
 * @fn BOOL getGeoByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_Geo_Pt_D geoPt)
 *  @brief �����ⲿ��Ļ�����ȡ��������.�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] screenPt  �ⲿ��Ļ����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoByScreenPt(VOIDPtr scene, const PT_2I screenPt, LP_Geo_Pt_D geoPt);
/**
 * @fn BOOL getScreenPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I screenPt)
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
BOOL getScreenPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I screenPt); 



/**
 * @fn BOOL getGeoByViewPt(VOIDPtr scene, const PT_2I viewPt, LP_Geo_Pt_D geoPt);
 *  @brief �����ⲿ�ӿ������ȡ��������.�ⲿ�ӿ�����->�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����->��������.
 *         �����ڲ��ӿ�����->��������Ҫ������ģʽ�����ͶӰ����.֧����ͶӰ��ī����ͶӰ.
 *  @param[in] pscene    ���Ƴ������.
 *  @param[in] viewPt    �ⲿ�ӿ�����.
 *  @param[out] geoPt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoByViewPt(VOIDPtr scene, const PT_2I viewPt, LP_Geo_Pt_D geoPt);

/**
 * @fn BOOL getViewPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I viewPt)
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
BOOL getViewPtByGeo(VOIDPtr pscene, const Geo_Pt_D geoPt, LP_PT_2I viewPt);



/**
 * @fn BOOL getViewPtByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt)
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
BOOL getViewPtByScreenPt(VOIDPtr pscene, const PT_2I screenPt, LP_PT_2I viewPt); 
/**
 * @fn BOOL getScreenPtByViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt)
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
BOOL getScreenPtByViewPt(VOIDPtr pscene, const PT_2I viewPt, LP_PT_2I screenPt);  


/**
 * @fn BOOL getDrawPtByScreenPt(VOIDPtr pscene, const PT_2I s_pt, LP_PT_2F draw_pt)
 *  @brief �����ⲿ��Ļ�����ȡ�ڲ��ӿ�����.�ⲿ��Ļ����->�ڲ���Ļ����->�ڲ��ӿ�����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] s_pt       �ⲿ��Ļ����.
 *  @param[out] draw_pt   �ڲ��ӿ�����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getDrawPtByScreenPt(VOIDPtr pscene, const PT_2I s_pt, LP_PT_2I draw_pt);  

/**
 * @fn BOOL getScreenPtByDrawPt(VOIDPtr pscene, const PT_2I draw_pt, LP_PT_2I s_pt)
 *  @brief �����ڲ��ӿ������ȡ�ⲿ��Ļ����.�ⲿ��Ļ����<-�ڲ���Ļ����<-�ڲ��ӿ�����.
 *         ��������ģʽ�����ͶӰ����,��������Ļ�����ڷŵĿ�Ȼ�߶��й�
 *  @param[in] pscene     ���Ƴ������.
 *  @param[in] draw_pt    �ڲ��ӿ�����.
 *  @param[out] s_pt      �ⲿ��Ļ����.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getScreenPtByDrawPt(VOIDPtr pscene, const PT_2I draw_pt, LP_PT_2I s_pt);


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
BOOL getDrawPtByGeoPt(const Geo_Pt_D geo_pt, LP_PT_3D draw_pt); 

/**
 * @fn BOOL getGeoPtByDrawPt(const PT_3D draw_pt, LP_Geo_Pt_D geo_pt)
 *  @brief �������������ȡ��������.
 *         ��������ģʽ�����ͶӰ����,���漰��ѧ����
 *  @param[in] draw_pt    ��������.
 *  @param[out] geo_pt    ��������.
 *  @exception void
 *  @return ��ȡ�ɹ�����־,1-�ɹ�,0-ʧ��
 * @see 
 * @note
*/
BOOL getGeoPtByDrawPt(const PT_3D draw_pt, LP_Geo_Pt_D geo_pt); 

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
