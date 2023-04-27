#include "frustum.h"
#include "../define/macrodefine.h"
#include <math.h>
#include "../projection/project.h"
#include "../projection/coord.h"

#ifdef WIN32
#include <assert.h>
#endif

/*
���ܣ��������ģʽ��ͶӰ����ĳ˻�����,����6����ķ���,���з���ϵ�������˹�һ������
���룺
	mat       ���ģʽ��ͶӰ����ĳ˻�����ָ��
	row       ʹ�õĳ˻����������
�����
    plane     ��ŷ��̵��ĸ�ϵ���Ľṹ��ָ��
����ֵ��
    ��
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
���ܣ���������Ӿ����6����ķ���,�����frustum��������,������left right bottom top far near
���룺
    pScene     ��ͼ�������
�����
    frustum    �Ӿ������ָ��
����ֵ��
    ��
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
	// extract each plane,�ֶ������ͶӰ����ģʽ����
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
���ܣ��жϰ�Χ���Ƿ���������Ӿ����Ӿ�����ӷ�Χ֮��
���룺
    view_near  �Ӿ�����ý���
    view_far   �Ӿ���Զ�ý���
	x,y,z      ��Χ�����ĵ�
	radius     ��Χ��뾶
	pFrustum   �Ӿ���6����ķ���
�����
    pDis       ��Χ�����ĵ�near��ľ������Χ��뾶֮��(������û���õ�)
����ֵ��
    FALSE(0)  ����
    TRUE(1)   ��
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
		/* ���μ����Χ�����ĵ�(x,y,z)���Ӿ��������ľ���,d=(Ax+By+Cz+D)/�����θ�(A*A+B*B+C*C) */
		/* ��Ϊ(A,B,C)�Ƿ����ϵĵ�λ����,�ʷ�ĸΪ1 */
		dist = pFrustum->plane[i].A * x + 
			   pFrustum->plane[i].B * y + 
			   pFrustum->plane[i].C * z + 
			   pFrustum->plane[i].D;

		/* ������ķ����������Ӿ����ڲ�,���ڷ�����������ͬһ��,�򵽸���ľ������0,���ڷ����������෴һ��,�򵽸���ľ���С��0 */
		/* �����ǰ5����������һ����ľ���С�ڻ����-radius,���Ȼ���Ӿ����ⲿ; */
		/* �������6����(near)�ľ���С��-radius,���Ȼ���Ӿ����ⲿ;�������6����(near)�ľ���ǡ�õ���-radius,�������Ӿ����ڲ� */
		/* ������������Ӿ����ڲ�,һ��������� */
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
	
	/* �����near��ľ�����ڰ�Χ��뾶,��dist���ھ���Ͱ�Χ��뾶֮��;����distΪ0 */
	dist = MAX(0, dist - radius);

	/* ���dist����Զ���ý������֮��,˵����Χ�����Ӿ���Զ�ý��������,�����ĵ����ý���ľ���=���ĵ�Զ�ý���ľ���+(view_far-view_near) */
	/* �����ĵ�Զ�ý������<=radius,�����ĵ����ý���ľ���<=(view_far-view_near)+radius,�Ƶ��õ�dist-radius<=(view_far-view_near) */
	/* ��������������������,�ж϶��� */
	if(dist > view_far - view_near)
		return FALSE;
	
	/* �������ĵ�near��ľ������Χ��뾶֮�� */
	if(pDis !=  NULL)
	{
		*pDis = dist;
	}

	/* ���˴�,˵����Χ�����Ӿ�����ӷ�Χ֮�� */
	return TRUE;
}
