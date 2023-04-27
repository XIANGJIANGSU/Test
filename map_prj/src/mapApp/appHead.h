/**
 * @file appHead.h
 * @brief 该文件定义了app层的数据结构
 * @author 615地图团队
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _APP_HEAD_h_ 
#define _APP_HEAD_h_ 

#include "../mapApi/common.h"
#include "../define/macrodefine.h"
#include "../define/mbaseType.h"
#include "../engine/osAdapter.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"
#include "../engine/Roam.h"
#include "../engine/libList.h"
#include "fileSort.h"
#include "../engine/TerrainMap.h"

/** @struct sSCREENPARAM
*  @brief 显示器参数
*
*/
typedef struct tagScreenParam{
	f_float32_t phi_width;     /**<  显示器的物理宽度(单位：厘米) */
	f_float32_t phi_height;    /**<  显示器的物理高度(单位：厘米) */
	f_int32_t   rotate_type;   /**<  显示器的旋转方式,0:正常,1:顺时针90度,2:顺时针180度,3:顺时针270度 */
	f_int32_t   width;         /**<  显示器的分辨率宽度(单位：像素) */
	f_int32_t   height;        /**<  显示器的分辨率宽度(单位：像素) */
	}sSCREENPARAM;

/** @struct sViewWindowSize
*  @brief 视窗位置参数
*
*/	
typedef struct tagViewWindowSize{
    f_int32_t llx;
    f_int32_t lly;              /**< 视窗左下角的位置，单位：像素. */
    f_int32_t width;            /**< 视窗的宽度，单位：像素. */
    f_int32_t height;           /**< 视窗的高度，单位：像素. */
	}sViewWindowSize;
	
/** @struct sViewWindowParam
*  @brief 视窗相关控制
*
*/
typedef struct tagViewWindowParam
{
    sViewWindowSize outer_view_port;    /**< 外部视窗     */    
    f_float32_t     alpha;              /**< 视窗的透明度 */
    sColor3f        bkcolor;            /**< 视窗的背景色 */
    f_float32_t     rtangle;            /**< 视窗的旋转角（单位：度） */
    sViewWindowSize inner_view_port;    /**< 内部视窗     */  
}sViewWindowParam;

/** @struct sCAMERACTRLPARAM
*  @brief 相机控制参数
*
*/
typedef struct tagCameraCtrlParam
{
	f_float32_t view_near;          /**< 近裁截面的距离（单位：米）   */
	f_float32_t view_far;           /**< 远裁截面的距离（单位：米）   */
	f_float32_t view_angle;         /**< 平截头视角(单位：度)         */
	f_float32_t view_res;           /**< 显示分辨率 （单位：米/像素） */    
	f_int32_t   compass_radius;     /**< 刻度环半径（单位：像素）     */
	f_int32_t   view_type;          /**< 视角类型                     */
    PT_2I       rotate_center;      /**< 地图旋转中心（单位：像素）   */
}sCAMERACTRLPARAM;

/** @struct sCameraParam
*  @brief 相机参数
*
*/
typedef struct tagCameraParam
{
	Obj_Pt_D    m_ptEye;		/**< 视点位置(球坐标) */
	Geo_Pt_D    m_geoptEye;     /**< 视点位置(地理坐标) */
	PT_3D		m_ptCenter;		/**< 视点中心点位置(高度为0的地面点) */
	PT_3D		m_ptUp;			/**< 相机上方向 */
	PT_3D		m_ptTo;			/**< 相机方向 */
	f_float64_t	m_lfDistance;	/**< 相机到中心点(地面点)的距离 */
	f_float64_t	m_lfNear;		/**< 视锥体近裁剪面 */
	f_float64_t	m_lfFar;		/**< 视锥体远裁剪面 */
}sCameraParam;

