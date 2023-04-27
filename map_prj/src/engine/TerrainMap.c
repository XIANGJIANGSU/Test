/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2018��
**
** �ļ���: TerrainMap.c
**
** ����: ���ļ�������·�㴹ֱ��������ͼ����صĺ�����
**
** ����ĺ���:  
**
**                            
** ���ע��: 
**
** ���ߣ�
**		LPF��
** 
**
** ������ʷ:
**		2017-6-26 9:06 LPF �������ļ�
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#include <math.h>
#include "../utility/tools.h"
#include "../mapApp/bil.h"
#include "../mapApi/mapApi.h"
#include "../mapApp/appHead.h"
#include "../mapApp/mapApp.h"
#include "../mapApi/common.h"
#include "../mapApi/initApi.h"
#include "../mapApi/paramGetApi.h"
#include "memoryPool.h"

#ifdef WIN32
#include "../../../include_vecMap/2dmapApi.h"
#include "../../../vecMapEngine_prj/src/2dEngine/vecMapFont.h"
#endif
/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*/
extern TTFONT GetHandleFont();
extern f_int32_t createPngTexture(f_int8_t* filename, f_int32_t* texture_id);

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/



/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
static float y_start = 0.0;	//y����ʼ��ĺ��θ߶�, ������͵�򺽵���͵�
static float y_end = 0.0;		//y����ֹ��ĺ��θ߶ȣ�������ߵ�򺽵���ߵ�
int texture_id_terrainmap = 0;  //��ͼID
static f_int32_t terrainMapListIdNeedRegenerate = 0;  //��ʾ�б��Ƿ���Ҫ����,0-����Ҫ,1-��Ҫ
/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/


/*
*	����ɫ������������ͼ����һ�����μ���
*/
void static DrawQuadsTerrainEx(sGLRENDERSCENE* pScene)
{
	f_float32_t s_height_pass = pScene->termap_param.y_pass;
	f_float32_t s_height_pass_h = pScene->termap_param.y_pass_h;
	f_float32_t s_width_pass = pScene->termap_param.x_pass;
	f_float32_t s_width_pass_h = pScene->termap_param.x_pass_h;

	/*���ƾ���ɫ��*/
	if(pScene->terrainMap_param_used.s_TerrainMapLISTID[2] == 0)
	{
		pScene->terrainMap_param_used.s_TerrainMapLISTID[2] = glGenLists(1);
	}

	if(terrainMapListIdNeedRegenerate){
		glNewList(pScene->terrainMap_param_used.s_TerrainMapLISTID[2], GL_COMPILE);
		{
			glBegin(GL_QUADS);
			glVertex2f(s_width_pass, s_height_pass);
			glVertex2f(s_width_pass_h, s_height_pass);
			glVertex2f(s_width_pass_h,  s_height_pass_h);
			glVertex2f(s_width_pass, s_height_pass_h);
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->terrainMap_param_used.s_TerrainMapLISTID[2]);
	}
}


/*.BH--------------------------------------------------------
**
** ������: DrawDynamicTerrainMap
**
** ����:  ����Ļ�ϻ��ƶ�̬�ĵ���
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�1,����ģ�建�����ķ���������������
**
**.EH--------------------------------------------------------
*/
static void DrawDynamicTerrainMap(sGLRENDERSCENE* pScene)
{

#if 1
	/*������������,����ģ�建�����ķ�ʽ*/
	/* ����ģ����ԣ�Ĭ�ϲ�������ģ����Է�����͸���Ȳ���֮����Ȳ���֮ǰ*/
    glEnableEx(GL_STENCIL_TEST);
	/* ģ�建�����е�ֵ�������3(0x1)���������,��������2(0x1)���бȽ�,�ȽϺ���Ϊ����1. GL_NEVER��ʾģ�������Զ��ͨ��*/
	/* ģ����Բ�ͨ�����ʾ��Ӧ���ص����ɫֵ�������*/
	glStencilFunc(GL_NEVER, 0x1, 0x1);
	/*��һ������sfail�� ���ģ�����ʧ�ܽ���ȡ�Ķ�����
	  �ڶ�������dpfail�� ���ģ�����ͨ����������Ȳ���ʧ��ʱ��ȡ�Ķ�����
	  ����������dppass�� �����Ȳ��Ժ�ģ����Զ�ͨ��������ȡ�Ķ�����
	  GL_INVERT��ʾģ��ֵ��λȡ��
    */
	glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

#if 0	
	//���ƶ��������
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_DOUBLE, 0, &pScene->prefile_param_used.s_vDetectHeights_cal[0]);

	glDrawElements(GL_TRIANGLE_FAN, (SAMPLE_NUM + 3), GL_UNSIGNED_INT, &pScene->prefile_param_used.pIndex_tri_fan[0]);	

	glDisableClientState(GL_VERTEX_ARRAY);

#else

#if 0
	{
		int			i;
		LP_PT_3F	pVtx = pScene->terrainMap_param_used.p_WayPointInterpolation;

		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(pScene->termap_param.x_pass, 0);
		for (i=0; i<pScene->terrainMap_param_used.Interpolation_sum ; ++i)
		{
			glVertex2f(pVtx[i].x, pVtx[i].y);
		}
		glVertex2f(pScene->termap_param.x_pass_h, 0);
		glEnd();
	}
