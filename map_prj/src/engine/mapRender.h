#ifndef _mapRender_h_ 
#define _mapRender_h_ 

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../mapApp/appHead.h"
#include "libList.h"
#include "frustum.h"
#include "../../../include_ttf/ttf_api.h"
#include "../../../include_vecMap/mapCommon.h"

#ifdef WIN32
#include "../../../SE_api_prj/src/SqliteOne.h"
#else
#include "../../../SE_api_prj/src/SqliteOne.h"	
#endif
#define zzqqxx_

#define TEST_PT_NUM   17
#define EDEG_TEST_PT_NUM (TEST_PT_NUM * 2)
#define MAXMAPLAYER 1
#define MAXQTNSQLNUM	1024

#define	DNST_WAITFORLOAD	(0x00000001)
#define	DNST_LOADING		(0x00000002)
#define	DNST_READY			(0x00000004)
#define	DNST_LOADFAILED		(0x00000008)
#define	DNST_REQFORDELETE	(0x00000010)
#define	DNST_READYFORDELETE (0x00000020)
#define DNST_EMPTY          (0x00000040)
#define DNST_SLEEP          (0x00000080)
#define DNST_ALL			(0x000000FF)

#define USE_FAULT_Tile		// 启用断层瓦块显示模式
//#undef	USE_FAULT_Tile		// 连续层瓦块显示模式

/* 正常模式瓦片每一行的网格个数 */
#define VTXCOUNT	    32
/* 正常模式瓦片每一行的顶点个数(网格顶点数=网格数+1) */
#define VTXCOUNT1   (VTXCOUNT + 1)
/* 正常模式瓦片绘制的顶点索引数组大小(用三角形的方式绘制) */
//#define  index_count (VTXCOUNT * (VTXCOUNT+4) * 2 * 3)
/* 正常模式瓦片绘制的顶点索引数组大小(用三角形条带的方式绘制) */
#define  index_count (2 * VTXCOUNT * VTXCOUNT + 12 * VTXCOUNT + 14)
/* 正常模式瓦片绘制的顶点索引数组 */
extern f_uint32_t gindices[];
/* 正常模式瓦片绘制的顶点数组大小 */
#define vertexNum  (VTXCOUNT1 * (VTXCOUNT1+4))	// +4:每一列的上下左右分别增加一个顶点
/* 正常模式瓦片绘制的顶点数组 */
extern f_float32_t gTextureBuf[];

/* 鹰眼模式瓦片每一行的网格个数 */
#define VTXCOUNT_EYEBIRD (VTXCOUNT / 4)
/* 鹰眼模式瓦片每一行的顶点个数 */
#define VTXCOUNT1_EYEBIRD (VTXCOUNT_EYEBIRD + 1)
/* 鹰眼模式瓦片绘制的顶点索引数组大小(用三角形的方式绘制) */
#define index_count_eyebird (VTXCOUNT_EYEBIRD * VTXCOUNT_EYEBIRD * 2 * 3)
/* 鹰眼模式瓦片绘制的顶点索引数组 */
extern f_uint32_t indices_eyebird[];
/* 鹰眼模式瓦片绘制的顶点数组大小 */
#define vertexNum_eyebird  (VTXCOUNT1_EYEBIRD * VTXCOUNT1_EYEBIRD)
/* 鹰眼模式瓦片绘制的顶点数组 */
extern f_float32_t gTextureBuf_eyebird[];



/*地图数据相关的API*/
typedef f_int32_t (*NODETERRAINLOADFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                                         f_uint64_t param, f_int16_t ** ppTerrain);   /* 地形数据加载函数 */
typedef f_int32_t (*NODEINFOLOADFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                                      f_uint64_t param, f_float32_t * ppInfo);          /* 瓦片信息加载函数 */
typedef f_int32_t (*NODEIMGLOADFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                                     f_uint64_t param, f_uint8_t ** ppImg);   /* 瓦片影像数据加载函数 */
typedef f_int32_t (*NODEIMGISEXISTFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param); /* 瓦片影像数据是否存在函数 */
typedef f_int32_t (*NODEIMG2TEXFUNC)(f_uint64_t param , f_uint8_t *pImg); 

