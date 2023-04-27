#include "Roam.h"
#include <math.h>

#ifdef WIN32
#include <memory.h>
#endif


#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#include "GL/freeglut.h"
#else
#include "esUtil.h"
#endif

#include "../mapApp/appHead.h"
#include "../mapApi/common.h"
#include "../mapApp/paramGet.h"
#include "../projection/coord.h"


#define VIEW_FOV 60.0f
#define THIS_SCREEN_WIDTH	1600	//1640	//跟着外面的屏幕大小改变
#define THIS_SCREEN_HEIGHT  1050	//840

extern void SetRoamState2D(BOOL flag);
extern void OnMouse(int button,int state,int x,int y);

extern BOOL geoDPt2objDPt(const LP_Geo_Pt_D geoPt, LP_Obj_Pt_D objPt);
extern BOOL innerViewDPt2GeoDPtOpenglEs(sGLRENDERSCENE *pScene,
										const PT_2I viewPt, LP_Geo_Pt_D geoPt);




static RoamT* pRoam = NULL;
//static sGLRENDERSCENE *pscene_roam = NULL;
PT_2F    MousePt;											// NEW: Current Mouse Point
int thisZTrans = 0;

PT_2F mouseEventPt;
int mouseType = 0;   //1:leftButtonDown; 2:RightButtonDown; 3:twoButtonDown

void Maxtrix4x4_Mul(Matrix44* pOut, Matrix44* pIn0, Matrix44 *pIn1)
{
	dMatrices44MultNoNormal(pIn1->m, pIn0->m, pOut->m);
}

static float RotPointX = THIS_SCREEN_WIDTH/2.0, RotPointY = THIS_SCREEN_HEIGHT/2.0;
static float MiddleDelta = 10000.0;
static float out_pitch = 0.0, out_yaw = 0.0;


void SetOutPitch(float value)
{
	out_pitch = value;
}

void SetOutYaw(float value)
{
	out_yaw =  value;
}
void SetMiddleDelta(float value)
{
	MiddleDelta = value;
}
//左下角为原点，设置旋转中心,目前只能在屏幕中心线上的
void SetRotPointXY(float x, float y)
{
	RotPointX = x;
	RotPointY = y;
}

void initRoam(RoamT* pRoam)
{
	if(NULL == pRoam)
		return;
		
	pRoam->isInRoam = 0;
	pRoam->startX = 0.0f;
	pRoam->startY = 0.0f;
	pRoam->width = 1024.0f;
	pRoam->height = 768.0f;
	pRoam->lastZTrans = 0;
	pRoam->isLeftDrag = 0;
	pRoam->isRightDrag = 0;
}


/*从矩阵中拆分出旋转矩阵和位移矩阵，in=trans*rot*/
void GetRotAndTrans(Matrix44* in, Matrix44* trans, Matrix44* rot)
{
	if(NULL == in || NULL == trans || NULL == rot)
		return;
	dMatrix44MemSet(trans, 1,         0,         0,         0,
						    0,         1,         0,         0,
						    0,         0,         1,         0,
						    in->m[12], in->m[13], in->m[14], 1);
						    
	dMatrix44MemSet(rot,   in->m[0],  in->m[1],  in->m[2],  in->m[3],
						    in->m[4],  in->m[5],  in->m[6],  in->m[7],
						    in->m[8],  in->m[9],  in->m[10], in->m[11],
						    0,         0,         0,         1);
}

float GetRollByMatrix(Matrix44* mv)
{
	Matrix44 trans;
	Matrix44 rot;	
	double deltaX;	
	double deltaY;
	double lenth;
	double roll;
	
	if(NULL == mv)
		return 0.0f;		

	GetRotAndTrans(mv, &trans, &rot);
	deltaX = trans.m[12];	
	deltaY = trans.m[13];
	
	if(deltaX < 10.0 && deltaX > -10.0)  //这里是给一个俯视情况下一个容差，防止在俯视时乱跳
		return 0.0f;	
		
	lenth = sqrt(deltaX*deltaX + deltaY*deltaY);
	
	roll = RADIAN_TO_DEGREE(acos(-deltaY/lenth));
	
	if(deltaX > 0.0)
		roll = -roll;
	
	return (float)roll;
}