#else
	{
		/*������ʾ�б�Ļ��Ʒ�ʽ*/
		int			i;
		LP_PT_3F	pVtx = pScene->terrainMap_param_used.p_WayPointInterpolation;

		if(pScene->terrainMap_param_used.s_TerrainMapLISTID[1] == 0)
		{
			pScene->terrainMap_param_used.s_TerrainMapLISTID[1] = glGenLists(1);
		}

		if(terrainMapListIdNeedRegenerate){
			glNewList(pScene->terrainMap_param_used.s_TerrainMapLISTID[1], GL_COMPILE);
			{
				glBegin(GL_TRIANGLE_FAN);
				glVertex2f(pScene->termap_param.x_pass, pScene->termap_param.y_pass);
				for (i=0; i<pScene->terrainMap_param_used.Interpolation_sum; ++i)
				{
					glVertex2f(pVtx[i].x, pVtx[i].y);
				}
				glVertex2f(pScene->termap_param.x_pass_h, pScene->termap_param.y_pass);
				glEnd();
			}
			glEndList();
		}
		else
		{
			glCallList(pScene->terrainMap_param_used.s_TerrainMapLISTID[1]);
		}

	}
#endif

#endif

	
	/* �ػ���Ļ,ֻ����ģ�建��ֵ��0������,ɫ��ֻ����������������ʾ */

	/* ģ�建�����е�ֵ�������3(0x1)���������,��������2(0x1)���бȽ�,�ȽϺ���Ϊ����1. GL_EQUAL��ʾ������ȱȽ�*/
	/* ģ����Բ�ͨ�����ʾ��Ӧ���ص����ɫֵ�������*/	
	glStencilFunc(GL_EQUAL,0x1,0x1);	
	/*GL_ZERO��ʾģ��ֵ����Ϊ0*/
	glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO); 

	//����ɫ��
    glColor4f(50.0 / 255.0, 205 / 255.0, 50.0 / 255.0, 1.0f);
 	DrawQuadsTerrainEx(pScene);

	/* ����ģ�������Զͨ�� */
	glStencilFunc(GL_ALWAYS,0,0x1);
#else
// 	{
// 		int			i;
// 		LP_PT_2F	pVtx = pScene->terrainMap_param_used.p_WayPointInterpolation;
// 		
// 		glBegin(GL_TRIANGLE_FAN);
// 		glVertex2f(0, 0);
// 		for (i=0; i<pScene->terrainMap_param_used.Interpolation_sum ; ++i)
// 		{
// 			glVertex2f(pVtx[i].x, pVtx[i].y);
// 		}
// 		glVertex2f(width, 0);
// 		glEnd();
// 	}
	DrawQuadsTerrainEx(pScene);


#endif

}

/*.BH--------------------------------------------------------
**
** ������: GetTerrainMapPointHei
**
** ����:  ����ָ����γ�ȵĸ߶�,��ԭʼ�߳������в�ѯ����,�����Ƭ�����в�
**
** �������:  pScene����Ⱦ�������
**            lon_dest������
**            lat_dest��γ��
**
** �����������
**
** ����ֵ����ѯ���ĸ߶�
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
f_float32_t GetTerrainMapPointHei(sGLRENDERSCENE*pScene, f_float64_t lon_dest,  f_float64_t lat_dest)
{
	f_int16_t hei_dest = 0;
	f_float64_t pz = 0.0;
	
	/* ���ݾ�γ�ȴ�ԭʼ�߳������в�ѯ�߳�ֵ */
	if(FALSE == getAltByGeoPos( lon_dest, lat_dest, &hei_dest))
	{
		/* ���ݾ�γ�ȴ��ڴ����������Ƭ�����в�ѯ�߳�ֵ */
		if(FALSE == GetZ((sGLRENDERSCENE*)pScene, lon_dest, lat_dest, &pz))
		{
			//printf("z = 0\n");
			hei_dest = 0;
		}
		else
		{
			hei_dest = (f_int16_t)pz;
		}	
	}

	return (hei_dest*1.0f);
}

