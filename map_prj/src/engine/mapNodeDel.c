#include "mapNodeDel.h"
#include "mapNodeGen.h"
#include "memoryPool.h"
#include "osAdapter.h"

// ������������ʾ�б�
f_int32_t qtmapnodeDestroyGpuData(sMAPHANDLE * pHandle, sQTMAPNODE * pNode)
{
	f_int32_t i;
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeDestroyData with error node\n");
		return 0;
	}

	/*ɾ������������ʾ�б���ص��ڴ棬��ʡ֡��*/
	if (0 != (pNode->texid))
	{
		// �ϲ�ģʽ,���ʹ���˸�������,�Ͳ���Ҫ�Լ��ͷ�
#ifdef USE_FAULT_Tile
		// �ڵ�tqs�����ļ�����,һ��ʹ���˸�������
		if(pNode->tqsInFile == 0)
		{
			pNode->tqsParentInFile->tqsUsedNumInChild--;// ����Ը����������
		}
		// �ڵ�tqs���ļ�����,���δ���������ͷ���������
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
	
	/*����ý����QTMAPDATA�Ļ�����У����ȴӱ��н���*/
	for (i = 0; i < MAXMAPLAYER; i++)
	{
		if (pHandle->layerfunc[i].funcdestroy != NULL)
		{
			pHandle->layerfunc[i].funcdestroy(pNode->nodeid[i]);
			pNode->nodeid[i] = 0;
		}
	}
	
	/*���GPU֡����ص���Ϣ������/��ʾ�б��*/
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
    ɾ��һ����ͼ���ݽ�㣬�ͷ�ռ�õ���Դ������ý�����ӽ�㣬����ɾ��֮��
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
	/*ֻɾ������״̬�Ľڵ㣬�����ڵ㲻��*/
	if(DNST_READY != st)
		return -1;
	//qtmapnodeDeleteFromList(pData, pNode);  //���ﲻ���Ƴ�������Ĳ����ˣ��ýڵ㻹׼��������
	
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

	/*�����͸ո����ɵĽ�㲻ɾ��*/
	if ((pNode->level > 0)  
//		&& (pNode->drawnumber != 0)   //���Ǹո����ɵ�
		&& (nodetime > 3)          //����ʱ�䳬��5��
		&& (abs(pNode->drawnumber - pHandle->drawnumber) > MAPNODEDELNUM)   //�ϴλ�����XXX֮֡ǰ
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

	/*ɾ������δ������Ƭ��������ʡ�Դ�*/
	if((pNode->drawnumber != 0)
	 && (nodetime > 3)
	 && (abs(pNode->lastDrawNumber - pHandle->drawnumber) > MAPNODEDELNUM)   //�ϴλ�����XXX֮֡ǰ	
	 )
	{
		/*���GPU֡����ص���Ϣ������/��ʾ�б��*/
		qtmapnodeDestroyGpuData(pHandle, pNode);
	}

	/*����ӽ��*/
	for (i = 0;i<4;i++)
	{
		qtmapClearCache(pHandle, pNode->children[i]);
	}	
	
	return 1;	
}
