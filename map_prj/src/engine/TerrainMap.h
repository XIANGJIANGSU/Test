/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2018年
**
** 文件名: TerrainMap.c
**
** 描述: 本文件包含航路点垂直地形剖面图及相关函数的头文件。
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
**		2018-7-2 11:04 LPF 创建此文件
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** 头文件引用
**-----------------------------------------------------------
*/
#ifndef _TERRAINMAP_H_
#define _TERRAINMAP_H_


#include "../mapApp/appHead.h"

/*-----------------------------------------------------------
** 结构体声明
**-----------------------------------------------------------
*/

/*航路垂直剖面图设置接口结构体*/
typedef struct tagTerrainMapContext
{
	int isShow;			//0:不显示，1:显示
	int	isUpdata;		//0:不更新, 1:更新

	float x,y,width,height;	//航剖图显示的左下角坐标、宽度、高度,相对于屏幕左下角
	float x_ratio, y_ratio;	//单位是米每像素
	
	float x_pass, y_pass;	//显示范围的水平x轴最近点、垂直y轴最低点
	float y_pass_h;		    //显示范围的垂直y轴最高点，单位像素
	float x_pass_h;		    //显示范围的水平x轴最远点，单位像素
	
	float x_interpolation;	//间隔多少m插入一个插值点,默认是5000

	int way_point_num;      //航路点个数
	LP_Geo_Pt_D p_WayPoints;	//存放外部输入的航路点经纬高的内存区域指针
}TerrainMapContext, *pTerrainMapContext;


/*航路垂直剖面图内部使用结构体*/
typedef struct tagTerrainMapUsed
{
	SEMID eventparamset_sem;	
	int s_TerrainMapLISTID[10] ;		//存储的显示列表
	int Interpolation_sum;			    //全部的插值点和航路点的个数和
	int *p_Interpolation_num;		    //指针,存放每2个航路点之间的插值点个数
	LP_PT_2F p_WayPointScreen;	        //指针,存放所有航路点的屏幕坐标
	LP_PT_3F p_WayPointInterpolation;	//指针,存放所有插值点的地形高度的屏幕坐标(x,y)+ 地形高度(z)	
}TerrainMapUsed, *pTerrainMapUsed;


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
int SetTerrainMap(VOIDPtr render_scene, TerrainMapContext  text);

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
BOOL RenderTerrainMap(VOIDPtr render_scene);

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
int InitTerrainMap(VOIDPtr render_scene);


#endif
