#include <math.h>
#include <string.h>
#include "../utility/tools.h"
#include "../projection/project.h"
#include "../projection/coord.h"
#include "mapNodeJudge.h"
#include "mapNodeGen.h"


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

// Ritter方法计算包围球半径 pNode_center = ComputeBoundingSphereRitter(pNode, 4, &radiu);
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

	// 确定图幅四至
	for(i = 0; i < vertex_num; i++)
	{
		if(vertices[i].x > vertices[maxX].x) maxX = i;
		if(vertices[i].y > vertices[maxY].y) maxY= i;
		if(vertices[i].z > vertices[maxZ].z) maxZ= i;

		if(vertices[i].x < vertices[minX].x) minX = i;
		if(vertices[i].y < vertices[minY].y) minY= i;
		if(vertices[i].z < vertices[minZ].z) minZ= i;
	}

	// 计算最长对角线,并作为初始半径长度
	{
		// 以对角线长度作为初始x长度
		{
			PT_3F sub1 = {0}, sub2 = {0}, d = {0};
			sub1.x = vertices[maxX].x;
			sub1.y = vertices[maxX].y;
			sub1.z = vertices[maxX].z;
			sub2.x = vertices[minX].x;
			sub2.y = vertices[minX].y;
			sub2.z = vertices[minX].z;
			vector3FSub(&d, &sub1, &sub2);
			x = vector3FDotProduct(&d, &d);	// 基于点乘计算长度
		}

		// 以对角线长度作为初始y长度
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

		// 以对角线长度作为初始z长度
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
		
		// 找到球 x|y|z 上最长距离
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

		// 计算最长对角线上的中心点compute the center point
		center.x = 0.5 * (vertices[maxIndex].x + vertices[minIndex].x);
		center.y = 0.5 * (vertices[maxIndex].y + vertices[minIndex].y);
		center.z = 0.5 * (vertices[maxIndex].z + vertices[minIndex].z);
		radius = 0.5 * sqrt(dia);
	}

	// 计算中心点与四至点最远距离,作为作为包围盒半径
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


//通过OSG 的方法计算瓦片的中心点及包围球半径lpf add 2017年11月27日14:14:25
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

	//计算包围球和包围球半径
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
功能：获取俯视视角下瓦片节点的包围球及4个顶点(高度为0，依次为左下角、右下角、右上角、左上角) + 4个顶点(高度为-1000，依次为左下角、右下角、右上角、左上角)
      
输入：
    pNode      瓦片节点指针

输出：    
    sNODERANGE  包围球结构体(包括中心点坐标、包围球半径、最小包围球半径)
*/
sNODERANGE getNodeRangeOSG_Overlook(sQTMAPNODE* pNode)
{
	sNODERANGE nodeRange = {0};
	PT_3F pNode_center = {0};
	double x = 0.0, y =0.0, z=0.0;
	int i = 0;
	PT_3F pNode_vertex_overlook[4] = {0};
	double hei_offset = -1000.0;

	/* 如果瓦片节点指针为空,则返回的包围球结构体中的参数均为0 */
	if (pNode == NULL)
		return nodeRange;

	/* 计算8个瓦片顶点的球坐标,其中4个顶点(高度为0，依次为左下角、右下角、右上角、左上角) + 4个顶点(高度为-1000，依次为左下角、右下角、右上角、左上角) */
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

	/* 复制前4个顶点的坐标值到数组pNode_vertex_overlook中 */
	memcpy(&pNode_vertex_overlook[0].x, 
		pNode->fvertex_overlook, 
		4 * 3 * sizeof(f_float32_t));

	/* 计算瓦片包围球的中心点坐标及包围球半径,仅仅使用瓦片4个角点的坐标 */
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

//返回值
// 0:准备好
// -1:子节点不存在
// -2:子节点数据未准备好
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
	
	/*前面循环里都没返回，那么所有子瓦片数据都准备好*/
	return 0;
}

