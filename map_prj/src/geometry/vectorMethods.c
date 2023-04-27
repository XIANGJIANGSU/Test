#ifdef WIN32
#include <assert.h>
#endif

#include <math.h>
#include <stdio.h>
#include "../define/macrodefine.h"
#include "vectorMethods.h"
#include "matrixMethods.h"

/* 3F向量运算 */
/* 赋值*/
void vector3FMemSet(LP_PT_3F pt, const f_float32_t x, const f_float32_t y, const f_float32_t z)
{
	if(pt == NULL)
		return;
	
	pt->x = x;
	pt->y = y;
	pt->z = z;
}
void vector3FSetValue(LP_PT_3F pLeft, LP_PT_3F pRight)
{
	if(pLeft == NULL || pRight == NULL)
		return;
	
	pLeft->x = pRight->x;
	pLeft->y = pRight->y;
	pLeft->z = pRight->z;
}

/* 加法*/
void vector3FAdd(LP_PT_3F pOut, const LP_PT_3F pt0, const LP_PT_3F pt1)
{
	if(pOut == NULL || pt0 == NULL || pt1 == NULL)
		return;
	
	pOut->x = pt0->x + pt1->x;
	pOut->y = pt0->y + pt1->y;
	pOut->z = pt0->z + pt1->z;
}

/* 减法*/
void vector3FSub(LP_PT_3F pOut, const LP_PT_3F pt0, const LP_PT_3F pt1)
{
	if(pOut == NULL || pt0 == NULL || pt1 == NULL)
		return;
	
	pOut->x = pt0->x - pt1->x;
	pOut->y = pt0->y - pt1->y;
	pOut->z = pt0->z - pt1->z;
}

/* 乘法*/
void vector3FMul(LP_PT_3F pOut, const LP_PT_3F pIn, const f_float32_t lf)
{
	if(pOut == NULL || pIn == NULL)
		return;
	
	pOut->x = pIn->x * lf;
	pOut->y = pIn->y * lf;
	pOut->z = pIn->z * lf;
}

/* 除法*/
void vector3FDiv(LP_PT_3F pOut, const LP_PT_3F pIn, const f_float32_t lf)
{
	if(pOut == NULL || pIn == NULL)
		return;
	if(fabs(lf) < PRECISION)
	    return;
	    
	pOut->x = pIn->x / lf;
	pOut->y = pIn->y / lf;
	pOut->z = pIn->z / lf;
}

/* 长度*/
f_float32_t vector3FLength(const LP_PT_3F pt)
{
	f_float32_t s;
	if(pt == NULL) 
		return (0);
	
	s = pt->x * pt->x + pt->y * pt->y + pt->z * pt->z;
	s = (f_float32_t)sqrt(s);
	return s;
}

/* 单位化*/
void vector3FNormalize(LP_PT_3F pInOut)
{
	f_float32_t s = vector3FLength(pInOut);
	if( s > PRECISION )
	{
#ifdef WIN32
	assert(s != 0);
#endif
 		pInOut->x /= s;
		pInOut->y /= s;
		pInOut->z /= s;
	}
}

/* 点乘*/
f_float32_t vector3FDotProduct(const LP_PT_3F pt0, const LP_PT_3F pt1)
{
	if(pt0 == NULL || pt1 == NULL)
		return (0);

	return pt0->x * pt1->x + pt0->y * pt1->y + pt0->z * pt1->z;
}

/* 负号*/
void vector3FNegative(LP_PT_3F pInOut)
{
	if(pInOut == NULL)
		return;
	
	pInOut->x = -pInOut->x;
	pInOut->y = -pInOut->y;
	pInOut->z = -pInOut->z;
}

/* 叉乘*/
void vector3FCrossProduct(LP_PT_3F pOut, const LP_PT_3F pIn0,  const LP_PT_3F pIn1)
{
	pOut->x = pIn0->y * pIn1->z - pIn0->z * pIn1->y;
	pOut->y = pIn0->z * pIn1->x - pIn0->x * pIn1->z;
	pOut->z = pIn0->x * pIn1->y - pIn0->y * pIn1->x;
}

/* 求两向量间的夹角 */
f_float32_t getVa2FVbAngle(const LP_PT_3F pt0, const LP_PT_3F pt1)
{
	f_float32_t moda, modb, vdot, angle;
	moda = (f_float32_t)sqrt(pt0->x * pt0->x + pt0->y * pt0->y + pt0->z * pt0->z);
	modb = (f_float32_t)sqrt(pt1->x * pt1->x + pt1->y * pt1->y + pt1->z * pt1->z);
	
	if( (fabs(moda) < PRECISION) || (fabs(modb) < PRECISION) )
	{
	    printf("The length of vector is 0.\n");
	    return(0.0f);
	}
	
	vdot = vector3FDotProduct(pt0, pt1) / (moda * modb);
	if(fabs(vdot) > 1.0f)
	{
	    printf(" The value is out of the range of acos.\n")	;
	    return(0.0f);
	}
	else
	    angle = (f_float32_t)acos(vdot);
	
	return(angle);
}

