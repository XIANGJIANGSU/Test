#include <math.h>
#ifdef WIN32
#include "../map_prj/src/define/macrodefine.h"
#include "../map_prj/src/define/mbaseType.h"
#else
#include "../define/macrodefine.h"
#include "../define/mbaseType.h"
#endif
#include "simpleModels.h"

/*
���ܣ����������
���룺width, height, length    ������ĳ��������(��λ:��)
�������
*/
void avicCube(f_float64_t width, f_float64_t height, f_float64_t length)
{	
	PT_3D pt_blb, pt_blt, pt_brb, pt_brt, pt_tlb, pt_tlt, pt_trb, pt_trt;   /* �������8������ */
	f_float64_t hf_width = 0.0, hf_height = 0.0;    
    
	hf_width  = 0.5 * width;
	hf_height = 0.5 * height;
    pt_blb.x = pt_tlb.x = -hf_width;
    pt_blb.y = pt_tlb.y = -hf_height;
    pt_blb.z = 0.0 ;
	pt_tlb.z = length;
    
    pt_brb.x = pt_trb.x = hf_width;
    pt_brb.y = pt_trb.y = -hf_height;
    pt_brb.z = 0.0 ;
	pt_trb.z = length;
    
    pt_brt.x = pt_trt.x = hf_width;
    pt_brt.y = pt_trt.y = hf_height;
    pt_brt.z = 0.0 ;
	pt_trt.z = length;
    
    pt_blt.x = pt_tlt.x = -hf_width;
    pt_blt.y = pt_tlt.y = hf_height;
    pt_blt.z = 0.0 ;
	pt_tlt.z = length;
	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisableEx(GL_CULL_FACE);
		
	glBegin(GL_QUAD_STRIP);
		glVertex3d(pt_tlb.x, pt_tlb.y, pt_tlb.z);
		glVertex3d(pt_blb.x, pt_blb.y, pt_blb.z);
		glVertex3d(pt_trb.x, pt_trb.y, pt_trb.z);
		glVertex3d(pt_brb.x, pt_brb.y, pt_brb.z);
		glVertex3d(pt_trt.x, pt_trt.y, pt_trt.z);
		glVertex3d(pt_brt.x, pt_brt.y, pt_brt.z);
		glVertex3d(pt_tlt.x, pt_tlt.y, pt_tlt.z);
		glVertex3d(pt_blt.x, pt_blt.y, pt_blt.z);
		glVertex3d(pt_tlb.x, pt_tlb.y, pt_tlb.z);
		glVertex3d(pt_blb.x, pt_blb.y, pt_blb.z);
	glEnd();

	glBegin(GL_QUADS);
	    glVertex3d(pt_blb.x, pt_blb.y, pt_blb.z);
		glVertex3d(pt_brb.x, pt_brb.y, pt_brb.z);
		glVertex3d(pt_brt.x, pt_brt.y, pt_brt.z);
		glVertex3d(pt_blt.x, pt_blt.y, pt_blt.z);

        glVertex3d(pt_tlb.x, pt_tlb.y, pt_tlb.z);
		glVertex3d(pt_trb.x, pt_trb.y, pt_trb.z);
		glVertex3d(pt_trt.x, pt_trt.y, pt_trt.z);
		glVertex3d(pt_tlt.x, pt_tlt.y, pt_tlt.z);
	glEnd();
    
	//glDisableEx(GL_BLEND);
	glBegin(GL_LINE_LOOP);
	    glVertex3d(pt_blb.x, pt_blb.y, pt_blb.z);
		glVertex3d(pt_brb.x, pt_brb.y, pt_brb.z);
		glVertex3d(pt_brt.x, pt_brt.y, pt_brt.z);
		glVertex3d(pt_blt.x, pt_blt.y, pt_blt.z);
	glEnd();

	glBegin(GL_LINE_LOOP);
	    glVertex3d(pt_tlb.x, pt_tlb.y, pt_tlb.z);
		glVertex3d(pt_trb.x, pt_trb.y, pt_trb.z);
		glVertex3d(pt_trt.x, pt_trt.y, pt_trt.z);
		glVertex3d(pt_tlt.x, pt_tlt.y, pt_tlt.z);
	glEnd();
    
	glBegin(GL_LINES);
	    glVertex3d(pt_blb.x, pt_blb.y, pt_blb.z);
		glVertex3d(pt_tlb.x, pt_tlb.y, pt_tlb.z);

		glVertex3d(pt_brb.x, pt_brb.y, pt_brb.z);
		glVertex3d(pt_trb.x, pt_trb.y, pt_trb.z);

		glVertex3d(pt_brt.x, pt_brt.y, pt_brt.z);
		glVertex3d(pt_trt.x, pt_trt.y, pt_trt.z);

		glVertex3d(pt_blt.x, pt_blt.y, pt_blt.z);
		glVertex3d(pt_tlt.x, pt_tlt.y, pt_tlt.z);
	glEnd();
	glDisableEx(GL_BLEND);		
}

