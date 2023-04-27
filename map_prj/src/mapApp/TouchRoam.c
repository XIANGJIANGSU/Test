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
** ������: BOOL touchRoamXY()
**
** ����:  �ƶ�����
**
** �������:  
**	state ��0 �������ƶ���1�������ƶ�
**  x, y�����ε���Ļ���꣬��Ļ���Ͻ�Ϊԭ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�1
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
** ������: BOOL touchScale()
**
** ����:  ���Ų���
**
** �������:  
**	state ��0�����������ţ�1����������
**  scale��0����С��1���Ŵ�
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�2
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
** ������: BOOL touchYaw()
**
** ����:  ƫ������ת����
**
** �������:  
**	state ��0��������ƫ������ת��1������ƫ������ת
**  yaw��0-360��˳ʱ��Ϊ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�3
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
** ������: BOOL touchPitch()
**
** ����:  ��������ת����
**
** �������:  
**	state ��0����������������ת��1��������������ת
**  pitch��(0,-90)������Ϊ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�4
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
** ������: BOOL touchRotPointXY()
**
** ����:  ����yaw��pitch����ת���ĵ���Ļ����
**
** �������: 
**		width :	��Ļ�Ŀ��
**		height :	��Ļ�ĳ���	 
**		x: Ŀǰ�������ã�Ĭ������Ļ������ 
**		y����ת���ĵ���Ļ���꣬��Ļ���Ͻ�Ϊԭ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�5
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
** ������: BOOL touchMiddleDeltaSet()
**
** ����:  �������ŵĳ߶�,��λ��
**
** �������: 
**		value: ���ŵĳ߶ȣ���λ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**          
**
** ���ע��:  �ⲿ�ӿ�6
**
**.EH--------------------------------------------------------
*/
void touchMiddleDeltaSet(float value)
{
	SetMiddleDelta(value);
}

#endif