#include <math.h>
#include <string.h>
#include "../utility/tools.h"
#include "../projection/project.h"
#include "../projection/coord.h"
#include "mapNodeJudge.h"
#include "mapNodeGen.h"


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

// Ritter���������Χ��뾶 pNode_center = ComputeBoundingSphereRitter(pNode, 4, &radiu);
PT_3F ComputeBoundingSphereRitter(LP_PT_3F vertices, unsigned int vertex_num, f_float64_t * radiu_out)
{
	unsigned int i = 0;
	PT_3F center = {0};
	f_float64_t radius = 0.0;

	unsigned int maxX = 0, maxY = 0, maxZ = 0, minX = 0 , minY = 0, minZ = 0;

	float x = 0;
	float y = 0;
	float z = 0;

	float dia = 0.0;
	int maxIndex = maxX, minIndex = minX;

	// ȷ��ͼ������
	for(i = 0; i < vertex_num; i++)
	{
		if(vertices[i].x > vertices[maxX].x) maxX = i;
		if(vertices[i].y > vertices[maxY].y) maxY= i;
		if(vertices[i].z > vertices[maxZ].z) maxZ= i;

		if(vertices[i].x < vertices[minX].x) minX = i;
		if(vertices[i].y < vertices[minY].y) minY= i;
		if(vertices[i].z < vertices[minZ].z) minZ= i;
	}

	// ������Խ���,����Ϊ��ʼ�뾶����
	{
		// �ԶԽ��߳�����Ϊ��ʼx����
		{
			PT_3F sub1 = {0}, sub2 = {0}, d = {0};
			sub1.x = vertices[maxX].x;
			sub1.y = vertices[maxX].y;
			sub1.z = vertices[maxX].z;
			sub2.x = vertices[minX].x;
			sub2.y = vertices[minX].y;
			sub2.z = vertices[minX].z;
			vector3FSub(&d, &sub1, &sub2);
			x = vector3FDotProduct(&d, &d);	// ���ڵ�˼��㳤��
		}

		// �ԶԽ��߳�����Ϊ��ʼy����
		{
			PT_3F sub1 = {0}, sub2 = {0}, d = {0};
			sub1.x = vertices[maxY].x;
			sub1.y = vertices[maxY].y;
			sub1.z = vertices[maxY].z;
			sub2.x = vertices[minY].x;
			sub2.y = vertices[minY].y;
			sub2.z = vertices[minY].z;
			vector3FSub(&d, &sub1, &sub2);
			y = vector3FDotProduct(&d, &d);
		}

		// �ԶԽ��߳�����Ϊ��ʼz����
		{
			PT_3F sub1 = {0}, sub2 = {0}, d = {0};
			sub1.x = vertices[maxZ].x;
			sub1.y = vertices[maxZ].y;
			sub1.z = vertices[maxZ].z;
			sub2.x = vertices[minZ].x;
			sub2.y = vertices[minZ].y;
			sub2.z = vertices[minZ].z;
			vector3FSub(&d, &sub1, &sub2);
			z = vector3FDotProduct(&d, &d);
		}
		
		// �ҵ��� x|y|z �������
		{
			dia = x;
			maxIndex = maxX, minIndex = minX;
			if((z > x)&&(z > y))
			{
				maxIndex = maxZ;
				minIndex = minZ;
				dia = z;
			}
			else if((y > x)&&(y > z))
			{
				maxIndex = maxY;
				minIndex = minY;
				dia = y;
			}
		}

		// ������Խ����ϵ����ĵ�compute the center point
		center.x = 0.5 * (vertices[maxIndex].x + vertices[minIndex].x);
		center.y = 0.5 * (vertices[maxIndex].y + vertices[minIndex].y);
		center.z = 0.5 * (vertices[maxIndex].z + vertices[minIndex].z);
		radius = 0.5 * sqrt(dia);
	}

	// �������ĵ�����������Զ����,��Ϊ��Ϊ��Χ�а뾶
	for(i = 0; i < vertex_num; i++)
	{
		PT_3F d = {0};
		f_float64_t dist2 = 0.0;
		
		vector3FSub(&d, &vertices[i], &center);
		dist2 = vector3FDotProduct(&d, &d);

		if(dist2  > radius * radius)
		{
			f_float64_t dist = sqrt(dist2);
			f_float64_t newRadius = (dist + radius ) * 0.5;
			f_float64_t k = (newRadius - radius) / dist;
			PT_3F temp = {0};
			
			radius = newRadius;
			vector3FMul(&temp, &d, k);
			vector3FAdd(&center, &center, &temp);
		}
	}

	* radiu_out = radius;
	return center;
}

