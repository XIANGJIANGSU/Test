#include "mapNodeDel.h"
#include "mapNodeGen.h"
#include "memoryPool.h"
#include "osAdapter.h"

// 清理纹理与显示列表
f_int32_t qtmapnodeDestroyGpuData(sMAPHANDLE * pHandle, sQTMAPNODE * pNode)
{
	f_int32_t i;
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeDestroyData with error node\n");
		return 0;
	}

	/*删除结点的纹理，显示列表相关的内存，节省帧存*/
	if (0 != (pNode->texid))
	{
		// 断层模式,如果使用了父层纹理,就不需要自己释放
#ifdef USE_FAULT_Tile
		// 节点tqs不在文件树上,一定使用了父层纹理
		if(pNode->tqsInFile == 0)
		{
			pNode->tqsParentInFile->tqsUsedNumInChild--;// 解除对父纹理的引用
		}
		// 节点tqs在文件树上,如果未被引用则释放自身纹理
		else
		{
			if(pNode->tqsUsedNumInChild <= 0)
			{
				//glextDeleteTextures(1, (const GLuint *)&pNode->texid);
				//pHandle->delTexNumber++;
			}
		}
#else
		glextDeleteTextures(1, (const GLuint *)&pNode->texid);
		pHandle->delTexNumber++;
#endif

#ifdef USE_FAULT_Tile
#else
		pNode->texid = 0;
		pNode->texUsed[0] = 0;
#endif
	}

	if (0 != (pNode->vtexid))
	{
		glDeleteTextures(1, (const GLuint *)&pNode->vtexid);
		pNode->vtexid = 0;
		//pHandle->delTexNumber++;

		pNode->texUsed[1] = 0;
	}

	if (0 != (pNode->clrtexid))
	{
		glextDeleteTextures(1, (const GLuint *)&pNode->clrtexid);
		pNode->clrtexid = 0;
		//pHandle->delTexNumber++;
	}

	if (0 != (pNode->hiltexid))
	{
		glextDeleteTextures(1, (const GLuint *)&pNode->hiltexid);
		pNode->hiltexid = 0;
		//pHandle->delTexNumber++;
	}

	if(0 != (pNode->tileList) )
	{
		glDeleteLists(pNode->tileList, 1);
		pNode->tileList = 0;
	}

	if(0 != (pNode->tileListOverlook) )
	{
		glDeleteLists(pNode->tileListOverlook, 1);
		pNode->tileListOverlook = 0;
	}
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	if (0 != (pNode->tileOverlookVertexVBO))
	{
		glDeleteBuffers(1, &(pNode->tileOverlookVertexVBO));		
		pNode->tileOverlookVertexVBO = 0;
	}

	if (0 != (pNode->tileVertexVBO))
	{
		glDeleteBuffers(1, &(pNode->tileVertexVBO));		
		pNode->tileVertexVBO = 0;
	}

	if (0 != (pNode->tileTerrainVBO))
	{
		glDeleteBuffers(1, &(pNode->tileTerrainVBO));		
		pNode->tileTerrainVBO = 0;
	}

	if (0 != (pNode->tileColorVBO))
	{
		glDeleteBuffers(1, &(pNode->tileColorVBO));		
		pNode->tileColorVBO = 0;
	}

	if (0 != (pNode->tileNormalVBO))
	{
		glDeleteBuffers(1, &(pNode->tileNormalVBO));		
		pNode->tileNormalVBO = 0;
	}

	if (0 != (pNode->tileGridTexVBO))
	{
		glDeleteBuffers(1, &(pNode->tileGridTexVBO));		
		pNode->tileGridTexVBO = 0;
	}
#endif
	if(0 != (pNode->tileVecShadeList) )
	{
		glDeleteLists(pNode->tileVecShadeList, 1);
		pNode->tileVecShadeList = 0;
	}

#ifdef _JM7200_
	if(0 != (pNode->tileSVSList) )
	{
		glDeleteLists(pNode->tileSVSList, 1);
		pNode->tileSVSList = 0;
	}

	if(0 != (pNode->tileSVSGridList) )
	{
		glDeleteLists(pNode->tileSVSGridList, 1);
		pNode->tileSVSGridList = 0;
	}
#endif

	if(0 != (pNode->edgeList) )
	{
		glDeleteLists(pNode->edgeList, 1);
		pNode->edgeList = 0;
	}

	if(0 != (pNode->edgeListOverlook) )
	{
		glDeleteLists(pNode->edgeListOverlook, 1);
		pNode->edgeListOverlook = 0;
	}

	return 0;
}


