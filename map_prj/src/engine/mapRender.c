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
#endif                                   /* 影像数据生成纹理函数 */
#endif

#define MAXQTNODE	  60//110
#define NODE_NOT_READY   0
#define NODE_DRAWING     1

#define NODE_NOT_IN_VIEW 0   /* 节点不在视口内 */
#define NODE_IN_VIEW     1   /* 节点在视口内 */

#define NODE_AT_EARTH_BACK     0   /* 节点在地球背面 */
#define NODE_NOT_AT_EARTH_BACK 1   /* 节点不在地球背面 */

f_uint32_t gindices[index_count];  /* 正常模式顶点的索引号 */
f_float32_t gTextureBuf[vertexNum*2];  /* 正常模式顶点的二维纹理坐标 */
f_uint32_t indices_eyebird[index_count_eyebird];  /* 鹰眼模式顶点的索引号 */
f_float32_t gTextureBuf_eyebird[vertexNum_eyebird*2]; /* 鹰眼模式顶点的二维纹理坐标 */

static f_int32_t sMaxLevel = 0;
static f_int32_t sMinLevel = 0;

/* 瓦片各层级对应经度方向的跨度所代表的距离(单位是m) */
f_float64_t glfLevelDelta[LEVEL_MAX] = 
{
	10018754.17139462, /* 0层瓦片对应经度方向的跨度所代表的距离是赤道周长的1/4,赤道周长为40075016.6855784861531768177614 */
	5009377.085697311, /* 1层是0层的1/2,之后依次类推 */
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

VOIDPtr pMapDataSubTree;			/*地图场景树*/

/* JM7200使用显示列表比顶点数组快很多 */
/* 绘制瓦片时是否强制使用显示列表,0-强制使用,1-不强制使用 */
f_int32_t useArray = 1;	// gx test

/*ibo-顶点索引的VBO，tvbo-顶点纹理坐标的vbo*/
static GLuint ibo = 0, tvbo = 0;
/*ibo-俯视视角顶点索引的VBO，tvbo-俯视视角顶点纹理坐标的vbo*/
static GLuint ibo_overlook = 0, tvbo_overlook = 0;

/*是否绘制瓦片边界线，0-不绘制，1-绘制*/
static int bDrawTileGrid = 0;
/*是否绘制瓦片名称，0-不绘制，1-绘制*/
static int bDrawTileName = 0;

/* 字体绘制函数的声明 */
extern int textOutPutOnScreen(int fontType, float viewportX, float viewportY, unsigned char * stringOut, sColor4f textColor);

extern void EnableLight(BOOL bEnable);
extern void EnableLight0(BOOL bEnable);
extern void EnableLight1(BOOL bEnable);
extern void SetRenderState(int value);

/*
功能：根据视口范围内期望观察到的瓦片经度方向跨度所代表的距离来判断调用的最大层级
输入：瓦片经度方向跨度所代表的距离
输出：最大调度层级
*/
static f_int32_t radioToLevel(const f_float64_t radio)
{
	f_int32_t i = 0;
	f_float64_t radio_parent_level = glfLevelDelta[0];
	for( i = 0; i < LEVEL_MAX; i++)
	{
		/* 当radio比第i层的跨度距离大,则更新radio上一层的跨度距离为第i层的距离 */
		if(radio > glfLevelDelta[i])
		{
			/* 当radio比第i层的跨度距离小,则取 第i层和第i-1层的距离之和的1/2 与radio进行比较; */
			/* 若radio更大,则返回的层级为i-1; 若radio更小,则返回的层级为i */
			f_float64_t middle = (radio_parent_level + glfLevelDelta[i]) / 2;
			if(radio > middle)
				return (i!=0)?(i-1):0;
			else
				return i;
		}
		/* 当radio比第i层的跨度距离大,则更新radio上一层的跨度距离为第i层的距离 */
		radio_parent_level = glfLevelDelta[i];	
	}
	return LEVEL_MAX;
}

static f_int32_t getCurrentMaxLevel(f_int32_t scene_mode, f_float32_t view_angle, f_float32_t height)
{
	/* 假设瓦片是一个正方形,radio表示正方形一条边所代表的距离(单位是m) */
	f_float64_t scale, radio;

	/* 通过调整scale的大小可以控制绘制图块的多少,如果想视景体高度能显示1个图块,则scale取2.0； */
	/* 如果想视景体高度能显示2个图块,则scale取1.0；显示4个图块,则取1/2；显示8个图块,则取1/4；依次类推；*/
	/* scale越小,图块越多,scale越大,图块越少 */	
	scale = 1.0/4.0;

	/* 鹰眼模式下将调用的瓦片层级下调，调用分辨率更高的瓦片 */
	//if(eScene_eyebird == scene_mode)
	//    scale *= 0.5; 
    
	/* 该计算模型(简化成一个等腰三角形)只对俯视视角有效 */
	radio = tan(view_angle * 0.5 * DE2RA) * height * scale;

	/* 计算最大调度层级 */
	return radioToLevel(radio);
}

/*
功能：创建输入节点的孩子节点
输入：
	pHandle             地图场景树指针
	pNode               瓦片节点指针
	emptyOrload         是否开始加载(FALSE-开始加载,TRUE-不加载)
输出：
    无
返回值：
    -1：节点不存在或者子节点创建失败
     0：子节点不在数据树里
     4：子节点创建成功
*/
static f_int32_t qtmapnodeCreateChildren(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, BOOL emptyOrload )
{
	f_int32_t i;
	/* 孩子节点指针 */
	sQTMAPNODE * pChildNode;
	/* 子树指针 */
	sQTMAPSUBTREE * pTree = NULL;
	/* 记录成功创建的孩子节点个数 */
	f_int32_t ret = 0;

	/* 判断输入的瓦片节点是否为空且key值是否正确 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeCreateChildren with error node\n");
		return -1;
	}

	/* 四叉树,每个瓦片节点有4个孩子节点,依次进行判断 */
	for (i = 0; i < 4; i++)
	{
		f_int32_t level, xidx, yidx;
		/* 当前瓦片节点的第i个孩子节点不为空 */
		if ((pNode->children[i] != NULL))
		{
			/* 当孩子节点是SLEEP状态(表明该孩子节点已创建但未加载数据),并且需要加载数据时,改变孩子节点状态,*/
			/* 三维加载任务会根据该状态对该孩子节点进行数据加载 */
			if( (emptyOrload == FALSE)
				&&( DNST_SLEEP == qtmapnodeGetStatus(pNode->children[i], __LINE__) ) )
			{
 				qtmapnodeSetStatus(pNode->children[i], DNST_WAITFORLOAD, __LINE__);
				//printf("level = %d, x = %d, y = %d, change node state\n",pNode->children[i]->level, pNode->children[i]->xidx, pNode->children[i]->yidx);
			}
			
			/* 创建的孩子节点个数加1,继续判断下一个孩子节点 */
			ret ++;
			continue;			
		}
		
		/* 当前瓦片节点的第i个孩子节点为NULL,则创建孩子节点 */
		/* 计算孩子节点的层级,x方向索引,y方向索引*/
		level = pNode->level + 1;
		xidx = pNode->xidx * 2 + (i & 1);	// 对应于lon方向(横轴方向)
		yidx = pNode->yidx * 2 + (i / 2);	// 对应于lat方向
		
		// 始终创建子节点，不论子树上有没有
		{
			/* 判断当前瓦片节点的第i个孩子节点是否在地图数据子树的链表中,并返回所在子树链表头节点指针 0-不在 1-在 */
			/* 若不在,则所有孩子节点无法创建,直接返回0 */
			if (isNodeExistInMapSubreeList(pHandle, level, xidx, yidx, &pTree) == 0)
				return 0;
		}

		/* 创建当前瓦片节点的第i个孩子节点 0-成功 -1-失败 */
		/* 若创建失败,其他孩子节点也无需再创建,直接返回-1 */
		if (qtmapnodeCreate(pHandle, level, xidx, yidx, pTree, &pChildNode, emptyOrload)<0)
			return -1;
		else
		{
			/* 维护节点上下关系:若创建成功,记录当前瓦片节点的孩子节点,记录孩子节点的父节点,形成四叉树 */
			pNode->children[i] = pChildNode;
			pChildNode->parent = pNode;
			/* 创建的孩子节点个数加1,一个瓦片必然有4个孩子节点,正常ret的值必然为4*/
			ret ++;
		}
	}
	
	return(ret);
}

/*
功能：地图句柄创建
输入：
    无
输出：创建成功标志
    0  成功
    -1 失败
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
	
	/* 生成瓦片顶点的索引 */
	generateTileIndices();
	/* 生成瓦片顶点的纹理坐标 */
	generateTexBuffer();
	/* 生成鹰眼模式瓦片顶点的索引 */
	generateTileIndicesEyeBird();
	/* 生成鹰眼模式瓦片顶点的纹理坐标 */
	generateTexBufferEyeBird();

    memset(pHandle, 0, sizeof(sMAPHANDLE));
	pHandle->key = MAPHANDLEKEY;

    {
		stList_Head *pstListHead = NULL;
		/**全局子树链表初始化**/
		pstListHead = &(pHandle->sub_tree.stListHead);
		LIST_INIT(pstListHead);
		/**缓冲区中瓦片节点链表初始化**/
		pstListHead = NULL;
		pstListHead = &(pHandle->nodehead.stListHead);
		LIST_INIT(pstListHead);
    }	    
	
	/* 创建二值信号量,用于绘制任务告知三维数据加载任务有新创建的瓦片节点 */
	pHandle->anyrequire    = createBSem(Q_FIFO_SEM, FULL_SEM);
	pHandle->endrequire    = createBSem(Q_FIFO_SEM, EMPTY_SEM);
	pHandle->threadended   = createBSem(Q_FIFO_SEM, EMPTY_SEM);
	pHandle->rendscenelock = createBSem(Q_FIFO_SEM, FULL_SEM);
	/* 创建二值信号量,用于绘制任务与三维数据加载任务同时访问qtn相关数据时的互斥问题 */
	pHandle->qtnthread = createBSem(Q_FIFO_SEM, FULL_SEM);
    // 有些数据需要保证一帧完整性(即获取一帧完全执行完的状态,如果统计瓦块绘制总数时)
	pHandle->rendscendDrawlock = createBSem(Q_FIFO_SEM, FULL_SEM);
	giveSem(pHandle->rendscenelock);
	giveSem(pHandle->qtnthread);
	giveSem(pHandle->rendscendDrawlock);

#ifdef ACOREOS
	{
		ACoreOs_id tstTaskid;
		/* 启动三维数据加载任务 */
		//pHandle->loadthread    = spawnTask("imgloadthread", 103, FP_TASK_VX, 0x200000, qtmapDataLoadRoute, (f_int64_t)pHandle);
		spawnTaskTM3(0, 103, 0x200000, "imgloadthread", qtmapDataLoadRoute, (f_int32_t)pHandle, &tstTaskid);
		pHandle->loadthread = (f_int32_t)tstTaskid;
	}

#else

	/* 启动三维数据加载任务 */
#ifdef TASKAFFINITY
	pHandle->loadthread = spawnTaskAffinity(1, "imgloadthread", 103, FP_TASK_VX, 0x200000, qtmapDataLoadRoute, (f_int64_t)pHandle);
#else
	pHandle->loadthread = spawnTask("imgloadthread", 95, FP_TASK_VX, 0x200000, qtmapDataLoadRoute, (f_int64_t)pHandle);
#endif

#endif
	/* 创建的地图句柄赋值给全局的地图场景树 */
	pMapDataSubTree = pHandle;

    return 0;	
}

