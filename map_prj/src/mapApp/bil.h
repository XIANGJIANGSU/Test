/**
 * @file bil.h
 * @brief 该文件提供获取地形高程的外部接口函数
 * @author 615地图团队
 * @date 2016-12-30
 * @version v1.0.0
 * @copyright 615地图团队\n
 * 更改历史：    日期        更改人      更改描述  \n
 * 
*/

#ifndef _BIL_h_ 
#define _BIL_h_ 

#include "../define/mbaseType.h"
#include "../mapApi/common.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
 
/**
 * @fn BOOL createBilHandle(f_char_t *data_path)
 *  @brief 字体BIL句柄.
 *  @param[in] data_path BIL数据路径（不含文件名）.
 *  @exception void
 *  @return BOOL,BIL句柄创建成功标志.
 *  @retval TRUE 成功.
 *  @retval FALSE 失败.
 * @see 
 * @note 
*/
BOOL createBilHandle(f_char_t *data_path);

/*
功能：根据经纬度获取高程值,世界上的最高峰珠穆朗玛峰，8848.86米世界最高点；世界最低点马里亚纳海沟，负的11034米
      程序中用的最高值为10000,最低值为-9999
输入：
    lon 经度
	lat 纬度
输出：
    pzvalue 查询得到的高度值存放区域指针
返回值：
    TRUE  查询成功,*pAlt为查询得到的高程值(
	      >-9999: 正常高程
		   -9999: 原始高程数据不存在)
	FALSE 查询失败,*pAlt为一个异常值(
	      -10001: 原始高程数据超出范围
		  -10002: 数据句柄无效
		  -10003: 参数无效
		  -10004: 缓存空间出错)
*/
BOOL getAltByGeoPos(const f_float64_t lon, const f_float64_t lat, f_int16_t *pAlt);


/*
功能：根据经纬度、计算半径，获取正方形范围(也可以看做圆)内最大高程值
输入：
    lon 经度
	lat 纬度
	radius 正方形范围内接圆半径
输出：
    返回高程值
*/
f_int16_t getMaxAltInRect(const f_float64_t lon, const f_float64_t lat, f_float64_t radius);

/*
功能：清理高程获取功能环境
输入：
    无
输出：
    无
返回值：
	TRUE 清理成功
	FALSE 清理失败
*/
BOOL deleteBilHandle();

/* 显示当前内存中已载入的图幅高程数量及已打开的图幅数据文件数量 */
void printBilFileInMem();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 