/*.BH--------------------------------------------------------
**
** ������: CalcTerrainMapRatio
**
** ����:  ���㺽�ߴ�ֱ����ͼ��x��y����ķֱ��ʡ�ÿ2����·��֮���ֵ����������к�·��Ͳ�ֵ��ĸ߶ȡ����߶ȼ���С�߶�
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
void CalcTerrainMapRatio(sGLRENDERSCENE* pScene)
{
	double dis_sum = 0.0;	//������ܺ�
	double dis = 0.0;
	int i = 0 , j = 0, k = 0;
	int Interpolation_num = 0;
	float z_hei = 0.0;

	/*����ʵ�ʺ�·����������������ÿ2����·��֮��Ĳ�ֵ��������ڴ�����*/
	pScene->terrainMap_param_used.p_Interpolation_num 
		= (int*)NewAlterableMemory(sizeof(int) * pScene->termap_param.way_point_num);
	/*ѭ���������к�·��*/
	for(i = 0; i < (pScene->termap_param.way_point_num - 1); i ++)
	{	
		/*��������2����·��Ĵ�Բ����*/
		dis = caculateGreatCircleDis(
			pScene->termap_param.p_WayPoints[i].lon,
			pScene->termap_param.p_WayPoints[i].lat,
			pScene->termap_param.p_WayPoints[i + 1].lon,
			pScene->termap_param.p_WayPoints[i + 1].lat);
		/*��������2����·��֮��Ĳ�ֵ�����*/
		pScene->terrainMap_param_used.p_Interpolation_num[i] 
			= (int)(dis /pScene->termap_param.x_interpolation);
		/*�����ܵĲ�ֵ�����*/
		Interpolation_num += pScene->terrainMap_param_used.p_Interpolation_num[i] ;
		/*������κ�·�ľ����ܺ�*/
		dis_sum += dis;
	}
	/*ʵ��ȫ����ֵ��=����õ��Ĳ�ֵ��+��·��*/
	pScene->terrainMap_param_used.Interpolation_sum 
		= pScene->termap_param.way_point_num + Interpolation_num;
	/*����ʵ�ʺ�·�������������������в�ֵ�����Ļ����(x,y)+ ���θ߶�(z)���ڴ�����*/
	pScene->terrainMap_param_used.p_WayPointInterpolation
		= (LP_PT_3F)NewAlterableMemory(pScene->terrainMap_param_used.Interpolation_sum * sizeof(PT_3F));

	/*����ȫ����ֵ��ĸ߶�*/
	y_end = y_start = pScene->termap_param.p_WayPoints[i].height;	
	/*���η��ʸ�����·��*/
	for(i = 0; i < (pScene->termap_param.way_point_num - 1); i ++)
	{	
		/*��ѯ������·��ĸ߶�,��Ϊ�뺽·�㾭γ����ͬ�Ĳ�ֵ��߶�(��ʹ����ԭʼ�߶�),�����µ�ָ���ṹ����*/
		z_hei = GetTerrainMapPointHei(pScene, 
				pScene->termap_param.p_WayPoints[i].lon,
				pScene->termap_param.p_WayPoints[i].lat);
		pScene->terrainMap_param_used.p_WayPointInterpolation[k].z = z_hei;
		/*������к�·��Ͳ�ֵ������͸߶�*/
		if(y_start > z_hei)
		{
			y_start = z_hei;
		}
		if(y_start > pScene->termap_param.p_WayPoints[i].height)
		{
			y_start = pScene->termap_param.p_WayPoints[i].height;
		}
		/*������к�·��Ͳ�ֵ������߸߶�*/
		if(y_end < z_hei)
		{
			y_end = z_hei;
		}
		if(y_end < pScene->termap_param.p_WayPoints[i].height)
		{
			y_end = pScene->termap_param.p_WayPoints[i].height;
		}

		
		k++;
		/*���δ��������ֵ��*/
		for(j = 1; j <= pScene->terrainMap_param_used.p_Interpolation_num[i] ; j++, k ++)
		{
			/*��ѯ������ֵ��ĸ߶�,�����µ�ָ���ṹ����*/	
			z_hei = GetTerrainMapPointHei(pScene, 
					pScene->termap_param.p_WayPoints[i].lon 
					+ j * (pScene->termap_param.p_WayPoints[i+1].lon -  pScene->termap_param.p_WayPoints[i].lon )/(pScene->terrainMap_param_used.p_Interpolation_num[i] + 1),
					pScene->termap_param.p_WayPoints[i].lat 
					+ j * (pScene->termap_param.p_WayPoints[i+1].lat -  pScene->termap_param.p_WayPoints[i].lat)/(pScene->terrainMap_param_used.p_Interpolation_num[i] + 1)
					);
			pScene->terrainMap_param_used.p_WayPointInterpolation[k].z = z_hei;
			/*������к�·��Ͳ�ֵ������͸߶�*/
			if(y_start > z_hei)
			{
				y_start = z_hei;
			}
			/*������к�·��Ͳ�ֵ������߸߶�*/
			if(y_end < z_hei)
			{
				y_end = z_hei;
			}		
		}
	}
	
	/*��ѯ���һ����·��ĸ߶�,��Ϊ�뺽·�㾭γ����ͬ�Ĳ�ֵ��߶�(��ʹ����ԭʼ�߶�),�����µ�ָ���ṹ����*/
	z_hei = GetTerrainMapPointHei(pScene, 
		pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].lon,
		pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].lat);
	pScene->terrainMap_param_used.p_WayPointInterpolation[k].z	= z_hei;

	/*������к�·��Ͳ�ֵ������͸߶�*/
	if(y_start > z_hei)
	{
		y_start = z_hei;
	}
	if(y_start > pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height)
	{
		y_start = pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height;
	}

	/*������к�·��Ͳ�ֵ������߸߶�*/
	if(y_end < z_hei)
	{
		y_end = z_hei;
	}
	if(y_end < pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height)
	{
		y_end = pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height;
	}
	
	/*Ϊ����ʾЧ��������,�����߶�����200m,��С�߶ȼ�С200m*/
	y_end += 200.0f;
	y_start -= 200.0f;

	/*����x����1�����������ʵ�ʾ���*/
	pScene->termap_param.x_ratio = 
		dis_sum / (pScene->termap_param.x_pass_h -pScene->termap_param.x_pass);

	/*����y����1�����������ʵ�ʸ߶�ֵ*/
	pScene->termap_param.y_ratio =
		(y_end - y_start) / (pScene->termap_param.y_pass_h -pScene->termap_param.y_pass);
}


