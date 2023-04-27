#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../projection/coord.h"
#include "../geometry/matrixMethods.h"
#include "../projection/project.h"
#include "../mapApi/common.h"
#include "../utility/tools.h"
#include "osAdapter.h"
#include "mapRender.h"
#include "terWarning.h"
#include "mapLoad.h"
#include "mapNodeGen.h"
#include "mapNodeJudge.h"
#include "mapNodeDel.h"
#include "atmosphere.h"
#include "textCollision.h"
#include "memoryPool.h"
#include "LayerCombineApi.h"
#include "../mapApp/mapApp.h"
#include "qtnLoad.h"
#include "shadeMap.h"
#include "../define/macrodefine.h"
#include "../model/WhiteCity.h"
#include "../../../include_vecMap/2dmapApi.h"
#include "../mapApp/paramGet.h"
//#include "../../vecMapEngine_prj/2dEngine/vecMapFont.h"

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32	
extern PFNGLACTIVETEXTUREPROC glActiveTexture;	
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;	
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f;
#endif                                   /* Ӱ���������������� */
#endif

#define MAXQTNODE	  60//110
#define NODE_NOT_READY   0
#define NODE_DRAWING     1

#define NODE_NOT_IN_VIEW 0   /* �ڵ㲻���ӿ��� */
#define NODE_IN_VIEW     1   /* �ڵ����ӿ��� */

#define NODE_AT_EARTH_BACK     0   /* �ڵ��ڵ����� */
#define NODE_NOT_AT_EARTH_BACK 1   /* �ڵ㲻�ڵ����� */

f_uint32_t gindices[index_count];  /* ����ģʽ����������� */
f_float32_t gTextureBuf[vertexNum*2];  /* ����ģʽ����Ķ�ά�������� */
f_uint32_t indices_eyebird[index_count_eyebird];  /* ӥ��ģʽ����������� */
f_float32_t gTextureBuf_eyebird[vertexNum_eyebird*2]; /* ӥ��ģʽ����Ķ�ά�������� */

static f_int32_t sMaxLevel = 0;
static f_int32_t sMinLevel = 0;

/* ��Ƭ���㼶��Ӧ���ȷ���Ŀ��������ľ���(��λ��m) */
f_float64_t glfLevelDelta[LEVEL_MAX] = 
{
	10018754.17139462, /* 0����Ƭ��Ӧ���ȷ���Ŀ��������ľ����ǳ���ܳ���1/4,����ܳ�Ϊ40075016.6855784861531768177614 */
	5009377.085697311, /* 1����0���1/2,֮���������� */
	2504688.542848655,
	1252344.271424327,
	626172.1357121638,
	313086.0678560819,
	156543.0339280409,
	78271.51696402048,
	39135.75848201024,
	19567.87924100512,
	9783.939620502560,
	4891.969810251280,
	2445.984905125640,
	1222.992452562820,
	611.4962262814100,
	305.7481131407050,
	152.8740565703525,
	76.43702828517625,
	38.21851414258812,
	19.10925707129406,
	9.554628535647032,
	4.777314267823516,
	2.388657133911758,
	1.194328566955879,
	0.597164283477939
};

VOIDPtr pMapDataSubTree;			/*��ͼ������*/

/* JM7200ʹ����ʾ�б�ȶ��������ܶ� */
/* ������Ƭʱ�Ƿ�ǿ��ʹ����ʾ�б�,0-ǿ��ʹ��,1-��ǿ��ʹ�� */
f_int32_t useArray = 1;	// gx test

/*ibo-����������VBO��tvbo-�������������vbo*/
static GLuint ibo = 0, tvbo = 0;
/*ibo-�����ӽǶ���������VBO��tvbo-�����ӽǶ������������vbo*/
static GLuint ibo_overlook = 0, tvbo_overlook = 0;

/*�Ƿ������Ƭ�߽��ߣ�0-�����ƣ�1-����*/
static int bDrawTileGrid = 0;
/*�Ƿ������Ƭ���ƣ�0-�����ƣ�1-����*/
static int bDrawTileName = 0;

/* ������ƺ��������� */
extern int textOutPutOnScreen(int fontType, float viewportX, float viewportY, unsigned char * stringOut, sColor4f textColor);

extern void EnableLight(BOOL bEnable);
extern void EnableLight0(BOOL bEnable);
extern void EnableLight1(BOOL bEnable);
extern void SetRenderState(int value);

/*
���ܣ������ӿڷ�Χ�������۲쵽����Ƭ���ȷ�����������ľ������жϵ��õ����㼶
���룺��Ƭ���ȷ�����������ľ���
����������Ȳ㼶
*/
static f_int32_t radioToLevel(const f_float64_t radio)
{
	f_int32_t i = 0;
	f_float64_t radio_parent_level = glfLevelDelta[0];
	for( i = 0; i < LEVEL_MAX; i++)
	{
		/* ��radio�ȵ�i��Ŀ�Ⱦ����,�����radio��һ��Ŀ�Ⱦ���Ϊ��i��ľ��� */
		if(radio > glfLevelDelta[i])
		{
			/* ��radio�ȵ�i��Ŀ�Ⱦ���С,��ȡ ��i��͵�i-1��ľ���֮�͵�1/2 ��radio���бȽ�; */
			/* ��radio����,�򷵻صĲ㼶Ϊi-1; ��radio��С,�򷵻صĲ㼶Ϊi */
			f_float64_t middle = (radio_parent_level + glfLevelDelta[i]) / 2;
			if(radio > middle)
				return (i!=0)?(i-1):0;
			else
				return i;
		}
		/* ��radio�ȵ�i��Ŀ�Ⱦ����,�����radio��һ��Ŀ�Ⱦ���Ϊ��i��ľ��� */
		radio_parent_level = glfLevelDelta[i];	
	}
	return LEVEL_MAX;
}

static f_int32_t getCurrentMaxLevel(f_int32_t scene_mode, f_float32_t view_angle, f_float32_t height)
{
	/* ������Ƭ��һ��������,radio��ʾ������һ����������ľ���(��λ��m) */
	f_float64_t scale, radio;

	/* ͨ������scale�Ĵ�С���Կ��ƻ���ͼ��Ķ���,������Ӿ���߶�����ʾ1��ͼ��,��scaleȡ2.0�� */
	/* ������Ӿ���߶�����ʾ2��ͼ��,��scaleȡ1.0����ʾ4��ͼ��,��ȡ1/2����ʾ8��ͼ��,��ȡ1/4���������ƣ�*/
	/* scaleԽС,ͼ��Խ��,scaleԽ��,ͼ��Խ�� */	
	scale = 1.0/4.0;

	/* ӥ��ģʽ�½����õ���Ƭ�㼶�µ������÷ֱ��ʸ��ߵ���Ƭ */
	//if(eScene_eyebird == scene_mode)
	//    scale *= 0.5; 
    
	/* �ü���ģ��(�򻯳�һ������������)ֻ�Ը����ӽ���Ч */
	radio = tan(view_angle * 0.5 * DE2RA) * height * scale;

	/* ���������Ȳ㼶 */
	return radioToLevel(radio);
}

/*
���ܣ���������ڵ�ĺ��ӽڵ�
���룺
	pHandle             ��ͼ������ָ��
	pNode               ��Ƭ�ڵ�ָ��
	emptyOrload         �Ƿ�ʼ����(FALSE-��ʼ����,TRUE-������)
�����
    ��
����ֵ��
    -1���ڵ㲻���ڻ����ӽڵ㴴��ʧ��
     0���ӽڵ㲻����������
     4���ӽڵ㴴���ɹ�
*/
static f_int32_t qtmapnodeCreateChildren(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, BOOL emptyOrload )
{
	f_int32_t i;
	/* ���ӽڵ�ָ�� */
	sQTMAPNODE * pChildNode;
	/* ����ָ�� */
	sQTMAPSUBTREE * pTree = NULL;
	/* ��¼�ɹ������ĺ��ӽڵ���� */
	f_int32_t ret = 0;

	/* �ж��������Ƭ�ڵ��Ƿ�Ϊ����keyֵ�Ƿ���ȷ */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeCreateChildren with error node\n");
		return -1;
	}

	/* �Ĳ���,ÿ����Ƭ�ڵ���4�����ӽڵ�,���ν����ж� */
	for (i = 0; i < 4; i++)
	{
		f_int32_t level, xidx, yidx;
		/* ��ǰ��Ƭ�ڵ�ĵ�i�����ӽڵ㲻Ϊ�� */
		if ((pNode->children[i] != NULL))
		{
			/* �����ӽڵ���SLEEP״̬(�����ú��ӽڵ��Ѵ�����δ��������),������Ҫ��������ʱ,�ı亢�ӽڵ�״̬,*/
			/* ��ά�����������ݸ�״̬�Ըú��ӽڵ�������ݼ��� */
			if( (emptyOrload == FALSE)
				&&( DNST_SLEEP == qtmapnodeGetStatus(pNode->children[i], __LINE__) ) )
			{
 				qtmapnodeSetStatus(pNode->children[i], DNST_WAITFORLOAD, __LINE__);
				//printf("level = %d, x = %d, y = %d, change node state\n",pNode->children[i]->level, pNode->children[i]->xidx, pNode->children[i]->yidx);
			}
			
			/* �����ĺ��ӽڵ������1,�����ж���һ�����ӽڵ� */
			ret ++;
			continue;			
		}
		
		/* ��ǰ��Ƭ�ڵ�ĵ�i�����ӽڵ�ΪNULL,�򴴽����ӽڵ� */
		/* ���㺢�ӽڵ�Ĳ㼶,x��������,y��������*/
		level = pNode->level + 1;
		xidx = pNode->xidx * 2 + (i & 1);	// ��Ӧ��lon����(���᷽��)
		yidx = pNode->yidx * 2 + (i / 2);	// ��Ӧ��lat����
		
		// ʼ�մ����ӽڵ㣬������������û��
		{
			/* �жϵ�ǰ��Ƭ�ڵ�ĵ�i�����ӽڵ��Ƿ��ڵ�ͼ����������������,������������������ͷ�ڵ�ָ�� 0-���� 1-�� */
			/* ������,�����к��ӽڵ��޷�����,ֱ�ӷ���0 */
			if (isNodeExistInMapSubreeList(pHandle, level, xidx, yidx, &pTree) == 0)
				return 0;
		}

		/* ������ǰ��Ƭ�ڵ�ĵ�i�����ӽڵ� 0-�ɹ� -1-ʧ�� */
		/* ������ʧ��,�������ӽڵ�Ҳ�����ٴ���,ֱ�ӷ���-1 */
		if (qtmapnodeCreate(pHandle, level, xidx, yidx, pTree, &pChildNode, emptyOrload)<0)
			return -1;
		else
		{
			/* ά���ڵ����¹�ϵ:�������ɹ�,��¼��ǰ��Ƭ�ڵ�ĺ��ӽڵ�,��¼���ӽڵ�ĸ��ڵ�,�γ��Ĳ��� */
			pNode->children[i] = pChildNode;
			pChildNode->parent = pNode;
			/* �����ĺ��ӽڵ������1,һ����Ƭ��Ȼ��4�����ӽڵ�,����ret��ֵ��ȻΪ4*/
			ret ++;
		}
	}
	
	return(ret);
}

/*
���ܣ���ͼ�������
���룺
    ��
����������ɹ���־
    0  �ɹ�
    -1 ʧ��
*/
f_int32_t createMapHandle()
{
	sMAPHANDLE *pHandle;
    f_int32_t ret = -1;

	pHandle = (sMAPHANDLE *)NewFixedMemory(sizeof(sMAPHANDLE));
	if (pHandle == NULL)
	{
		return(-1);
	}
	else
	{
	    memset((VOIDPtr *)pHandle, 0, sizeof(sMAPHANDLE));
	}
	
	/* ������Ƭ��������� */
	generateTileIndices();
	/* ������Ƭ������������� */
	generateTexBuffer();
	/* ����ӥ��ģʽ��Ƭ��������� */
	generateTileIndicesEyeBird();
	/* ����ӥ��ģʽ��Ƭ������������� */
	generateTexBufferEyeBird();

    memset(pHandle, 0, sizeof(sMAPHANDLE));
	pHandle->key = MAPHANDLEKEY;

    {
		stList_Head *pstListHead = NULL;
		/**ȫ�����������ʼ��**/
		pstListHead = &(pHandle->sub_tree.stListHead);
		LIST_INIT(pstListHead);
		/**����������Ƭ�ڵ������ʼ��**/
		pstListHead = NULL;
		pstListHead = &(pHandle->nodehead.stListHead);
		LIST_INIT(pstListHead);
    }	    
	
	/* ������ֵ�ź���,���ڻ��������֪��ά���ݼ����������´�������Ƭ�ڵ� */
	pHandle->anyrequire    = createBSem(Q_FIFO_SEM, FULL_SEM);
	pHandle->endrequire    = createBSem(Q_FIFO_SEM, EMPTY_SEM);
	pHandle->threadended   = createBSem(Q_FIFO_SEM, EMPTY_SEM);
	pHandle->rendscenelock = createBSem(Q_FIFO_SEM, FULL_SEM);
	/* ������ֵ�ź���,���ڻ�����������ά���ݼ�������ͬʱ����qtn�������ʱ�Ļ������� */
	pHandle->qtnthread = createBSem(Q_FIFO_SEM, FULL_SEM);
    // ��Щ������Ҫ��֤һ֡������(����ȡһ֡��ȫִ�����״̬,���ͳ���߿��������ʱ)
	pHandle->rendscendDrawlock = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pHandle->rendscenelock);
	giveSem(pHandle->qtnthread);
	giveSem(pHandle->rendscendDrawlock);

#ifdef ACOREOS
	{
		ACoreOs_id tstTaskid;
		/* ������ά���ݼ������� */
		//pHandle->loadthread    = spawnTask("imgloadthread", 103, FP_TASK_VX, 0x200000, qtmapDataLoadRoute, (f_int64_t)pHandle);
		spawnTaskTM3(0, 103, 0x200000, "imgloadthread", qtmapDataLoadRoute, (f_int32_t)pHandle, &tstTaskid);
		pHandle->loadthread = (f_int32_t)tstTaskid;
	}

#else

	/* ������ά���ݼ������� */
#ifdef TASKAFFINITY
	pHandle->loadthread = spawnTaskAffinity(1, "imgloadthread", 103, FP_TASK_VX, 0x200000, qtmapDataLoadRoute, (f_int64_t)pHandle);
#else
	pHandle->loadthread = spawnTask("imgloadthread", 95, FP_TASK_VX, 0x200000, qtmapDataLoadRoute, (f_int64_t)pHandle);
#endif

#endif
	/* �����ĵ�ͼ�����ֵ��ȫ�ֵĵ�ͼ������ */
	pMapDataSubTree = pHandle;

    return 0;	
}

/*
	���ø���ͼ��Ĺ��ܺ���
	������map_andle -- ��createMapHandle���ص�VOIDPtr���
	      layer -- ͼ���, 0..MAXMAPLAYER-1
		  param -- ���ܺ����ĵ��ò������ڵ��ø������ܺ���ʱ����ò���
		  funccreate -- ��Ӧͼ��Ľ�����ɺ���
		  funcdestroy -- ��Ӧͼ��Ľ������ɾ������
		  funcdisplay -- ��Ӧͼ��Ľ����ʾ����
		  funcdisplaypre -- ��Ӧͼ����ʾǰ�Ĵ�����
		  funcdisplaypro -- ��Ӧͼ����ʾ��Ĵ�����
    ����ֵ��
	      0 -- �ɹ�
		 -1 -- ��ͼ���ݾ����Ч
		 -2 -- ͼ�����Ч
*/
f_int32_t qtmapSetLayerFunc(VOIDPtr map_handle, 
						    f_int32_t layer,
						    f_uint64_t param,
						    NODECREATEFUNC funccreate,
						    NODEFUNCDES funcdestroy,
						    NODEFUNC funcdisplay,
						    LAYERFUNCPRE funcdisplaypre,
						    LAYERFUNCPRo funcdisplaypro)
{
	sMAPHANDLE * pHandle = NULL;
	sQTMAPSUBTREE * pSubTree = NULL;
	
	if (!ISMAPHANDLEVALID(map_handle))
		return -1;
	if ( (layer < 0) || (layer >= MAXMAPLAYER))
		return -2;
	pHandle = (sMAPHANDLE *)map_handle;
	pHandle->layerfunc[layer].param = param;
	pHandle->layerfunc[layer].funccreate = funccreate;
	pHandle->layerfunc[layer].funcdestroy = funcdestroy;
	pHandle->layerfunc[layer].funcdisplay = funcdisplay;
	pHandle->layerfunc[layer].funcdisplaypre = funcdisplaypre;
	pHandle->layerfunc[layer].funcdisplaypro = funcdisplaypro;
	return 0;
}

