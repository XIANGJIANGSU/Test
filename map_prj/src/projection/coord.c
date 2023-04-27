#include <math.h>
#include "../define/macrodefine.h"
#include "../mapApp/appHead.h"
#include "../mapApi/common.h"
#include "project.h"
#include "mct.h"
#include "coord.h"
#include "../engine/mapRender.h"
#include "../../../vecMapEngine_prj/src/coord/2dcoord.h"



extern sSCREENPARAM screen_param;
extern WDSINZE_2I   window_size;

BOOL InScreenTileAll(sGLRENDERSCENE *pScene,const sQTMAPNODE  *pTile, 
					 const f_float64_t x, const f_float64_t y, 
 	LP_PT_3D out0, LP_PT_3D out1, LP_PT_3D out2);

static BOOL PtInMapPolygon(LP_PT_3D pPolyPts, f_uint16_t poly_pt_num, LP_PT_2D pPtPos);

extern void GetNormalOverlook(const f_float64_t x0, const f_float64_t y0, const f_float64_t z0, 
							  const f_float64_t x1, const f_float64_t y1, const f_float64_t z1, double* pNormal);

extern void Ray2TrangleIntersect(PT_3D n, double d, PT_3D p0, PT_3D d0, double *t);
extern BOOL drawGeoUserLayerPre(VOIDPtr pscene);
extern void drawUserLayerPro(void);
extern PT_3F ComputeBoundingSphereRitter(LP_PT_3F vertices, unsigned int vertex_num, f_float64_t * radiu_out);



//screen ��Ļ����������Ļ���Ͻ�Ϊԭ��
//view    �ӿ����������ӿ����½�Ϊԭ��




/* �������� --> ��������(������) */
BOOL geoDPt2objDPt(const LP_Geo_Pt_D geoPt, LP_Obj_Pt_D objPt)
{
	double lon, lat, height, lon_cos, lon_sin, lat_cos, lat_sin;
    if(NULL == geoPt || NULL == objPt)
		return(FALSE);
    
	lon = geoPt->lon * DE2RA;
	lat = geoPt->lat * DE2RA;
	height = geoPt->height + EARTH_RADIUS;
	lon_cos = cos(lon);
	lon_sin = sin(lon);
	lat_cos = cos(lat);
	lat_sin = sin(lat);	
	
	objPt->x = height * lat_cos * lon_cos;
	objPt->y = height * lat_cos * lon_sin;
	objPt->z = height * lat_sin;
	return(TRUE);
}

/* �������� --> ��������,�뺯��geoDPt2objDPt������ͬ,ֻ�ǲ������ɻ�������double */
int EarthToXYZ(const f_float64_t lon,const f_float64_t lat,const f_float64_t hgt, f_float64_t *pX,f_float64_t *pY,f_float64_t *pZ)
{
	Geo_Pt_D geoPt;
	Obj_Pt_D objPt ;

	geoPt.lon = lon;
	geoPt.lat = lat;
	geoPt.height = hgt;

	if(TRUE ==  geoDPt2objDPt(&geoPt, &objPt))
	{
		*pX = objPt.x;
		*pY = objPt.y;
		*pZ = objPt.z;

		return TRUE;
		
	}

	return FALSE;

	
}


/* �������� --> �������� */
BOOL objDPt2geoDPt(const LP_Obj_Pt_D objPt, LP_Geo_Pt_D geoPt)
{
	double r, tlat, tlon;

	if(NULL == geoPt || NULL == objPt)
		return(FALSE);
	r = sqrt(objPt->x * objPt->x + objPt->y * objPt->y + objPt->z * objPt->z);
	if(r < FLT_EPSILON)
	{
		DEBUG_PRINT("There is no geo value of the screen point.");
		return(FALSE);
	}
	else
		tlat = asin(objPt->z / r);
	
	tlon = atan2(objPt->y, objPt->x);
	geoPt->lon = tlon * RA2DE;
	geoPt->lat = tlat * RA2DE;
	geoPt->height  = r - EARTH_RADIUS;	
	return(TRUE);
}


/* �������� --> �ڲ��ӿ�����,ī����ͶӰ��ʹ�� */
BOOL geoDPt2InnerViewDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt)
{
	PT_2D mctPt;
	Obj_Pt_D mctObjPt ;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);

	if(!geoDPt2mctDPt(geoPt, &mctPt))
	    return(FALSE);

	mctObjPt.x = mctPt.x;
	mctObjPt.y = mctPt.y;
	mctObjPt.z = 0.0;

	if(!objDPt2InnerViewDPt(&mctObjPt, mdviewMatrix, innerviewport, viewPt))
	    return(FALSE);
	    	    
	return(TRUE);
}

/* �ڲ��ӿ����� --> ��������,ī����ͶӰ��ʹ�� */
BOOL innerViewDPt2geoDPtMct(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	Obj_Pt_D objPt;
	PT_2D mctPt;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);
    if(!innerViewDPt2objDPt(viewPt, invmatrix, innerviewport, &objPt))
	    return(FALSE);
	mctPt.x = objPt.x;
	mctPt.y = objPt.y;
	if(!mctDPt2geoDPt(&mctPt, geoPt))
	    return(FALSE);
	    	    
	return(TRUE);		
}


/* �������� --> �ڲ���Ļ����,ī����ͶӰ��ʹ�� */
BOOL geoDPt2InnerScreenDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_2I screenPt)
{
	PT_3D viewPt;
	PT_2I inner_view_pt;
	if(NULL == geoPt || NULL == screenPt)
		return(FALSE);
	if(!geoDPt2InnerViewDPtMct(geoPt, mdviewMatrix, innerviewport, &viewPt))
		return(FALSE);
	inner_view_pt.x = (f_int32_t)viewPt.x;
	inner_view_pt.y = (f_int32_t)viewPt.y;
	if(!innerViewPt2innerScreenPt(innerviewport, inner_view_pt, screenPt))   
		return(FALSE); 

	return(TRUE);
}

/* �ڲ���Ļ���� --> ��������,ī����ͶӰ��ʹ�� */
BOOL innerScreenDPt2GeoDPtMct(const PT_2I screenPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
#if 1
	PT_2D viewPt;
	Obj_Pt_D objPt;
	PT_2I inner_view_pt;
	PT_2D mctPt;

	if(NULL == geoPt)
		return(FALSE);
	if(!innerScreenPt2innerViewPt(innerviewport, screenPt, &inner_view_pt))
		return(FALSE);

	viewPt.x = inner_view_pt.x;
	viewPt.y = inner_view_pt.y;

	if(!innerViewXYDPt2objDPtMct(&viewPt, mdviewMatrix, invmatrix, innerviewport, &objPt))
		return(FALSE);

	mctPt.x = objPt.x;
	mctPt.y = objPt.y;
	if(!mctDPt2geoDPt(&mctPt, geoPt))
		return(FALSE);

	return(TRUE);
#else

	PT_3D viewPt;
	PT_2I inner_view_pt;
	f_float32_t view_winz = 0.0f;

	if(NULL == geoPt)
		return(FALSE);
	if(!innerScreenPt2innerViewPt(innerviewport, screenPt, &inner_view_pt))
		return(FALSE);

	/*���Ӵ�ʸ����ͼģʽ�£�û�и߶ȣ���ʱ��ȡ�������ֵ��׼ȷ(Ĭ��Ϊ0)��ת�������겻��ȷ��С��������Ϊ����*/
	/*����Ӱ��ȵ�ͼģʽ�£���Ƭ���Ƶĸ߶�Ϊ0����δ�������Ǹ�������ʱ��ȡ�������ֵ��׼ȷ(����ֵ)��ת�������겻׼ȷ*/
	glReadPixels(inner_view_pt.x + innerviewport[0], inner_view_pt.y + innerviewport[1], 
		1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &view_winz);
	if(fabs(view_winz - 1.0) < FLT_EPSILON)
		view_winz = 0.0f;

	viewPt.x = inner_view_pt.x;
	viewPt.y = inner_view_pt.y;
	viewPt.z = view_winz;

	if(!innerViewDPt2geoDPtMct(&viewPt, invmatrix, innerviewport, geoPt))
		return(FALSE);

	return(TRUE);
#endif
}


