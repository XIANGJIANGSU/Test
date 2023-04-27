#ifdef WIN32
#include <memory.h>
#endif
#include "bil.h"
#include "../engine/bildata.h"

#define USE_BLOCK_BIL_DATA_LEVEL 3

/*所有层级高程数据管理总结构体指针*/
BILDATA bil_data_handle;

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
BOOL createBilHandle(f_char_t *data_path)
{
	//f_char_t buf[256] = {0};
	/*创建并初始化所有层级高程数据管理总结构体*/
	if (bildataCreate(&bil_data_handle) < 0)
	{
		DEBUG_PRINT("BILDATA create failed\n");
		return FALSE;
	}
#if 0
	sprintf(buf, "%s\\%s", data_path, "BIL\\0\\");
	bildataSetDataPath(bil_data_handle, 0, buf, 3600 * 128 * 2, 3600 * 128 * 2);

	memset(buf, 0, 256);
	sprintf(buf, "%s\\%s", data_path, "BIL\\1\\");
	bildataSetDataPath(bil_data_handle, 1, buf, 3600 * 128 * 1, 3600 * 128 * 1);

	memset(buf, 0, 256);
	sprintf(buf, "%s\\%s", data_path, "BIL\\2\\");
	bildataSetDataPath(bil_data_handle, 2, buf, 3600 * 128 / 2, 3600 * 128 / 2);

	memset(buf, 0, 256);
	sprintf(buf, "%s\\%s", data_path, "BIL\\3\\");
	bildataSetDataPath(bil_data_handle, 3, buf, 3600 * 128 / 4, 3600 * 128 / 4);
#endif

	/*更新指定层级的高程数据目录、高程图幅经度跨度、高程图幅纬度跨度*/
	/*使用第三层,第三层经度和纬度跨度均为0.25°,每个高程图幅包含256*256个点,故第三层每个高程点之间的间隔为0.0009765625°*/
	/*比原始SRTM-3数据间隔3秒大,3秒对应的度数是0.000833333333°*/
	if( bildataSetDataPath(bil_data_handle, USE_BLOCK_BIL_DATA_LEVEL, data_path, 3600 * 128 / 4, 3600 * 128 / 4) < 0 )
	{
		DEBUG_PRINT("BILDATA set data path failed\n");
		return FALSE;
	}

	return TRUE;

}

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
BOOL getAltByGeoPos(const f_float64_t lon, const f_float64_t lat, f_int16_t *pAlt)
{
	f_int16_t view_z = 0;
	/* 经度和纬度是否在有效范围内,不在直接返回查询失败 */
	if( (lon < -180.0f) || (lon > 360.0f) || (lat < -90.f) || (lat > 90.f) )
	{
		*pAlt = -10003;
		return FALSE;
	}
	/* 高程存放区域指针为空,或者高程数据管理总结构体指针为空,无法进行高程查询,直接返回查询失败 */
	if( (pAlt == NULL) || (bil_data_handle == NULL) )
	{
		*pAlt = -10003;
		return FALSE;
	}

	/* 查找指定经纬度的高程 */
	view_z = bildataGetHeight(bil_data_handle, USE_BLOCK_BIL_DATA_LEVEL, (int)(lon * 3600 * 128), (int)(lat * 3600 * 128));
    *pAlt = view_z;

   	if( view_z >= INVALID_DEM_VALUE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*
功能：根据经纬度、计算半径，获取正方形范围(也可以看做圆)内最大高程值
输入：
    lon 经度
	lat 纬度
	radius 正方形范围内接圆半径
输出：
    返回高程值
*/
#define ELEVATION_DATA_RESOLUTION 90/*高程数据精度*/
f_int16_t getMaxAltInRect(const f_float64_t lon, const f_float64_t lat, f_float64_t radius)
{
    /* 目前使用的高程数据精度约为90m，也可以称为3弧秒，这里的弧秒就是角秒，1度=60分=3600秒
     * 赤道上1经度对应的距离约为111km，实际上高程数据点与点之间的间隔约为0.000833333度
     * 以下这种算法是错误的：1弧度=57.3度，1弧度=3600弧秒，1弧秒=0.01592度*/
    f_int16_t valueZ = -9999;
    f_int16_t maxValueZ = -9999;
    f_int32_t ret = 0;
    f_int32_t range = (int)(radius / ELEVATION_DATA_RESOLUTION);
    f_float64_t tempLon,tempLat;
    f_int32_t i, j;

    for(i=-range; i<range+1; i++)
    {
        tempLon = lon + (i * 0.000833333);

        for(j=-range; j<range+1; j++)
        {
            tempLat = lat + (j * 0.000833333);

            ret = getAltByGeoPos(lon, lat, &valueZ);
            //获取失败，高度设置为异常值-9999
            if(FALSE == ret)
            {
                valueZ = -9999;
            }
            //得到范围内最大高度值
            if(maxValueZ < valueZ)
            {
                maxValueZ = valueZ;
            }
        }
    }

    return maxValueZ;
}
#undef ELEVATION_DATA_RESOLUTION

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
BOOL deleteBilHandle()
{
	if( bildataDestroy(bil_data_handle)<0 )
	{
		DEBUG_PRINT("BILDATA destroy failed\n");
		return FALSE;
	}
	
	return TRUE;
}

/* 显示当前内存中已载入的图幅高程数量及已打开的图幅数据文件数量 */
void printBilFileInMem()
{
	bildataPrintCacheCount(bil_data_handle, USE_BLOCK_BIL_DATA_LEVEL);
}
