/**
 * @file macrodefine.h
 * @brief 该文件提供了一些宏定义及枚举型变量
 * @author 615地图团队 张仟新
 * @date 2016-05-08
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _Hlmacrodefine_h_ 
#define _Hlmacrodefine_h_ 

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef FLT_EPSILON
#define FLT_EPSILON 1.192092896e-017F
#endif

#define PRECISION		0.0000001    /* 浮点比较精度 */
#define RA2DE 57.295779513082320876798154814105
#define DE2RA 0.017453292519943295769236907684883
#define EARTH_RADIUS	6378137.0f                  /* 地球半径，单位:米 */
#define MAPLOGSIZE      20037508.342789             /* 墨卡托投影后矩形高度的一半，单位（米） */

#define TILESIZE           256     /* 瓦片尺寸（像素点数） */
#define VIEW_ANGLE 60.0

#define TER_WARNING_MAX_LAYERS 	128// 16     /* 地形告警颜色设置的最大级数 */
#define TER_WARNING_USE_TEXTURE_1D     /* 地形告警采用1维纹理的标志 */
#undef TER_WARNING_USE_TEXTURE_1D

#define sqr(a)  (a)*(a)
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b)) 
#define DEGREE_TO_RADIAN(x)		((x) * 0.017453292519943295769236907684883)
#define RADIAN_TO_DEGREE(x)		((x) * 57.295779513082320876798154814105)


#define QTNSQLNUM 		5		//QTN 格式的sql 数据库类型个数

#define MAPHANDLEKEY  0x121833EF
#define ISMAPHANDLEVALID(_d) ( ((_d) != NULL) && ( ((sMAPHANDLE *)(_d))->key == MAPHANDLEKEY ) )
#define MAPDATANODEKEY	(0x07314227)
#define ISMAPDATANODE(_node) ( (_node != 0) && (_node->nodekey == MAPDATANODEKEY) )


/** @enum eMatrixMode
* @brief 矩阵类型
*/
typedef enum
{
    eMatrix_Mdview,			/**< 模视矩阵 */
    eMatrix_Mdprj		    /**< 投影矩阵 */
}eMatrixMode;

/** @enum eRotateType
* @brief 场景视角模式
*/
typedef enum
{
    eROTATE_CW_0,			/**< 正常模式,不旋转 */
    eROTATE_CW_90,		    /**< 顺时针旋转90度  */
    eROTATE_CW_180,		    /**< 顺时针旋转180度 */
    eROTATE_CW_270          /**< 顺时针旋转270度 */
}eRotateType;

/** @enum eColorMode
* @brief 颜色模式
*/
typedef enum
{
	eCOR_normal,	/**< 正常色 */
	eCOR_green, 	/**< 绿色   */
	eCOR_gray   	/**< 灰色   */
}eColorMode;

/** @enum eDNmode
* @brief 昼夜模式
*/
typedef enum
{
	eDN_daytime,	/**< 昼 */
	eDN_nighttime	/**< 夜 */
}eDNmode;

/** @enum eMoveMode
* @brief 地图运动模式
*/
typedef enum
{
	eMV_headup,	/**< 航向朝上 */
	eMV_northupmapmove,	/**< 北朝上，地图动 */
	eMV_northupmapstatic  /**< 北朝上，地图静止 */
}eMoveMode;

/** @enum eFreezeMode
* @brief 地图冻结模式
*/
typedef enum
{
	eFRZ_outfreeze,	/**< 解冻模式 */
	eFRZ_infreeze	/**< 冻结模式 */
}eFreezeMode;

/** @enum eRoamMode
* @brief 地图漫游模式
*/
typedef enum
{
	eROM_outroam,	/**< 非漫游模式 */
	eROM_inroam  	/**< 漫游模式 */
}eRoamMode;

/** @enum eSceneMode
* @brief 地图场景显示模式
*/
typedef enum
{
	eScene_normal,	/**< 正常模式 */
	eScene_eyebird 	/**< 鹰眼模式 */
}eSceneMode;

/** @enum eIsRoamed
* @brief 地图漫游标志
*/
typedef enum
{
	eROM_notroamed,	/**< 还没漫游 */
	eROM_roamed  	/**< 已漫游 */
}eIsRoamed;

/** @enum eIsRoamHomed
* @brief 地图漫游归位标志
*/
typedef enum
{
	eROM_notroamhomed,	/**< 还没漫游归位 */
	eROM_roamhomed  	/**< 已漫游归位 */
}eIsRoamHomed;

/** @enum eTerwarnMode
* @brief 地形告警模式
*/
typedef enum
{
	eTERWARN_outwarn,	/**< 非地形告警模式 */
	eTERWARN_inwarn  	/**< 地形告警模式 */
}eTerwarnMode;

/** @enum eViewMode
* @brief 场景视角模式
*/
typedef enum
{
	eVM_DEFAULT_VIEW,           /**< 默认视角(自由视角) */
    eVM_FIXED_VIEW,			    /**< 定点观测视角  */
    eVM_COCKPIT_VIEW,		    /**< 座舱视角      */
    eVM_FOLLOW_VIEW,		    /**< 尾随视角      */
    eVM_OVERLOOK_VIEW,	        /**< 俯视视角      */
    eVM_SCALE_VIEW              /**< 根据显示分辨率对地图进行缩放的视角,即30/45/75俯仰角视角*/
}eViewMode;

/** @enum eProjectMode
* @brief 投影模式
*/
typedef enum
{
	ePRJ_mercator,	    /**< 墨卡托 */
	ePRJ_lambert,      	   /**< 兰勃特 */
	ePRJ_ball		   /**球面 */
}eProjectMode;

/** @enum eMap23dMode
* @brief 二三维模式
*/
typedef enum
{
	eMAPMode_2D,	    /**< 二维模式 */
	eMAPMode_3D      	/**< 三维模式 */
}eMap23dMode;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