/*
���ܣ����ɵ�ͼ���������������ڵ�ǰ�Ѿ����ڵĽ����������һ�����������У�<level, xidx, yidx>Ϊ������
	�����λ�ã�levelsΪ�����ĸ߶ȡ�ע�⣬ÿ�����ӵ�����������ȫ������ÿ����㶼���ڣ����û�
	���Զ�ε��øú�������һ�÷���ȫ���Ĳ�����
	������map_andle -- ��createMapHandle���ص�VOIDPtr���
		  level -- ��� 0 ~ 40
		  xidx, yidx -- �ڸò��еĽ����
		  levels -- �����ĸ߶�
		  funcLoadTerrain -- �õ��������ݵĺ���
		  loadterrainparam -- �������ݵĲ���
		  infoloader   ��ȡ��Ƭ��Ϣ����
		  infoparam    ��Ƭ����
		  imgisexist   �ж���Ƭ�Ƿ���ڵĺ���
		  imgloader -- �õ�����ͼ�����ݵĺ���
		  img2tex -- ����ͼ�����ݱ��OpenGL����ĺ���
		  imgparam -- ����ͼ�����ݴ���ʱ���Ĳ���
    ����ֵ��
	      0 -- �ɹ�
		 -1 -- ��ͼ���ݾ����Ч
		 -2 -- ������Ч
		 -3 -- �ڴ����ʧ��
		 -4 -- �����û�е���
*/
f_int32_t createMapSubtreeList(VOIDPtr map_andle, 
						       f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_int32_t levels,
						       NODETERRAINLOADFUNC funcLoadTerrain,
						       f_uint64_t          loadterrainparam,
						       NODEINFOLOADFUNC    infoloader,
	                           f_uint64_t          infoparam,
	                           NODEIMGISEXISTFUNC  imgisexist,
						       NODEIMGLOADFUNC     imgloader,
						       NODEIMG2TEXFUNC     img2tex,
						       f_uint64_t          imgparam,
						       
						       NODEIMGLOADFUNC     vqtloader,
						       NODEIMG2TEXFUNC     vqt2tex,
						       f_uint64_t          vqtparam,
						       
						       NODEIMGLOADFUNC     clrloader,
						       NODEIMG2TEXFUNC     clr2tex,
						       f_uint64_t          clrparam,

						       NODEIMGLOADFUNC     hilloader,
						       NODEIMG2TEXFUNC     hil2tex,
						       f_uint64_t          hilparam	
						       )
{
	sMAPHANDLE * pHandle;
	sQTMAPSUBTREE * pSubTree;
	
	if (!ISMAPHANDLEVALID(map_andle))
		return -1;

	/* ��ʼ�㼶level������ڵ���0���ܲ㼶levels������ڵ���1����ʼ��Ƭx��������xidx��[0,2^(level+2))����ʼ��Ƭy��������yidx��[0,2^(level+1)) */
	if ((level < 0) || (levels < 1) || (xidx < 0) || (yidx < 0) || (xidx >= (1 << (level+2))) || (yidx >= (1 << (level+1))))
		return -2;
	
	pHandle = (sMAPHANDLE *)map_andle;
	/* �����ʼ�㼶����0�������Ȼ�����ҵ�һ�ø��������йҽӣ�����Ҳ������򲻻�����ʹ�� */
	/* �����ʼ�㼶����0�����Ȼ��8�ø������������жϣ�ֱ�����ø��ຯ��ָ�뼴�� */
	if (level > 0)
	{
		/* �ж��������ڵ�ĸ��ڵ��Ƿ��������һ�������У�������ڣ�ֱ�Ӷ��������� */
		/* �ж��������ڵ����ʼ�ڵ�(level,xidx,yidx)�����丸�ڵ��Ȼ��(level-1,xidx/2,yidx/2) */
		if (!isNodeExistInMapSubreeList(pHandle, level - 1, xidx / 2, yidx / 2, NULL))
			return -4;
	}
	/* �����ڴ�ռ����ڴ�������ṹ��sQTMAPSUBTREE */
	pSubTree = (sQTMAPSUBTREE *)NewFixedMemory(sizeof(sQTMAPSUBTREE));
	if (pSubTree == NULL)
		return -3;

	/* ������������ʼ�㼶����ʼ��Ƭx������������ʼ��Ƭy�����������ܲ㼶 */
	pSubTree->level = level;
	pSubTree->xidx = xidx;
	pSubTree->yidx = yidx;
	pSubTree->levels = levels;

	/* ����������ter���ݶ�ȡ�������ú������� */
	pSubTree->funcloadterrain  = funcLoadTerrain;
	pSubTree->loadterrainparam = loadterrainparam;
	/* ����������info���ݶ�ȡ�������ú������� */
	pSubTree->infoloader       = infoloader;
	pSubTree->infoparam        = infoparam;
	/* ����������ָ����Ƭ�Ƿ���ڵ�ǰ�����е��жϺ��� */
	pSubTree->imgisexist       = imgisexist;
	/* ����������tqs���ݶ�ȡ������tqs������������ĺ�����tqs���ݶ�ȡ�������� */
	pSubTree->imgloader        = imgloader;
	pSubTree->img2tex          = img2tex;
	pSubTree->imgparam         = imgparam;
	/* ����������vqt���ݶ�ȡ������vqt������������ĺ�����vqt���ݶ�ȡ�������� */
	pSubTree->vqtloader	       = vqtloader;
	pSubTree->vqt2tex		   = vqt2tex;
	pSubTree->vqtparam	       = vqtparam;
	/* ����������crh���ݶ�ȡ������crh������������ĺ�����crh���ݶ�ȡ�������� */
	pSubTree->clrloader	       = clrloader;
	pSubTree->clr2tex		   = clr2tex;
	pSubTree->clrparam	       = clrparam;
	/* ����������hil���ݶ�ȡ������hil������������ĺ�����hil���ݶ�ȡ�������� */
	pSubTree->hilloader	       = hilloader;
	pSubTree->hil2tex		   = hil2tex;
	pSubTree->hilparam	       = hilparam;
	
    {
        /* �����������뵽ȫ�ֵ����������У��������������ǰ�� */
		stList_Head *pstListHead = NULL;
		pstListHead = &(pHandle->sub_tree.stListHead);
		LIST_ADD(&pSubTree->stListHead, pstListHead);	
    }	
	
	return 0;
}

/*
���ܣ���ƬSVSģʽ�»��ƺ�����ֻ����SVSģʽ���޵��ӷ�������ˮƽ�澯����

���룺
	pHandle              ��ͼ������ָ��
	pNode                ��Ƭ�ڵ�ָ��
	pScene               ��Ⱦ����ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��������
	-1  ��Ƭ�ڵ�����Ч
	-2  ��ͼ��������Ч
*/
static f_int32_t mapNodeRenderImgSVS(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* ���ͼ���Ƿ���� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return 0;
	}
	if(NULL == pHandle)
	    return 0;
		
	switch(pScene->mdctrl_cmd.scene_mode)
	{
	    default:
	        count = index_count;
	        pIndex = gindices;
	        pTex = gTextureBuf;
	    break;

	}
	
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);
		

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3,GL_UNSIGNED_BYTE,0,pNode->m_pColor);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);		
	glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	


	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, pIndex);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);	
	glDisableClientState(GL_COLOR_ARRAY);

	//glFinish();

#endif
	pHandle->drawnnodnum++;
	//printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	pNode->lastDrawNumber = pHandle->drawnumber;
	return 0;

}

/*
���ܣ���ƬVecShadeģʽ�»��ƺ���

���룺
	pHandle              ��ͼ������ָ��
	pNode                ��Ƭ�ڵ�ָ��
	pScene               ��Ⱦ����ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��������
	-1  ��Ƭ�ڵ�����Ч
	-2  ��ͼ��������Ч
*/
static f_int32_t mapNodeRenderVecImg(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �ж��������Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}

	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* Ĭ��Ϊ����ģʽ */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* �������л�����Ƭ�ڵ����С�㼶 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* ʹ������Ԫ0,�ر�һά����,������ά���� */
	glActiveTexture(GL_TEXTURE0);
	//glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
	    
	/* ��������Ԫ0������id������任����,����Ԫ0ΪVecShade���� */
	RenderShadeMapPre(pScene);

	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,������,����������� */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* �ر�����Ԫ1��һά����Ͷ�ά����ʹ�ܿ��� */
		glActiveTexture(GL_TEXTURE1);
	    //glDisableEx(GL_TEXTURE_1D);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* ʹ������Ԫ0 */
	glActiveTexture(GL_TEXTURE0);

	/* �ָ������ջΪģ����ͼ���� */
	glMatrixMode(GL_MODELVIEW);	
	/* ��ԭ����ģ����ͼ�������ѹջ���� */
	glPushMatrix();

#ifndef _JM7200_
	/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������꣬�˴���������任 */
	/* Ϊ��ʹ�����ͻ������Ƭ���㣬����ģ����ͼ�������û�׼��xyz������ϵ�� */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif

	/* ������ɫΪ��ɫ,͸����Ϊ��ȫ��͸�� */
	glColor4f(1.0f,1.0f,1.0f, 1.0f);	

	/* ���ǿ��ʹ����ʾ�б�(useArray=0ʱ),��ʹ����ʾ�б�ķ�ʽ���� */
	/* ����ͨ����������ķ�ʽ���� */
	if((useArray == 0))	
    {
#ifdef _JM7200_
		/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������� �����������õ����������ľֲ����꣬����任����*/
		CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif
        qtmapNodeCreateVecShadeList(pNode, pScene->is_need_terwarning, pScene->mdctrl_cmd.scene_mode);

		if((pNode->tileVecShadeList) != 0)
			glCallList(pNode->tileVecShadeList);
			
    }
	else
	{
		/* ������Ƭ�Ķ������� */
		glEnableClientState(GL_VERTEX_ARRAY);	

	#ifdef _JM7200_
		/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������� */
		glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);
	#else
		/* ʹ�����͵Ķ��� */
		glVertexPointer(3, GL_INT, 0, pNode->fvertex_part);	
	#endif

		/* ������Ƭ��������0��������������,��VecShadeģʽ������ */
		RenderShadeMap(pScene, pNode);
	
		/* �����Ҫ���θ澯,��������Ƭ��������1�������������� */
		if(pScene->is_need_terwarning)
		{
			RenderTerrainWarningStripeColor2(pScene, pNode);
		}

		/* ������Ƭ����,�������������ķ�ʽ���� */
		glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);
    
		/* �ر���������Ͷ��������ʹ��,��Enable��Ӧ */
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);		
	}

	/* �����Ҫ���θ澯,��ָ�����1��״̬ */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

	glDisableEx(GL_TEXTURE_2D);

	/* �ָ�����0��״̬��������� */
	RenderShadeMapPro();

	
#if 0
	/* ����ģʽ�»�����Ƭ���ߵ���ʾ�б�ӥ��ģʽ������ */
	if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
	{	
		/* ������Ƭ���ߵ���ɫ */
		glColor3f(0.5f, 0.8f, 0.8f);
		/* ��������ģʽ����Ƭ���ߵ���ʾ�б� */
		qtmapNodeCreateEdgeList(pNode);

		/* ������ƽ�� */
		glEnableEx(GL_LINE_SMOOTH);
		/* �ر���Ȳ���,��ֹ�߱��ڵ� */
		glDisableEx(GL_DEPTH_TEST);

		/* ��������ģʽ����Ƭ������ʾ�б���� */
		if((pNode->edgeList) != 0)
			glCallList(pNode->edgeList);
		
		glDisableEx(GL_LINE_SMOOTH);
		glEnableEx(GL_DEPTH_TEST);

	}
#endif

	/* ��ջ�лָ�ԭ����ģ����ͼ���� */
	glPopMatrix();

#if 0
{
	PT_3D nodexyz = {0};
	sColor4f color1={0.0};
	f_char_t timeInfo[128] = {0};
	Geo_Pt_D geoPt;
	PT_2I screenPt;

	/* ������Ƭ�㼶������������ֻ��Ƶ���ɫ,ʹ�ú�ɫ��ȫ��͸�� */
	color1.red = 1.0;
	color1.alpha = 1.0;


	//	nodexyz = pNode->obj_cent;
	//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

	/* �������ֻ��Ƶ�ľ�γ��,���ǵ������п��,Ϊ�������屣��������,���õ�����Ƭ����1/4����,�߶�Ϊƽ���߶� */
	/* δ������ת */
	geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
	geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
	geoPt.height = pNode->height_ct;

	//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
	/* ���ݾ�γ�߼�����Ļ���� */
	getOuterViewPtByGeo(pScene, geoPt, &screenPt);


	/* �ر���Ȳ���,��ֹ���ֱ��ڵ� */
	glDisableEx(GL_DEPTH_TEST);
	//	glDisableEx(GL_CULL_FACE);
	/* ����Ļ�ϻ�����Ƭ�㼶��������� */
	sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
	textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

	//	glEnableEx(GL_CULL_FACE);	
	glEnableEx(GL_DEPTH_TEST);
}
#endif

#endif
	/* ��ͼ�������л��ƵĽڵ��������1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* ��¼�ڵ���һ�λ��Ƶ�֡���� */
    pNode->lastDrawNumber = pHandle->drawnumber;
	return 0;
}



/*
���ܣ���ƬVecShadeģʽ�»��ƺ���(OpenGL ES)

���룺
	pHandle              ��ͼ������ָ��
	pNode                ��Ƭ�ڵ�ָ��
	pScene               ��Ⱦ����ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��������
	-1  ��Ƭ�ڵ�����Ч
	-2  ��ͼ��������Ч
*/
static f_int32_t mapNodeRenderVecImgEs(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �ж��������Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}

	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* Ĭ��Ϊ����ģʽ */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* �������л�����Ƭ�ڵ����С�㼶 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* ʹ������Ԫ0,�ر�һά����,������ά���� */
	glActiveTextureARB(GL_TEXTURE0);
	glEnableEx(GL_TEXTURE_2D);
	    
	/* ��������Ԫ0������id������任����,����Ԫ0ΪVecShade���� */
	RenderShadeMapPre(pScene);

	/* ��������Ԫ0���������ģʽΪ����GL_MODULATE,�����������RGBA=����Դ��RGBA*Ƭ�ε�RGBA */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,������,����������� */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* �ر�����Ԫ1��һά����Ͷ�ά����ʹ�ܿ��� */
		glActiveTextureARB(GL_TEXTURE1);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* ʹ������Ԫ0 */
	glActiveTextureARB(GL_TEXTURE0);

	/* �ָ������ջΪģ����ͼ���� */
	glMatrixMode(GL_MODELVIEW);	
	/* ��ԭ����ģ����ͼ�������ѹջ���� */
	glPushMatrix();

#ifndef VERTEX_USE_FLOAT
	/* ʹ�����εĶ������ݣ������ƶ�ģ����ͼ�����ƶ���ƫ����ʼ�� */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0],pNode->point_vertex_xyz[1],pNode->point_vertex_xyz[2],pNode->point_vertex_xyz[3]);
