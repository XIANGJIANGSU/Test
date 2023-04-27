#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#include <math.h>

#ifdef WIN32
#include <memory.h>	
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#include "GL/freeglut.h"
#else
#include "esUtil.h"
#endif
#endif

#include "../mapApp/appHead.h"
#include "../mapApi/common.h"
#include "../engine/Roam.h"

#ifndef WIN32
/* Mouse buttons. */
#define GLUT_LEFT_BUTTON		0
#define GLUT_MIDDLE_BUTTON		1
#define GLUT_RIGHT_BUTTON		2

/* Mouse button  state. */
#define GLUT_DOWN			0
#define GLUT_UP				1
#endif


static float MAP_WIDTH, MAP_HEIGHT;

void touchYaw(int state, float yaw);

/*.BH--------------------------------------------------------
**
** 函数名: BOOL touchRoamXY()
**
** 描述:  移动操作
**
** 输入参数:  
**	state ：0 不开启移动，1：开启移动
**  x, y：漫游的屏幕坐标，屏幕左上角为原点
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口1
**
**.EH--------------------------------------------------------
*/
void touchRoamXY(int state, float x, float y )
{

	static int state_mouse = 0;

	if(state == 1)
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, x, y);
		RoamMouseFunc(GLUT_LEFT_BUTTON, GLUT_DOWN, x, y);

		RoamMotionFunc(x, y);
	}
	else
	{


		RoamMouseFunc(GLUT_LEFT_BUTTON, GLUT_UP, x, y);

	
	}

	state_mouse = state;

/*
	if(state == 0)
	{
		//RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, x, y);
		RoamMouseFunc(0, 1, x, y);		
	}
	else
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, x, y);
		RoamMouseFunc(0, 0, x, y);			
	}
	*/
}

/*.BH--------------------------------------------------------
**
** 函数名: BOOL touchScale()
**
** 描述:  缩放操作
**
** 输入参数:  
**	state ：0：不开启缩放，1：开启缩放
**  scale：0：缩小，1：放大
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口2
**
**.EH--------------------------------------------------------
*/
void touchScale(int state, float scale)
{
	if(state == 0)
		return;

	//touchYaw(0, 0.0);
		
	if(state == 0)
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, 1, (int)(MAP_HEIGHT / 2.0) );
	}
	else
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, 1, (int)(MAP_HEIGHT / 2.0) );
	

	}


	if(scale < 0.0001)
	{
		SetZDelta(0.0001);		
	}
	else
	{
		SetZDelta(scale);
	}

	
}

/*.BH--------------------------------------------------------
**
** 函数名: BOOL touchYaw()
**
** 描述:  偏航角旋转操作
**
** 输入参数:  
**	state ：0：不开启偏航角旋转，1：开启偏航角旋转
**  yaw：0-360，顺时针为正
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口3
**
**.EH--------------------------------------------------------
*/
void touchYaw(int state, float yaw)
{
	/*if(state == 0)
	{
		RoamMouseFunc(2, 1, 1, (int)(MAP_HEIGHT / 2.0) );
	}
	else
	{
		RoamMouseFunc(2, 0, 1, (int)(MAP_HEIGHT / 2.0) );
	
		SetOutYaw(yaw);		

	}*/

	static int state_old = 0;



	if(state_old == 0)
	{
		if(state == 1)
		{
			RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, 1, (int)(MAP_HEIGHT / 2.0) );
		}
	}

	state_old = state;
	

	if(state_old == 0)
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, 1, (int)(MAP_HEIGHT / 2.0) );
	}


	SetOutYaw(yaw);	
	
}

/*.BH--------------------------------------------------------
**
** 函数名: BOOL touchPitch()
**
** 描述:  俯仰角旋转操作
**
** 输入参数:  
**	state ：0：不开启俯仰角旋转，1：开启俯仰角旋转
**  pitch：(0,-90)，向上为正
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口4
**
**.EH--------------------------------------------------------
*/
void touchPitch(int state, float pitch)
{

/*	if(state == 0)
	{
		RoamMouseFunc(2, 1, 1, (int)(MAP_HEIGHT / 2.0) );
	}
	else
	{
		RoamMouseFunc(2, 0, 1, (int)(MAP_HEIGHT / 2.0) );
	
		SetOutPitch(pitch);		

	}*/

	if(state == 0)
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_UP, 1, (int)(MAP_HEIGHT / 2.0) );
	}
	else
	{
		RoamMouseFunc(GLUT_RIGHT_BUTTON, GLUT_DOWN, 1, (int)(MAP_HEIGHT / 2.0) );
	}
	SetOutPitch(pitch);	
	
}

/*.BH--------------------------------------------------------
**
** 函数名: BOOL touchRotPointXY()
**
** 描述:  设置yaw、pitch的旋转中心的屏幕坐标
**
** 输入参数: 
**		width :	屏幕的宽度
**		height :	屏幕的长度	 
**		x: 目前不起作用，默认是屏幕中心线 
**		y：旋转中心的屏幕坐标，屏幕左上角为原点
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口5
**
**.EH--------------------------------------------------------
*/
void touchRotPointXY(float width ,float height, float x, float y)
{
	SetRotPointXY(width / 2.0, height - y);

	MAP_HEIGHT = height;
	
}

/*.BH--------------------------------------------------------
**
** 函数名: BOOL touchMiddleDeltaSet()
**
** 描述:  设置缩放的尺度,单位米
**
** 输入参数: 
**		value: 缩放的尺度，单位米
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**          
**
** 设计注记:  外部接口6
**
**.EH--------------------------------------------------------
*/
void touchMiddleDeltaSet(float value)
{
	SetMiddleDelta(value);
}

#endif