/* �������� --> �ⲿ��Ļ����,ī����ͶӰ��ʹ�� */
BOOL geoDPt2OuterScreenDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t innerviewport[4], LP_PT_2I screenPt)
{
	PT_2I inner_screenPt;
	if(NULL == geoPt || NULL == screenPt)
		return(FALSE);
	if(!geoDPt2InnerScreenDPtMct(geoPt, mdviewMatrix, innerviewport, &inner_screenPt))
		return(FALSE);
	if(!innerScreenPt2outerScreenPt(inner_screenPt, screenPt))
		return(FALSE);

	return(TRUE);
}

/* �ⲿ��Ļ���� --> ��������,ī����ͶӰ��ʹ�� */
BOOL outerScreenDPt2GeoDPtMct(const PT_2I screenPt, const f_float64_t mdviewMatrix[16], 
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I inner_screen_pt;	
	if(NULL == geoPt)
		return(FALSE);
	if(!outerScreenPt2innerScreenPt(screenPt, &inner_screen_pt))
		return(FALSE);	    
	if(!innerScreenDPt2GeoDPtMct(inner_screen_pt, mdviewMatrix, invmatrix, innerviewport, geoPt))
		return(FALSE);	

	return(TRUE);
}


/* �������� --> �ⲿ�ӿ�����,ī����ͶӰ��ʹ�� */
BOOL geoDPt2OuterViewDPtMct(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_PT_2I viewPt)
{
	PT_2I outer_screenPt;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);
	if(!geoDPt2OuterScreenDPtMct(geoPt, mdviewMatrix, outerviewport, &outer_screenPt))
		return(FALSE);
	if(!outerScreenPt2outerViewPt(innerviewport, outer_screenPt, viewPt))        
		return(FALSE);

	return(TRUE);
}

/* �ⲿ�ӿ����� --> ��������,ī����ͶӰ��ʹ�� */
BOOL outerViewDPt2geoDPtMct(const PT_2I viewPt, const f_float64_t mdviewMatrix[16],const f_float64_t invmatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I outer_screen_pt;
	if(NULL == geoPt)
		return(FALSE);
	if(!outerViewPt2outerScreenPt(outerviewport, viewPt, &outer_screen_pt))
		return(FALSE);			
	if(!outerScreenDPt2GeoDPtMct(outer_screen_pt, mdviewMatrix, invmatrix, innerviewport, geoPt))
		return(FALSE);

	return(TRUE);		
}

/* �������� --> �߼����� */
BOOL geoDPt2logicDPt(const LP_Geo_Pt_D geoPt, LP_PT_2D logicPt)
{
	PT_2D mctPt;
	if(NULL == geoPt || NULL == logicPt)
		return(FALSE);
		
	if(!geoDPt2mctDPt(geoPt, &mctPt))
	    return(FALSE);
    if(!mctDPt2logicDPt(&mctPt, logicPt))
        return(FALSE);	 
        
    return(TRUE);   
}

/* �߼����� --> �������� */
BOOL logicDPt2geoDpt(const LP_PT_2D logicPt, LP_Geo_Pt_D geoPt)
{
	PT_2D mctPt;
	if(NULL == logicPt || NULL == geoPt)
		return(FALSE);
		
    if(!logicDPt2mctDPt(logicPt, &mctPt))
        return(FALSE);
    if(!mctDPt2geoDPt(&mctPt, geoPt))
        return(FALSE);	 
        
    return(TRUE);        
}

/* �������� --> ī�������� */
BOOL geoDPt2mctDPt(const LP_Geo_Pt_D geoPt, LP_PT_2D mctPt)
{
	f_float64_t lon, lat;
	if(NULL == geoPt || NULL == mctPt)
		return(FALSE);
		
    lon = geoPt->lon * DE2RA;
    lat = geoPt->lat * DE2RA;
	if(0 != Convert_Geodetic_To_Mercator(lat, lon, &mctPt->x, &mctPt->y))
        return(FALSE);
	
	return(TRUE);
}

/* ī�������� --> �������� */
BOOL mctDPt2geoDPt(const LP_PT_2D mctPt, LP_Geo_Pt_D geoPt)
{
	f_float64_t lon, lat;
	if(NULL == mctPt || NULL == geoPt)
		return(FALSE);
		
    if(0 != Convert_Mercator_To_Geodetic(mctPt->x, mctPt->y, &lat, &lon))
        return(FALSE);
        
    geoPt->lon = lon * RA2DE;
    geoPt->lat = lat * RA2DE;
    geoPt->height = 0.0;
    
    return(TRUE);
}

/* �߼�����(0~1.0) --> ī�������� */
BOOL logicDPt2mctDPt(const LP_PT_2D logicPt, LP_PT_2D mctPt)
{
	f_float64_t mct_x, mct_y;
	if(NULL == logicPt || NULL == mctPt)
		return(FALSE);
	
	mct_x  = logicPt->x - 0.5;
	mct_x *= 2.0;
	mct_x *= MAPLOGSIZE;
	mct_y  = logicPt->y - 0.5;
	mct_y *= 2.0;
	mct_y *= MAPLOGSIZE;
	
	mctPt->y = mct_y;
	mctPt->x = mct_x;
	return(TRUE);
}

/* ī�������� --> �߼�����(0~1.0) */
BOOL mctDPt2logicDPt(const LP_PT_2D mctPt, LP_PT_2D logicPt)
{
	f_float64_t logic_x, logic_y;
	if(NULL == mctPt || NULL == logicPt)
		return(FALSE);
		
	logic_x = mctPt->x / MAPLOGSIZE;
	logic_x /= 2.0;
	logic_x += 0.5;
	logic_y = mctPt->y / MAPLOGSIZE;
	logic_y /= 2.0;
	logic_y += 0.5;
	
	logicPt->y = logic_y;
	logicPt->x = logic_x;
	return(TRUE);
}

/* �������� --> ī�������� */
BOOL objDPt2mctDPt(const LP_PT_3D objPt, LP_PT_2D mctPt)
{
	Geo_Pt_D geoPt;
	if(NULL == objPt || NULL == mctPt)
		return(FALSE);
		
    if(!objDPt2geoDPt(objPt, &geoPt))
        return(FALSE);        
    if(!geoDPt2mctDPt(&geoPt, mctPt))
        return(FALSE);
        
    return(TRUE);
}

