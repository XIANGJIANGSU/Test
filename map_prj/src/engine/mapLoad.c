#include "../define/mbaseType.h"
#include "osAdapter.h"
#include "mapLoad.h"
#include "mapNodeGen.h"
#include "mapNodeDel.h"
#include "tqtfile.h"
#include <math.h>
#ifdef WIN32
#include "../../../SE_api_prj/src/SqliteOne.h"
#endif
#include "../projection/coord.h"
#include "qtnLoad.h"
#include "mapNodeJudge.h"
#include "../../../include_vecMap/mapCommon.h"
#include "shadeMap.h"
#include "memoryPool.h"

/*记录影像等数据加载任务的心跳*/
static unsigned long imgLoadTaskHeartBeat = 0;

int SetClrHilBlend(sQTMAPNODE  *pNode, f_int32_t width, f_int32_t height)
{
	int i = 0;
	int sum = width * height;
	f_uint8_t	clr;
	f_uint32_t	alpha;
	f_uint8_t	*src = pNode->clrdata;
	f_uint8_t	*dst = pNode->hildata;

#define INTERSEC(_s, _d)	do { \
	clr = (f_uint8_t)(((f_uint32_t)_s * (255 - alpha) + (f_uint32_t)_d * alpha) / 255); \
	if (clr > 255) clr=255; \
	_s = clr; \
}while(0)

	for(i=0; i<sum; i++)
	{
		alpha = dst[3];
		INTERSEC(src[0], dst[0]);
		INTERSEC(src[1], dst[1]);
		INTERSEC(src[2], dst[2]);
		src[3] = 255;
		dst += 4;
		src += 4;
	}
#undef INTERSEC

	return 1;
}

#ifdef USE_VQT

static f_uint8_t dst_vqt[256*256*4];
int AddVqtAlpha(sQTMAPNODE  *pNode, f_int32_t width, f_int32_t height, f_uint8_t dst_color[3])
{	
	f_uint8_t	*dst = &dst_vqt[0];
	f_uint8_t	*src = pNode->vqtdata;

	int i = 0;
	int sum = width * height;
	f_uint8_t	alpha = 255;

	memset(dst, 0, sizeof(dst));

	for(i=0; i<sum; i++)
	{
// 		if ((src[0] != 0)||(src[1] != 0)||(src[2] != 0))
// 		{
// //			printf("%d-%d-%d-\n",src[0],src[1],src[2]);
// 		}

		if((src[0] == 1)&&(src[1] == 1)&&(src[2] == 1))
		{
//			dst[0] = dst[1] = dst[2] = dst[3] = 1;		
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
			dst[3] = alpha;//0;	
		}
		else
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];

//			dst[0] = dst_color[0];
//			dst[1] = dst_color[1];
//			dst[2] = dst_color[2];
			dst[3] = alpha;	
		}

//  		printf("src: %d-%d-%d-\n",src[0],src[1],src[2]);
//  		printf("dst: %d-%d-%d-%d\n",dst[0],dst[1],dst[2],dst[3]);
		

		dst += 4;
		src += 3;
	}


	DeleteAlterableMemory(pNode->vqtdata);
	pNode->vqtdata = NewAlterableMemory(width * height * 4);
	memcpy(pNode->vqtdata, dst_vqt, sizeof(dst_vqt));
	
	return 1;
}

#endif

