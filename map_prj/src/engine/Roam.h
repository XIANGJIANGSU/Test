#ifndef __ROAM_H__
#define __ROAM_H__


#include "../camera/camera3d.h"

typedef struct{
	
	int isInRoam;
	
	float startX;
	float startY;
	float width;
	float height;

	float origYaw;
	float thisYaw;
	
	Matrix44 lastTrans;   //上一个平移矩阵，指向地心	
	Matrix44 lastRot;     //上一个旋转矩阵，指向地心
	Matrix44 thisTrans;   //本次平移矩阵	
	Matrix44 thisRot;	   //本次旋转矩阵
	float lastPitch;       //上次俯仰起始角
	float thisPitch;       //本次俯仰起始角	

	PT_3D stVec;           //左键平移起始矢量
	PT_3D endVec; 	       //左键平移终止矢量
	PT_2F stRot;        //右键旋转起始坐标
	int   lastZTrans;       //中键滚轮起始值
	
	int   isLeftDrag;					                    // NEW: Dragging The left Mouse button
	int   isRightDrag;					                    // NEW: Dragging The left Mouse button

	Matrix44 mv;       //最后的modelview矩阵

	float thisYawTrue;       //本次真实yaw值

	Obj_Pt_D objPt;			//旋转中心在视点坐标系下的坐标值
}RoamT;

void initRoam(RoamT* pRoam);

//void StartRoam(RoamT* pRoam, int isRoam, double* mv, int* viewport, float origYaw, sGLRENDERSCENE *pscene);

void RoamKeyFunc(unsigned char key, int x, int y);

void RoamMouseFunc(int button, int state, int x, int y);

void RoamMotionFunc(int x, int y);

BOOL GetPtEye(RoamT* pRoam, LP_PT_3D ptEye);

int IsMouseInViewPort(PT_2F mouse, float startX, float startY, float width, float height);

//旋转中心的俯仰角度设置
void SetOutPitch(float value);

//旋转中心的偏航角设置
void SetOutYaw(float value);

//鼠标中键的变化尺度
void SetMiddleDelta(float value);

//左下角为原点，设置旋转中心
void SetRotPointXY(float x, float y);

void SetZDelta(float value);


#endif
