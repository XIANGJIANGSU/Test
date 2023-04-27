/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: TerrainPrefile.c
**
** 描述: 本文件包含垂直地形剖面图VSD及相关的函数。
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
//#include "../../vecMapEngine_prj/2dEngine/vecMapFont.h"

/*-----------------------------------------------------------
** 文字量和宏声明
**-----------------------------------------------------------
*/

#ifdef WIN32
#define SPANPICPATH "../地图数据/DataFile/SpanPic.bmp"
#else
#define SPANPICPATH "/ahci00/mapData3d/DataFile/SpanPic.bmp"
#endif



/*-----------------------------------------------------------
** 全局变量定义
**-----------------------------------------------------------
*/
extern VOIDPtr pMapDataSubTree;

//记录上一次飞机的经纬高及航向,2路分别记录
static f_float64_t AirPlane_oldpos[2][4] = {{0.0},{0.0}};

//记录上一次水平方向每米多少像素、垂直方向每米多少像素
static f_float64_t s_HWinPerMeter_old[2] = {0.0,0.0};
static f_float64_t s_VWinPerMeter_old[2] = {0.0,0.0};

//是否使用瓦片高度,0-不使用,1-使用
static f_int8_t use_hei_tile = 0;  

//记录上一时刻瓦片加载的数量,当使用瓦片高度时,会根据该变量判断瓦片是否更新,从而决定是否更新剖面图中的高度值
static f_int32_t loadedTileNumber_old = 0;

static f_int32_t listIdNeedRegenerate[2] = {0,0};
/*-----------------------------------------------------------
** 函数定义
**-----------------------------------------------------------
*/

extern int textOutPutOnScreen(int fontType,float viewportX, float viewportY, unsigned char* stringOut, sColor4f textColor);

/*
 *	设置投影矩阵和模型变换矩阵
 *	世界坐标转为屏幕坐标
 */
void PushWorldCoord2CliendCoord(f_float64_t left, f_float64_t right, f_float64_t bottom, f_float64_t top)
{
	glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
	glPushMatrix();										// 保存当前的投影矩阵
	glLoadIdentity();									// 重置投影矩阵
	glOrtho(left, right, bottom, top, -1, 1);			//设置正投影的可视区域
	glMatrixMode(GL_MODELVIEW);							// 选择模型变换矩阵
	glPushMatrix();										// 保存当前的模型变换矩阵
	glLoadIdentity();									// 重置模型变换矩阵	
}

/*
 *	恢复投影矩阵和模型变换矩阵
 *	屏幕坐标转为世界坐标
 */
void PopWorldCoord2CliendCoord()
{
	glMatrixMode(GL_PROJECTION);						// 选择投影矩阵
	glPopMatrix();										// 设置为保存的矩阵
	glMatrixMode(GL_MODELVIEW);							// 选择模型矩阵
	glPopMatrix();										// 设置为保存的矩阵
}

/*
*	绘制色带，根据不同的色带宽度
*/
// void static DrawColorQuads(	float color_area[4] , int hei_low, int hei_hi)
// {
// 	glColor4fv(color_area);
// 	glBegin(GL_QUADS);
// 	glVertex2i(s_StartWinPosX, s_StartWinPosY+hei_low);
// 	glVertex2i(s_StartWinPosX + s_WinWidth, s_StartWinPosY+hei_low);
// 	glVertex2i(s_StartWinPosX + s_WinWidth, s_StartWinPosY + hei_hi);
// 	glVertex2i(s_StartWinPosX, s_StartWinPosY + hei_hi);
// 	glEnd();
// 
// }

/*.BH--------------------------------------------------------
**
** 函数名: DrawQuadsEx
**
** 描述:  在屏幕上绘制一个矩形色带
**
** 输入参数:  hei_low：矩形色带下边垂直方向距离左下角的像素值
**            hei_hi：矩形色带上边垂直方向距离左下角的像素值
**            pScene：渲染场景句柄
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
static void DrawQuadsEx(f_float32_t hei_low, f_float32_t hei_hi, sGLRENDERSCENE* pScene)
{
	/*从VSD参数结构体中获取VSD窗口起始位置、宽度和长度(单位为像素)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	/*绘制矩形色带*/
	glBegin(GL_QUADS);
	glVertex2f(s_StartWinPosX, s_StartWinPosY + hei_low);
	glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + hei_low);
	glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + hei_hi);
	glVertex2f(s_StartWinPosX, s_StartWinPosY + hei_hi);
	glEnd();

}


