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

/*��¼Ӱ������ݼ������������*/
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
	f_int16_t terrain[VTXCOUNT1 * VTXCOUNT1];	// һ���߿��Ӧ��33*33�����θ߳̽ڵ�
	f_int32_t ret, i;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pNode = NULL;
    BOOL is_dnst_waitforload = FALSE;
	static int zeroNodeLoaded = 0;		//�Ѽ��ص�0����Ƭ����
    	
	while (1)
	{
       if(SEM_TAKE_OK == takeSem(pHandle->endrequire, 0))   /* ��ֹ���� */
           break;	

		// 1.���Ȳ�����û�д����ص�0����Ƭ
		if(zeroNodeLoaded < 8)
		{
			takeSem(pHandle->rendscenelock, FOREVER_WAIT);
			/*����LRU��������û�д����صĽڵ�*/
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
			// 2.Ȼ���ټ���������Ƭ

			takeSem(pHandle->rendscenelock, FOREVER_WAIT);
			/*����LRU��������û�д����صĽڵ�*/
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

		// 3.��������			
		if(is_dnst_waitforload)
		{
			if (!ISMAPDATANODE(pNode))
			{
				DEBUG_PRINT("Load Node data with error node.");
				continue;
			}

			qtmapnodeSetStatus(pNode, DNST_LOADING, __LINE__);

            /* ����ģʽ�ŵ��õ������ݣ�ӥ��ģʽ����Ҫ�������� */
#if 1            
            //if(eScene_normal == pHandle->mdctrl.scene_mode)
            {
			/*�����������*/
		
                ret = -1;
				
				// �����������
			    ret = pNode->pSubTree->funcloadterrain(pNode->level, pNode->xidx, pNode->yidx, 
			                                           pNode->pSubTree->loadterrainparam, (f_int16_t**)&terrain[0]);
				
				// �ӵ�ǰ�������޷������Ӧ�߿�
				if(ret != 0)
				{
					// �ϲ�ģʽ, �޷������߿����ֵ���ɸ�����߳�
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
							int startParentXidx = 0, endParentXidx = 0;	// �Ӹ��ڵ���ȡ�̵߳Ŀ�ʼ-����λ��
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
								�����ڵ�(parentXidx,parentYidx)λ�õĸ߳����ݣ���ֵ��䵽�ӽڵ�(i,j)λ����
								����fterrain�ǽ����¶���,��terrain���������϶���(������Сγ��д��������),�����Ҫ���·�ת
								1  ��ֵ��	1 2
											3 4
							*/

							{
								int j=VTXCOUNT,i=0;
								int tmp_xi=0,tmp_yi=0;
								for (tmp_yi = startParentYidx;tmp_yi<=endParentYidx;tmp_yi++)	
								{
									i=0;
									for(tmp_xi=startParentXidx;tmp_xi<=endParentXidx;tmp_xi++)	// ���ڵ�ǰ��
									{
										int idx_curTerPos = j* VTXCOUNT1 + i;
										int idx_parentTerPos = tmp_yi*VTXCOUNT1 + tmp_xi;
										terrain[idx_curTerPos] = parentNode->fterrain[idx_parentTerPos]; // 1->1
										//printf("<%d,%d> ",i,j);

										// ����ǰ�����������������(һ����)
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
												// ���е���һ��
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
						// �Ƕϲ�ģʽ��,����޷��������ÿ�
						memset(terrain, 0, VTXCOUNT1 * VTXCOUNT1 * sizeof(f_int16_t));
					}
					#endif
				}
		    }
#endif

			/*����Ӱ����������*/
			ret = pNode->pSubTree->imgloader(pNode->level, pNode->xidx, pNode->yidx, 
				    pNode->pSubTree->imgparam, &pNode->imgdata);

			// Ӱ���������ʧ��
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

					//	//1  ��ֵ��	3 4
					//	//			1 2
					//	//		
					//	{
					//		int i=0,j=0;
					//		for (tmp_yi = startParentYidx;tmp_yi<=endParentYidx;tmp_yi++)	
					//		{
					//			i=0;	// ��һ��,��ͷ
					//			for(tmp_xi=startParentXidx;tmp_xi<=endParentXidx;tmp_xi++)
					//			{
					//				int idx_parentNode = tmp_yi*tileSize + tmp_xi;
					//				int idx_curNode =j*tileSize + i;
					//				// RGB���� 1->1
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
					pNode->tqsInFile = 0;	// ��ǽڵ�tqs�����ļ�����
					pNode->tqsParentInFile = pNode->parent->tqsParentInFile;	// ��ȡtqs�������ļ����ڸ��ڵ�
				}
				else
				{
					pNode->tqsInFile = 1;	// ��ǽڵ�tqs���ļ�����
					pNode->tqsParentInFile = pNode;	// ����ýڵ�tqs���ļ�����,�ͼ�¼���Լ�
				}
			#endif


#ifdef USE_VQT
			/*����ʸ����������*/
			if ((pNode->pSubTree->vqtloader != NULL)&&(pNode->pSubTree->vqtparam != NULL))
			{
				if (-1 == pNode->pSubTree->vqtloader(pNode->level, pNode->xidx, pNode->yidx, 
					pNode->pSubTree->vqtparam, &pNode->vqtdata))
				{
					DEBUG_PRINT("load vqt failed.");
				}
				else	
				{

					if(0)	//jpg���ݲ���alphaͨ��
					{
						//��RGB��������չΪRGBA����ɫ����
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
			/*������ѣ�������ݵ���ͼ*/
			if (pNode->pSubTree->clrloader != NULL )
			{
				pNode->pSubTree->clrloader(pNode->level, pNode->xidx, pNode->yidx, 
					    pNode->pSubTree->clrparam, &pNode->clrdata);
			}
#endif

#ifdef USE_HIL
			/*����shade �������ݸ߳�ͼ*/
			if (( pNode->pSubTree->hilloader )&&(pNode->pSubTree->hilparam != NULL))
			{
				pNode->pSubTree->hilloader(pNode->level, pNode->xidx, pNode->yidx, 
					    pNode->pSubTree->hilparam, &pNode->hildata);

#if 0
				/*�����޸�shade ���ݵ�alphaֵ*/
				if (pNode->hildata != NULL)
				{
					setTqtAlpha(pNode->hildata, 256, 256, 19);
				}

				/* lpf add �ֶ����clr��hil����*/
				SetClrHilBlend(pNode, 256, 256);
#endif
			}
#endif
				    
			/*��������*/
//			if(eScene_normal == pHandle->mdctrl.scene_mode)
            {   /* ����ģʽ�����õ��θ߳��������� */
				if (qtmapNodeGenGlobalGrid(pNode, &terrain[0])<0)
				    DEBUG_PRINT("qtmapNodeGenGlobalGrid failed.");				
			}
// 			else
// 			{   /* ӥ��ģʽ�������õ��θ߳��������� */
// 				if (qtmapNodeGenGlobalGridForEyeBird(pNode)<0)
// 				    DEBUG_PRINT("qtmapNodeGenGlobalGridForEyeBird failed.");
// 			}


#ifdef  SVS_SET
	//���ɷ���
	GenerateTileNormal(pNode);

	//����SVS ��������ռ�
	GenerateNodePVertexTex(pNode);

	//����SVS ��������
	GenerateTexCoord(pNode);
	
	if (qtmapNodeGenGlobalGridFlat(pNode, &terrain[0])<0)
		DEBUG_PRINT("qtmapNodeGenGlobalGrid failed.");

	//����SVS ��ɫ����
	GenerateVertexColor(pNode);

#endif	// SVS_SET

			// ������Ƭ�����µİ�Χ�� lpf add 2018��5��10��9:59:35
	 		pNode->nodeRangeOverlook = getNodeRangeOSG_Overlook(pNode);

			//������Ƭ�İ�Χ������
			if((pNode->level < 2)||(pNode->level > 12))		//lpf add 2017��12��7��15:31:04 ��Ϊ�ж���Ƭ�ĳߴ��ǰ��վɵİ�Χ����㣬�ھ�ϸ����Ƭ�£���Ҫ���þɵİ�Χ��0��1�������µİ�Χ��
	 			pNode->nodeRange = getNodeRangeOSG(pNode);
			else
				pNode->nodeRange = getNodeRange(pNode);

			//���Ӿ�׼����İ�Χ��Ľṹ��
			if((pNode->level < 2)||(pNode->level > 12))		
				pNode->nodeRangeOsg = pNode->nodeRange;
			else
				pNode->nodeRangeOsg = getNodeRangeOSG(pNode);

			// �����߿鶥��ֲ�����
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
				/*������ڵ㵽���ڵ��ָ��*/
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
			/* �ȴ��ź���,�ж��Ƿ����´�������Ƭ�ڵ�,timeoutʱ��Ϊ100ms */
			takeSem(pHandle->anyrequire, 100);
		}

#ifdef USE_SQLITE
		/* ����qtn ���ݿ� */
		QtnSqlLoad(pHandle);
#endif

		/* ���������������� */
		imgLoadTaskHeartBeat++;
	}
	
	giveSem(pHandle->threadended);
}

/*
���ܣ���ȡӰ�����ݼ������������
���룺
      ��
�����
      ��
����ֵ��
	  ��������
*/
unsigned long getImgLoadTaskHeartBeat()
{
	return imgLoadTaskHeartBeat;
}