f_int32_t qtmapnodeDestroyData(sMAPHANDLE * pHandle, sQTMAPNODE * pNode)
{
	f_int32_t i;
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeDestroyData with error node\n");
		return 0;
	}
	
	/*如果该结点在QTMAPDATA的缓存表中，则先从表中解下*/
	for (i = 0; i < MAXMAPLAYER; i++)
	{
		if (pHandle->layerfunc[i].funcdestroy != NULL)
		{
			pHandle->layerfunc[i].funcdestroy(pNode->nodeid[i]);
			pNode->nodeid[i] = 0;
		}
	}
	
	/*清除GPU帧存相关的信息，纹理/显示列表等*/
	qtmapnodeDestroyGpuData(pHandle, pNode);

	if (pNode->imgdata != NULL)
	{
		DeleteAlterableMemory(pNode->imgdata);
		pNode->imgdata = NULL;
	}

	if (pNode->vqtdata != NULL)
	{
		DeleteAlterableMemory(pNode->vqtdata);
		pNode->vqtdata = NULL;
	}

	if (pNode->clrdata != NULL)
	{
		DeleteAlterableMemory(pNode->clrdata);
		pNode->clrdata = NULL;
	}

	if (pNode->hildata != NULL)
	{
		DeleteAlterableMemory(pNode->hildata);
		pNode->hildata = NULL;
	}
	
	if (pNode->fterrain != NULL)
	{
		DeleteAlterableMemory(pNode->fterrain);
		pNode->fterrain = NULL;
	}
	if (pNode->fvertex != NULL)
	{
		DeleteAlterableMemory(pNode->fvertex);
		pNode->fvertex = NULL;
	}

	if (pNode->fvertex_flat!= NULL)
	{
		DeleteAlterableMemory(pNode->fvertex_flat);
		pNode->fvertex_flat = NULL;
	}
		
	if (pNode->fvertex_part != NULL)
	{
		DeleteAlterableMemory(pNode->fvertex_part);
		pNode->fvertex_part = NULL;
	}

	if (pNode->pNormal != NULL)
	{
		DeleteAlterableMemory(pNode->pNormal);
		pNode->pNormal = NULL;
	}
	
	// SVS_SET
	if (pNode->m_pVertexTex != NULL)
	{
		DeleteAlterableMemory(pNode->m_pVertexTex);
		pNode->m_pVertexTex = NULL;
	}

	if (pNode->m_pColor!= NULL)
	{
		DeleteAlterableMemory(pNode->m_pColor);
		pNode->m_pColor = NULL;
	}
	
	for(i=0; i<4; i++)
	{
		if(pNode->parent->children[i] == pNode)
		{
			pNode->parent->children[i] = NULL;
		}
	}
	//pNode->nodekey = 0;
	//memset(pNode, 0, sizeof(sQTMAPDATANODE));
	//free(pNode);
	//pData->nodecount--;
	pHandle->delNodeNumber++;
	return 0;
}

void qtmapnodeDeleteFromList(sMAPHANDLE * pHandle, sQTMAPNODE * pNode)
{
	if (pNode == 0)
		return ;
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeDeleteFromList with error node\n");
		return;
	}
	takeSem(pHandle->rendscenelock, FOREVER_WAIT);
	LIST_REMOVE(&(pNode->stListHead));
	giveSem(pHandle->rendscenelock);
}

/*
    删除一个地图数据结点，释放占用的资源，如果该结点有子结点，则先删除之。
*/
f_int32_t qtmapnodeDestroy(sMAPHANDLE * pHandle, sQTMAPNODE * pNode)
{
	f_uint64_t st;
	if (pNode == 0)
		return 0;
	if (!ISMAPDATANODE(pNode))
	{
		printf("Try to destroy a removed node\n");
		return 0;
	}

	st = qtmapnodeGetStatus(pNode, __LINE__); 
	/*只删除就绪状态的节点，其它节点不管*/
	if(DNST_READY != st)
		return -1;
	//qtmapnodeDeleteFromList(pData, pNode);  //这里不做移除出链表的操作了，该节点还准备复用呢
	
	qtmapnodeSetStatus(pNode, DNST_READYFORDELETE, __LINE__);
	qtmapnodeDestroyData(pHandle, pNode);	
	qtmapnodeSetStatus(pNode, DNST_EMPTY, __LINE__);
	
	return 0;	
}

f_int32_t qtmapClearCache(sMAPHANDLE * pHandle, sQTMAPNODE * pNode)
{
	f_int32_t i, nodetime;
	f_int32_t ret = 0;
	
	if (pNode == NULL)
		return 0;
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapdataClearCache with error node\n");
		return 0;
	}

	nodetime = (tickCountGet() - pNode->createtime)/getSysClkRate();

	/*根结点和刚刚生成的结点不删除*/
	if ((pNode->level > 0)  
//		&& (pNode->drawnumber != 0)   //不是刚刚生成的
		&& (nodetime > 3)          //生成时间超过5秒
		&& (abs(pNode->drawnumber - pHandle->drawnumber) > MAPNODEDELNUM)   //上次画是在XXX帧之前
		&& (pNode->children[0] == NULL)
		&& (pNode->children[1] == NULL)
		&& (pNode->children[2] == NULL)
		&& (pNode->children[3] == NULL)
	   )
	{
		ret = qtmapnodeDestroy(pHandle, pNode);

//		printf("delete node level = %d, x = %d, y = %d,	nodetime = %d,pNode->drawnumber=%d, pHandle->drawnumber=%d, ret = %d\n",
//			pNode->level, pNode->xidx, pNode->yidx,
//			nodetime, pNode->drawnumber, pHandle->drawnumber, ret);
		return ret;
	}

	/*删除长久未画的瓦片的纹理，节省显存*/
	if((pNode->drawnumber != 0)
	 && (nodetime > 3)
	 && (abs(pNode->lastDrawNumber - pHandle->drawnumber) > MAPNODEDELNUM)   //上次画是在XXX帧之前	
	 )
	{
		/*清除GPU帧存相关的信息，纹理/显示列表等*/
		qtmapnodeDestroyGpuData(pHandle, pNode);
	}

	/*清除子结点*/
	for (i = 0;i<4;i++)
	{
		qtmapClearCache(pHandle, pNode->children[i]);
	}	
	
	return 1;	
}