/*.BH--------------------------------------------------------
**
** 函数名: DrawQuardTerrain
**
** 描述:  根据预设的告警色带颜色值,在VSD的矩形框中绘制多条矩形色带
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
static void  DrawQuardTerrain(sGLRENDERSCENE* pScene)
{
	/*从VSD参数结构体中获取VSD窗口起始位置、宽度和长度(单位为像素)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	f_float32_t s_winHeiMeter = 1000.0f;
	f_float32_t prefile_height[5] = {0.0f};
	f_float32_t fly[5] = {0.0f};
	f_int32_t color_num = 6;
	f_int32_t i = 0;

	/*从VSD参数结构体中获取垂直方向半屏代表实际距离(米)、色带的数量*/
	s_winHeiMeter = pScene->auxnavinfo_param.prefile_param.s_winHeiMeter * 1.0f;
	color_num = pScene->auxnavinfo_param.prefile_param.color_num;

	/*色带数量有效判断,[2,6]有效*/
	if( (color_num < 2) || (color_num > 6))
	{
		//printf("input color_num error!\n");
		return;
	}

	/*获取色带区间的高度,高度的数量为色带数量-1*/
	for(i = 0;i < color_num-1; i++)
	{
		prefile_height[i] = pScene->auxnavinfo_param.prefile_param.prefile_height[i];
	}


	/* 告警颜色层数为2时，计算色带的分界高度，并绘制色带 */
	/*
	  高度范围				                                RGB						VSD窗口高度范围
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight			
	  -s_winHeiMeter -- 	prefile_height[0]			prefile_color[1]		  0	-- fly[0]
	*/
	if(2 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
		(prefile_height[0] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(0, fly[0], pScene);

	}

	/* 告警颜色层数为3时，计算色带的分界高度，并绘制色带 */
	/*
	  高度范围				                                RGB						VSD窗口高度范围
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]		
	  -s_winHeiMeter -- 	prefile_height[1]			prefile_color[2]		  0	-- fly[1]
	*/
	if(3 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
		    (prefile_height[1] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(0, fly[1], pScene);

	}

	/* 告警颜色层数为4时，计算色带的分界高度，并绘制色带 */
	/*
	  高度范围				                                RGB						VSD窗口高度范围
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]	
	  prefile_height[2] -- 	prefile_height[1]			prefile_color[2]		  fly[2]--fly[1]	
	  -s_winHeiMeter -- 	prefile_height[2]			prefile_color[3]		  0	-- fly[2]
	*/
	if(4 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
			(prefile_height[2] > prefile_height[1]) ||
		    (prefile_height[2] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(fly[2], fly[1], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
		DrawQuadsEx(0, fly[2], pScene);
	}

	/* 告警颜色层数为5时，计算色带的分界高度，并绘制色带 */
	/*
	  高度范围				                                RGB						VSD窗口高度范围
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]	
	  prefile_height[2] -- 	prefile_height[1]			prefile_color[2]		  fly[2]--fly[1]
	  prefile_height[3] -- 	prefile_height[2]			prefile_color[3]		  fly[3]--fly[2]	
	  -s_winHeiMeter -- 	prefile_height[3]			prefile_color[4]		  0	-- fly[4]
	*/
	if(5 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
			(prefile_height[2] > prefile_height[1]) ||
			(prefile_height[3] > prefile_height[2]) ||
		    (prefile_height[3] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(fly[2], fly[1], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
		DrawQuadsEx(fly[3], fly[2], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[4][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][3]);
		DrawQuadsEx(0, fly[3], pScene);
	}

	/* 告警颜色层数为6时，计算色带的分界高度，并绘制色带 */
	/*
	  高度范围				                                RGB						VSD窗口高度范围
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]	
	  prefile_height[2] -- 	prefile_height[1]			prefile_color[2]		  fly[2]--fly[1]
	  prefile_height[3] -- 	prefile_height[2]			prefile_color[3]		  fly[3]--fly[2]	
	  prefile_height[4] -- 	prefile_height[3]			prefile_color[4]		  fly[4]--fly[3]
	  -s_winHeiMeter -- 	prefile_height[4]			prefile_color[5]		  0	-- fly[4]
	*/
	if(6 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
			(prefile_height[2] > prefile_height[1]) ||
			(prefile_height[3] > prefile_height[2]) ||
			(prefile_height[4] > prefile_height[3]) ||
		    (prefile_height[4] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(fly[2], fly[1], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
		DrawQuadsEx(fly[3], fly[2], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[4][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][3]);
		DrawQuadsEx(fly[4], fly[3], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[5][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][3]);
		DrawQuadsEx(0, fly[4], pScene);
	}


#if 0 //告警层数固定的绘制方法,已废弃
	//绘制色带,直接绘制,不再使用显示列表
#if 0
	
	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][3]);

	if(pScene->prefile_param_used.s_QuardLISTID[0] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[0] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[0], GL_COMPILE);
		{
			DrawQuadsEx(0, fly1,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[0]);
	}	

	


	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[1] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[1] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[1], GL_COMPILE);
		{
			DrawQuadsEx(fly1, fly2,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[1]);
	}	

	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[2] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[2] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[2], GL_COMPILE);
		{
			DrawQuadsEx(fly2, fly3,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[2]);
	}	

	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[3] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[3] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[3], GL_COMPILE);
		{
			DrawQuadsEx(fly3, fly4,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[3]);
	}	

	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[4] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[4] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[4], GL_COMPILE);
		{
			DrawQuadsEx(fly4, fly5,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[4]);
	}	
	
	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[5] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[5] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[5], GL_COMPILE);
		{
			DrawQuadsEx(fly5, s_WinHeight,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[5]);
	}	

#else
	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[5][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][3]);
	DrawQuadsEx(0, fly[0], pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[4][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][3]);
	DrawQuadsEx(fly1, fly2, pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
	DrawQuadsEx(fly2, fly3, pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
	DrawQuadsEx(fly3, fly4, pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
	DrawQuadsEx(fly4, fly5, pScene);
	
	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
	DrawQuadsEx(fly5, s_WinHeight, pScene);

#endif

#endif

}

/*.BH--------------------------------------------------------
**
** 函数名: DrawDynamicHeightMap
**
** 描述:  在屏幕上绘制动态的地形
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口,采用模板缓冲区的方法绘制三角形扇
**
**.EH--------------------------------------------------------
*/
static void DrawDynamicHeightMap(sGLRENDERSCENE* pScene)
{
	f_int32_t i;
	f_float32_t cur_posx;

	/*从VSD参数结构体中获取VSD窗口起始位置、宽度和长度(单位为像素)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	f_float32_t s_winHeiMeter = pScene->auxnavinfo_param.prefile_param.s_winHeiMeter * 1.0f;


	/*更新三角扇的顶点数组,总共303个点,其中第1个为起始点,第2个点与起始点水平方向坐标相同但垂直方向坐标不同,
	中间采样点299个,倒数第2个点与终点水平方向坐标相同但垂直方向坐标不同,最后一个点为终点*/
    /*起始点*/
	pScene->prefile_param_used.s_vDetectHeights_cal[0] = s_StartWinPosX;
	pScene->prefile_param_used.s_vDetectHeights_cal[1] = s_StartWinPosY;

	/*中间301个点*/
	for(i = 0; i <= SAMPLE_NUM; ++i)
	{
		/*计算水平方向坐标,即每个点距离起始点的实际距离(m)*水平方向每米像素值+起始点水平方向坐标*/
		cur_posx = s_StartWinPosX + (pScene->prefile_param_used.s_HWinPerMeter * i * pScene->prefile_param_used.s_WorldSampleInterval);

		pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2] = cur_posx;

		/*计算垂直方向坐标,先计算每个点高度与起始点高度之间的差值(m),起始点高度为飞机高度-1000(这个1000是固定值,暂时外部无法改变)*/
		/*再按差值*垂直方向每米像素值+起始点垂直方向坐标,即可得到结果*/
		/*超过窗口范围，取边框值即可*/
		if((pScene->prefile_param_used.s_VWinPerMeter * (pScene->prefile_param_used.s_vDetectHeights[i] - (pScene->geopt_pos.height - s_winHeiMeter)))> s_WinHeight)
		{
			pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = (s_StartWinPosY + s_WinHeight);	
		}
		else if((pScene->prefile_param_used.s_VWinPerMeter * (pScene->prefile_param_used.s_vDetectHeights[i] - (pScene->geopt_pos.height - s_winHeiMeter)))< 0)
		{
			pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = (s_StartWinPosY + 0);
		}
		else
		{
			pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = s_StartWinPosY + ( pScene->prefile_param_used.s_VWinPerMeter * 
				(pScene->prefile_param_used.s_vDetectHeights[i] - (pScene->geopt_pos.height - s_winHeiMeter)) );
		}
	}

	/*终点*/
	pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2] = s_StartWinPosX + s_WinWidth;
	pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = s_StartWinPosY;
			
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

	/*默认模板值为0,模板测试必然不通过,则三角扇不会显示，但绘制了三角扇的区域模板值为0xff(0按位取反)*/
#if 0	
	/*绘制多边形扇面,采用顶点数组的方式*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_DOUBLE, 0, &pScene->prefile_param_used.s_vDetectHeights_cal[0]);

	glDrawElements(GL_TRIANGLE_FAN, (SAMPLE_NUM + 3), GL_UNSIGNED_INT, &pScene->prefile_param_used.pIndex_tri_fan[0]);	

	glDisableClientState(GL_VERTEX_ARRAY);

#else

#if 1
	{
		/*绘制三角扇面,采用直接绘制的方式,共303个点,pc下严重影响帧率,7200soc实测影响不大*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];
		glBegin(GL_TRIANGLE_FAN);
		for (i=0; i<SAMPLE_NUM + 3; ++i)
		{
			glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
		}
		glEnd();
	}
#else
	{
		/*采用显示列表的绘制方式,7200下如果频繁重新生成显示列表，驱动会报错pvtxbuf malloc failed*/
	    /*但是如果需要频繁创建显示列表会导致闪屏，所以还是采用直接绘制*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];

		if(pScene->prefile_param_used.s_TerrainPrefileLISTID[5] == 0)
		{
			pScene->prefile_param_used.s_TerrainPrefileLISTID[5] = glGenLists(1);
		}

		if(listIdNeedRegenerate[pScene->scene_index]){
			glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[5], GL_COMPILE);
			{
				glBegin(GL_TRIANGLE_FAN);
				for (i=0; i<SAMPLE_NUM + 3; ++i)
				{
					glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
				}
				glEnd();
			}
			glEndList();
		}
		else
		{
			glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[5]);
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
	DrawQuardTerrain(pScene);

	/* 设置模板测试永远通过 */
	glStencilFunc(GL_ALWAYS,0,0x1);

}