float GetPitchByMatrix(Matrix44* mv)
{
	Matrix44 trans;
	Matrix44 rot;	
	double deltaY;
	double deltaZ;
	double lenth;
	double pitch;
	
	if(NULL == mv)
		return 0.0f;		

	GetRotAndTrans(mv, &trans, &rot);
	deltaY = trans.m[13];
	deltaZ = trans.m[14];
	lenth = sqrt(deltaY*deltaY + deltaZ*deltaZ);
	
	pitch = RADIAN_TO_DEGREE(asin(deltaZ/lenth));

	if(deltaY > 0.0f) 
	{
		if(pitch < 0.0) pitch = -180.0 - pitch;
		else pitch = 180.0 - pitch;
	}
	
	return (float)pitch;
}

double GetYawByMatrix(Matrix44* vmat)
{
	//先把矩阵旋转到指向地心，消除俯仰和横滚的影响
	double roll = GetRollByMatrix(vmat);
	double pitch = GetPitchByMatrix(vmat);		

	Matrix44 tempMat;	
	PT_3D  vv = {0.0, 0.0, 1.0};
	PT_3D  axiZ = {0};
	Matrix33 mat = {0};

	double deltaX ;
	double deltaY ;
	double lenth ;
	double yaw;

	/*计算初始矩阵*/
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(vmat->m);	

	/*旋转为垂直指向地心*/
	glRotated(90+pitch, 1.0, 0.0, 0.0);		

	/*旋转为平飞*/
	glRotated(roll, 0.0, 0.0, 1.0);



	/*挪回原点*/
	glGetDoublev(GL_MODELVIEW_MATRIX, tempMat.m);
	tempMat.m[12]=tempMat.m[13]=	tempMat.m[14] = 0;
	glLoadMatrixd(tempMat.m);	

	//原始的地轴转换到当前坐标系下
	mat.m[0] = tempMat.m[0];
	mat.m[1] = tempMat.m[1];
	mat.m[2] = tempMat.m[2];
	mat.m[3] = tempMat.m[4];
	mat.m[4] = tempMat.m[5];
	mat.m[5] = tempMat.m[6];
	mat.m[6] = tempMat.m[8];
	mat.m[7] = tempMat.m[9];
	mat.m[8] = tempMat.m[10];
	
	dMatrix33Multi(&axiZ, vv, &mat);


	//接下来，航向角就是当前（y轴） 与 （axiZ和z轴组成的平面） 的夹角
	//也就是axiZ 与 yoz平面 的夹角的负值
	 deltaX = axiZ.x;
	 deltaY = axiZ.y;
	 lenth = sqrt(deltaX*deltaX + deltaY*deltaY);

	if(lenth < PRECISION) return 0.0;  //极点

	 yaw = RADIAN_TO_DEGREE(asin(deltaX/lenth));
	if( deltaY > 0.0)
	{
		yaw = -yaw;
	}else
	{
		yaw = yaw-180.0;
	}
	if(yaw < 0.0)
	{
		yaw += 360.0;
	}
	//printf("yaw = %f\n", yaw);
	return yaw;
}



/*从初始参数中计算出当前矩阵*/
void initThisMat(RoamT* pRoam)
{
	float roll = 0.0f;
	if(NULL == pRoam)
		return;
		
	roll = GetRollByMatrix(&pRoam->mv);
	pRoam->thisPitch = GetPitchByMatrix(&pRoam->mv);		
	pRoam->thisYawTrue = GetYawByMatrix(&pRoam->mv);		

	/*计算初始矩阵*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
	
	/*旋转为垂直指向地心*/
	glRotated(90+pRoam->thisPitch, 1.0, 0.0, 0.0);		
	
	/*旋转为平飞*/
	glRotated(roll, 0.0, 0.0, 1.0);
	
	/*初始模式矩阵*/
	glMultMatrixd(pRoam->mv.m);
	
	/*OK了，可以从当前的模式矩阵中分离出平移和旋转矩阵了*/
	{
		Matrix44 tempMat;
		glGetDoublev(GL_MODELVIEW_MATRIX, tempMat.m);
		GetRotAndTrans(&tempMat, &pRoam->thisTrans, &pRoam->thisRot);
	}

	pRoam->objPt.x = pRoam->objPt.y = pRoam->objPt.z = 0;
	
	
	pRoam->lastTrans = pRoam->thisTrans;
	pRoam->lastRot   = pRoam->thisRot;
	pRoam->lastPitch = pRoam->thisPitch;
}