/*.BH--------------------------------------------------------
**
** ������: UpdataTerrainMapScreenPoint
**
** ����:  �������к�·�����Ļ����
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
void UpdataTerrainMapScreenPoint(sGLRENDERSCENE* pScene)
{
	int i = 0;
	float x = 0.0, y = 0.0;
	int num  = pScene->termap_param.way_point_num;
	float width = pScene->termap_param.width;
	float height = pScene->termap_param.height;
	float x_ratio = pScene->termap_param.x_ratio;
	float y_ratio = pScene->termap_param.y_ratio;
	float x_sum = 0.0;

	/*����ʵ�ʺ�·�������������������к�·�����Ļ������ڴ�����*/
	pScene->terrainMap_param_used.p_WayPointScreen
		= (LP_PT_2F)NewAlterableMemory(num * sizeof(PT_2F));

	/*�����һ����·����Ļ����*/
	pScene->terrainMap_param_used.p_WayPointScreen[0].x = pScene->termap_param.x_pass;
	pScene->terrainMap_param_used.p_WayPointScreen[0].y 
		= (pScene->termap_param.p_WayPoints[0].height - y_start)/y_ratio
			+ pScene->termap_param.y_pass;

	x_sum = pScene->terrainMap_param_used.p_WayPointScreen[0].x;
	/*���δ������ຽ·��*/
	for(i = 1; i < num; i ++)
	{	
		/*�������ຽ·����Ļ����,x����:������ǰһ����·��Ĵ�Բ����,���ɾ���������������*/
		x = caculateGreatCircleDis(
			pScene->termap_param.p_WayPoints[i-1].lon,
			pScene->termap_param.p_WayPoints[i-1].lat,
			pScene->termap_param.p_WayPoints[i].lon,
			pScene->termap_param.p_WayPoints[i].lat)/x_ratio;
		/*y���򣺼�������͸߶ȵĲ�ֵ,���ɲ�ֵ�������������*/
		y = (pScene->termap_param.p_WayPoints[i].height - y_start)/y_ratio
			+ pScene->termap_param.y_pass;
		
		x_sum +=x;

		/*������·���x�����������֮ǰ���ۼ�ֵ*/
		pScene->terrainMap_param_used.p_WayPointScreen[i].x = x_sum;
		pScene->terrainMap_param_used.p_WayPointScreen[i].y = y;
	}
}

/*.BH--------------------------------------------------------
**
** ������: UpdataTerrainMapInterpolationScreenPoint
**
** ����:  �������в�ֵ�����Ļ����
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
void UpdataTerrainMapInterpolationScreenPoint(sGLRENDERSCENE* pScene)
{
	int i = 0 , j = 0, k = 0;
	float x_ratio = pScene->termap_param.x_ratio;
	float y_ratio = pScene->termap_param.y_ratio;
	double y_hei = 0.0;
	int num  = pScene->termap_param.way_point_num;

	/*�������в�ֵ�����Ļ����*/
	for(i = 0; i < (num - 1); i ++)
	{	
		/*�����뺽·�㾭γ����ͬ�Ĳ�ֵ�����Ļ����*/
		y_hei = pScene->terrainMap_param_used.p_WayPointInterpolation[k].z;
		/*y���򣺼�������͸߶ȵĲ�ֵ,���ɲ�ֵ�������������*/
		pScene->terrainMap_param_used.p_WayPointInterpolation[k].y	
			= (y_hei - y_start)/y_ratio+ pScene->termap_param.y_pass;
		/*x�����뺽·�������������ͬ*/
		pScene->terrainMap_param_used.p_WayPointInterpolation[k].x	
			= pScene->terrainMap_param_used.p_WayPointScreen[i].x;

		k++;
		/*���δ���·��֮��Ĳ�ֵ��*/
		for(j = 1; j <= pScene->terrainMap_param_used.p_Interpolation_num[i] ; j ++, k ++)
		{
			/*y���򣺼�������͸߶ȵĲ�ֵ,���ɲ�ֵ�������������*/
			y_hei = pScene->terrainMap_param_used.p_WayPointInterpolation[k].z;
			pScene->terrainMap_param_used.p_WayPointInterpolation[k].y	
				= (y_hei - y_start)/y_ratio+ pScene->termap_param.y_pass;
			/*x���򣺼���������·��֮������ؾ���,���ֳɲ�ֵ��+1�ȷ�,ǰһ����·���������Ӷ�Ӧ�ȷݵ����ؾ��뼴��*/
			pScene->terrainMap_param_used.p_WayPointInterpolation[k].x	
				= (pScene->terrainMap_param_used.p_WayPointScreen[i+1].x - 
				pScene->terrainMap_param_used.p_WayPointScreen[i].x) 
				/(pScene->terrainMap_param_used.p_Interpolation_num[i] + 1) * j
				+ pScene->terrainMap_param_used.p_WayPointScreen[i].x;
		}
	}
	
	/*�����뺽·�㾭γ����ͬ�Ĳ�ֵ�����Ļ����*/
	y_hei = pScene->terrainMap_param_used.p_WayPointInterpolation[k].z;
	/*y���򣺼�������͸߶ȵĲ�ֵ,���ɲ�ֵ�������������*/
	pScene->terrainMap_param_used.p_WayPointInterpolation[k].y	
		= (y_hei - y_start)/y_ratio+ pScene->termap_param.y_pass;
	/*x�����뺽·�������������ͬ*/
	pScene->terrainMap_param_used.p_WayPointInterpolation[k].x	
			= pScene->terrainMap_param_used.p_WayPointScreen[i].x;

}