/*.BH--------------------------------------------------------
**
** 函数名: UpdataTerrainProfile
**
** 描述:  更新垂直地形剖面图的需动态计算的参数,获取飞机航线上前向探测距离内的各个采样点高度值
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  内部接口, 获取飞机航线上前向探测距离内的各个采样点高度值
**
**.EH--------------------------------------------------------
*/
void UpdataTerrainProfile(sGLRENDERSCENE* pScene)
{
	f_int32_t i;
	f_uint32_t index;
//	f_float64_t sample_step;				            //目标的插值点距离
	f_float64_t lon_dest, lat_dest; 	                //目标点的经纬度
	f_int16_t hei_dest;                                 //目标点的高度
//	f_float64_t hei_dest_dom;					        //瓦片里的高度
	f_float64_t lon_final = 0.0, lat_final = 0.0;	    //终点的经纬度
	f_float64_t lon_step, lat_step;						//经纬度的间隔
	f_float32_t s_WinWidth =  0;
	f_float32_t s_WinHeight = 0;
	sMAPHANDLE * pHandle = NULL;                        //场景树句柄,全局唯一
	
	if(pMapDataSubTree == NULL)
	{
		return;
	}
	pHandle = (sMAPHANDLE *)pMapDataSubTree;

	index = pScene->scene_index;
	if (index > 1)
	{
		return;
	}

	/*获取外部设置的VSD窗口的宽度和高度*/
	s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	if(pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance < 1000.0)
	{
		//printf("input s_WorldDetectDistance must > 1km!\n");
		return;
	}

	if(pScene->auxnavinfo_param.prefile_param.s_winHeiMeter <= 0)
	{
		//printf("input s_winHeiMeter must > 0!\n");
		return;
	}

	/*默认不用重新生成显示列表*/
	listIdNeedRegenerate[index] = 0;

	/*根据外部设置的VSD参数，计算并更新到内部使用的VSD参数结构体prefile_param_used中*/
	/*计算VSD窗口水平方向两个刻度线之间的宽度(单位为像素)*/
	pScene->prefile_param_used.s_WinInterval = s_WinWidth / pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum;
	/*计算VSD窗口水平方向的采样间距(单位为m),默认300个采样点,最后一个采样点为终点*/
	pScene->prefile_param_used.s_WorldSampleInterval = pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance / SAMPLE_NUM;
	/*计算VSD窗口水平方向缩放比例,即实际距离1m代表多少个像素*/
	pScene->prefile_param_used.s_HWinPerMeter = (f_float64_t)s_WinWidth / pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance;
	//计算垂直方向缩放比例,即实际距离1米代表多少像素
	pScene->prefile_param_used.s_VWinPerMeter =(f_float64_t)( (s_WinHeight/2.0) / (pScene->auxnavinfo_param.prefile_param.s_winHeiMeter*1.0) );
	
	/*如果不使用瓦片高度*/
	if(!use_hei_tile){
		/* 根据经纬度获取高程值,首先查询原始DEM数据,未查询到再使用瓦片高度 */
		if(FALSE == getAltByGeoPos( pScene->geopt_pos.lon, pScene->geopt_pos.lat, &hei_dest))
		{
			/*使用瓦片高度*/
			use_hei_tile = 1;
		}
	}

	//每隔5km重新计算一次、飞机高度发生变化(大于0.1)或者飞机航向角发生变化(大于0.01)时重新计算一次
	if(	(fabs(s_HWinPerMeter_old[index] - pScene->prefile_param_used.s_HWinPerMeter) < FLT_EPSILON)
		&&(fabs(s_VWinPerMeter_old[index] - pScene->prefile_param_used.s_VWinPerMeter) < FLT_EPSILON)
		&&(fabs(AirPlane_oldpos[index][2] - pScene->geopt_pos.height) < 0.1)
		&&(fabs(AirPlane_oldpos[index][3] - pScene->attitude.yaw) < 0.01)
		&&(caculateGreatCircleDis(
			AirPlane_oldpos[index][0], 
			AirPlane_oldpos[index][1], 
			(f_float32_t)pScene->geopt_pos.lon, 
			(f_float32_t)pScene->geopt_pos.lat) < 5000) )
	{
		/*如果使用瓦片高度,当瓦片加载数量发生变化时也要重新查询高度*/
		if(use_hei_tile)
		{
			/*瓦片加载数量未变化,则无需重新查询高度,否则记录该时刻的瓦片加载数量,并重新查询高度*/
			if((loadedTileNumber_old == pHandle->loadedNumber))
			{
				return;
			}else{
				loadedTileNumber_old = pHandle->loadedNumber;
			}
		}else{
			return;
		}
	}
	else
	{
		/*记录当前飞机经纬高及航向*/
		AirPlane_oldpos[index][0] = pScene->geopt_pos.lon;
		AirPlane_oldpos[index][1] = pScene->geopt_pos.lat;	
		AirPlane_oldpos[index][2] = pScene->geopt_pos.height;
		AirPlane_oldpos[index][3] = pScene->attitude.yaw;	 

		/*记录当前水平方向缩放比例及垂直方向缩放比例*/
		s_HWinPerMeter_old[index] = pScene->prefile_param_used.s_HWinPerMeter;
		s_VWinPerMeter_old[index] = pScene->prefile_param_used.s_VWinPerMeter;
	}

	/*重新生成显示列表*/
	listIdNeedRegenerate[index] = 1;

	//printf("index=%d %f %f %f %f\n",index,AirPlane_oldpos[index][0],AirPlane_oldpos[index][1],AirPlane_oldpos[index][2],AirPlane_oldpos[index][3]);
	//printf("%f %f\n",s_HWinPerMeter_old[index],s_VWinPerMeter_old[index]);

//	printf("更新垂直地形剖面图的高程数据\n");

	//计算终点经纬度
//	rhumbLineDistanceAzimuth(
//		(f_float32_t)pScene->geopt_pos.lon, 
//		(f_float32_t)pScene->geopt_pos.lat,
//		(f_float32_t)(360.0 - pScene->attitude.yaw),
//		(f_float32_t)pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance,
//		&lon_final, &lat_final);

	{
		double temp_lon = 0.0, temp_lat = 0.0;
		//根据当前飞机经纬度和航向，计算最远探测距离的对应点的经纬度
		caculateLonLatOnGreateCircle(pScene->geopt_pos.lon, 
			pScene->geopt_pos.lat, 
			pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance,
			(360.0 - pScene->attitude.yaw), &temp_lon, &temp_lat);

		lon_final = temp_lon;
		lat_final = temp_lat;
		
		//printf("%f,%f,%f\n", lon_final, temp_lon, temp_lon - lon_final);
		//printf("%f,%f,%f\n", lat_final, temp_lat, temp_lat - lat_final);

	}

	//计算采样点之间的经纬度间隔
	lon_step = (lon_final - pScene->geopt_pos.lon)/SAMPLE_NUM;
	lat_step = (lat_final - pScene->geopt_pos.lat)/SAMPLE_NUM;

	// 获取飞机前向探测方向上的各个采样点的经纬高,
	// 共301个点,第1个采样点与当前飞机位置经纬度相同但高度为地形高度、中间299个采样点、
	// 最后1个采样点与终点位置经纬度相同但高度为地形高度
	for(i = 0; i <= SAMPLE_NUM; ++i)
	{
//		sample_step = i * s_WorldSampleInterval;
//		rhumbLineDistanceAzimuth(g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLon, 
//			g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLat,
//			g_pAirPlanes[g_nSelectPlane].m_Entity.m_nYaw,
//			sample_step,
//			&lon_dest, &lat_dest);

		// 获取飞机前向探测方向上的各个采样点的经纬,直接采用平均分的方法,
		// 该计算方法不是真实的大圆航线,不够准确
		lon_dest = (pScene->geopt_pos.lon + i * lon_step);
		lat_dest = (pScene->geopt_pos.lat + i *lat_step);


		/* 根据经纬度获取高程值,首先查询原始DEM数据,未查询到再使用瓦片高度(存在较大误差) */
		if(FALSE == getAltByGeoPos( lon_dest, lat_dest, &hei_dest))
		{
			double pz = 0.0;

			/*查询瓦片高度,查询失败,高度默认为0,这个值选取不合理,0是一个真实存在的高度*/
			if(FALSE == GetZ((sGLRENDERSCENE*)pScene, lon_dest, lat_dest, &pz))
			{
				hei_dest = 0;
			}
			else
			{
				hei_dest = (short)pz;
			}
			
			//获取的高度为瓦片高度时,状态设置为1,高度点绘制的颜色为红色
			pScene->prefile_param_used.s_vDetectHeights_state[i] = 1;	

			pScene->prefile_param_used.s_vDetectHeight_color[4*i] = 1.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 1] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 2] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 3] = 1.0;
			
		}
		else
		{
			//获取的高度为原始DEM高度时,状态设置为0,高度点绘制的颜色为黑色
			pScene->prefile_param_used.s_vDetectHeights_state[i] = 0;

			pScene->prefile_param_used.s_vDetectHeight_color[4*i] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 1] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 2] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 3] = 1.0;
		}

		//保存查询到的各个采样点的高度
		pScene->prefile_param_used.s_vDetectHeights[i] = (double)hei_dest;

	}

}