/*
 * 计算点或向量绕某向量旋转一定弧度的结果
 * 输入: 初始点或向量坐标, 轴向量, 旋转的弧度值
 * 返回: 旋转的结果值(输出)
 */
PT_3F vector3FRotate(const LP_PT_3F vpRotate, const LP_PT_3F vpAxis, const f_float32_t radRotate)
{
	PT_3F vAfterRotate;
	PT_3F ptPnew1, ptPnew2, ptPnew3, ptA;//中间变量
	
	vector3FSetValue(&ptA, vpAxis);
	vector3FMul(&ptPnew1, vpRotate, (f_float32_t)(cos(radRotate)));
	vector3FCrossProduct(&ptPnew2, &ptA, vpRotate);
	vector3FMul(&ptPnew2, &ptPnew2, (f_float32_t)(sin(radRotate)));
	vector3FMul(&ptPnew3, &ptA, vector3FDotProduct(&ptA, vpRotate)*(1.0f - (f_float32_t)(cos(radRotate))));
	vector3FAdd(&vAfterRotate, &ptPnew1, &ptPnew2);
	vector3FAdd(&vAfterRotate, &vAfterRotate, &ptPnew3);
	
	return vAfterRotate;
}

/* 3D向量运算 */
/* 赋值*/
void vector3DMemSet(LP_PT_3D pt, const f_float64_t x, const f_float64_t y, const f_float64_t z)
{
	if(pt == NULL)
		return;
	
	pt->x = x;
	pt->y = y;
	pt->z = z;
}

void vector3DSetValue(LP_PT_3D pLeft, LP_PT_3D pRight)
{
	if(pLeft == NULL || pRight == NULL)
		return;
	
	pLeft->x = pRight->x;
	pLeft->y = pRight->y;
	pLeft->z = pRight->z;
}

/* 加法*/
void vector3DAdd(LP_PT_3D pOut, const LP_PT_3D pt0, const LP_PT_3D pt1)
{
	if(pOut == NULL || pt0 == NULL || pt1 == NULL)
		return;
	
	pOut->x = pt0->x + pt1->x;
	pOut->y = pt0->y + pt1->y;
	pOut->z = pt0->z + pt1->z;
}

/* 减法*/
void vector3DSub(LP_PT_3D pOut, const LP_PT_3D pt0, const LP_PT_3D pt1)
{
	if(pOut == NULL || pt0 == NULL || pt1 == NULL)
		return;
	
	pOut->x = pt0->x - pt1->x;
	pOut->y = pt0->y - pt1->y;
	pOut->z = pt0->z - pt1->z;
}

/* 乘法*/
void vector3DMul(LP_PT_3D pOut, const LP_PT_3D pIn, const f_float64_t lf)
{
	if(pOut == NULL || pIn == NULL)
		return;
	
	pOut->x = pIn->x * lf;
	pOut->y = pIn->y * lf;
	pOut->z = pIn->z * lf;
}

/* 除法*/
void vector3DDiv(LP_PT_3D pOut, const LP_PT_3D pIn, const f_float64_t lf)
{
	if(pOut == NULL || pIn == NULL)
		return;
	if(fabs(lf) < FLT_EPSILON)
	    return;
	pOut->x = pIn->x / lf;
	pOut->y = pIn->y / lf;
	pOut->z = pIn->z / lf;
}

/* 长度*/
f_float64_t vector3DLength(const LP_PT_3D pt)
{
	f_float64_t s;
	if(pt == NULL) 
		return (0);
	
	s = pt->x * pt->x + pt->y * pt->y + pt->z * pt->z;
	s = sqrt(s);
	return s;
}

/* 单位化*/
void vector3DNormalize(LP_PT_3D pInOut)
{
	f_float64_t s = vector3DLength(pInOut);
	if( s > FLT_EPSILON )
	{
#ifdef WIN32
	assert(s != 0);
#endif
 		pInOut->x /= s;
		pInOut->y /= s;
		pInOut->z /= s;
	}
}

/* 点乘*/
f_float64_t vector3DDotProduct(const LP_PT_3D pt0, const LP_PT_3D pt1)
{
	if(pt0 == NULL || pt1 == NULL)
		return (0);

	return pt0->x * pt1->x + pt0->y * pt1->y + pt0->z * pt1->z;
}

/* 负号*/
void vector3DNegative(LP_PT_3D pInOut)
{
	if(pInOut == NULL)
		return;
	
	pInOut->x = -pInOut->x;
	pInOut->y = -pInOut->y;
	pInOut->z = -pInOut->z;
}