#endif	

	/* ������ɫΪ��ɫ,͸����Ϊ��ȫ��͸�� */
	glColor4f(1.0f,1.0f,1.0f, 1.0f);	

	{
		GLint prog = 0;
		int vLoc, tLoc, tLoc1, nLoc, cLoc;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation((GLuint)prog, "a_position");
		tLoc = glGetAttribLocation((GLuint)prog, "a_texCoord0");
		tLoc1 = glGetAttribLocation((GLuint)prog, "a_texCoord1");

		/*������Ƭ�Ķ������VBO(��������/����߶�)��VecShadeģʽ�������ӽǶ��ǻ���33*33�����㣬ÿ����Ƭ��VBO�е����ݲ�һ��*/
		qtmapNodeCreateVboEs(pNode);
		/*������Ƭ�Ķ������VBO(��������)��������Ƭ����ЩVBO�е����ݾ�һ��*/
		genVecShadeConstVBOs(&ibo, &tvbo);

		{
			//glDisableEx(GL_CULL_FACE);    // fix the warning function under overlook viewmode

			/*ÿ����Ƭ��33*33��������Ƴ�����������*/
			glActiveTextureARB(GL_TEXTURE0);
			/*���ö�������*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif

			/*���ö�������0���꣬VecShade����*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
			glVertexAttribPointer(tLoc, 1, GL_FLOAT, 0, 0, 0);

			if (pScene->is_need_terwarning)/* ��Ҫ���θ澯 */
			{
				/*���ö�������1���꣬ˮƽ�澯����*/
				glEnableVertexAttribArray(tLoc1);
				glActiveTextureARB(GL_TEXTURE1);
				glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
				glVertexAttribPointer(tLoc1, 1, GL_FLOAT, 0, 0, 0);
			}
			/*���ö������������������ж���*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(vLoc);
			glDisableVertexAttribArray(tLoc);
			if (pScene->is_need_terwarning){
				glDisableVertexAttribArray(tLoc1);
			}
			//glFinish();
		}
	}

	glDisableEx(GL_TEXTURE_2D);

	/* �ָ�����0��״̬��������� */
	RenderShadeMapPro();

	/* ��ջ�лָ�ԭ����ģ����ͼ���� */
	glPopMatrix();

#endif

	/* ��ͼ�������л��ƵĽڵ��������1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* ��¼�ڵ���һ�λ��Ƶ�֡���� */
    pNode->lastDrawNumber = pHandle->drawnumber;
	return 0;
}

/*
���ܣ���Ƭ����ģʽ�»��ƺ���

���룺
	pHandle              ��ͼ������ָ��
	pNode                ��Ƭ�ڵ�ָ��
	pScene               ��Ⱦ����ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��������
	-1  ��Ƭ�ڵ�����Ч
	-2  ��ͼ��������Ч
	-3  ��Ƭ�ڵ�Ķ�άӰ��������ʧ��
*/
#define Do_FileTextureVertexData 1	// ��0�����ݷ����ļ�������
static f_int32_t mapNodeRenderImg(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �ж��������Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    return -2;
	}
	
	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	    case eScene_normal:
	        count = index_count;
	        pIndex = gindices;
	        pTex = gTextureBuf;
			break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
 	    case eScene_eyebird:
 	        count = index_count_eyebird;
 	        pIndex = indices_eyebird;
 	        pTex = gTextureBuf_eyebird;
 			break;	
		/* Ĭ��Ϊ����ģʽ */
		default:
			count = index_count;
			pIndex = gindices;
			pTex = gTextureBuf;
			break;
	}
	
	/* �������л�����Ƭ�ڵ����С�㼶 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* ʹ������Ԫ0,�ر�һά����,������ά���� */
    glActiveTexture(GL_TEXTURE0);
    //glDisableEx(GL_TEXTURE_1D);
    glEnableEx(GL_TEXTURE_2D);
    
	/* ��һ����������Ƭ�ڵ�Ķ�άӰ������,֮ǰ�Ѿ����������ظ����� */
	if ( (0 == (pNode->texid)) && (pNode->imgdata != NULL))
	{
		// �ϲ�ģʽ,�������һ�����ļ����еĸ��ڵ�����
#ifdef USE_FAULT_Tile
		// �ڵ�tqs�����ļ�����,ֱ�����ø�����
		if(pNode->tqsInFile == 0)
		{
			pNode->texid = pNode->tqsParentInFile->texid;
		}
		// �ڵ�tqs���ļ�����,�򹹽��ýڵ�����
		else
		{
			pNode->texid = pNode->pSubTree->img2tex(pNode->pSubTree->imgparam, pNode->imgdata);
			pHandle->createTexNumber++;
		}
#else
		/* ���ùҽӺ���,����Ƭ��Ӱ��������������һ����ά���� */
		pNode->texid = pNode->pSubTree->img2tex(pNode->pSubTree->imgparam, pNode->imgdata);
		/* ��ͼ�������еĴ����������������1 */
        pHandle->createTexNumber++;
#endif
	}

	// �ϲ�ģʽ,ʹ��һ�θ���������һ������
	#ifdef USE_FAULT_Tile
		if(pNode->tqsInFile == 0)
			pNode->tqsParentInFile->tqsUsedNumInChild++;// ���ӶԸ����������
	#endif
	
	/* ������Ƭ�ڵ�Ķ�άӰ������ʧ��,ֱ�ӷ��� */
	if (0 >= (pNode->texid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -3;
	}

	/* ���ݳ������ƾ���еĵ�ͼ��ʾģʽ�����ڲ���ͼ�����ģʽ */
	setLayDisplay(pScene);

	/* ��������ĵ�0ͼ�����ģʽ,�жϵ�ǰ֡ʹ�õĵ�ͼ�����Ƿ�����һ֡ʹ�õĲ�һ��,�����ͬ����а������л�����Ĳ��� */
	bindAndSubRenderTexID(pNode, 0);	// ����������

	#if Do_FileTextureVertexData
	{
		// ��ȡ����������
		if( pNode->level == 0)
		{
			int texid=0;
			char filePath[256];
			unsigned char ImgData[TILESIZE * TILESIZE * 3];
			int level = 0, xidx = 1, yidx = 2;
			int imgSize, bytesRead;
			FILE* fp;

			sprintf(filePath, "D:\\ImgData\\img_lvl_%d_xidx_%d_yidx_%d.txt", pNode->level, pNode->xidx, pNode->yidx);
			fp = fopen(filePath, "rb");
			imgSize = TILESIZE * TILESIZE * 3;
			bytesRead = fread(ImgData, sizeof(unsigned char), imgSize, fp);
			fclose(fp);

			glextTexSubImage2D(GL_TEXTURE_2D,0, 
				0,0, TILESIZE, TILESIZE, GL_RGB, GL_UNSIGNED_BYTE, ImgData);
		}
	}
	#endif

		
	/* ��������Ԫ0���������ģʽΪ����GL_MODULATE,�����������RGBA=����Դ��RGBA*Ƭ�ε�RGBA */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,������,����������� */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* �ر�����Ԫ1��һά����Ͷ�ά����ʹ�ܿ��� */
		glActiveTexture(GL_TEXTURE1);
	    //glDisableEx(GL_TEXTURE_1D);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* ʹ������Ԫ0 */
	glActiveTexture(GL_TEXTURE0);
	
	/* �ָ������ջΪģ����ͼ���� */
	glMatrixMode(GL_MODELVIEW);	
	/* ��ԭ����ģ����ͼ�������ѹջ���� */
	glPushMatrix();

#ifndef _JM7200_
	/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������꣬�˴���������任 */
	/* Ϊ��ʹ�����ͻ������Ƭ���㣬����ģ����ͼ�������û�׼��xyz������ϵ�� */
	/* ʹ�����ͻ������Ƭ����ĺô�������float���Ȳ��������·��й����е��λ���ֶ������ĳ����ξ��Ⱦ͹��� */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif

	/* ������ɫΪ��ɫ,͸����Ϊ��ȫ��͸�� */
	glColor4f(1.0f,1.0f,1.0f,1.0f);	

	/* ���ǿ��ʹ����ʾ�б�,��ʹ����ʾ�б�ķ�ʽ���� */
	/* �������ͨ����������ķ�ʽ���� */
	if((useArray == 0)/*||(
			(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn))*/)	
    {

#ifdef _JM7200_
		/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������ꣻ���������õ����������ľֲ����꣬����任����*/
		CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif
		/* ������Ƭ������������ʾ�б�(����������Ƭ����(33*33)�͸����»��Ƶ���ʾ�б�(��������Ƭ��4������) */
        qtmapNodeCreateList(pNode, pScene->is_need_terwarning, pScene->mdctrl_cmd.scene_mode);			
		qtmapNodeCreateListOverlook(pNode, pScene->is_need_terwarning, pScene->mdctrl_cmd.scene_mode);

		if(	(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
		{
			//* �ӽ�Ϊ������δ�����߶�Ԥ�����ܣ����ø����µ���ʾ�б���� */
			if((pNode->tileListOverlook) != 0)
				glCallList(pNode->tileListOverlook);	
		}
		else
		{
			/* �ӽ�Ϊ���ӵ������߶�Ԥ�����ܡ������ӽ�Ϊ�Ǹ����ӽǣ�����������ʾ�б���� */
		    if((pNode->tileList) != 0)
    			glCallList(pNode->tileList);
		}

		//glFinish();

    }
	else
	{	
		/* ������Ƭ�Ķ������� */
		glEnableClientState(GL_VERTEX_ARRAY);
#ifdef _JM7200_
		/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������� */
		//glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	// gx test

		// gx test for Debug д���߿鶥������ 33*33
	#if Do_FileTextureVertexData
		{
			// gx test for Debug д�߿鶥������ 33*33
			{
				int i,j=0;
				char filename[256];
				FILE *file=NULL;
				static int HasWrited = 0;
				if(HasWrited == 0)
				{
					sprintf(filename, "D:\\ImgData\\vertex_lvl_%d_xidx_%d_yidx_%d.txt", pNode->level, pNode->xidx, pNode->yidx);
					file = fopen(filename, "w");
					if (file != NULL) 
					{
						for (i = 0; i < vertexNum; i++) {
							for (j = 0; j < 3; j++) {
								fprintf(file, "%.6f", pNode->fvertex[i * 3 + j]);
								if (j < 2) {
									fprintf(file, " ");
								}
							}
							fprintf(file, "\n");
						}
						fclose(file);
					} 
					HasWrited =1;
				}
			}

			// gx test for Debug ���߿鶥������ 33*33
			{
				char filename[256];
				f_float32_t *fvertex = (f_float32_t *)malloc(vertexNum * 3 * sizeof(f_float32_t));
				int read_count = 0;
				FILE *file=NULL;
				sprintf(filename, "D:\\ImgData\\vertex_lvl_%d_xidx_%d_yidx_%d.txt", pNode->level, pNode->xidx, pNode->yidx);
				file = fopen(filename, "r");
				if (file != NULL) {
					while (fscanf(file, "%f", &fvertex[read_count]) != EOF && read_count < vertexNum * 3) {
						read_count++;
					}
					fclose(file);
				}

				if( pNode->level == 0)
				{
					glVertexPointer(3, GL_FLOAT, 0, fvertex);
				}
			}
		}
	#endif

		// gx test for Debug ����������
	#if Do_FileTextureVertexData
		{
			// ��ȡ��������
			{
				int pos = 0;
				FILE* fp = fopen("D:\\ImgData\\TextCoord.txt", "r");
				pTex = (f_float32_t *)malloc(vertexNum * 2 * sizeof(f_float32_t));
				if (fp != NULL) 
				{
					while (fscanf(fp, "%f %f", &pTex[pos * 2], &pTex[pos * 2 + 1]) == 2) 
					{
						pos++;
					}
					fclose(fp);
				}
			}


			// д��������
			{
				if (pNode->level == 0)
				{
					char filePath[256]={0};
					FILE *fp = NULL;
					sprintf(filePath, "D:\\ImgData\\img_lvl_%d_xidx_%d_yidx_%d.txt", pNode->level, pNode->xidx, pNode->yidx);
					fp = fopen(filePath, "wb");
					// �����ļ�����
					if (fp == NULL) {
						printf("Failed to create file!\n");
						return 1;
					}
					// д������
					fwrite(pNode->imgdata, sizeof(unsigned char), TILESIZE * TILESIZE * 3, fp);
					// �ر��ļ�
					fclose(fp);
				}
			}
		}
	#endif




#else
		/* ʹ�ø����͵Ķ��� */
//		glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	
		/* ʹ�����͵Ķ��� */
		glVertexPointer(3, GL_INT, 0, pNode->fvertex_part);	
#endif

		/* ������Ƭ��������0�������������� */
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, pTex);



		/* �����Ҫ���θ澯,��������Ƭ��������1�������������� */
		if(pScene->is_need_terwarning)
		{
		    RenderTerrainWarningStripeColor2(pScene, pNode);
		}

		/* ������Ƭ��������2��������������,Ŀǰδ����,ע�͵�,��JM7200��֧���������� */
		//glClientActiveTexture(GL_TEXTURE2);                
		//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		//glTexCoordPointer(2, GL_FLOAT, 0, pTex);	

		/* ������Ƭ����,�������������ķ�ʽ���� */
		//glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);	// gx test

#if Do_FileTextureVertexData	// gx test for Debug ʹ��������ݻ����߿�
		{
			// ��ȡ���������ļ�
			int *indices = (int *)malloc(index_count * sizeof(int));
			{
				int read_count = 0;
				FILE *file = fopen("D:\\ImgData\\vertexIdx.txt", "r");
				if (file != NULL)
				{
					while (fscanf(file, "%d", &indices[read_count]) != EOF && read_count < index_count)
					{
						read_count++;
					}
					fclose(file);
				}
			}

			//if(pNode->xidx == 2 && pNode->yidx == 0)
			{
				glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, indices);	// gx test
			}
		}

#endif

		/* �ر���������Ͷ��������ʹ��,��Enable��Ӧ */
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);	

	//	glFinish();

	}

	/* �����Ҫ���θ澯,��ָ�����1��״̬ */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

#if 0
	/* �ָ�����2��״̬ */
	glActiveTexture(GL_TEXTURE2);					        // ��������Ԫ1
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);					// ��������Ԫ0