/*.BH--------------------------------------------------------
**
** 函数名: InitTriangleFanAndQuadIndex
**
** 描述:  初始化VSD窗口绘制的三角扇索引和四边形索引
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
void InitTriangleFanAndQuadIndex(sGLRENDERSCENE* pScene)
{
	int k = 0;
	/*三角扇顶点索引,共303个点,包含301个三角形*/
	for(k=0; k<(SAMPLE_NUM + 3); k++)
	{
		pScene->prefile_param_used.pIndex_tri_fan[k] = k;
	}
	/*四边形顶点索引,共4个点,包含1个四边形*/
	for(k=0; k<4; k++)
	{
		pScene->prefile_param_used.pIndex_quad[k] = k;
	}

}




/*.BH--------------------------------------------------------
**
** 函数名: InitTerrainProfile
**
** 描述:  初始化垂直地形剖面图(VSD-Vertical Situation Display)(更新只需要初始化一次的参数)
**
** 输入参数:  render_scene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口
**
**.EH--------------------------------------------------------
*/
void InitTerrainProfile(VOIDPtr render_scene)
{
    sGLRENDERSCENE *pScene = NULL;

	/*判断场景句柄是否有效,无效直接返回*/
	pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;

	/*初始化VSD窗口绘制的顶点数组索引*/
	InitTriangleFanAndQuadIndex(pScene);

	//初始化量程档位图片
	//LoadParticlesTexture(SPANPICPATH, &SpantextureID[0]);
}
 
