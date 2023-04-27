#ifdef WIN32
#include <memory.h>
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include <vxWorks.h>
#endif

#include <math.h>
#include "matrixMethods.h"
#include "vectorMethods.h"

/* 设置3x3单位阵(64位浮点型) */
void dMatrix33LoadIdentity(f_float64_t m[9])
{
	f_float64_t identity[9] = { 1.0, 0.0, 0.0, 
                                0.0, 1.0, 0.0, 
                                0.0, 0.0, 1.0 };

	memcpy(m, identity, sizeof(f_float64_t) * 9);
}

/*	向量乘以齐次矩阵 */
void dMatrix33Multi(LP_PT_3D pOut, const PT_3D pIn, const LPMatrix33 mx)
{
	pOut->x = pIn.x * mx->m[0] + pIn.y * mx->m[3] + pIn.z * mx->m[6];
	pOut->y = pIn.x * mx->m[1] + pIn.y * mx->m[4] + pIn.z * mx->m[7];
	pOut->z = pIn.x * mx->m[2] + pIn.y * mx->m[5] + pIn.z * mx->m[8];
}

/*
功能：设置向量旋转矩阵（3×3）
输入：sina,cosa 旋转角的正弦、余弦
      ux,uy,uz  旋转轴单位向量在x、y、z轴上的分量
输出：matrix33  旋转矩阵
*/
void createRotationMatrix33(LPMatrix33 matrix33, f_float64_t sina, f_float64_t cosa, f_float64_t ux, f_float64_t uy, f_float64_t uz)
{
	f_float64_t xx, yy, zz, oneMinusCos, xs, ys, zs;
	if(NULL == matrix33)
		return;
		
	oneMinusCos = 1.0 - cosa;
	xx = ux * ux;
	yy = uy * uy;
	zz = uz * uz;
	xs = sina * ux;
	ys = sina * uy;
	zs = sina * uz;
	
	/* 设置第0列 */	
	matrix33->m[0] = cosa + oneMinusCos * xx;
	matrix33->m[1] = oneMinusCos * ux * uy + zs;
	matrix33->m[2] = oneMinusCos * ux * uz - ys;

	/* 设置第1列 */	
	matrix33->m[3] = oneMinusCos * uy * ux - zs;
	matrix33->m[4] = cosa + oneMinusCos * yy;
	matrix33->m[5] = oneMinusCos * uy * uz + xs;

	/* 设置第2列 */
	matrix33->m[6] = oneMinusCos * uz * ux + ys;
	matrix33->m[7] = oneMinusCos * uz * uy - xs;
	matrix33->m[8] = cosa + oneMinusCos * zz;	
}

void dMatrix44MemSet(LPMatrix44 pOut, 
                     const f_float64_t in00, const f_float64_t in01, const f_float64_t in02, const f_float64_t in03,
					 const f_float64_t in10, const f_float64_t in11, const f_float64_t in12, const f_float64_t in13,
					 const f_float64_t in20, const f_float64_t in21, const f_float64_t in22, const f_float64_t in23,
					 const f_float64_t in30, const f_float64_t in31, const f_float64_t in32, const f_float64_t in33)
{
    pOut->m[0]  = in00;
	pOut->m[1]  = in01;
	pOut->m[2]  = in02;
	pOut->m[3]  = in03;
	pOut->m[4]  = in10;
	pOut->m[5]  = in11;
	pOut->m[6]  = in12;
	pOut->m[7]  = in13;
	pOut->m[8]  = in20;
	pOut->m[9]  = in21;
	pOut->m[10] = in22;
	pOut->m[11] = in23;
	pOut->m[12] = in30;
	pOut->m[13] = in31;
	pOut->m[14] = in32;
	pOut->m[15] = in33;
}
// 构建世界变换矩阵(从相机坐标系变换至球坐标系下)
void calcWorldMatrix44(const f_float64_t x, const f_float64_t y, const f_float64_t z, LPMatrix44 pMxWorld)
{
	PT_3D ptPos, ptUp, pNor, pEast, pNorth;
	
	vector3DMemSet(&ptPos, x, y, z);			
	vector3DSetValue(&ptUp, &ptPos);			
	vector3DNormalize(&ptUp);	
	vector3DMemSet(&pNor, 0, 0, 1);				
	vector3DCrossProduct(&pEast, &pNor, &ptUp);
	vector3DNormalize(&pEast);
	vector3DCrossProduct(&pNorth, &pEast, &ptUp);
	vector3DNormalize(&pNorth);

	// 相机坐标系的xyz坐标轴基向量作为旋转矩阵|相机原点坐标值作为平移矩阵,最终组成世界变换矩阵
	dMatrix44MemSet(pMxWorld, pEast.x,   pEast.y,   pEast.z,  0.0,
							  ptUp.x,    ptUp.y,    ptUp.z,   0.0,
						      pNorth.x,  pNorth.y,  pNorth.z, 0.0,
						      ptPos.x,   ptPos.y,   ptPos.z,  1.0 );
}