#endif

	glDisableEx(GL_TEXTURE_2D);

	if(bDrawTileGrid == 1)
	{
		/* ����ģʽ�»�����Ƭ���ߵ���ʾ�б�ӥ��ģʽ������ */
		if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
		{	
			/* ������Ƭ���ߵ���ɫ */
			glColor3f(0.5f, 0.8f, 0.8f);
			/* ��������ģʽ����Ƭ���ߵ���ʾ�б� */
			qtmapNodeCreateEdgeList(pNode);
			/* ������������Ƭ���ߵ���ʾ�б� */
			qtmapNodeCreateEdgeListOverlook(pNode);
	
			/* ������ƽ�� */
			glEnableEx(GL_LINE_SMOOTH);
			/* �ر���Ȳ���,��ֹ�߱��ڵ� */
			glDisableEx(GL_DEPTH_TEST);

			if(	(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
				&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
			{
				//* �ӽ�Ϊ������δ�����߶�Ԥ�����ܣ����ø����µ���Ƭ������ʾ�б���� */
				if((pNode->edgeListOverlook) != 0)
					glCallList(pNode->edgeListOverlook);
			}
			else
			{
				/* �ӽ�Ϊ���ӵ������߶�Ԥ�����ܡ������ӽ�Ϊ�Ǹ����ӽǣ���������ģʽ����Ƭ������ʾ�б���� */
				if((pNode->edgeList) != 0)
					glCallList(pNode->edgeList);
			}
		
			glDisableEx(GL_LINE_SMOOTH);
			glEnableEx(GL_DEPTH_TEST);
		
		}

	}

	/* ��ջ�лָ�ԭ����ģ����ͼ���� */
	glPopMatrix();


	// �����߿���|�߿��
#if 1
	if(bDrawTileName == 1)
	{
		PT_3D nodexyz = {0};
		sColor4f color1={0.0};
		f_char_t timeInfo[128] = {0};
		Geo_Pt_D geoPt;
		PT_2I screenPt;
	
		/* ������Ƭ�㼶������������ֻ��Ƶ���ɫ,ʹ�ú�ɫ��ȫ��͸�� */
		color1.red = 1.0;
		color1.alpha = 1.0;

	
	//	nodexyz = pNode->obj_cent;
	//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

		/* �������ֻ��Ƶ�ľ�γ��,���ǵ������п��,Ϊ�������屣��������,���õ�����Ƭ����1/4����,�߶�Ϊƽ���߶� */
		/* δ������ת */
		{
			geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
			geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
			geoPt.height = pNode->height_ct;
		}

	//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
		/* ���ݾ�γ�߼�����Ļ���� */
		getOuterViewPtByGeo(pScene, geoPt, &screenPt);


		/* �ر���Ȳ���,��ֹ���ֱ��ڵ� */
		glDisableEx(GL_DEPTH_TEST);
	//	glDisableEx(GL_CULL_FACE);
		/* ����Ļ�ϻ�����Ƭ�㼶��������� */

		sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
		textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

	//	glEnableEx(GL_CULL_FACE);	
		glEnableEx(GL_DEPTH_TEST);
	}
#endif

	/* ��ͼ�������л��ƵĽڵ��������1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* ��¼�ڵ���һ�λ��Ƶ�֡���� */
    pNode->lastDrawNumber = pHandle->drawnumber;
#endif

	return 0;
}


/*
���ܣ���Ƭ����ģʽ�»��ƺ���(OpenGL ES)

���룺
	pHandle              ��ͼ������ָ��
	pNode                ��Ƭ�ڵ�ָ��
	pScene               ��Ⱦ����ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��������
	-1  ��Ƭ�ڵ�����Ч
	-2  ��ͼ��������Ч
	-3  ��Ƭ�ڵ�Ķ�άӰ��������ʧ��
*/
static f_int32_t mapNodeRenderImgEs(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �ж��������Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    return -2;
	}
	
	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	    case eScene_normal:
	        count = index_count;
	        pIndex = gindices;
	        pTex = gTextureBuf;
			break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
 	    case eScene_eyebird:
 	        count = index_count_eyebird;
 	        pIndex = indices_eyebird;
 	        pTex = gTextureBuf_eyebird;
 			break;	
		/* Ĭ��Ϊ����ģʽ */
		default:
			count = index_count;
			pIndex = gindices;
			pTex = gTextureBuf;
			break;
	}
	
	/* �������л�����Ƭ�ڵ����С�㼶 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* ʹ������Ԫ0,�ر�һά����,������ά���� */
    glActiveTextureARB(GL_TEXTURE0);
    glEnableEx(GL_TEXTURE_2D);
    
	/* ��һ����������Ƭ�ڵ�Ķ�άӰ������,֮ǰ�Ѿ����������ظ����� */
	if ( (0 == (pNode->texid)) && (pNode->imgdata != NULL))
	{
		/* ���ùҽӺ���,����Ƭ��Ӱ��������������һ����ά���� */
		pNode->texid = pNode->pSubTree->img2tex(pNode->pSubTree->imgparam, pNode->imgdata);
		/* ��ͼ�������еĴ����������������1 */
        pHandle->createTexNumber++;
	}

	/* ������Ƭ�ڵ�Ķ�άӰ������ʧ��,ֱ�ӷ��� */
	if (0 >= (pNode->texid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -3;
	}

	/* ��άʸ��������ʱ��ʹ��,ע�͵� */
#if 0
	/* ��һ����������Ƭ�ڵ�Ķ�άʸ������,֮ǰ�Ѿ����������ظ����� */
	if ( (0 == (pNode->vtexid)) && (pNode->vqtdata != NULL))
	{
		/* ���ùҽӺ���,����Ƭ��ʸ��������������һ����ά���� */
		pNode->vtexid = pNode->pSubTree->vqt2tex(pNode->pSubTree->vqtparam, pNode->vqtdata);
		/* ��ͼ�������еĴ����������������1 */
        pHandle->createTexNumber++;
	}

	/* ������Ƭ�ڵ�Ķ�άʸ������ʧ��,ֱ�ӷ��� */
	if (0 >= (pNode->vtexid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -1;
	}
#endif

	/* Ϊ��ʡ֡��,������ͼ������������id,����Ӱ������id,�����滻������ķ�ʽ������������ */
#if 0
	/* ��һ����������Ƭ�ڵ�Ķ�άclr����,֮ǰ�Ѿ����������ظ����� */
	if ( (0 == (pNode->clrtexid)) && (pNode->clrdata != NULL))
	{
		pNode->clrtexid = pNode->pSubTree->clr2tex(pNode->pSubTree->clrparam, pNode->clrdata);

        pHandle->createTexNumber++;
	}

	if (0 == (pNode->clrtexid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -1;
	}

	/* ��һ����������Ƭ�ڵ�Ķ�άhil����,֮ǰ�Ѿ����������ظ����� */
	if ( (0 == (pNode->hiltexid)) && (pNode->hildata != NULL))
	{
		pNode->hiltexid = pNode->pSubTree->hil2tex(pNode->pSubTree->hilparam, pNode->hildata);

        pHandle->createTexNumber++;
	}

	if (0 == (pNode->hiltexid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -1;
	}

#endif


	/* ���ݳ������ƾ���еĵ�ͼ��ʾģʽ�����ڲ���ͼ�����ģʽ */
	setLayDisplay(pScene);

	/* ��������ĵ�0ͼ�����ģʽ,�жϵ�ǰ֡ʹ�õĵ�ͼ�����Ƿ�����һ֡ʹ�õĲ�һ��,�����ͬ����а������л�����Ĳ��� */
	bindAndSubRenderTexID(pNode, 0);
		
	/* ��������Ԫ0���������ģʽΪ����GL_MODULATE,�����������RGBA=����Դ��RGBA*Ƭ�ε�RGBA */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,������,����������� */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* �ر�����Ԫ1��һά����Ͷ�ά����ʹ�ܿ��� */
		glActiveTextureARB(GL_TEXTURE1);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* ��ʱ����Ҫ���ӵڶ���ͼ��,ע�͸ù���,��JM7200��֧�����ؼ����ϵ�������� */
#if 0
	/* ���ӵڶ���ͼ��(ʸ���������Ӱ����)begin */

	/* ʹ������Ԫ2 */
	glActiveTexture(GL_TEXTURE2);
	/* ��������Ԫ0���������ģʽΪ����GL_DECAL,�����������RGB=����Դ��RGB*(1-����Դ��A)+Ƭ�ε�RGB*����Դ��A,���������A=Ƭ�ε�A */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* �ر�һά����,������ά���� */
	//glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);

	/* ��������ĵ�1ͼ�����ģʽ,�жϵ�ǰ֡ʹ�õĵ�ͼ�����Ƿ�����һ֡ʹ�õĲ�һ��,�����ͬ����а������л�����Ĳ��� */
	bindAndSubRenderTexID(pNode, 1);

	/* ���ӵڶ���ͼ��end */
#endif

	/* ʹ������Ԫ0 */
	glActiveTextureARB(GL_TEXTURE0);
	
	/* �ָ������ջΪģ����ͼ���� */
	glMatrixMode(GL_MODELVIEW);	
	/* ��ԭ����ģ����ͼ�������ѹջ���� */
	glPushMatrix();
#ifndef VERTEX_USE_FLOAT
	/* ʹ�����εĶ������ݣ������ƶ�ģ����ͼ�����ƶ���ƫ����ʼ�� */
	/* ʹ�����ͻ������Ƭ����ĺô�������float���Ȳ��������·��й����е��λ���ֶ������ĳ����ξ��Ⱦ͹��� */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0],pNode->point_vertex_xyz[1],pNode->point_vertex_xyz[2],pNode->point_vertex_xyz[3]);
#endif	
	/* ������ɫΪ��ɫ,͸����Ϊ��ȫ��͸�� */
	glColor4f(1.0f,1.0f,1.0f,1.0f);	

	{
		GLint prog = 0;
		int vLoc, tLoc, tLoc1;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation((GLuint)prog, "a_position");
		tLoc = glGetAttribLocation((GLuint)prog, "a_texCoord0");
		tLoc1 = glGetAttribLocation((GLuint)prog, "a_texCoord1");

		/*������Ƭ�������ӽǻ���VBO(��������/����߶�)�͸����ӽǻ��Ƶ�VBO(�������꣬��4���ǵ������)��ÿ����Ƭ��VBO�е����ݲ�һ��*/
		qtmapNodeCreateVboEs(pNode);
		/*������Ƭ�������ӽǻ���VBO(��������/������������)�͸����ӽǻ��Ƶ�VBO(��������/������������)��������Ƭ����ЩVBO�е����ݾ�һ��*/
		genConstVBOs(&ibo, &tvbo, &tvbo_overlook, &ibo_overlook);


		if ( (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
		{
			/*�����ӽǣ���ˮƽ�澯ʱ��ÿ����Ƭֻ����Ƴ�2�������μ���*/
			//glDisableEx(GL_CULL_FACE);
			/*���ö�������*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif
			/*���ö�����������*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, tvbo_overlook);
			glVertexAttribPointer(tLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
			/*���ö��������������л���*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_overlook);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(vLoc);
			glDisableVertexAttribArray(tLoc);				
			
		}
		else 
		{
			//glDisableEx(GL_CULL_FACE);    // fix the warning function under overlook viewmode

			/*���������ÿ����Ƭ��33*33��������Ƴ�����������*/
			glActiveTextureARB(GL_TEXTURE0);
			/*���ö�������*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif
			/*���ö�������0����*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, tvbo);
			glVertexAttribPointer(tLoc, 2, GL_FLOAT, 0, 0, 0);
			
			if (pScene->is_need_terwarning)/* ��Ҫ���θ澯 */
			{
				/*���ö�������1����*/
				glEnableVertexAttribArray(tLoc1);
				glActiveTextureARB(GL_TEXTURE1);
				glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
				glVertexAttribPointer(tLoc1, 1, GL_FLOAT, 0, 0, 0);
			}
			/*���ö��������������л���*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(vLoc);
			glDisableVertexAttribArray(tLoc);
			if (pScene->is_need_terwarning){
				glDisableVertexAttribArray(tLoc1);
			}
			//glFinish();
		}
	}

	/* �����Ҫ���θ澯,��ָ�����1��״̬ */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

#if 0
	/* �ָ�����2��״̬ */
	glActiveTexture(GL_TEXTURE2);					        // ��������Ԫ1
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);					// ��������Ԫ0

#endif

	glDisableEx(GL_TEXTURE_2D);

	
#if 0
	/* ����ģʽ�»�����Ƭ���ߵ���ʾ�б�ӥ��ģʽ������ */
    if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
    {	
		f_int32_t i, j, index0, index1;
		int prog = 0;
		GLint aPositionLoc = -1;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		aPositionLoc = glGetAttribLocation(prog, "a_position");

		glEnableEx(GL_LINE_SMOOTH);

		glColor3f(0.5f, 0.8f, 0.8f);
		glLineWidth(2.0f);

		if ( (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&& (pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
		{			
			glEnableVertexAttribArray(aPositionLoc);

			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(aPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(aPositionLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif

			glDrawArrays(GL_LINE_LOOP, 0, 4);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(aPositionLoc);
		}
		else
		{
#ifdef VERTEX_USE_FLOAT
			f_float32_t vertexbuf[VTXCOUNT * 4 * 3] = { 0 };
			for (i = 0; i<4; i++)
			{				
				for (j = 0; j < VTXCOUNT; j++)
				{
					switch (i)
					{
					case 0:
						index0 = j;
						break;
					case 1:
						index0 = (j + 1) * (VTXCOUNT + 1) - 1;
						break;
					case 2:
						index0 = -j + (VTXCOUNT + 1)*(VTXCOUNT + 1)-1;
						break;
					case 3:
						index0 = (VTXCOUNT - j) * (VTXCOUNT + 1);
						break;
					}
					index0 *= 3;
					index1 = 3 * (j + i * VTXCOUNT);
					vertexbuf[index1 + 0] = pNode->fvertex[index0];
					vertexbuf[index1 + 1] = pNode->fvertex[index0 + 1];
					vertexbuf[index1 + 2] = pNode->fvertex[index0 + 2];
					//vertexbuf[index1 + 2] = pNode->fvertex[index0 + 2] + 10.0f;
				}

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glEnableVertexAttribArray(aPositionLoc);
				glVertexAttribPointer(aPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertexbuf);
				glDrawArrays(GL_LINE_LOOP, 0, VTXCOUNT * 4);
				glDisableVertexAttribArray(aPositionLoc);
#else
			f_int32_t vertexbuf[VTXCOUNT * 4 * 3] = { 0 };
			for (i = 0; i<4; i++)
			{				
				for (j = 0; j < VTXCOUNT; j++)
				{
					switch (i)
					{
					case 0:
						index0 = j;
						break;
					case 1:
						index0 = (j + 1) * (VTXCOUNT + 1) - 1;
						break;
					case 2:
						index0 = -j + (VTXCOUNT + 1)*(VTXCOUNT + 1)-1;
						break;
					case 3:
						index0 = (VTXCOUNT - j) * (VTXCOUNT + 1);
						break;
					}
					index0 *= 3;
					index1 = 3 * (j + i * VTXCOUNT);
					vertexbuf[index1 + 0] = pNode->fvertex_part[index0];
					vertexbuf[index1 + 1] = pNode->fvertex_part[index0 + 1];
					vertexbuf[index1 + 2] = pNode->fvertex_part[index0 + 2];
					//vertexbuf[index1 + 2] = pNode->fvertex[index0 + 2] + 10.0f;
				}

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glEnableVertexAttribArray(aPositionLoc);
				glVertexAttribPointer(aPositionLoc, 3, GL_INT, GL_FALSE, 0, vertexbuf);
				glDrawArrays(GL_LINE_STRIP, 0, VTXCOUNT * 4);
				glDisableVertexAttribArray(aPositionLoc);
#endif
			}
		}

		glDisableEx(GL_LINE_SMOOTH);
		
    }
#endif

	/* ��ջ�лָ�ԭ����ģ����ͼ���� */
	glPopMatrix();

#if 0
{
	PT_3D nodexyz = {0};
	sColor4f color1={0.0};
	f_char_t timeInfo[128] = {0};
	Geo_Pt_D geoPt;
	PT_2I screenPt;
	
	/* ������Ƭ�㼶������������ֻ��Ƶ���ɫ,ʹ�ú�ɫ��ȫ��͸�� */
	color1.red = 1.0;
	color1.alpha = 1.0;

	
//	nodexyz = pNode->obj_cent;
//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

	/* �������ֻ��Ƶ�ľ�γ��,���ǵ������п��,Ϊ�������屣��������,���õ�����Ƭ����1/4����,�߶�Ϊƽ���߶� */
	/* δ������ת */
	geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
	geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
	geoPt.height = pNode->height_ct;

//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
	/* ���ݾ�γ�߼�����Ļ���� */
	getOuterViewPtByGeo(pScene, geoPt, &screenPt);


	/* �ر���Ȳ���,��ֹ���ֱ��ڵ� */
	glDisableEx(GL_DEPTH_TEST);
//	glDisableEx(GL_CULL_FACE);
	/* ����Ļ�ϻ�����Ƭ�㼶��������� */
	sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
	textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

//	glEnableEx(GL_CULL_FACE);	
	glEnableEx(GL_DEPTH_TEST);
}
#endif

#endif

	/* ��ͼ�������л��ƵĽڵ��������1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* ��¼�ڵ���һ�λ��Ƶ�֡���� */
    pNode->lastDrawNumber = pHandle->drawnumber;

	return 0;
}

/*
���ܣ���ƬSVSģʽ�»��ƺ���

���룺
	pHandle              ��ͼ������ָ��
	pNode                ��Ƭ�ڵ�ָ��
	pScene               ��Ⱦ����ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��������
	-1  ��Ƭ�ڵ�����Ч
	-2  ��ͼ��������Ч
*/
static f_int32_t mapNodeRenderImgSVS_EX(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �ж��������Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* Ĭ��Ϊ����ģʽ */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* �������л�����Ƭ�ڵ����С�㼶 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;


#ifdef _JM7200_
	/* JM7200���ֻ֧�ֶ������������淽����֧�ַ������͸澯�����ͬʱ��ʾ��֧�ֶ����������ʾ�б����ַ�ʽ */

	/* ʹ������Ԫ0,�ر�һά����,������ά���� */
	glActiveTexture(GL_TEXTURE0);
	glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);

	/* JM7200�����֧�ֶ�ά�������·���������͸澯������ͬʱ����*/
	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,�󶨸澯����(һά),����������� */
		RenderTerrainWarningStripeColor(pScene);
	}
	else
	{

		/* ����Ҫ���θ澯,�󶨷���������(��ά),�ر�����Ԫ1��һά����,������ά���� */
		glActiveTexture(GL_TEXTURE1);
		GetGridTextureID();
		glDisableEx(GL_TEXTURE_1D);
		glEnableEx(GL_TEXTURE_2D);
		/* ��������Ԫ1���������ģʽΪ����GL_DECAL,�����������RGB=����Դ��RGB*����Դ��A + Ƭ�ε�RGB*(1-����Դ��A),���������RGB=Ƭ�ε�A */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glDisableEx(GL_BLEND);
	}

	/* ʹ������Ԫ0 */
	glActiveTexture(GL_TEXTURE0);

	/* ���ǿ��ʹ����ʾ�б�,��ʹ����ʾ�б�ķ�ʽ���� */
	/* �������ͨ����������ķ�ʽ���� */
	if((useArray == 0))	
	{
		/* JM7200��glVertexPointer��GL_INT��֧�֣������������ľֲ����꣬�ø��������ꣻ��ʾ�б�ģʽ�����õ����������ľֲ����꣬����任����*/
		CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);

#if 1
		if(pScene->is_need_terwarning)
		{
			/* ��Ҫ���θ澯������SVS���θ澯��ʾ�б�*/	
			qtmapNodeCreateSVSList(pNode);
			/* ��Ҫ���θ澯��ʹ��SVS���θ澯��ʾ�б�*/
			if((pNode->tileSVSList) != 0)
				glCallList(pNode->tileSVSList);
		}else{
			/* �����������SVS��������ʾ�б�*/	
			qtmapNodeCreateSVSGridList(pNode);
			/* �������ʹ��SVS��������ʾ�б�*/	
			if((pNode->tileSVSGridList) != 0)
				glCallList(pNode->tileSVSGridList);
		}
#endif

	}
	else
	{	
		/* ������Ƭ�Ķ������� */
		glEnableClientState(GL_VERTEX_ARRAY);	
		/* ʹ�ø����͵Ķ��� */
		glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);

		/* ������Ƭ��������0����ɫ���� */
		glEnableClientState(GL_COLOR_ARRAY);
		//glColorPointer(3,GL_UNSIGNED_BYTE,0,pNode->m_pColor);
		glColorPointer(3,GL_FLOAT,0,pNode->m_pColor);

		if(pScene->is_need_terwarning)	
		{
			/* �����Ҫ���θ澯,��������Ƭ��������1��������������Ϊ�澯���� */
			RenderTerrainWarningStripeColor2(pScene, pNode);
		}
		else
		{
			/* �������Ҫ���θ澯,��������Ƭ��������1��������������Ϊ���������� */
			glClientActiveTexture(GL_TEXTURE1);                
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			//SVSģʽ�·�������������������������
			glTexCoordPointer(2, GL_FLOAT, 0, pNode->m_pVertexTex);
		}

		/* ������Ƭ����,�������������ķ�ʽ���� */
		glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);

		/* �ر��������顢�����������ɫ�����ʹ��,��Enable��Ӧ */
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);	

	}

	/* �ָ�����1��״̬ */
	if(pScene->is_need_terwarning)	
	{
		RenderTerrainWarningStripeColor3();
	}
	else{
		glActiveTexture(GL_TEXTURE1);
		//glDisableEx(GL_TEXTURE_1D);
		glDisableEx(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

#else
	/* ���淽��֧�ַ������͸澯����ͬʱ��ʾ��ʹ��������������֧�ֶ������鷽ʽ����ʾ�б�ʽ������ */

	/* ʹ������Ԫ0,�ر�һά����,�رն�ά���� */
	glActiveTexture(GL_TEXTURE0);
	glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);
    
	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,������,����������� */
		RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* �ر�����Ԫ1��һά����Ͷ�ά����ʹ�ܿ��� */
		glActiveTexture(GL_TEXTURE1);
		glDisableEx(GL_TEXTURE_1D);
		glDisableEx(GL_TEXTURE_2D);
	}
	
	/* ʹ������Ԫ2 */
	glActiveTexture(GL_TEXTURE2);
	/* ��������Ԫ2���������ģʽΪ����GL_DECAL,�����������RGB=����Դ��RGB*����Դ��A + Ƭ�ε�RGB*(1-����Դ��A),���������RGB=Ƭ�ε�A */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
	/* �ر�һά����,������ά���� */
	//glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
	/* ��������Ԫ2������id,������������ */
	GetGridTextureID();


	
	/* ʹ������Ԫ0 */
	glActiveTexture(GL_TEXTURE0);

	/* �ָ������ջΪģ����ͼ���� */
	glMatrixMode(GL_MODELVIEW);	
	/* ��ԭ����ģ����ͼ�������ѹջ���� */
	glPushMatrix();
	
	/* Ϊ��ʹ�����ͻ������Ƭ���㣬����ģ����ͼ�������û�׼��xyz������ϵ�� */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);

	
	//glColor4f(1.0f,1.0f,1.0f, 1.0f);	
		
	/* ������Ƭ�Ķ������� */
	glEnableClientState(GL_VERTEX_ARRAY);	
	/* ʹ�ø����͵Ķ��� */
	//glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	
	/* ʹ�����͵Ķ��� */
	glVertexPointer(3, GL_INT, 0, pNode->fvertex_part);		

	/* ������Ƭ��������0����ɫ���� */
	glClientActiveTexture(GL_TEXTURE0); 
	glEnableClientState(GL_COLOR_ARRAY);
	//glColorPointer(3,GL_UNSIGNED_BYTE,0,pNode->m_pColor);
	glColorPointer(3,GL_FLOAT,0,pNode->m_pColor);
	
	/* �����Ҫ���θ澯,��������Ƭ��������1�������������� */
	if(pScene->is_need_terwarning)	
	{
		RenderTerrainWarningStripeColor2(pScene, pNode);
	}

	/* ������Ƭ��������2�������������� */
	glClientActiveTexture(GL_TEXTURE2);                
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, pNode->m_pVertexTex);


	/* ������Ƭ����,�������εķ�ʽ����,����̫��,�÷�ʽ���� */
	//glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, pIndex);
	/* ������Ƭ����,�������������ķ�ʽ���� */
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);

	/* �ر��������顢�����������ɫ�����ʹ��,��Enable��Ӧ */
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);	
	glDisableClientState(GL_COLOR_ARRAY);

//	glFinish();

	/* �����Ҫ���θ澯,��ָ�����1��״̬ */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

	/* �ָ�����2��״̬ */
	glActiveTexture(GL_TEXTURE2);					        
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);					


	glDisableEx(GL_TEXTURE_2D);

