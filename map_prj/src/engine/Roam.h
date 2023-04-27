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
	
	Matrix44 lastTrans;   //��һ��ƽ�ƾ���ָ�����	
	Matrix44 lastRot;     //��һ����ת����ָ�����
	Matrix44 thisTrans;   //����ƽ�ƾ���	
	Matrix44 thisRot;	   //������ת����
	float lastPitch;       //�ϴθ�����ʼ��
	float thisPitch;       //���θ�����ʼ��	

	PT_3D stVec;           //���ƽ����ʼʸ��
	PT_3D endVec; 	       //���ƽ����ֹʸ��
	PT_2F stRot;        //�Ҽ���ת��ʼ����
	int   lastZTrans;       //�м�������ʼֵ
	
	int   isLeftDrag;					                    // NEW: Dragging The left Mouse button
	int   isRightDrag;					                    // NEW: Dragging The left Mouse button

	Matrix44 mv;       //����modelview����

	float thisYawTrue;       //������ʵyawֵ

	Obj_Pt_D objPt;			//��ת�������ӵ�����ϵ�µ�����ֵ
}RoamT;

void initRoam(RoamT* pRoam);

//void StartRoam(RoamT* pRoam, int isRoam, double* mv, int* viewport, float origYaw, sGLRENDERSCENE *pscene);

void RoamKeyFunc(unsigned char key, int x, int y);

void RoamMouseFunc(int button, int state, int x, int y);

void RoamMotionFunc(int x, int y);

BOOL GetPtEye(RoamT* pRoam, LP_PT_3D ptEye);

int IsMouseInViewPort(PT_2F mouse, float startX, float startY, float width, float height);

//��ת���ĵĸ����Ƕ�����
void SetOutPitch(float value);

//��ת���ĵ�ƫ��������
void SetOutYaw(float value);

//����м��ı仯�߶�
void SetMiddleDelta(float value);

//���½�Ϊԭ�㣬������ת����
void SetRotPointXY(float x, float y);

void SetZDelta(float value);


#endif
