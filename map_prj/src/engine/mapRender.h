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

#define USE_FAULT_Tile		// ���öϲ��߿���ʾģʽ
//#undef	USE_FAULT_Tile		// �������߿���ʾģʽ

/* ����ģʽ��Ƭÿһ�е�������� */
#define VTXCOUNT	    32
/* ����ģʽ��Ƭÿһ�еĶ������(���񶥵���=������+1) */
#define VTXCOUNT1   (VTXCOUNT + 1)
/* ����ģʽ��Ƭ���ƵĶ������������С(�������εķ�ʽ����) */
//#define  index_count (VTXCOUNT * (VTXCOUNT+4) * 2 * 3)
/* ����ģʽ��Ƭ���ƵĶ������������С(�������������ķ�ʽ����) */
#define  index_count (2 * VTXCOUNT * VTXCOUNT + 12 * VTXCOUNT + 14)
/* ����ģʽ��Ƭ���ƵĶ����������� */
extern f_uint32_t gindices[];
/* ����ģʽ��Ƭ���ƵĶ��������С */
#define vertexNum  (VTXCOUNT1 * (VTXCOUNT1+4))	// +4:ÿһ�е��������ҷֱ�����һ������
/* ����ģʽ��Ƭ���ƵĶ������� */
extern f_float32_t gTextureBuf[];

/* ӥ��ģʽ��Ƭÿһ�е�������� */
#define VTXCOUNT_EYEBIRD (VTXCOUNT / 4)
/* ӥ��ģʽ��Ƭÿһ�еĶ������ */
#define VTXCOUNT1_EYEBIRD (VTXCOUNT_EYEBIRD + 1)
/* ӥ��ģʽ��Ƭ���ƵĶ������������С(�������εķ�ʽ����) */
#define index_count_eyebird (VTXCOUNT_EYEBIRD * VTXCOUNT_EYEBIRD * 2 * 3)
/* ӥ��ģʽ��Ƭ���ƵĶ����������� */
extern f_uint32_t indices_eyebird[];
/* ӥ��ģʽ��Ƭ���ƵĶ��������С */
#define vertexNum_eyebird  (VTXCOUNT1_EYEBIRD * VTXCOUNT1_EYEBIRD)
/* ӥ��ģʽ��Ƭ���ƵĶ������� */
extern f_float32_t gTextureBuf_eyebird[];



/*��ͼ������ص�API*/
typedef f_int32_t (*NODETERRAINLOADFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                                         f_uint64_t param, f_int16_t ** ppTerrain);   /* �������ݼ��غ��� */
typedef f_int32_t (*NODEINFOLOADFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                                      f_uint64_t param, f_float32_t * ppInfo);          /* ��Ƭ��Ϣ���غ��� */
typedef f_int32_t (*NODEIMGLOADFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                                     f_uint64_t param, f_uint8_t ** ppImg);   /* ��ƬӰ�����ݼ��غ��� */
typedef f_int32_t (*NODEIMGISEXISTFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param); /* ��ƬӰ�������Ƿ���ں��� */
typedef f_int32_t (*NODEIMG2TEXFUNC)(f_uint64_t param , f_uint8_t *pImg); 

typedef f_int32_t (*NODECREATEFUNC)(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint64_t *pNodeID);
//typedef f_int32_t (*NODEFUNC)(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid, Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4]);
typedef f_int32_t (*NODEFUNC)(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid,
                                Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4], sSqliteName*  pList, f_int32_t* QtnNum);

typedef f_int32_t (*NODEFUNCDES)(f_uint64_t nodeid);
typedef f_int32_t (*LAYERFUNCPRE)(f_int32_t view_type, sTtfFont tfont, f_int32_t h_viewport[4], f_float64_t matrix[16]);
typedef f_int32_t (*LAYERFUNCPRo)(f_int32_t view_type, sTtfFont tfont);