/*
���ܣ��������
���룺radius    ����İ뾶(��λ:��)
      slices    ����İ���
      stacks    ����Ĳ���
�������
*/
void avicSphere(f_float64_t radius, f_int32_t slices, f_int32_t stacks)
{
#define CACHE_SIZE (360)
    f_int32_t i, j, start, finish;
    GLfloat sinCache1a[CACHE_SIZE], cosCache1a[CACHE_SIZE], sinCache1b[CACHE_SIZE], cosCache1b[CACHE_SIZE], angle;

    if (slices >= CACHE_SIZE) slices = CACHE_SIZE-1;
    if (stacks >= CACHE_SIZE) stacks = CACHE_SIZE-1;
    if (slices < 2 || stacks < 1 || radius < 0.0) 
    {
	    DEBUG_PRINT("avicSphere input param error.");
	    return;
    }

    for (i = 0; i < slices; i++) 
    {
	    angle = 2 * PI * i / slices;
	    sinCache1a[i] = sin(angle);
	    cosCache1a[i] = cos(angle);
    }

    for (j = 0; j <= stacks; j++) 
    {
	    angle = PI * j / stacks;
	    sinCache1b[j] = radius * sin(angle);
	    cosCache1b[j] = radius * cos(angle);
    }
    /* Make sure it comes to a point */
    sinCache1b[0] = 0;
    sinCache1b[stacks] = 0;
    sinCache1a[slices] = sinCache1a[0];
    cosCache1a[slices] = cosCache1a[0];
 	
	start = 1;
	finish = stacks - 1;

    glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisableEx(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, 0.0, radius);
	for (i = slices; i >= 0; i--) 
	{
	    glVertex3f(sinCache1b[1] * sinCache1a[i],  sinCache1b[1] * cosCache1a[i], cosCache1b[1]);
	} 
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, 0.0, -radius);
	for (i = 0; i <= slices; i++) 
	{
	    glVertex3f(sinCache1b[stacks-1] * sinCache1a[i], sinCache1b[stacks-1] * cosCache1a[i], cosCache1b[stacks-1]);
	} 
	glEnd();
		
	for (j = start; j < finish; j++) 
	{
	    glBegin(GL_QUAD_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {
	    	glVertex3f(sinCache1b[j]   * sinCache1a[i], sinCache1b[j]   * cosCache1a[i], cosCache1b[j]);
	    	glVertex3f(sinCache1b[j+1] * sinCache1a[i], sinCache1b[j+1] * cosCache1a[i], cosCache1b[j+1]);
		}
	    glEnd();
	}
	glDepthMask(GL_TRUE);

	for (j = 1; j < stacks; j++) 
	{
	    glBegin(GL_LINE_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {		
		    glVertex3f(sinCache1b[j] * sinCache1a[i], sinCache1b[j] * cosCache1a[i], cosCache1b[j]);
	    }
	    glEnd();
	}
	
	for (i = 0; i < slices; i++) 
	{
	    glBegin(GL_LINE_STRIP);
	    for (j = 0; j <= stacks; j++) 
	    {		
		    glVertex3f(sinCache1a[i] * sinCache1b[j], cosCache1a[i] * sinCache1b[j], cosCache1b[j]);
	    }
	    glEnd();
	}	
	glDisableEx(GL_BLEND);
#undef CACHE_SIZE	
}

/*
���ܣ��������
���룺baseRadius    ����ĵ���뾶(��λ:��)
      topRadius     ����Ķ���뾶(��λ:��)
      height        ����ĸ߶�(��λ:��)
      slices        ����İ���
      stacks        ����Ĳ���
      needDrawTop   �����Ƿ���Ƶı�־
�������
*/
void avicCylinder(f_float64_t baseRadius, f_float64_t topRadius, f_float64_t height, f_int32_t slices, f_int32_t stacks, BOOL needDrawTop)
{
#define CACHE_SIZE (360)
    f_int32_t i,j;    
    f_float32_t sinCache[CACHE_SIZE], cosCache[CACHE_SIZE];
    f_float32_t angle, zLow, zHigh, length, deltaRadius, xyNormalRatio;
    f_float32_t radiusLow, radiusHigh;

    if (slices >= CACHE_SIZE) 
        slices = CACHE_SIZE-1;

    if (slices < 2 || stacks < 1 || baseRadius < 0.0 || topRadius < 0.0 || height < 0.0) 
    {
	    return;
    }

    /* Compute length (needed for normal calculations) */
    deltaRadius = baseRadius - topRadius;
    length = sqrt(deltaRadius*deltaRadius + height*height);
    if (fabs(length) < FLT_EPSILON) 
    {
	    return;
    }    
    xyNormalRatio = height / length;

    for (i = 0; i < slices; i++) 
    {
	    angle = 2.0 * PI * i / slices;
	    sinCache[i] = sin(angle);
	    cosCache[i] = cos(angle);
    }
    sinCache[slices] = sinCache[0];
    cosCache[slices] = cosCache[0];

    glEnableEx(GL_BLEND);		
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    /* ���� */
	for (j = 0; j < stacks; j++) 
	{
	    zLow = j * height / stacks;
	    zHigh = (j + 1) * height / stacks;
	    radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
	    radiusHigh = baseRadius - deltaRadius * ((float) (j + 1) / stacks);

	    glBegin(GL_QUAD_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {	    
		    glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], zLow);		    
		    glVertex3f(radiusHigh * sinCache[i], radiusHigh * cosCache[i], zHigh);
		}
	    glEnd();
	}
	
	if(needDrawTop && (topRadius > FLT_EPSILON))
	{
	    glBegin(GL_TRIANGLE_FAN); /* ���˵��� */
	    glVertex3f(0.0, 0.0, height);
	    for (i = 0; i <= slices; i++) 
	    {
		    glVertex3f(topRadius * sinCache[i], topRadius * cosCache[i], height);
	    }
	    glEnd();
    }	    
	//glDisableEx(GL_BLEND);
	
#if 0	
	for (j = 1; j < stacks; j++) /* �м�ĺ����� */
	{
	    zLow = j * height / stacks;
	    radiusLow = baseRadius - deltaRadius * ((float) j / stacks);
	    
	    glBegin(GL_LINE_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {		
		    glVertex3f(radiusLow * sinCache[i],	radiusLow * cosCache[i], zLow);
	    }
	    glEnd();
	}
#endif

	/* Intentionally fall through here... */
	for (j = 0; j <= stacks; j += stacks) /* ���˺͵ײ��ĺ����� */
	{
	    zLow = j * height / stacks;
	    radiusLow = baseRadius - deltaRadius * ((float) j / stacks);

	    glBegin(GL_LINE_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {		
		    glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], zLow);
	    }
	    glEnd();
	}
	
	for (i = 0; i <= slices; i++) /* ������ */
	{		
	    glBegin(GL_LINE_STRIP);
	    for (j = 0; j <= stacks; j++) 
	    {
		    zLow = j * height / stacks;
		    radiusLow = baseRadius - deltaRadius * ((float) j / stacks);		
		    glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], zLow);
	    }
	    glEnd();
	}
	
	if(needDrawTop && (topRadius > FLT_EPSILON)) 
	{
	    glBegin(GL_LINES); /* ���˵��� */	
	    for (i = 0; i <= slices; i++) 
	    {
		    glVertex3f(0.0, 0.0, height);
		    glVertex3f(topRadius * sinCache[i], topRadius * cosCache[i], height);
	    }
	    glEnd();
	}
    glDisableEx(GL_BLEND);