/* 叉乘*/
void vector3DCrossProduct(LP_PT_3D pOut, const LP_PT_3D pIn0,  const LP_PT_3D pIn1)
{
	pOut->x = pIn0->y * pIn1->z - pIn0->z * pIn1->y;
	pOut->y = pIn0->z * pIn1->x - pIn0->x * pIn1->z;
	pOut->z = pIn0->x * pIn1->y - pIn0->y * pIn1->x;
}

/* 求两向量间的夹角 */
f_float64_t getVa2DVbAngle(const LP_PT_3D pt0, const LP_PT_3D pt1)
{
	f_float64_t moda, modb, vdot, angle;
	moda = sqrt(pt0->x * pt0->x + pt0->y * pt0->y + pt0->z * pt0->z);
	modb = sqrt(pt1->x * pt1->x + pt1->y * pt1->y + pt1->z * pt1->z);
	
	if( (fabs(moda) < FLT_EPSILON) || (fabs(modb) < FLT_EPSILON) )
	{
	    printf("The length of vector is 0.\n");
	    return(0.0);
	}
	
	vdot = vector3DDotProduct(pt0, pt1) / (moda * modb);
	if(fabs(vdot) > 1.0)
	{
	    printf(" The value is out of the range of acos.\n")	;
	    return(0.0);
	}
	else
	    angle = acos(vdot);
	
	return(angle);
}

/*
 * 计算点或向量绕某向量旋转一定弧度的结果
 * 输入: 初始点或向量坐标, 轴向量, 旋转的弧度值
 * 返回: 旋转的结果值(输出)
 * 说明：向量旋转法1,与vector3DRotateM2结果一致
 */
PT_3D vector3DRotate(const LP_PT_3D vpRotate, const LP_PT_3D vpAxis, const f_float64_t radRotate)
{
	PT_3D vAfterRotate;
	PT_3D ptPnew1, ptPnew2, ptPnew3, ptA;//中间变量
	
	vector3DSetValue(&ptA, vpAxis);
	vector3DMul(&ptPnew1, vpRotate, cos(radRotate));
	vector3DCrossProduct(&ptPnew2, &ptA, vpRotate);
	vector3DMul(&ptPnew2, &ptPnew2, sin(radRotate));
	vector3DMul(&ptPnew3, &ptA, vector3DDotProduct(&ptA, vpRotate)*(1.0 - cos(radRotate)));
	vector3DAdd(&vAfterRotate, &ptPnew1, &ptPnew2);
	vector3DAdd(&vAfterRotate, &vAfterRotate, &ptPnew3);
	
	return vAfterRotate;
}

/*
功能：点或向量rotated_pt绕着旋转轴rotate_axis旋转一个角度angle_rad
输入：rotated_pt   点或向量
      rotate_axis  旋转轴
      angle_rad    旋转角(弧度)
输出：旋转后的点或向量
说明：向量旋转法2, 与vector3DRotate的结果一致
*/
PT_3D vector3DRotateM2(const PT_3D rotated_pt, const PT_3D rotate_axis, const f_float64_t angle_rad)
{
	/* 绕任意轴旋转的矩阵 */	
    Matrix33 matrix33;   
    /*
    sina,cosa 旋转角的正弦、余弦
    ux,uy,uz  旋转轴单位向量在x、y、z轴上的分量
    */
    f_float64_t sina, cosa;
    
    PT_3D out_pt;

    sina = sin(angle_rad);
    cosa = cos(angle_rad);

    /* 设置向量旋转矩阵 */
    createRotationMatrix33(&matrix33, sina, cosa, rotate_axis.x, rotate_axis.y, rotate_axis.z); 

    /* 向量旋转 */
    dMatrix33Multi(&out_pt, rotated_pt, &matrix33);

	return(out_pt);
}


/*
* 计算两个三维点的距离
* 输入: 两个三维点坐标
* 返回: 这两个三维点之间的距离(输出)
*/
double DistanceTwoPoints(const LP_PT_3D pPt1, const LP_PT_3D pPt2)
{
	return sqrt((pPt1->x - pPt2->x)*(pPt1->x - pPt2->x) + (pPt1->y - pPt2->y)*(pPt1->y - pPt2->y) + (pPt1->z - pPt2->z)*(pPt1->z - pPt2->z));
}


/* 4D向量运算 */
/* 赋值*/
void vector4DMemSet(LP_PT_4D pt, const f_float64_t x, const f_float64_t y, const f_float64_t z, const f_float64_t w)
{
	if(pt == NULL)
		return;

	pt->x = x;
	pt->y = y;
	pt->z = z;
	pt->w = w;
}
void vector4DSetValue(LP_PT_4D pLeft, LP_PT_4D pRight)
{
	if(pLeft == NULL || pRight == NULL)
		return;

	pLeft->x = pRight->x;
	pLeft->y = pRight->y;
	pLeft->z = pRight->z;
	pLeft->w = pRight->w;
}