/*从初始参数中计算出当前矩阵*/
void initThisMatForRot(RoamT* pRoam, sGLRENDERSCENE *pscene)
{
	if(NULL == pRoam)
		return;


	/*计算地形中心点的空间位置*/
	{
		PT_2I screenPt = {0};
		Geo_Pt_D temp = {0};	
		PT_3D xyz_out = {0};
		
		screenPt.x = RotPointX;
		screenPt.y = RotPointY;

		printf("screenPt.y = %d\n", screenPt.y);

		if(FALSE == innerViewDPt2GeoDPtOpenglEs(pscene,screenPt, &temp))
		{
			screenPt.y = 1050 -  RotPointY;

			if(FALSE == getGeoByOuterScreenPt(pscene, screenPt, &temp))	
			{
				printf("!!!!ERROR:NO POINT !!!!\n");
				pRoam->objPt.x = pRoam->objPt.y = pRoam->objPt.z = 0.0; 
			}

			screenPt.y = RotPointY;
			if(FALSE == innerViewDPt2GeoDPtOpenglEs(pscene,screenPt, &temp))
			{

				printf("!!!!ERROR:NO POINT !!!!\n");

			}
		}
		else
		{
			printf("%f,%f,%f\n", temp.lon, temp.lat, temp.height);
			//printf("%f,%f,%f\n",pscene->camParam.m_geoptEye.lon, pscene->camParam.m_geoptEye.lat, pscene->camParam.m_geoptEye.height);

			geoDPt2objDPt(&temp, &pRoam->objPt);

			dMatrix44Multi(&xyz_out, &pRoam->objPt, &pRoam->mv);

			pRoam->objPt.x = -xyz_out.x;
			pRoam->objPt.y = -xyz_out.y;
			pRoam->objPt.z = -xyz_out.z;
	//		pRoam->objPt.x -= pscene->camParam.m_ptEye.x;
	//		pRoam->objPt.y -= pscene->camParam.m_ptEye.y;
	//		pRoam->objPt.z -= pscene->camParam.m_ptEye.z;
			//printf("delta %f,%f,%f\n", pRoam->objPt.x, pRoam->objPt.y, pRoam->objPt.z);

			{
				PT_3D viewpt2;
				geoDPt2InnerViewDPt(&temp,pscene->matrix, pscene->innerviewport,&viewpt2);

				printf("%f,%f\n",  viewpt2.x, 1050-viewpt2.y);

			}

		}
		


	}	

	/*计算初始矩阵*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glTranslated(pRoam->objPt.x, pRoam->objPt.y, pRoam->objPt.z);
#else
	glTranslatef(pRoam->objPt.x, pRoam->objPt.y, pRoam->objPt.z);
#endif

	/*初始模式矩阵*/
	glMultMatrixd(pRoam->mv.m);

	{
		Matrix44 tempMat;
		glGetDoublev(GL_MODELVIEW_MATRIX, tempMat.m);

		pRoam->thisPitch = GetPitchByMatrix(&tempMat);	
//		pRoam->thisYawTrue = GetYawByMatrix(&tempMat);			

	}





	/*计算初始矩阵*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
	
	/*旋转为垂直指向地心*/
	glRotated(90+pRoam->thisPitch, 1.0, 0.0, 0.0);		

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glTranslated(pRoam->objPt.x, pRoam->objPt.y, pRoam->objPt.z);
#else
	glTranslatef(pRoam->objPt.x, pRoam->objPt.y, pRoam->objPt.z);
#endif
	
	/*初始模式矩阵*/
	glMultMatrixd(pRoam->mv.m);
	
	/*OK了，可以从当前的模式矩阵中分离出平移和旋转矩阵了*/
	{
		Matrix44 tempMat;
		glGetDoublev(GL_MODELVIEW_MATRIX, tempMat.m);
		GetRotAndTrans(&tempMat, &pRoam->thisTrans, &pRoam->thisRot);

		pRoam->thisYawTrue = GetYawByMatrix(&tempMat);			

	}

		
	pRoam->lastTrans = pRoam->thisTrans;
	pRoam->lastRot   = pRoam->thisRot;
	pRoam->lastPitch = pRoam->thisPitch;
		
}