/*.BH--------------------------------------------------------
**
** 函数名: RenderTerrainProfile
**
** 描述:  渲染垂直地形剖面图VSD
**
** 输入参数:  pScene：渲染场景句柄
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口
**
**.EH--------------------------------------------------------
*/
BOOL RenderTerrainProfile(sGLRENDERSCENE* pScene)
{
	f_int32_t viewport[4];
	f_int32_t i;
	f_char_t chNum[32] = {0};
	f_int16_t left, right, bottom, top;
	f_int16_t width, height;

	f_char_t textInfo[32] = {0};
	f_int32_t textLength = 0;
	sColor4f textColor = {0};
	f_float64_t hIntervalDistance = 0.0;

	/*每个绘制周期会通过函数setEventParam更新VSD参数，从auxnavinfo_param_input结构体更新到auxnavinfo_param结构体*/
	/*从VSD参数结构体中获取VSD窗口起始位置、宽度和长度(单位为像素)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;					
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	/*判断场景句柄是否有效,无效直接返回0*/
	if(NULL == pScene)
		return FALSE;	

	/*初始化VSD相关参数,引擎初始化的时候已初始化,只初始化一次,此处不再*/
	//InitTerrainProfile(pScene);

	/*剖面图的位置由视口相对位置改为屏幕相对位置,因为VSD有可能不绘制在地图视口内*/
#if 1
	
	if(!getInnerViewPort(pScene, viewport))
	    return FALSE;

	//startX = viewPort[0]; startY = viewPort[1]; width = viewPort[2]; height = viewPort[3];
#else
	{
		f_int32_t width_screen = 0, height_screen = 0;
		getScreenWindowSize(&width_screen, &height_screen);

		viewport[0] = 0;
		viewport[1] = 0;
		viewport[2] = width_screen;
		viewport[3] = height_screen;	
	}

#endif

	/*计算整个窗口(非地图视口)的宽、高、左下角坐标及右上角坐标(单位为像素)*/
	width = viewport[2];
	height = viewport[3];
	left = viewport[0];
	right = viewport[0] + viewport[2];
	bottom = viewport[1];
	top = viewport[1] + viewport[3];
	
	/*更新垂直地形剖面图的需动态计算的参数*/
	UpdataTerrainProfile(pScene);

	/*设置视口大小*/
	glViewport(left, bottom, width, height);
	/*设置投影矩阵(正投影)和模式变换矩阵*/
	//PushWorldCoord2CliendCoord(left*1.0, width*1.0, bottom*1.0, height*1.0);
	PushWorldCoord2CliendCoord(0.0, width*1.0, bottom*1.0, height*1.0);

	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisableEx(GL_CULL_FACE);
	glDisableEx(GL_DEPTH_TEST);

	/*使能线图元的平滑着色,避免线段呈现锯齿状*/
	glEnableEx(GL_LINE_SMOOTH);

	/*绘制VSD的界面框，默认为矩形*/
	glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
#if 0
	/*为了实现框的位置和大小动态可变，不用显示列表的方式绘制，但帧率影响较大*/
	glBegin(GL_QUADS);
		glVertex2f(s_StartWinPosX, s_StartWinPosY);
		glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY);
		glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + s_WinHeight);
		glVertex2f(s_StartWinPosX, s_StartWinPosY + s_WinHeight);
	glEnd();
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[0] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[0] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[0], GL_COMPILE);
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
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[0]);
	}
