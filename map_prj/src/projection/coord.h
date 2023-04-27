#ifndef _HlCoord_h_ 
#define _HlCoord_h_

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

#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* �������� --> ��������,�뺯��geoDPt2objDPt������ͬ,ֻ�ǲ������ɻ�������double */
int EarthToXYZ(const f_float64_t lon,const f_float64_t lat,const f_float64_t hgt, f_float64_t *pX,f_float64_t *pY,f_float64_t *pZ);
/* �������� --> �������� */
BOOL geoDPt2objDPt(const LP_Geo_Pt_D geoPt, LP_Obj_Pt_D objPt);
/* �������� --> �������� */
BOOL objDPt2geoDPt(const LP_Obj_Pt_D objPt, LP_Geo_Pt_D geoPt);

/* �������� --> �߼����� */
BOOL geoDPt2logicDPt(const LP_Geo_Pt_D geoPt, LP_PT_2D logicPt);
/* �߼����� --> �������� */
BOOL logicDPt2geoDpt(const LP_PT_2D logicPt, LP_Geo_Pt_D geoPt);

/* �������� --> �ڲ��ӿ����� */
BOOL objDPt2InnerViewDPt(const LP_Obj_Pt_D objPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt);
/* �ڲ��ӿ����� --> �������� */
BOOL innerViewDPt2objDPt(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt);

/* �ڲ��ӿ����� --> OpenGL���� */
BOOL innerViewDPt2openGlDPt(const LP_PT_2D viewPt, const f_int32_t innerviewport[4], LP_PT_2D openGlPt);
/* OpenGL���� --> �ڲ��ӿ����� */
BOOL openGlDPt2InnerViewDPt(const LP_PT_2D openGlPt, const f_int32_t innerviewport[4], LP_PT_2D viewPt);
                   
#if 0  //20160719 ��Ǫ�� ��ʱ����Ҫ���¹��ܺ���
/* �������� --> �߼����� */
//BOOL objDPt2logicDPt();
/* �߼����� --> �������� */
//BOOL logicDPt2objDPt();
#endif

/* �ⲿ��Ļ����-->�ڲ���Ļ���� */
BOOL outerScreenPt2innerScreenPt(const PT_2I outer_pt, PT_2I *inner_pt);
/* �ڲ���Ļ����-->�ⲿ��Ļ���� */
BOOL innerScreenPt2outerScreenPt(const PT_2I inner_pt, PT_2I *outer_pt);

/* �ڲ���Ļ����-->�ڲ��ӿ����� */
BOOL innerScreenPt2innerViewPt(const f_int32_t innerviewport[4], const PT_2I inner_screen_pt, PT_2I *inner_view_pt);
/* �ڲ��ӿ�����-->�ڲ���Ļ���� */
BOOL innerViewPt2innerScreenPt(const f_int32_t innerviewport[4], const PT_2I inner_view_pt, PT_2I *inner_screen_pt);

/* �ⲿ��Ļ����-->�ⲿ�ӿ����� */
BOOL outerScreenPt2outerViewPt(const f_int32_t outerviewport[4], const PT_2I outer_screen_pt, PT_2I *outer_view_pt);
/* �ⲿ�ӿ�����-->�ⲿ��Ļ���� */
BOOL outerViewPt2outerScreenPt(const f_int32_t outerviewport[4], const PT_2I outer_view_pt, PT_2I *outer_screen_pt);





/* �������� --> ī�������� */
BOOL geoDPt2mctDPt(const LP_Geo_Pt_D geoPt, LP_PT_2D mctPt);
/* ī�������� --> �������� */
BOOL mctDPt2geoDPt(const LP_PT_2D mctPt, LP_Geo_Pt_D geoPt);

/* �߼����� --> ī�������� */
BOOL logicDPt2mctDPt(const LP_PT_2D logicPt, LP_PT_2D mctPt);
/* ī�������� --> �߼����� */
BOOL mctDPt2logicDPt(const LP_PT_2D mctPt, LP_PT_2D logicPt);