/*从当前模式矩阵中重新计算出当前矩阵*/
void reCalcThisMat(RoamT* pRoam)
{
	if(NULL == pRoam)
		return;
		
	pRoam->thisPitch = GetPitchByMatrix(&pRoam->mv);		

	pRoam->thisYawTrue = GetYawByMatrix(&pRoam->mv);		
	
	/*计算初始矩阵*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	/*旋转为垂直指向地心*/
	glRotated(90+pRoam->thisPitch, 1.0, 0.0, 0.0);
	
	/*初始模式矩阵*/
	glMultMatrixd(pRoam->mv.m);
	
	/*OK了，可以从当前的模式矩阵中分离出平移和旋转矩阵了*/
	{
		Matrix44 tempMat;
		glGetDoublev(GL_MODELVIEW_MATRIX, tempMat.m);
		GetRotAndTrans(&tempMat, &pRoam->thisTrans, &pRoam->thisRot);
	}

	pRoam->objPt.x = pRoam->objPt.y = pRoam->objPt.z = 0;
	
	pRoam->lastTrans = pRoam->thisTrans;
	pRoam->lastRot   = pRoam->thisRot;
	pRoam->lastPitch = pRoam->thisPitch;
}

double GetTransLength(Matrix44* in)
{
	double length = 0.0;
	double x, y, z;
	if(NULL == in)
		return 0.0;
	x = in->m[12];
	y = in->m[13];
	z = in->m[14];
	length = sqrt(x*x + y*y + z*z);
	return length;
}

void MapToSphere(RoamT* pRoam, const PT_2F* NewPt, PT_3D* NewVec)
{
    PT_2F TempPt;
    double lenEye;  //视点矢量长度
    double length;  //当前矢量长度
    double height;  //当前飞机高度
    double scaleX;   //当前屏幕坐标映射到实际距离的缩放比例
    double scaleY;
    
    float AdjustWidth;  
    float AdjustHeight;
    
    lenEye = GetTransLength(&pRoam->thisTrans);
    height = lenEye - EARTH_RADIUS;
    scaleX = height * 0.57735;  //默认为60度视角
    scaleX = scaleX * (pRoam->thisPitch / 90.0 + 2.0);
    scaleY = scaleX * (pRoam->thisPitch / 22.5 + 5.0);


    AdjustWidth = 1.0f / ((pRoam->width  - 1.0f) * 0.5f);
    AdjustHeight = 1.0f / ((pRoam->height - 1.0f) * 0.5f);
    
    //Copy paramter into temp point
    TempPt = *NewPt;
    TempPt.x -= pRoam->startX;
    TempPt.y -= (THIS_SCREEN_HEIGHT - pRoam->startY - pRoam->height);
    
    //Adjust point coords and scale down to range of [-1 ... 1]
    TempPt.x  =        (TempPt.x * AdjustWidth)  - 1.0f;
    TempPt.y  = 1.0f - (TempPt.y * AdjustHeight);
    
    //换算为实际距离
    TempPt.x *= scaleX;
    TempPt.y *= scaleY;

    //Compute the square of the length of the vector to the point from the center
    length      = (TempPt.x * TempPt.x) + (TempPt.y * TempPt.y);

    //If the point is mapped outside of the sphere... (length > radius squared)
    if (length > EARTH_RADIUS*EARTH_RADIUS)
    {
        double norm;

        //Compute a normalizing factor (radius / sqrt(length))
        norm    = EARTH_RADIUS / sqrt(length);

        //Return the "normalized" vector, a point on the sphere
        NewVec->x = TempPt.x * norm;
        NewVec->y = TempPt.y * norm;
        NewVec->z = 0.0f;
    }
    else    //Else it's on the inside
    {
        //Return a vector to a point mapped inside the sphere sqrt(radius squared - length)
        NewVec->x = TempPt.x;
        NewVec->y = TempPt.y;
        NewVec->z = sqrt(EARTH_RADIUS*EARTH_RADIUS - length);
    }
    vector3DNormalize(NewVec);
}