/* ī�������� --> �������� */
BOOL mctDPt2objDPt(const LP_PT_2D mctPt, const f_float64_t height, LP_PT_3D objPt)
{
	Geo_Pt_D geoPt;
	if(NULL == mctPt || NULL == objPt)
		return(FALSE);
		
	if(!mctDPt2geoDPt(mctPt, &geoPt))
	    return(FALSE);
	geoPt.height = height;
	
	if(!geoDPt2objDPt(&geoPt, objPt))
	    return(FALSE);
	    
	return(TRUE);
}

/*
1. �ڲ�����Ļ�������ڷŵ�״̬,�ⲿ��Ļ����ת���״̬,�ڲ��ӿں��ⲿ�ӿڶ��������ڷŵ�;
2. �ⲿ��Ļ���������ԭ��������Ļ��ת������Ͻ�,��������������,��������������;
3. �ڲ���Ļ���������Ļ��תǰ(��Ļ�����ڷ�)������,ԭ��������Ļ�����Ͻ�,��������������,��������������;
4. ��������ʱ���ǻ����ڲ��ӿ�����ģ�
5. �ӿ�����ԭ�����������Ļ���½ǵ�,������ⲿ�����ӿڲ���ʱ,��Ѳ���ת�����ڲ��������У�
6. ����ʱ��
   a. �����������ⲿ��Ļ����,�轫�ⲿ��Ļ����ת�����ڲ���Ļ����,�ٽ��ڲ���Ļ����ת�����ڲ��ӿ�����,Ȼ����л���;
   b. �����������ⲿ�ӿ�����,�轫�ⲿ�ӿ�����ת�����ⲿ��Ļ����,�ٽ��ⲿ��Ļ����ת�����ڲ���Ļ����,
      Ȼ���ڲ���Ļ����ת�����ڲ��ӿ�����,�����л���;
*/

/* �ⲿ��Ļ����-->�ڲ���Ļ���� */
BOOL outerScreenPt2innerScreenPt(const PT_2I outer_pt, PT_2I *inner_pt)
{
	if(NULL == inner_pt)
	    return FALSE;

	switch(screen_param.rotate_type)
	{   
	    default:
	    case eROTATE_CW_0:
            inner_pt->x = outer_pt.x;
            inner_pt->y = outer_pt.y;
	    break;
	    case eROTATE_CW_90: 
			inner_pt->x = outer_pt.y;
            inner_pt->y = window_size.height - outer_pt.x;
	    break;
	    case eROTATE_CW_180:
            inner_pt->x = window_size.width - outer_pt.x;
            inner_pt->y = window_size.height - outer_pt.y;
	    break;
	    case eROTATE_CW_270:
            inner_pt->x = window_size.width - outer_pt.y;
            inner_pt->y = outer_pt.x;
	    break;
    }

    return TRUE;
}

/* �ڲ���Ļ����-->�ⲿ��Ļ���� */
BOOL innerScreenPt2outerScreenPt(const PT_2I inner_pt, PT_2I *outer_pt)
{
	if(NULL == outer_pt)
	    return FALSE;
     
	switch(screen_param.rotate_type)
	{   
	    default:
	    case eROTATE_CW_0:
            outer_pt->x = inner_pt.x;
            outer_pt->y = inner_pt.y;
	    break;
	    case eROTATE_CW_90: 
			outer_pt->x = window_size.height - inner_pt.y;
            outer_pt->y = inner_pt.x;
	    break;
	    case eROTATE_CW_180:
            outer_pt->x = window_size.width - inner_pt.x;
            outer_pt->y = window_size.height - inner_pt.y;
	    break;
	    case eROTATE_CW_270:
            outer_pt->x = inner_pt.y;
            outer_pt->y = window_size.width - inner_pt.x;
	    break;
    }

    return TRUE;
}

/* �ڲ���Ļ����-->�ڲ��ӿ����� */
BOOL innerScreenPt2innerViewPt(const f_int32_t innerviewport[4], const PT_2I inner_screen_pt, PT_2I *inner_view_pt)
{
	if(NULL == inner_view_pt)
	    return FALSE;
	       
	inner_view_pt->x = inner_screen_pt.x - innerviewport[0];
	inner_view_pt->y = window_size.height - inner_screen_pt.y - innerviewport[1];
	return TRUE;
}

/* �ڲ��ӿ�����-->�ڲ���Ļ���� */
BOOL innerViewPt2innerScreenPt(const f_int32_t innerviewport[4], const PT_2I inner_view_pt, PT_2I *inner_screen_pt)
{
	if(NULL == inner_screen_pt)
	    return FALSE;
	
	inner_screen_pt->x = inner_view_pt.x + innerviewport[0];
	inner_screen_pt->y = window_size.height - (inner_view_pt.y + innerviewport[1]);	
	return TRUE;
}

/* �ⲿ��Ļ����-->�ⲿ�ӿ����� */
BOOL outerScreenPt2outerViewPt(const f_int32_t outerviewport[4], const PT_2I outer_screen_pt, PT_2I *outer_view_pt)
{
	f_int32_t w_height = 0;
	if(NULL == outer_view_pt)
	    return FALSE;
	    
	switch(screen_param.rotate_type)
	{   
	    default:
	    case eROTATE_CW_0:
	    case eROTATE_CW_180:
            w_height = window_size.height;
	    break;
	    case eROTATE_CW_90: 
	    case eROTATE_CW_270:
            w_height = window_size.width;
	    break;
    }
	    
	outer_view_pt->x = outer_screen_pt.x - outerviewport[0];
	outer_view_pt->y = w_height - outer_screen_pt.y - outerviewport[1];
	return TRUE;
}

/* �ⲿ�ӿ�����-->�ⲿ��Ļ���� */
BOOL outerViewPt2outerScreenPt(const f_int32_t outerviewport[4], const PT_2I outer_view_pt, PT_2I *outer_screen_pt)
{
	f_int32_t w_height = 0;
	if(NULL == outer_screen_pt)
	    return FALSE;
	    
	switch(screen_param.rotate_type)
	{   
	    default:
	    case eROTATE_CW_0:
	    case eROTATE_CW_180:
            w_height = window_size.height;
	    break;
	    case eROTATE_CW_90: 
	    case eROTATE_CW_270:
            w_height = window_size.width;
	    break;
    }
	    
	outer_screen_pt->x = outer_view_pt.x + outerviewport[0];
	outer_screen_pt->y = w_height - (outer_view_pt.y + outerviewport[1]);	
	return TRUE;
}

/* �������� --> �ڲ��ӿ����� */
BOOL objDPt2InnerViewDPt(const LP_Obj_Pt_D objPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt)
{
	f_float64_t viewx, viewy, viewz;
	if(NULL == objPt || NULL == viewPt)
		return(FALSE);
		
	if(!avicglProject(objPt->x, objPt->y, objPt->z, mdviewMatrix, innerviewport, &viewx, &viewy, &viewz))
	    return(FALSE);
	    
	viewPt->x = viewx;
	viewPt->y = viewy;
	viewPt->z = viewz;    
	return(TRUE); 
}

/* �ڲ��ӿ�����(�����ֵ) --> �������� */
BOOL innerViewDPt2objDPt(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt)
{
	f_float64_t objx, objy, objz;
	if(NULL == objPt || NULL == viewPt)
		return(FALSE);
		
	if(!avicglUnProject(viewPt->x, viewPt->y, viewPt->z, invmatrix, innerviewport, &objx, &objy, &objz))
	    return(FALSE);
	    
	objPt->x = objx;
	objPt->y = objy;
	objPt->z = objz;    
	return(TRUE); 
}


