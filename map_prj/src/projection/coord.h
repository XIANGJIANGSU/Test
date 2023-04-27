#ifndef _HlCoord_h_ 
#define _HlCoord_h_

/*
引擎中的几个坐标系：
1. 外部屏幕坐标系：屏幕旋转后的坐标,原点是在屏幕的左上角,横坐标向左增大,纵坐标向下增大;
2. 内部屏幕坐标系：屏幕旋转前(屏幕正常摆放)的坐标,原点是在屏幕的左上角,横坐标向左增大,纵坐标向下增大;
3. 外部视口坐标系：屏幕旋转后的坐标,原点是在屏幕的左下角,横坐标向左增大,纵坐标向上增大;
4. 内部视口坐标系：屏幕旋转前(屏幕正常摆放)的坐标,原点是在屏幕的左下角,横坐标向左增大,纵坐标向上增大;
5. 物体坐标系：在球显示模式下就是指球坐标;
6. 地理坐标系：经纬高;

坐标转换的几点说明：
1. 引擎内部场景绘制时都是基于内部视口坐标的;
   屏幕坐标原点定义在屏幕的左下角；
   视口坐标原点定义在视口的左下角；
   物体坐标在球显示模式下就是指球坐标,在球投影下可以直接绘制；
   物体坐标在MCT显示模式下就是指MCT坐标,在MCT投影下可以直接绘制；
   逻辑坐标是与mct坐标对应的，将mct坐标转换到0～1之间；
   openGL坐标是与视口坐标对应的，将视口坐标转换到-1～1之间；
2. 绘制时：
   a. 如果输入的是外部屏幕坐标,需将外部屏幕坐标转换成内部屏幕坐标,再将内部屏幕坐标转换成内部视口坐标,然后进行绘制;
   b. 如果输入的是外部视口坐标,需将外部视口坐标转换成外部屏幕坐标,再将外部屏幕坐标转换成内部屏幕坐标,
      然后将内部屏幕坐标转换成内部视口坐标,最后进行绘制;
3. 屏幕未旋转时，外部屏幕坐标与内部屏幕坐标一致，外部视口坐标与内部视口坐标一致;
*/

#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* 地理坐标 --> 物体坐标,与函数geoDPt2objDPt功能相同,只是参数换成基本类型double */
int EarthToXYZ(const f_float64_t lon,const f_float64_t lat,const f_float64_t hgt, f_float64_t *pX,f_float64_t *pY,f_float64_t *pZ);
/* 地理坐标 --> 物体坐标 */
BOOL geoDPt2objDPt(const LP_Geo_Pt_D geoPt, LP_Obj_Pt_D objPt);
/* 物体坐标 --> 地理坐标 */
BOOL objDPt2geoDPt(const LP_Obj_Pt_D objPt, LP_Geo_Pt_D geoPt);

/* 地理坐标 --> 逻辑坐标 */
BOOL geoDPt2logicDPt(const LP_Geo_Pt_D geoPt, LP_PT_2D logicPt);
/* 逻辑坐标 --> 地理坐标 */
BOOL logicDPt2geoDpt(const LP_PT_2D logicPt, LP_Geo_Pt_D geoPt);

/* 物体坐标 --> 内部视口坐标 */
BOOL objDPt2InnerViewDPt(const LP_Obj_Pt_D objPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt);
/* 内部视口坐标 --> 物体坐标 */
BOOL innerViewDPt2objDPt(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt);

/* 内部视口坐标 --> OpenGL坐标 */
BOOL innerViewDPt2openGlDPt(const LP_PT_2D viewPt, const f_int32_t innerviewport[4], LP_PT_2D openGlPt);
/* OpenGL坐标 --> 内部视口坐标 */
BOOL openGlDPt2InnerViewDPt(const LP_PT_2D openGlPt, const f_int32_t innerviewport[4], LP_PT_2D viewPt);
                   
#if 0  //20160719 张仟新 暂时不需要以下功能函数
/* 物体坐标 --> 逻辑坐标 */
//BOOL objDPt2logicDPt();
/* 逻辑坐标 --> 物体坐标 */
//BOOL logicDPt2objDPt();
#endif

/* 外部屏幕坐标-->内部屏幕坐标 */
BOOL outerScreenPt2innerScreenPt(const PT_2I outer_pt, PT_2I *inner_pt);
/* 内部屏幕坐标-->外部屏幕坐标 */
BOOL innerScreenPt2outerScreenPt(const PT_2I inner_pt, PT_2I *outer_pt);

/* 内部屏幕坐标-->内部视口坐标 */
BOOL innerScreenPt2innerViewPt(const f_int32_t innerviewport[4], const PT_2I inner_screen_pt, PT_2I *inner_view_pt);
/* 内部视口坐标-->内部屏幕坐标 */
BOOL innerViewPt2innerScreenPt(const f_int32_t innerviewport[4], const PT_2I inner_view_pt, PT_2I *inner_screen_pt);