/*.BH--------------------------------------------------------
**
** ������: SetTerrainMapWayPoints
**
** ����:  ���º�·�����ݣ���termap_param_input�еĲ������µ�termap_param��,���㲢����terrainMap_param_used
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
void SetTerrainMapWayPoints(sGLRENDERSCENE* pScene)
{
	LP_Geo_Pt_D temp = NULL;
	int point_num = pScene->termap_param_input.way_point_num;

	/*�����ڲ�ʹ�õĺ�·���������ͼ����,������к�·�㾭γ�ߵ��ڴ�����ָ�벻ֱ�Ӹ���*/
	temp = pScene->termap_param.p_WayPoints;
	pScene->termap_param = pScene->termap_param_input;
	pScene->termap_param.p_WayPoints = temp;
	/*���ͷ�ԭ��������к�·�㾭γ�ߵ��ڴ�����*/	
	if (pScene->termap_param.p_WayPoints != NULL)
	{
		DeleteAlterableMemory(pScene->termap_param.p_WayPoints);
	}
	/*�ٸ���ʵ�ʺ�·�������������������к�·�㾭γ�ߵ��ڴ�����*/
	pScene->termap_param.p_WayPoints 
		= (LP_Geo_Pt_D)NewAlterableMemory(point_num * sizeof(Geo_Pt_D));
	/*�����к�·�㾭γ�߸��Ƶ���������ڴ�����*/
	memcpy(pScene->termap_param.p_WayPoints, 
		pScene->termap_param_input.p_WayPoints, point_num * sizeof(Geo_Pt_D));

	/*�����ڲ�����ʱʹ�õĺ�·���������ͼϸ������*/
	/*�ͷ�ԭ�����ÿ2����·��֮��Ĳ�ֵ��������ڴ�����ָ��*/
	if (pScene->terrainMap_param_used.p_Interpolation_num != NULL)
	{
		DeleteAlterableMemory(pScene->terrainMap_param_used.p_Interpolation_num);
	}
	/*�ͷ�ԭ��������к�·�����Ļ������ڴ�����ָ��*/
	if(pScene->terrainMap_param_used.p_WayPointScreen != NULL)
	{
		DeleteAlterableMemory(pScene->terrainMap_param_used.p_WayPointScreen);
	}
	/*�ͷ�ԭ��������в�ֵ��ĵ��θ߶ȵ���Ļ����(x,y)+ ���θ߶�(z)���ڴ�����ָ��*/
	if (pScene->terrainMap_param_used.p_WayPointInterpolation != NULL)
	{
		DeleteAlterableMemory(pScene->terrainMap_param_used.p_WayPointInterpolation);
	}
	
	/*���㺽�ߴ�ֱ����ͼ��x��y����ķֱ���*/
	CalcTerrainMapRatio(pScene);
	/*�������к�·�����Ļ����*/
	UpdataTerrainMapScreenPoint(pScene);
	/*�������в�ֵ�����Ļ����*/
	UpdataTerrainMapInterpolationScreenPoint(pScene);

#if 0
	/*���Դ�ӡ���в�ֵ���x����*/
	{
		int k = 0, i = 0, j = 0;
		
		for(i = 0; i < (pScene->termap_param.way_point_num - 1); i ++)
		{	
			printf("%f-", pScene->terrainMap_param_used.p_WayPointInterpolation[k].x);

			k ++;
		
			for(j = 1; j <= pScene->terrainMap_param_used.p_Interpolation_num[i] ; j ++, k ++)
			{
				printf("%f-", pScene->terrainMap_param_used.p_WayPointInterpolation[k].x);

			}
		}
		
		printf("%f-\n", pScene->terrainMap_param_used.p_WayPointInterpolation[k].x);
		printf("%d\n",k);
	}