/*
	设置各个图层的功能函数
	参数：map_andle -- 用createMapHandle返回的VOIDPtr句柄
	      layer -- 图层号, 0..MAXMAPLAYER-1
		  param -- 功能函数的调用参数，在调用各个功能函数时传入该参数
		  funccreate -- 对应图层的结点生成函数
		  funcdestroy -- 对应图层的结点数据删除函数
		  funcdisplay -- 对应图层的结点显示函数
		  funcdisplaypre -- 对应图层显示前的处理函数
		  funcdisplaypro -- 对应图层显示后的处理函数
    返回值：
	      0 -- 成功
		 -1 -- 地图数据句柄无效
		 -2 -- 图层号无效
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
功能：生成地图数据子树的链表，在当前已经存在的结点树上增加一棵子树，其中，<level, xidx, yidx>为子树的
	根结点位置，levels为子树的高度。注意，每棵增加的子树都是完全子树（每个结点都存在）。用户
	可以多次调用该函数生成一棵非完全的四叉树。
	参数：map_andle -- 用createMapHandle返回的VOIDPtr句柄
		  level -- 层号 0 ~ 40
		  xidx, yidx -- 在该层中的结点编号
		  levels -- 子树的高度
		  funcLoadTerrain -- 得到地形数据的函数
		  loadterrainparam -- 地形数据的参数
		  infoloader   获取瓦片信息函数
		  infoparam    瓦片参数
		  imgisexist   判断瓦片是否存在的函数
		  imgloader -- 得到表面图像数据的函数
		  img2tex -- 表面图像数据变成OpenGL纹理的函数
		  imgparam -- 表面图像数据处理时带的参数
    返回值：
	      0 -- 成功
		 -1 -- 地图数据句柄无效
		 -2 -- 参数无效
		 -3 -- 内存分配失败
		 -4 -- 父结点没有调入
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

	/* 起始层级level必须大于等于0，总层级levels必须大于等于1，起始瓦片x方向索引xidx在[0,2^(level+2))，起始瓦片y方向索引yidx在[0,2^(level+1)) */
	if ((level < 0) || (levels < 1) || (xidx < 0) || (yidx < 0) || (xidx >= (1 << (level+2))) || (yidx >= (1 << (level+1))))
		return -2;
	
	pHandle = (sMAPHANDLE *)map_andle;
	/* 如果起始层级大于0，则其必然可以找到一棵根子树进行挂接，如果找不到，则不会真正使用 */
	/* 如果起始层级等于0，则必然是8棵根子树，无需判断，直接设置各类函数指针即可 */
	if (level > 0)
	{
		/* 判断子树根节点的父节点是否存在于上一级的树中，如果不在，直接丢弃该子树 */
		/* 判断子树根节点的起始节点(level,xidx,yidx)，则其父节点必然是(level-1,xidx/2,yidx/2) */
		if (!isNodeExistInMapSubreeList(pHandle, level - 1, xidx / 2, yidx / 2, NULL))
			return -4;
	}
	/* 申请内存空间用于存放子树结构体sQTMAPSUBTREE */
	pSubTree = (sQTMAPSUBTREE *)NewFixedMemory(sizeof(sQTMAPSUBTREE));
	if (pSubTree == NULL)
		return -3;

	/* 设置子树的起始层级、起始瓦片x方向索引、起始瓦片y方向索引、总层级 */
	pSubTree->level = level;
	pSubTree->xidx = xidx;
	pSubTree->yidx = yidx;
	pSubTree->levels = levels;

	/* 设置子树的ter数据读取函数及该函数参数 */
	pSubTree->funcloadterrain  = funcLoadTerrain;
	pSubTree->loadterrainparam = loadterrainparam;
	/* 设置子树的info数据读取函数及该函数参数 */
	pSubTree->infoloader       = infoloader;
	pSubTree->infoparam        = infoparam;
	/* 设置子树的指定瓦片是否存在当前子树中的判断函数 */
	pSubTree->imgisexist       = imgisexist;
	/* 设置子树的tqs数据读取函数、tqs数据生成纹理的函数、tqs数据读取函数参数 */
	pSubTree->imgloader        = imgloader;
	pSubTree->img2tex          = img2tex;
	pSubTree->imgparam         = imgparam;
	/* 设置子树的vqt数据读取函数、vqt数据生成纹理的函数、vqt数据读取函数参数 */
	pSubTree->vqtloader	       = vqtloader;
	pSubTree->vqt2tex		   = vqt2tex;
	pSubTree->vqtparam	       = vqtparam;
	/* 设置子树的crh数据读取函数、crh数据生成纹理的函数、crh数据读取函数参数 */
	pSubTree->clrloader	       = clrloader;
	pSubTree->clr2tex		   = clr2tex;
	pSubTree->clrparam	       = clrparam;
	/* 设置子树的hil数据读取函数、hil数据生成纹理的函数、hil数据读取函数参数 */
	pSubTree->hilloader	       = hilloader;
	pSubTree->hil2tex		   = hil2tex;
	pSubTree->hilparam	       = hilparam;
	
    {
        /* 将该子树加入到全局的子树链表中，后加入的在链表的前部 */
		stList_Head *pstListHead = NULL;
		pstListHead = &(pHandle->sub_tree.stListHead);
		LIST_ADD(&pSubTree->stListHead, pstListHead);	
    }	
	
	return 0;
}

/*
功能：瓦片SVS模式下绘制函数，只绘制SVS模式，无叠加方里网和水平告警纹理

输入：
	pHandle              地图场景树指针
	pNode                瓦片节点指针
	pScene               渲染场景指针

输入输出：
	无

输出：
    无

返回值：
    0   正常绘制
	-1  瓦片节点句柄无效
	-2  地图场景树无效
*/
static f_int32_t mapNodeRenderImgSVS(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 检查图像是否就绪 */
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
功能：瓦片VecShade模式下绘制函数

输入：
	pHandle              地图场景树指针
	pNode                瓦片节点指针
	pScene               渲染场景指针

输入输出：
	无

输出：
    无

返回值：
    0   正常绘制
	-1  瓦片节点句柄无效
	-2  地图场景树无效
*/
static f_int32_t mapNodeRenderVecImg(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 判断输入的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}

	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* 判断当前渲染场景的场景模式 */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* 默认为正常模式 */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* 计算所有绘制瓦片节点的最小层级 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* 使能纹理单元0,关闭一维纹理,开启二维纹理 */
	glActiveTexture(GL_TEXTURE0);
	//glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
	    
	/* 设置纹理单元0的纹理id及纹理变换矩阵,纹理单元0为VecShade纹理 */
	RenderShadeMapPre(pScene);

	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定纹理,设置纹理矩阵 */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* 关闭纹理单元1的一维纹理和二维纹理使能开关 */
		glActiveTexture(GL_TEXTURE1);
	    //glDisableEx(GL_TEXTURE_1D);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* 使能纹理单元0 */
	glActiveTexture(GL_TEXTURE0);

	/* 恢复矩阵堆栈为模型视图矩阵 */
	glMatrixMode(GL_MODELVIEW);	
	/* 将原来的模型视图矩阵进行压栈保存 */
	glPushMatrix();

#ifndef _JM7200_
	/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标，此处矩阵无需变换 */
	/* 为了使用整型化后的瓦片顶点，调整模型视图矩阵，设置基准点xyz和缩放系数 */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif

	/* 设置颜色为白色,透明度为完全不透明 */
	glColor4f(1.0f,1.0f,1.0f, 1.0f);	

	/* 如果强制使用显示列表(useArray=0时),则使用显示列表的方式绘制 */
	/* 否则通过顶点数组的方式绘制 */
	if((useArray == 0))	
    {
#ifdef _JM7200_
		/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标 ；俯视下仍用的是整数化的局部坐标，故需变换矩阵*/
		CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif
        qtmapNodeCreateVecShadeList(pNode, pScene->is_need_terwarning, pScene->mdctrl_cmd.scene_mode);

		if((pNode->tileVecShadeList) != 0)
			glCallList(pNode->tileVecShadeList);
			
    }
	else
	{
		/* 设置瓦片的顶点数组 */
		glEnableClientState(GL_VERTEX_ARRAY);	

	#ifdef _JM7200_
		/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标 */
		glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);
	#else
		/* 使用整型的顶点 */
		glVertexPointer(3, GL_INT, 0, pNode->fvertex_part);	
	#endif

		/* 设置瓦片顶点纹理0的纹理坐标数组,即VecShade模式的纹理 */
		RenderShadeMap(pScene, pNode);
	
		/* 如果需要地形告警,则设置瓦片顶点纹理1的纹理坐标数组 */
		if(pScene->is_need_terwarning)
		{
			RenderTerrainWarningStripeColor2(pScene, pNode);
		}

		/* 绘制瓦片顶点,以三角形条带的方式绘制 */
		glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);
    
		/* 关闭纹理数组和顶点数组的使能,与Enable对应 */
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);		
	}

	/* 如果需要地形告警,则恢复纹理1的状态 */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

	glDisableEx(GL_TEXTURE_2D);

	/* 恢复纹理0的状态及纹理矩阵 */
	RenderShadeMapPro();

	
#if 0
	/* 正常模式下绘制瓦片边线的显示列表，鹰眼模式不绘制 */
	if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
	{	
		/* 设置瓦片边线的颜色 */
		glColor3f(0.5f, 0.8f, 0.8f);
		/* 创建正常模式下瓦片边线的显示列表 */
		qtmapNodeCreateEdgeList(pNode);

		/* 开启线平滑 */
		glEnableEx(GL_LINE_SMOOTH);
		/* 关闭深度测试,防止线被遮挡 */
		glDisableEx(GL_DEPTH_TEST);

		/* 调用正常模式下瓦片边线显示列表绘制 */
		if((pNode->edgeList) != 0)
			glCallList(pNode->edgeList);
		
		glDisableEx(GL_LINE_SMOOTH);
		glEnableEx(GL_DEPTH_TEST);

	}
#endif

	/* 从栈中恢复原来的模型视图矩阵 */
	glPopMatrix();

#if 0
{
	PT_3D nodexyz = {0};
	sColor4f color1={0.0};
	f_char_t timeInfo[128] = {0};
	Geo_Pt_D geoPt;
	PT_2I screenPt;

	/* 设置瓦片层级及索引编号文字绘制的颜色,使用红色完全不透明 */
	color1.red = 1.0;
	color1.alpha = 1.0;


	//	nodexyz = pNode->obj_cent;
	//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

	/* 计算文字绘制点的经纬高,考虑到字体有宽度,为了让字体保持在中心,设置点在瓦片的左1/4中心,高度为平均高度 */
	/* 未考虑旋转 */
	geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
	geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
	geoPt.height = pNode->height_ct;

	//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
	/* 根据经纬高计算屏幕坐标 */
	getOuterViewPtByGeo(pScene, geoPt, &screenPt);


	/* 关闭深度测试,防止文字被遮挡 */
	glDisableEx(GL_DEPTH_TEST);
	//	glDisableEx(GL_CULL_FACE);
	/* 在屏幕上绘制瓦片层级及索引编号 */
	sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
	textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

	//	glEnableEx(GL_CULL_FACE);	
	glEnableEx(GL_DEPTH_TEST);
}
#endif

#endif
	/* 地图场景树中绘制的节点个数自增1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* 记录节点上一次绘制的帧计数 */
    pNode->lastDrawNumber = pHandle->drawnumber;
	return 0;
}



/*
功能：瓦片VecShade模式下绘制函数(OpenGL ES)

输入：
	pHandle              地图场景树指针
	pNode                瓦片节点指针
	pScene               渲染场景指针

输入输出：
	无

输出：
    无

返回值：
    0   正常绘制
	-1  瓦片节点句柄无效
	-2  地图场景树无效
*/
static f_int32_t mapNodeRenderVecImgEs(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 判断输入的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}

	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* 判断当前渲染场景的场景模式 */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* 默认为正常模式 */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* 计算所有绘制瓦片节点的最小层级 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* 使能纹理单元0,关闭一维纹理,开启二维纹理 */
	glActiveTextureARB(GL_TEXTURE0);
	glEnableEx(GL_TEXTURE_2D);
	    
	/* 设置纹理单元0的纹理id及纹理变换矩阵,纹理单元0为VecShade纹理 */
	RenderShadeMapPre(pScene);

	/* 设置纹理单元0的纹理组合模式为调整GL_MODULATE,即最终纹理的RGBA=纹理源的RGBA*片段的RGBA */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定纹理,设置纹理矩阵 */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* 关闭纹理单元1的一维纹理和二维纹理使能开关 */
		glActiveTextureARB(GL_TEXTURE1);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* 使能纹理单元0 */
	glActiveTextureARB(GL_TEXTURE0);

	/* 恢复矩阵堆栈为模型视图矩阵 */
	glMatrixMode(GL_MODELVIEW);	
	/* 将原来的模型视图矩阵进行压栈保存 */
	glPushMatrix();

#ifndef VERTEX_USE_FLOAT
	/* 使用整形的顶点数据，需先移动模型视图矩阵，移动到偏移起始点 */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0],pNode->point_vertex_xyz[1],pNode->point_vertex_xyz[2],pNode->point_vertex_xyz[3]);