extern void geo2obj(f_float64_t lon, f_float64_t lat, f_float64_t ht,
                    f_float64_t *x, f_float64_t *y, f_float64_t *z);


//ͨ��OSG �ķ���������Ƭ�����ĵ㼰��Χ��뾶lpf add 2017��11��27��14:14:25
static int pNode_vertex_num = (VTXCOUNT1 * VTXCOUNT1 + 4);
static PT_3F pNode_vertex[VTXCOUNT1 * VTXCOUNT1 + 4] = {0};


sNODERANGE getNodeRangeOSG(sQTMAPNODE* pNode)
{
	sNODERANGE nodeRange;

	f_float64_t dis = 0.0, dis1 = 0.0;

	PT_3F pNode_center = {0};

	nodeRange.x = 0.0;
	nodeRange.y = 0.0;
	nodeRange.z = 0.0;
	nodeRange.radius = 0.0;
	nodeRange.radiusMin = 0.0;

	if (pNode == NULL)
		return nodeRange;
	
    	{
    		nodeRange.x = pNode->obj_cent.x;
	    	nodeRange.y = pNode->obj_cent.y;
	    	nodeRange.z = pNode->obj_cent.z;
	    	dis  = caculateGreatCircleDis(pNode->hstart, 
		                              pNode->vstart + 0.5 * pNode->vscale,
						 pNode->hstart + 0.5 * pNode->hscale, 
		                              pNode->vstart + 0.5 * pNode->vscale); 
        	dis1 = caculateGreatCircleDis(pNode->hstart + 0.5 * pNode->hscale, 
		                              pNode->vstart,
						 pNode->hstart + 0.5 * pNode->hscale, 
		                              pNode->vstart + 0.5 * pNode->vscale);	
        

        	dis = max(dis, dis1);	                              	                             
		nodeRange.radiusMin = dis;
    	}	

	//�����Χ��Ͱ�Χ��뾶
	memcpy(&pNode_vertex[0].x, 
		pNode->fvertex, 
		VTXCOUNT1 * VTXCOUNT1  * 3 * sizeof(f_float32_t));
	
//	pNode_center = ComputeBoundingSphereRitter(pNode_vertex, pNode_vertex_num, &nodeRange.radius);
	pNode_center = ComputeBoundingSphereRitter(pNode_vertex, VTXCOUNT1 * VTXCOUNT1, &nodeRange.radius);

	nodeRange.x = pNode_center.x;
	nodeRange.y = pNode_center.y;
	nodeRange.z = pNode_center.z;

	
	return nodeRange;
	
}