/* �������� --> ī�������� */
BOOL objDPt2mctDPt(const LP_PT_3D objPt, LP_PT_2D mctPt);
/* ī�������� --> �������� */
BOOL mctDPt2objDPt(const LP_PT_2D mctPt, const f_float64_t height, LP_PT_3D objPt);



/* �ڲ��ӿ�����(�������ֵ) --> ��������,ī����ͶӰʹ�� */
BOOL innerViewXYDPt2objDPtMct(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt);

/* �������� --> �ڲ��ӿ�����,ī����ͶӰʹ�� */
BOOL geoDPt2InnerViewDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_3D viewPt);
/* �ڲ��ӿ����� --> ��������,ī����ͶӰʹ�� */
BOOL innerViewDPt2geoDPtMct(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
	const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* �������� --> �ڲ���Ļ����,ī����ͶӰʹ�� */
BOOL geoDPt2InnerScreenDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* �ڲ���Ļ���� --> ��������,ī����ͶӰʹ�� */
BOOL innerScreenDPt2GeoDPtMct(const PT_2I screenPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt); 


/* �������� --> �ⲿ��Ļ����,ī����ͶӰʹ�� */
BOOL geoDPt2OuterScreenDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* �ⲿ��Ļ���� --> ��������,ī����ͶӰ��ʹ�� */
BOOL outerScreenDPt2GeoDPtMct(const PT_2I screenPt, const f_float64_t mdviewMatrix[16], 
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* �������� --> �ⲿ�ӿ�����,ī����ͶӰʹ�� */
BOOL geoDPt2OuterViewDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_PT_2I viewPt);     
/* �ⲿ�ӿ����� --> ��������,ī����ͶӰ��ʹ�� */
BOOL outerViewDPt2geoDPtMct(const PT_2I viewPt, const f_float64_t mdviewMatrix[16],const f_float64_t invmatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);






/* �ڲ��ӿ�����(�������ֵ) --> ��������, ��ͶӰʹ�� */
BOOL innerViewXYDPt2objDPt(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt);


/* �������� --> �ڲ��ӿ�����,��ͶӰʹ�� */
BOOL geoDPt2InnerViewDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt);
/* �ڲ��ӿ����� --> ��������,��ͶӰʹ�� */
BOOL innerViewDPt2geoDPt(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);
/* �ڲ��ӿ����� --> ��������,��ͶӰ��ʹ��,������� */
BOOL innerViewDPt2geoDPtCorrect(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* �������� --> �ڲ���Ļ����,��ͶӰʹ�� */
BOOL geoDPt2InnerScreenDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                           const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* �ڲ���Ļ���� --> ��������,��ͶӰʹ�� */
BOOL innerScreenDPt2GeoDPt(const PT_2I screenPt, const f_float64_t invmatrix[16], 
                           const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt); 
/* �ڲ���Ļ���� --> ��������,��ͶӰ��ʹ��,������� */
BOOL innerScreenDPt2GeoDPtCorrect(const PT_2I screenPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* �������� --> �ⲿ��Ļ����,��ͶӰʹ�� */
BOOL geoDPt2OuterScreenDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                           const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* �ⲿ��Ļ���� --> ��������,��ͶӰʹ�� */
BOOL outerScreenDPt2GeoDPt(const PT_2I screenPt, const f_float64_t invmatrix[16], 
                           const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);  
/* �ⲿ��Ļ���� --> ��������,��ͶӰ��ʹ��,������� */
BOOL outerScreenDPt2GeoDPtCorrect(const PT_2I screenPt, const f_float64_t mdviewMatrix[16], 
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* �������� --> �ⲿ�ӿ�����,��ͶӰʹ�� */
BOOL geoDPt2OuterViewDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_PT_2I viewPt); 
/* �ⲿ�ӿ����� --> ��������,��ͶӰʹ�� */
BOOL outerViewDPt2geoDPt(const PT_2I viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);    
/* �ⲿ�ӿ����� --> ��������,��ͶӰ��ʹ��,������� */
BOOL outerViewDPt2geoDPtCorrect(const PT_2I viewPt, const f_float64_t mdviewMatrix[16],const f_float64_t invmatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