#endif

	/*绘制VSD的轨迹面*/
	DrawDynamicHeightMap(pScene);

	/*绘制VSD的四周边框线*/
	glColor3f(1.0f, 1.0f, 1.0f);
#if 0
	/*外框扩大1个像素*/
	glBegin(GL_LINE_LOOP);
		glVertex2f(s_StartWinPosX-1, s_StartWinPosY-1);
		glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY -1);
		glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY + s_WinHeight + 1);
		glVertex2f(s_StartWinPosX - 1, s_StartWinPosY + s_WinHeight + 1);
	glEnd();
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[1] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[1] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[1], GL_COMPILE);
		{
			/*外框扩大1个像素*/
			glBegin(GL_LINE_LOOP);
			glVertex2f(s_StartWinPosX-1, s_StartWinPosY-1);
			glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY -1);
			glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY + s_WinHeight + 1);
			glVertex2f(s_StartWinPosX - 1, s_StartWinPosY + s_WinHeight + 1);
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[1]);
	}
	
#endif

	/*绘制VSD的水平方向刻度线,刻度线为竖线,高度为10个像素*/
#if 0
	for (i = 1; i < pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum; ++i)
	{
		glBegin(GL_LINES);
		glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY);
		glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY + 10);
		glEnd();
	}
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[2] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[2] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[2], GL_COMPILE);
		{
			for (i = 1; i < pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum; ++i)
			{
				glBegin(GL_LINES);
				glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY);
				glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY + 10);
				glEnd();
			}
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[2]);
	}