#undef CACHE_SIZE    
}

/*
���ܣ����岿�ֵĻ���
���룺innerRadius    ������ڰ뾶(��λ:��)
      outerRadius    �������뾶(��λ:��)
      slices         ����İ���
      loops          ����Ĳ���
      startAngle     �������ʼ�Ƕ�(��λ:��)
      sweepAngle     �Ƕȿ��(��λ:��)
�������
*/
void avicPartialDisk(f_float64_t innerRadius, f_float64_t outerRadius, 
                     f_int32_t slices, f_int32_t loops, f_float64_t startAngle, f_float64_t sweepAngle)
{
#define CACHE_SIZE (360)	
    f_int32_t i, j, slices2, finish;
    f_float32_t sinCache[CACHE_SIZE], cosCache[CACHE_SIZE];
    f_float32_t angle, deltaRadius, radiusLow, radiusHigh, angleOffset;

    if (slices >= CACHE_SIZE) 
        slices = CACHE_SIZE-1;
    if (slices < 2 || loops < 1 || outerRadius <= 0.0 || innerRadius < 0.0 || innerRadius > outerRadius) 
    {
	    DEBUG_PRINT("avicPartialDisk input param error.");
	    return;
    }

    if (sweepAngle < -360.0) 
        sweepAngle = 360.0;
    if (sweepAngle > 360.0) 
        sweepAngle = 360.0;
    if (sweepAngle < 0) 
    {
	    startAngle += sweepAngle;
	    sweepAngle = -sweepAngle;
    }

    if (fabs(sweepAngle - 360.0) < FLT_EPSILON) 
    {
	    slices2 = slices;
    } 
    else 
    {
	    slices2 = slices + 1;
    }

    deltaRadius = outerRadius - innerRadius;
    angleOffset = startAngle * DE2RA;
    for (i = 0; i <= slices; i++) 
    {
	    angle = angleOffset + sweepAngle * DE2RA * i / slices;
	    sinCache[i] = sin(angle);
	    cosCache[i] = cos(angle);
    }

    if (fabs(sweepAngle - 360.0) < FLT_EPSILON) 
    {
	    sinCache[slices] = sinCache[0];
	    cosCache[slices] = cosCache[0];
    }

    glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	if (fabs(innerRadius) < FLT_EPSILON) 
	{
	    finish = loops - 1;
	    glBegin(GL_TRIANGLE_FAN);	    
	    glVertex3f(0.0, 0.0, 0.0);
	    radiusLow = outerRadius - deltaRadius * ((float) (loops-1) / loops);
		for (i = slices; i >= 0; i--) 
		{		    
		    glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);
		} 
	    glEnd();
	} 
	else 
	{
	    finish = loops;
	}
	for (j = 0; j < finish; j++) 
	{
	    radiusLow = outerRadius - deltaRadius * ((float) j / loops);
	    radiusHigh = outerRadius - deltaRadius * ((float) (j + 1) / loops);
	    
	    glBegin(GL_QUAD_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {
		    glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);		    
		    glVertex3f(radiusHigh * sinCache[i], radiusHigh * cosCache[i], 0.0);	
	    }
	    glEnd();
	}
	//glDisableEx(GL_BLEND);

	if (fabs(innerRadius - outerRadius) < FLT_EPSILON) 
	{
	    glBegin(GL_LINE_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {
		    glVertex3f(innerRadius * sinCache[i], innerRadius * cosCache[i], 0.0);
	    }
	    glEnd();
	}
	else
	{
	    for (j = 0; j <= loops; j++) 
	    {
	        radiusLow = outerRadius - deltaRadius * ((float) j / loops);
	        glBegin(GL_LINE_STRIP);
	        for (i = 0; i <= slices; i++) 
	        {		
		        glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);
	        }
	        glEnd();
	    }
	    
	    for (i=0; i < slices2; i++) 
	    {
	        glBegin(GL_LINE_STRIP);
	        for (j = 0; j <= loops; j++) 
	        {
		        radiusLow = outerRadius - deltaRadius * ((float) j / loops);
				glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);
	        }
	        glEnd();
	    }
    }

	if (sweepAngle < 360.0) 
	{
	    for (i = 0; i <= slices; i+= slices) 
	    {
		    glBegin(GL_LINE_STRIP);
		    for (j = 0; j <= loops; j++) 
		    {
		        radiusLow = outerRadius - deltaRadius * ((float) j / loops);		    
		        glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);
		    }
		    glEnd();
	    }
	}
	
	for (j = 0; j <= loops; j += loops) 
	{
	    radiusLow = outerRadius - deltaRadius * ((float) j / loops);	    
	    glBegin(GL_LINE_STRIP);
	    for (i = 0; i <= slices; i++) 
	    {
			glVertex3f(radiusLow * sinCache[i], radiusLow * cosCache[i], 0.0);
	    }
	    glEnd();
	    if (fabs(innerRadius - outerRadius) < FLT_EPSILON) 
	        break;
	}
    glDisableEx(GL_BLEND);