typedef f_int32_t (*NODECREATEFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint64_t *pNodeID);
//typedef f_int32_t (*NODEFUNC)(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid, Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4]);
typedef f_int32_t (*NODEFUNC)(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid,
                                Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4], sSqliteName*  pList, f_int32_t* QtnNum);

typedef f_int32_t (*NODEFUNCDES)(f_uint64_t nodeid);
typedef f_int32_t (*LAYERFUNCPRE)(f_int32_t view_type, sTtfFont tfont, f_int32_t h_viewport[4], f_float64_t matrix[16]);
typedef f_int32_t (*LAYERFUNCPRo)(f_int32_t view_type, sTtfFont tfont);

/* 地图数据四叉树结构 */
typedef struct tagQTMapSubTree
{
	f_int32_t				 level; /*子树的根结点级别*/
	f_int32_t				 xidx;  /*子树的根结点x方向编号*/
	f_int32_t				 yidx;  /*子树的根结点y方向编号*/
	f_int32_t				 levels;/*子树本身的层数*/
	NODETERRAINLOADFUNC      funcloadterrain;
	f_uint64_t  			 loadterrainparam;
	NODEINFOLOADFUNC         infoloader;
	f_uint64_t               infoparam;
	NODEIMGISEXISTFUNC       imgisexist;
	NODEIMGLOADFUNC          imgloader;
	NODEIMG2TEXFUNC          img2tex;
	f_uint64_t               imgparam;	

	NODEIMGLOADFUNC          vqtloader;
	NODEIMG2TEXFUNC          vqt2tex;
	f_uint64_t               vqtparam;	

	NODEIMGLOADFUNC          clrloader;
	NODEIMG2TEXFUNC          clr2tex;
	f_uint64_t               clrparam;	

	NODEIMGLOADFUNC          hilloader;
	NODEIMG2TEXFUNC          hil2tex;
	f_uint64_t               hilparam;	
	
	
	stList_Head              stListHead;
}sQTMAPSUBTREE;

/* 节点范围 */
typedef struct tagNodeRange
{
	f_float64_t x, y, z;   /*瓦片中心点*/
	f_float64_t radius;    /*瓦片半径*/
	f_float64_t radiusMin;
}sNODERANGE;

typedef struct tagWarningData
{
	f_float32_t height; 
	f_float32_t color[4];
}WARNINGDATA;

