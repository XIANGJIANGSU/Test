/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2018年
**
** 文件名: TerrainMap.c
**
** 描述: 本文件包含航路点垂直地形剖面图及相关的函数。
**
** 定义的函数:  
**
**                            
** 设计注记: 
**
** 作者：
**		LPF。
** 
**
** 更改历史:
**		2017-6-26 9:06 LPF 创建此文件
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** 头文件引用
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
** 文字量和宏声明
**-----------------------------------------------------------
*/
extern TTFONT GetHandleFont();
extern f_int32_t createPngTexture(f_int8_t* filename, f_int32_t* texture_id);

/*-----------------------------------------------------------
** 函数声明
**-----------------------------------------------------------
*/



/*-----------------------------------------------------------
** 全局变量定义
**-----------------------------------------------------------
*/
static float y_start = 0.0;	//y轴起始点的海拔高度, 地形最低点或航点最低点
static float y_end = 0.0;		//y轴终止点的海拔高度，地形最高点或航点最高点
int texture_id_terrainmap = 0;  //贴图ID
static f_int32_t terrainMapListIdNeedRegenerate = 0;  //显示列表是否需要更新,0-不需要,1-需要
/*-----------------------------------------------------------
** 函数定义
**-----------------------------------------------------------
*/


/*
*	绘制色带，整个剖面图绘制一个矩形即可
*/
void static DrawQuadsTerrainEx(sGLRENDERSCENE* pScene)
{
	f_float32_t s_height_pass = pScene->termap_param.y_pass;
	f_float32_t s_height_pass_h = pScene->termap_param.y_pass_h;
	f_float32_t s_width_pass = pScene->termap_param.x_pass;
	f_float32_t s_width_pass_h = pScene->termap_param.x_pass_h;

	/*绘制矩形色带*/
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
** 函数名: DrawDynamicTerrainMap
**
** 描述:  在屏幕上绘制动态的地形
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口1,采用模板缓冲区的方法绘制三角形扇
**
**.EH--------------------------------------------------------
*/
static void DrawDynamicTerrainMap(sGLRENDERSCENE* pScene)
{

#if 1
	/*绘制三角形扇,采用模板缓冲区的方式*/
	/* 开启模板测试，默认不开启，模板测试发生在透明度测试之后，深度测试之前*/
    glEnableEx(GL_STENCIL_TEST);
	/* 模板缓冲区中的值先与参数3(0x1)进行与操作,结果与参数2(0x1)进行比较,比较函数为参数1. GL_NEVER表示模板测试永远不通过*/
	/* 模板测试不通过则表示对应像素点的颜色值不会更新*/
	glStencilFunc(GL_NEVER, 0x1, 0x1);
	/*第一个参数sfail： 如果模板测试失败将采取的动作。
	  第二个参数dpfail： 如果模板测试通过，但是深度测试失败时采取的动作。
	  第三个参数dppass： 如果深度测试和模板测试都通过，将采取的动作。
	  GL_INVERT表示模板值按位取反
    */
	glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

#if 0	
	//绘制多边形扇面
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
		/*采用显示列表的绘制方式*/
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

	
	/* 重绘屏幕,只绘制模板缓存值非0的像素,色带只会在三角扇区域显示 */

	/* 模板缓冲区中的值先与参数3(0x1)进行与操作,结果与参数2(0x1)进行比较,比较函数为参数1. GL_EQUAL表示进行相等比较*/
	/* 模板测试不通过则表示对应像素点的颜色值不会更新*/	
	glStencilFunc(GL_EQUAL,0x1,0x1);	
	/*GL_ZERO表示模板值设置为0*/
	glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO); 

	//绘制色带
    glColor4f(50.0 / 255.0, 205 / 255.0, 50.0 / 255.0, 1.0f);
 	DrawQuadsTerrainEx(pScene);

	/* 设置模板测试永远通过 */
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
** 函数名: GetTerrainMapPointHei
**
** 描述:  计算指定经纬度的高度,若原始高程数据中查询不到,则从瓦片数据中查
**
** 输入参数:  pScene：渲染场景句柄
**            lon_dest：经度
**            lat_dest：纬度
**
** 输出参数：无
**
** 返回值：查询到的高度
**          
**
** 设计注记:  内部接口
**
**.EH--------------------------------------------------------
*/
f_float32_t GetTerrainMapPointHei(sGLRENDERSCENE*pScene, f_float64_t lon_dest,  f_float64_t lat_dest)
{
	f_int16_t hei_dest = 0;
	f_float64_t pz = 0.0;
	
	/* 根据经纬度从原始高程数据中查询高程值 */
	if(FALSE == getAltByGeoPos( lon_dest, lat_dest, &hei_dest))
	{
		/* 根据经纬度从内存中载入的瓦片数据中查询高程值 */
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
** 函数名: CalcTerrainMapRatio
**
** 描述:  计算航线垂直剖面图的x和y方向的分辨率、每2个航路点之间插值点个数、所有航路点和插值点的高度、最大高度及最小高度
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口
**
**.EH--------------------------------------------------------
*/
void CalcTerrainMapRatio(sGLRENDERSCENE* pScene)
{
	double dis_sum = 0.0;	//距离的总和
	double dis = 0.0;
	int i = 0 , j = 0, k = 0;
	int Interpolation_num = 0;
	float z_hei = 0.0;

	/*根据实际航路点数量重新申请存放每2个航路点之间的插值点个数的内存区域*/
	pScene->terrainMap_param_used.p_Interpolation_num 
		= (int*)NewAlterableMemory(sizeof(int) * pScene->termap_param.way_point_num);
	/*循环访问所有航路点*/
	for(i = 0; i < (pScene->termap_param.way_point_num - 1); i ++)
	{	
		/*计算相邻2个航路点的大圆距离*/
		dis = caculateGreatCircleDis(
			pScene->termap_param.p_WayPoints[i].lon,
			pScene->termap_param.p_WayPoints[i].lat,
			pScene->termap_param.p_WayPoints[i + 1].lon,
			pScene->termap_param.p_WayPoints[i + 1].lat);
		/*计算相邻2个航路点之间的插值点个数*/
		pScene->terrainMap_param_used.p_Interpolation_num[i] 
			= (int)(dis /pScene->termap_param.x_interpolation);
		/*计算总的插值点个数*/
		Interpolation_num += pScene->terrainMap_param_used.p_Interpolation_num[i] ;
		/*计算各段航路的距离总和*/
		dis_sum += dis;
	}
	/*实际全部插值点=计算得到的插值点+航路点*/
	pScene->terrainMap_param_used.Interpolation_sum 
		= pScene->termap_param.way_point_num + Interpolation_num;
	/*根据实际航路点数量重新申请存放所有插值点的屏幕坐标(x,y)+ 地形高度(z)的内存区域*/
	pScene->terrainMap_param_used.p_WayPointInterpolation
		= (LP_PT_3F)NewAlterableMemory(pScene->terrainMap_param_used.Interpolation_sum * sizeof(PT_3F));

	/*更新全部插值点的高度*/
	y_end = y_start = pScene->termap_param.p_WayPoints[i].height;	
	/*依次访问各个航路点*/
	for(i = 0; i < (pScene->termap_param.way_point_num - 1); i ++)
	{	
		/*查询各个航路点的高度,作为与航路点经纬度相同的插值点高度(不使用其原始高度),并更新到指定结构体中*/
		z_hei = GetTerrainMapPointHei(pScene, 
				pScene->termap_param.p_WayPoints[i].lon,
				pScene->termap_param.p_WayPoints[i].lat);
		pScene->terrainMap_param_used.p_WayPointInterpolation[k].z = z_hei;
		/*求出所有航路点和插值点中最低高度*/
		if(y_start > z_hei)
		{
			y_start = z_hei;
		}
		if(y_start > pScene->termap_param.p_WayPoints[i].height)
		{
			y_start = pScene->termap_param.p_WayPoints[i].height;
		}
		/*求出所有航路点和插值点中最高高度*/
		if(y_end < z_hei)
		{
			y_end = z_hei;
		}
		if(y_end < pScene->termap_param.p_WayPoints[i].height)
		{
			y_end = pScene->termap_param.p_WayPoints[i].height;
		}

		
		k++;
		/*依次处理各个插值点*/
		for(j = 1; j <= pScene->terrainMap_param_used.p_Interpolation_num[i] ; j++, k ++)
		{
			/*查询各个插值点的高度,并更新到指定结构体中*/	
			z_hei = GetTerrainMapPointHei(pScene, 
					pScene->termap_param.p_WayPoints[i].lon 
					+ j * (pScene->termap_param.p_WayPoints[i+1].lon -  pScene->termap_param.p_WayPoints[i].lon )/(pScene->terrainMap_param_used.p_Interpolation_num[i] + 1),
					pScene->termap_param.p_WayPoints[i].lat 
					+ j * (pScene->termap_param.p_WayPoints[i+1].lat -  pScene->termap_param.p_WayPoints[i].lat)/(pScene->terrainMap_param_used.p_Interpolation_num[i] + 1)
					);
			pScene->terrainMap_param_used.p_WayPointInterpolation[k].z = z_hei;
			/*求出所有航路点和插值点中最低高度*/
			if(y_start > z_hei)
			{
				y_start = z_hei;
			}
			/*求出所有航路点和插值点中最高高度*/
			if(y_end < z_hei)
			{
				y_end = z_hei;
			}		
		}
	}
	
	/*查询最后一个航路点的高度,作为与航路点经纬度相同的插值点高度(不使用其原始高度),并更新到指定结构体中*/
	z_hei = GetTerrainMapPointHei(pScene, 
		pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].lon,
		pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].lat);
	pScene->terrainMap_param_used.p_WayPointInterpolation[k].z	= z_hei;

	/*求出所有航路点和插值点中最低高度*/
	if(y_start > z_hei)
	{
		y_start = z_hei;
	}
	if(y_start > pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height)
	{
		y_start = pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height;
	}

	/*求出所有航路点和插值点中最高高度*/
	if(y_end < z_hei)
	{
		y_end = z_hei;
	}
	if(y_end < pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height)
	{
		y_end = pScene->termap_param.p_WayPoints[pScene->termap_param.way_point_num - 1].height;
	}
	
	/*为了显示效果更明显,将最大高度增加200m,最小高度减小200m*/
	y_end += 200.0f;
	y_start -= 200.0f;

	/*计算x方向1像素所代表的实际距离*/
	pScene->termap_param.x_ratio = 
		dis_sum / (pScene->termap_param.x_pass_h -pScene->termap_param.x_pass);

	/*计算y方向1像素所代表的实际高度值*/
	pScene->termap_param.y_ratio =
		(y_end - y_start) / (pScene->termap_param.y_pass_h -pScene->termap_param.y_pass);
}


/*.BH--------------------------------------------------------
**
** 函数名: UpdataTerrainMapScreenPoint
**
** 描述:  计算所有航路点的屏幕坐标
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口
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

	/*根据实际航路点数量重新申请存放所有航路点的屏幕坐标的内存区域*/
	pScene->terrainMap_param_used.p_WayPointScreen
		= (LP_PT_2F)NewAlterableMemory(num * sizeof(PT_2F));

	/*计算第一个航路点屏幕坐标*/
	pScene->terrainMap_param_used.p_WayPointScreen[0].x = pScene->termap_param.x_pass;
	pScene->terrainMap_param_used.p_WayPointScreen[0].y 
		= (pScene->termap_param.p_WayPoints[0].height - y_start)/y_ratio
			+ pScene->termap_param.y_pass;

	x_sum = pScene->terrainMap_param_used.p_WayPointScreen[0].x;
	/*依次处理其余航路点*/
	for(i = 1; i < num; i ++)
	{	
		/*计算其余航路点屏幕坐标,x方向:计算与前一个航路点的大圆距离,再由距离计算出像素坐标*/
		x = caculateGreatCircleDis(
			pScene->termap_param.p_WayPoints[i-1].lon,
			pScene->termap_param.p_WayPoints[i-1].lat,
			pScene->termap_param.p_WayPoints[i].lon,
			pScene->termap_param.p_WayPoints[i].lat)/x_ratio;
		/*y方向：计算与最低高度的差值,再由差值计算出像素坐标*/
		y = (pScene->termap_param.p_WayPoints[i].height - y_start)/y_ratio
			+ pScene->termap_param.y_pass;
		
		x_sum +=x;

		/*后续航路点的x方向坐标等于之前的累加值*/
		pScene->terrainMap_param_used.p_WayPointScreen[i].x = x_sum;
		pScene->terrainMap_param_used.p_WayPointScreen[i].y = y;
	}
}