#endif	

	/* 设置颜色为白色,透明度为完全不透明 */
	glColor4f(1.0f,1.0f,1.0f, 1.0f);	

	{
		GLint prog = 0;
		int vLoc, tLoc, tLoc1, nLoc, cLoc;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation((GLuint)prog, "a_position");
		tLoc = glGetAttribLocation((GLuint)prog, "a_texCoord0");
		tLoc1 = glGetAttribLocation((GLuint)prog, "a_texCoord1");

		/*创建瓦片的顶点绘制VBO(顶点坐标/顶点高度)，VecShade模式下所有视角都是绘制33*33个顶点，每个瓦片的VBO中的数据不一样*/
		qtmapNodeCreateVboEs(pNode);
		/*创建瓦片的顶点绘制VBO(顶点索引)，所有瓦片的这些VBO中的数据均一样*/
		genVecShadeConstVBOs(&ibo, &tvbo);

		{
			//glDisableEx(GL_CULL_FACE);    // fix the warning function under overlook viewmode

			/*每个瓦片按33*33个顶点绘制成三角形条带*/
			glActiveTextureARB(GL_TEXTURE0);
			/*设置顶点坐标*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif

			/*设置顶点纹理0坐标，VecShade纹理*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
			glVertexAttribPointer(tLoc, 1, GL_FLOAT, 0, 0, 0);

			if (pScene->is_need_terwarning)/* 需要地形告警 */
			{
				/*设置顶点纹理1坐标，水平告警纹理*/
				glEnableVertexAttribArray(tLoc1);
				glActiveTextureARB(GL_TEXTURE1);
				glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
				glVertexAttribPointer(tLoc1, 1, GL_FLOAT, 0, 0, 0);
			}
			/*设置顶点索引，并绘制所有顶点*/
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

	/* 恢复纹理0的状态及纹理矩阵 */
	RenderShadeMapPro();

	/* 从栈中恢复原来的模型视图矩阵 */
	glPopMatrix();

#endif

	/* 地图场景树中绘制的节点个数自增1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* 记录节点上一次绘制的帧计数 */
    pNode->lastDrawNumber = pHandle->drawnumber;
	return 0;
}

/*
功能：瓦片正常模式下绘制函数

输入：
	pHandle              地图场景树指针
	pNode                瓦片节点指针
	pScene               渲染场景指针

输入输出：
	无

输出：
    无

返回值：
    0   正常绘制
	-1  瓦片节点句柄无效
	-2  地图场景树无效
	-3  瓦片节点的二维影像纹理创建失败
*/
#define Do_FileTextureVertexData 1	// 将0层数据放入文件并读出
static f_int32_t mapNodeRenderImg(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 判断输入的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    return -2;
	}
	
	/* 判断当前渲染场景的场景模式 */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	    case eScene_normal:
	        count = index_count;
	        pIndex = gindices;
	        pTex = gTextureBuf;
			break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
 	    case eScene_eyebird:
 	        count = index_count_eyebird;
 	        pIndex = indices_eyebird;
 	        pTex = gTextureBuf_eyebird;
 			break;	
		/* 默认为正常模式 */
		default:
			count = index_count;
			pIndex = gindices;
			pTex = gTextureBuf;
			break;
	}
	
	/* 计算所有绘制瓦片节点的最小层级 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* 使能纹理单元0,关闭一维纹理,开启二维纹理 */
    glActiveTexture(GL_TEXTURE0);
    //glDisableEx(GL_TEXTURE_1D);
    glEnableEx(GL_TEXTURE_2D);
    
	/* 第一次则生成瓦片节点的二维影像纹理,之前已经生成则不再重复生成 */
	if ( (0 == (pNode->texid)) && (pNode->imgdata != NULL))
	{
		// 断层模式,引用最近一层在文件树中的父节点纹理
#ifdef USE_FAULT_Tile
		// 节点tqs不在文件树中,直接引用父纹理
		if(pNode->tqsInFile == 0)
		{
			pNode->texid = pNode->tqsParentInFile->texid;
		}
		// 节点tqs在文件树中,则构建该节点纹理
		else
		{
			pNode->texid = pNode->pSubTree->img2tex(pNode->pSubTree->imgparam, pNode->imgdata);
			pHandle->createTexNumber++;
		}
#else
		/* 调用挂接函数,将瓦片的影像纹理数据生成一个二维纹理 */
		pNode->texid = pNode->pSubTree->img2tex(pNode->pSubTree->imgparam, pNode->imgdata);
		/* 地图场景树中的创建纹理计数则增加1 */
        pHandle->createTexNumber++;
#endif
	}

	// 断层模式,使用一次父纹理增加一次引用
	#ifdef USE_FAULT_Tile
		if(pNode->tqsInFile == 0)
			pNode->tqsParentInFile->tqsUsedNumInChild++;// 增加对父纹理的引用
	#endif
	
	/* 生成瓦片节点的二维影像纹理失败,直接返回 */
	if (0 >= (pNode->texid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -3;
	}

	/* 根据场景绘制句柄中的底图显示模式设置内部的图层叠加模式 */
	setLayDisplay(pScene);

	/* 根据输入的第0图层叠加模式,判断当前帧使用的底图纹理是否与上一帧使用的不一致,如果不同则进行绑定纹理及切换纹理的操作 */
	bindAndSubRenderTexID(pNode, 0);	// 绑定纹理数据

	#if Do_FileTextureVertexData
	{
		// 读取绑定纹理数据
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

		
	/* 设置纹理单元0的纹理组合模式为调整GL_MODULATE,即最终纹理的RGBA=纹理源的RGBA*片段的RGBA */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定纹理,设置纹理矩阵 */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* 关闭纹理单元1的一维纹理和二维纹理使能开关 */
		glActiveTexture(GL_TEXTURE1);
	    //glDisableEx(GL_TEXTURE_1D);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* 使能纹理单元0 */
	glActiveTexture(GL_TEXTURE0);
	
	/* 恢复矩阵堆栈为模型视图矩阵 */
	glMatrixMode(GL_MODELVIEW);	
	/* 将原来的模型视图矩阵进行压栈保存 */
	glPushMatrix();

#ifndef _JM7200_
	/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标，此处矩阵无需变换 */
	/* 为了使用整型化后的瓦片顶点，调整模型视图矩阵，设置基准点xyz和缩放系数 */
	/* 使用整型化后的瓦片顶点的好处：由于float经度不够，导致飞行过程中地形会出现抖动，改成整形精度就够了 */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif

	/* 设置颜色为白色,透明度为完全不透明 */
	glColor4f(1.0f,1.0f,1.0f,1.0f);	

	/* 如果强制使用显示列表,则使用显示列表的方式绘制 */
	/* 其他情况通过顶点数组的方式绘制 */
	if((useArray == 0)/*||(
			(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn))*/)	
    {

#ifdef _JM7200_
		/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标；俯视下仍用的是整数化的局部坐标，故需变换矩阵*/
		CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);
#endif
		/* 创建瓦片的正常绘制显示列表(绘制所有瓦片顶点(33*33)和俯视下绘制的显示列表(仅绘制瓦片的4个顶点) */
        qtmapNodeCreateList(pNode, pScene->is_need_terwarning, pScene->mdctrl_cmd.scene_mode);			
		qtmapNodeCreateListOverlook(pNode, pScene->is_need_terwarning, pScene->mdctrl_cmd.scene_mode);

		if(	(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
		{
			//* 视角为俯视且未开启高度预警功能，调用俯视下的显示列表绘制 */
			if((pNode->tileListOverlook) != 0)
				glCallList(pNode->tileListOverlook);	
		}
		else
		{
			/* 视角为俯视但开启高度预警功能、或则视角为非俯视视角，调用正常显示列表绘制 */
		    if((pNode->tileList) != 0)
    			glCallList(pNode->tileList);
		}

		//glFinish();

    }
	else
	{	
		/* 设置瓦片的顶点数组 */
		glEnableClientState(GL_VERTEX_ARRAY);
#ifdef _JM7200_
		/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标 */
		//glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	// gx test

		// gx test for Debug 写读瓦块顶点数据 33*33
	#if Do_FileTextureVertexData
		{
			// gx test for Debug 写瓦块顶点数据 33*33
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

			// gx test for Debug 读瓦块顶点数据 33*33
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

		// gx test for Debug 读纹理数据
	#if Do_FileTextureVertexData
		{
			// 读取纹理坐标
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


			// 写纹理数据
			{
				if (pNode->level == 0)
				{
					char filePath[256]={0};
					FILE *fp = NULL;
					sprintf(filePath, "D:\\ImgData\\img_lvl_%d_xidx_%d_yidx_%d.txt", pNode->level, pNode->xidx, pNode->yidx);
					fp = fopen(filePath, "wb");
					// 创建文件并打开
					if (fp == NULL) {
						printf("Failed to create file!\n");
						return 1;
					}
					// 写入数据
					fwrite(pNode->imgdata, sizeof(unsigned char), TILESIZE * TILESIZE * 3, fp);
					// 关闭文件
					fclose(fp);
				}
			}
		}
	#endif




#else
		/* 使用浮点型的顶点 */
//		glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	
		/* 使用整型的顶点 */
		glVertexPointer(3, GL_INT, 0, pNode->fvertex_part);	
#endif

		/* 设置瓦片顶点纹理0的纹理坐标数组 */
		glClientActiveTexture(GL_TEXTURE0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, pTex);



		/* 如果需要地形告警,则设置瓦片顶点纹理1的纹理坐标数组 */
		if(pScene->is_need_terwarning)
		{
		    RenderTerrainWarningStripeColor2(pScene, pNode);
		}

		/* 设置瓦片顶点纹理2的纹理坐标数组,目前未启用,注释掉,且JM7200不支持三重纹理 */
		//glClientActiveTexture(GL_TEXTURE2);                
		//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		//glTexCoordPointer(2, GL_FLOAT, 0, pTex);	

		/* 绘制瓦片顶点,以三角形条带的方式绘制 */
		//glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);	// gx test

#if Do_FileTextureVertexData	// gx test for Debug 使用输出数据绘制瓦块
		{
			// 读取顶点索引文件
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

		/* 关闭纹理数组和顶点数组的使能,与Enable对应 */
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);	

	//	glFinish();

	}

	/* 如果需要地形告警,则恢复纹理1的状态 */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

#if 0
	/* 恢复纹理2的状态 */
	glActiveTexture(GL_TEXTURE2);					        // 操作纹理单元1
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);					// 操作纹理单元0

#endif

	glDisableEx(GL_TEXTURE_2D);

	if(bDrawTileGrid == 1)
	{
		/* 正常模式下绘制瓦片边线的显示列表，鹰眼模式不绘制 */
		if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
		{	
			/* 设置瓦片边线的颜色 */
			glColor3f(0.5f, 0.8f, 0.8f);
			/* 创建正常模式下瓦片边线的显示列表 */
			qtmapNodeCreateEdgeList(pNode);
			/* 创建俯视下瓦片边线的显示列表 */
			qtmapNodeCreateEdgeListOverlook(pNode);
	
			/* 开启线平滑 */
			glEnableEx(GL_LINE_SMOOTH);
			/* 关闭深度测试,防止线被遮挡 */
			glDisableEx(GL_DEPTH_TEST);

			if(	(pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
				&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
			{
				//* 视角为俯视且未开启高度预警功能，调用俯视下的瓦片边线显示列表绘制 */
				if((pNode->edgeListOverlook) != 0)
					glCallList(pNode->edgeListOverlook);
			}
			else
			{
				/* 视角为俯视但开启高度预警功能、或则视角为非俯视视角，调用正常模式下瓦片边线显示列表绘制 */
				if((pNode->edgeList) != 0)
					glCallList(pNode->edgeList);
			}
		
			glDisableEx(GL_LINE_SMOOTH);
			glEnableEx(GL_DEPTH_TEST);
		
		}

	}

	/* 从栈中恢复原来的模型视图矩阵 */
	glPopMatrix();


	// 绘制瓦块名|瓦块号
#if 1
	if(bDrawTileName == 1)
	{
		PT_3D nodexyz = {0};
		sColor4f color1={0.0};
		f_char_t timeInfo[128] = {0};
		Geo_Pt_D geoPt;
		PT_2I screenPt;
	
		/* 设置瓦片层级及索引编号文字绘制的颜色,使用红色完全不透明 */
		color1.red = 1.0;
		color1.alpha = 1.0;

	
	//	nodexyz = pNode->obj_cent;
	//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

		/* 计算文字绘制点的经纬高,考虑到字体有宽度,为了让字体保持在中心,设置点在瓦片的左1/4中心,高度为平均高度 */
		/* 未考虑旋转 */
		{
			geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
			geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
			geoPt.height = pNode->height_ct;
		}

	//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
		/* 根据经纬高计算屏幕坐标 */
		getOuterViewPtByGeo(pScene, geoPt, &screenPt);


		/* 关闭深度测试,防止文字被遮挡 */
		glDisableEx(GL_DEPTH_TEST);
	//	glDisableEx(GL_CULL_FACE);
		/* 在屏幕上绘制瓦片层级及索引编号 */

		sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
		textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

	//	glEnableEx(GL_CULL_FACE);	
		glEnableEx(GL_DEPTH_TEST);
	}
#endif

	/* 地图场景树中绘制的节点个数自增1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* 记录节点上一次绘制的帧计数 */
    pNode->lastDrawNumber = pHandle->drawnumber;
#endif

	return 0;
}


/*
功能：瓦片正常模式下绘制函数(OpenGL ES)

输入：
	pHandle              地图场景树指针
	pNode                瓦片节点指针
	pScene               渲染场景指针

输入输出：
	无

输出：
    无

返回值：
    0   正常绘制
	-1  瓦片节点句柄无效
	-2  地图场景树无效
	-3  瓦片节点的二维影像纹理创建失败
*/
static f_int32_t mapNodeRenderImgEs(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 判断输入的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    return -2;
	}
	
	/* 判断当前渲染场景的场景模式 */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	    case eScene_normal:
	        count = index_count;
	        pIndex = gindices;
	        pTex = gTextureBuf;
			break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
 	    case eScene_eyebird:
 	        count = index_count_eyebird;
 	        pIndex = indices_eyebird;
 	        pTex = gTextureBuf_eyebird;
 			break;	
		/* 默认为正常模式 */
		default:
			count = index_count;
			pIndex = gindices;
			pTex = gTextureBuf;
			break;
	}
	
	/* 计算所有绘制瓦片节点的最小层级 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* 使能纹理单元0,关闭一维纹理,开启二维纹理 */
    glActiveTextureARB(GL_TEXTURE0);
    glEnableEx(GL_TEXTURE_2D);
    
	/* 第一次则生成瓦片节点的二维影像纹理,之前已经生成则不再重复生成 */
	if ( (0 == (pNode->texid)) && (pNode->imgdata != NULL))
	{
		/* 调用挂接函数,将瓦片的影像纹理数据生成一个二维纹理 */
		pNode->texid = pNode->pSubTree->img2tex(pNode->pSubTree->imgparam, pNode->imgdata);
		/* 地图场景树中的创建纹理计数则增加1 */
        pHandle->createTexNumber++;
	}

	/* 生成瓦片节点的二维影像纹理失败,直接返回 */
	if (0 >= (pNode->texid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -3;
	}

	/* 二维矢量纹理暂时不使用,注释掉 */
#if 0
	/* 第一次则生成瓦片节点的二维矢量纹理,之前已经生成则不再重复生成 */
	if ( (0 == (pNode->vtexid)) && (pNode->vqtdata != NULL))
	{
		/* 调用挂接函数,将瓦片的矢量纹理数据生成一个二维纹理 */
		pNode->vtexid = pNode->pSubTree->vqt2tex(pNode->pSubTree->vqtparam, pNode->vqtdata);
		/* 地图场景树中的创建纹理计数则增加1 */
        pHandle->createTexNumber++;
	}

	/* 生成瓦片节点的二维矢量纹理失败,直接返回 */
	if (0 >= (pNode->vtexid))
	{
		//printf("mapNodeRenderImg texid invalid: %d, %d, %d\n", pNode->level, pNode->xidx, pNode->yidx);
		return -1;
	}
#endif

	/* 为节省帧存,其他底图纹理不创建纹理id,复用影像纹理id,采用替换子纹理的方式更新纹理数据 */
#if 0
	/* 第一次则生成瓦片节点的二维clr纹理,之前已经生成则不再重复生成 */
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

	/* 第一次则生成瓦片节点的二维hil纹理,之前已经生成则不再重复生成 */
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


	/* 根据场景绘制句柄中的底图显示模式设置内部的图层叠加模式 */
	setLayDisplay(pScene);

	/* 根据输入的第0图层叠加模式,判断当前帧使用的底图纹理是否与上一帧使用的不一致,如果不同则进行绑定纹理及切换纹理的操作 */
	bindAndSubRenderTexID(pNode, 0);
		
	/* 设置纹理单元0的纹理组合模式为调整GL_MODULATE,即最终纹理的RGBA=纹理源的RGBA*片段的RGBA */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定纹理,设置纹理矩阵 */
	    RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* 关闭纹理单元1的一维纹理和二维纹理使能开关 */
		glActiveTextureARB(GL_TEXTURE1);
	    glDisableEx(GL_TEXTURE_2D);
	}

	/* 暂时不需要叠加第二层图层,注释该功能,且JM7200不支持三重及以上的纹理叠加 */
#if 0
	/* 叠加第二层图层(矢量纹理或阴影纹理)begin */

	/* 使能纹理单元2 */
	glActiveTexture(GL_TEXTURE2);
	/* 设置纹理单元0的纹理组合模式为贴花GL_DECAL,即最终纹理的RGB=纹理源的RGB*(1-纹理源的A)+片段的RGB*纹理源的A,最终纹理的A=片段的A */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	/* 关闭一维纹理,开启二维纹理 */
	//glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);

	/* 根据输入的第1图层叠加模式,判断当前帧使用的底图纹理是否与上一帧使用的不一致,如果不同则进行绑定纹理及切换纹理的操作 */
	bindAndSubRenderTexID(pNode, 1);

	/* 叠加第二层图层end */
#endif

	/* 使能纹理单元0 */
	glActiveTextureARB(GL_TEXTURE0);
	
	/* 恢复矩阵堆栈为模型视图矩阵 */
	glMatrixMode(GL_MODELVIEW);	
	/* 将原来的模型视图矩阵进行压栈保存 */
	glPushMatrix();
#ifndef VERTEX_USE_FLOAT
	/* 使用整形的顶点数据，需先移动模型视图矩阵，移动到偏移起始点 */
	/* 使用整型化后的瓦片顶点的好处：由于float经度不够，导致飞行过程中地形会出现抖动，改成整形精度就够了 */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0],pNode->point_vertex_xyz[1],pNode->point_vertex_xyz[2],pNode->point_vertex_xyz[3]);
#endif	
	/* 设置颜色为白色,透明度为完全不透明 */
	glColor4f(1.0f,1.0f,1.0f,1.0f);	

	{
		GLint prog = 0;
		int vLoc, tLoc, tLoc1;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		vLoc = glGetAttribLocation((GLuint)prog, "a_position");
		tLoc = glGetAttribLocation((GLuint)prog, "a_texCoord0");
		tLoc1 = glGetAttribLocation((GLuint)prog, "a_texCoord1");

		/*创建瓦片的座舱视角绘制VBO(顶点坐标/顶点高度)和俯视视角绘制的VBO(顶点坐标，即4个角点的坐标)，每个瓦片的VBO中的数据不一样*/
		qtmapNodeCreateVboEs(pNode);
		/*创建瓦片的座舱视角绘制VBO(顶点索引/顶点纹理坐标)和俯视视角绘制的VBO(顶点索引/顶点纹理坐标)，所有瓦片的这些VBO中的数据均一样*/
		genConstVBOs(&ibo, &tvbo, &tvbo_overlook, &ibo_overlook);


		if ( (pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
			&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn) )
		{
			/*俯视视角，不水平告警时，每个瓦片只需绘制成2个三角形即可*/
			//glDisableEx(GL_CULL_FACE);
			/*设置顶点坐标*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif
			/*设置顶点纹理坐标*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, tvbo_overlook);
			glVertexAttribPointer(tLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
			/*设置顶点索引，并进行绘制*/
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

			/*其他情况，每个瓦片按33*33个顶点绘制成三角形条带*/
			glActiveTextureARB(GL_TEXTURE0);
			/*设置顶点坐标*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif
			/*设置顶点纹理0坐标*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, tvbo);
			glVertexAttribPointer(tLoc, 2, GL_FLOAT, 0, 0, 0);
			
			if (pScene->is_need_terwarning)/* 需要地形告警 */
			{
				/*设置顶点纹理1坐标*/
				glEnableVertexAttribArray(tLoc1);
				glActiveTextureARB(GL_TEXTURE1);
				glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
				glVertexAttribPointer(tLoc1, 1, GL_FLOAT, 0, 0, 0);
			}
			/*设置顶点索引，并进行绘制*/
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

	/* 如果需要地形告警,则恢复纹理1的状态 */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

#if 0
	/* 恢复纹理2的状态 */
	glActiveTexture(GL_TEXTURE2);					        // 操作纹理单元1
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);					// 操作纹理单元0

#endif

	glDisableEx(GL_TEXTURE_2D);

	
#if 0
	/* 正常模式下绘制瓦片边线的显示列表，鹰眼模式不绘制 */
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

	/* 从栈中恢复原来的模型视图矩阵 */
	glPopMatrix();

#if 0
{
	PT_3D nodexyz = {0};
	sColor4f color1={0.0};
	f_char_t timeInfo[128] = {0};
	Geo_Pt_D geoPt;
	PT_2I screenPt;
	
	/* 设置瓦片层级及索引编号文字绘制的颜色,使用红色完全不透明 */
	color1.red = 1.0;
	color1.alpha = 1.0;

	
//	nodexyz = pNode->obj_cent;
//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

	/* 计算文字绘制点的经纬高,考虑到字体有宽度,为了让字体保持在中心,设置点在瓦片的左1/4中心,高度为平均高度 */
	/* 未考虑旋转 */
	geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
	geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
	geoPt.height = pNode->height_ct;

//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
	/* 根据经纬高计算屏幕坐标 */
	getOuterViewPtByGeo(pScene, geoPt, &screenPt);


	/* 关闭深度测试,防止文字被遮挡 */
	glDisableEx(GL_DEPTH_TEST);
//	glDisableEx(GL_CULL_FACE);
	/* 在屏幕上绘制瓦片层级及索引编号 */
	sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
	textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

//	glEnableEx(GL_CULL_FACE);	
	glEnableEx(GL_DEPTH_TEST);
}
#endif

#endif

	/* 地图场景树中绘制的节点个数自增1 */
    pHandle->drawnnodnum++;
    //printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* 记录节点上一次绘制的帧计数 */
    pNode->lastDrawNumber = pHandle->drawnumber;

	return 0;
}

/*
功能：瓦片SVS模式下绘制函数

输入：
	pHandle              地图场景树指针
	pNode                瓦片节点指针
	pScene               渲染场景指针

输入输出：
	无

输出：
    无

返回值：
    0   正常绘制
	-1  瓦片节点句柄无效
	-2  地图场景树无效
*/
static f_int32_t mapNodeRenderImgSVS_EX(sMAPHANDLE * pHandle, sQTMAPNODE * pNode, sGLRENDERSCENE * pScene)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 判断输入的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* 判断当前渲染场景的场景模式 */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* 默认为正常模式 */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* 计算所有绘制瓦片节点的最小层级 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;


#ifdef _JM7200_
	/* JM7200最多只支持二重纹理，故下面方法不支持方里网和告警纹理的同时显示，支持顶点数组和显示列表两种方式 */

	/* 使能纹理单元0,关闭一维纹理,开启二维纹理 */
	glActiveTexture(GL_TEXTURE0);
	glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);

	/* JM7200：最多支持二维纹理，导致方里网纹理和告警纹理不能同时存在*/
	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定告警纹理(一维),设置纹理矩阵 */
		RenderTerrainWarningStripeColor(pScene);
	}
	else
	{

		/* 不需要地形告警,绑定方里网纹理(二维),关闭纹理单元1的一维纹理,开启二维纹理 */
		glActiveTexture(GL_TEXTURE1);
		GetGridTextureID();
		glDisableEx(GL_TEXTURE_1D);
		glEnableEx(GL_TEXTURE_2D);
		/* 设置纹理单元1的纹理组合模式为贴花GL_DECAL,即最终纹理的RGB=纹理源的RGB*纹理源的A + 片段的RGB*(1-纹理源的A),最终纹理的RGB=片段的A */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
		glDisableEx(GL_BLEND);
	}

	/* 使能纹理单元0 */
	glActiveTexture(GL_TEXTURE0);

	/* 如果强制使用显示列表,则使用显示列表的方式绘制 */
	/* 其他情况通过顶点数组的方式绘制 */
	if((useArray == 0))	
	{
		/* JM7200：glVertexPointer中GL_INT不支持，不用整数化的局部坐标，用浮点数坐标；显示列表模式下仍用的是整数化的局部坐标，故需变换矩阵*/
		CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);