#endif

	/*绘制VSD的飞机高度线,水平方向的虚线,长度为30像素,间隔10像素,线在界面框中间*/
	glLineWidth(1.0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#if 0
	glBegin(GL_LINES);
	for (i = 0; i < s_WinWidth; i += 40)
	{
		glVertex2f(s_StartWinPosX + (i) , 		s_StartWinPosY + s_WinHeight /2);
		glVertex2f(s_StartWinPosX + (i)  + 30, 	s_StartWinPosY + s_WinHeight /2);
	}
	glEnd();
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[3] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[3] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[3], GL_COMPILE);
		{
			glBegin(GL_LINES);
			for (i = 0; i < s_WinWidth; i += 40)
			{
				glVertex2i(s_StartWinPosX + (i) , 		s_StartWinPosY + s_WinHeight /2);
				glVertex2i(s_StartWinPosX + (i)  + 30, 	s_StartWinPosY + s_WinHeight /2);
			}
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[3]);
	}
#endif

	/*关闭混合,后续绘制均不透明*/
	glDisableEx(GL_BLEND);
	/*绘制301个采样点的连线*/
	glLineWidth(1.0);
	
#if 0	
	/*采用顶点数组的方式,帧率影响较大*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, pScene->prefile_param_used.s_vDetectHeight_color);

	//不绘制第一个点和最后一个点
	glVertexPointer(2, GL_DOUBLE, 0, &pScene->prefile_param_used.s_vDetectHeights_cal[2]);
	glDrawElements(GL_LINE_STRIP, (SAMPLE_NUM + 1), GL_UNSIGNED_INT, &pScene->prefile_param_used.pIndex_tri_fan[0]);	

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#else

#if 1
	{
		/*采用直接绘制的方式,帧率影响较大*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLfloat * pCol = &pScene->prefile_param_used.s_vDetectHeight_color[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];
		glBegin(GL_LINE_STRIP);
		for (i=0; i<SAMPLE_NUM + 1; ++i)
		{
			glColor4f(pCol[4 * pIndex[i]], pCol[4 * pIndex[i] + 1], pCol[4 * pIndex[i] + 2], pCol[4 * pIndex[i] + 3]);
			glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
		}
		glEnd();
	}
#else

	{
		/*采用显示列表的绘制方式，但是如果需要频繁创建显示列表会导致闪屏，所以还是采用直接绘制*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLfloat     *pCol = &pScene->prefile_param_used.s_vDetectHeight_color[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];

		if(pScene->prefile_param_used.s_TerrainPrefileLISTID[4] == 0)
		{
			pScene->prefile_param_used.s_TerrainPrefileLISTID[4] = glGenLists(1);
		}

		if(listIdNeedRegenerate[pScene->scene_index]){
			glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[4], GL_COMPILE);
			{
				glBegin(GL_LINE_STRIP);
				for (i=0; i<SAMPLE_NUM + 1; ++i)
				{
					glColor4f(pCol[4 * pIndex[i]], pCol[4 * pIndex[i] + 1], pCol[4 * pIndex[i] + 2], pCol[4 * pIndex[i] + 3]);
					glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
				}
				glEnd();
			}
			glEndList();
		}
		else
		{
			glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[4]);
		}

	}
	
#endif

#endif

	
	// 绘制VSD垂直方向上3个高度注记,分别是：左中为飞机当前高度,左下为飞机高度-s_winHeiMeter,
	// 左上为飞机高度+s_winHeiMeter
#if 1
	
	memset(textInfo,0,sizeof(textInfo));

	textColor.red = 1.0; textColor.blue=1.0; textColor.green = 1.0; textColor.alpha = 1.0;
		
	sprintf(textInfo, "%d", (int)pScene->geopt_pos.height);
	textLength = strlen(textInfo);
	textOutPutOnScreen(0, s_StartWinPosX - textLength * 15, s_StartWinPosY + s_WinHeight /2.0 - 7.5, (unsigned char*)textInfo, textColor);		

	sprintf(textInfo, "%d", (int)pScene->geopt_pos.height  - pScene->auxnavinfo_param.prefile_param.s_winHeiMeter);
	textLength = strlen(textInfo);
	textOutPutOnScreen(0, s_StartWinPosX - textLength * 15, s_StartWinPosY, (unsigned char*)textInfo, textColor);		

	sprintf(textInfo, "%d", (int)pScene->geopt_pos.height + pScene->auxnavinfo_param.prefile_param.s_winHeiMeter);
	textLength = strlen(textInfo);	
	textOutPutOnScreen(0, s_StartWinPosX - textLength * 15, s_StartWinPosY + s_WinHeight - 7.5, (unsigned char*)textInfo, textColor);	
	
#else

//	if ((g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei > -1000)&&(g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei < 9000))
	{
		// 1.更新纹理坐标
		f_float32_t low_coord = 0.0, hi_coord = 0.0;

		//因为侧面量程是-1000 至9000, 显示量程范围是-1000 + 飞机，+1000 + 飞机

		//首先计算飞机所在的纹理坐标, 上下的纹理坐标分别加减0.1
		
		low_coord = (g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei + 1000)/10000 - 0.1 /*+ 0.013*/;
		hi_coord = (g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei + 1000)/10000 + 0.1 /*+ 0.013*/;

		// 2.绘制带纹理的长方形
		glEnableEx(GL_TEXTURE_2D);
		glEnableEx(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, SpantextureID[0]);

		
		glBegin(GL_QUADS);
			glTexCoord2f(0.1,low_coord);		glVertex2i(s_StartWinPosX-35, s_StartWinPosY);
			glTexCoord2f(0.8,low_coord);		glVertex2i(s_StartWinPosX-5 , s_StartWinPosY);
			glTexCoord2f(0.8,hi_coord);			glVertex2i(s_StartWinPosX-5 , s_StartWinPosY + s_WinHeight);
			glTexCoord2f(0.1,hi_coord);			glVertex2i(s_StartWinPosX-35,s_StartWinPosY + s_WinHeight);
		glEnd();


		glDisableEx(GL_BLEND);
		glDisableEx(GL_TEXTURE_2D);


	}