/*
 *	计算旋转矩阵
 *	顺序旋转->缩放->平移
 */
void dMatrix44RPYmemSet( LPMatrix44 pOut, const f_float64_t pitch, const f_float64_t yaw, const f_float64_t roll )
{
	f_float64_t tPitch = DEGREE_TO_RADIAN(pitch);
	f_float64_t tYaw = DEGREE_TO_RADIAN(yaw);
	f_float64_t tRoll = DEGREE_TO_RADIAN(roll);
	f_float64_t tSinP = sin(tPitch);
	f_float64_t tCosP = cos(tPitch);
	f_float64_t tSinY = sin(tYaw);
	f_float64_t tCosY = cos(tYaw);
	f_float64_t tSinR = sin(tRoll); 
	f_float64_t tCosR = cos(tRoll);
	
	dMatrix44MemSet(pOut,  tCosR * tCosY + tSinR * tSinP * tSinY, tSinR * tCosP, -tCosR * tSinY + tSinR * tSinP * tCosY, 0.0,
						  -tSinR * tCosY+ tCosR * tSinP * tSinY,  tCosR * tCosP,  tSinR * tSinY + tCosR * tSinP * tCosY, 0.0,
						   tCosP * tSinY,                        -tSinP,          tCosP * tCosY,		                 0.0,
						   0.0,                                   0.0,            0.0,                                   1.0);
}

/*	向量乘以齐次矩阵 */
void dMatrix44Multi(LP_PT_3D pOut, const LP_PT_3D pIn, const LPMatrix44 mx)
{
	PT_4D tempIn, tempOut;
	vector4DMemSet(&tempIn, pIn->x, pIn->y, pIn->z, 1);

	tempOut.x = tempIn.x * mx->m[0] + tempIn.y * mx->m[4] + tempIn.z * mx->m[8] +  tempIn.w * mx->m[12];
	tempOut.y = tempIn.x * mx->m[1] + tempIn.y * mx->m[5] + tempIn.z * mx->m[9] +  tempIn.w * mx->m[13];
	tempOut.z = tempIn.x * mx->m[2] + tempIn.y * mx->m[6] + tempIn.z * mx->m[10] + tempIn.w * mx->m[14];
	tempOut.w = tempIn.x * mx->m[3] + tempIn.y * mx->m[7] + tempIn.z * mx->m[11] + tempIn.w * mx->m[15];
	if (fabs(tempOut.w) < FLT_EPSILON)
	    return;
	vector3DMemSet(pOut, tempOut.x / tempOut.w, tempOut.y / tempOut.w, tempOut.z / tempOut.w);
}