#if 1
		if(pScene->is_need_terwarning)
		{
			/* 需要地形告警，创建SVS地形告警显示列表*/	
			qtmapNodeCreateSVSList(pNode);
			/* 需要地形告警，使用SVS地形告警显示列表*/
			if((pNode->tileSVSList) != 0)
				glCallList(pNode->tileSVSList);
		}else{
			/* 其他情况创建SVS方里网显示列表*/	
			qtmapNodeCreateSVSGridList(pNode);
			/* 其他情况使用SVS方里网显示列表*/	
			if((pNode->tileSVSGridList) != 0)
				glCallList(pNode->tileSVSGridList);
		}
#endif

	}
	else
	{	
		/* 设置瓦片的顶点数组 */
		glEnableClientState(GL_VERTEX_ARRAY);	
		/* 使用浮点型的顶点 */
		glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);

		/* 设置瓦片顶点纹理0的颜色数组 */
		glEnableClientState(GL_COLOR_ARRAY);
		//glColorPointer(3,GL_UNSIGNED_BYTE,0,pNode->m_pColor);
		glColorPointer(3,GL_FLOAT,0,pNode->m_pColor);

		if(pScene->is_need_terwarning)	
		{
			/* 如果需要地形告警,则设置瓦片顶点纹理1的纹理坐标数组为告警纹理 */
			RenderTerrainWarningStripeColor2(pScene, pNode);
		}
		else
		{
			/* 如果不需要地形告警,则设置瓦片顶点纹理1的纹理坐标数组为方里网纹理 */
			glClientActiveTexture(GL_TEXTURE1);                
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			//SVS模式下方里网纹理各个顶点的纹理坐标
			glTexCoordPointer(2, GL_FLOAT, 0, pNode->m_pVertexTex);
		}

		/* 绘制瓦片顶点,以三角形条带的方式绘制 */
		glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);

		/* 关闭纹理数组、顶点数组和颜色数组的使能,与Enable对应 */
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);	

	}

	/* 恢复纹理1的状态 */
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
	/* 下面方法支持方里网和告警纹理同时显示，使用了三重纹理，仅支持顶点数组方式，显示列表方式待增加 */

	/* 使能纹理单元0,关闭一维纹理,关闭二维纹理 */
	glActiveTexture(GL_TEXTURE0);
	glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);
    
	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定纹理,设置纹理矩阵 */
		RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* 关闭纹理单元1的一维纹理和二维纹理使能开关 */
		glActiveTexture(GL_TEXTURE1);
		glDisableEx(GL_TEXTURE_1D);
		glDisableEx(GL_TEXTURE_2D);
	}
	
	/* 使能纹理单元2 */
	glActiveTexture(GL_TEXTURE2);
	/* 设置纹理单元2的纹理组合模式为贴花GL_DECAL,即最终纹理的RGB=纹理源的RGB*纹理源的A + 片段的RGB*(1-纹理源的A),最终纹理的RGB=片段的A */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); 
	/* 关闭一维纹理,开启二维纹理 */
	//glDisableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
	/* 设置纹理单元2的纹理id,即方里网纹理 */
	GetGridTextureID();


	
	/* 使能纹理单元0 */
	glActiveTexture(GL_TEXTURE0);

	/* 恢复矩阵堆栈为模型视图矩阵 */
	glMatrixMode(GL_MODELVIEW);	
	/* 将原来的模型视图矩阵进行压栈保存 */
	glPushMatrix();
	
	/* 为了使用整型化后的瓦片顶点，调整模型视图矩阵，设置基准点xyz和缩放系数 */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0], pNode->point_vertex_xyz[1], pNode->point_vertex_xyz[2], pNode->point_vertex_xyz[3]);

	
	//glColor4f(1.0f,1.0f,1.0f, 1.0f);	
		
	/* 设置瓦片的顶点数组 */
	glEnableClientState(GL_VERTEX_ARRAY);	
	/* 使用浮点型的顶点 */
	//glVertexPointer(3, GL_FLOAT, 0, pNode->fvertex);	
	/* 使用整型的顶点 */
	glVertexPointer(3, GL_INT, 0, pNode->fvertex_part);		

	/* 设置瓦片顶点纹理0的颜色数组 */
	glClientActiveTexture(GL_TEXTURE0); 
	glEnableClientState(GL_COLOR_ARRAY);
	//glColorPointer(3,GL_UNSIGNED_BYTE,0,pNode->m_pColor);
	glColorPointer(3,GL_FLOAT,0,pNode->m_pColor);
	
	/* 如果需要地形告警,则设置瓦片顶点纹理1的纹理坐标数组 */
	if(pScene->is_need_terwarning)	
	{
		RenderTerrainWarningStripeColor2(pScene, pNode);
	}

	/* 设置瓦片顶点纹理2的纹理坐标数组 */
	glClientActiveTexture(GL_TEXTURE2);                
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, pNode->m_pVertexTex);


	/* 绘制瓦片顶点,以三角形的方式绘制,顶点太多,该方式废弃 */
	//glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, pIndex);
	/* 绘制瓦片顶点,以三角形条带的方式绘制 */
	glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, pIndex);

	/* 关闭纹理数组、顶点数组和颜色数组的使能,与Enable对应 */
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);	
	glDisableClientState(GL_COLOR_ARRAY);

