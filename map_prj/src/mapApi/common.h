/**
 * @file common.h
 * @brief 该文件提供地图引擎与上层应用程序都要用到的一些资源
 * @author 615地图团队
 * @date 2016-05-05
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _COMMON_h_ 
#define _COMMON_h_ 

#include <stdio.h>
#include <stdlib.h>
#include "../define/mbaseType.h"
#include "../../../include_ttf/ttf_api.h"
#include "../engine/TextureCache.h"


#define DEBUG_PRINT(info)  printf("%s, %s, %d\n", info, __FILE__, __LINE__); 
#define	SAMPLE_NUM	300//60								//剖面图的采样数量，窗口中绘制的分辨率是由多少个点组成

/** @enum eStatus
* @brief 状态
*/
typedef enum
{
	sFail,	    /**< 失败 */
	sSuccess	/**< 成功 */
}eStatus;

/** @struct sColor3f
*  @brief 3分量颜色
*
*/
typedef struct tagColor3f{
	f_float32_t red;     /**< 红色. */
	f_float32_t green;   /**< 绿色. */
	f_float32_t blue;    /**< 蓝色. */
	}sColor3f;

/** @enum sColor4f
* @brief 颜色值
*/
typedef struct tagColor4f
{
	f_float32_t red;     /**< 红色分量 */
	f_float32_t green;   /**< 绿色分量 */
	f_float32_t blue;    /**< 蓝色分量 */
	f_float32_t alpha;   /**< 透明分量 */
}sColor4f;

/** @enum sColor3uc
* @brief 颜色值
*/
typedef struct tagColor3uc
{
	f_uint8_t red;     /**< 红色分量 */
	f_uint8_t green;   /**< 绿色分量 */
	f_uint8_t blue;    /**< 蓝色分量 */
}sColor3uc;

/** @enum sColor4uc
* @brief 颜色值
*/
typedef struct tagColor4uc
{
	f_uint8_t red;     /**< 红色分量 */
	f_uint8_t green;   /**< 绿色分量 */
	f_uint8_t blue;    /**< 蓝色分量 */
	f_uint8_t alpha;   /**< 透明分量 */
}sColor4uc;


/** @enum sZplParam
* @brief 0俯仰角线参数
*/
typedef struct tagZplParam{
	BOOL     is_display;   /**< 是否显示开关，TRUE显示，FALSE不显示 */
	sColor3f color;        /**< 显示的颜色值 */
	}sZplParam;
	
/** @enum sCpsParam
* @brief 罗盘参数
*/
typedef struct tagCpsParam{
	BOOL        is_display;   /**< 是否显示开关，TRUE显示，FALSE不显示 */
	sColor3f    color;        /**< 显示的颜色值 */
	f_float32_t cent_x;       /**< 罗盘中心点横坐标，相对于视口左下角 */
	f_float32_t cent_y;       /**< 罗盘中心点纵坐标，相对于视口左下角 */
	f_float32_t width;        /**< 罗盘宽度 */
	f_float32_t height;       /**< 罗盘高度 */
	}sCpsParam;	


/** @enum sPrefileParam
* @brief剖面图参数
*/
typedef struct tagPreParam{
	BOOL        is_display;				/**< 是否显示开关，TRUE显示，FALSE不显示 */
	f_float32_t x;						/**< 左下角横坐标，相对于外部屏幕左下角 */
	f_float32_t y;						/**< 左下角纵坐标，相对于外部屏幕左下角 */
	f_float32_t width;					/**< 剖面图宽度 */
	f_float32_t height;					/**< 剖面图高度 */

	f_float64_t s_WorldDetectDistance;						//水平方向探测距离（米），探测飞机前方XX米的距离
	f_int32_t s_WinIntervalNum;								//水平方向刻度的间隔数（只是在窗口中绘制间隔线用）
	f_int32_t s_winHeiMeter;								//垂直方向半屏代表实际距离(米)
	f_int32_t color_num;                                    //色带的数量,最多6条色带
	f_float32_t prefile_height[5];                          //色带区间的高度,每条色带对应一个区间范围,5个值对应最多6个区间,
	f_uint8_t prefile_color[6][4];					        //色带的颜色
}sPrefileParam;	

/** @enum sPrefileParam
* @brief剖面图参数
*/
typedef struct tagPreParamUsed{
	//以下是内部计算使用的值
	f_float64_t s_WorldSampleInterval;						//采样间距(米)
	f_float64_t s_WinInterval ;                             //水平方向每个刻度的长度(像素)
	f_float64_t s_VWinPerMeter;								//垂直缩放比例
	f_float64_t s_HWinPerMeter;								//水平缩放比例
	f_float64_t s_vDetectHeights[SAMPLE_NUM + 1];			// 飞机高度点（在地理坐标中）
	int s_vDetectHeights_state[SAMPLE_NUM + 1];				// 飞机高度点的状态: 0:高度值存在，1 :不存在
	f_float32_t s_vDetectHeight_color[(SAMPLE_NUM + 1)*4];	//飞行高度点的颜色数组
	int s_TerrainPrefileLISTID[10] ;						//存储的显示列表
	int pIndex_tri_fan[SAMPLE_NUM + 3];						//三角扇的顶点索引数组
	int pIndex_quad[4];										//四边形的顶点索引
	f_float64_t s_vDetectHeights_cal[(SAMPLE_NUM + 3) * 2];	//三角形的顶点数组值 
	f_float64_t s_vDetectQuads[8][10];						//四边形的顶点数组值，最多允许10个长方体带
	int s_QuardLISTID[6];									//存储色带的显示列表ID
	f_uint32_t  SpantextureID[2];							//侧面量程图片
}sPrefileParamUsed;	