/* �ڲ��ӿ�����(�������ֵ) --> ��������,��ͶӰ��ʹ�� */
BOOL innerViewXYDPt2objDPt(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt)
{
	/* �м�ת��ʹ�õ��������� */
	f_float64_t obj_x0, obj_y0, obj_z0, obj_x1, obj_y1, obj_z1;
	/* ������������� */
	sPOINT3D point, point0, point1;
	f_float64_t z0 = 0.0f, z1 = 0.0f;
	int ret;

	if(NULL == objPt || NULL == viewPt)
		return(FALSE);
	/*�������Ϊ0.01���ڲ��ӿ������Ӧ����������*/
	if(!avicglUnProject(viewPt->x, viewPt->y, 0.01, invmatrix, innerviewport, &obj_x0, &obj_y0, &obj_z0))
		return(FALSE);
	/*�������Ϊ0.99998���ڲ��ӿ������Ӧ����������*/
	if(!avicglUnProject(viewPt->x, viewPt->y, 0.99999, invmatrix, innerviewport, &obj_x1, &obj_y1, &obj_z1))
		return(FALSE);
	/*����2�������������ɵ�ֱ��������Ľ���*/
	ret = getIntersectionPointOfLineAndSphere(obj_x0, obj_y0, obj_z0, obj_x1, obj_y1, obj_z1, EARTH_RADIUS, &point0, &point1);
	if(-1 != ret)
	{
		/*ֱ������Ӧ����2�����㣬ȡ��������Ǹ����㣬��z����Խ���ֵ(Ϊʲô����)*/
		avicgetViewcoordZ(mdviewMatrix, point0.x, point0.y, point0.z, &z0);
		avicgetViewcoordZ(mdviewMatrix, point1.x, point1.y, point1.z, &z1);
		if(z0 > z1)
			point = point0;
		else
			point = point1;

		objPt->x = point.x;
		objPt->y = point.y;
		objPt->z = point.z; 
	}else{
		return(FALSE);
	}
	return(TRUE);
}


/* �ڲ��ӿ�����(�������ֵ) --> ��������,ī����ͶӰ��ʹ�� */
BOOL innerViewXYDPt2objDPtMct(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Obj_Pt_D objPt)
{
	/* �м�ת��ʹ�õ��������� */
	f_float64_t obj_x0, obj_y0, obj_z0, obj_x1, obj_y1, obj_z1;
	/* ������������� */
	sPOINT3D point;
	f_float64_t z0 = 0.0f, z1 = 0.0f;
	int ret;

	if(NULL == objPt || NULL == viewPt)
		return(FALSE);
	/*�������Ϊ0.01���ڲ��ӿ������Ӧ����������*/
	if(!avicglUnProject(viewPt->x, viewPt->y, 0.01, invmatrix, innerviewport, &obj_x0, &obj_y0, &obj_z0))
		return(FALSE);
	/*�������Ϊ0.99998���ڲ��ӿ������Ӧ����������*/
	if(!avicglUnProject(viewPt->x, viewPt->y, 0.99999, invmatrix, innerviewport, &obj_x1, &obj_y1, &obj_z1))
		return(FALSE);
	/*����2�������������ɵ�ֱ����ƽ��Ľ���*/
	ret = getIntersectionPointOfLineAndPlane(obj_x0, obj_y0, obj_z0, obj_x1, obj_y1, obj_z1, &point);
	if(-1 != ret)
	{
		objPt->x = point.x;
		objPt->y = point.y;
		objPt->z = point.z; 
	}else{
		return(FALSE);
	}
	return(TRUE);
}

/* �ڲ��ӿ����� --> OpenGL���� */
BOOL innerViewDPt2openGlDPt(const LP_PT_2D viewPt, const f_int32_t innerviewport[4], LP_PT_2D openGlPt)
{
	if(NULL == viewPt || NULL == openGlPt)
		return(FALSE);    
	
	openGlPt->x = viewPt->x / innerviewport[2];
	openGlPt->x = (openGlPt->x - 0.5) * 2.0;
	openGlPt->y = viewPt->y / innerviewport[3];
	openGlPt->y = (openGlPt->y - 0.5) * 2.0;
	
	return(TRUE);
}

/* OpenGL���� --> �ڲ��ӿ����� */
BOOL openGlDPt2InnerViewDPt(const LP_PT_2D openGlPt, const f_int32_t innerviewport[4], LP_PT_2D viewPt)
{
	if(NULL == viewPt || NULL == openGlPt)
		return(FALSE);
		
    viewPt->x = openGlPt->x * 0.5 + 0.5;
    viewPt->y = openGlPt->y * 0.5 + 0.5;
    viewPt->x = viewPt->x * innerviewport[2];
    viewPt->y = viewPt->y * innerviewport[3];
    		
    return(TRUE); 
}

/* �������� --> �ڲ��ӿ�����,��ͶӰ��ʹ�� */
BOOL geoDPt2InnerViewDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t innerviewport[4], LP_PT_3D viewPt)
{
	Obj_Pt_D objPt;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);
	if(!geoDPt2objDPt(geoPt, &objPt))
	    return(FALSE);
	if(!objDPt2InnerViewDPt(&objPt, mdviewMatrix, innerviewport, viewPt))
	    return(FALSE);
	    	    
	return(TRUE);
}

/* �ڲ��ӿ����� --> ��������,��ͶӰ��ʹ��,����� */
BOOL innerViewDPt2geoDPt(const LP_PT_3D viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	Obj_Pt_D objPt;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);
    if(!innerViewDPt2objDPt(viewPt, invmatrix, innerviewport, &objPt))
	    return(FALSE);
	if(!objDPt2geoDPt(&objPt, geoPt))
	    return(FALSE);
	    	    
	return(TRUE);		
}


/* �ڲ��ӿ����� --> ��������,��ͶӰ��ʹ��,������� */
BOOL innerViewDPt2geoDPtCorrect(const LP_PT_2D viewPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	Obj_Pt_D objPt;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);
	if(!innerViewXYDPt2objDPt(viewPt, mdviewMatrix, invmatrix, innerviewport, &objPt))
		return(FALSE);
	if(!objDPt2geoDPt(&objPt, geoPt))
		return(FALSE);

	return(TRUE);		
}

/* �������� --> �ڲ���Ļ����,��ͶӰ��ʹ�� */
BOOL geoDPt2InnerScreenDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                           const f_int32_t innerviewport[4], LP_PT_2I screenPt)
{
	PT_3D viewPt;
	PT_2I inner_view_pt;
	if(NULL == geoPt || NULL == screenPt)
		return(FALSE);
	if(!geoDPt2InnerViewDPt(geoPt, mdviewMatrix, innerviewport, &viewPt))
	    return(FALSE);
	inner_view_pt.x = (f_int32_t)viewPt.x;
	inner_view_pt.y = (f_int32_t)viewPt.y;
	if(!innerViewPt2innerScreenPt(innerviewport, inner_view_pt, screenPt))   
	    return(FALSE); 
	    
	return(TRUE);
}