/*
功能：正常调度(mapNodeRender)下,判断指定的节点是否在相机的视景体可视范围内(会进一步判断孙子节点是否在)
      存在父节点在视景体内,而16个孙节点均不在视景体内的情况
输入：
    view_near  视景体近裁截面
    view_far   视景体远裁截面
	pFrustum   视景体6个面的方程
    pNode      瓦片节点指针
输出：    
    0  在
    -1 不在
*/
f_int32_t isNodeInFrustum(f_float32_t view_near, f_float32_t view_far, LPFRUSTUM pFrustum, sQTMAPNODE * pNode)
{
	f_int32_t ret = FALSE;
	f_float64_t dis = 0.0;
	sNODERANGE nodeRange;	

	if (pNode == NULL)
		return -1;	
	
	/* 获取瓦片节点的包围球 */
	nodeRange = pNode->nodeRange;
	
	/* 通过视景体判断瓦片长边包围球(半径大)是否在视景体可视范围内 FALSE(0)-不在 TRUE(1)-在 */
	/* 如果不在,表明瓦片不在视景体可视范围内,返回-1 */
	ret = InFrustum(view_near, view_far, nodeRange.x, nodeRange.y, nodeRange.z, nodeRange.radius, pFrustum, &dis);
	if(FALSE == ret)
		return -1;	
	
	/* 如果在,进一步判断瓦片短边包围球(半径小)是否在视景体可视范围内 */
	ret = InFrustum(view_near, view_far, nodeRange.x, nodeRange.y, nodeRange.z, nodeRange.radiusMin, pFrustum, &dis);
	/* 如果在,表明瓦片在视景体可视范围内,返回0;如果不在,进一步判断该瓦片的孙子瓦片是否在视景体可视范围内 */
	if(TRUE == ret)
		return 0;
		
    /* 判断孙子节点是否在视景体可视范围内 */
    {
        f_int32_t i = 0, j = 0;
	    f_float64_t hstep = 0.0, vstep = 0.0, minRadius = 0.0;
	    Geo_Pt_D geoPt; 
	    Obj_Pt_D objPt;
	    
		/* 一个节点有16个孙子节点,包围球参数均缩小1/4 */
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
				/* 孙子节点看成正方形,包围球半径采用正方形中心点到四个顶点的距离 */
	            ret = InFrustum(view_near, view_far, objPt.x, objPt.y, objPt.z, minRadius * 1.414, pFrustum, &dis);
				/* 只要有一个孙子节点的包围球在视景体可视范围内,则直接返回0 */
			    if(TRUE == ret)
				    return 0;
	        }/* end for(j = 0; j < 4; j++) */
	    }/* end for(i = 0; i < 4; i++) */	
    }

	/* 所有孙子节点的包围球均不在视景体可视范围内,则返回-1 */
	return -1;
}


/*
功能：俯视调度(mapNodeRenderOverlook)下,判断指定的节点是否在相机的视景体可视范围内(会进一步判断孙子节点是否在)
      存在父节点在视景体内,而16个孙节点均不在视景体内的情况
输入：
    view_near  视景体近裁截面
    view_far   视景体远裁截面
	pFrustum   视景体6个面的方程
    pNode      瓦片节点指针
输出：    
    0  在
    -1 不在
*/
f_int32_t isNodeInFrustumOverlook(f_float32_t view_near, f_float32_t view_far, LPFRUSTUM pFrustum, sQTMAPNODE * pNode)
{
	f_int32_t ret = FALSE;
	f_float64_t dis = 0.0;
	sNODERANGE nodeRange;	

	if (pNode == NULL)
		return -1;	
	
	/* 获取俯视视角下的瓦片节点的包围球 */
	nodeRange = pNode->nodeRangeOverlook;
	
	/* 通过视景体判断瓦片俯视视角下的包围球是否在视景体可视范围内 FALSE(0)-不在 TRUE(1)-在 */
	/* 如果不在,表明瓦片不在视景体可视范围内,直接返回-1 */
	/* 如果在,需进一步判断是否有孙子节点在视景体可视范围内 */
	ret = InFrustum(view_near, view_far, nodeRange.x, nodeRange.y, nodeRange.z, nodeRange.radius, pFrustum, &dis);
	if(FALSE == ret)
		return -1;	
#if 0
	return 0;
#else	
    /* 判断孙子节点是否在视景体可视范围内 */
    {
        f_int32_t i = 0, j = 0;
	    f_float64_t hstep = 0.0, vstep = 0.0, minRadius = 0.0;
	    Geo_Pt_D geoPt; 
	    Obj_Pt_D objPt;
	    
		/* 一个节点有16个孙子节点,包围球参数均缩小1/4 */
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
				/* 俯视节点的包围球不考虑高度,默认给0 */
				geo2obj(geoPt.lon, geoPt.lat, 0.0, &objPt.x, &objPt.y, &objPt.z);
				/* 孙子节点看成正方形,包围球半径采用正方形中心点到四个顶点的距离 */
	            ret = InFrustum(view_near, view_far, objPt.x, objPt.y, objPt.z, minRadius * 1.414, pFrustum, &dis);
				/* 只要有一个孙子节点的包围球在视景体可视范围内,则直接返回0 */
			    if(TRUE == ret)
				    return 0;
	        }/* end for(j = 0; j < 4; j++) */
	    }/* end for(i = 0; i < 4; i++) */	
    }

	/* 所有孙子节点的包围球均不在视景体可视范围内,则返回-1 */
	return -1;