typedef struct tagQtMapNode
{
	f_uint64_t  	    nodekey;                  /* 节点标志 */
	f_int32_t			level, xidx, yidx;        /* 节点所在层级、x、y方向的索引 */
	sQTMAPSUBTREE		*pSubTree;                /* 所在子树 */

	f_int32_t			createtime;               /* 节点创建时间 */
	f_int32_t			lastdraw;
	f_int32_t			drawnumber;
	f_int32_t           lastDrawNumber;
	f_uint64_t		    nodeid[MAXMAPLAYER];
	f_uint64_t  		datastatus; 
	f_int32_t           testPt_in_node;
	
	//sMAPBLOCK         vecBlock;
	f_uint32_t          edgeList;
	f_uint32_t          edgeListOverlook;
	f_uint32_t          tileList;
	f_uint32_t          tileListOverlook;
	f_uint32_t          tileVecShadeList;
#ifdef _JM7200_
	f_uint32_t          tileSVSList;
	f_uint32_t          tileSVSGridList;
#endif
	f_uint32_t          tileVertexVBO;
	f_uint32_t          tileTerrainVBO;
	f_uint32_t          tileColorVBO;
	f_uint32_t          tileNormalVBO;
	f_uint32_t          tileGridTexVBO;
	f_uint32_t          tileOverlookVertexVBO;
	SEMID               semlock;
	f_uint32_t		    texid;                         /* 纹理id   */	 
	f_uint8_t *		    imgdata;                       /* 影像数据 */
	f_float32_t *	    fvertex;                       /* 顶点数据 */
	f_float32_t *	    fvertex_flat;                       /* 平面的顶点数据 */

	f_char_t*			pNormal;					   /* 瓦片法线数据 */
    f_float32_t *       fterrain;                      /* 地形高程数据 */
	f_uint32_t		vtexid;			/*矢量纹理id*/
	f_uint8_t *		    vqtdata;                       /* 矢量纹理数据 */

	f_uint32_t		clrtexid;			/*地貌晕眩纹理id*/
	f_uint8_t *		    clrdata;             /* 地貌晕眩纹理数据 */

	f_uint32_t		hiltexid;			/*shade 纹理id*/
	f_uint8_t *		    hildata;             /* shade 纹理数据 */

	f_uint32_t		texUsed[2];		/*两个纹理对象对应的纹理ID，卫片0，矢量1，晕渲2，阴影3*/
	
	f_float32_t         hstart, vstart, hscale, vscale;              /* 瓦片左下角起始经度、起始纬度、经度跨度、纬度跨度 */
	f_float32_t         height_lt, height_lb, height_rt, height_rb, height_ct;  /* 图幅左上、左下、右上、右下角、中心点高度 */
	PT_3D               obj_lt;
	PT_3D               obj_rt;
	PT_3D               obj_rb;
	PT_3D               obj_lb;
	PT_3D               obj_cent;                      /* 节点中心的物体坐标 */	
	sNODERANGE nodeRange;	
    struct tagQtMapNode *parent;                       /* 父节点 */
	struct tagQtMapNode *children[4];                  /* 子节点 */
    stList_Head         stListHead;		
    stList_Head         stListHeadDrawing;		/*需要绘制的链表节点*/	
	
	f_float32_t*	m_pVertexTex;	//纹理坐标指针
	f_int32_t*	fvertex_part;		//整数化的局部坐标
	f_float64_t	point_vertex_xyz[4];	//x, y, z, scale
	f_float32_t*	m_pColor;		//瓦片颜色数据

	PT_3I	fvertex_part_overlook[8];		//整数化的局部坐标俯视下坐标,左下角开始逆时针
	f_float32_t fvertex_overlook[24];			//瓦片的四个角点坐标，xyz 值
	sNODERANGE nodeRangeOverlook;		//俯视下的包围球

	sNODERANGE nodeRangeOsg;			//计算瓦片的精准包围球

	int tqsInFile;		// tqs是否在文件树中标识符(0:不在;1:在)
	struct tagQtMapNode * tqsParentInFile;// 记录存在在文件树中的最近父节点
	int tqsUsedNumInChild;	// 记录 子节点使用了本节点纹理资源的数量(子节点使用时+1,子节点析构时-1)
}sQTMAPNODE;

typedef struct tagQtMapFunc
{
	f_uint64_t   			param;
	NODECREATEFUNC			funccreate;
	NODEFUNCDES				funcdestroy;
	NODEFUNC				funcdisplay;
	LAYERFUNCPRE    		funcdisplaypre;
	LAYERFUNCPRo            funcdisplaypro;
}sQTMAPFUNC;


typedef struct tagQtnNode
{
	f_int32_t			qtn_num[QTNSQLNUM];	//需要显示的注记个数
	sSqliteName		qtn_sql[QTNSQLNUM * MAXQTNSQLNUM];	//注记结构体数组
	f_uint32_t 		qtn_status;	// 是否需要调度qtn, 0 : 不需要，1: 需要
//	f_float64_t 		pre_cameraLonLatHei[3];	//上一次的视点经纬度
//	f_float64_t 		pre_cameraYPR[3];	//上一次的视点经纬度

//	f_float64_t		roam_res;		//漫游情况下的反算量程分辨率
	f_float32_t 		qtn_lonlat_range[4];//lon_low, lon_hi, lat_low, lat_hi;

}sQtnNode;