/* �ڲ���Ļ���� --> ��������,��ͶӰ��ʹ��,����� */
BOOL innerScreenDPt2GeoDPt(const PT_2I screenPt, const f_float64_t invmatrix[16], 
                           const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_3D viewPt;
	PT_2I inner_view_pt;
	f_float32_t view_winz = 0.0f;
	
	if(NULL == geoPt)
		return(FALSE);
	if(!innerScreenPt2innerViewPt(innerviewport, screenPt, &inner_view_pt))
	    return(FALSE);
    
	/*���Ӵ�ʸ����ͼģʽ�£�û�и߶ȣ���ʱ��ȡ�������ֵ��׼ȷ(Ĭ��Ϊ0)��ת�������겻��ȷ��С��������Ϊ����*/
	/*����Ӱ��ȵ�ͼģʽ�£���Ƭ���Ƶĸ߶�Ϊ0����δ�������Ǹ�������ʱ��ȡ�������ֵ��׼ȷ(����ֵ)��ת�������겻׼ȷ*/
	glReadPixels(inner_view_pt.x + innerviewport[0], inner_view_pt.y + innerviewport[1], 
	             1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &view_winz);
    if(fabs(view_winz - 1.0) < FLT_EPSILON)
        view_winz = 0.0f;


    viewPt.x = inner_view_pt.x;
    viewPt.y = inner_view_pt.y;
    viewPt.z = view_winz;    
	if(!innerViewDPt2geoDPt(&viewPt, invmatrix, innerviewport, geoPt))
	    return(FALSE);
	    
	return(TRUE);
}

/* �ڲ���Ļ���� --> ��������,��ͶӰ��ʹ��,������� */
BOOL innerScreenDPt2GeoDPtCorrect(const PT_2I screenPt, const f_float64_t mdviewMatrix[16],
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2D viewPt;
	Obj_Pt_D objPt;
	PT_2I inner_view_pt;

	if(NULL == geoPt)
		return(FALSE);
	if(!innerScreenPt2innerViewPt(innerviewport, screenPt, &inner_view_pt))
		return(FALSE);

	viewPt.x = inner_view_pt.x;
	viewPt.y = inner_view_pt.y;
   
	if(!innerViewXYDPt2objDPt(&viewPt, mdviewMatrix, invmatrix, innerviewport, &objPt))
		return(FALSE);

	if(!objDPt2geoDPt(&objPt, geoPt))
		return(FALSE);

	return(TRUE);
}

/* �������� --> �ⲿ��Ļ����,��ͶӰ��ʹ�� */
BOOL geoDPt2OuterScreenDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                           const f_int32_t innerviewport[4], LP_PT_2I screenPt)
{
	PT_2I inner_screenPt;
	if(NULL == geoPt || NULL == screenPt)
		return(FALSE);
	if(!geoDPt2InnerScreenDPt(geoPt, mdviewMatrix, innerviewport, &inner_screenPt))
	    return(FALSE);
	if(!innerScreenPt2outerScreenPt(inner_screenPt, screenPt))
	    return(FALSE);
	    
	return(TRUE);
}

/* �ⲿ��Ļ���� --> ��������,��ͶӰ��ʹ��,������� */
BOOL outerScreenDPt2GeoDPt(const PT_2I screenPt, const f_float64_t invmatrix[16], 
                           const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I inner_screen_pt;	
	if(NULL == geoPt)
		return(FALSE);
	if(!outerScreenPt2innerScreenPt(screenPt, &inner_screen_pt))
	    return(FALSE);	    
	if(!innerScreenDPt2GeoDPt(inner_screen_pt, invmatrix, innerviewport, geoPt))
	    return(FALSE);	
	    
	return(TRUE);
}

/* �ⲿ��Ļ���� --> ��������,��ͶӰ��ʹ��,������� */
BOOL outerScreenDPt2GeoDPtCorrect(const PT_2I screenPt, const f_float64_t mdviewMatrix[16], 
	const f_float64_t invmatrix[16], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I inner_screen_pt;	
	if(NULL == geoPt)
		return(FALSE);
	if(!outerScreenPt2innerScreenPt(screenPt, &inner_screen_pt))
		return(FALSE);	    
	if(!innerScreenDPt2GeoDPtCorrect(inner_screen_pt, mdviewMatrix, invmatrix, innerviewport, geoPt))
		return(FALSE);	

	return(TRUE);
}


/* �������� --> �ⲿ�ӿ�����,��ͶӰ��ʹ�� */
BOOL geoDPt2OuterViewDPt(const LP_Geo_Pt_D geoPt, const f_float64_t mdviewMatrix[16], 
                         const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_PT_2I viewPt)
{
	PT_2I outer_screenPt;
	if(NULL == geoPt || NULL == viewPt)
		return(FALSE);
	if(!geoDPt2OuterScreenDPt(geoPt, mdviewMatrix, outerviewport, &outer_screenPt))
	    return(FALSE);
	if(!outerScreenPt2outerViewPt(innerviewport, outer_screenPt, viewPt))        
	    return(FALSE);
	    
	return(TRUE);
}

/* �ⲿ�ӿ����� --> ��������,��ͶӰ��ʹ��,������� */
BOOL outerViewDPt2geoDPt(const PT_2I viewPt, const f_float64_t invmatrix[16], 
                         const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I outer_screen_pt;
	if(NULL == geoPt)
		return(FALSE);
	if(!outerViewPt2outerScreenPt(outerviewport, viewPt, &outer_screen_pt))
	    return(FALSE);			
    if(!outerScreenDPt2GeoDPt(outer_screen_pt, invmatrix, innerviewport, geoPt))
	    return(FALSE);
	    	    
	return(TRUE);		
}

/* �ⲿ�ӿ����� --> ��������,��ͶӰ��ʹ��,������� */
BOOL outerViewDPt2geoDPtCorrect(const PT_2I viewPt, const f_float64_t mdviewMatrix[16],const f_float64_t invmatrix[16], 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I outer_screen_pt;
	if(NULL == geoPt)
		return(FALSE);
	if(!outerViewPt2outerScreenPt(outerviewport, viewPt, &outer_screen_pt))
		return(FALSE);			
	if(!outerScreenDPt2GeoDPtCorrect(outer_screen_pt, mdviewMatrix, invmatrix, innerviewport, geoPt))
		return(FALSE);

	return(TRUE);		
}

/* �ڲ���Ļ���� --> �������� */


