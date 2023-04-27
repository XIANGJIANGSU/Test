#include "frustum.h"
#include "../define/macrodefine.h"
#include <math.h>
#include "../projection/project.h"
#include "../projection/coord.h"

#ifdef WIN32
#include <assert.h>
#endif

/*
功能：根据相机模式和投影矩阵的乘积矩阵,计算6个面的方程,其中方程系数进行了归一化处理
输入：
	mat       相机模式和投影矩阵的乘积矩阵指针
	row       使用的乘积矩阵的行数
输出：
    plane     存放方程的四个系数的结构体指针
返回值：
    无
*/
void ExtractPlane(LPPLANE plane,f_float64_t *mat,f_int32_t row)
{
	f_float32_t length;
	f_int32_t scale = (row < 0) ? -1 : 1;
	row = abs(row)-1;

	// calculate plane coefficients from the matrix
	plane->A = mat[3] + scale * mat[row];
	plane->B = mat[7] + scale * mat[row+4];
	plane->C = mat[11] + scale * mat[row+8];
	plane->D = mat[15] + scale * mat[row+12];

	// normalize the plane
	length = sqrt(sqr(plane->A) + sqr(plane->B) + sqr(plane->C));
#ifdef WIN32
	//assert(length != 0);
#endif
	if(length < FLT_EPSILON )
		return;
	plane->A /= length;
	plane->B /= length;
	plane->C /= length;
	plane->D /= length;
}

/*
功能：计算相机视景体的6个面的方程,存放在frustum的数组中,依次是left right bottom top far near
输入：
    pScene     绘图场景句柄
输出：
    frustum    视景体参数指针
返回值：
    无
*/
void CalculateFrustum(FRUSTUM* frustum, sGLRENDERSCENE *pScene)
{
#if 0

	// get the projection and modelview matrices
	f_float32_t projection[16];
	f_float32_t modelview[16];

	glGetFloatv(GL_PROJECTION_MATRIX,projection);
	glGetFloatv(GL_MODELVIEW_MATRIX,modelview);

	// use opengl to multiply them
	glPushMatrix();
	glLoadMatrixf(projection);
	glMultMatrixf(modelview);
	glGetFloatv(GL_MODELVIEW_MATRIX,modelview);
	glPopMatrix();

//	Matrix44 projection, modelview, modelview_last;
//	Matrix44_Copy(&modelview, &g_SphereRender.m_Render.m_lfModelMatrix);
//	Matrix44_Copy(&projection, &g_SphereRender.m_Render.m_lfProjMatrix);
//	Maxtrix44_Mul(&modelview_last,&modelview ,&projection);

	// extract each plane
	ExtractPlane(&(frustum->plane[0]), modelview, 1);//left
	ExtractPlane(&(frustum->plane[1]), modelview, -1);//right
	ExtractPlane(&(frustum->plane[2]), modelview, 2);//bottom
	ExtractPlane(&(frustum->plane[3]), modelview, -2);//top
	ExtractPlane(&(frustum->plane[4]), modelview, -3);//far
	ExtractPlane(&(frustum->plane[5]), modelview, 3);//near

#else
	// extract each plane,手动计算的投影矩阵、模式矩阵
	{
		f_float64_t modelviewall2[16];
		dMatrices44MultNoNormal(pScene->m_lfProjMatrix, pScene->m_lfModelMatrix, modelviewall2);
		ExtractPlane(&(frustum->plane[0]), modelviewall2, 1);//left
		ExtractPlane(&(frustum->plane[1]), modelviewall2, -1);//right
		ExtractPlane(&(frustum->plane[2]), modelviewall2, 2);//bottom
		ExtractPlane(&(frustum->plane[3]), modelviewall2, -2);//top
		ExtractPlane(&(frustum->plane[4]), modelviewall2, -3);//far
		ExtractPlane(&(frustum->plane[5]), modelviewall2, 3);//near	
	}	
	#endif
}

/*
功能：判断包围球是否在相机的视景体视景体可视范围之内
输入：
    view_near  视景体近裁截面
    view_far   视景体远裁截面
	x,y,z      包围球中心点
	radius     包围球半径
	pFrustum   视景体6个面的方程
输出：
    pDis       包围球球心到near面的距离与包围球半径之差(根本就没有用到)
返回值：
    FALSE(0)  不在
    TRUE(1)   在
*/
BOOL InFrustum(const f_float32_t view_near, const f_float32_t view_far,
			   const f_float64_t x, const f_float64_t y, const f_float64_t z,
			   const f_float64_t radius,const LPFRUSTUM pFrustum, f_float64_t *pDis)
{
	f_uint8_t i = 0;
	f_float64_t dist = 0;

	if(pFrustum == NULL)
	{
		return FALSE;
	}

	for(i = 0; i < 6; i++)
	{
		/* 依次计算包围球中心点(x,y,z)到视景体各个面的距离,d=(Ax+By+Cz+D)/开二次根(A*A+B*B+C*C) */
		/* 因为(A,B,C)是法线上的单位向量,故分母为1 */
		dist = pFrustum->plane[i].A * x + 
			   pFrustum->plane[i].B * y + 
			   pFrustum->plane[i].C * z + 
			   pFrustum->plane[i].D;

		/* 六个面的法向量均朝视景体内部,点在法向量方向相同一侧,则到该面的距离大于0,点在法向量方向相反一侧,则到该面的距离小于0 */
		/* 如果到前5个面中任意一个面的距离小于或等于-radius,则必然在视景体外部; */
		/* 如果到第6个面(near)的距离小于-radius,则必然在视景体外部;如果到第6个面(near)的距离恰好等于-radius,则算在视景体内部 */
		/* 其余情况均在视景体内部,一种情况除外 */
		if( dist <= (-radius))
		{
			if(i < 5)
			{
				return FALSE;
			}
			else if(dist < (-radius))
			{
				return FALSE;
			}
		}
	}
	
	/* 如果到near面的距离大于包围球半径,则dist等于距离和包围球半径之差;否则dist为0 */
	dist = MAX(0, dist - radius);

	/* 如果dist大于远近裁截面距离之差,说明包围球在视景体远裁截面的外面,则球心到近裁截面的距离=球心到远裁截面的距离+(view_far-view_near) */
	/* 但球心到远裁截面距离<=radius,则球心到近裁截面的距离<=(view_far-view_near)+radius,推导得到dist-radius<=(view_far-view_near) */
	/* 故这种情况基本不会出现,判断多余 */
	if(dist > view_far - view_near)
		return FALSE;
	
	/* 返回球心到near面的距离与包围球半径之差 */
	if(pDis !=  NULL)
	{
		*pDis = dist;
	}

	/* 到此处,说明包围球在视景体可视范围之内 */
	return TRUE;
}