/** @enum sTtfFont
* @brief 字体
*/
typedef struct tagTtfFont
{
	TTFONT      font;   /**< 字体句柄 */
	f_int32_t   size;   /**< 字体尺寸（像素点） */
	f_int32_t   edge;   /**< 字体边界宽度（像素点） */
	f_float32_t color_font[4]; /**< 字体颜色 */
	f_float32_t color_edge[4]; /**< 边框颜色 */
}sTtfFont;

/** @struct sMCTPARAM
*  @brief 墨卡托投影参数
*
*/	
typedef struct tagMercatorParam{
	f_float64_t a;                       /**<  Semi-major axis of ellipsoid, in meters  */
    f_float64_t f;                       /**<  Flattening of ellipsoid  */
    f_float64_t lat_true_scale;          /**< Latitude in radians at which the point scale factor is 1.0  */
    f_float64_t lon_cent;                /**< Longitude in radians at the center of the projection  */
    f_float64_t fale_east;               /**< A coordinate value in meters assigned to the central meridian of the projection. */
    f_float64_t false_north;             /**< A coordinate value in meters assigned to the origin latitude of the projection.  */
	}sMCTPARAM;

/** @struct sLBTPARAM
*  @brief 兰勃特投影参数
*
*/		
typedef struct tagLambertParam{
	f_float64_t a;                      /**< Semi-major axis of ellipsoid, in meters     */
	f_float64_t b;                      /**< Semi-minor axis of ellipsoid, in meters     */
	f_float64_t lat_org;                /**< Latitude of origin in radians               */
	f_float64_t lon_org;                /**< Longitude of origin in radians              */
	f_float64_t lat_std_1;              /**< First standard parallel                     */
	f_float64_t lat_std_2;              /**< Second standard parallel                    */
	f_float64_t false_east;             /**< False easting in meters                     */
	f_float64_t false_north;            /**< False northing in meters                    */
	}sLBTPARAM;

/** @struct WDSINZE_2I
*  @brief 窗口尺寸
*
*/	
typedef struct tagWDSINZE_2I 
{
	f_int32_t width;   /**< 宽度(像素点) */
	f_int32_t height;  /**< 高度(像素点) */
} WDSINZE_2I, *LP_WDSINZE_2I;

/** @struct PT_2I
*  @brief 二维点(32位整型)
*
*/	
typedef struct tagPT_2I 
{
	f_int32_t x;  /**< x坐标 */
	f_int32_t y;  /**< y坐标 */
} PT_2I, *LP_PT_2I;

/** @struct PT_2F
*  @brief 二维点(32位浮点型)
*
*/	
typedef struct tagPT_2F 
{
	f_float32_t x;  /**< x坐标 */
	f_float32_t y;  /**< y坐标 */
} PT_2F, *LP_PT_2F;

/** @struct PT_2D
*  @brief 二维点(64位浮点型)
*
*/	
typedef struct tagPT_2D 
{
	f_float64_t x;  /**< x坐标 */
	f_float64_t y;  /**< y坐标 */
} PT_2D, *LP_PT_2D;

/** @struct PT_3I
*  @brief 三维点/向量(32位整型)
*
*/	
typedef struct tagPT_3I 
{
	f_int32_t x;  /**< x坐标 */
	f_int32_t y;  /**< y坐标 */
	f_int32_t z;  /**< z坐标 */
} PT_3I, *LP_PT_3I, VECOTR_3I, *LP_VECOTR_3I;

/** @struct PT_3F
*  @brief 三维点/向量(32位浮点型)
*
*/	
typedef struct tagPT_3F 
{
	f_float32_t x;  /**< x坐标 */
	f_float32_t y;  /**< y坐标 */
	f_float32_t z;  /**< z坐标 */
} PT_3F, *LP_PT_3F, VECOTR_3F, *LP_VECOTR_3F;

/** @struct PT_3D
*  @brief 三维点/向量/物体坐标(64位浮点型)
*
*/	
typedef struct tagPT_3D 
{
	f_float64_t x;  /**< x坐标 */
	f_float64_t y;  /**< y坐标 */
	f_float64_t z;  /**< y坐标 */
} PT_3D, *LP_PT_3D, VECOTR_3D, *LP_VECOTR_3D, Obj_Pt_D, *LP_Obj_Pt_D;

typedef struct _PT_3D_8
{
	f_uint8_t x;
	f_uint8_t y;
	f_uint8_t z;
}PT_3D_8, *LP_PT_3D_8;

/** @struct Geo_Pt_D
*  @brief 三维点(地理坐标，64位浮点型)
*
*/
typedef struct tagGeo_Pt_D
{
    f_float64_t lon;        /**< 经度 */
    f_float64_t lat;        /**< 纬度 */
    f_float64_t height;     /**< 高度（海拔高度） */
} Geo_Pt_D, *LP_Geo_Pt_D;

/** @struct PT_4D
*  @brief 四维点(64位浮点型)
*
*/
typedef struct tagPT_4D
{
	f_float64_t x;
	f_float64_t y;
	f_float64_t z;
	f_float64_t w;
} PT_4D, *LP_PT_4D;

/** @struct Matrix33
*  @brief 33矩阵(64位浮点型)
*
*/
typedef struct tagMatrix33{
	    f_float64_t m[9];
	} Matrix33, *LPMatrix33;

/** @struct Matrix44
*  @brief 4x4矩阵(64位浮点型)
*
*/	
typedef struct tagMatrix44
{
	f_float64_t m[16];
} Matrix44, *LPMatrix44;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 