// ���Ƿ��ڶ������
static BOOL PtInMapPolygon(LP_PT_3D pPolyPts, f_uint16_t poly_pt_num, LP_PT_2D pPtPos)
{
	f_int32_t i, nCross = 0;
	for(i = 0; i < poly_pt_num; ++i)
	{
		f_float64_t x;
		PT_3D p1 = pPolyPts[i];
		PT_3D p2 = pPolyPts[(i + 1) % poly_pt_num];
		//���y = p.y��p1p2�Ľ���
		if(p1.y == p2.y)			//p1p2��y = p0.yƽ��
			continue;
		if(pPtPos->y < MIN(p1.y, p2.y))	//������p1p2���ӳ�����
			continue;
		if(pPtPos->y >= MAX(p1.y, p2.y))	//������p1p2�ӳ�����
			continue;
		//�򽻵��x����
		x = (pPtPos->y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
		if(x > pPtPos->x)
			nCross++;	//ֻͳ�Ƶ��߽���
	}
	//���߽���Ϊż�������ڶ����֮��
	if (nCross % 2 == 1)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
/*	return (nCross % 2 == 1);*/
}



//
// 3--------2
// |	  B	/	|
// |	      /A	|
// 0--------1
//
// AorBֵ�� 0:A��, 1:B��
 BOOL InScreenTile(sGLRENDERSCENE *pScene,const sQTMAPNODE  *pTile, 
 	const f_float64_t x, const f_float64_t y, int * AorB)
{
	PT_2D PtPos = {x, y};

	PT_3D PSreen[4];	//��Ƭ���ĸ��ǵ����Ļ����
	Obj_Pt_D o0, o1, o2, o3;	//��Ƭ���ĸ��ǵ�XYZ��������

	o0.x = pTile->fvertex_overlook[0];
	o0.y = pTile->fvertex_overlook[1];
	o0.z = pTile->fvertex_overlook[2];
	
	o1.x = pTile->fvertex_overlook[3];
	o1.y = pTile->fvertex_overlook[4];
	o1.z = pTile->fvertex_overlook[5];

	o2.x = pTile->fvertex_overlook[6];
	o2.y = pTile->fvertex_overlook[7];
	o2.z = pTile->fvertex_overlook[8];

	o3.x = pTile->fvertex_overlook[9];
	o3.y = pTile->fvertex_overlook[10];
	o3.z = pTile->fvertex_overlook[11];

	objDPt2InnerViewDPt(&o0,	pScene->matrix, pScene->innerviewport, &PSreen[0]);
	objDPt2InnerViewDPt(&o1,	pScene->matrix, pScene->innerviewport, &PSreen[1]);
	objDPt2InnerViewDPt(&o2,	pScene->matrix, pScene->innerviewport, &PSreen[2]);
	objDPt2InnerViewDPt(&o3,	pScene->matrix, pScene->innerviewport, &PSreen[3]);

	if(FALSE == PtInMapPolygon(&PSreen[0], 4, &PtPos))
	{
		return FALSE;
	}
	else
	{

		if(TRUE == PtInMapPolygon(&PSreen[0], 3, &PtPos))
		{

			if((PSreen[0].z < 1.0)&&(PSreen[0].z > 0.0))
			{
				if((PSreen[1].z < 1.0)&&(PSreen[1].z > 0.0))
				{
					if((PSreen[2].z < 1.0)&&(PSreen[2].z > 0.0))
					{

						*AorB = 0;

						return TRUE;

					}
				}	
			}
	

		}
		else
		{

			if((PSreen[0].z < 1.0)&&(PSreen[0].z > 0.0))
			{
				if((PSreen[2].z < 1.0)&&(PSreen[2].z > 0.0))
				{
					if((PSreen[3].z < 1.0)&&(PSreen[3].z > 0.0))
					{
						*AorB = 1;

						return TRUE;
					}
				}
			}


		

		}
	

			
	}


	return FALSE;
	
}



/*
 *	����������
 *	������
 *			pRayPos	�������
 *			pRayTo	���߷��򣨵�λ������
 *			pSphCenter����
 *			radius	��뾶
 *	����ֵ��
 *			���㵽�������ľ���-1:���ཻ
 */
f_float64_t Ray2SphereIntersect(LP_PT_3D pRayPos, LP_PT_3D pRayTo, LP_PT_3D pSphCenter, f_float64_t radius)
{
	PT_3D rayorig;
	f_float64_t a, b, c, d;
	vector3DSub(&rayorig, pRayPos, pSphCenter);
	//���������
	if(vector3DLength(&rayorig) <= sqrt(radius))
	{
		return 0;
	} 
	a = vector3DDotProduct(pRayTo, pRayTo);
	b = 2 * vector3DDotProduct(&rayorig, pRayTo);
	c = vector3DDotProduct(&rayorig, &rayorig) - sqr(radius);
	d = sqr(b)- (4 * a * c);
	if(d<0)
	{
		return -1;
	}
	else
	{
		f_float64_t t = (-b - sqrt(d)) / (2 * a);
		if( t < 0 )
			t = (-b + sqrt(d) ) / (2 * a);
		return t;
	}
}



//
// 3--------2
// |	  \	B	|
// |	 A \ 
// 0--------1
//
// AorBֵ�� 0:A��, 1:B��
 BOOL InScreenTileAll(sGLRENDERSCENE *pScene,const sQTMAPNODE  *pTile, 
 	const f_float64_t x, const f_float64_t y, 
 	LP_PT_3D out0, LP_PT_3D out1, LP_PT_3D out2)
{
	PT_2D PtPos = {x , y };

	PT_3D PSreen[4] = {0};			//��Ƭ���ĸ��ǵ����Ļ����
	Obj_Pt_D o0, o1, o2, o3;	//��Ƭ���ĸ��ǵ�XYZ��������

	double radius_screen = 0.0;

	// 1.�ж��Ƿ�����Ƭ��,ͨ����Ƭ�İ�Χ���������Ƿ��ཻ�ж�
	{	
		double obj_x0, obj_y0, obj_z0, obj_x1, obj_y1, obj_z1;
		int ret;
		PT_3D pRayPos = {0};
		PT_3D pRayTo = {0};
		PT_3D pSphCenter = {pTile->nodeRangeOsg.x, pTile->nodeRangeOsg.y, pTile->nodeRangeOsg.z};
		double tempPosTo = 0.0;	
			
		/* ��ֱ�ߵ������˵� */
		if(!avicglUnProject(PtPos.x, PtPos.y, 0.01,  pScene->invmat, pScene->innerviewport, &obj_x0, &obj_y0, &obj_z0))
		{
		    printf("gluUnproject fail 0.\n");
		    return(FALSE);
		}

		if(!avicglUnProject(PtPos.x, PtPos.y, 0.99999f,  pScene->invmat, pScene->innerviewport, &obj_x1, &obj_y1, &obj_z1))
		{
		    printf("gluUnproject fail 1.\n");
		    return(FALSE);
		}

		pRayPos.x  = obj_x0;
		pRayPos.y  = obj_y0;
		pRayPos.z  = obj_z0;


		tempPosTo = (sqrt(sqr(obj_x1 - obj_x0) + sqr(obj_y1 - obj_y0) + sqr(obj_z1 - obj_z0)));

		pRayTo.x = (obj_x1 - obj_x0)/tempPosTo;
		pRayTo.y = (obj_y1 - obj_y0)/tempPosTo;
		pRayTo.z = (obj_z1 - obj_z0)/tempPosTo;
 
		ret = Ray2SphereIntersect(&pRayPos, &pRayTo, 
			&pSphCenter, pTile->nodeRangeOsg.radius);

		if(ret == -1)
		{
			return FALSE;
		}
	

	}	

	radius_screen = sqrt(sqr(pScene->innerviewport[2]/2.0) + sqr(pScene->innerviewport[3]/2.0)  );
		
	// 2.ȷ�����ĸ��ı������Լ�ȷ����A/B��Χ��
	{
		int i = 0 , j = 0;


		for(i = 0; i < VTXCOUNT; i++)		// i �е�����
		{
			for(j = 0; j < VTXCOUNT; j++)	// j �е�����
			{

				int o0_num, o3_num ;

				o0_num = VTXCOUNT1 * i + j ;
				o3_num = VTXCOUNT1 *( i + 1)+ j ;

			
				o0.x = pTile->fvertex[3 * o0_num];
				o0.y = pTile->fvertex[3 * o0_num + 1];
				o0.z = pTile->fvertex[3 * o0_num + 2];
				
				o1.x = pTile->fvertex[3 * o0_num + 3];
				o1.y = pTile->fvertex[3 * o0_num + 4];
				o1.z = pTile->fvertex[3 * o0_num + 5];

				o2.x = pTile->fvertex[3 * o3_num + 3];
				o2.y = pTile->fvertex[3 * o3_num + 4];
				o2.z = pTile->fvertex[3 * o3_num + 5];

				o3.x = pTile->fvertex[3 * o3_num];
				o3.y = pTile->fvertex[3 * o3_num + 1];
				o3.z = pTile->fvertex[3 * o3_num + 2];

				objDPt2InnerViewDPt(&o0,	pScene->matrix, pScene->innerviewport, &PSreen[0]);
				objDPt2InnerViewDPt(&o1,	pScene->matrix, pScene->innerviewport, &PSreen[1]);
				objDPt2InnerViewDPt(&o2,	pScene->matrix, pScene->innerviewport, &PSreen[2]);
				objDPt2InnerViewDPt(&o3,	pScene->matrix, pScene->innerviewport, &PSreen[3]);

				//�ж��ĸ����Ƿ����ӿ���
				#if 0
				{
					int pt_i = 0;
					int pt_out_screen_count = 0;
					for(pt_i = 0; pt_i < 4; pt_i ++)
					{

					
						if(  (PSreen[pt_i].x > pScene->innerviewport[2])
						  || (PSreen[pt_i].x < 0.0)
						  || (PSreen[pt_i].y > pScene->innerviewport[3]) 
						  || (PSreen[pt_i].y < 0.0))
						{
							pt_out_screen_count ++;
						}

					}

					if(pt_out_screen_count >= 4)		//LPF MODIFY 2020��5��11��20:21:47
					{
						continue;
					}

				}

				#else
				//ͨ����ΧԲ����Ļ�Ƿ��ཻ���Ƽ���

				{
					// 1.�����ΧԲ
				
					PT_3F PSreen_f[4] = {0};
					int i = 0;

					PT_3F PScreen_core = {0};
					double radius = 0.0;
					
					for(i = 0 ; i < 4; i ++)
					{
						PSreen_f[i].x = PSreen[i].x;
						PSreen_f[i].x = PSreen[i].x;
						PSreen_f[i].x = PSreen[i].x;
					}
					
					PScreen_core = ComputeBoundingSphereRitter(PSreen_f, 4, &radius);

					// 2.�ж��Ƿ��ཻ
					{
						double d_ball_2 = 0.0;

						d_ball_2 = sqr(PScreen_core.x - pScene->innerviewport[2]/2.0) 
							+ sqr(PScreen_core.y - pScene->innerviewport[3]/2.0);

						if(d_ball_2 > (sqr(radius_screen) + sqr(radius)))
						{
							continue;
						}
						
					}

				}

				
				#endif
				

				//�ж�ת�����Ƿ����ı�����
				if(FALSE == PtInMapPolygon(&PSreen[0], 4, &PtPos))
				{
					//return FALSE;
				}
				else
				{

					if(TRUE == PtInMapPolygon(&PSreen[1], 3, &PtPos))
					{


//						InFrustum(const f_float32_t view_near, const f_float32_t view_far, const f_float64_t x, const f_float64_t y, const f_float64_t z, const f_float64_t radius, const LPFRUSTUM pFrustum, f_float64_t * pDis);


						if((PSreen[1].z < 1.0)&&(PSreen[1].z > 0.0))
						{
							if((PSreen[2].z < 1.0)&&(PSreen[2].z > 0.0))
							{
								if((PSreen[3].z < 1.0)&&(PSreen[3].z > 0.0))
								{
									*out0 = o1;
									*out1 = o2;
									*out2 = o3;

									//printf("i = %d, j = %d\n", i, j);
									//printf("PScreen 1 = %f,%f,%f\n", PSreen[1].x, PSreen[1].y, PSreen[1].z);
									//printf("PScreen 2 = %f,%f,%f\n", PSreen[2].x, PSreen[2].y, PSreen[2].z);
									//printf("PScreen 3 = %f,%f,%f\n", PSreen[3].x, PSreen[3].y, PSreen[3].z);
									//printf("point x = %f, y = %f\n", PtPos.x, PtPos.y);

									return TRUE;	

								}
							}

						}


	


						
					}
					else
					{


						if((PSreen[0].z < 1.0)&&(PSreen[0].z > 0.0))
						{
							if((PSreen[1].z < 1.0)&&(PSreen[1].z > 0.0))
							{
								if((PSreen[3].z < 1.0)&&(PSreen[3].z > 0.0))
								{
					
									*out0 = o0;
									*out1 = o1;
									*out2 = o3;

									//printf("i = %d, j = %d\n", i, j);
									//printf("PScreen 0 = %f,%f,%f\n", PSreen[0].x, PSreen[0].y, PSreen[0].z);
									//printf("PScreen 1 = %f,%f,%f\n", PSreen[1].x, PSreen[1].y, PSreen[1].z);
									//printf("PScreen 3 = %f,%f,%f\n", PSreen[3].x, PSreen[3].y, PSreen[3].z);
									//printf("point x = %f, y = %f\n", PtPos.x, PtPos.y);


									return TRUE;

									
								}
							}
						}

					}

				

						
				}





			}

		}

	}

	return FALSE;
}


extern VOIDPtr pMapDataSubTree;			/*��ͼ������*/
void CalLineP0d0(float screen_x, float screen_y, 
	sGLRENDERSCENE *pScene, LP_PT_3D p0, LP_PT_3D d0)
{
	/* �������� */
	double obj_x0, obj_y0, obj_z0, obj_x1, obj_y1, obj_z1;
	/* ������������� */

	
	/* ��ֱ�ߵ������˵� */
	if(!avicglUnProject(screen_x, screen_y, 0.01,  pScene->invmat, pScene->innerviewport, &obj_x0, &obj_y0, &obj_z0))
	{
	    printf("gluUnproject fail 0.\n");
	    return;
	}

	if(!avicglUnProject(screen_x, screen_y, 0.99999f,  pScene->invmat, pScene->innerviewport, &obj_x1, &obj_y1, &obj_z1))
	{
	    printf("gluUnproject fail 1.\n");
	    return;
	}

	p0->x = obj_x0;
	p0->y = obj_y0;
	p0->z = obj_z0;

	d0->x = obj_x1 - obj_x0;
	d0->y = obj_y1 - obj_y0;
	d0->z = obj_z1 - obj_z0;
	
}

BOOL innerViewDPt2GeoDPtOpenglEs(sGLRENDERSCENE *pScene,
		const PT_2I viewPt, LP_Geo_Pt_D geoPt)
{
	// 1.��ȡ������

	f_int32_t iLevel = 0;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
	sMAPHANDLE * pHandle = pMapDataSubTree;
	PT_3D p1, p2, p3;
	int AorB = 0;
	int i = 0;

	double  normal[3] = {0};
	double d  = 0.0;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	if(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
	{
		/*����LRU��������û�д����صĽڵ�*/
		pstListHead = &(pScene->scene_draw_list);

		if (LIST_IS_EMPTY(pstListHead))
		{
			giveSem(pHandle->rendscenelock);
			return FALSE;
		}
		
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
				pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);

				if((pNode != NULL)&&(pNode->nodekey == MAPDATANODEKEY))
				{
					i ++;

					//printf("%d-",i);

					if( pNode->level < iLevel)
						continue;

					if(InScreenTile(pScene, pNode, viewPt.x, viewPt.y, &AorB))
					{
			
						if(AorB  == 1)
						{
							p1.x = pNode->fvertex_overlook[0];//pNode->hstart;
							p1.y = pNode->fvertex_overlook[1];//pNode->vstart;
							p1.z = pNode->fvertex_overlook[2];//pNode->height_lb;

							p2.x = pNode->fvertex_overlook[6];//pNode->hstart + pNode->hscale;
							p2.y = pNode->fvertex_overlook[7];//pNode->vstart + pNode->vscale;
							p2.z = pNode->fvertex_overlook[8];//pNode->height_rt;

							
							p3.x = pNode->fvertex_overlook[9];//pNode->hstart;
							p3.y = pNode->fvertex_overlook[10];//pNode->vstart + pNode->vscale;
							p3.z = pNode->fvertex_overlook[11];//pNode->height_lt;
							
						}
						else
						{
							p1.x = pNode->fvertex_overlook[0];//pNode->hstart;
							p1.y = pNode->fvertex_overlook[1];//pNode->vstart;
							p1.z = pNode->fvertex_overlook[2];//pNode->height_lb;

							p2.x = pNode->fvertex_overlook[6];//pNode->hstart + pNode->hscale;
							p2.y = pNode->fvertex_overlook[7];//pNode->vstart + pNode->vscale;
							p2.z = pNode->fvertex_overlook[8];//pNode->height_rt;

							
							p3.x = pNode->fvertex_overlook[3];//pNode->hstart + pNode->hscale;
							p3.y = pNode->fvertex_overlook[4];//pNode->vstart;
							p3.z = pNode->fvertex_overlook[5];//pNode->height_rb;
						}
						

						GetNormalOverlook(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z,
							p3.x - p1.x, p3.y - p1.y, p3.z - p1.z,
							&normal[0]) ;

						d = normal[0] * p1.x + normal[1] * p1.y +normal[2] * p1.z ;
						
						iLevel = pNode->level + 1;
					
					}		



				}
				else
				{
					break;
				}
			
			}
		}	

	}
	else
	{
		/*����LRU��������û�д����صĽڵ�*/
		pstListHead = &(pScene->scene_draw_list);

		if (LIST_IS_EMPTY(pstListHead))
		{
			giveSem(pHandle->rendscenelock);
			return FALSE;
		}

//		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)

		pstTmpList = pstListHead->pNext;

		//printf("%x\n", pstListHead);

//		for( ; pstTmpList != pstListHead; )	
		while(1)
		{
			if(pstTmpList == pstListHead)
			{
				//printf("\n i =  %d\n",i);
				//printf("\n%x-%x\n", pstListHead, pstTmpList);
				break;
			}

			//printf("%x-", pstTmpList);
			
			if( NULL != pstTmpList )
			{
				pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);

				if((pNode != NULL)&&(pNode->nodekey == MAPDATANODEKEY))
				{
					double point_x = viewPt.x + 0.0;//pScene->innerviewport[0];
					double point_y = viewPt.y + 0.0;//pScene->innerviewport[1];
				
					i ++;

					//printf("%d-",i);

					if( pNode->level < iLevel)
					{	
						pstTmpList = pstTmpList->pNext;
						continue;
					}
					
					if(InScreenTileAll(pScene, pNode, point_x, point_y, &p1, &p2, &p3))
					{
						
						GetNormalOverlook(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z,
							p3.x - p1.x, p3.y - p1.y, p3.z - p1.z,
							&normal[0]) ;
						
						d = normal[0] * p1.x + normal[1] * p1.y +normal[2] * p1.z ;
						
						iLevel = pNode->level + 1;
						
						//printf("%d, %d, %d\n", pNode->xidx, pNode->yidx, pNode->level);
						
						
					}
				}
				else
				{
					break;
				}
		
			
			}

			pstTmpList = pstTmpList->pNext;
		}	





	}
	
	giveSem(pHandle->rendscenelock);

	//�������߷���					
	//���߷����� p(t) = p0 + t*d0,����d0��p(t)��p0����ά����
	if (iLevel != 0)
	{
		PT_3D p0 = {0};
		PT_3D d0 = {0};
		double t = 0.0;
		PT_3D pt ={0};

		PT_3D n = {normal[0], normal[1], normal[2]};

		CalLineP0d0(viewPt.x, viewPt.y, 
			pScene, &p0,  &d0);
	
		Ray2TrangleIntersect(n,  d,  p0,  d0, &t);

		vector3DMul(&pt, &d0, t);

		vector3DAdd(&pt,&pt,&p0);

		objDPt2geoDPt(&pt, geoPt);

	
	}

	

	#if 0
	{
		if(!drawGeoUserLayerPre(pScene))
		{
		    DEBUG_PRINT("drawGeoUserLayerPre error.");
			return FALSE;
		}

			
		glColor3f(1.0,0.0,0.0);
		glLineWidth(5.0);
		glDisableEx(GL_DEPTH_TEST);
		glDisableEx(GL_TEXTURE_2D);
		glDisableEx(GL_BLEND);
		
		glBegin(GL_LINE_LOOP);
			glVertex3f(p1.x , p1.y, p1.z);		
			glVertex3f(p2.x , p2.y, p2.z);		
			glVertex3f(p3.x , p3.y, p3.z);		
		glEnd();

		#if 0
		{		

			PT_3D d0 = {
					2*EARTH_RADIUS * cos(lfLon * DE2RA) * cos(lfLat * DE2RA), 
					2*EARTH_RADIUS * sin(lfLon * DE2RA) * cos(lfLat * DE2RA), 
					2*EARTH_RADIUS * sin(lfLat * DE2RA) };

			glColor3f(1.0,1.0,0.0);
			glLineWidth(10.0);

			glBegin(GL_LINES);
						glVertex3f(0.0,0.0,0.0);				
						glVertex3f(d0.x , d0.y, d0.z);		
			glEnd();


		}
		#endif


		glEnableEx(GL_TEXTURE_2D);
		glEnableEx(GL_BLEND);


		drawUserLayerPro();	
	}
	#endif

	
	if (iLevel != 0)
		return TRUE;
	else
		return FALSE;

}