#ifdef WIN32
void WINAPI qtmapDataLoadRoute(sMAPHANDLE *pHandle)
#else
void qtmapDataLoadRoute(sMAPHANDLE *pHandle)
#endif
{
	f_int16_t terrain[VTXCOUNT1 * VTXCOUNT1];	// 一个瓦块对应的33*33个地形高程节点
	f_int32_t ret, i;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
    BOOL is_dnst_waitforload = FALSE;
	static int zeroNodeLoaded = 0;		//已加载的0级瓦片个数
    	
	while (1)
	{
       if(SEM_TAKE_OK == takeSem(pHandle->endrequire, 0))   /* 终止程序 */
           break;	

		// 1.首先查找有没有待加载的0级瓦片
		if(zeroNodeLoaded < 8)
		{
			takeSem(pHandle->rendscenelock, FOREVER_WAIT);
			/*查找LRU链表里有没有待加载的节点*/
			pstListHead = &(pHandle->nodehead.stListHead);
			LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
			{
				if( NULL != pstTmpList )
				{
					pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
					if((pNode->level == 0)&&(DNST_WAITFORLOAD == qtmapnodeGetStatus(pNode, __LINE__)))
					{
						is_dnst_waitforload = TRUE;
						zeroNodeLoaded ++;
					    break;
					}			
				}
			}		
			giveSem(pHandle->rendscenelock);
		}
		else		
		{
			// 2.然后再加载其他瓦片

			takeSem(pHandle->rendscenelock, FOREVER_WAIT);
			/*查找LRU链表里有没有待加载的节点*/
			pstListHead = &(pHandle->nodehead.stListHead);
			LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
			{
				if( NULL != pstTmpList )
				{
					pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
					if(DNST_WAITFORLOAD == qtmapnodeGetStatus(pNode, __LINE__))
					{
						is_dnst_waitforload = TRUE;
					    break;
					}			
				}
			}		
			giveSem(pHandle->rendscenelock);
		}

		// 3.加载数据			
		if(is_dnst_waitforload)
		{
			if (!ISMAPDATANODE(pNode))
			{
				DEBUG_PRINT("Load Node data with error node.");
				continue;
			}

			qtmapnodeSetStatus(pNode, DNST_LOADING, __LINE__);

            /* 正常模式才调用地形数据，鹰眼模式不需要地形数据 */
#if 1            
            //if(eScene_normal == pHandle->mdctrl.scene_mode)
            {
			/*调入地形数据*/
		
                ret = -1;
				
				// 载入地形数据
			    ret = pNode->pSubTree->funcloadterrain(pNode->level, pNode->xidx, pNode->yidx, 
			                                           pNode->pSubTree->loadterrainparam, (f_int16_t**)&terrain[0]);
				
				// 从当前子树上无法载入对应瓦块
				if(ret != 0)
				{
					// 断层模式, 无法加载瓦块则插值生成格网点高程
					#ifdef USE_FAULT_Tile
					{
						int i,j=0;
						f_int16_t tmp_terrain[VTXCOUNT1 * VTXCOUNT1];
						sQTMAPNODE  *parentNode = pNode->parent;
						int parentLvl = parentNode->level;
						int parentXidx = parentNode->xidx;
						int parentYidx = parentNode->yidx;
						{
							int tmp_xi=0,tmp_yi=0;
							int startParentXidx = 0, endParentXidx = 0;	// 从父节点上取高程的开始-结束位置
							int startParentYidx = 0, endParentYidx = 0;
							if( parentXidx*2 == pNode->xidx)
							{
								startParentXidx = 0;
								endParentXidx = VTXCOUNT/2;
							}
							else
							{
								startParentXidx = VTXCOUNT/2;
								endParentXidx = VTXCOUNT;
							}
							if(parentYidx*2 == pNode->yidx)
							{
								startParentYidx = 0;
								endParentYidx = VTXCOUNT/2;
							}
							else
							{
								startParentYidx = VTXCOUNT/2;
								endParentYidx = VTXCOUNT;
							}

							/* 
								将父节点(parentXidx,parentYidx)位置的高程数据，插值填充到子节点(i,j)位置上
								由于fterrain是将至下而上,而terrain数据是至上而下(即将最小纬度写在最上面),因此需要上下翻转
								1  插值到	1 2
											3 4
							*/

							{
								int j=VTXCOUNT,i=0;
								int tmp_xi=0,tmp_yi=0;
								for (tmp_yi = startParentYidx;tmp_yi<=endParentYidx;tmp_yi++)	
								{
									i=0;
									for(tmp_xi=startParentXidx;tmp_xi<=endParentXidx;tmp_xi++)	// 对于当前行
									{
										int idx_curTerPos = j* VTXCOUNT1 + i;
										int idx_parentTerPos = tmp_yi*VTXCOUNT1 + tmp_xi;
										terrain[idx_curTerPos] = parentNode->fterrain[idx_parentTerPos]; // 1->1
										//printf("<%d,%d> ",i,j);

										// 将当前点扩充出其余三个点(一插四)
										{
											// 1 -> 3
											if (tmp_yi < endParentYidx)
											{
												idx_curTerPos = (j-1)*VTXCOUNT1 + i;
												terrain[idx_curTerPos] = parentNode->fterrain[idx_parentTerPos];
												//printf("<%d,%d> ",i,j-1);
											}

											// 1 -> 2;
											if(tmp_xi < endParentXidx)
											{
												idx_curTerPos = j*VTXCOUNT1+i+1;
												terrain[idx_curTerPos] = parentNode->fterrain[idx_parentTerPos];// 1->2
												//printf("<%d,%d> ",i+1,j);
											}

											// 1 -> 4
											if(tmp_xi < endParentXidx && tmp_yi < endParentYidx)
											{
												// 新列的下一行
												idx_curTerPos = (j-1)*VTXCOUNT1+i+1;	
												terrain[idx_curTerPos] = parentNode->fterrain[idx_parentTerPos];// 1->4
												//printf("<%d,%d> ",i+1,j-1);
											}
										}

										//printf("<%d,%d> ",tmp_xi,tmp_yi);
										i=i+2;
									}
									j=j-2;
									//printf(" \n ");
								}
							}
						}
					}
					#else
					{
						// 非断层模式下,如果无法加载则置空
						memset(terrain, 0, VTXCOUNT1 * VTXCOUNT1 * sizeof(f_int16_t));
					}
					#endif
				}
		    }
#endif

			/*调入影像纹理数据*/
			ret = pNode->pSubTree->imgloader(pNode->level, pNode->xidx, pNode->yidx, 
				    pNode->pSubTree->imgparam, &pNode->imgdata);

			// 影像纹理调用失败
			#ifdef USE_FAULT_Tile
				if(ret != 0)
				{
#if 0
					//{
					//	sQTMAPNODE  *parentNode = pNode->parent;
					//	int parentLvl = parentNode->level;
					//	int parentXidx = parentNode->xidx;
					//	int parentYidx = parentNode->yidx;
					//	int startParentXidx = 0, endParentXidx = 0;
					//	int startParentYidx = 0, endParentYidx = 0;
					//	int tileSize = 256;
					//	int tmp_xi,tmp_yi=0;
					//	if( parentXidx*2 == pNode->xidx)	
					//	{
					//		startParentXidx = 0;
					//		endParentXidx = tileSize/2-1;
					//	}
					//	else
					//	{
					//		startParentXidx = tileSize/2;
					//		endParentXidx = tileSize-1;
					//	}
					//	if(parentYidx*2 == pNode->yidx)
					//	{
					//		startParentYidx = 0;
					//		endParentYidx = tileSize/2-1;
					//	}
					//	else
					//	{
					//		startParentYidx = tileSize/2;
					//		endParentYidx = tileSize-1;
					//	}

					//	//1  插值到	3 4
					//	//			1 2
					//	//		
					//	{
					//		int i=0,j=0;
					//		for (tmp_yi = startParentYidx;tmp_yi<=endParentYidx;tmp_yi++)	
					//		{
					//			i=0;	// 新一行,回头
					//			for(tmp_xi=startParentXidx;tmp_xi<=endParentXidx;tmp_xi++)
					//			{
					//				int idx_parentNode = tmp_yi*tileSize + tmp_xi;
					//				int idx_curNode =j*tileSize + i;
					//				// RGB拷贝 1->1
					//				pNode->imgdata[idx_curNode*3] = parentNode->imgdata[idx_parentNode*3];
					//				pNode->imgdata[idx_curNode*3+1] = parentNode->imgdata[idx_parentNode*3+1];
					//				pNode->imgdata[idx_curNode*3+2] = parentNode->imgdata[idx_parentNode*3+2];

					//				// 1->2
					//				{
					//					idx_curNode =j*tileSize + i+1;
					//					pNode->imgdata[idx_curNode*3] = parentNode->imgdata[idx_parentNode*3];
					//					pNode->imgdata[idx_curNode*3+1] = parentNode->imgdata[idx_parentNode*3+1];
					//					pNode->imgdata[idx_curNode*3+2] = parentNode->imgdata[idx_parentNode*3+2];
					//				}

					//				// 1->3
					//				{
					//					idx_curNode =(j+1)*tileSize + i;
					//					pNode->imgdata[idx_curNode*3] = parentNode->imgdata[idx_parentNode*3];
					//					pNode->imgdata[idx_curNode*3+1] = parentNode->imgdata[idx_parentNode*3+1];
					//					pNode->imgdata[idx_curNode*3+2] = parentNode->imgdata[idx_parentNode*3+2];
					//				}

					//				// 1->4
					//				{
					//					idx_curNode =(j+1)*tileSize + i+1;
					//					pNode->imgdata[idx_curNode*3] = parentNode->imgdata[idx_parentNode];
					//					pNode->imgdata[idx_curNode*3+1] = parentNode->imgdata[idx_parentNode+1];
					//					pNode->imgdata[idx_curNode*3+2] = parentNode->imgdata[idx_parentNode+2];
					//				}
					//				i+=2;
					//			}
					//			j+=2;
					//		}
					//	}
					//}
#endif
					pNode->tqsInFile = 0;	// 标记节点tqs不在文件树中
					pNode->tqsParentInFile = pNode->parent->tqsParentInFile;	// 获取tqs存在在文件树在父节点
				}
				else
				{
					pNode->tqsInFile = 1;	// 标记节点tqs在文件树中
					pNode->tqsParentInFile = pNode;	// 如果该节点tqs在文件树中,就记录它自己
				}
			#endif


#ifdef USE_VQT
			/*调入矢量纹理数据*/
			if ((pNode->pSubTree->vqtloader != NULL)&&(pNode->pSubTree->vqtparam != NULL))
			{
				if (-1 == pNode->pSubTree->vqtloader(pNode->level, pNode->xidx, pNode->yidx, 
					pNode->pSubTree->vqtparam, &pNode->vqtdata))
				{
					DEBUG_PRINT("load vqt failed.");
				}
				else	
				{

					if(0)	//jpg数据补充alpha通道
					{
						//把RGB的数据扩展为RGBA的颜色数据
						f_uint8_t dst_color[3] = {0, 0, 255};
						AddVqtAlpha(pNode, 256,256, dst_color);
					}
					else
					{

					}					
				}			
			}
#endif

#ifdef USE_CRH
			/*调入晕眩纹理数据地形图*/
			if (pNode->pSubTree->clrloader != NULL )
			{
				pNode->pSubTree->clrloader(pNode->level, pNode->xidx, pNode->yidx, 
					    pNode->pSubTree->clrparam, &pNode->clrdata);
			}
#endif

#ifdef USE_HIL
			/*调入shade 纹理数据高程图*/
			if (( pNode->pSubTree->hilloader )&&(pNode->pSubTree->hilparam != NULL))
			{
				pNode->pSubTree->hilloader(pNode->level, pNode->xidx, pNode->yidx, 
					    pNode->pSubTree->hilparam, &pNode->hildata);

#if 0
				/*增加修改shade 数据的alpha值*/
				if (pNode->hildata != NULL)
				{
					setTqtAlpha(pNode->hildata, 256, 256, 19);
				}

				/* lpf add 手动混合clr和hil纹理*/
				SetClrHilBlend(pNode, 256, 256);
#endif
			}
#endif
				    
			/*生成网格*/
//			if(eScene_normal == pHandle->mdctrl.scene_mode)
            {   /* 正常模式，需用地形高程生成网格 */
				if (qtmapNodeGenGlobalGrid(pNode, &terrain[0])<0)
				    DEBUG_PRINT("qtmapNodeGenGlobalGrid failed.");				
			}
// 			else
// 			{   /* 鹰眼模式，不需用地形高程生成网格 */
// 				if (qtmapNodeGenGlobalGridForEyeBird(pNode)<0)
// 				    DEBUG_PRINT("qtmapNodeGenGlobalGridForEyeBird failed.");
// 			}


#ifdef  SVS_SET
	//生成法线
	GenerateTileNormal(pNode);

	//生成SVS 纹理坐标空间
	GenerateNodePVertexTex(pNode);

	//生成SVS 纹理坐标
	GenerateTexCoord(pNode);
	
	if (qtmapNodeGenGlobalGridFlat(pNode, &terrain[0])<0)
		DEBUG_PRINT("qtmapNodeGenGlobalGrid failed.");

	//生成SVS 颜色数组
	GenerateVertexColor(pNode);

#endif	// SVS_SET

			// 计算瓦片俯视下的包围球 lpf add 2018年5月10日9:59:35
	 		pNode->nodeRangeOverlook = getNodeRangeOSG_Overlook(pNode);

			//计算瓦片的包围球及球心
			if((pNode->level < 2)||(pNode->level > 12))		//lpf add 2017年12月7日15:31:04 因为判断瓦片的尺寸是按照旧的包围球计算，在精细的瓦片下，需要采用旧的包围球，0和1级采用新的包围球。
	 			pNode->nodeRange = getNodeRangeOSG(pNode);
			else
				pNode->nodeRange = getNodeRange(pNode);

			//增加精准计算的包围球的结构体
			if((pNode->level < 2)||(pNode->level > 12))		
				pNode->nodeRangeOsg = pNode->nodeRange;
			else
				pNode->nodeRangeOsg = getNodeRangeOSG(pNode);

			// 计算瓦块顶点局部坐标
			if(0 !=  qtmapNodeGenPartGrid(pNode))
			{
				DEBUG_PRINT("qtmapNodeGenPartGrid failed.");
			}

			if (pNode->fvertex != NULL)
			{
				qtmapnodeSetStatus(pNode, DNST_READY, __LINE__);
				is_dnst_waitforload = FALSE;
				takeSem(pHandle->rendscenelock, FOREVER_WAIT);
				pHandle->loadedNumber++;
				giveSem(pHandle->rendscenelock);
			}
			else
			{
				/*解除父节点到本节点的指针*/
				for(i = 0; i < 4; i++)
				{
					if(NULL != pNode->parent)
					{
					    if(pNode->parent->children[i] == pNode)
						    pNode->parent->children[i] = NULL;
				    }
				}	
				
				qtmapnodeSetStatus(pNode, DNST_READYFORDELETE, __LINE__);
	            qtmapnodeDestroyData(pHandle, pNode);	
	            qtmapnodeSetStatus(pNode, DNST_EMPTY, __LINE__);			
				printf("Node %d, %d, %d load failed\n", pNode->level, pNode->xidx, pNode->yidx);
			}
		}
		else
		{
			/* 等待信号量,判断是否有新创建的瓦片节点,timeout时间为100ms */
			takeSem(pHandle->anyrequire, 100);
		}

#ifdef USE_SQLITE
		/* 调度qtn 数据库 */
		QtnSqlLoad(pHandle);
#endif

		/* 任务心跳计数自增 */
		imgLoadTaskHeartBeat++;
	}
	
	giveSem(pHandle->threadended);
}

/*
功能：获取影像数据加载任务的心跳
输入：
      无
输出：
      无
返回值：
	  任务心跳
*/
unsigned long getImgLoadTaskHeartBeat()
{
	return imgLoadTaskHeartBeat;
}