int IsMouseInViewPort(PT_2F mouse, float startX, float startY, float width, float height)
{
	if( (mouse.x < startX)
	 || (mouse.x > (startX+width))
	 || ((THIS_SCREEN_HEIGHT - mouse.y) < startY)
	 || ((THIS_SCREEN_HEIGHT - mouse.y) > (startY+height)) )
	 return 0;
	return 1;
}

static float z_delta = 1.0;
void SetZDelta(float value)
{
	z_delta = value;
}

void SetModelViewMat(RoamT* pRoam)
{
	int deltaZTrans = 0;
	double lenth = 0.0;

	if(NULL == pRoam)
		return;	
		
	/*计算中键缩放的长度*/	
	lenth = GetTransLength(&pRoam->thisTrans);
	lenth -= EARTH_RADIUS;
	if(lenth < 0.0)
		lenth = 0.0;
	//lenth += 10000.0;
	lenth += MiddleDelta;
	
	/*计算缩放地图的平移量，只在左右键都没按下时*/	
	deltaZTrans = thisZTrans - pRoam->lastZTrans;
	if( (0 != pRoam->isLeftDrag)
	 || (0 != pRoam->isRightDrag)
	 || (1 != IsMouseInViewPort(mouseEventPt, pRoam->startX, pRoam->startY, pRoam->width, pRoam->height)) )
	{
		deltaZTrans = 0;
	}

	pRoam->lastZTrans = thisZTrans;
		
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();



#if 1
{
	float temp_x = 0.0;
	float temp_y = 0.0;
	float temp_z = 0.0;

	temp_x = pRoam->objPt.x * z_delta;
	temp_y = pRoam->objPt.y * z_delta;
	temp_z = pRoam->objPt.z * z_delta;
	
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glTranslated(-temp_x, -temp_y, -temp_z);
#else
	glTranslatef(-temp_x, -temp_y, -temp_z);
#endif
	
	
}


#else
{

	pRoam->objPt.x *= z_delta;
	pRoam->objPt.y *= z_delta;
	pRoam->objPt.z *= z_delta;
	
	glTranslated(-pRoam->objPt.x , -pRoam->objPt.y, -pRoam->objPt.z );
	
}
	
#endif	


//	printf("translate %f,%f,%f\n", -pRoam->objPt.x, -pRoam->objPt.y, -pRoam->objPt.z);
	
	glTranslatef(0.0f, 0.0f, deltaZTrans*lenth/10.0f);
	
//	glRotated(-pRoam->roll, 0.0, 0.0, 1.0);	
	
	glRotated(-pRoam->thisPitch-90, 1.0, 0.0, 0.0);
	
	glMultMatrixd(pRoam->thisTrans.m);
	
	glMultMatrixd(pRoam->thisRot.m);
	
	glGetDoublev(GL_MODELVIEW_MATRIX, pRoam->mv.m);		
	
	if((0 != deltaZTrans))
	{
		reCalcThisMat(pRoam);
	}
}

void StartTransRoam(RoamT* pRoam)
{
	if(NULL == pRoam)
		return;
	
	reCalcThisMat(pRoam);
	
	/*要记录下当前鼠标左键按下点对应的矢量哦*/
	MapToSphere(pRoam, &MousePt, &pRoam->stVec);
}