/*
���ܣ���ȡ�����ӽ�����Ƭ�ڵ�İ�Χ��4������(�߶�Ϊ0������Ϊ���½ǡ����½ǡ����Ͻǡ����Ͻ�) + 4������(�߶�Ϊ-1000������Ϊ���½ǡ����½ǡ����Ͻǡ����Ͻ�)
      
���룺
    pNode      ��Ƭ�ڵ�ָ��

�����    
    sNODERANGE  ��Χ��ṹ��(�������ĵ����ꡢ��Χ��뾶����С��Χ��뾶)
*/
sNODERANGE getNodeRangeOSG_Overlook(sQTMAPNODE* pNode)
{
	sNODERANGE nodeRange = {0};
	PT_3F pNode_center = {0};
	double x = 0.0, y =0.0, z=0.0;
	int i = 0;
	PT_3F pNode_vertex_overlook[4] = {0};
	double hei_offset = -1000.0;

	/* �����Ƭ�ڵ�ָ��Ϊ��,�򷵻صİ�Χ��ṹ���еĲ�����Ϊ0 */
	if (pNode == NULL)
		return nodeRange;

	/* ����8����Ƭ�����������,����4������(�߶�Ϊ0������Ϊ���½ǡ����½ǡ����Ͻǡ����Ͻ�) + 4������(�߶�Ϊ-1000������Ϊ���½ǡ����½ǡ����Ͻǡ����Ͻ�) */
	for(i = 0 ; i < 8; i ++)
	{
		switch(i)
		{		
			case 0:		
				geo2obj(pNode->hstart, pNode->vstart, 0.0, &x, &y, &z);	
				break;
			case 1:		
				geo2obj(pNode->hstart + pNode->hscale, pNode->vstart, 0.0, &x, &y, &z);	
				break;
			case 2:		
				geo2obj(pNode->hstart + pNode->hscale, pNode->vstart + pNode->vscale, 0.0, &x, &y, &z);	
				break;
			case 3:		
				geo2obj(pNode->hstart, pNode->vstart+ pNode->vscale, 0.0, &x, &y, &z);	
				break;
			case 4:		
				geo2obj(pNode->hstart, pNode->vstart, hei_offset, &x, &y, &z);	
				break;
			case 5:		
				geo2obj(pNode->hstart + pNode->hscale, pNode->vstart, hei_offset, &x, &y, &z);	
				break;
			case 6:		
				geo2obj(pNode->hstart + pNode->hscale, pNode->vstart + pNode->vscale, hei_offset, &x, &y, &z);	
				break;
			case 7:		
				geo2obj(pNode->hstart, pNode->vstart+ pNode->vscale, hei_offset, &x, &y, &z);	
				break;
			default:
				break;
		}

		pNode->fvertex_overlook[3 * i  + 0] = x;
		pNode->fvertex_overlook[3 * i  + 1] = y;
		pNode->fvertex_overlook[3 * i  + 2] = z;
		
	}

	/* ����ǰ4�����������ֵ������pNode_vertex_overlook�� */
	memcpy(&pNode_vertex_overlook[0].x, 
		pNode->fvertex_overlook, 
		4 * 3 * sizeof(f_float32_t));

	/* ������Ƭ��Χ������ĵ����꼰��Χ��뾶,����ʹ����Ƭ4���ǵ������ */
	pNode_center = ComputeBoundingSphereRitter(pNode_vertex_overlook, 4, &nodeRange.radius);
	nodeRange.x = pNode_center.x;
	nodeRange.y = pNode_center.y;
	nodeRange.z = pNode_center.z;

	return nodeRange;
}


sNODERANGE getNodeRange(sQTMAPNODE* pNode)
{
	sNODERANGE nodeRange;
	nodeRange.x = 0.0;
	nodeRange.y = 0.0;
	nodeRange.z = 0.0;
	nodeRange.radius = 0.0;
	nodeRange.radiusMin = 0.0;
	if (pNode == NULL)
		return nodeRange;
	
    {
    	f_float64_t dis = 0.0, dis1 = 0.0;
    	nodeRange.x = pNode->obj_cent.x;
	    nodeRange.y = pNode->obj_cent.y;
	    nodeRange.z = pNode->obj_cent.z;
	    dis  = caculateGreatCircleDis(pNode->hstart, 
		                              pNode->vstart + 0.5 * pNode->vscale,
						              pNode->hstart + 0.5 * pNode->hscale, 
		                              pNode->vstart + 0.5 * pNode->vscale); 
        dis1 = caculateGreatCircleDis(pNode->hstart + 0.5 * pNode->hscale, 
		                              pNode->vstart,
						              pNode->hstart + 0.5 * pNode->hscale, 
		                              pNode->vstart + 0.5 * pNode->vscale);	
        
        nodeRange.radius = sqrt(dis * dis + dis1 * dis1);	
        dis = max(dis, dis1);	                              	                             
        //dis = glfLevelDelta[pNode->level];
	    //nodeRange.radius = dis * 1.414;
	    nodeRange.radiusMin = dis;
    }	
	return nodeRange;
}