/* ��ͼ�����Ĳ����ṹ */
typedef struct tagQTMapSubTree
{
	f_int32_t				 level; /*�����ĸ���㼶��*/
	f_int32_t				 xidx;  /*�����ĸ����x������*/
	f_int32_t				 yidx;  /*�����ĸ����y������*/
	f_int32_t				 levels;/*��������Ĳ���*/
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

/* �ڵ㷶Χ */
typedef struct tagNodeRange
{
	f_float64_t x, y, z;   /*��Ƭ���ĵ�*/
	f_float64_t radius;    /*��Ƭ�뾶*/
	f_float64_t radiusMin;
}sNODERANGE;

typedef struct tagWarningData
{
	f_float32_t height; 
	f_float32_t color[4];
}WARNINGDATA;

typedef struct tagQtMapNode
{
	f_uint64_t  	    nodekey;                  /* �ڵ��־ */
	f_int32_t			level, xidx, yidx;        /* �ڵ����ڲ㼶��x��y��������� */
	sQTMAPSUBTREE		*pSubTree;                /* �������� */

	f_int32_t			createtime;               /* �ڵ㴴��ʱ�� */
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
	f_uint32_t		    texid;                         /* ����id   */	 
	f_uint8_t *		    imgdata;                       /* Ӱ������ */
	f_float32_t *	    fvertex;                       /* �������� */
	f_float32_t *	    fvertex_flat;                       /* ƽ��Ķ������� */

	f_char_t*			pNormal;					   /* ��Ƭ�������� */
    f_float32_t *       fterrain;                      /* ���θ߳����� */
	f_uint32_t		vtexid;			/*ʸ������id*/
	f_uint8_t *		    vqtdata;                       /* ʸ���������� */

	f_uint32_t		clrtexid;			/*��ò��ѣ����id*/
	f_uint8_t *		    clrdata;             /* ��ò��ѣ�������� */

	f_uint32_t		hiltexid;			/*shade ����id*/
	f_uint8_t *		    hildata;             /* shade �������� */

	f_uint32_t		texUsed[2];		/*������������Ӧ������ID����Ƭ0��ʸ��1������2����Ӱ3*/
	
	f_float32_t         hstart, vstart, hscale, vscale;              /* ��Ƭ���½���ʼ���ȡ���ʼγ�ȡ����ȿ�ȡ�γ�ȿ�� */
	f_float32_t         height_lt, height_lb, height_rt, height_rb, height_ct;  /* ͼ�����ϡ����¡����ϡ����½ǡ����ĵ�߶� */
	PT_3D               obj_lt;
	PT_3D               obj_rt;
	PT_3D               obj_rb;
	PT_3D               obj_lb;
	PT_3D               obj_cent;                      /* �ڵ����ĵ��������� */	
	sNODERANGE nodeRange;	
    struct tagQtMapNode *parent;                       /* ���ڵ� */
	struct tagQtMapNode *children[4];                  /* �ӽڵ� */
    stList_Head         stListHead;		
    stList_Head         stListHeadDrawing;		/*��Ҫ���Ƶ�����ڵ�*/	
	
	f_float32_t*	m_pVertexTex;	//��������ָ��
	f_int32_t*	fvertex_part;		//�������ľֲ�����
	f_float64_t	point_vertex_xyz[4];	//x, y, z, scale
	f_float32_t*	m_pColor;		//��Ƭ��ɫ����

	PT_3I	fvertex_part_overlook[8];		//�������ľֲ����긩��������,���½ǿ�ʼ��ʱ��
	f_float32_t fvertex_overlook[24];			//��Ƭ���ĸ��ǵ����꣬xyz ֵ
	sNODERANGE nodeRangeOverlook;		//�����µİ�Χ��

	sNODERANGE nodeRangeOsg;			//������Ƭ�ľ�׼��Χ��

	int tqsInFile;		// tqs�Ƿ����ļ����б�ʶ��(0:����;1:��)
	struct tagQtMapNode * tqsParentInFile;// ��¼�������ļ����е�������ڵ�
	int tqsUsedNumInChild;	// ��¼ �ӽڵ�ʹ���˱��ڵ�������Դ������(�ӽڵ�ʹ��ʱ+1,�ӽڵ�����ʱ-1)
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
	f_int32_t			qtn_num[QTNSQLNUM];	//��Ҫ��ʾ��ע�Ǹ���
	sSqliteName		qtn_sql[QTNSQLNUM * MAXQTNSQLNUM];	//ע�ǽṹ������
	f_uint32_t 		qtn_status;	// �Ƿ���Ҫ����qtn, 0 : ����Ҫ��1: ��Ҫ
//	f_float64_t 		pre_cameraLonLatHei[3];	//��һ�ε��ӵ㾭γ��
//	f_float64_t 		pre_cameraYPR[3];	//��һ�ε��ӵ㾭γ��

//	f_float64_t		roam_res;		//��������µķ������̷ֱ���
	f_float32_t 		qtn_lonlat_range[4];//lon_low, lon_hi, lat_low, lat_hi;

}sQtnNode;


typedef struct tagMapHandle
{
	f_uint64_t  			key;         /*must be MAPHANDLEKEY*/
	sQTMAPSUBTREE		    sub_tree;    /*�Ѿ����������е������������ж�ĳ������Ƿ�������ݿ���*/
	
	f_int32_t				nodecount;   /*�Ѿ������ڴ�Ľ������*/
	sQTMAPNODE			    nodehead;    /*���LRU��*/
	sQTMAPNODE *		    pNodeRoot[8];   /*�����*/
	f_int32_t				drawnumber;		//��ǰ�Ļ���֡����

//    sCAMERACTRLPARAM        cameractl; 
//    sMODECTRLCMD            mdctrl;
//    Geo_Pt_D                cam_geo_pos; 
//    Obj_Pt_D                cam_obj_pos;
//    sATTITUDE               atd;
//	f_int32_t               prjmode;         /**< ͶӰ����     */
	
//	FRUSTUM                 frustum;     /*�Ӿ������*/
	f_int32_t               drawnnodnum; /* ��ǰ֡�ѻ��ƵĽڵ��� */
	f_int32_t               loadedNumber;/* �Ѽ��صĽڵ��� */

	f_int32_t               createTexNumber;
	f_int32_t               delTexNumber;
	f_int32_t               maxTexNumber;
	f_int32_t               startTick;
	f_int32_t               delNodeNumber;
	
//	f_int32_t               h_viewport[4];
//	f_float64_t				h_matrix[16];
//	f_float64_t             h_invmat[16];

//	BOOL                    is_need_terwarning;
//	f_int32_t				warn_texid;          // ���θ澯һά�������
//	f_float32_t             terwaring_height;
//	f_float32_t             s_nTexCoordPerMeter;  // ÿ������Ҫ������������
//	f_float64_t             cur_plane_height;
//	sTerWarningSet          m_terwarning_set;
//	BOOL                    is_warn_data_set_success;  // �������óɹ���־
//	f_uint32_t              atms_text;            /**< ���ڴ������һά���� */
//   	f_uint32_t              atms_textgray;        /**< ���ڴ�����ĻҶ�һά���� */
	
	sTtfFont                ttf_font;
	sQTMAPFUNC				layerfunc[MAXMAPLAYER];
	
    f_int32_t               loadthread;
	SEMID                   anyrequire;
	SEMID                   threadended;
	SEMID                   endrequire;
    SEMID                   rendscenelock;
    SEMID                   rendscendDrawlock;  // Ӱ�����һ֡��

	SEMID		 	qtnthread;	//qtn �����Ƿ��ڵ���lpf add 2017��10��20��10:29:37
	sQtnNode			qtn_node[SCENE_NUM];		//qtn ���ݵĽṹ������

	sMAPTREE         *map_trees;           /**< ��ͼ��     */
	f_int32_t        data3d_init_status;   /**< 3D���ݳ�ʼ��״̬     */	
	
}sMAPHANDLE;

#define LEVEL_MAX	25
extern f_float64_t glfLevelDelta[LEVEL_MAX];

/*
���ܣ���ͼ�������
���룺
    ��
����������ɹ���־
    0  �ɹ�
    -1 ʧ��
*/
f_int32_t createMapHandle();

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
						       f_uint64_t          hilparam	);
						       
/*
	���ø���ͼ��Ĺ��ܺ���
	������map_handle -- ��createMapHandle���ص�VOIDPtr���
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
						    LAYERFUNCPRo funcdisplaypro);						       

/*
���ܣ����²���
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