void TransDragRoam(RoamT* pRoam)
{
	PT_3D perp;
	Matrix44 rotMat;
	double angle = 0.0;
	if(NULL == pRoam)
		return;
	
	/*要记录下当前鼠标左键按下点对应的矢量哦*/
	MapToSphere(pRoam, &MousePt, &pRoam->endVec);
	
	vector3DCrossProduct(&perp, &pRoam->stVec, &pRoam->endVec);
	
	if(vector3DLength(&perp) > PRECISION)
	{
		angle = vector3DDotProduct(&pRoam->stVec, &pRoam->endVec);
		angle = RADIAN_TO_DEGREE(acos(angle));
		
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated(angle, perp.x, perp.y, perp.z);
	glGetDoublev(GL_MODELVIEW_MATRIX, rotMat.m);
	
	Maxtrix4x4_Mul(&(pRoam->thisRot), &(pRoam->lastRot), &(rotMat));
}

void StartRotRoam(RoamT* pRoam, sGLRENDERSCENE *pscene)
{
	if(NULL == pRoam)
		return;

//		if(2 == mouseType)
		{
			initThisMatForRot(pRoam, pscene);
		}
//		else
//		{
//			reCalcThisMat(pRoam);
//		}
	/*要记录下当前鼠标右键按下点*/
	pRoam->stRot = MousePt;
	pRoam->origYaw = pRoam->thisYaw;

	//printf("pRoam->origYaw = %f\n", pRoam->origYaw);
}




void RotDragRoam(RoamT* pRoam)
{
	Matrix44 rotMat;
	double angle = 0.0;
	float deltaPitch = 0.0f;
	
	if(NULL == pRoam)
		return;
		
	/*计算纵向俯仰角*/
	deltaPitch = (MousePt.y-pRoam->stRot.y)/pRoam->height*VIEW_FOV;

	deltaPitch = out_pitch;
	
	pRoam->thisPitch = pRoam->lastPitch + deltaPitch;
	if(pRoam->thisPitch > 90.0f)
		pRoam->thisPitch = 90.0f;
	else if(pRoam->thisPitch < -90.0f)
		pRoam->thisPitch = -90.0f;
	
	/*计算横向旋转角*/
	//angle = (MousePt.x-pRoam->stRot.x)/pRoam->width*80.0;
	angle = (MousePt.x-pRoam->stRot.x)/pRoam->height*VIEW_FOV;

	angle = out_yaw;

	
	pRoam->thisYaw = pRoam->origYaw + angle;

//	printf("pRoam->thisYaw = %f\n", pRoam->thisYaw);

	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated(-angle, 0.0, 0.0, 1.0);
	glGetDoublev(GL_MODELVIEW_MATRIX, rotMat.m);
	
	Maxtrix4x4_Mul(&(pRoam->thisRot), &(pRoam->lastRot), &(rotMat));

		pRoam->thisYawTrue = GetYawByMatrix(&pRoam->mv);		
}

void UpdateRoam(RoamT* pRoam, sGLRENDERSCENE *pscene)									// Perform Motion Updates Here
{		
	static int mm = 0;

	
    switch(mouseType)
    {
    	case 0:
    		pRoam->isLeftDrag = 0;
    		pRoam->isRightDrag = 0;
    		break;
    	case 1:            //leftButtonDown
    		if(1 == IsMouseInViewPort(mouseEventPt, pRoam->startX, pRoam->startY, pRoam->width, pRoam->height))
    		{
	    		if(0 == pRoam->isLeftDrag)
	    		{
	    			pRoam->isLeftDrag = 1;
	    			StartTransRoam(pRoam);
	
	    		}else
	    		{
	    			
	    			TransDragRoam(pRoam);
	    		}
	    	}
    		break;
    	case 2:            //RightButtonDown
//    		if(1 == IsMouseInViewPort(mouseEventPt, pRoam->startX, pRoam->startY, pRoam->width, pRoam->height))
    		{
	    		if(0 == pRoam->isRightDrag)
	    		{
	    			pRoam->isRightDrag = 1;
					
				pscene->roam.objPt.x 
					= pscene->roam.objPt.y
					= pscene->roam.objPt.z = 0;
					
	    			StartRotRoam(pRoam, pscene);

	    		}else
	    		{
	    			RotDragRoam(pRoam);
	    		}
	    	}
	    	break;
    	case 3:            //twoButtonDown
    	default:
    		break;
    }
    SetModelViewMat(pRoam);

	
}

/*
U = East
V = Up
N = North
P0 = (X0, Y0, Z0);
*/

void StartRoam(RoamT* pRoam, int isRoam, double* mv, int* viewport, float origYaw, sGLRENDERSCENE *pscene)
{
	if(NULL == pRoam || NULL == viewport || NULL == mv)
		return;		
	
	/*不在漫游状态，重新初始化参数*/
	if(1 != isRoam)
	{
		/*重新初始化参数*/		
		pRoam->isInRoam = 0;
		pRoam->startX = (float)viewport[0];
		pRoam->startY = (float)viewport[1];
		pRoam->width = (float)viewport[2];
		pRoam->height = (float)viewport[3];
		pRoam->lastZTrans = thisZTrans;
		pRoam->isLeftDrag = 0;
		pRoam->isRightDrag = 0;
		pRoam->origYaw = origYaw;
		pRoam->thisYaw = origYaw;
		memcpy(pRoam->mv.m, mv, 128);
	}else
	{
		if(0 == pRoam->isInRoam)
		{
			pRoam->isInRoam = 1;

			printf("%d\n", mouseType);

			{

				{
					initThisMat(pRoam);
				}
				
			}

			

		}

		//pscene_roam = pscene;
		
		UpdateRoam(pRoam,  pscene);
	}






	
}

void RoamKeyFunc(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 'w': 
			thisZTrans++; 
			mouseEventPt.x = x;
			mouseEventPt.y = y;
			break;
		case 's': 
			thisZTrans--; 
			mouseEventPt.x = x;
			mouseEventPt.y = y;
			break;
		default:
			break;
	}
}