//����ֵ
// 0:׼����
// -1:�ӽڵ㲻����
// -2:�ӽڵ�����δ׼����
f_int32_t isAllChildNodeDataReady(sQTMAPNODE * pNode)
{
	f_int32_t ret = 0;
	f_int32_t i = 0;
	if (pNode == NULL)
		return -1;	
	
	for(i=0; i<4; i++)
	{
		if(NULL == pNode->children[i])
		{
//			printf("NULL %d, %d, %d\n", pNode->xidx, pNode->yidx, 
//				pNode->level);			
			return -1;

		}
		ret = qtmapnodeGetStatus(pNode->children[i], __LINE__);
		if(DNST_READY != ret)
		{
//			printf("not ready %d, %d, %d\n", pNode->children[i]->xidx, pNode->children[i]->yidx, 
//				pNode->children[i]->level);
			return -2;
		}		
	}
	
	/*ǰ��ѭ���ﶼû���أ���ô��������Ƭ���ݶ�׼����*/
	return 0;
}

/*
���ܣ���������(mapNodeRender)��,�ж�ָ���Ľڵ��Ƿ���������Ӿ�����ӷ�Χ��(���һ���ж����ӽڵ��Ƿ���)
      ���ڸ��ڵ����Ӿ�����,��16����ڵ�������Ӿ����ڵ����
���룺
    view_near  �Ӿ�����ý���
    view_far   �Ӿ���Զ�ý���
	pFrustum   �Ӿ���6����ķ���
    pNode      ��Ƭ�ڵ�ָ��
�����    
    0  ��
    -1 ����
*/
f_int32_t isNodeInFrustum(f_float32_t view_near, f_float32_t view_far, LPFRUSTUM pFrustum, sQTMAPNODE * pNode)
{
	f_int32_t ret = FALSE;
	f_float64_t dis = 0.0;
	sNODERANGE nodeRange;	

	if (pNode == NULL)
		return -1;	
	
	/* ��ȡ��Ƭ�ڵ�İ�Χ�� */
	nodeRange = pNode->nodeRange;
	
	/* ͨ���Ӿ����ж���Ƭ���߰�Χ��(�뾶��)�Ƿ����Ӿ�����ӷ�Χ�� FALSE(0)-���� TRUE(1)-�� */
	/* �������,������Ƭ�����Ӿ�����ӷ�Χ��,����-1 */
	ret = InFrustum(view_near, view_far, nodeRange.x, nodeRange.y, nodeRange.z, nodeRange.radius, pFrustum, &dis);
	if(FALSE == ret)
		return -1;	
	
	/* �����,��һ���ж���Ƭ�̱߰�Χ��(�뾶С)�Ƿ����Ӿ�����ӷ�Χ�� */
	ret = InFrustum(view_near, view_far, nodeRange.x, nodeRange.y, nodeRange.z, nodeRange.radiusMin, pFrustum, &dis);
	/* �����,������Ƭ���Ӿ�����ӷ�Χ��,����0;�������,��һ���жϸ���Ƭ��������Ƭ�Ƿ����Ӿ�����ӷ�Χ�� */
	if(TRUE == ret)
		return 0;
		
    /* �ж����ӽڵ��Ƿ����Ӿ�����ӷ�Χ�� */
    {
        f_int32_t i = 0, j = 0;
	    f_float64_t hstep = 0.0, vstep = 0.0, minRadius = 0.0;
	    Geo_Pt_D geoPt; 
	    Obj_Pt_D objPt;
	    
		/* һ���ڵ���16�����ӽڵ�,��Χ���������С1/4 */
	    minRadius = nodeRange.radiusMin * 0.25;
	    hstep = pNode->hscale * 0.25;
	    vstep = pNode->vscale * 0.25;
	    for(i = 0; i < 4; i++)
	    {
	        for(j = 0; j < 4; j++)	
	        {
	        	ret = FALSE;
	            geoPt.lon = pNode->hstart + hstep * i + hstep * 0.5;
	            geoPt.lat = pNode->vstart + vstep * j + vstep * 0.5;
	            geoPt.height = pNode->height_ct;
	            geoDPt2objDPt(&geoPt, &objPt);
				/* ���ӽڵ㿴��������,��Χ��뾶�������������ĵ㵽�ĸ�����ľ��� */
	            ret = InFrustum(view_near, view_far, objPt.x, objPt.y, objPt.z, minRadius * 1.414, pFrustum, &dis);
				/* ֻҪ��һ�����ӽڵ�İ�Χ�����Ӿ�����ӷ�Χ��,��ֱ�ӷ���0 */
			    if(TRUE == ret)
				    return 0;
	        }/* end for(j = 0; j < 4; j++) */
	    }/* end for(i = 0; i < 4; i++) */	
    }

	/* �������ӽڵ�İ�Χ��������Ӿ�����ӷ�Χ��,�򷵻�-1 */
	return -1;
}