#endif

#if 0
	/* ����ģʽ�»�����Ƭ���ߵ���ʾ�б�ӥ��ģʽ������ */
	if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
	{	
		/* ������Ƭ���ߵ���ɫ */
		glColor3f(0.5f, 0.8f, 0.8f);
		/* ��������ģʽ����Ƭ���ߵ���ʾ�б� */
		qtmapNodeCreateEdgeList(pNode);

		/* ������ƽ�� */
		glEnableEx(GL_LINE_SMOOTH);
		/* �ر���Ȳ���,��ֹ�߱��ڵ� */
		glDisableEx(GL_DEPTH_TEST);

		/* ��������ģʽ����Ƭ������ʾ�б���� */
		if((pNode->edgeList) != 0)
			glCallList(pNode->edgeList);


		glDisableEx(GL_LINE_SMOOTH);
		glEnableEx(GL_DEPTH_TEST);

	}
#endif

	/* ��ջ�лָ�ԭ����ģ����ͼ���� */
	glPopMatrix();

#if 0
	{
		PT_3D nodexyz = {0};
		sColor4f color1={0.0};
		f_char_t timeInfo[128] = {0};
		Geo_Pt_D geoPt;
		PT_2I screenPt;

		/* ������Ƭ�㼶������������ֻ��Ƶ���ɫ,ʹ�ú�ɫ��ȫ��͸�� */
		color1.red = 1.0;
		color1.alpha = 1.0;


		//	nodexyz = pNode->obj_cent;
		//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

		/* �������ֻ��Ƶ�ľ�γ��,���ǵ������п��,Ϊ�������屣��������,���õ�����Ƭ����1/4����,�߶�Ϊƽ���߶� */
		/* δ������ת */
		geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
		geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
		geoPt.height = pNode->height_ct;

		//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
		/* ���ݾ�γ�߼�����Ļ���� */
		getOuterViewPtByGeo(pScene, geoPt, &screenPt);


		/* �ر���Ȳ���,��ֹ���ֱ��ڵ� */
		glDisableEx(GL_DEPTH_TEST);
		//	glDisableEx(GL_CULL_FACE);
		/* ����Ļ�ϻ�����Ƭ�㼶��������� */
		sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
		textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

		//	glEnableEx(GL_CULL_FACE);	
		glEnableEx(GL_DEPTH_TEST);
	}
#endif

	/* ��ͼ�������л��ƵĽڵ��������1 */
	pHandle->drawnnodnum++;
	//printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* ��¼�ڵ���һ�λ��Ƶ�֡���� */
	pNode->lastDrawNumber = pHandle->drawnumber;
#endif
	return 0;
}


static f_int32_t mapNodeRenderImgSvsEs(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;

	/* �ж��������Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* Ĭ��Ϊ����ģʽ */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* �������л�����Ƭ�ڵ����С�㼶 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* ʹ������Ԫ0,������ά�����󶨷���������(��ά) */
	glActiveTextureARB(GL_TEXTURE0);
	glEnableEx(GL_TEXTURE_2D);
	GetGridTextureID();

	/* ��������Ԫ0���������ģʽΪ����GL_DECAL,�����������RGB=����Դ��RGB*(1-����Դ��A)+Ƭ�ε�RGB*����Դ��A,���������A=Ƭ�ε�A */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	/* �ж��Ƿ���Ҫ���θ澯 */
	if(pScene->is_need_terwarning)
	{
		/* ��Ҫ���θ澯,����������Ԫ1,���ø���OpenGL״̬,������,����������� */
		RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* �ر�����Ԫ1�Ķ�ά����ʹ�ܿ��� */
		glActiveTextureARB(GL_TEXTURE1);
		glDisableEx(GL_TEXTURE_2D);
	}

	/* ʹ������Ԫ0 */
	glActiveTextureARB(GL_TEXTURE0);

	/* �ָ������ջΪģ����ͼ���� */
	glMatrixMode(GL_MODELVIEW);	
	/* ��ԭ����ģ����ͼ�������ѹջ���� */
	glPushMatrix();

#ifndef VERTEX_USE_FLOAT
	/* ʹ�����εĶ������ݣ������ƶ�ģ����ͼ�����ƶ���ƫ����ʼ�� */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0],pNode->point_vertex_xyz[1],pNode->point_vertex_xyz[2],pNode->point_vertex_xyz[3]);
#endif	

	/* ������ɫΪ��ɫ,͸����Ϊ��ȫ��͸�� */
	//glColor4f(1.0f,1.0f,1.0f,1.0f);	

	{
		GLint prog = 0;
		int vLoc, tLoc, tLoc1, nLoc, cLoc;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation((GLuint)prog, "a_position");
		tLoc = glGetAttribLocation((GLuint)prog, "a_texCoord0");
		tLoc1 = glGetAttribLocation((GLuint)prog, "a_texCoord1");
		nLoc = glGetAttribLocation((GLuint)prog, "a_normal");
		cLoc = glGetAttribLocation((GLuint)prog, "a_color");

		/*������Ƭ�Ķ������VBO(��������/����߶�/������ɫ/���㷨��/���㷽��������)��SVSģʽ�������ӽǶ��ǻ���33*33�����㣬ÿ����Ƭ��VBO�е����ݲ�һ��*/
		qtmapNodeCreateSvsVboEs(pNode);
		/*������Ƭ�Ķ������VBO(��������)��������Ƭ����ЩVBO�е����ݾ�һ��*/
		genSvsConstVBOs(&ibo);

		{
			//glDisableEx(GL_CULL_FACE);    // fix the warning function under overlook viewmode

			/*ÿ����Ƭ��33*33��������Ƴ�����������*/
			glActiveTextureARB(GL_TEXTURE0);
			/*���ö������꣬shader�е�a_position��4ά�ģ���3άҲ���ԣ���ɫ�����������������ƣ�δ����ά�ȱ���Ĭ��ֵ*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif
			/*���ö�����ɫ,���Է���GL_UNSIGNED_BYTE��֧��*/
			glEnableVertexAttribArray(cLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileColorVBO);
			glVertexAttribPointer(cLoc, 3, GL_FLOAT, 0, 0, 0);
			/*���ö�������0���꣬����������*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileGridTexVBO);
			glVertexAttribPointer(tLoc, 2, GL_FLOAT, 0, 0, 0);

			if (pScene->is_need_terwarning)/* ��Ҫ���θ澯 */
			{
				/*���ö�������1���꣬ˮƽ�澯����*/
				glEnableVertexAttribArray(tLoc1);
				glActiveTextureARB(GL_TEXTURE1);
				glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
				glVertexAttribPointer(tLoc1, 1, GL_FLOAT, 0, 0, 0);
			}
			/*���ö������������������ж���*/
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, 0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(vLoc);
			glDisableVertexAttribArray(cLoc);
			glDisableVertexAttribArray(tLoc);
			if (pScene->is_need_terwarning){
				glDisableVertexAttribArray(tLoc1);
			}
			//glFinish();
		}
	}

	/* �����Ҫ���θ澯,��ָ�����1��״̬ */
	if(pScene->is_need_terwarning)
		RenderTerrainWarningStripeColor3();

	glActiveTextureARB(GL_TEXTURE0);
	glDisableEx(GL_TEXTURE_2D);