#endif
}

/*
功能：判断指定的节点是否在地图数据子树的链表中，若ppTree != NULL，则得到对应的子树，否则只得到存在与否标志
输入：
	pHandle     地图场景树句柄
	level       节点层级
	xidx, yidx  结点x、y方向的索引号

输入输出：
	ppTree      子树链表头结点指针的指针

输出：
    无

返回值：
    0 不存在
    1 存在
*/
f_int32_t isNodeExistInMapSubreeList(sMAPHANDLE * pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, sQTMAPSUBTREE ** ppTree)
{
    stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPSUBTREE *pSubTree = NULL;

	if (pHandle == NULL)
		return 0;	
	/**获取全局子树链表的头节点**/
	pstListHead = &(pHandle->sub_tree.stListHead);
	/**正向遍历全局子树链表中的所有子树**/
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			/**获取子树的节点指针**/
			pSubTree = ASDE_LIST_ENTRY(pstTmpList, sQTMAPSUBTREE, stListHead);

			/**判断节点的层级(或则子树的起始层级)是否在当前子树覆盖的层级范围内**/
			#ifdef USE_FAULT_Tile	// 为支持断层,不需要层级上连接在子树中
				if ( (level >= pSubTree->level) )
			#else
				if ( (level >= pSubTree->level) && (level < pSubTree->level + pSubTree->levels))
			#endif
			{
				/**判断当前节点数否属于子树(判断节点的xidx与yidx(或则子树的起始节点的xidx与yidx)是否在与当前子树起始节点的xidx与yidx相等)**/
				if ( ((xidx >> (level - pSubTree->level)) == pSubTree->xidx) &&
					((yidx >> (level - pSubTree->level)) == pSubTree->yidx) )
				{
					// 为支持断层: 不再判断能否加载,默认都能加载(如果不能加载,则在加载任务里进行插值)
					#ifdef	USE_FAULT_Tile
					{
						if (ppTree != NULL)
							*ppTree = pSubTree;
						return 1;
					}
					#else
					{
						/**基本可以判断该节点在该子树中，为了确保正确，真实读取该子树的起始节点层级、xidx与yidx，再判断一次**/
						if (pSubTree->imgisexist(level, xidx, yidx, pSubTree->loadterrainparam) == 0)
						{
							/**该节点在该子树中，返回该子树的指针**/
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

	/*求过(x, y)的水平线与四边形的各条边的交点的水平位置中大于x和小于x的个数*/
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
			continue; /*无交点*/
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
   判断相机所在位置的经纬度是否在节点所在的范围内
   返回值：在(TRUE)；不在(FALSE)
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
		//判断是否在边上
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
功能：判断瓦片节点是否在视景体看到的地球背面
输入：
	pHandle       地图场景树指针
	pNode         瓦片节点指针
输出：
    无
返回值：
    TRUE(1):在背面
	FALSE(0):不在背面
*/
BOOL isNodeAtEarthBack(sGLRENDERSCENE * pHandle, sQTMAPNODE * pNode)
{
	/* 视点到地球球心的距离的平方,视点到节点中心的距离的平方 */ 
    f_float64_t dis_eye2earth = 0.0, dis_eye2nodect = 0.0; 
    /* 地球球心到视点的向量 与 地球球心到节点中心的向量 之间的夹角 */
    f_float64_t  angle = 0.0;
	/* 地球球心 */
    PT_3D earth_center = {0.0, 0.0, 0.0}; 
	BOOL ret = FALSE;
	
    if(NULL == pHandle || NULL == pNode)
        return(FALSE);
    
	/* 计算视点到地球球心的距离的平方 */ 
    dis_eye2earth  = cacDistance(pHandle->camParam.m_ptEye, earth_center);
	/* 视点到节点中心的距离的平方 */
    dis_eye2nodect = cacDistance(pHandle->camParam.m_ptEye, pNode->obj_cent);  
	/* 计算地球球心到视点的向量 与 地球球心到节点中心的向量 之间的夹角 */
    angle = cacAngle(pHandle->camParam.m_ptEye, pNode->obj_cent);


	if(1)
	{
		/* 问题:为啥这样判断?为啥是120.0? */
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
功能：判断是否需要进一步创建输入节点的孩子节点
输入：
	pNode             瓦片节点指针
	pPlanePosNode     飞机所在的瓦片节点指针
	cameraPos         相机位置的经纬高
	camObjPos         相机位置的物体坐标xyz
	view_type         视角类型
输出：
    无
返回值：
    0        需要创建
	其他值   不需要创建
*/
f_int32_t isNeedCreateChildren(sQTMAPNODE * pNode, sQTMAPNODE * pPlanePosNode, Geo_Pt_D cameraPos, Obj_Pt_D camObjPos, int view_type)
{
	f_float64_t dis = 0.0, ratio = 1.0, enlarge = 1.0;
	sNODERANGE nodeRange;	
	f_int32_t ret = 0;

	if (pNode == NULL)
		return -1;
		
	/*获取瓦片的包围球*/
	nodeRange = pNode->nodeRange;
				
	/*瓦片初步判断在视景体内，判断是否要往下走去判断子瓦片*/
#if 0		
    dis = caculateGreatCircleDis(pNode->hstart + 0.5 * pNode->hscale, 
		                         pNode->vstart + 0.5 * pNode->vscale,
						         cameraPos.lon, 
						         cameraPos.lat);
#else		
    /* zqx 2017-2-27 15:52 距离改成视点到瓦片中心点的距离, 否则越靠近两极, 调用的瓦片越多, 最多会到180多个瓦片*/						         
    dis = sqrt((pNode->obj_cent.x - camObjPos.x) * (pNode->obj_cent.x - camObjPos.x) + 
               (pNode->obj_cent.y - camObjPos.y) * (pNode->obj_cent.y - camObjPos.y) + 
               (pNode->obj_cent.z - camObjPos.z) * (pNode->obj_cent.z - camObjPos.z));
#endif
	/* zqx 2017-2-28 11:17 为了控制两极调用的图块数, 需通过纬线方向与经线方向的长度比来调整判断细分子瓦片的距离*/
	//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//赤道上一度经度的距离为111319.5m
	/* 计算在相机视点所在纬度的1度经度的距离 与 赤道上1度经度的距离 的比值,其值必然小于1,且随着纬度的增大必然减小,两极接近于0 */
	ratio = caculateGreatCircleDis(cameraPos.lon, cameraPos.lat, cameraPos.lon + 1.0, cameraPos.lat) / 111319.5;

	/* 扩大5倍,但保证其值不超过1 */
	ratio *= 5.0;
	if(ratio > 1.0)
		ratio = 1.0;

    /* 2017-3-6 10:04 在获取飞机位置的节点,判断是否需要子节点时,输入的pPlanePosNode是NULL;
       在绘制判断是否需要子节点时,输入的是之前得到的飞机位置所在的节点 */	
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

	/* 如果 视点到瓦片中心点的距离 大于 瓦片半径*ratio(0-1之间,纬度越高越接近于0)*enlarge(默认是1.414),表示不需要创建孩子节点*/
	/* 如果小于或等于,表示需创建孩子节点 */
	/* 该方法保证离视点距离越远的瓦片必然层级值更小,离视点距离越近的瓦片必然层级值更大(即更精细) */
	if(dis > nodeRange.radius * enlarge * ratio)
	{
//		printf("out %d, dis=%f\n", pNode->level, dis);
		return -2;
	}else
	{
		return 0;
	}
}