/* 设置4x4单位阵(64位浮点型) */
void dMatrix44Identity(f_float64_t m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

/*	4维向量(64位浮点型)乘以齐次矩阵 */
void vec4dMultMatrix(const f_float64_t matrix[16], const f_float64_t in[4], f_float64_t out[4])
{
    f_int32_t i;

    for (i=0; i<4; i++) 
    {
	    out[i] = in[0] * matrix[0*4+i] + in[1] * matrix[1*4+i] +
	             in[2] * matrix[2*4+i] + in[3] * matrix[3*4+i];
    }
}

/* 4x4矩阵(64位浮点型)相乘 */
void dMatrices44Mult(const f_float64_t in_left[16], const f_float64_t in_right[16], f_float64_t out[16])
{
    f_int32_t i, j;
    for (i = 0; i < 4; i++) 
    {
	    for (j = 0; j < 4; j++)
	    { 
	        out[i*4+j] = in_right[i*4+0] * in_left[0*4+j] + in_right[i*4+1] * in_left[1*4+j] +
	                     in_right[i*4+2] * in_left[2*4+j] + in_right[i*4+3] * in_left[3*4+j];
	    }
    }

	/*归一化*/
	if (fabs(out[15]) < FLT_EPSILON)
	    return;
	    
    for (i = 0;i<16;i++)
   		out[i] /= out[15];  
}

/* 4x4矩阵(64位浮点型)相乘 */
void dMatrices44MultNoNormal(const f_float64_t in_left[16], const f_float64_t in_right[16], f_float64_t out[16])
{
    f_int32_t i, j;

	f_float64_t out_he = 0.0;
	
    for (i = 0; i < 4; i++) 
    {
	    for (j = 0; j < 4; j++)
	    { 
	        out[i*4+j] = in_right[i*4+0] * in_left[0*4+j] + in_right[i*4+1] * in_left[1*4+j] +
	                     in_right[i*4+2] * in_left[2*4+j] + in_right[i*4+3] * in_left[3*4+j];

		out_he += sqr(out[i*4+j]);
	    }
    }

// 	out_he = sqrt(out_he);
// 
// 	for(i=0; i<16;i++)
// 		out[i] /= out_he;
}

#define MAT(mm,r,c) (mm)[(c)*4+(r)]
#define SWAP_ROWS(a, b) { f_float64_t *_tmp = a; (a)=(b); (b)=_tmp; }
/* 4x4矩阵(64位浮点型)求逆 */
BOOL dMatrix44Invert(const f_float64_t m[16], f_float64_t invOut[16])
{
   f_float64_t wtmp[4][8];
   f_float64_t m0, m1, m2, m3, s;
   f_float64_t *r0, *r1, *r2, *r3;
   r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

   r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
   r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
   r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

   r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
   r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
   r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

   r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
   r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
   r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

   r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
   r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
   r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

   /* choose pivot - or die */
   if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
   if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
   if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
   if (fabs(r0[0]) < FLT_EPSILON)
	   return(FALSE);

   /* eliminate first variable*/
   m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
   s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
   s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
   s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
   s = r0[4];
   if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r0[5];
   if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r0[6];
   if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r0[7];
   if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }
   /* choose pivot - or die */
   if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
   if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
   if (fabs(r1[1]) < FLT_EPSILON)
	   return(FALSE);

   /* eliminate second variable */
   m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
   r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
   r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
   s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
   s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
   s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
   s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

   /* choose pivot - or die */
   if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
   if (fabs(r2[2]) < FLT_EPSILON)
	   return(FALSE);

   /* eliminate third variable */
   m3 = r3[2]/r2[2];
   r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
   r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
   r3[7] -= m3 * r2[7];

   /* last check */
   if (fabs(r3[3]) < FLT_EPSILON)
	   return(FALSE);

   s = 1.0F/r3[3];             /* now back substitute row 3 */
   r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

   m2 = r2[3];                 /* now back substitute row 2 */
   s  = 1.0F/r2[2];
   r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
   r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
   m1 = r1[3];
   r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
   r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
   m0 = r0[3];
   r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
   r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

   m1 = r1[2];                 /* now back substitute row 1 */
   s  = 1.0F/r1[1];
   r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
   r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
   m0 = r0[2];
   r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
   r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

   m0 = r0[1];                 /* now back substitute row 0 */
   s  = 1.0F/r0[0];
   r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
   r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

   MAT(invOut,0,0) = r0[4]; MAT(invOut,0,1) = r0[5],
   MAT(invOut,0,2) = r0[6]; MAT(invOut,0,3) = r0[7],
   MAT(invOut,1,0) = r1[4]; MAT(invOut,1,1) = r1[5],
   MAT(invOut,1,2) = r1[6]; MAT(invOut,1,3) = r1[7],
   MAT(invOut,2,0) = r2[4]; MAT(invOut,2,1) = r2[5],
   MAT(invOut,2,2) = r2[6]; MAT(invOut,2,3) = r2[7],
   MAT(invOut,3,0) = r3[4]; MAT(invOut,3,1) = r3[5],
   MAT(invOut,3,2) = r3[6]; MAT(invOut,3,3) = r3[7];
   return(TRUE);
}
#undef SWAP_ROWS
#undef MAT