typedef struct tagMapHandle
{
	f_uint64_t  			key;         /*must be MAPHANDLEKEY*/
	sQTMAPSUBTREE		    sub_tree;    /*已经加入链表中的子树，用来判断某个结点是否存在数据库中*/
	
	f_int32_t				nodecount;   /*已经调入内存的结点数量*/
	sQTMAPNODE			    nodehead;    /*结点LRU表*/
	sQTMAPNODE *		    pNodeRoot[8];   /*根结点*/
	f_int32_t				drawnumber;		//当前的绘制帧计数

//    sCAMERACTRLPARAM        cameractl; 
//    sMODECTRLCMD            mdctrl;
//    Geo_Pt_D                cam_geo_pos; 
//    Obj_Pt_D                cam_obj_pos;
//    sATTITUDE               atd;
//	f_int32_t               prjmode;         /**< 投影类型     */
	
//	FRUSTUM                 frustum;     /*视景体参数*/
	f_int32_t               drawnnodnum; /* 当前帧已绘制的节点数 */
	f_int32_t               loadedNumber;/* 已加载的节点数 */

	f_int32_t               createTexNumber;
	f_int32_t               delTexNumber;
	f_int32_t               maxTexNumber;
	f_int32_t               startTick;
	f_int32_t               delNodeNumber;
	
//	f_int32_t               h_viewport[4];
//	f_float64_t				h_matrix[16];
//	f_float64_t             h_invmat[16];

//	BOOL                    is_need_terwarning;
//	f_int32_t				warn_texid;          // 地形告警一维纹理对象
//	f_float32_t             terwaring_height;
//	f_float32_t             s_nTexCoordPerMeter;  // 每米所需要的纹理坐标间隔
//	f_float64_t             cur_plane_height;
//	sTerWarningSet          m_terwarning_set;
//	BOOL                    is_warn_data_set_success;  // 参数设置成功标志
//	f_uint32_t              atms_text;            /**< 用于大气层的一维纹理 */
//   	f_uint32_t              atms_textgray;        /**< 用于大气层的灰度一维纹理 */
	
	sTtfFont                ttf_font;
	sQTMAPFUNC				layerfunc[MAXMAPLAYER];
	
    f_int32_t               loadthread;
	SEMID                   anyrequire;
	SEMID                   threadended;
	SEMID                   endrequire;
    SEMID                   rendscenelock;
    SEMID                   rendscendDrawlock;  // 影像绘制一帧锁

	SEMID		 	qtnthread;	//qtn 数据是否在调度lpf add 2017年10月20日10:29:37
	sQtnNode			qtn_node[SCENE_NUM];		//qtn 数据的结构体数组

	sMAPTREE         *map_trees;           /**< 地图树     */
	f_int32_t        data3d_init_status;   /**< 3D数据初始化状态     */	
	
}sMAPHANDLE;

#define LEVEL_MAX	25
extern f_float64_t glfLevelDelta[LEVEL_MAX];

/*
功能：地图句柄创建
输入：
    无
输出：创建成功标志
    0  成功
    -1 失败
*/
f_int32_t createMapHandle();

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
						       f_uint64_t          hilparam	);
						       
/*
	设置各个图层的功能函数
	参数：map_handle -- 用createMapHandle返回的VOIDPtr句柄
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
						    LAYERFUNCPRo funcdisplaypro);						       

/*
功能：更新参数
*/						 
f_int32_t updateRenderParam(VOIDPtr map_handle, sTtfFont ttf_font, 
                            const f_int32_t viewport[4], const f_float64_t matrix[16], const f_float64_t invmat[16],
                            f_int32_t prjmd, sCAMERACTRLPARAM camctl, sMODECTRLCMD mdctrl, sATTITUDE attitude,
                            Geo_Pt_D cam_geo_pos, Obj_Pt_D cam_obj_pos, f_float64_t cur_plane_height);

f_int32_t drawListDeleteNode(sGLRENDERSCENE *pScene);

f_int32_t qtmapRender(VOIDPtr map_handle, f_int32_t terwarn_md, f_float32_t pitch, VOIDPtr scene);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