/** @struct sModeCtrlCmd
*  @brief 地图显示模式控制命令
*
*/
typedef struct tagModeCtrlCmd
{
	f_int32_t cor_mode;      /**< 颜色模式 */
    f_int32_t dn_mode;       /**< 昼夜模式 */
    f_int32_t mv_mode;       /**< 运动模式 */
    f_int32_t frz_mode;      /**< 冻结模式 */
    f_int32_t rom_mode;      /**< 漫游模式 */
    f_int32_t terwarn_mode;  /**< 地形告警模式 */
    f_int32_t scene_mode;    /**< 场景模式(0:正常模式,1:鹰眼模式)  */
    f_int32_t map2dor3d;	/*0:  矢量显示或者1 :不显示矢量*/
    f_int32_t mapDisplay_mode;	/*地图图层叠加方式0:卫片tqs/tqt,1:卫片+矢量vqt(暂时未使用),2:晕渲clr,3:晕渲+阴影hil(暂时未使用),4:矢量化栅格,5:纯矢量*/
	f_int32_t close3d_mode;	/*屏蔽三维开关0: 不屏蔽, 1 : 屏蔽*/
	f_int32_t rom2d_mode;	/**<俯视下的漫游模式>*/
	f_int32_t rom2d_mode_old;   /**<俯视下的前一帧的漫游模式，为了地稳模式漫游后能回到原位>*/
	f_int32_t svs_mode;		/**<svs的经纬网光照显示模式>*/
	f_int32_t is_3dplane_display;	/**<是否显示3D模型>*/
	f_int32_t plane_symbol_display;	/**<是否显示飞机符号0:不显示,1:显示>*/
	f_int32_t color_layermask_cfg_index;	/**<颜色防拥配置文件索引号>*/
	f_int32_t area_ba_draw;	        /**<是否绘制矢量政区面0:不绘制,1:绘制>*/
	f_int32_t sea_map_draw;	        /**<是否绘制海图0:不绘制,1:绘制>*/
	f_int32_t text_draw_level;      /* 地名注记和符号显示档位0-不显示任何注记和符号 1-只显示省会城市和中等城市的地名注记和符号 2-正常显示所有的注记和符号*/
}sMODECTRLCMD;

/** @struct sATTITUDE
*  @brief 航姿
*
*/
typedef struct tagAttitude
{
    f_float64_t yaw;     /**< 航向角 */
	f_float64_t mag;     /**< 磁差角 */
    f_float64_t pitch;   /**< 俯仰角 */
    f_float64_t roll;    /**< 横滚角 */
}sATTITUDE;

/** @struct sMAPTREE
*  @brief 树
*
*/
typedef struct tagMapTree
{
	f_int32_t rootlevel;      /**< 根节点的层级 */
	f_int32_t rootxidx;       /**< 根节点的x方向索引号 */
	f_int32_t rootyidx;       /**< 根节点的y方向索引号 */
	f_int32_t level;          /**< 树的深度      */
	VOIDPtr   texfile;        /**< 树所对应的纹理数据 */
	VOIDPtr   terfile;        /**< 树所对应的地形数据 */
	VOIDPtr   infofile;       /**< 树所对应的节点信息数据 */
	VOIDPtr	vqtfile;	    /**<树所对应的矢量纹理数据*/
	VOIDPtr	clrfile;	    /**<树所对应的晕眩纹理数据*/
	VOIDPtr	hilfile;	   /**<树所对应的shade纹理数据*/
}sMAPTREE;

/** @struct sMAPFILE
*  @brief 数据文件
*
*/
typedef struct tagMapFile{
	f_int32_t fcount;       /**< 文件数 */
	sQTFName *fName_list;   /**< tqt 文件名列表 */

//	f_int32_t fcount_vqt;	/*vqt 文件个数*/
//	sQTFName *fName_list_vqt;	/*vqt 文件名列表*/
	}sMAPFILE;

/** @struct sTerWarningData
*  @brief 地形告警颜色设置
*
*/
typedef struct tagTerWarningData{
    f_int32_t   index;        /**< 层级索引 */
	f_float32_t height_min;   /**< 告警高度区间的最低值 */
	f_float32_t height_max;   /**< 告警高度区间的最高值 */
	f_float32_t red;          /**< 红色分量 */
	f_float32_t green;        /**< 绿色分量 */
	f_float32_t blue;         /**< 蓝色分量 */
}sTerWarningData;

/** @struct sTerWarningData
*  @brief 地形告警颜色设置集合
*
*/
typedef struct sTerWarningSet{
    f_int32_t   layers;              /**< 总级数 */
	f_float32_t height_max;          /**< 告警高度的最大值 */
	f_int32_t   height_max_index;    /**< 告警高度最大值对应的层级索引 */
	f_float32_t height_min;          /**< 告警高度的最小值 */
	f_int32_t   height_min_index;    /**< 告警高度最小值对应的层级索引 */
	sTerWarningData ter_warning_data[TER_WARNING_MAX_LAYERS]; /**< 告警设置 */
}sTerWarningSet;

/** @struct sAuxNavInfoDisSwitch
*  @brief 辅助导航信息显示参数
*
*/
typedef struct sAuxNavInfoParam{
	sZplParam zpl_param;   /**< 0俯仰角线参数 */
	sCpsParam cps_param;   /**< 罗盘参数      */
	sPrefileParam		prefile_param;			/**剖面图的参数设置表*/
	BOOL is_pos_dis;       /**< 位置信息是否显示开关 FALSE：不显示，TRUE：显示 */
	}sAuxNavInfoParam;

typedef struct tagDisList{
	f_int32_t displaylist_id;
	f_int32_t texture_id;
	}sDisList;	
	