/*.BH--------------------------------------------------------
**
** 函数名: UpdataTerrainMapInterpolationScreenPoint
**
** 描述:  计算所有插值点的屏幕坐标
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口
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

	/*计算所有插值点的屏幕坐标*/
	for(i = 0; i < (num - 1); i ++)
	{	
		/*计算与航路点经纬度相同的插值点的屏幕坐标*/
		y_hei = pScene->terrainMap_param_used.p_WayPointInterpolation[k].z;
		/*y方向：计算与最低高度的差值,再由差值计算出像素坐标*/
		pScene->terrainMap_param_used.p_WayPointInterpolation[k].y	
			= (y_hei - y_start)/y_ratio+ pScene->termap_param.y_pass;
		/*x方向：与航路点的像素坐标相同*/
		pScene->terrainMap_param_used.p_WayPointInterpolation[k].x	
			= pScene->terrainMap_param_used.p_WayPointScreen[i].x;

		k++;
		/*依次处理航路点之间的插值点*/
		for(j = 1; j <= pScene->terrainMap_param_used.p_Interpolation_num[i] ; j ++, k ++)
		{
			/*y方向：计算与最低高度的差值,再由差值计算出像素坐标*/
			y_hei = pScene->terrainMap_param_used.p_WayPointInterpolation[k].z;
			pScene->terrainMap_param_used.p_WayPointInterpolation[k].y	
				= (y_hei - y_start)/y_ratio+ pScene->termap_param.y_pass;
			/*x方向：计算两个航路点之间的像素距离,均分成插值点+1等份,前一个航路点坐标增加对应等份的像素距离即可*/
			pScene->terrainMap_param_used.p_WayPointInterpolation[k].x	
				= (pScene->terrainMap_param_used.p_WayPointScreen[i+1].x - 
				pScene->terrainMap_param_used.p_WayPointScreen[i].x) 
				/(pScene->terrainMap_param_used.p_Interpolation_num[i] + 1) * j
				+ pScene->terrainMap_param_used.p_WayPointScreen[i].x;
		}
	}
	
	/*计算与航路点经纬度相同的插值点的屏幕坐标*/
	y_hei = pScene->terrainMap_param_used.p_WayPointInterpolation[k].z;
	/*y方向：计算与最低高度的差值,再由差值计算出像素坐标*/
	pScene->terrainMap_param_used.p_WayPointInterpolation[k].y	
		= (y_hei - y_start)/y_ratio+ pScene->termap_param.y_pass;
	/*x方向：与航路点的像素坐标相同*/
	pScene->terrainMap_param_used.p_WayPointInterpolation[k].x	
			= pScene->terrainMap_param_used.p_WayPointScreen[i].x;

}