void RoamMouseFunc(int button, int state, int x, int y)
{
	MousePt.x = (float)x;
	MousePt.y = (float)y;	

	//printf("Roam Mouse Func button = %d, state = %d\n", button, state);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	
	switch(button)
	{
		case GLUT_LEFT_BUTTON:
			if(GLUT_DOWN == state)
			{
				if(mouseType%2 == 0)
				{
					mouseType+=1;
					mouseEventPt.x = x;
					mouseEventPt.y = y;
				}
				

//				SetRoamState2D(TRUE);
			}else if(GLUT_UP == state)
			{
				if(mouseType%2 != 0)
					mouseType-=1;

			}else
			{
			}


			break;
		case GLUT_RIGHT_BUTTON:
			if(GLUT_DOWN == state)
			{
				if(mouseType < 2)
				{
					mouseType+=2;
					mouseEventPt.x = x;
					mouseEventPt.y = y;
				}


//				pscene_roam->roam.objPt.x 
//					= pscene_roam->roam.objPt.y
//					= pscene_roam->roam.objPt.z = 0;
				
				
			}else if(GLUT_UP == state)
			{
				if(mouseType >= 2)
					mouseType-=2;


			}else
			{
			}
			break;
		case GLUT_MIDDLE_BUTTON:
		default:
			break;
	}


#ifdef WIN32
	//OnMouse(button, state, x,  y);	//lpf add test for 标绘测试接口
#endif

#endif
}
extern int GetMouseCenterInViewport();
void RoamMotionFunc(int x, int y)
{
	MousePt.x = (float)x;
	MousePt.y = (float)y;
#ifdef WIN32
//	if(0 == GetMouseCenterInViewport())
//		SetRoamState2D(FALSE);
#endif
}

BOOL GetPtEye(RoamT* pRoam, LP_PT_3D ptEye)
{
	BOOL ret;
	f_float64_t vMatInvert[16];
	f_float64_t eye[4] = {0.0, 0.0, 0.0, 1.0};
	f_float64_t eyeOut[4];
	if(NULL == pRoam || NULL == ptEye)
		return FALSE;
	ret = dMatrix44Invert(pRoam->mv.m, vMatInvert);
	if(TRUE != ret)
		return ret;
	vec4dMultMatrix(vMatInvert, eye, eyeOut);
	ptEye->x = eyeOut[0];
	ptEye->y = eyeOut[1];
	ptEye->z = eyeOut[2];
	return TRUE;
}