BOOL outerScreenDPt2GeoDPtOpengEs(sGLRENDERSCENE *pScene, const PT_2I screenPt, 
	const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I inner_view_pt = {0};	
	PT_2I inner_screen_pt = {0};

	if(NULL == geoPt)
		return(FALSE);

	if(!outerScreenPt2innerScreenPt(screenPt, &inner_screen_pt))
		return(FALSE);

	if(!innerScreenPt2innerViewPt(innerviewport, inner_screen_pt, &inner_view_pt))
		return(FALSE);

	if(!innerViewDPt2GeoDPtOpenglEs(pScene, inner_view_pt, geoPt))
		return(FALSE);	

	return(TRUE);
}

BOOL outerViewDPt2GeoDPtOpengEs(sGLRENDERSCENE *pScene, const PT_2I viewPt, 
	const f_int32_t outerviewport[4], const f_int32_t innerviewport[4], LP_Geo_Pt_D geoPt)
{
	PT_2I outer_screen_pt;
	if(NULL == geoPt)
		return(FALSE);
	if(!outerViewPt2outerScreenPt(outerviewport, viewPt, &outer_screen_pt))
		return(FALSE);			
	if(!outerScreenDPt2GeoDPtOpengEs(pScene, outer_screen_pt, innerviewport, geoPt))
		return(FALSE);

	return(TRUE);		
}