/*
���ܣ����ӵ���(mapNodeRenderOverlook)��,�ж�ָ���Ľڵ��Ƿ���������Ӿ�����ӷ�Χ��(���һ���ж����ӽڵ��Ƿ���)
      ���ڸ��ڵ����Ӿ�����,��16����ڵ�������Ӿ����ڵ����
���룺
    view_near  �Ӿ�����ý���
    view_far   �Ӿ���Զ�ý���
	pFrustum   �Ӿ���6����ķ���
    pNode      ��Ƭ�ڵ�ָ��
�����    
    0  ��
    -1 ����
*/
f_int32_t isNodeInFrustumOverlook(f_float32_t view_near, f_float32_t view_far, LPFRUSTUM pFrustum, sQTMAPNODE * pNode)
{
	f_int32_t ret = FALSE;
	f_float64_t dis = 0.0;
	sNODERANGE nodeRange;	

	if (pNode == NULL)
		return -1;	
	
	/* ��ȡ�����ӽ��µ���Ƭ�ڵ�İ�Χ�� */
	nodeRange = pNode->nodeRangeOverlook;
	
	/* ͨ���Ӿ����ж���Ƭ�����ӽ��µİ�Χ���Ƿ����Ӿ�����ӷ�Χ�� FALSE(0)-���� TRUE(1)-�� */
	/* �������,������Ƭ�����Ӿ�����ӷ�Χ��,ֱ�ӷ���-1 */
	/* �����,���һ���ж��Ƿ������ӽڵ����Ӿ�����ӷ�Χ�� */
	ret = InFrustum(view_near, view_far, nodeRange.x, nodeRange.y, nodeRange.z, nodeRange.radius, pFrustum, &dis);
	if(FALSE == ret)
		return -1;	
#if 0
	return 0;
#else	
    /* �ж����ӽڵ��Ƿ����Ӿ�����ӷ�Χ�� */
    {
        f_int32_t i = 0, j = 0;
	    f_float64_t hstep = 0.0, vstep = 0.0, minRadius = 0.0;
	    Geo_Pt_D geoPt; 
	    Obj_Pt_D objPt;
	    
		/* һ���ڵ���16�����ӽڵ�,��Χ���������С1/4 */
	    minRadius = nodeRange.radius * 0.25;
	    hstep = pNode->hscale * 0.25;
	    vstep = pNode->vscale * 0.25;
	    for(i = 0; i < 4; i++)
	    {
	        for(j = 0; j < 4; j++)	
	        {
	        	ret = FALSE;
	            geoPt.lon = pNode->hstart + hstep * i + hstep * 0.5;
	            geoPt.lat = pNode->vstart + vstep * j + vstep * 0.5;
				//geoPt.height = pNode->height_ct;
				/* ���ӽڵ�İ�Χ�򲻿��Ǹ߶�,Ĭ�ϸ�0 */
				geo2obj(geoPt.lon, geoPt.lat, 0.0, &objPt.x, &objPt.y, &objPt.z);
				/* ���ӽڵ㿴��������,��Χ��뾶�������������ĵ㵽�ĸ�����ľ��� */
	            ret = InFrustum(view_near, view_far, objPt.x, objPt.y, objPt.z, minRadius * 1.414, pFrustum, &dis);
				/* ֻҪ��һ�����ӽڵ�İ�Χ�����Ӿ�����ӷ�Χ��,��ֱ�ӷ���0 */
			    if(TRUE == ret)
				    return 0;
	        }/* end for(j = 0; j < 4; j++) */
	    }/* end for(i = 0; i < 4; i++) */	
    }

	/* �������ӽڵ�İ�Χ��������Ӿ�����ӷ�Χ��,�򷵻�-1 */
	return -1;
#endif
}