#endif
}


 
/*.BH--------------------------------------------------------
**
** ������: RenderTerrainMap
**
** ����:  ��Ⱦ��·�㴹ֱ��������ͼ
**
** �������:  render_scene����Ⱦ�������
**
** �����������
**
** ����ֵ��TRUE or FALSE
**          
**
** ���ע��:  �ⲿ�ӿ�3
**
**.EH--------------------------------------------------------
*/
BOOL RenderTerrainMap(VOIDPtr render_scene)
{	
	f_int32_t i;
	f_int32_t viewport[4] = {0};
	f_char_t chNum[32] = {0};
	f_int16_t left, right, bottom, top;
	f_int16_t width, height;
	sColor4f textColor = {0};
	sGLRENDERSCENE *pScene = NULL;	
	f_float32_t s_StartWinPosX, s_StartWinPosY, s_WinWidth, s_WinHeight ;
	f_int32_t width_screen = 0, height_screen = 0;
  	f_float32_t half = 10.0f, x = 0.0f, y = 0.0f;

	/*�жϳ�������Ƿ���Ч,��Чֱ�ӷ���0*/
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(FALSE);	

	takeSem(pScene->terrainMap_param_used.eventparamset_sem, FOREVER_WAIT);
	/*Ĭ����ʾ�б�������������*/
	terrainMapListIdNeedRegenerate = 0;

	/*��termap_param_input�еĲ������µ�termap_param��,���㲢����terrainMap_param_used*/
	if(pScene->termap_param.isUpdata == 1)
	{
		SetTerrainMapWayPoints(pScene);
		/*��������,��ʾ�б�������������*/
		terrainMapListIdNeedRegenerate = 1;
		pScene->termap_param.isUpdata = 0;
	}

	/*��λ����ͼ���ڴ�С����*/
	s_StartWinPosX = pScene->termap_param.x;					
	s_StartWinPosY = pScene->termap_param.y;
	s_WinWidth = pScene->termap_param.width;
	s_WinHeight = pScene->termap_param.height;

	getScreenWindowSize(&width_screen, &height_screen);

	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = width_screen;
	viewport[3] = height_screen;	
	
	/*������������(�ǵ�ͼ�ӿ�)�Ŀ��ߡ����½����꼰���Ͻ�����(��λΪ����)*/
	width = viewport[2];
	height = viewport[3];
	left = viewport[0];
	right = viewport[0] + viewport[2];
	bottom = viewport[1];
	top = viewport[1] + viewport[3];
	
	/*�����ӿڴ�С*/
	glViewport(left, bottom, width, height);

	glMatrixMode(GL_PROJECTION);						// ѡ��ͶӰ����
	glPushMatrix();										// ���浱ǰ��ͶӰ����
	glLoadIdentity();									// ����ͶӰ����
	glOrtho(left, right, bottom, top, -1, 1);			//������ͶӰ�Ŀ�������
	glMatrixMode(GL_MODELVIEW);							// ѡ��ģ�ͱ任����
	glPushMatrix();										// ���浱ǰ��ģ�ͱ任����
	glLoadIdentity();									// ����ģ�ͱ任����	


	glDisableEx(GL_CULL_FACE);
	glDisableEx(GL_DEPTH_TEST);	
	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//���ƽ����
	glColor4f(0.2f, 0.2f, 0.2f, 0.6f);
	//glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
	if(pScene->terrainMap_param_used.s_TerrainMapLISTID[0] == 0)
	{
		pScene->terrainMap_param_used.s_TerrainMapLISTID[0] = glGenLists(1);
		glNewList(pScene->terrainMap_param_used.s_TerrainMapLISTID[0], GL_COMPILE);
		{
			glBegin(GL_QUADS);
			glVertex2f(s_StartWinPosX, s_StartWinPosY);
			glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY);
			glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + s_WinHeight);
			glVertex2f(s_StartWinPosX, s_StartWinPosY + s_WinHeight);
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->terrainMap_param_used.s_TerrainMapLISTID[0]);
	}
	glDisableEx(GL_BLEND);

	/*���Ʋ�ֵ�㹹�ɵĹ켣�ߺ͹켣��*/
	DrawDynamicTerrainMap(pScene);

	glColor4f(0.0, 1.0, 1.0, 0.9);
	/*���ɺ�·�������*/
	if (texture_id_terrainmap == 0)
	{
		createPngTexture("HANGDIAN.png", &texture_id_terrainmap);
	}

	glEnableEx(GL_TEXTURE_2D);
	glEnableEx(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, texture_id_terrainmap);
	/*���ƺ�·��*/	  
	if(pScene->terrainMap_param_used.s_TerrainMapLISTID[3] == 0)
	{
		pScene->terrainMap_param_used.s_TerrainMapLISTID[3] = glGenLists(1);
	}

	if(terrainMapListIdNeedRegenerate){
		glNewList(pScene->terrainMap_param_used.s_TerrainMapLISTID[3], GL_COMPILE);
		{
			for (i = 0; i < pScene->termap_param.way_point_num; i ++)
			{
				x = pScene->terrainMap_param_used.p_WayPointScreen[i].x;
				y = pScene->terrainMap_param_used.p_WayPointScreen[i].y;
     
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f); glVertex2f(x - half, y - half);
				glTexCoord2f(1.0f, 0.0f); glVertex2f(x + half, y - half);
				glTexCoord2f(1.0f, 1.0f); glVertex2f(x + half, y + half);
				glTexCoord2f(0.0f, 1.0f); glVertex2f(x - half, y + half);
				glEnd();
			}
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->terrainMap_param_used.s_TerrainMapLISTID[3]);
	}
	
	glDisableEx(GL_TEXTURE_2D);
	glDisableEx(GL_BLEND);
	
	/*�������к�·���ֱ������*/
    glLineWidth(4.0);
	glColor4f(0.0, 1.0, 1.0, 1.0);
	
	if(pScene->terrainMap_param_used.s_TerrainMapLISTID[4] == 0)
	{
		pScene->terrainMap_param_used.s_TerrainMapLISTID[4] = glGenLists(1);
	}

	if(terrainMapListIdNeedRegenerate){
		glNewList(pScene->terrainMap_param_used.s_TerrainMapLISTID[4], GL_COMPILE);
		{
			glBegin(GL_LINE_STRIP);
			for (i = 0; i < pScene->termap_param.way_point_num; i++)
			{
				x = pScene->terrainMap_param_used.p_WayPointScreen[i].x;
				y = pScene->terrainMap_param_used.p_WayPointScreen[i].y;
				glVertex2f(x,y);
			}
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->terrainMap_param_used.s_TerrainMapLISTID[4]);
	}

	/*�������к�·��Ĵ���,�Ե㻮�ߵ���ʽ����*/
	glLineWidth(2.0);
	glLineStipple(2, 0x5555);	
	glEnableEx(GL_LINE_STIPPLE);	
	glColor4f(0.0, 1.0, 1.0, 1.0);

	if(pScene->terrainMap_param_used.s_TerrainMapLISTID[5] == 0)
	{
		pScene->terrainMap_param_used.s_TerrainMapLISTID[5] = glGenLists(1);
	}

	if(terrainMapListIdNeedRegenerate){
		glNewList(pScene->terrainMap_param_used.s_TerrainMapLISTID[5], GL_COMPILE);
		{
			for (i = 0; i < pScene->termap_param.way_point_num; i++)
			 {	
				x = pScene->terrainMap_param_used.p_WayPointScreen[i].x;
				y = pScene->terrainMap_param_used.p_WayPointScreen[i].y;

				glBegin(GL_LINES);
				glVertex2f(x, pScene->termap_param.y_pass);
				glVertex2f(x, pScene->termap_param.y_pass_h);
				glEnd();
			}
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->terrainMap_param_used.s_TerrainMapLISTID[5]);
	}
	 
	glDisableEx(GL_LINE_STIPPLE);	
	glLineWidth(1.0);