/* 外部屏幕坐标-->外部视口坐标 */
BOOL outerScreenPt2outerViewPt(const f_int32_t outerviewport[4], const PT_2I outer_screen_pt, PT_2I *outer_view_pt);
/* 外部视口坐标-->外部屏幕坐标 */
BOOL outerViewPt2outerScreenPt(const f_int32_t outerviewport[4], const PT_2I outer_view_pt, PT_2I *outer_screen_pt);





/* 地理坐标 --> 墨卡托坐标 */
BOOL geoDPt2mctDPt(const LP_Geo_Pt_D geoPt, LP_PT_2D mctPt);
/* 墨卡托坐标 --> 地理坐标 */
BOOL mctDPt2geoDPt(const LP_PT_2D mctPt, LP_Geo_Pt_D geoPt);

/* 逻辑坐标 --> 墨卡托坐标 */
BOOL logicDPt2mctDPt(const LP_PT_2D logicPt, LP_PT_2D mctPt);
/* 墨卡托坐标 --> 逻辑坐标 */
BOOL mctDPt2logicDPt(const LP_PT_2D mctPt, LP_PT_2D logicPt);

/* 物体坐标 --> 墨卡托坐标 */
BOOL objDPt2mctDPt(const LP_PT_3D objPt, LP_PT_2D mctPt);
/* 墨卡托坐标 --> 物体坐标 */
BOOL mctDPt2objDPt(const LP_PT_2D mctPt, const f_float64_t height, LP_PT_3D objPt);



/* 内部视口坐标(不含深度值) --> 物体坐标,墨卡托投影使用 */
BOOL innerViewXYDPt2objDPtMct(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt);

/* 地理坐标 --> 内部视口坐标,墨卡托投影使用 */
BOOL geoDPt2InnerViewDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_3D viewPt);
/* 内部视口坐标 --> 地理坐标,墨卡托投影使用 */
BOOL innerViewDPt2geoDPtMct(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
	const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* 地理坐标 --> 内部屏幕坐标,墨卡托投影使用 */
BOOL geoDPt2InnerScreenDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* 内部屏幕坐标 --> 地理坐标,墨卡托投影使用 */
BOOL innerScreenDPt2GeoDPtMct(const PT_2I screenPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt); 


/* 地理坐标 --> 外部屏幕坐标,墨卡托投影使用 */
BOOL geoDPt2OuterScreenDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* 外部屏幕坐标 --> 地理坐标,墨卡托投影下使用 */
BOOL outerScreenDPt2GeoDPtMct(const PT_2I screenPt, const f_float64_t mdviewMatrix[16], 
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* 地理坐标 --> 外部视口坐标,墨卡托投影使用 */
BOOL geoDPt2OuterViewDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_PT_2I viewPt);     
/* 外部视口坐标 --> 地理坐标,墨卡托投影下使用 */
BOOL outerViewDPt2geoDPtMct(const PT_2I viewPt, const f_float64_t mdviewMatrix[16],const f_float64_t invmatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);






/* 内部视口坐标(不含深度值) --> 物体坐标, 球投影使用 */
BOOL innerViewXYDPt2objDPt(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt);


/* 地理坐标 --> 内部视口坐标,球投影使用 */
BOOL geoDPt2InnerViewDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt);
/* 内部视口坐标 --> 地理坐标,球投影使用 */
BOOL innerViewDPt2geoDPt(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);
/* 内部视口坐标 --> 地理坐标,球投影下使用,纠正误差 */
BOOL innerViewDPt2geoDPtCorrect(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* 地理坐标 --> 内部屏幕坐标,球投影使用 */
BOOL geoDPt2InnerScreenDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                           const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* 内部屏幕坐标 --> 地理坐标,球投影使用 */
BOOL innerScreenDPt2GeoDPt(const PT_2I screenPt, const f_float64_t invmatrix[16], 
                           const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt); 
/* 内部屏幕坐标 --> 地理坐标,球投影下使用,修正误差 */
BOOL innerScreenDPt2GeoDPtCorrect(const PT_2I screenPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* 地理坐标 --> 外部屏幕坐标,球投影使用 */
BOOL geoDPt2OuterScreenDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                           const f_int32_t innerviewport[4], LP_PT_2I screenPt);
/* 外部屏幕坐标 --> 地理坐标,球投影使用 */
BOOL outerScreenDPt2GeoDPt(const PT_2I screenPt, const f_float64_t invmatrix[16], 
                           const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);  
/* 外部屏幕坐标 --> 地理坐标,球投影下使用,修正误差 */
BOOL outerScreenDPt2GeoDPtCorrect(const PT_2I screenPt, const f_float64_t mdviewMatrix[16], 
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);


/* 地理坐标 --> 外部视口坐标,球投影使用 */
BOOL geoDPt2OuterViewDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_PT_2I viewPt); 
/* 外部视口坐标 --> 地理坐标,球投影使用 */
BOOL outerViewDPt2geoDPt(const PT_2I viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);    
/* 外部视口坐标 --> 地理坐标,球投影下使用,纠正误差 */
BOOL outerViewDPt2geoDPtCorrect(const PT_2I viewPt, const f_float64_t mdviewMatrix[16],const f_float64_t invmatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