#if 0
	/* ����ģʽ�»�����Ƭ���ߵ���ʾ�б�ӥ��ģʽ������ */
	if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
	{	
		f_int32_t i, j, index0, index1;
		int prog = 0;
		GLint aPositionLoc = -1;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		aPositionLoc = glGetAttribLocation(prog, "a_position");

		glEnableEx(GL_LINE_SMOOTH);

		glColor3f(0.5f, 0.8f, 0.8f);
		glLineWidth(2.0f);

		if ( (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&& (pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
		{			
			glEnableVertexAttribArray(aPositionLoc);

			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
			glVertexAttribPointer(aPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glDrawArrays(GL_LINE_LOOP, 0, 4);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDisableVertexAttribArray(aPositionLoc);
		}
		else
		{
			f_float32_t vertexbuf[VTXCOUNT * 4 * 3] = { 0 };
			for (i = 0; i<4; i++)
			{				
				for (j = 0; j < VTXCOUNT; j++)
				{
					switch (i)
					{
					case 0:
						index0 = j;
						break;
					case 1:
						index0 = (j + 1) * (VTXCOUNT + 1) - 1;
						break;
					case 2:
						index0 = -j + (VTXCOUNT + 1)*(VTXCOUNT + 1)-1;
						break;
					case 3:
						index0 = (VTXCOUNT - j) * (VTXCOUNT + 1);
						break;
					}
					index0 *= 3;
					index1 = 3 * (j + i * VTXCOUNT);
					vertexbuf[index1 + 0] = pNode->fvertex[index0];
					vertexbuf[index1 + 1] = pNode->fvertex[index0 + 1];
					vertexbuf[index1 + 2] = pNode->fvertex[index0 + 2];
					//vertexbuf[index1 + 2] = pNode->fvertex[index0 + 2] + 10.0f;
				}

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glEnableVertexAttribArray(aPositionLoc);

				glVertexAttribPointer(aPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, vertexbuf);

				glDrawArrays(GL_LINE_LOOP, 0, VTXCOUNT * 4);

				glDisableVertexAttribArray(aPositionLoc);
			}
		}

		glDisableEx(GL_LINE_SMOOTH);

	}
#endif

	/* ��ջ�лָ�ԭ����ģ����ͼ���� */
	glPopMatrix();

#if 0
	{
		PT_3D nodexyz = {0};
		sColor4f color1={0.0};
		f_char_t timeInfo[128] = {0};
		Geo_Pt_D geoPt;
		PT_2I screenPt;

		/* ������Ƭ�㼶������������ֻ��Ƶ���ɫ,ʹ�ú�ɫ��ȫ��͸�� */
		color1.red = 1.0;
		color1.alpha = 1.0;


		//	nodexyz = pNode->obj_cent;
		//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

		/* �������ֻ��Ƶ�ľ�γ��,���ǵ������п��,Ϊ�������屣��������,���õ�����Ƭ����1/4����,�߶�Ϊƽ���߶� */
		/* δ������ת */
		geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
		geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
		geoPt.height = pNode->height_ct;

		//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
		/* ���ݾ�γ�߼�����Ļ���� */
		getOuterViewPtByGeo(pScene, geoPt, &screenPt);


		/* �ر���Ȳ���,��ֹ���ֱ��ڵ� */
		glDisableEx(GL_DEPTH_TEST);
		//	glDisableEx(GL_CULL_FACE);
		/* ����Ļ�ϻ�����Ƭ�㼶��������� */
		sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
		textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

		//	glEnableEx(GL_CULL_FACE);	
		glEnableEx(GL_DEPTH_TEST);
	}
#endif

#endif

	/* ��ͼ�������л��ƵĽڵ��������1 */
	pHandle->drawnnodnum++;
	//printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* ��¼�ڵ���һ�λ��Ƶ�֡���� */
	pNode->lastDrawNumber = pHandle->drawnumber;

	return 0;
}


/*
���ܣ����ݾ�γ�ȣ���ȡ�ɻ�λ�������е�ͼ���������������Ƭ�㼶����Ƭ�ڵ�ָ��(��ϸ�ж�,���õݹ�ķ�ʽ)
���룺
	pHandle     ��ͼ���������
	pNode       ��ǰ��Ƭ�ڵ�ָ��
	pos_lon     �ɻ�����
	pos_lat     �ɻ�γ��
	pScene      �������ƾ��

���������
	��

�����
    ��

����ֵ��
    tpNode ��Ч����Ƭ�ڵ�ָ��
    NULL   ��Ч����Ƭ�ڵ�ָ��,���ɻ�λ�ò�����pNodeΪͷ������Ƭ����������
*/
static sQTMAPNODE * getTreeLevel(sMAPHANDLE *pHandle, sQTMAPNODE *pNode, f_float32_t pos_lon, f_float32_t pos_lat, sGLRENDERSCENE *pScene)
{
    f_int32_t ret = -1, i = 0;
	sQTMAPNODE *tpNode = NULL;
    
	/* �ж���Ƭ�ڵ��Ƿ�Ϊ�ռ��ڵ��keyֵ�Ƿ���ȷ */
	if (!ISMAPDATANODE(pNode))
	{
		DEBUG_PRINT("call getTreeLevelOfPlanepos with error node.");
		return NULL;
	}

	/* �жϵ�ͼ����������Ƿ�Ϊ�ռ������keyֵ�Ƿ���ȷ */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		DEBUG_PRINT("call getTreeLevelOfPlanepos with error pHandle.");
		return NULL;
	}
	
	/* 1.�ж��������λ��(���ɻ�λ��)�ľ�γ���Ƿ�����Ƭ�ڵ�(pNode)�����ķ�Χ�� 1-��,0-���� */
	if(!isCaminNode(pScene, pNode))
    {
		/* 1.1 �������1�����㣬���һ���ж���Ƭ�ڵ�(pNode)�Ƿ���������Ӿ�����ӷ�Χ�� 0-��,-1-����  */
		ret = isNodeInFrustum(pScene->camctrl_param.view_near, pScene->camctrl_param.view_far ,&(pScene->frustum), pNode);
		/* �������1.1������,�򷵻�NULL,�����ж���һ������  */
		if(0 != ret)
		{
			return NULL; //NODE_NOT_IN_VIEW;
		}	
		
		/* 1.2 ����ģʽ�²����ж��Ƿ��ڱ���,�����ڵͲ㼶������Ƭ̫���¶�� ����*/
		if(eROM_outroam == pScene->mdctrl_cmd.rom_mode)
		{
		    /* 1.3 �ж���Ƭ�ڵ��Ƿ�������Ӿ��忴���ĵ���ı���,����ڱ����򲻻���,ֱ�ӷ���NULL,�����ж���һ������ */
		    if(isNodeAtEarthBack(pScene, pNode))
		    {
			    return NULL; //NODE_AT_EARTH_BACK;	
		    }
		}
    }

    /* �������1�������ж�ͨ����Ȼ���ж��Ƿ���ڻ�������㼶���������㼶�󲻻������ж�����Ƭ */
	if(pNode->level < sMaxLevel)
	{
	    /* ���δ���ﵽ�����㼶�����һ���ж��Ƿ���Ҫ��������Ƭ 0-��Ҫ���� ����-���贴��*/
	    ret = isNeedCreateChildren(pNode, NULL, pScene->camParam.m_geoptEye, pScene->camParam.m_ptEye, pScene->camctrl_param.view_type);  //����0��ʾ��Ҫ���·�
	}
	else
	{
		/* ����ѵ������㼶,�����Ƭ�ڵ����Ҫ�ҵ���Ƭ�ڵ�,���ýڵ㷵�� */
		ret = -1;
	}
	
	/* retΪ0��ʾ��Ҫ�ݹ�����Ƭ,����ֵ��ֱ�ӷ��ص�ǰ��Ƭ�ڵ� */
	if(0 == ret)
	{	
		/* ������ǰ��Ƭ�ڵ�ĺ��ӽڵ�,����������ΪTRUE,����ֻ�����ڵ㵫�����ؽڵ����� */
		if(qtmapnodeCreateChildren(pHandle, pNode, TRUE) > 0)
		{
			/* ��ǰ��Ƭ�ڵ�ĺ��ӽڵ㴴���ɹ�,�����ж�4�����ӽڵ� */
			for(i = 0; i < 4; i++)
			{
				if (ISMAPDATANODE(pNode->children[i]))
	            {
					/* �����ӽڵ���Ч��keyֵ����ȷ,���жϷɻ��ľ�γ���Ƿ��ں��ӽڵ�ľ�γ�ȷ�Χ */
					if((pos_lon >= pNode->children[i]->hstart) && (pos_lon < pNode->children[i]->hstart + pNode->children[i]->hscale) &&
						(pos_lat >= pNode->children[i]->vstart) && (pos_lat < pNode->children[i]->vstart + pNode->children[i]->vscale))
					{
						/* ���ڷ�Χ��,��ݹ��жϸú��ӽڵ�,ֱ���ҵ����������зɻ�λ�����ڵ���߲㼶����Ƭ�ڵ� */
						tpNode = getTreeLevel(pHandle, pNode->children[i], pos_lon, pos_lat, pScene);	
						if(NULL == tpNode)
							tpNode = pNode;
					}
	            }
	            else
	            {
					/* �����ӽڵ���Ч���ӽڵ���Ч��keyֵ����ȷ,�򷵻ص�tpNode���ڵ�ǰ��Ƭ�ڵ� */
					DEBUG_PRINT("call getTreeLevelOfPlanepos with error node.");
					tpNode = pNode;
			    }
			}/* for(i = 0; i < 4; i++) */
		}/* end if(qtmapnodeCreateChildren(pHandle, pNode) > 0) */
		else
		{
			/* ������ǰ��Ƭ�ڵ�ĺ��ӽڵ�ʧ��,�򷵻ص�tpNode���ڵ�ǰ��Ƭ�ڵ� */
		    tpNode = pNode;
		}
	}
	else
	{
		tpNode = pNode;
	}

    return(tpNode);	
}

/*
���ܣ����ݾ�γ�ȣ���ȡ�ɻ�λ���������Ƭ�㼶����Ƭ�ڵ�ָ��(�����ж�)
���룺��ͼ���������������ڵ�ָ�롢�ɻ���γ�ȡ�������Ⱦ���
�������Ƭ�ڵ�ָ��
      NULL����Чָ��
*/
static sQTMAPNODE* getTreeLevelOfPlanepos(sMAPHANDLE *pHandle, sQTMAPNODE *pNode, f_float32_t pos_lon, f_float32_t pos_lat, sGLRENDERSCENE *pScene)
{
	sQTMAPNODE *tNode = NULL;

	if (pNode == NULL)
		return NULL;

	if (!ISMAPDATANODE(pNode))
	{
		DEBUG_PRINT("call getTreeLevelOfPlanepos with error node.");
		return NULL;
	}

	/* �ȳ���ͨ����Ƭ��������Χ�����ж� */
	if((pos_lon >= pNode->hstart) && (pos_lon <= pNode->hstart + pNode->hscale) &&
	   (pos_lat >= pNode->vstart) && (pos_lat <= pNode->vstart + pNode->vscale))
	{
		/* �õݹ�ķ�ʽ������ϸ�ж� */
        tNode = getTreeLevel(pHandle, pNode, pos_lon, pos_lat, pScene);
    }

	return(tNode);
}

//��scene �Ļ�����������ӽڵ�
static f_int32_t drawListAddNode(sQTMAPNODE *pNode, sGLRENDERSCENE *pScene)
{
	/**�ҵ�ͷ���֮��**/
	stList_Head *pstListHead = NULL;
	pstListHead = &(pScene->scene_draw_list);
		
	LIST_ADD(&pNode->stListHeadDrawing, pstListHead);	

	return 0;
}

//���scene ������
f_int32_t drawListDeleteNode(sGLRENDERSCENE *pScene)
{
    stList_Head *pstListHead = NULL;
    /**�����ʼ��**/
	pstListHead = &(pScene->scene_draw_list);
	LIST_INIT(pstListHead);

	return 0;
}	

/*
���ܣ�ѭ�����Ƶ�ǰ֡������Ƭ�ڵ������е����нڵ�,��ͬģʽ���ò�ͬ�Ļ��Ʒ���
���룺
	pHandle     ��ͼ���������
	pScene      �������ƾ��

���������
	��

�����
    ��

����ֵ��
	0  ��ǰ֡��������
    -1 ��ǰ֡������Ƭ�ڵ�����Ϊ��
*/
// ��ȡ��ǰ֡��Ҫ���Ƶ��߿�
#define MAX_PNODE_COUNT 400
char* pNodeNames[MAX_PNODE_COUNT];
int pNodeNameCount = 0;
char** GetTileDrawList(int* tileNum)	// �ⲿ�ӿ�, ��Render��ͨ���ýӿڻ�ȡ��ǰ֡���ڻ��Ƶ��߿��б�
{
	*tileNum = pNodeNameCount;
	return pNodeNames;
}
 void PriTileDrawList()	// ��ӡ�߿���
{
	int tileNum;
	char** tileDrawList = GetTileDrawList(&tileNum);
	int i=0;
	for (i = 0; i < tileNum; i++)
	{
		printf("TileDrawList[%d]: %s\n", i, tileDrawList[i]);
	}
}
static f_int32_t drawListDraw(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene)
{
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	sQTMAPNODE *pNode = NULL;

	pstListHead = &(pScene->scene_draw_list);

	/* �жϵ�ǰ֡����Ƭ�ڵ���������Ƿ�Ϊ�� */
	if (LIST_IS_EMPTY(pstListHead))
	{
		return -1;
	}

	/* ���ӽ�Ϊ������δ���ø߶�Ԥ����SVSģʽʱ,��ʱ���Ƶ��Ǿ���,����������������������� */
	/* ָ������ε����淽��,�������а�˳ʱ��,Ĭ������ʱ��(GL_CCW),���ڱ������ε�����GL_CULL_FACE */
	if((pScene->camctrl_param.view_type == eVM_DEFAULT_VIEW)
		||(pScene->camctrl_param.view_type == eVM_FIXED_VIEW)
		||(pScene->camctrl_param.view_type == eVM_COCKPIT_VIEW)
		||(pScene->camctrl_param.view_type == eVM_FOLLOW_VIEW)
		||(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_inwarn)
		||(pScene->mdctrl_cmd.svs_mode != FALSE))
	{
		glFrontFace(GL_CW);
	}

	/* ������ǰ֡����Ƭ�ڵ���������еĽڵ� */
	pNodeNameCount = 0;
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);
			if(pNode != NULL)
			{
				if(pScene->mdctrl_cmd.close3d_mode == 1)	//������ά���ƵĿ��ؿ���
				{
					continue;
				}


				// gx test for nibiru
				{
					// �洢pNode������
					if (pNodeNameCount < MAX_PNODE_COUNT)
					{
						pNodeNames[pNodeNameCount] = (char*)malloc(64 * sizeof(char));
						if (pNodeNames[pNodeNameCount] != NULL)
						{
							sprintf(pNodeNames[pNodeNameCount], "%d_%d_%d", pNode->level, pNode->xidx, pNode->yidx);
							pNodeNameCount++;
						}
					}
				}
				
				/* ��ǰΪSVSģʽ,����SVSģʽ�Ļ��ƺ��� */
				/* ��ǰΪVecShadeģʽ,����VecShadeģʽ�Ļ��ƺ��� */
				/* ��ǰΪ����ģʽ,��������ģʽ�Ļ��ƺ��� */
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
				if(pScene->mdctrl_cmd.svs_mode == 1)
				{	
					mapNodeRenderImgSvsEs(pHandle, pNode, pScene);
				}
				else if(pScene->mdctrl_cmd.svs_mode == 2)
				{
					mapNodeRenderVecImgEs(pHandle, pNode, pScene);
				}
				else
				{
					mapNodeRenderImgEs(pHandle, pNode, pScene);
				}
#else
				if(pScene->mdctrl_cmd.svs_mode == 1)
				{
					mapNodeRenderImgSVS_EX(pHandle, pNode, pScene);	
				}
				else if(pScene->mdctrl_cmd.svs_mode == 2)
				{
					mapNodeRenderVecImg(pHandle, pNode, pScene);
				}
				else
				{
					mapNodeRenderImg(pHandle, pNode, pScene);	// gx test
				}
#endif
			}
		}
	}

	/* ָ������ε����淽��,�������а���ʱ��,���ڱ������ε�����GL_CULL_FACE */
	glFrontFace(GL_CCW);

	return 0;
}

/*
���ܣ��жϵ�ǰ֡������Ƭ�ڵ��������Ƿ��нڵ�����δ�������,��������֪��������ǰ֡��SwapBuffer,�Ա�����һ֡�Ļ���,
      �����ݼ��������SwapBuffer,�ѵ�ǰ֡��������ʾ;
���룺
	pHandle     ��ͼ���������
	pScene      �������ƾ��

���������
	��

�����
    ��

����ֵ��
    1 �нڵ�����δ�������,��ǰ֡������
	0 ���нڵ����ݾ��������,��ǰ֡��������
    -1 ��ǰ֡������Ƭ�ڵ�����Ϊ��,��ǰ֡������
*/
static f_int32_t DrawOverlookCallback(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene)
{
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	sQTMAPNODE *pNode = NULL;
	int node_level = 0;
	int node_lastdraw = 0;

	pstListHead = &(pScene->scene_draw_list);

	/* �жϵ�ǰ֡����Ƭ�ڵ���������Ƿ�Ϊ�� */
	if (LIST_IS_EMPTY(pstListHead))
	{
		return -1;
	}

	/* ������ǰ֡����Ƭ�ڵ���������еĽڵ� */
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);

			if(pNode != NULL)
			{
				if(node_level == 0)
				{
					node_level = pNode->level;
				}
				/* �õ�������Ƭ����С�㼶 */
				if(node_level >= pNode->level)
				{
					node_level = pNode->level;
				}

				/* lastdrawΪ3,��ʾ��Ƭ�ڵ������δ������� */
				if(3 == pNode->lastdraw)
				{
					node_lastdraw++;
				}
			}

		}
	}
#if 0
	if((pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn))
	{
		if(pScene->is_romed == eROM_notroamed)
		{
			SetRenderState(1);
		}
		else if(node_level < (sMaxLevel - 1))
		{
			SetRenderState(1);
		}
		else
		{
			SetRenderState(0);
		}
	}
	else
#endif


	/* ���������Ƭ�ڵ������δ�������,���֪��������ǰ֡��SwapBuffer,�Ա�����һ֡�Ļ��� */
	if(node_lastdraw > 0)
	{
		SetRenderState(0);;
	}	
	else
	{
		SetRenderState(1);;
	}
	
	return 0;
}