#endif


	/*绘制VSD水平方向刻度线的文字注记,刻度值为刻度线位置到起始位置的实际距离(km),终点处绘制距离单位注记km*/
	for (i = 0; i < pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum; ++i)
	{
		//清空文字注记数组
		memset(textInfo,0,sizeof(textInfo));
		//设置文字注记颜色,始终为白色
		textColor.red = 1.0; textColor.blue=1.0; textColor.green = 1.0; textColor.alpha = 1.0;
		//计算水平方向刻度线对应的距离
		hIntervalDistance = (pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance/pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum * i / 1000);
		//将距离转换成文字注记,整数小数分别区分处理
		if( (fabs(hIntervalDistance-(int)hIntervalDistance)) < FLT_EPSILON )
		{
			sprintf(textInfo, "%d", (int)hIntervalDistance);
		}else{
			sprintf(textInfo, "%.1f", hIntervalDistance);
		}
		//获取文字注记长度
		textLength = strlen(textInfo);
		//绘制文字注记,采用左上角是原点
		textOutPutOnScreen(0, 
			(s_StartWinPosX +  i * pScene->prefile_param_used.s_WinInterval - textLength/2.0 * 15.0), 
			(s_StartWinPosY - 20), (unsigned char*)textInfo, textColor);	
		
// 		printf("%d,%d,%d,%s,%f\n",i,(int)(s_StartWinPosX + ( i ) * pScene->prefile_param_used.s_WinInterval - s_WinWidth * 0.02), 
// 			(int)(s_StartWinPosY - s_WinHeight * 0.10),timeInfo,pScene->prefile_param_used.s_WinInterval);
	}	
	//绘制距离单位的文字注记,默认为km
	memset(textInfo,0,sizeof(textInfo));
	strcpy(textInfo,"(km)");
	textLength = strlen(textInfo);
	textOutPutOnScreen(0, 
		(s_StartWinPosX + ( pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum ) * pScene->prefile_param_used.s_WinInterval - textLength/2.0 * 15), 
		(s_StartWinPosY - 20), (unsigned char*)textInfo, textColor);		

	//glEnableEx(GL_CULL_FACE);
	//glEnableEx(GL_DEPTH_TEST);	


	/*恢复投影矩阵和模型变换矩阵*/
	PopWorldCoord2CliendCoord();

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
** 函数名: GetHandleFont
**
** 描述:  获取三维数据中字体句柄
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:  外部接口
**
**.EH--------------------------------------------------------
*/
TTFONT GetHandleFont()
{
	if(pMapDataSubTree == NULL)
		return NULL;
	else
	{
		sMAPHANDLE * pHandle = (sMAPHANDLE *)pMapDataSubTree;
		return pHandle->ttf_font.font;
	}
		
}