/*
���ܣ��ж�ָ���Ľڵ��Ƿ��ڵ�ͼ���������������У���ppTree != NULL����õ���Ӧ������������ֻ�õ���������־
���룺
	pHandle     ��ͼ���������
	level       �ڵ�㼶
	xidx, yidx  ���x��y�����������

���������
	ppTree      ��������ͷ���ָ���ָ��

�����
    ��

����ֵ��
    0 ������
    1 ����
*/
f_int32_t isNodeExistInMapSubreeList(sMAPHANDLE * pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, sQTMAPSUBTREE ** ppTree)
{
    stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPSUBTREE *pSubTree = NULL;

	if (pHandle == NULL)
		return 0;	
	/**��ȡȫ�����������ͷ�ڵ�**/
	pstListHead = &(pHandle->sub_tree.stListHead);
	/**�������ȫ�����������е���������**/
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			/**��ȡ�����Ľڵ�ָ��**/
			pSubTree = ASDE_LIST_ENTRY(pstTmpList, sQTMAPSUBTREE, stListHead);

			/**�жϽڵ�Ĳ㼶(������������ʼ�㼶)�Ƿ��ڵ�ǰ�������ǵĲ㼶��Χ��**/
			#ifdef USE_FAULT_Tile	// Ϊ֧�ֶϲ�,����Ҫ�㼶��������������
				if ( (level >= pSubTree->level) )
			#else
				if ( (level >= pSubTree->level) && (level < pSubTree->level + pSubTree->levels))
			#endif
			{
				/**�жϵ�ǰ�ڵ�������������(�жϽڵ��xidx��yidx(������������ʼ�ڵ��xidx��yidx)�Ƿ����뵱ǰ������ʼ�ڵ��xidx��yidx���)**/
				if ( ((xidx >> (level - pSubTree->level)) == pSubTree->xidx) &&
					((yidx >> (level - pSubTree->level)) == pSubTree->yidx) )
				{
					// Ϊ֧�ֶϲ�: �����ж��ܷ����,Ĭ�϶��ܼ���(������ܼ���,���ڼ�����������в�ֵ)
					#ifdef	USE_FAULT_Tile
					{
						if (ppTree != NULL)
							*ppTree = pSubTree;
						return 1;
					}
					#else
					{
						/**���������жϸýڵ��ڸ������У�Ϊ��ȷ����ȷ����ʵ��ȡ����������ʼ�ڵ�㼶��xidx��yidx�����ж�һ��**/
						if (pSubTree->imgisexist(level, xidx, yidx, pSubTree->loadterrainparam) == 0)
						{
							/**�ýڵ��ڸ������У����ظ�������ָ��**/
							if (ppTree != NULL)
								*ppTree = pSubTree;
							return 1;
						}
					}
					#endif

				}
			}
		}
	}	
	
	return 0;
}

static f_int32_t isPointInQuad(f_float64_t x, f_float64_t y, f_float64_t quad[4][2])
{
	f_int32_t i, pointsrighttox;
	f_float64_t xx;

	/*���(x, y)��ˮƽ�����ı��εĸ����ߵĽ����ˮƽλ���д���x��С��x�ĸ���*/
	pointsrighttox = 0;
	for (i = 0;i<4;i++)
	{
		f_int32_t i1;
#define ZEROERR	(0.0000001f)
		i1 = (i + 1) & 3;
#define px x
#define py y
#define x1 quad[i][0]
#define y1 quad[i][1]
#define x2 quad[i1][0]
#define y2 quad[i1][1]
		if ( (y >= max(y1, y2)) || (y <= min(y1, y2)) )
			continue; /*�޽���*/
		if (fabs(y2 - y1) < ZEROERR)
			continue;

		xx = (x2 - x1) / (y2 - y1) * (y - y1) + x1;
#undef px
#undef py
#undef x1
#undef y1
#undef x2
#undef y2
		if (xx >= x)
			pointsrighttox++;
	}
	return pointsrighttox & 1;
}