/*
���ܣ�������δ�����߶�Ԥ������ʱ,�ж�������������Щ��Ƭ�ڵ���Ҫ���ص�ǰ֡����������(�ݹ��ж�)
���룺
	pHandle     ��ͼ���������
	pNode       ��Ƭ�ڵ���
	pScene      �������ƾ��

���������
	��

�����
    ��

����ֵ��
    NODE_NOT_READY(0) ������,�������
    NODE_NOT_IN_VIEW(0) �����Ӿ��巶Χ,�������
	NODE_AT_EARTH_BACK(0) �ڵ����Ӿ��巶Χ���ڵ�����,�������
	NODE_DRAWING(1)   �ڵ���Ҫ����,�Ѽӵ���ǰ֡����������
*/
static f_int32_t mapNodeRenderOverlook(sMAPHANDLE *pHandle, sQTMAPNODE *pNode,  sGLRENDERSCENE *pScene)
{
	f_int32_t ret = 0, i = 0;

	/* �ж�����ĵ�ͼ����������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    DEBUG_PRINT("call mapNodeRender with error handle.");
	    return NODE_NOT_READY;
	}
	
	/* �ж�����ĵ�ǰ��Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
	    DEBUG_PRINT("call mapNodeRender with error node.");
	    return NODE_NOT_READY;
	}
	
	/* �жϵ�ǰ��Ƭ�ڵ������Ƿ�������(����ά���ݼ���������ִ��),��δ�����ֱ�ӷ��� */
	ret = qtmapnodeGetStatus(pNode, __LINE__);
	if(DNST_READY != ret)
		return NODE_NOT_READY;
	
	/**************�����ǵ�ǰ��Ƭ�ڵ������Ѿ�������ɵ����*******************/

	/* ��¼�ڵ�ĵ�ǰ���Ƶ�֡��,����ӿڻ��ۼ� */
	pNode->drawnumber = pHandle->drawnumber;

    /* 1.�ж��������λ��(���ɻ�λ��)�ľ�γ���Ƿ�����Ƭ�ڵ�(pNode)�����ķ�Χ�� 1-��,0-���� */
    if(!isCaminNode(pScene, pNode))
    {
		/* �ж��Ƿ����Ӿ�����(�������ӽڵ�)  0-��,-1-����*/
		/* 1.1 �������1�����㣬���һ���ж���Ƭ�ڵ�(pNode)�Ƿ���������Ӿ�����ӷ�Χ�� 0-��,-1-����  */
		/* ��mapNodeRender������ͬ�ĵط�,������ʹ�õ�������Ƭ�ĸ��ǵ�(�߶�Ĭ��Ϊ0)�γɵİ�Χ�� */
		/* �ж�ʱ,��ǰ�ڵ㲻���Ӿ�����,�����һ�������ӽڵ� */
		ret = isNodeInFrustumOverlook(pScene->camctrl_param.view_near, pScene->camctrl_param.view_far ,&(pScene->frustum), pNode);

		/* �������1.1������,�򲻻���,ֱ�ӷ��� */
		if(0 != ret)
		{
			//printf(" data not in frustum ");
			return NODE_NOT_IN_VIEW;
		}	
		
		/* 1.2 ����ģʽ�²����ж��Ƿ��ڱ���,�����ڵͲ㼶������Ƭ̫���¶�� ����*/
		if(eROM_outroam == pScene->mdctrl_cmd.rom_mode)
		{
		    /* 1.3 �ж���Ƭ�ڵ��Ƿ�������Ӿ��忴���ĵ���ı���,����ڱ����򲻻���,ֱ�ӷ��� */
		    if(isNodeAtEarthBack(pScene, pNode))
		    {
		    	//	printf(" data in back ");
			    return NODE_AT_EARTH_BACK;	
		    }
		}
    }
	
	/* �������1�������ж�ͨ��,Ȼ���ж��Ƿ���ڻ����(���㼶-1)������(���㼶-1)�󲻻������ж�����Ƭ */
	/* ��getTreeLevel�����в�ͬ,�˴����ٵ���isNeedCreateChildren�����ж��Ƿ���Ҫ��������Ƭ,������Ĭ����Ϊ����Ƭ��Ҫ���� */
	/* ,�������Ƭ������������,���Կ��ƻ��ƵĽڵ���ൽ(���㼶-1)��,���ٻ��ƽڵ������ */
	if(pNode->level < (sMaxLevel - 1))
	{
		/* ��ʾ��Ҫ�ݹ�����Ƭ*/
	    ret = 0;   
//		printf("not max level %d \n",pNode->level );
	}
	else
	{	
		/* ����ݹ�����Ƭ,ֱ�ӻ��Ƶ�ǰ��Ƭ�ڵ� */
		ret = -1; 
//		printf("max level %d \n",pNode->level );
	}

	if(0 != ret)
	{
		/* ���ݹ�����Ƭ��������;����ǰ��Ƭ�ڵ���������Ƭ�ڵ����� */		
		drawListAddNode(pNode, pScene);
		/* ���õ�ǰ��Ƭ�ڵ��lastdraw=1,��ʾ����ƬΪҶ�ӽڵ�,�޺��ӽڵ�,ֱ�ӷ��� */
		pNode->lastdraw = 1;
		return NODE_DRAWING;
	}
	
	/* ������ǰ��Ƭ�ڵ�ĺ��ӽڵ�,����������������ΪFALSE, ��֪��Щ�ڵ�Ҫ�������� */
	/* -1���ڵ㲻���ڻ����ӽڵ㴴��ʧ�� 0���ӽڵ㲻���������� 4���ӽڵ㴴���ɹ�*/
	ret = qtmapnodeCreateChildren(pHandle, pNode, FALSE);  
	if(0 >= ret)
	{
		/* ��������Ƭʧ�ܣ�������;����ǰ��Ƭ�ڵ���������Ƭ�ڵ����� */
		drawListAddNode(pNode, pScene);

		/* ���õ�ǰ��Ƭ�ڵ��lastdrawֵ,Ȼ��ֱ�ӷ��� */
		if(ret == 0)
			pNode->lastdraw = 2;	//�ӽڵ㲻����������
		else
			pNode->lastdraw = 3;	//�ڵ�����ӽڵ㴴��ʧ��

		return NODE_DRAWING;
	}
	
	/* �ж��Ƿ���������Ƭ�����ݶ��������(����ά���ݼ���������ִ��) */
	ret = isAllChildNodeDataReady(pNode);   //0��ʾyes
	if(0 != ret)
	{
		/* ������Ƭû׼���ã�������;����ǰ��Ƭ�ڵ���������Ƭ�ڵ����� */
		drawListAddNode(pNode, pScene);
		
		/* ���õ�ǰ��Ƭ�ڵ��lastdrawֵΪ3,�ӽڵ�����δ�������Ҳ���ӽڵ㴴��ʧ��,Ȼ��ֱ�ӷ��� */
		pNode->lastdraw = 3;
		return NODE_DRAWING;
	}
	
	/* ��ǰ��Ƭ�Ƿ���Ҫ�����ж�����,�ݹ��жϸ���Ƭ�ڵ���ĸ�����Ƭ�Ƿ���� */
	for(i=0; i<4; i++)
	{
		mapNodeRenderOverlook(pHandle, pNode->children[i], pScene);
	}
	/* ���õ�ǰ��Ƭ�ڵ��lastdrawֵΪ4,�ӽڵ����ݼ�����ɵ�����Ƭ�ڵ��������,Ȼ��ֱ�ӷ��� */
	pNode->lastdraw = 4;
	
	return 0;
}

/*
���ܣ������ҿ����߶�Ԥ������/�����Ǹ����ӽ�ʱ,�ж�������������Щ��Ƭ�ڵ���Ҫ���ص�ǰ֡����������(�ݹ��ж�)
���룺
	pHandle         ��ͼ���������
	pNode           ��Ƭ�ڵ���
	pScene          �������ƾ��
	pPlanePosNode   �ɻ�λ��������Ƭ�ڵ���
	pitch           �ɻ�������
���������
	��

�����
    ��

����ֵ��
    NODE_NOT_READY(0) ������,�������
    NODE_NOT_IN_VIEW(0) �����Ӿ��巶Χ,�������
	NODE_AT_EARTH_BACK(0) �ڵ����Ӿ��巶Χ���ڵ�����,�������
	NODE_DRAWING(1)   �ڵ���Ҫ����,�Ѽӵ���ǰ֡����������
*/
static f_int32_t mapNodeRender(sMAPHANDLE *pHandle, sQTMAPNODE *pNode, sQTMAPNODE *pPlanePosNode, f_float32_t pitch, sGLRENDERSCENE *pScene)
{
	f_int32_t ret = 0, i = 0;
	
	/* �ж�����ĵ�ͼ����������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    DEBUG_PRINT("call mapNodeRender with error handle.");
	    return NODE_NOT_READY;
	}
	
	/* �ж�����ĵ�ǰ��Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pNode))
	{
	    DEBUG_PRINT("call mapNodeRender with error node.");
	    return NODE_NOT_READY;
	}
	
	/* �ж�����ķɻ�λ�����ڵ���Ƭ�ڵ����Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPDATANODE(pPlanePosNode))
	{
	    DEBUG_PRINT("call mapNodeRender with error pPlanePosNode.");
	    return NODE_NOT_READY;
	}

	/* �жϵ�ǰ��Ƭ�ڵ������Ƿ�������(����ά���ݼ���������ִ��),��δ�����ֱ�ӷ��� */
	ret = qtmapnodeGetStatus(pNode, __LINE__);
	if(DNST_READY != ret)
		return NODE_NOT_READY;
	
	/**************�����ǵ�ǰ�ڵ��Ѿ�׼�����˵����*******************/	

	/* ��¼�ڵ�ĵ�ǰ���Ƶ�֡��,����ӿڻ��ۼ� */
	pNode->drawnumber = pHandle->drawnumber;

    /* 1.�ж��������λ��(���ɻ�λ��)�ľ�γ���Ƿ�����Ƭ�ڵ�(pNode)�����ķ�Χ�� 1-��,0-���� */
    if(!isCaminNode(pScene, pNode))
    {
		/* 1.1 �������1�����㣬���һ���ж���Ƭ�ڵ�(pNode)�Ƿ���������Ӿ�����ӷ�Χ�� 0-��,-1-����  */
		/* ��mapNodeRenderOverlook������ͬ�ĵط�,ʹ�õ��Ǵ��߶���Ϣ�İ�Χ��,������ʹ�õ�������Ƭ�ĸ��ǵ�(�߶�Ĭ��Ϊ0)�γɵİ�Χ�� */
		/* �ж�ʱ,��ǰ�ڵ㲻���Ӿ�����,�����һ�������ӽڵ� */
		ret = isNodeInFrustum(pScene->camctrl_param.view_near, pScene->camctrl_param.view_far ,&(pScene->frustum), pNode);
		if(0 != ret)
		{
			//printf(" data not in frustum ");
			return NODE_NOT_IN_VIEW;
		}	
		
		/* 1.2 ����ģʽ�²����ж��Ƿ��ڱ���,�����ڵͲ㼶������Ƭ̫���¶�� ����*/
		if(eROM_outroam == pScene->mdctrl_cmd.rom_mode)
		{
		    /* 1.3 �ж���Ƭ�ڵ��Ƿ�������Ӿ��忴���ĵ���ı���,����ڱ����򲻻���,ֱ�ӷ��� */
		    if(isNodeAtEarthBack(pScene, pNode))
		    {
		    	//	printf(" data in back ");
			    return NODE_AT_EARTH_BACK;	
		    }
		}
    }
	
	/* �������1�������ж�ͨ��,Ȼ���ж��Ƿ���ڻ����(���㼶)������(���㼶)�󲻻������ж�����Ƭ */
	/* ��getTreeLevel��������ͬ,��mapNodeRenderOverlook�����в�ͬ,�˴����ƻ��ƵĽڵ���ൽ(���㼶)��,��Ϊ���ж�����Ƭ�Ƿ���Ҫ���� */
	/* ����1���жϳ��������Ӿ����ڵ���Ƭ���ǵ�ͬһ�㼶,�����ӽ�Ϊ�˼�����Ƭ����,��ȡ���ӵ������Ƭ���ø߲㼶,���ӵ�Զ�Ĳ��õͲ㼶�Ĳ��� */
	if(pNode->level < sMaxLevel)
	{
		/* ���δ���ﵽ�����㼶�����һ���ж��Ƿ���Ҫ��������Ƭ 0-��Ҫ���� ����-���贴�� */
	    ret = isNeedCreateChildren(pNode, pPlanePosNode, pScene->camParam.m_geoptEye, pScene->camParam.m_ptEye, pScene->camctrl_param.view_type);  //����0��ʾ��Ҫ���·�
		/* ����������[82.5,,97.5]֮��,���ӽ������ӽ�ʱ */
		if(fabs(fabs(pitch) - 90.0) < 7.5)
		{
			if(0 != ret)
			{
				/* ������贴����ǰ��Ƭ������Ƭ,����ǰ��Ƭ�㼶С�ڷɻ�������Ƭ�Ĳ㼶,����ǿ��ȥ��������Ƭ */
			    if(pNode->level < pPlanePosNode->level)
			        ret = 0;
			}
			else
			{
				/* �����Ҫ������ǰ��Ƭ������Ƭ,����ǰ��Ƭ�㼶���ڻ���ڷɻ�������Ƭ�Ĳ㼶,��ȥ��������Ƭ */
				if(pNode->level >= pPlanePosNode->level)
			        ret = -2;
			}
			//printf("pitch = %f, level = %d\n", pitch, pPlanePosNode->level);
		}	  


	}
	else
	{	
		 /* �ѵ������㼶 */
		ret = -1;
	}

	if(0 != ret)
	{
		/* ���ݹ�����Ƭ��������;����ǰ��Ƭ�ڵ���������Ƭ�ڵ����� */
		drawListAddNode(pNode, pScene);
		/* ���õ�ǰ��Ƭ�ڵ��lastdraw=1,��ʾ����ƬΪҶ�ӽڵ�,�޺��ӽڵ�,������Ƭ�Ѵﵽ���㼶,ֱ�ӷ��� */
		pNode->lastdraw = 1;
		return NODE_DRAWING;
	}
	
	/*�����жϳ���Ҫ�ݹ�����Ƭ�����Ե���ȥ��������Ƭ*/
	ret = qtmapnodeCreateChildren(pHandle, pNode, FALSE);  //����0��4��0������һ������Ƭ����ʧ�ܣ�4������Ƭ�������ɹ�
	if(0 >= ret)
	{
		/* ��������Ƭʧ�ܣ�������;����ǰ��Ƭ�ڵ���������Ƭ�ڵ����� */
		drawListAddNode(pNode, pScene);

		/* ���õ�ǰ��Ƭ�ڵ��lastdrawֵ,Ȼ��ֱ�ӷ��� */
		if(ret == 0)
			pNode->lastdraw = 2;  //�ӽڵ㲻����������
		else
			pNode->lastdraw = 3;  //�ڵ�����ӽڵ㴴��ʧ��

		return NODE_DRAWING;
	}
	
	/* �ж��Ƿ���������Ƭ�����ݶ��������(����ά���ݼ���������ִ��) */
	ret = isAllChildNodeDataReady(pNode);   //0��ʾyes
	if(0 != ret)
	{
		/* ������Ƭû׼���ã�������;����ǰ��Ƭ�ڵ���������Ƭ�ڵ����� */
		drawListAddNode(pNode, pScene);
		/* ���õ�ǰ��Ƭ�ڵ��lastdrawֵΪ3,�ӽڵ�����δ�������Ҳ���ӽڵ㴴��ʧ��,Ȼ��ֱ�ӷ��� */
		pNode->lastdraw = 3;		
		return NODE_DRAWING;
	}
	
	/* ��ǰ��Ƭ�Ƿ���Ҫ�����ж�����,�ݹ��жϸ���Ƭ�ڵ���ĸ�����Ƭ�Ƿ���� */
	for(i=0; i<4; i++)
	{
		mapNodeRender(pHandle, pNode->children[i], pPlanePosNode, pitch,pScene);
	}
	/* ���õ�ǰ��Ƭ�ڵ��lastdrawֵΪ4,�ӽڵ����ݼ�����ɵ�����Ƭ�ڵ��������,Ȼ��ֱ�ӷ��� */
	pNode->lastdraw = 4;
	
	return 0;
}

static f_float32_t rgb2GrayValue(f_float32_t r, f_float32_t g, f_float32_t b)
{
	f_float32_t gray = r * 0.299f + g * 0.587f + b * 0.114f;
	return gray;
}