/*.BH--------------------------------------------------------
**
** 函数名: SetTerrainMapWayPoints
**
** 描述:  更新航路点数据，将termap_param_input中的参数更新到termap_param中,计算并更新terrainMap_param_used
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口
**
**.EH--------------------------------------------------------
*/
void SetTerrainMapWayPoints(sGLRENDERSCENE* pScene)
{
	LP_Geo_Pt_D temp = NULL;
	int point_num = pScene->termap_param_input.way_point_num;

	/*更新内部使用的航路点地形剖面图参数,存放所有航路点经纬高的内存区域指针不直接复制*/
	temp = pScene->termap_param.p_WayPoints;
	pScene->termap_param = pScene->termap_param_input;
	pScene->termap_param.p_WayPoints = temp;
	/*先释放原来存放所有航路点经纬高的内存区域*/	
	if (pScene->termap_param.p_WayPoints != NULL)
	{
		DeleteAlterableMemory(pScene->termap_param.p_WayPoints);
	}
	/*再根据实际航路点数量重新申请存放所有航路点经纬高的内存区域*/
	pScene->termap_param.p_WayPoints 
		= (LP_Geo_Pt_D)NewAlterableMemory(point_num * sizeof(Geo_Pt_D));
	/*将所有航路点经纬高复制到新申请的内存区域*/
	memcpy(pScene->termap_param.p_WayPoints, 
		pScene->termap_param_input.p_WayPoints, point_num * sizeof(Geo_Pt_D));

	/*更新内部绘制时使用的航路点地形剖面图细化参数*/
	/*释放原来存放每2个航路点之间的插值点个数的内存区域指针*/
	if (pScene->terrainMap_param_used.p_Interpolation_num != NULL)
	{
		DeleteAlterableMemory(pScene->terrainMap_param_used.p_Interpolation_num);
	}
	/*释放原来存放所有航路点的屏幕坐标的内存区域指针*/
	if(pScene->terrainMap_param_used.p_WayPointScreen != NULL)
	{
		DeleteAlterableMemory(pScene->terrainMap_param_used.p_WayPointScreen);
	}
	/*释放原来存放所有插值点的地形高度的屏幕坐标(x,y)+ 地形高度(z)的内存区域指针*/
	if (pScene->terrainMap_param_used.p_WayPointInterpolation != NULL)
	{
		DeleteAlterableMemory(pScene->terrainMap_param_used.p_WayPointInterpolation);
	}
	
	/*计算航线垂直剖面图的x和y方向的分辨率*/
	CalcTerrainMapRatio(pScene);
	/*计算所有航路点的屏幕坐标*/
	UpdataTerrainMapScreenPoint(pScene);
	/*计算所有插值点的屏幕坐标*/
	UpdataTerrainMapInterpolationScreenPoint(pScene);

#if 0
	/*测试打印所有插值点的x坐标*/
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
** 函数名: RenderTerrainMap
**
** 描述:  渲染航路点垂直地形剖面图
**
** 输入参数:  render_scene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：TRUE or FALSE
**          
**
** 设计注记:  外部接口3
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

	/*判断场景句柄是否有效,无效直接返回0*/
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return(FALSE);	

	takeSem(pScene->terrainMap_param_used.eventparamset_sem, FOREVER_WAIT);
	/*默认显示列表无需重新生成*/
	terrainMapListIdNeedRegenerate = 0;

	/*将termap_param_input中的参数更新到termap_param中,计算并更新terrainMap_param_used*/
	if(pScene->termap_param.isUpdata == 1)
	{
		SetTerrainMapWayPoints(pScene);
		/*参数更新,显示列表无需重新生成*/
		terrainMapListIdNeedRegenerate = 1;
		pScene->termap_param.isUpdata = 0;
	}

	/*定位剖面图窗口大小参数*/
	s_StartWinPosX = pScene->termap_param.x;					
	s_StartWinPosY = pScene->termap_param.y;
	s_WinWidth = pScene->termap_param.width;
	s_WinHeight = pScene->termap_param.height;

	getScreenWindowSize(&width_screen, &height_screen);

	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = width_screen;
	viewport[3] = height_screen;	
	
	/*计算整个窗口(非地图视口)的宽、高、左下角坐标及右上角坐标(单位为像素)*/
	width = viewport[2];
	height = viewport[3];
	left = viewport[0];
	right = viewport[0] + viewport[2];
	bottom = viewport[1];
	top = viewport[1] + viewport[3];
	
	/*设置视口大小*/
	glViewport(left, bottom, width, height);

	glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
	glPushMatrix();										// 保存当前的投影矩阵
	glLoadIdentity();									// 重置投影矩阵
	glOrtho(left, right, bottom, top, -1, 1);			//设置正投影的可视区域
	glMatrixMode(GL_MODELVIEW);							// 选择模型变换矩阵
	glPushMatrix();										// 保存当前的模型变换矩阵
	glLoadIdentity();									// 重置模型变换矩阵	


	glDisableEx(GL_CULL_FACE);
	glDisableEx(GL_DEPTH_TEST);	
	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//绘制界面框
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

	/*绘制插值点构成的轨迹线和轨迹面*/
	DrawDynamicTerrainMap(pScene);

	glColor4f(0.0, 1.0, 1.0, 0.9);
	/*生成航路点的纹理*/
	if (texture_id_terrainmap == 0)
	{
		createPngTexture("HANGDIAN.png", &texture_id_terrainmap);
	}

	glEnableEx(GL_TEXTURE_2D);
	glEnableEx(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, texture_id_terrainmap);
	/*绘制航路点*/	  
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
	
	/*绘制所有航路点的直线连线*/
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

	/*绘制所有航路点的垂线,以点划线的形式绘制*/
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

		/*输出航路点对应的插值点的地形高度信息*/
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
		
		/*输出各个航路点与第一个航路点之间的距离信息*/
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

		/*输出各个航路点的高度信息*/
		sprintf(stringOut, "%d", (int)pScene->termap_param.p_WayPoints[i].height);
		text_length = strlen(stringOut);
		textColor.red = 50.0 / 255.0;
		textColor.green = 205.0 / 255.0;
		textColor.blue = 50.0 / 255.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, x - text_length/2.0 * 15.0, y + 10.0, (unsigned char*)stringOut, textColor);

		/*输出各个航路点的序号,从1开始编号*/
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
		/*输出x轴的单位km*/
		sprintf(stringOut, "D(km)");
		textColor.red = 0.0;
		textColor.green = 1.0;
		textColor.blue = 1.0;
		textColor.alpha = 1.0;
		textOutPutOnScreen(0, 
			s_StartWinPosX + s_WinWidth + 10.0, 
			s_StartWinPosY - 15.0, (unsigned char*)stringOut, textColor);

		/*输出y轴的单位m*/
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

				
	glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
	glPopMatrix();										// 设置为保存的矩阵
	glMatrixMode(GL_MODELVIEW);							// 选择模型矩阵
	glPopMatrix();										// 设置为保存的矩阵

	giveSem(pScene->terrainMap_param_used.eventparamset_sem);

	/*恢复视口矩阵*/
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
** 函数名: SetTerrainMap
**
** 描述:  将外部传入的航路点地形剖面图参数更新到引擎中
**
** 输入参数:  render_scene：渲染场景句柄
**            text： 外部传入的航路点地形剖面图参数
**
** 输出参数：无
**
** 返回值：TRUE or FALSE
**          
**
** 设计注记:  外部接口2
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

	/*更新外部接收的航路点地形剖面图参数,存放所有航路点经纬高的内存区域指针不直接复制*/
	temp = pScene->termap_param_input.p_WayPoints;
	pScene->termap_param_input = text;
	pScene->termap_param_input.p_WayPoints = temp;
	/*先释放原来存放所有航路点经纬高的内存区域*/	
	if (pScene->termap_param_input.p_WayPoints != NULL)
	{
		DeleteAlterableMemory(pScene->termap_param_input.p_WayPoints);
	}
	/*再根据实际航路点数量重新申请存放所有航路点经纬高的内存区域*/
	pScene->termap_param_input.p_WayPoints 
		= (LP_Geo_Pt_D)NewAlterableMemory(text.way_point_num * sizeof(Geo_Pt_D));
	/*将所有航路点经纬高复制到新申请的内存区域*/
	memcpy(pScene->termap_param_input.p_WayPoints, 
		text.p_WayPoints, text.way_point_num * sizeof(Geo_Pt_D));
	/*内部使用的航路点地形剖面图参数更新标志置为1,即将termap_param_input中的参数更新到termap_param*/
	pScene->termap_param.isUpdata = 1;	
	
	giveSem(pScene->terrainMap_param_used.eventparamset_sem);
	
	return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: InitTerrainMap