/* 
   �ж��������λ�õľ�γ���Ƿ��ڽڵ����ڵķ�Χ��
   ����ֵ����(TRUE)������(FALSE)
*/
BOOL isCaminNode(sGLRENDERSCENE * pHandle, sQTMAPNODE * pNode)
{
    f_float64_t x, y, quad[4][2];
    
    if(NULL == pHandle || NULL == pNode)
        return(FALSE);
    
    x = pHandle->camParam.m_geoptEye.lon;
    y = pHandle->camParam.m_geoptEye.lat;
    quad[0][0] = pNode->hstart;
    quad[0][1] = pNode->vstart;
    quad[1][0] = pNode->hstart;
    quad[1][1] = pNode->vstart + pNode->vscale;
    quad[2][0] = pNode->hstart + pNode->hscale;
    quad[2][1] = pNode->vstart + pNode->vscale;
    quad[3][0] = pNode->hstart + pNode->hscale;
    quad[3][1] = pNode->vstart;
    
    if(isPointInQuad(x, y, quad))
    {
		return(TRUE);
	}   
	else
	{
		//�ж��Ƿ��ڱ���
		if (
			(fabs(quad[0][0] - x) < FLT_EPSILON)
			||(fabs(quad[2][0] - x) < FLT_EPSILON)
			||(fabs(quad[0][1] - y) < FLT_EPSILON)
			||(fabs(quad[2][1] - y) < FLT_EPSILON)
			)
		{
			return(TRUE);
		}


	}
	
	return(FALSE);
}

static f_float64_t cacDistance(PT_3D pt1, PT_3D pt2)
{
    return((pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y) + (pt1.z - pt2.z) * (pt1.z - pt2.z));	
}

static f_float64_t cacAngle(PT_3D pt1, PT_3D pt2)
{
	vector3DNormalize(&pt1);
	vector3DNormalize(&pt2);
    return(getVa2DVbAngle(&pt1, &pt2) * RA2DE);
}

/*
���ܣ��ж���Ƭ�ڵ��Ƿ����Ӿ��忴���ĵ�����
���룺
	pHandle       ��ͼ������ָ��
	pNode         ��Ƭ�ڵ�ָ��
�����
    ��
����ֵ��
    TRUE(1):�ڱ���
	FALSE(0):���ڱ���
*/
BOOL isNodeAtEarthBack(sGLRENDERSCENE * pHandle, sQTMAPNODE * pNode)
{
	/* �ӵ㵽�������ĵľ����ƽ��,�ӵ㵽�ڵ����ĵľ����ƽ�� */ 
    f_float64_t dis_eye2earth = 0.0, dis_eye2nodect = 0.0; 
    /* �������ĵ��ӵ������ �� �������ĵ��ڵ����ĵ����� ֮��ļн� */
    f_float64_t  angle = 0.0;
	/* �������� */
    PT_3D earth_center = {0.0, 0.0, 0.0}; 
	BOOL ret = FALSE;
	
    if(NULL == pHandle || NULL == pNode)
        return(FALSE);
    
	/* �����ӵ㵽�������ĵľ����ƽ�� */ 
    dis_eye2earth  = cacDistance(pHandle->camParam.m_ptEye, earth_center);
	/* �ӵ㵽�ڵ����ĵľ����ƽ�� */
    dis_eye2nodect = cacDistance(pHandle->camParam.m_ptEye, pNode->obj_cent);  
	/* ����������ĵ��ӵ������ �� �������ĵ��ڵ����ĵ����� ֮��ļн� */
    angle = cacAngle(pHandle->camParam.m_ptEye, pNode->obj_cent);


	if(1)
	{
		/* ����:Ϊɶ�����ж�?Ϊɶ��120.0? */
		if((dis_eye2nodect > (dis_eye2earth - EARTH_RADIUS * EARTH_RADIUS)) && angle > 120.0)
		{
			ret = TRUE;
		}
	}
	else
	{
		f_float64_t angle_1 = 0.0;
		f_float64_t angle_2 = pNode->vscale;

		angle_1 = acos(EARTH_RADIUS /sqrt(dis_eye2earth) ) * RA2DE;

		angle_2 *= 0.8;

		if(angle > (angle_1 + angle_2))
		{

			ret = TRUE;
		}

	}	 

	return ret;

}