//	glFinish();

	/* 如果需要地形告警,则恢复纹理1的状态 */
	if(pScene->is_need_terwarning)
        RenderTerrainWarningStripeColor3();

	/* 恢复纹理2的状态 */
	glActiveTexture(GL_TEXTURE2);					        
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisableEx(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);					


	glDisableEx(GL_TEXTURE_2D);

#endif

#if 0
	/* 正常模式下绘制瓦片边线的显示列表，鹰眼模式不绘制 */
	if(eScene_normal == pScene->mdctrl_cmd.scene_mode)
	{	
		/* 设置瓦片边线的颜色 */
		glColor3f(0.5f, 0.8f, 0.8f);
		/* 创建正常模式下瓦片边线的显示列表 */
		qtmapNodeCreateEdgeList(pNode);

		/* 开启线平滑 */
		glEnableEx(GL_LINE_SMOOTH);
		/* 关闭深度测试,防止线被遮挡 */
		glDisableEx(GL_DEPTH_TEST);

		/* 调用正常模式下瓦片边线显示列表绘制 */
		if((pNode->edgeList) != 0)
			glCallList(pNode->edgeList);


		glDisableEx(GL_LINE_SMOOTH);
		glEnableEx(GL_DEPTH_TEST);

	}
#endif

	/* 从栈中恢复原来的模型视图矩阵 */
	glPopMatrix();

#if 0
	{
		PT_3D nodexyz = {0};
		sColor4f color1={0.0};
		f_char_t timeInfo[128] = {0};
		Geo_Pt_D geoPt;
		PT_2I screenPt;

		/* 设置瓦片层级及索引编号文字绘制的颜色,使用红色完全不透明 */
		color1.red = 1.0;
		color1.alpha = 1.0;


		//	nodexyz = pNode->obj_cent;
		//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

		/* 计算文字绘制点的经纬高,考虑到字体有宽度,为了让字体保持在中心,设置点在瓦片的左1/4中心,高度为平均高度 */
		/* 未考虑旋转 */
		geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
		geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
		geoPt.height = pNode->height_ct;

		//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
		/* 根据经纬高计算屏幕坐标 */
		getOuterViewPtByGeo(pScene, geoPt, &screenPt);


		/* 关闭深度测试,防止文字被遮挡 */
		glDisableEx(GL_DEPTH_TEST);
		//	glDisableEx(GL_CULL_FACE);
		/* 在屏幕上绘制瓦片层级及索引编号 */
		sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
		textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

		//	glEnableEx(GL_CULL_FACE);	
		glEnableEx(GL_DEPTH_TEST);
	}