#if 1
	for (i = 0; i < pScene->termap_param.way_point_num; i++)
	{	
		char stringOut[32] = { 0 };
		sColor4f textColor = { 0 };
		int text_length = 0;
		int num_i = 0;
		int num_sum = 0;
		int x = pScene->terrainMap_param_used.p_WayPointScreen[i].x;
		int y = pScene->terrainMap_param_used.p_WayPointScreen[i].y;

		/*�����·���Ӧ�Ĳ�ֵ��ĵ��θ߶���Ϣ*/
		num_sum = 0;
		for(num_i = 0; num_i < i ; num_i ++)
		{
			num_sum += pScene->terrainMap_param_used.p_Interpolation_num[num_i] + 1;
		}
		num_sum += 1;
		sprintf(stringOut, "%d", 
			(int)(pScene->terrainMap_param_used.p_WayPointInterpolation[num_sum - 1].z));
		text_length = strlen(stringOut);
		textColor.red = 50.0 / 255.0;
		textColor.green = 205.0 / 255.0;
		textColor.blue = 50.0 / 255.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, 
			pScene->terrainMap_param_used.p_WayPointInterpolation[num_sum - 1].x - text_length/2.0 * 15.0,
			pScene->terrainMap_param_used.p_WayPointInterpolation[num_sum - 1].y + 10.0, (unsigned char*)stringOut, textColor);
		
		/*���������·�����һ����·��֮��ľ�����Ϣ*/
		num_sum = 0;
		for(num_i = 0; num_i < i ; num_i ++)
		{
			num_sum += pScene->terrainMap_param_used.p_Interpolation_num[num_i] + 1;
		}
		sprintf(stringOut, "%d", (int)((num_sum)* pScene->termap_param.x_interpolation/1000.0));
		text_length = strlen(stringOut);
		textColor.red = 0.0;
		textColor.green = 1.0;
		textColor.blue = 1.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, x - text_length/2.0 * 15.0, s_StartWinPosY-15.0, (unsigned char*)stringOut, textColor);

		/*���������·��ĸ߶���Ϣ*/
		sprintf(stringOut, "%d", (int)pScene->termap_param.p_WayPoints[i].height);
		text_length = strlen(stringOut);
		textColor.red = 50.0 / 255.0;
		textColor.green = 205.0 / 255.0;
		textColor.blue = 50.0 / 255.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, x - text_length/2.0 * 15.0, y + 10.0, (unsigned char*)stringOut, textColor);

		/*���������·������,��1��ʼ���*/
		sprintf(stringOut, "%d", i+1);
		text_length = strlen(stringOut);
		textColor.red = 1.0;
		textColor.green = 1.0;
		textColor.blue = 1.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, x - text_length/2.0 * 15.0, s_StartWinPosY + s_WinHeight + 15.0, (unsigned char*)stringOut, textColor);

	}

	{

		char stringOut[32] = { 0 };
		int text_length = 0;
		/*���x��ĵ�λkm*/
		sprintf(stringOut, "D(km)");
		textColor.red = 0.0;
		textColor.green = 1.0;
		textColor.blue = 1.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, 
			s_StartWinPosX + s_WinWidth + 10.0, 
			s_StartWinPosY - 15.0, (unsigned char*)stringOut, textColor);

		/*���y��ĵ�λm*/
		sprintf(stringOut, "H(m)");
		text_length = strlen(stringOut);
		textColor.red = 50.0 / 255.0;
		textColor.green = 205.0 / 255.0;
		textColor.blue = 50.0 / 255.0;
		textColor.alpha = 1.0;

		textOutPutOnScreen(0, 
			s_StartWinPosX - text_length*15.0,
			s_StartWinPosY + s_WinHeight, 
			 (unsigned char*)stringOut, textColor);
	}