/*�������Ч��*/
static void setFog(sGLRENDERSCENE * pHandle)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_float32_t fogColor[4] = {0.75f, 0.75f, 0.75f, 0.0f};
//	if(!ISMAPHANDLEVALID(pHandle))
//	    return;
	if(pHandle == NULL)
		return;
	
	// ����ģʽ�²������������ӵ�λ�úܸߣ���������������ε���ɫ�����仯
	if ( eVM_OVERLOOK_VIEW == pHandle->camctrl_param.view_type )
	{
		glDisableEx(GL_FOG);
		return;
	}
	
	glEnableEx(GL_FOG);
	{
		if (eCOR_gray == pHandle->mdctrl_cmd.cor_mode)
		{
			f_float32_t fogGrey = rgb2GrayValue(fogColor[0], fogColor[1], fogColor[2]);
			fogColor[0] = fogGrey;
			fogColor[1] = fogGrey;
			fogColor[2] = fogGrey;
		}

		glFogi (GL_FOG_MODE, GL_LINEAR);
		glFogfv (GL_FOG_COLOR, fogColor);
		glHint (GL_FOG_HINT, GL_DONT_CARE);
		{
			Geo_Pt_D geoptTemp;
			f_float32_t disNear, disFar, ratio;
			/*
			 * ���¼���10000�׸߶���30000�׸߶�֮���Զ������ı���
			 * ���� a*height+b=ratio
			 * ��    10000*a+b=5
			 *       30000*a+b=ratio30000
			 * ��30000�׵�ratio30000=sqrt(ptTemp.z * ptTemp.z + 2 * EARTH_RADIUS * ptTemp.z + 2 * EARTH_RADIUS * EARTH_RADIUS)
			 */
			f_float32_t a=(f_float32_t)(sqrt(30000.0f * 30000.0f + 2.0f * EARTH_RADIUS * 30000.0f + 2.0f * EARTH_RADIUS * EARTH_RADIUS)/80000.0f - 5.0f) / 20000.0f;
			f_float32_t b=(f_float32_t)(15.0f - sqrt(30000.0f * 30000.0f + 2.0f * EARTH_RADIUS * 30000.0f + 2.0f * EARTH_RADIUS * EARTH_RADIUS) / 80000.0f) / 2.0f;
            
            objDPt2geoDPt(&(pHandle->camParam.m_ptEye), &geoptTemp);
			disNear = MAX(geoptTemp.height, 25000.0f);
			if (geoptTemp.height < 10000.0f)
			{
				disFar = disNear * 5.0f;
			}
			else if (geoptTemp.height >= 10000.0f && geoptTemp.height <= 30000.0f)
			{
				ratio = geoptTemp.height * a + b;
				disFar = disNear * ratio;
			}
			else
			{
				disFar = sqrt(geoptTemp.height * geoptTemp.height + 2.0f * EARTH_RADIUS * geoptTemp.height + 2.0f * EARTH_RADIUS * EARTH_RADIUS);
			}
			glFogf (GL_FOG_START, disNear);
			glFogf (GL_FOG_END, disFar);
		}
	}
#endif
}

//static f_int32_t qtmapDrawNodeLayer(sMAPHANDLE * pHandle, sQTMAPNODE *pNode, f_int32_t layer)
f_int32_t qtmapDrawNodeLayer(sMAPHANDLE * pHandle, sQTMAPNODE *pNode, f_int32_t layer)
{
	if (!ISMAPHANDLEVALID(pHandle))
	{
		DEBUG_PRINT("call qtmapDrawNodeLayer with error handle.");
		return 0;
	}
	if (!ISMAPDATANODE(pNode))
	{
		DEBUG_PRINT("call qtmapDrawNodeLayer with error node.");
		return 0;
	}
	if (pNode->lastdraw == 0)
		return 0;
	if (pNode->lastdraw == 2)
	{
		f_int32_t i;
		for (i = 0; i < 4; i++)
		{
			if(NULL != pNode->children[i])
			    qtmapDrawNodeLayer(pHandle, pNode->children[i], layer);
		}
		return 0;
	}
	if (qtmapnodeGetStatus(pNode, __LINE__) != DNST_READY)
		return -1;
 
// 	if (pHandle->layerfunc[layer].funcdisplay != NULL)
// 		pHandle->layerfunc[layer].funcdisplay( pHandle->cameractl.view_type, pHandle->ttf_font, 
// 		                                       pNode->nodeid[layer], pHandle->cam_obj_pos, pHandle->cur_plane_height, pHandle->h_viewport);

	return 0;
}

f_int32_t qtmapRender(VOIDPtr map_handle, f_int32_t terwarn_md, f_float32_t pitch, VOIDPtr scene)
{
	/* ��ͼ������,ȫ��Ψһ */
	sMAPHANDLE * pHandle = NULL;
	/* ��ͼ�������еĸ������� */
	sQTMAPSUBTREE * pSubTree = NULL;
	/* �ɻ�λ����������Ƭ */
	sQTMAPNODE *pPlanePosNode = NULL;
	/* ��Ⱦ�������,һ���ӿڶ�Ӧһ�� */
	sGLRENDERSCENE *pScene = NULL;
	f_int32_t i = 0, j = 0;

	/* �ж��������Ⱦ��������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return -1;	
	
	/* �ж�����ĵ�ͼ�������Ƿ���Ч,����Ч��ֱ�ӷ��� */
	if (!ISMAPHANDLEVALID(map_handle))
		return -1;
	pHandle = (sMAPHANDLE *)map_handle;
	
    takeSem(pHandle->rendscendDrawlock,FOREVER_WAIT);

	// ������ջ�������
	drawListDeleteNode(pScene);

	/* ��¼��ǰ���Ƶ�֡��,����ӿڻ��ۼ�,�ú�������һ�μ�������1 */
	pHandle->drawnumber = (pHandle->drawnumber + 1) & ((1 << 20) - 1); 	

	/* ���ڼ�¼��ǰ֡������Ƭ�ڵ���,ÿ֡ÿ���ӿڿ�ʼ����ǰ������ */
	pHandle->drawnnodnum = 0;

	/* ���ò��������Ч�� */
	//setFog(pScene);

	/* ������պ�,�����ӽ��²����� */
	if( pScene->camctrl_param.view_type != eVM_OVERLOOK_VIEW )
	{
		renderAtmosphere(pScene);
	}

	/* ����ȫ���0���8�������ĸ��ڵ� */
	for(i  = 0; i < 4; i++)
	{
		for(j = 0; j < 2; j++)
		{
	        if (pHandle->pNodeRoot[i * 2 + j] == NULL)
	        {
				/* ���ݸ��ڵ����Ƭ�㼶(��Ϊ0)��x����������y���������жϸ���Ƭ�ڵ��Ƿ����Ѷ�ȡ������������,�������,����ֱ���˳�; */
		        /* �����,���ظ�������ָ��; */
				if (!isNodeExistInMapSubreeList(pHandle, 0, i, j, &pSubTree))
			        return -2;
				/* �������ڵ�,����֪��ά�������������Ƭ�ڵ����ݼ��� */
		        qtmapnodeCreate(pHandle, 0, i, j, pSubTree, &pHandle->pNodeRoot[i * 2 + j], FALSE);
	        }
	    }
	}	
	
	/* �������㼶,�����ӳ��Ǻ��ӵ�߶ȼ�����Ƭ���ȵ����㼶 */
    sMaxLevel = getCurrentMaxLevel(pScene->mdctrl_cmd.scene_mode, pScene->camctrl_param.view_angle, pScene->camParam.m_geoptEye.height);                           
	
	// gx test
	sMaxLevel = 1;	

	/* ������С�㼶 */
	sMinLevel = 9999;
	
	/* �����ӵ�ľ�γ����8�������в����ӵ����ڵ��Ǹ���Ƭ�ڵ�(������������߲㼶 < level < sMaxLevel-1) */
	for(i = 0; i < 8; i++)
	{
		/* ���øú����ṹ���Ĳ���,����δ��֪��ά�������������Щ�ڵ����� */
	    pPlanePosNode = getTreeLevelOfPlanepos(pHandle, pHandle->pNodeRoot[i], pScene->camParam.m_geoptEye.lon, pScene->camParam.m_geoptEye.lat, pScene);
		/* ֻ��������һ���ڵ���,�ҵ�����ֱ������ѭ�� */
	    if(NULL != pPlanePosNode)
	        break;
    }
	
	/* �����ⲿ����������رյ��θ澯���� */ 
	if(eTERWARN_outwarn == terwarn_md)
	{
	    pScene->is_need_terwarning = FALSE;
	}
	else
	{
	    pScene->is_need_terwarning = TRUE;
	}
	    
	/* ӥ��ģʽǿ�ƹرյ��θ澯���� */    
	if(eScene_eyebird == pScene->mdctrl_cmd.scene_mode)
	{
		pScene->is_need_terwarning = FALSE;
	}

	/* ���α���ȫ��8������,������Щ��Ƭ�ڵ���Ҫ����,������뵽���������� */ 
	/* �����������:������δ�����߶�Ԥ��ʱ,ÿ����Ƭ�ڵ�ֻ����Ƴ�һ������,����Ҫ�ؼ���,�㼶�������,���ӻ��ƽڵ� */
	/* �������ʱ,ÿ����Ƭ�ڵ���밴����������������,����Ҫ�ض�,�㼶Ҫ����,���ٻ��ƽڵ� */
	if((pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn))
	{
		/* ����ģʽ��δ�����߶�Ԥ��ʱ,���贫��ɻ�λ�����ڵĽڵ� */
		for(i = 0; i < 8; i++)
		{
			mapNodeRenderOverlook(pHandle, pHandle->pNodeRoot[i], pScene);
		}

	}
	else
	{
		/* ����ģʽʱ,�贫��ɻ�λ�����ڵĽڵ�ͷɻ��ĸ����� */
		for(i = 0; i < 8; i++)
		{
		    mapNodeRender(pHandle, pHandle->pNodeRoot[i], pPlanePosNode, pitch, pScene);	
		}
	}
	
	/* �ж���ά���ݼ��������Ƿ���Ƭ���ݼ������,δ�����SwapBuffer,�Ա�����һ֡�Ļ������� */
	/* ������֤������ʾ���ȹ������м����Ƭ������,ֱ����ʾ��������ɺ����Ƭ,���������ģ���������Ĺ��� */
	/* ��ʱ������ */
	//DrawOverlookCallback(pHandle, pScene);

	/* ѭ�����Ƶ�ǰ֡������Ƭ�ڵ������е����нڵ�,��ͬģʽ���ò�ͬ�Ļ��Ʒ��� */
	drawListDraw(pHandle, pScene);

	/* �ر����� */
	glDisableEx(GL_FOG);


	if(pScene->mdctrl_cmd.svs_mode == 1)
	{
		EnableLight(FALSE);
		EnableLight0(FALSE);
	}

#ifdef USE_SQLITE

	/* ����/����/ӥ���²����Ƶ��� */
	if( ((pScene->camctrl_param.view_type != eVM_OVERLOOK_VIEW)
		&&(pScene->camctrl_param.view_type != eVM_SCALE_VIEW))
		||(pScene->mdctrl_cmd.scene_mode == eScene_eyebird) )
	{

		/* �ж��Ƿ���Ҫ���µ���qtn���� */
		QtnSqlJudge(pHandle, pScene);

		/* ���Ƹ���ͼ��,Ŀǰֻ֧��1��,����ע��ͼ�� */
		for (i = 0; i < MAXMAPLAYER; i++)
		{
			f_float32_t projection[16] = {0};
			f_float32_t modelview[16] = {0};

			glGetFloatv(GL_PROJECTION_MATRIX,projection);
			glGetFloatv(GL_MODELVIEW_MATRIX,modelview);
			
			/* ����ǰ��׼��,������ά�г�ʼ����������,ʵ�ʵ��õ��Ǻ���qtndisplaypre */
			if (pHandle->layerfunc[i].funcdisplaypre != NULL)
			{
				pHandle->layerfunc[i].funcdisplaypre(pScene->camctrl_param.view_type, pHandle->ttf_font, pScene->innerviewport, pScene->matrix);
			}
			/*ע����ײ��������ʼ�����ж���Щע����Ҫ������ʾ*/
			if (pHandle->layerfunc[i].funcdisplay != NULL)
			{
				/*��ʼ����ײ���Ĳ���*/
			    collisionInfoInit3d(pScene->innerviewport[2], pScene->innerviewport[3]);
//			    for(j = 0; j < 8; j++)
//					qtmapDrawNodeLayer(pHandle, pHandle->pNodeRoot[j], i);

				takeSem(pHandle->qtnthread, FOREVER_WAIT);

				//printf("pHandle->cam_geo_pos.height =  %f\n",pHandle->cam_geo_pos.height);
				/*�ж���Щע����Ҫ������ʾ,����Ҫ���Ƶ�ע�Ƿ��뵽ȫ�ֵ�ע�ǻ���������,ʵ�ʵ��õ��Ǻ���qtndisplaynode*/
				pHandle->layerfunc[i].funcdisplay( pScene->camctrl_param.view_type, pHandle->ttf_font, 
			                                    0, pScene->camParam.m_ptEye, pScene->camParam.m_geoptEye.height/*pHandle->cur_plane_height*/, pScene->innerviewport,
			                                    pHandle->qtn_node[pScene->qtn_nodeID].qtn_sql, pHandle->qtn_node[pScene->qtn_nodeID].qtn_num);

				giveSem(pHandle->qtnthread);		
			}

			/* ע����ײ��ע�ǻ�����ʾ�����ƺ��״̬�ָ�,ʵ�ʵ��õ��Ǻ���qtndisplaypro */
			if (pHandle->layerfunc[i].funcdisplaypro != NULL)
			{
				pHandle->layerfunc[i].funcdisplaypro(pScene->camctrl_param.view_type, pHandle->ttf_font);
			}

			/*�ָ�ԭ����ͶӰģ�Ӿ���*/
			glMatrixMode(GL_PROJECTION);						
			glLoadMatrixf(projection);
		    glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(modelview);
		}
	}
#endif	

	/* �������Դ��е������������ */
	/* createTexNumber��mapNodeRenderImg�����и�ֵ,ÿ����һ���ڵ��������1 */
	/* delTexNumber��qtmapClearCache�����и�ֵ,ÿɾ��һ���ڵ�򵥶�ɾ���ڵ�����ʱ�������1 */
	if(pHandle->createTexNumber - pHandle->delTexNumber > pHandle->maxTexNumber)
		pHandle->maxTexNumber = pHandle->createTexNumber - pHandle->delTexNumber;

#if 0
	/* ÿ��600tick��ӡһ�������ڲ��ľ������ */
	/* drawnnodnum��ʾ��ǰ֡��ͼ�������л��Ƶ��ܽڵ���,ÿһ֡������ */
	/* nodecount��ʾ��ͼ����������ʷ�ۼƴ������ܽڵ��� */
	/* loadedNumber��ʾ��ͼ����������ʷ�ۼ����ݼ�����ɵ��ܽڵ��� */
	/* delNodeNumber��ʾ��ͼ����������ʷ�ۼƴ��ڴ���ɾ�����ܽڵ���,loadedNumber-delNodeNumber��ʾʵ���ڴ��е��ܽڵ��� */
	if(tickCountGet()- pHandle->startTick > 600)
	{
		printf("scene_index=%d, drawNodeCurrent=%d, nodeInRamCurrent=%d, texInGpuCurrent=%d, maxTexInGpuHistory=%d, totalCreatedNodeHistory=%d, totalCreatedTexHistory=%d\n", 
        	   pScene->scene_index, pHandle->drawnnodnum, pHandle->loadedNumber - pHandle->delNodeNumber, pHandle->createTexNumber-pHandle->delTexNumber,
			   pHandle->maxTexNumber, pHandle->nodecount, pHandle->createTexNumber);

		PrintMemoryPools();

		pHandle->startTick = tickCountGet();
	}
#endif

#if 0
    {
    stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	sQTMAPNODE  *pRet = NULL;
	BOOL is_dnst_empty = FALSE;
	f_int32_t empty_count = 0, count = 0;
	
    /*����LRU��������û�п��õĽڵ�*/
	pstListHead = &(pHandle->nodehead.stListHead);
	/**�������**/
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pRet = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
			if(DNST_EMPTY == qtmapnodeGetStatus(pRet, __LINE__))
			{
				is_dnst_empty = TRUE;
			    empty_count++;
			}
			else
			{
				printf("%d_%d_%d ", pRet->level, pRet->xidx, pRet->yidx);
			    count++;
			}
		
		}
	}	
	printf("empty_count = %d, count = %d\n", empty_count, count);
    }
#endif
			
	/* ���ͼ�� */
	if (qtmapGetNodePeakValue(pHandle) > MAXQTNODE)
	{
		for(i = 0; i < 8; i++)
		    qtmapClearCache(pHandle, pHandle->pNodeRoot[i]);
	}

    giveSem(pHandle->rendscendDrawlock);

	return 0;
}


/*�����Ƿ������Ƭ�߽��ߣ�0-�����ƣ�1-����*/
void setDrawTileGrid(int bDraw)
{
	bDrawTileGrid = bDraw;
}

/*�����Ƿ������Ƭ���ƣ�0-�����ƣ�1-����*/
void setDrawTileName(int bDraw)
{
	bDrawTileName = bDraw;
}

// ��ȡ��ǰ�߿����ȼ�
int GetCurMaxTileLvl()
{
	return sMaxLevel;
}