///////////////////////////////////////////////////////////////////////////////
/* 设置4x4单位阵(64位浮点型) */
void dMatrix44LoadIdentity(f_float64_t m[16])
{
	f_float64_t identity[16] = { 1.0, 0.0, 0.0, 0.0,
                                 0.0, 1.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0 };

	memcpy(m, identity, sizeof(f_float64_t) * 16);
}
	
///////////////////////////////////////////////////////////////////////////////
// Creates a 4x4 rotation matrix, takes degrees NOT radians
void createRotationMatrix44(f_float64_t angle, f_float64_t x, f_float64_t y, f_float64_t z, f_float64_t mMatrix[16])
{
    f_float64_t vecLength, sinSave, cosSave, oneMinusCos;
    f_float64_t xx, yy, zz, xy, yz, zx, xs, ys, zs;

    // Scale vector
    vecLength = sqrt( x*x + y*y + z*z );
    if(fabs(vecLength) < PRECISION)
    {
    	dMatrix44LoadIdentity(mMatrix);
        return;
    }

    // Rotation matrix is normalized
    x /= vecLength;
    y /= vecLength;
    z /= vecLength;
        
    sinSave = (f_float32_t)sin(angle);
    cosSave = (f_float32_t)cos(angle);
    oneMinusCos = 1.0 - cosSave;

    xx = x * x;
    yy = y * y;
    zz = z * z;
    xy = x * y;
    yz = y * z;
    zx = z * x;
    xs = x * sinSave;
    ys = y * sinSave;
    zs = z * sinSave;

    mMatrix[0] = (oneMinusCos * xx) + cosSave;
    mMatrix[4] = (oneMinusCos * xy) - zs;
    mMatrix[8] = (oneMinusCos * zx) + ys;
    mMatrix[12] = 0.0;

    mMatrix[1] = (oneMinusCos * xy) + zs;
    mMatrix[5] = (oneMinusCos * yy) + cosSave;
    mMatrix[9] = (oneMinusCos * yz) - xs;
    mMatrix[13] = 0.0;

    mMatrix[2] = (oneMinusCos * zx) - ys;
    mMatrix[6] = (oneMinusCos * yz) + xs;
    mMatrix[10] = (oneMinusCos * zz) + cosSave;
    mMatrix[14] = 0.0;

    mMatrix[3] = 0.0;
    mMatrix[7] = 0.0;
    mMatrix[11] = 0.0;
    mMatrix[15] = 1.0;
}

//矩阵拷贝函数
void dMatrix44Copy(LPMatrix44 pLeft, LPMatrix44 pRight)
{
	int i = 0;
	for (i = 0; i < 16; ++i)
	{
		pLeft->m[i] = pRight->m[i];
	}
	return;
}

//矩阵转置函数
void dMaxtrix44_Inversion(LPMatrix44 pInOut)
{
	f_float64_t m4, m8, m9, m12, m13, m14;
	m4 = pInOut->m[4];
	m8 = pInOut->m[8];
	m9 = pInOut->m[9];
	m12 = pInOut->m[12];
	m13 = pInOut->m[13];
	m14 = pInOut->m[14];

	pInOut->m[4] = pInOut->m[1];
	pInOut->m[8] = pInOut->m[2];
	pInOut->m[9] = pInOut->m[6];
	pInOut->m[12] = pInOut->m[3];
	pInOut->m[13] = pInOut->m[7];
	pInOut->m[14] = pInOut->m[11];

	pInOut->m[1] = m4;
	pInOut->m[2] = m8;
	pInOut->m[6] = m9;
	pInOut->m[3] = m12;
	pInOut->m[7] = m13;
	pInOut->m[11] = m14;
	return;
}