#endif

				
	glMatrixMode(GL_PROJECTION);						// ѡ��ͶӰ����
	glPopMatrix();										// ����Ϊ����ľ���
	glMatrixMode(GL_MODELVIEW);							// ѡ��ģ�;���
	glPopMatrix();										// ����Ϊ����ľ���

	giveSem(pScene->terrainMap_param_used.eventparamset_sem);

	/*�ָ��ӿھ���*/
	if(!getInnerViewPort(pScene, viewport))
		return FALSE;

	width = viewport[2];
	height = viewport[3];
	left = viewport[0];
	right = viewport[0] + viewport[2];
	bottom = viewport[1];
	top = viewport[1] + viewport[3];
		
	glViewport(left, bottom, width, height);

	return TRUE;
}


/*.BH--------------------------------------------------------
**
** ������: SetTerrainMap
**
** ����:  ���ⲿ����ĺ�·���������ͼ�������µ�������
**
** �������:  render_scene����Ⱦ�������
**            text�� �ⲿ����ĺ�·���������ͼ����
**
** �����������
**
** ����ֵ��TRUE or FALSE
**          
**
** ���ע��:  �ⲿ�ӿ�2
**
**.EH--------------------------------------------------------
*/
int SetTerrainMap(VOIDPtr render_scene, TerrainMapContext  text)
{
	sGLRENDERSCENE *pScene = NULL;
	LP_Geo_Pt_D temp = NULL;
    
	pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return FALSE;

	takeSem(pScene->terrainMap_param_used.eventparamset_sem, FOREVER_WAIT);

	/*�����ⲿ���յĺ�·���������ͼ����,������к�·�㾭γ�ߵ��ڴ�����ָ�벻ֱ�Ӹ���*/
	temp = pScene->termap_param_input.p_WayPoints;
	pScene->termap_param_input = text;
	pScene->termap_param_input.p_WayPoints = temp;
	/*���ͷ�ԭ��������к�·�㾭γ�ߵ��ڴ�����*/	
	if (pScene->termap_param_input.p_WayPoints != NULL)
	{
		DeleteAlterableMemory(pScene->termap_param_input.p_WayPoints);
	}
	/*�ٸ���ʵ�ʺ�·�������������������к�·�㾭γ�ߵ��ڴ�����*/
	pScene->termap_param_input.p_WayPoints 
		= (LP_Geo_Pt_D)NewAlterableMemory(text.way_point_num * sizeof(Geo_Pt_D));
	/*�����к�·�㾭γ�߸��Ƶ���������ڴ�����*/
	memcpy(pScene->termap_param_input.p_WayPoints, 
		text.p_WayPoints, text.way_point_num * sizeof(Geo_Pt_D));
	/*�ڲ�ʹ�õĺ�·���������ͼ�������±�־��Ϊ1,����termap_param_input�еĲ������µ�termap_param*/
	pScene->termap_param.isUpdata = 1;	
	
	giveSem(pScene->terrainMap_param_used.eventparamset_sem);
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: InitTerrainMap
**
** ����: ��ʼ����·�㴹ֱ��������ͼ(����ֻ��Ҫ��ʼ��һ�εĲ���)
**
** �������:  render_scene����Ⱦ�������
**
** �����������
**
** ����ֵ��TRUE or FALSE
**          
**
** ���ע��:  �ⲿ�ӿ�1
**
**.EH--------------------------------------------------------
*/
int InitTerrainMap(VOIDPtr render_scene)
{
	f_uint32_t size = 32;
	sGLRENDERSCENE *pScene = NULL;

	/*�жϳ�������Ƿ���Ч,��Чֱ�ӷ���*/
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return FALSE;
	
	/*Ԥ�ȷ����ڲ�����ʱʹ�õĺ�·���������ͼϸ�������е��ڴ�����,Ĭ�ϴ�С,��������ʵ�ʺ�·���������������ڴ�ռ�*/
	pScene->terrainMap_param_used.p_Interpolation_num = (int*)NewAlterableMemory(sizeof(int));
	pScene->terrainMap_param_used.p_WayPointScreen = (LP_PT_2F)NewAlterableMemory(sizeof(PT_2F));
	pScene->terrainMap_param_used.p_WayPointInterpolation = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F));
	/*Ԥ�ȷ����ڲ�ʹ�õĺ�·���������ͼ�����еĴ�ź�·����ڴ�����,Ĭ�ϴ�С,��������ʵ�ʺ�·���������������ڴ�ռ�*/
	pScene->termap_param.p_WayPoints = (LP_Geo_Pt_D)NewAlterableMemory(sizeof(Geo_Pt_D));

	return TRUE;	
}