#undef CACHE_SIZE    
}

/*
���ܣ�Բ�̵Ļ���
���룺innerRadius    Բ�̵��ڰ뾶(��λ:��)
      outerRadius    Բ�̵���뾶(��λ:��)
      slices         Բ�̵İ���
      loops          Բ�̵Ĳ���
�������
*/
void avicDisk(f_float64_t innerRadius, f_float64_t outerRadius, f_int32_t slices, f_int32_t loops)
{
    avicPartialDisk(innerRadius, outerRadius, slices, loops, 0.0, 360.0);
}

/*
���ܣ�����������
���룺edgeLength  �������߳�
�������
*/
void avicPyramid(f_float64_t edgeLength)
{
	PT_3D pt[4];
	
	f_float64_t hf_edge = 0.5 * edgeLength;
	f_float64_t tscale = sqrt(3.0) / 3.0;
	pt[0].x = -hf_edge;
	pt[0].y = -hf_edge * tscale;
	pt[0].z = 0.0;
	
	pt[1].x = hf_edge;
	pt[1].y = -hf_edge * tscale;
	pt[1].z = 0.0;
	
	pt[2].x = 0.0;
	pt[2].y = edgeLength * tscale;
	pt[2].z = 0.0;
	
	pt[3].x = 0.0;
	pt[3].y = 0.0;
	pt[3].z = edgeLength * tscale * sqrt(2.0);
	
	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	glBegin(GL_TRIANGLES);
	    //glVertex3d(pt[0].x, pt[0].y, pt[0].z);
	    //glVertex3d(pt[1].x, pt[1].y, pt[1].z);
	    //glVertex3d(pt[2].x, pt[2].y, pt[2].z);
	    
	    glVertex3d(pt[0].x, pt[0].y, pt[0].z);
	    glVertex3d(pt[1].x, pt[1].y, pt[1].z);
	    glVertex3d(pt[3].x, pt[3].y, pt[3].z);
	    
	    glVertex3d(pt[1].x, pt[1].y, pt[1].z);
	    glVertex3d(pt[2].x, pt[2].y, pt[2].z);
	    glVertex3d(pt[3].x, pt[3].y, pt[3].z);
	    
	    glVertex3d(pt[0].x, pt[0].y, pt[0].z);
	    glVertex3d(pt[2].x, pt[2].y, pt[2].z);
	    glVertex3d(pt[3].x, pt[3].y, pt[3].z);
	glEnd();	
	//glDisableEx(GL_BLEND);
	
	glPolygonMode(GL_FRONT_AND_BACK , GL_LINE);
	glBegin(GL_TRIANGLES);
	    //glVertex3d(pt[0].x, pt[0].y, pt[0].z);
	    //glVertex3d(pt[1].x, pt[1].y, pt[1].z);
	    //glVertex3d(pt[2].x, pt[2].y, pt[2].z);
	    
	    glVertex3d(pt[0].x, pt[0].y, pt[0].z);
	    glVertex3d(pt[1].x, pt[1].y, pt[1].z);
	    glVertex3d(pt[3].x, pt[3].y, pt[3].z);
	    
	    glVertex3d(pt[1].x, pt[1].y, pt[1].z);
	    glVertex3d(pt[2].x, pt[2].y, pt[2].z);
	    glVertex3d(pt[3].x, pt[3].y, pt[3].z);
	    
	    glVertex3d(pt[0].x, pt[0].y, pt[0].z);
	    glVertex3d(pt[2].x, pt[2].y, pt[2].z);
	    glVertex3d(pt[3].x, pt[3].y, pt[3].z);
	glEnd();
	glDisableEx(GL_BLEND);	
	glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);
}