#endif

	/* 地图场景树中绘制的节点个数自增1 */
	pHandle->drawnnodnum++;
	//printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* 记录节点上一次绘制的帧计数 */
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

	/* 判断输入的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
		printf("call mapNodeRenderImg with error node\n");
		return -1;
	}
	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		return -2;
	}

	/* 判断当前渲染场景的场景模式 */
	switch(pScene->mdctrl_cmd.scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* 默认为正常模式 */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	/* 计算所有绘制瓦片节点的最小层级 */
	if(sMinLevel > pNode->level)
		sMinLevel = pNode->level;

	/* 使能纹理单元0,开启二维纹理，绑定方里网纹理(二维) */
	glActiveTextureARB(GL_TEXTURE0);
	glEnableEx(GL_TEXTURE_2D);
	GetGridTextureID();

	/* 设置纹理单元0的纹理组合模式为贴花GL_DECAL,即最终纹理的RGB=纹理源的RGB*(1-纹理源的A)+片段的RGB*纹理源的A,最终纹理的A=片段的A */
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	/* 判断是否需要地形告警 */
	if(pScene->is_need_terwarning)
	{
		/* 需要地形告警,则启用纹理单元1,设置各种OpenGL状态,绑定纹理,设置纹理矩阵 */
		RenderTerrainWarningStripeColor(pScene);
	}
	else
	{
		/* 关闭纹理单元1的二维纹理使能开关 */
		glActiveTextureARB(GL_TEXTURE1);
		glDisableEx(GL_TEXTURE_2D);
	}

	/* 使能纹理单元0 */
	glActiveTextureARB(GL_TEXTURE0);

	/* 恢复矩阵堆栈为模型视图矩阵 */
	glMatrixMode(GL_MODELVIEW);	
	/* 将原来的模型视图矩阵进行压栈保存 */
	glPushMatrix();

#ifndef VERTEX_USE_FLOAT
	/* 使用整形的顶点数据，需先移动模型视图矩阵，移动到偏移起始点 */
	CalModelMatrix(pScene, pNode->point_vertex_xyz[0],pNode->point_vertex_xyz[1],pNode->point_vertex_xyz[2],pNode->point_vertex_xyz[3]);
#endif	

	/* 设置颜色为白色,透明度为完全不透明 */
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

		/*创建瓦片的顶点绘制VBO(顶点坐标/顶点高度/顶点颜色/顶点法线/顶点方里网纹理)，SVS模式下所有视角都是绘制33*33个顶点，每个瓦片的VBO中的数据不一样*/
		qtmapNodeCreateSvsVboEs(pNode);
		/*创建瓦片的顶点绘制VBO(顶点索引)，所有瓦片的这些VBO中的数据均一样*/
		genSvsConstVBOs(&ibo);

		{
			//glDisableEx(GL_CULL_FACE);    // fix the warning function under overlook viewmode

			/*每个瓦片按33*33个顶点绘制成三角形条带*/
			glActiveTextureARB(GL_TEXTURE0);
			/*设置顶点坐标，shader中的a_position是4维的，给3维也可以；颜色和纹理坐标属性类似，未给的维度保持默认值*/
			glEnableVertexAttribArray(vLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
			glVertexAttribPointer(vLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
#else
			glVertexAttribPointer(vLoc, 3, GL_INT, GL_FALSE, 0, 0);
#endif
			/*设置顶点颜色,测试发现GL_UNSIGNED_BYTE不支持*/
			glEnableVertexAttribArray(cLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileColorVBO);
			glVertexAttribPointer(cLoc, 3, GL_FLOAT, 0, 0, 0);
			/*设置顶点纹理0坐标，方里网纹理*/
			glEnableVertexAttribArray(tLoc);
			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileGridTexVBO);
			glVertexAttribPointer(tLoc, 2, GL_FLOAT, 0, 0, 0);

			if (pScene->is_need_terwarning)/* 需要地形告警 */
			{
				/*设置顶点纹理1坐标，水平告警纹理*/
				glEnableVertexAttribArray(tLoc1);
				glActiveTextureARB(GL_TEXTURE1);
				glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
				glVertexAttribPointer(tLoc1, 1, GL_FLOAT, 0, 0, 0);
			}
			/*设置顶点索引，并绘制所有顶点*/
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

	/* 如果需要地形告警,则恢复纹理1的状态 */
	if(pScene->is_need_terwarning)
		RenderTerrainWarningStripeColor3();

	glActiveTextureARB(GL_TEXTURE0);
	glDisableEx(GL_TEXTURE_2D);


#if 0
	/* 正常模式下绘制瓦片边线的显示列表，鹰眼模式不绘制 */
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

	/* 从栈中恢复原来的模型视图矩阵 */
	glPopMatrix();

#if 0
	{
		PT_3D nodexyz = {0};
		sColor4f color1={0.0};
		f_char_t timeInfo[128] = {0};
		Geo_Pt_D geoPt;
		PT_2I screenPt;

		/* 设置瓦片层级及索引编号文字绘制的颜色,使用红色完全不透明 */
		color1.red = 1.0;
		color1.alpha = 1.0;


		//	nodexyz = pNode->obj_cent;
		//	worldXYZ2screenXY( nodexyz, &node_x, &node_y);

		/* 计算文字绘制点的经纬高,考虑到字体有宽度,为了让字体保持在中心,设置点在瓦片的左1/4中心,高度为平均高度 */
		/* 未考虑旋转 */
		geoPt.lon = pNode->hstart + pNode->hscale / 4.0;
		geoPt.lat = pNode->vstart + pNode->vscale / 2.0;
		geoPt.height = pNode->height_ct;

		//	getOuterScreenPtByGeo(pScene, geoPt, &screenPt);
		/* 根据经纬高计算屏幕坐标 */
		getOuterViewPtByGeo(pScene, geoPt, &screenPt);


		/* 关闭深度测试,防止文字被遮挡 */
		glDisableEx(GL_DEPTH_TEST);
		//	glDisableEx(GL_CULL_FACE);
		/* 在屏幕上绘制瓦片层级及索引编号 */
		sprintf(timeInfo, "%d-%d-%d", pNode->level, pNode->xidx, pNode->yidx);
		textOutPutOnScreen(0, screenPt.x, screenPt.y, timeInfo, color1);

		//	glEnableEx(GL_CULL_FACE);	
		glEnableEx(GL_DEPTH_TEST);
	}
#endif

#endif

	/* 地图场景树中绘制的节点个数自增1 */
	pHandle->drawnnodnum++;
	//printf("%d_%d_%d ", pNode->level, pNode->xidx, pNode->yidx);
	/* 记录节点上一次绘制的帧计数 */
	pNode->lastDrawNumber = pHandle->drawnumber;

	return 0;
}


/*
功能：根据经纬度，获取飞机位置在已有地图数据子树中最大瓦片层级的瓦片节点指针(详细判断,采用递归的方式)
输入：
	pHandle     地图场景树句柄
	pNode       当前瓦片节点指针
	pos_lon     飞机经度
	pos_lat     飞机纬度
	pScene      场景绘制句柄

输入输出：
	无

输出：
    无

返回值：
    tpNode 有效的瓦片节点指针
    NULL   无效的瓦片节点指针,即飞机位置不在以pNode为头结点的瓦片子树链表中
*/
static sQTMAPNODE * getTreeLevel(sMAPHANDLE *pHandle, sQTMAPNODE *pNode, f_float32_t pos_lon, f_float32_t pos_lat, sGLRENDERSCENE *pScene)
{
    f_int32_t ret = -1, i = 0;
	sQTMAPNODE *tpNode = NULL;
    
	/* 判断瓦片节点是否为空及节点的key值是否正确 */
	if (!ISMAPDATANODE(pNode))
	{
		DEBUG_PRINT("call getTreeLevelOfPlanepos with error node.");
		return NULL;
	}

	/* 判断地图场景树句柄是否为空及句柄的key值是否正确 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
		DEBUG_PRINT("call getTreeLevelOfPlanepos with error pHandle.");
		return NULL;
	}
	
	/* 1.判断相机所在位置(即飞机位置)的经纬度是否在瓦片节点(pNode)所属的范围内 1-在,0-不在 */
	if(!isCaminNode(pScene, pNode))
    {
		/* 1.1 如果步骤1不满足，则进一步判断瓦片节点(pNode)是否在相机的视景体可视范围内 0-在,-1-不在  */
		ret = isNodeInFrustum(pScene->camctrl_param.view_near, pScene->camctrl_param.view_far ,&(pScene->frustum), pNode);
		/* 如果步骤1.1不满足,则返回NULL,继续判断下一棵子树  */
		if(0 != ret)
		{
			return NULL; //NODE_NOT_IN_VIEW;
		}	
		
		/* 1.2 漫游模式下不用判断是否在背面,否则在低层级由于瓦片太大会露白 问题*/
		if(eROM_outroam == pScene->mdctrl_cmd.rom_mode)
		{
		    /* 1.3 判断瓦片节点是否在相机视景体看到的地球的背面,如果在背面则不绘制,直接返回NULL,继续判断下一棵子树 */
		    if(isNodeAtEarthBack(pScene, pNode))
		    {
			    return NULL; //NODE_AT_EARTH_BACK;	
		    }
		}
    }

    /* 如果步骤1中所有判断通过，然后判断是否大于或等于最大层级，到达最大层级后不会往下判断子瓦片 */
	if(pNode->level < sMaxLevel)
	{
	    /* 如果未到达到达最大层级，则进一步判断是否需要创建子瓦片 0-需要创建 其他-无需创建*/
	    ret = isNeedCreateChildren(pNode, NULL, pScene->camParam.m_geoptEye, pScene->camParam.m_ptEye, pScene->camctrl_param.view_type);  //返回0表示需要往下分
	}
	else
	{
		/* 如果已到达最大层级,则该瓦片节点就是要找的瓦片节点,将该节点返回 */
		ret = -1;
	}
	
	/* ret为0表示需要递归子瓦片,其他值则直接返回当前瓦片节点 */
	if(0 == ret)
	{	
		/* 创建当前瓦片节点的孩子节点,第三个参数为TRUE,表明只创建节点但不加载节点数据 */
		if(qtmapnodeCreateChildren(pHandle, pNode, TRUE) > 0)
		{
			/* 当前瓦片节点的孩子节点创建成功,依次判断4个孩子节点 */
			for(i = 0; i < 4; i++)
			{
				if (ISMAPDATANODE(pNode->children[i]))
	            {
					/* 若孩子节点有效且key值不正确,则判断飞机的经纬度是否在孩子节点的经纬度范围 */
					if((pos_lon >= pNode->children[i]->hstart) && (pos_lon < pNode->children[i]->hstart + pNode->children[i]->hscale) &&
						(pos_lat >= pNode->children[i]->vstart) && (pos_lat < pNode->children[i]->vstart + pNode->children[i]->vscale))
					{
						/* 若在范围内,则递归判断该孩子节点,直至找到已有子树中飞机位置所在的最高层级的瓦片节点 */
						tpNode = getTreeLevel(pHandle, pNode->children[i], pos_lon, pos_lat, pScene);	
						if(NULL == tpNode)
							tpNode = pNode;
					}
	            }
	            else
	            {
					/* 若孩子节点无效或孩子节点有效但key值不正确,则返回的tpNode等于当前瓦片节点 */
					DEBUG_PRINT("call getTreeLevelOfPlanepos with error node.");
					tpNode = pNode;
			    }
			}/* for(i = 0; i < 4; i++) */
		}/* end if(qtmapnodeCreateChildren(pHandle, pNode) > 0) */
		else
		{
			/* 创建当前瓦片节点的孩子节点失败,则返回的tpNode等于当前瓦片节点 */
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
功能：根据经纬度，获取飞机位置在最大瓦片层级的瓦片节点指针(初步判断)
输入：地图场景树、子树根节点指针、飞机经纬度、场景渲染句柄
输出：瓦片节点指针
      NULL：无效指针
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

	/* 先初步通过瓦片的四至范围进行判断 */
	if((pos_lon >= pNode->hstart) && (pos_lon <= pNode->hstart + pNode->hscale) &&
	   (pos_lat >= pNode->vstart) && (pos_lat <= pNode->vstart + pNode->vscale))
	{
		/* 用递归的方式进行详细判断 */
        tNode = getTreeLevel(pHandle, pNode, pos_lon, pos_lat, pScene);
    }

	return(tNode);
}

//在scene 的绘制链表中添加节点
static f_int32_t drawListAddNode(sQTMAPNODE *pNode, sGLRENDERSCENE *pScene)
{
	/**挂到头结点之后**/
	stList_Head *pstListHead = NULL;
	pstListHead = &(pScene->scene_draw_list);
		
	LIST_ADD(&pNode->stListHeadDrawing, pstListHead);	

	return 0;
}

//清空scene 的链表
f_int32_t drawListDeleteNode(sGLRENDERSCENE *pScene)
{
    stList_Head *pstListHead = NULL;
    /**链表初始化**/
	pstListHead = &(pScene->scene_draw_list);
	LIST_INIT(pstListHead);

	return 0;
}	

/*
功能：循环绘制当前帧绘制瓦片节点链表中的所有节点,不同模式采用不同的绘制方法
输入：
	pHandle     地图场景树句柄
	pScene      场景绘制句柄

输入输出：
	无

输出：
    无

返回值：
	0  当前帧正常绘制
    -1 当前帧绘制瓦片节点链表为空
*/
// 获取当前帧需要绘制的瓦块
#define MAX_PNODE_COUNT 400
char* pNodeNames[MAX_PNODE_COUNT];
int pNodeNameCount = 0;
char** GetTileDrawList(int* tileNum)	// 外部接口, 在Render后通过该接口获取当前帧正在绘制的瓦块列表
{
	*tileNum = pNodeNameCount;
	return pNodeNames;
}
 void PriTileDrawList()	// 打印瓦块名
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

	/* 判断当前帧的瓦片节点绘制链表是否为空 */
	if (LIST_IS_EMPTY(pstListHead))
	{
		return -1;
	}

	/* 当视角为俯视且未启用高度预警及SVS模式时,此时绘制的是矩形,其他情况绘制是三角形条带 */
	/* 指定多边形的正面方向,顶点序列按顺时针,默认是逆时针(GL_CCW),用于背面多边形的消除GL_CULL_FACE */
	if((pScene->camctrl_param.view_type == eVM_DEFAULT_VIEW)
		||(pScene->camctrl_param.view_type == eVM_FIXED_VIEW)
		||(pScene->camctrl_param.view_type == eVM_COCKPIT_VIEW)
		||(pScene->camctrl_param.view_type == eVM_FOLLOW_VIEW)
		||(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_inwarn)
		||(pScene->mdctrl_cmd.svs_mode != FALSE))
	{
		glFrontFace(GL_CW);
	}

	/* 遍历当前帧的瓦片节点绘制链表中的节点 */
	pNodeNameCount = 0;
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHeadDrawing);
			if(pNode != NULL)
			{
				if(pScene->mdctrl_cmd.close3d_mode == 1)	//屏蔽三维绘制的开关控制
				{
					continue;
				}


				// gx test for nibiru
				{
					// 存储pNode的名称
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
				
				/* 当前为SVS模式,调用SVS模式的绘制函数 */
				/* 当前为VecShade模式,调用VecShade模式的绘制函数 */
				/* 当前为正常模式,调用正常模式的绘制函数 */
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

	/* 指定多边形的正面方向,顶点序列按逆时针,用于背面多边形的消除GL_CULL_FACE */
	glFrontFace(GL_CCW);

	return 0;
}

/*
功能：判断当前帧绘制瓦片节点链表中是否有节点数据未加载完成,如果有则告知绘制任务当前帧不SwapBuffer,仍保留上一帧的绘制,
      待数据加载完成再SwapBuffer,把当前帧的内容显示;
输入：
	pHandle     地图场景树句柄
	pScene      场景绘制句柄

输入输出：
	无

输出：
    无

返回值：
    1 有节点数据未加载完成,当前帧不绘制
	0 所有节点数据均加载完成,当前帧正常绘制
    -1 当前帧绘制瓦片节点链表为空,当前帧不绘制
*/
static f_int32_t DrawOverlookCallback(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene)
{
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	sQTMAPNODE *pNode = NULL;
	int node_level = 0;
	int node_lastdraw = 0;

	pstListHead = &(pScene->scene_draw_list);

	/* 判断当前帧的瓦片节点绘制链表是否为空 */
	if (LIST_IS_EMPTY(pstListHead))
	{
		return -1;
	}

	/* 遍历当前帧的瓦片节点绘制链表中的节点 */
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
				/* 得到所有瓦片的最小层级 */
				if(node_level >= pNode->level)
				{
					node_level = pNode->level;
				}

				/* lastdraw为3,表示瓦片节点的数据未加载完成 */
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


	/* 如果还有瓦片节点的数据未加载完成,则告知绘制任务当前帧不SwapBuffer,仍保留上一帧的绘制 */
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
功能：俯视且未开启高度预警功能时,判断子树链表中哪些瓦片节点需要加载当前帧绘制链表中(递归判断)
输入：
	pHandle     地图场景树句柄
	pNode       瓦片节点句柄
	pScene      场景绘制句柄

输入输出：
	无

输出：
    无

返回值：
    NODE_NOT_READY(0) 不存在,无需绘制
    NODE_NOT_IN_VIEW(0) 不在视景体范围,无需绘制
	NODE_AT_EARTH_BACK(0) 节点在视景体范围但在地球背面,无需绘制
	NODE_DRAWING(1)   节点需要绘制,已加到当前帧绘制链表中
*/
static f_int32_t mapNodeRenderOverlook(sMAPHANDLE *pHandle, sQTMAPNODE *pNode,  sGLRENDERSCENE *pScene)
{
	f_int32_t ret = 0, i = 0;

	/* 判断输入的地图场景树句柄是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    DEBUG_PRINT("call mapNodeRender with error handle.");
	    return NODE_NOT_READY;
	}
	
	/* 判断输入的当前瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
	    DEBUG_PRINT("call mapNodeRender with error node.");
	    return NODE_NOT_READY;
	}
	
	/* 判断当前瓦片节点数据是否加载完成(在三维数据加载任务中执行),若未完成则直接返回 */
	ret = qtmapnodeGetStatus(pNode, __LINE__);
	if(DNST_READY != ret)
		return NODE_NOT_READY;
	
	/**************下面是当前瓦片节点数据已经加载完成的情况*******************/

	/* 记录节点的当前绘制的帧数,多个视口会累加 */
	pNode->drawnumber = pHandle->drawnumber;

    /* 1.判断相机所在位置(即飞机位置)的经纬度是否在瓦片节点(pNode)所属的范围内 1-在,0-不在 */
    if(!isCaminNode(pScene, pNode))
    {
		/* 判断是否在视景体内(会判孙子节点)  0-在,-1-不在*/
		/* 1.1 如果步骤1不满足，则进一步判断瓦片节点(pNode)是否在相机的视景体可视范围内 0-在,-1-不在  */
		/* 与mapNodeRender函数不同的地方,俯视下使用的是由瓦片四个角点(高度默认为0)形成的包围球 */
		/* 判断时,当前节点不在视景体内,还会进一步判孙子节点 */
		ret = isNodeInFrustumOverlook(pScene->camctrl_param.view_near, pScene->camctrl_param.view_far ,&(pScene->frustum), pNode);

		/* 如果步骤1.1不满足,则不绘制,直接返回 */
		if(0 != ret)
		{
			//printf(" data not in frustum ");
			return NODE_NOT_IN_VIEW;
		}	
		
		/* 1.2 漫游模式下不用判断是否在背面,否则在低层级由于瓦片太大会露白 问题*/
		if(eROM_outroam == pScene->mdctrl_cmd.rom_mode)
		{
		    /* 1.3 判断瓦片节点是否在相机视景体看到的地球的背面,如果在背面则不绘制,直接返回 */
		    if(isNodeAtEarthBack(pScene, pNode))
		    {
		    	//	printf(" data in back ");
			    return NODE_AT_EARTH_BACK;	
		    }
		}
    }
	
	/* 如果步骤1中所有判断通过,然后判断是否大于或等于(最大层级-1)，到达(最大层级-1)后不会往下判断子瓦片 */
	/* 与getTreeLevel函数中不同,此处不再调用isNeedCreateChildren函数判断是否需要创建子瓦片,俯视下默认认为子瓦片就要绘制 */
	/* ,会造成瓦片数量大量增加,所以控制绘制的节点最多到(最大层级-1)层,减少绘制节点的数量 */
	if(pNode->level < (sMaxLevel - 1))
	{
		/* 表示需要递归子瓦片*/
	    ret = 0;   
//		printf("not max level %d \n",pNode->level );
	}
	else
	{	
		/* 无需递归子瓦片,直接绘制当前瓦片节点 */
		ret = -1; 
//		printf("max level %d \n",pNode->level );
	}

	if(0 != ret)
	{
		/* 不递归子瓦片，画本身;将当前瓦片节点加入绘制瓦片节点链表 */		
		drawListAddNode(pNode, pScene);
		/* 设置当前瓦片节点的lastdraw=1,表示该瓦片为叶子节点,无孩子节点,直接返回 */
		pNode->lastdraw = 1;
		return NODE_DRAWING;
	}
	
	/* 创建当前瓦片节点的孩子节点,将第三个参数设置为FALSE, 告知这些节点要加载数据 */
	/* -1：节点不存在或者子节点创建失败 0：子节点不在数据树里 4：子节点创建成功*/
	ret = qtmapnodeCreateChildren(pHandle, pNode, FALSE);  
	if(0 >= ret)
	{
		/* 创建子瓦片失败，画本身;将当前瓦片节点加入绘制瓦片节点链表 */
		drawListAddNode(pNode, pScene);

		/* 设置当前瓦片节点的lastdraw值,然后直接返回 */
		if(ret == 0)
			pNode->lastdraw = 2;	//子节点不在数据树里
		else
			pNode->lastdraw = 3;	//节点或者子节点创建失败

		return NODE_DRAWING;
	}
	
	/* 判断是否所有子瓦片的数据都加载完成(在三维数据加载任务中执行) */
	ret = isAllChildNodeDataReady(pNode);   //0表示yes
	if(0 != ret)
	{
		/* 有子瓦片没准备好，画本身;将当前瓦片节点加入绘制瓦片节点链表 */
		drawListAddNode(pNode, pScene);
		
		/* 设置当前瓦片节点的lastdraw值为3,子节点数据未加载完成也算子节点创建失败,然后直接返回 */
		pNode->lastdraw = 3;
		return NODE_DRAWING;
	}
	
	/* 当前瓦片是否需要绘制判断完了,递归判断该瓦片节点的四个子瓦片是否绘制 */
	for(i=0; i<4; i++)
	{
		mapNodeRenderOverlook(pHandle, pNode->children[i], pScene);
	}
	/* 设置当前瓦片节点的lastdraw值为4,子节点数据加载完成但该瓦片节点无需绘制,然后直接返回 */
	pNode->lastdraw = 4;
	
	return 0;
}

/*
功能：俯视且开启高度预警功能/其他非俯视视角时,判断子树链表中哪些瓦片节点需要加载当前帧绘制链表中(递归判断)
输入：
	pHandle         地图场景树句柄
	pNode           瓦片节点句柄
	pScene          场景绘制句柄
	pPlanePosNode   飞机位置所在瓦片节点句柄
	pitch           飞机俯仰角
输入输出：
	无

输出：
    无

返回值：
    NODE_NOT_READY(0) 不存在,无需绘制
    NODE_NOT_IN_VIEW(0) 不在视景体范围,无需绘制
	NODE_AT_EARTH_BACK(0) 节点在视景体范围但在地球背面,无需绘制
	NODE_DRAWING(1)   节点需要绘制,已加到当前帧绘制链表中
*/
static f_int32_t mapNodeRender(sMAPHANDLE *pHandle, sQTMAPNODE *pNode, sQTMAPNODE *pPlanePosNode, f_float32_t pitch, sGLRENDERSCENE *pScene)
{
	f_int32_t ret = 0, i = 0;
	
	/* 判断输入的地图场景树句柄是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(pHandle))
	{
	    DEBUG_PRINT("call mapNodeRender with error handle.");
	    return NODE_NOT_READY;
	}
	
	/* 判断输入的当前瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pNode))
	{
	    DEBUG_PRINT("call mapNodeRender with error node.");
	    return NODE_NOT_READY;
	}
	
	/* 判断输入的飞机位置所在的瓦片节点句柄是否有效,若无效则直接返回 */
	if (!ISMAPDATANODE(pPlanePosNode))
	{
	    DEBUG_PRINT("call mapNodeRender with error pPlanePosNode.");
	    return NODE_NOT_READY;
	}

	/* 判断当前瓦片节点数据是否加载完成(在三维数据加载任务中执行),若未完成则直接返回 */
	ret = qtmapnodeGetStatus(pNode, __LINE__);
	if(DNST_READY != ret)
		return NODE_NOT_READY;
	
	/**************下面是当前节点已经准备好了的情况*******************/	

	/* 记录节点的当前绘制的帧数,多个视口会累加 */
	pNode->drawnumber = pHandle->drawnumber;

    /* 1.判断相机所在位置(即飞机位置)的经纬度是否在瓦片节点(pNode)所属的范围内 1-在,0-不在 */
    if(!isCaminNode(pScene, pNode))
    {
		/* 1.1 如果步骤1不满足，则进一步判断瓦片节点(pNode)是否在相机的视景体可视范围内 0-在,-1-不在  */
		/* 与mapNodeRenderOverlook函数不同的地方,使用的是带高度信息的包围球,俯视下使用的是由瓦片四个角点(高度默认为0)形成的包围球 */
		/* 判断时,当前节点不在视景体内,还会进一步判孙子节点 */
		ret = isNodeInFrustum(pScene->camctrl_param.view_near, pScene->camctrl_param.view_far ,&(pScene->frustum), pNode);
		if(0 != ret)
		{
			//printf(" data not in frustum ");
			return NODE_NOT_IN_VIEW;
		}	
		
		/* 1.2 漫游模式下不用判断是否在背面,否则在低层级由于瓦片太大会露白 问题*/
		if(eROM_outroam == pScene->mdctrl_cmd.rom_mode)
		{
		    /* 1.3 判断瓦片节点是否在相机视景体看到的地球的背面,如果在背面则不绘制,直接返回 */
		    if(isNodeAtEarthBack(pScene, pNode))
		    {
		    	//	printf(" data in back ");
			    return NODE_AT_EARTH_BACK;	
		    }
		}
    }
	
	/* 如果步骤1中所有判断通过,然后判断是否大于或等于(最大层级)，到达(最大层级)后不会往下判断子瓦片 */
	/* 与getTreeLevel函数中相同,与mapNodeRenderOverlook函数中不同,此处控制绘制的节点最多到(最大层级)层,因为会判断子瓦片是否需要绘制 */
	/* 步骤1中判断出来的在视景体内的瓦片都是到同一层级,座舱视角为了减少瓦片数量,采取离视点近的瓦片采用高层级,离视点远的采用低层级的策略 */
	if(pNode->level < sMaxLevel)
	{
		/* 如果未到达到达最大层级，则进一步判断是否需要创建子瓦片 0-需要创建 其他-无需创建 */
	    ret = isNeedCreateChildren(pNode, pPlanePosNode, pScene->camParam.m_geoptEye, pScene->camParam.m_ptEye, pScene->camctrl_param.view_type);  //返回0表示需要往下分
		/* 当俯仰角在[82.5,,97.5]之间,即接近俯视视角时 */
		if(fabs(fabs(pitch) - 90.0) < 7.5)
		{
			if(0 != ret)
			{
				/* 如果无需创建当前瓦片的子瓦片,但当前瓦片层级小于飞机所在瓦片的层级,还是强制去创建子瓦片 */
			    if(pNode->level < pPlanePosNode->level)
			        ret = 0;
			}
			else
			{
				/* 如果需要创建当前瓦片的子瓦片,但当前瓦片层级大于或等于飞机所在瓦片的层级,则不去创建子瓦片 */
				if(pNode->level >= pPlanePosNode->level)
			        ret = -2;
			}
			//printf("pitch = %f, level = %d\n", pitch, pPlanePosNode->level);
		}	  


	}
	else
	{	
		 /* 已到达最大层级 */
		ret = -1;
	}

	if(0 != ret)
	{
		/* 不递归子瓦片，画本身;将当前瓦片节点加入绘制瓦片节点链表 */
		drawListAddNode(pNode, pScene);
		/* 设置当前瓦片节点的lastdraw=1,表示该瓦片为叶子节点,无孩子节点,或则瓦片已达到最大层级,直接返回 */
		pNode->lastdraw = 1;
		return NODE_DRAWING;
	}
	
	/*上面判断出来要递归子瓦片，所以得先去创建子瓦片*/
	ret = qtmapnodeCreateChildren(pHandle, pNode, FALSE);  //返回0或4，0：至少一个子瓦片创建失败；4：子瓦片都创建成功
	if(0 >= ret)
	{
		/* 创建子瓦片失败，画本身;将当前瓦片节点加入绘制瓦片节点链表 */
		drawListAddNode(pNode, pScene);

		/* 设置当前瓦片节点的lastdraw值,然后直接返回 */
		if(ret == 0)
			pNode->lastdraw = 2;  //子节点不在数据树里
		else
			pNode->lastdraw = 3;  //节点或者子节点创建失败

		return NODE_DRAWING;
	}
	
	/* 判断是否所有子瓦片的数据都加载完成(在三维数据加载任务中执行) */
	ret = isAllChildNodeDataReady(pNode);   //0表示yes
	if(0 != ret)
	{
		/* 有子瓦片没准备好，画本身;将当前瓦片节点加入绘制瓦片节点链表 */
		drawListAddNode(pNode, pScene);
		/* 设置当前瓦片节点的lastdraw值为3,子节点数据未加载完成也算子节点创建失败,然后直接返回 */
		pNode->lastdraw = 3;		
		return NODE_DRAWING;
	}
	
	/* 当前瓦片是否需要绘制判断完了,递归判断该瓦片节点的四个子瓦片是否绘制 */
	for(i=0; i<4; i++)
	{
		mapNodeRender(pHandle, pNode->children[i], pPlanePosNode, pitch,pScene);
	}
	/* 设置当前瓦片节点的lastdraw值为4,子节点数据加载完成但该瓦片节点无需绘制,然后直接返回 */
	pNode->lastdraw = 4;
	
	return 0;
}

static f_float32_t rgb2GrayValue(f_float32_t r, f_float32_t g, f_float32_t b)
{
	f_float32_t gray = r * 0.299f + g * 0.587f + b * 0.114f;
	return gray;
}

/*启用雾的效果*/
static void setFog(sGLRENDERSCENE * pHandle)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_float32_t fogColor[4] = {0.75f, 0.75f, 0.75f, 0.0f};
//	if(!ISMAPHANDLEVALID(pHandle))
//	    return;
	if(pHandle == NULL)
		return;
	
	// 俯视模式下不绘制雾，由于视点位置很高，如果有雾会引起地形的颜色发生变化
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
			 * 以下计算10000米高度至30000米高度之间的远近雾面的比例
			 * 方程 a*height+b=ratio
			 * 有    10000*a+b=5
			 *       30000*a+b=ratio30000
			 * 而30000米的ratio30000=sqrt(ptTemp.z * ptTemp.z + 2 * EARTH_RADIUS * ptTemp.z + 2 * EARTH_RADIUS * EARTH_RADIUS)
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
	/* 地图场景树,全局唯一 */
	sMAPHANDLE * pHandle = NULL;
	/* 地图场景树中的各个子树 */
	sQTMAPSUBTREE * pSubTree = NULL;
	/* 飞机位置所处的瓦片 */
	sQTMAPNODE *pPlanePosNode = NULL;
	/* 渲染场景句柄,一个视口对应一个 */
	sGLRENDERSCENE *pScene = NULL;
	f_int32_t i = 0, j = 0;

	/* 判断输入的渲染场景句柄是否有效,若无效则直接返回 */
	pScene = scenePtrValid(scene);
	if(NULL == pScene)
	    return -1;	
	
	/* 判断输入的地图场景树是否有效,若无效则直接返回 */
	if (!ISMAPHANDLEVALID(map_handle))
		return -1;
	pHandle = (sMAPHANDLE *)map_handle;
	
    takeSem(pHandle->rendscendDrawlock,FOREVER_WAIT);

	// 首先清空绘制链表
	drawListDeleteNode(pScene);

	/* 记录当前绘制的帧数,多个视口会累加,该函数调用一次计数增加1 */
	pHandle->drawnumber = (pHandle->drawnumber + 1) & ((1 << 20) - 1); 	

	/* 用于记录当前帧绘制瓦片节点数,每帧每个视口开始绘制前会清零 */
	pHandle->drawnnodnum = 0;

	/* 设置并启用雾的效果 */
	//setFog(pScene);

	/* 绘制天空盒,俯视视角下不绘制 */
	if( pScene->camctrl_param.view_type != eVM_OVERLOOK_VIEW )
	{
		renderAtmosphere(pScene);
	}

	/* 创建全球第0层的8棵子树的根节点 */
	for(i  = 0; i < 4; i++)
	{
		for(j = 0; j < 2; j++)
		{
	        if (pHandle->pNodeRoot[i * 2 + j] == NULL)
	        {
				/* 根据根节点的瓦片层级(必为0)、x方向索引、y方向索引判断该瓦片节点是否在已读取的子树链表中,如果不在,程序直接退出; */
		        /* 如果在,返回该子树的指针; */
				if (!isNodeExistInMapSubreeList(pHandle, 0, i, j, &pSubTree))
			        return -2;
				/* 创建根节点,并告知三维加载任务进行瓦片节点数据加载 */
		        qtmapnodeCreate(pHandle, 0, i, j, pSubTree, &pHandle->pNodeRoot[i * 2 + j], FALSE);
	        }
	    }
	}	
	
	/* 设置最大层级,根据视场角和视点高度计算瓦片调度的最大层级 */
    sMaxLevel = getCurrentMaxLevel(pScene->mdctrl_cmd.scene_mode, pScene->camctrl_param.view_angle, pScene->camParam.m_geoptEye.height);                           
	
	// gx test
	sMaxLevel = 1;	

	/* 设置最小层级 */
	sMinLevel = 9999;
	
	/* 根据视点的经纬度在8个子树中查找视点所在的那个瓦片节点(已有子树的最高层级 < level < sMaxLevel-1) */
	for(i = 0; i < 8; i++)
	{
		/* 调用该函数会构建四叉树,但并未告知三维加载任务加载这些节点数据 */
	    pPlanePosNode = getTreeLevelOfPlanepos(pHandle, pHandle->pNodeRoot[i], pScene->camParam.m_geoptEye.lon, pScene->camParam.m_geoptEye.lat, pScene);
		/* 只可能落在一个节点中,找到了则直接跳出循环 */
	    if(NULL != pPlanePosNode)
	        break;
    }
	
	/* 根据外部参数开启或关闭地形告警功能 */ 
	if(eTERWARN_outwarn == terwarn_md)
	{
	    pScene->is_need_terwarning = FALSE;
	}
	else
	{
	    pScene->is_need_terwarning = TRUE;
	}
	    
	/* 鹰眼模式强制关闭地形告警功能 */    
	if(eScene_eyebird == pScene->mdctrl_cmd.scene_mode)
	{
		pScene->is_need_terwarning = FALSE;
	}

	/* 依次遍历全球8棵子树,决定哪些瓦片节点需要绘制,将其加入到绘制链表中 */ 
	/* 区分两种情况:俯视且未开启高度预警时,每个瓦片节点只需绘制成一个矩形,绘制要素减少,层级可以提高,增加绘制节点 */
	/* 其他情况时,每个瓦片节点必须按三角形条带来绘制,绘制要素多,层级要降低,减少绘制节点 */
	if((pScene->camctrl_param.view_type == eVM_OVERLOOK_VIEW)
		&&(pScene->mdctrl_cmd.terwarn_mode == eTERWARN_outwarn))
	{
		/* 俯视模式且未开启高度预警时,无需传入飞机位置所在的节点 */
		for(i = 0; i < 8; i++)
		{
			mapNodeRenderOverlook(pHandle, pHandle->pNodeRoot[i], pScene);
		}

	}
	else
	{
		/* 其他模式时,需传入飞机位置所在的节点和飞机的俯仰角 */
		for(i = 0; i < 8; i++)
		{
		    mapNodeRender(pHandle, pHandle->pNodeRoot[i], pPlanePosNode, pitch, pScene);	
		}
	}
	
	/* 判断三维数据加载任务是否将瓦片数据加载完成,未完成则不SwapBuffer,仍保留上一帧的绘制内容 */
	/* 这样保证不会显示调度过程中中间层瓦片的数据,直接显示最后调度完成后的瓦片,不会出现由模糊到清晰的过程 */
	/* 暂时不启用 */
	//DrawOverlookCallback(pHandle, pScene);

	/* 循环绘制当前帧绘制瓦片节点链表中的所有节点,不同模式采用不同的绘制方法 */
	drawListDraw(pHandle, pScene);

	/* 关闭雾功能 */
	glDisableEx(GL_FOG);


	if(pScene->mdctrl_cmd.svs_mode == 1)
	{
		EnableLight(FALSE);
		EnableLight0(FALSE);
	}

#ifdef USE_SQLITE

	/* 俯视/缩放/鹰眼下不绘制地名 */
	if( ((pScene->camctrl_param.view_type != eVM_OVERLOOK_VIEW)
		&&(pScene->camctrl_param.view_type != eVM_SCALE_VIEW))
		||(pScene->mdctrl_cmd.scene_mode == eScene_eyebird) )
	{

		/* 判断是否需要重新调度qtn数据 */
		QtnSqlJudge(pHandle, pScene);

		/* 绘制各个图层,目前只支持1层,地名注记图层 */
		for (i = 0; i < MAXMAPLAYER; i++)
		{
			f_float32_t projection[16] = {0};
			f_float32_t modelview[16] = {0};

			glGetFloatv(GL_PROJECTION_MATRIX,projection);
			glGetFloatv(GL_MODELVIEW_MATRIX,modelview);
			
			/* 绘制前的准备,采用三维中初始化的字体句柄,实际调用的是函数qtndisplaypre */
			if (pHandle->layerfunc[i].funcdisplaypre != NULL)
			{
				pHandle->layerfunc[i].funcdisplaypre(pScene->camctrl_param.view_type, pHandle->ttf_font, pScene->innerviewport, pScene->matrix);
			}
			/*注记碰撞检测参数初始化及判断哪些注记需要绘制显示*/
			if (pHandle->layerfunc[i].funcdisplay != NULL)
			{
				/*初始化碰撞检测的参数*/
			    collisionInfoInit3d(pScene->innerviewport[2], pScene->innerviewport[3]);
//			    for(j = 0; j < 8; j++)
//					qtmapDrawNodeLayer(pHandle, pHandle->pNodeRoot[j], i);

				takeSem(pHandle->qtnthread, FOREVER_WAIT);

				//printf("pHandle->cam_geo_pos.height =  %f\n",pHandle->cam_geo_pos.height);
				/*判断哪些注记需要绘制显示,将需要绘制的注记放入到全局的注记绘制数组中,实际调用的是函数qtndisplaynode*/
				pHandle->layerfunc[i].funcdisplay( pScene->camctrl_param.view_type, pHandle->ttf_font, 
			                                    0, pScene->camParam.m_ptEye, pScene->camParam.m_geoptEye.height/*pHandle->cur_plane_height*/, pScene->innerviewport,
			                                    pHandle->qtn_node[pScene->qtn_nodeID].qtn_sql, pHandle->qtn_node[pScene->qtn_nodeID].qtn_num);

				giveSem(pHandle->qtnthread);		
			}

			/* 注记碰撞、注记绘制显示及绘制后的状态恢复,实际调用的是函数qtndisplaypro */
			if (pHandle->layerfunc[i].funcdisplaypro != NULL)
			{
				pHandle->layerfunc[i].funcdisplaypro(pScene->camctrl_param.view_type, pHandle->ttf_font);
			}

			/*恢复原来的投影模视矩阵*/
			glMatrixMode(GL_PROJECTION);						
			glLoadMatrixf(projection);
		    glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(modelview);
		}
	}
#endif	

	/* 计算在显存中的最大纹理数量 */
	/* createTexNumber在mapNodeRenderImg函数中赋值,每创建一个节点则会自增1 */
	/* delTexNumber在qtmapClearCache函数中赋值,每删除一个节点或单独删除节点纹理时则会自增1 */
	if(pHandle->createTexNumber - pHandle->delTexNumber > pHandle->maxTexNumber)
		pHandle->maxTexNumber = pHandle->createTexNumber - pHandle->delTexNumber;

#if 0
	/* 每隔600tick打印一下引擎内部的具体参数 */
	/* drawnnodnum表示当前帧地图场景树中绘制的总节点数,每一帧会清零 */
	/* nodecount表示地图场景树中历史累计创建的总节点数 */
	/* loadedNumber表示地图场景树中历史累计数据加载完成的总节点数 */
	/* delNodeNumber表示地图场景树中历史累计从内存中删除的总节点数,loadedNumber-delNodeNumber表示实际内存中的总节点数 */
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
	
    /*查找LRU链表里有没有空置的节点*/
	pstListHead = &(pHandle->nodehead.stListHead);
	/**正向遍历**/
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
			
	/* 清除图幅 */
	if (qtmapGetNodePeakValue(pHandle) > MAXQTNODE)
	{
		for(i = 0; i < 8; i++)
		    qtmapClearCache(pHandle, pHandle->pNodeRoot[i]);
	}

    giveSem(pHandle->rendscendDrawlock);

	return 0;
}


/*设置是否绘制瓦片边界线，0-不绘制，1-绘制*/
void setDrawTileGrid(int bDraw)
{
	bDrawTileGrid = bDraw;
}

/*设置是否绘制瓦片名称，0-不绘制，1-绘制*/
void setDrawTileName(int bDraw)
{
	bDrawTileName = bDraw;
}

// 获取当前瓦块最大等级
int GetCurMaxTileLvl()
{
	return sMaxLevel;
}