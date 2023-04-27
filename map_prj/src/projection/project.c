#include <math.h>
#include "../define/macrodefine.h"
#include "../geometry/vectorMethods.h"
#include "../geometry/matrixMethods.h"
#include "project.h"

void avicglOrtho2D(f_float64_t left, f_float64_t right, f_float64_t bottom, f_float64_t top)
{
    glOrtho(left, right, bottom, top, -1, 1);
}

void avicglPerspective(f_float64_t fovy, f_float64_t aspect, f_float64_t zNear, f_float64_t zFar)
{
    f_float64_t m[4][4];
    f_float64_t sine, cotangent, deltaZ;
    f_float64_t radians = fovy * 0.5 * DE2RA;

    deltaZ = zFar - zNear;
    sine = sin(radians);
    if ((fabs(deltaZ) < FLT_EPSILON) || (fabs(sine) < FLT_EPSILON) || (fabs(aspect) < FLT_EPSILON)) 
    {
	    return;
    }
    cotangent = cos(radians) / sine;

    dMatrix44Identity(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1;
    m[3][2] = -2 * zNear * zFar / deltaZ;
    m[3][3] = 0;
    glMultMatrixd(&m[0][0]);
}

void avicglLookAt(f_float64_t eyex,    f_float64_t eyey,    f_float64_t eyez, 
                  f_float64_t centerx, f_float64_t centery, f_float64_t centerz, 
                  f_float64_t upx,     f_float64_t upy,     f_float64_t upz)
{
    f_float64_t forward[3], side[3], up[3];
    f_float64_t m[4][4];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    vector3DNormalize((LP_PT_3D)forward);

    /* Side = forward x up */
    vector3DCrossProduct((LP_PT_3D)side, (LP_PT_3D)forward, (LP_PT_3D)up);
    vector3DNormalize((LP_PT_3D)side);

    /* Recompute up as: up = side x forward */
    vector3DCrossProduct( (LP_PT_3D)up, (LP_PT_3D)side, (LP_PT_3D)forward);

    dMatrix44Identity(&m[0][0]);
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixd(&m[0][0]);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
    glTranslated(-eyex, -eyey, -eyez);
#else
	glTranslatef(-eyex, -eyey, -eyez);
#endif
}

//物体坐标-->视口坐标
BOOL avicglProject(f_float64_t objx, f_float64_t objy, f_float64_t objz, 
	               const f_float64_t modelViewMatrix[16], const f_int32_t viewport[4], 
	               f_float64_t *viewx, f_float64_t *viewy, f_float64_t *viewz)
{
    double in[4];
    double out[4];

    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    
    vec4dMultMatrix(modelViewMatrix, in, out);
    if (fabs(out[3]) < FLT_EPSILON) 
        return(FALSE);
        
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    /* Map x, y and z to range 0-1 */
    out[0] = out[0] * 0.5 + 0.5;
    out[1] = out[1] * 0.5 + 0.5;
    out[2] = out[2] * 0.5 + 0.5;

    /* Map x,y to viewport */
    out[0] = out[0] * viewport[2];// + viewport[0];
    out[1] = out[1] * viewport[3];// + viewport[1];

    *viewx=out[0];
    *viewy=out[1];
    *viewz=out[2];
    return(TRUE);
}

//视口坐标-->物体坐标
BOOL avicglUnProject(f_float64_t viewx, f_float64_t viewy, f_float64_t viewz,
		             const f_float64_t invModelViewMatrix[16], const f_int32_t viewport[4],
	                 f_float64_t *objx, f_float64_t *objy, f_float64_t *objz)
{
    f_float64_t in[4], out[4];

    in[0] = viewx;
    in[1] = viewy;
    in[2] = viewz;
    in[3] = 1.0;

    /* Map x and y from window coordinates */
    //in[0] = (in[0] - viewport[0]) / viewport[2];
    //in[1] = (in[1] - viewport[1]) / viewport[3];
	in[0] = in[0] / viewport[2];
    in[1] = in[1] / viewport[3];

    /* Map to range -1 to 1 */
    in[0] = in[0] * 2.0 - 1.0;
    in[1] = in[1] * 2.0 - 1.0;
    in[2] = in[2] * 2.0 - 1.0;

    vec4dMultMatrix(invModelViewMatrix, in, out);
    if (fabs(out[3]) < FLT_EPSILON) 
        return(FALSE);
        
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];
    return(TRUE);
}


/*通过模视矩阵计算点的视觉Z坐标*/
BOOL avicgetViewcoordZ(const f_float64_t modelViewMatrix[16], 
	f_float64_t objx, f_float64_t objy, f_float64_t objz, f_float64_t *pZ)
{
	f_float64_t *m = modelViewMatrix;
	if(NULL == m)
		return(FALSE);

	*pZ = m[2] * objx + m[6] * objy + m[10] * objz + m[14];

	return(TRUE);
}