/** @struct sAuxNavInfoList
*  @brief 辅助导航信息list
*
*/
typedef struct tagAuxNavInfoList{
	sDisList zpl_list;      
	sDisList cps_list;     
	sDisList pos_list;      
	}sAuxNavInfoList;

typedef struct tagPLANE
{
	f_float64_t A;
	f_float64_t B;
	f_float64_t C;
	f_float64_t D;
}PLANE, *LPPLANE;	// 视景体面

typedef struct tagFRUSTUM
{
	PLANE plane[6];
}FRUSTUM, *LPFRUSTUM;	// 视景体6面体

/** @struct sGLRENDERSCENE
*  @brief 场景结构
*
*/
typedef struct tagGLRenderScene{
//	VOIDPtr          map_handle;           /**< 地图句柄   */
	f_uint32_t       scene_index;          /**< 场景索引     */
	f_int32_t        project_mode;         /**< 投影类型  0墨卡托 1兰伯特 2球坐标*/
	
	FUNCTION         func2dMap;            /**< 二维地图功能的入口函数 */
	FUNCTION         func3dMap;            /**< 三维地图功能的入口函数 */
	USERLAYERFUNCTION  drawScreenUserLayer;   /**< 用户屏幕图层功能的入口函数 */
	USERLAYERFUNCTION  drawGeoUserLayer;      /**< 用户地理图层功能的入口函数 */
	FUNCTION         funcScreenPt2GeoPt;   /**< 屏幕坐标到地理坐标功能的入口函数 */
	FUNCTION         funcGeoPt2ScreenPt;   /**< 地理坐标到屏幕坐标功能的入口函数 */
	
//	sMAPTREE         *map_trees;           /**< 地图树     */
//	f_int32_t        data3d_init_status;   /**< 3D数据初始化状态     */	
//	sTtfFont         ttf_font;             /**< ttf 字体 */
	
	SEMID            periodparaminput_sem; /**< 设置周期参数(应用层参数输入到param_input)的信号量     */
	SEMID            periodparamset_sem;   /**< 设置周期参数(从param_input到引擎内部使用的param)的信号量     */	
	SEMID            eventparaminput_sem;  /**< 设置事件参数(应用层参数输入到param_input)的信号量     */
	SEMID            eventparamset_sem;    /**< 设置事件参数(从param_input到引擎内部使用的param)的信号量     */
	
	sCAMERACTRLPARAM camctrl_param_input;  /**< 外部接收到的相机控制参数 */
	sMODECTRLCMD     mdctrl_cmd_input;     /**< 外部接收到的模式控制命令 */
	Geo_Pt_D         geopt_pos_input;          /**< 外部接收到的位置信息，在三方视角时，输入的是焦点位置；
	                                           在其它视角时，输入的是视点位置（在俯视情况下，视点高度是根据量程反算过来的）； */	
    PT_2I            rom_pt_input;         /**< 外部接收到的漫游位置 */	                                           
	sATTITUDE        attitude_input;       /**< 外部接收到的航姿信息 */
	sTerWarningSet   m_terwarning_set_input;  /**< 外部接收到的地形告警参数设置 */
	sAuxNavInfoParam auxnavinfo_param_input;   /**< 外部接收到的辅助导航信息显示参数 */

	f_int32_t is_romed;          /**< 是否已经漫游过的标志0:还没漫游,1:已漫游  */
	f_int32_t is_rom_homed;      /**< 是否已经漫游归位过的标志0:还没漫游归位,1:已漫游归位  */
	
	sCAMERACTRLPARAM camctrl_param;        /**< 引擎内部使用的相机控制参数 */
	sMODECTRLCMD     mdctrl_cmd;           /**< 引擎内部使用的模式控制命令 */
	Geo_Pt_D         geopt_pos;            /**< 引擎内部使用的飞行参数经纬高信息 */
	Geo_Pt_D         planesymbol_pos;      /**< 引擎内部使用的飞机符号经纬高信息 */
	f_float64_t      planesymbol_yaw;      /**< 引擎内部使用的飞机符号航向信息 */
	PT_2I            rom_pt;               /**< 外部接收到的漫游位置 */	
	sATTITUDE        attitude;             /**< 引擎内部使用的航姿信息 */
	sTerWarningSet   m_terwarning_set;     /**< 引擎内部使用的地形告警参数设置 */
	Obj_Pt_D         objpt_pos;            /**< 飞行参数经纬高对应的物体坐标点 */
	sAuxNavInfoParam auxnavinfo_param;     /**< 引擎内部使用的辅助导航信息显示参数 */
	sAuxNavInfoList  auxnav_list;         /**< 辅助导航信息的显示列表 */
	
	//Geo_Pt_D         cam_geo_pos;          /**< 相机的地理坐标系下的位置  */
	//Obj_Pt_D         cam_obj_pos;          /**< 相机的物体坐标系下的位置  */
	
	sCameraParam     camParam;             /**< 相机参数                */
	Matrix44	     m_mxRotate;           /**< 局部坐标的姿态旋转矩阵  */
	Matrix44	     m_mxWorld;	           /**< 局部坐标到世界坐标变换矩阵  */
	f_float64_t	     m_lfProjMatrix[16];   /**< 投影矩阵  */
	f_float64_t	     m_lfModelMatrix[16];  /**< 模视矩阵  */
	f_int32_t        outerviewport[4];     /**< 外部视口参数  */
	f_int32_t        innerviewport[4];     /**< 内部视口参数  */
	f_float64_t		 matrix[16];           /**< 裁剪矩阵  */
	f_float64_t      invmat[16];           /**< 裁剪矩阵逆矩阵  */
	
	//f_float64_t      m_lfDelta;		   /**< 屏幕尺寸到三维空间尺寸的比例 */
	PT_2D            rotate_opengl_pt;     /**< 俯视模式下旋转中心的openGL坐标 */
	
	RoamT            roam;  //DXJ
	stList_Head 	pScene_ModelList;	/*场景内的模型链表节点头地址lpf add */	
	f_int32_t		airplane_id;			/*场景内的飞机模型id lpf add */

//    Geo_Pt_D                cam_geo_pos; 
//    Obj_Pt_D                cam_obj_pos;
	
	FRUSTUM                 frustum;     /*视景体参数*/

	BOOL                    is_need_terwarning;
	f_int32_t				warn_texid;          // 地形告警一维纹理对象
	f_int32_t				is_need_regenerate_warn_texid;          // 地形告警一维纹理对象是否需重新生成
	f_float32_t             terwaring_height;
	f_float32_t             s_nTexCoordPerMeter;  // 每米所需要的纹理坐标间隔
	f_float64_t             cur_plane_height;
//	sTerWarningSet          m_terwarning_set;
	BOOL                    is_warn_data_set_success;  // 参数设置成功标志
	f_uint32_t              atms_text;            /**< 用于大气层的一维纹理 */
   	f_uint32_t              atms_textgray;        /**< 用于大气层的灰度一维纹理 */

	f_float64_t		roam_res;			//漫游情况下的反算量程分辨率
	f_float64_t		roam_res_for_QTN;	//漫游情况下适用于QTN的反算量程分辨率
	
	stList_Head		scene_draw_list;	//需要绘制的节点链表

	f_float64_t 		pre_cameraLonLatHei[3];	//上一次的视点经纬度
	f_float64_t 		pre_cameraYPR[3];	//上一次的视点经纬度
	f_uint32_t		qtn_nodeID;				//需要采用的调度qtn 数组的ID 号

	sPrefileParamUsed	prefile_param_used;		//内部使用的垂直地形剖面图细化参数

	TerrainMapContext termap_param_input;		/**<外部接收的航路点地形剖面图参数>*/
	TerrainMapContext termap_param;			    /**<内部使用的航路点地形剖面图参数>*/
	TerrainMapUsed	terrainMap_param_used;	    //内部绘制时使用的航路点地形剖面图细化参数
	f_uint32_t       set_center_pos;          /**< 地稳模式下是否响应设置中心点功能，0-不响应，1-响应 */
	Geo_Pt_D         centerpt_pos;            /**< 地稳模式下使用的设置中心点经纬高信息 */
	Geo_Pt_D         roam_start_pt_pos;       /**< 由非漫游进入漫游时，记录当前中心点经纬高信息，用于漫游归位 */
	}sGLRENDERSCENE;

/** @struct sViewWindow
*  @brief 视窗结构
*
*/	
typedef struct tagViewWindow{
	SEMID              viewwdparaminput_sem;     /**< 设置参数(应用层参数输入到param_input)的信号量     */
	SEMID              viewwdparamset_sem;       /**< 设置参数(从param_input到引擎内部使用的param)的信号量     */
	sViewWindowParam   viewwd_param_input;       /**< 外部接收到的视窗控制参数     */
	sViewWindowParam   viewwd_param;             /**< 引擎内部使用的视窗控制参数     */
	}sViewWindow;

/** @struct sPAINTUNIT
*  @brief 绘制单元结构.
*  @note  绘制单元由一个绘制视窗和一个渲染场景两部分组成，\n
          在绘制前首先要创建绘制单元，\n
          然后要创建视窗和场景，可以创建多个视窗和多个场景，\n
          最后后要将视窗和场景关联，一个视窗只能关联一个场景，但可以变更管理关系.
*/	
typedef struct tagPainUnit{
	sViewWindow *pViewWindow;
	sGLRENDERSCENE *pScene;
	}sPAINTUNIT;
	

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