**
** 描述: 初始化航路点垂直地形剖面图(更新只需要初始化一次的参数)
**
** 输入参数:  render_scene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：TRUE or FALSE
**          
**
** 设计注记:  外部接口1
**
**.EH--------------------------------------------------------
*/
int InitTerrainMap(VOIDPtr render_scene)
{
	f_uint32_t size = 32;
	sGLRENDERSCENE *pScene = NULL;

	/*判断场景句柄是否有效,无效直接返回*/
    pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return FALSE;
	
	/*预先分配内部绘制时使用的航路点地形剖面图细化参数中的内存区域,默认大小,后面会根据实际航路点数量重新申请内存空间*/
	pScene->terrainMap_param_used.p_Interpolation_num = (int*)NewAlterableMemory(sizeof(int));
	pScene->terrainMap_param_used.p_WayPointScreen = (LP_PT_2F)NewAlterableMemory(sizeof(PT_2F));
	pScene->terrainMap_param_used.p_WayPointInterpolation = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F));
	/*预先分配内部使用的航路点地形剖面图参数中的存放航路点的内存区域,默认大小,后面会根据实际航路点数量重新申请内存空间*/
	pScene->termap_param.p_WayPoints = (LP_Geo_Pt_D)NewAlterableMemory(sizeof(Geo_Pt_D));

	return TRUE;	
}