/*
���ܣ��ж��Ƿ���Ҫ��һ����������ڵ�ĺ��ӽڵ�
���룺
	pNode             ��Ƭ�ڵ�ָ��
	pPlanePosNode     �ɻ����ڵ���Ƭ�ڵ�ָ��
	cameraPos         ���λ�õľ�γ��
	camObjPos         ���λ�õ���������xyz
	view_type         �ӽ�����
�����
    ��
����ֵ��
    0        ��Ҫ����
	����ֵ   ����Ҫ����
*/
f_int32_t isNeedCreateChildren(sQTMAPNODE * pNode, sQTMAPNODE * pPlanePosNode, Geo_Pt_D cameraPos, Obj_Pt_D camObjPos, int view_type)
{
	f_float64_t dis = 0.0, ratio = 1.0, enlarge = 1.0;
	sNODERANGE nodeRange;	
	f_int32_t ret = 0;

	if (pNode == NULL)
		return -1;
		
	/*��ȡ��Ƭ�İ�Χ��*/
	nodeRange = pNode->nodeRange;
				
	/*��Ƭ�����ж����Ӿ����ڣ��ж��Ƿ�Ҫ������ȥ�ж�����Ƭ*/
#if 0		
    dis = caculateGreatCircleDis(pNode->hstart + 0.5 * pNode->hscale, 
		                         pNode->vstart + 0.5 * pNode->vscale,
						         cameraPos.lon, 
						         cameraPos.lat);
#else		
    /* zqx 2017-2-27 15:52 ����ĳ��ӵ㵽��Ƭ���ĵ�ľ���, ����Խ��������, ���õ���ƬԽ��, ���ᵽ180�����Ƭ*/						         
    dis = sqrt((pNode->obj_cent.x - camObjPos.x) * (pNode->obj_cent.x - camObjPos.x) + 
               (pNode->obj_cent.y - camObjPos.y) * (pNode->obj_cent.y - camObjPos.y) + 
               (pNode->obj_cent.z - camObjPos.z) * (pNode->obj_cent.z - camObjPos.z));
#endif
	/* zqx 2017-2-28 11:17 Ϊ�˿����������õ�ͼ����, ��ͨ��γ�߷����뾭�߷���ĳ��ȱ��������ж�ϸ������Ƭ�ľ���*/
	//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//�����һ�Ⱦ��ȵľ���Ϊ111319.5m
	/* ����������ӵ�����γ�ȵ�1�Ⱦ��ȵľ��� �� �����1�Ⱦ��ȵľ��� �ı�ֵ,��ֵ��ȻС��1,������γ�ȵ������Ȼ��С,�����ӽ���0 */
	ratio = caculateGreatCircleDis(cameraPos.lon, cameraPos.lat, cameraPos.lon + 1.0, cameraPos.lat) / 111319.5;

	/* ����5��,����֤��ֵ������1 */
	ratio *= 5.0;
	if(ratio > 1.0)
		ratio = 1.0;

    /* 2017-3-6 10:04 �ڻ�ȡ�ɻ�λ�õĽڵ�,�ж��Ƿ���Ҫ�ӽڵ�ʱ,�����pPlanePosNode��NULL;
       �ڻ����ж��Ƿ���Ҫ�ӽڵ�ʱ,�������֮ǰ�õ��ķɻ�λ�����ڵĽڵ� */	
    if(pPlanePosNode == NULL)
        enlarge = 1.414;
    else
    {
        if((pPlanePosNode->level - pNode->level) > 2)
            enlarge = 1.414;
        else if((pPlanePosNode->level - pNode->level) > 1)
            enlarge = 2.0;
        else 
            enlarge = 2.5;
    }
	//if(dis > nodeRange.radius * 1.414 * ratio) //zqx 2017-3-6 10:38

// 	if (view_type == eVM_OVERLOOK_VIEW))
// 	{
// 		enlarge *= 3.0;
// 	}

	/* ��� �ӵ㵽��Ƭ���ĵ�ľ��� ���� ��Ƭ�뾶*ratio(0-1֮��,γ��Խ��Խ�ӽ���0)*enlarge(Ĭ����1.414),��ʾ����Ҫ�������ӽڵ�*/
	/* ���С�ڻ����,��ʾ�贴�����ӽڵ� */
	/* �÷�����֤���ӵ����ԽԶ����Ƭ��Ȼ�㼶ֵ��С,���ӵ����Խ������Ƭ��Ȼ�㼶ֵ����(������ϸ) */
	if(dis > nodeRange.radius * enlarge * ratio)
	{
//		printf("out %d, dis=%f\n", pNode->level, dis);
		return -2;
	}else
	{
		return 0;
	}
}
