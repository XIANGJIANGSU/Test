#include "../define/mbaseType.h"
#include "osAdapter.h"
#include "mapLoad.h"
#include "mapNodeGen.h"
#include "mapNodeDel.h"
#include "tqtfile.h"
#include <math.h>
#include "../../../SE_api_prj/src/SqliteOne.h"
#include "../projection/coord.h"
#include "../mapApi/mapApi.h"

#ifdef USE_SQLITE


/*
	判断是否需要重新调度qtn 数据
	参数：pHandle -- 全局唯一的树句柄
	      pScene  -- 绘制场景句柄
    返回值：
	      无
*/
void QtnSqlJudge(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene)
{
	f_float64_t geo_width, geo_height;	//屏幕范围内的地理范围
	f_float64_t lon_widht, lat_height;	//屏幕范围内的经纬度范围
	f_float64_t min_screen_geo_width;	//长或宽上的屏幕最小距离
	f_float64_t lonlat_max = 0.0;		//屏幕范围内的经纬度最大覆盖范围
//	f_float32_t lonlat_range[4];//lon_low, lon_hi, lat_low, lat_hi;


	//屏幕范围变化1/4 则重新调度数据
	geo_width = pScene->roam_res_for_QTN * pScene->innerviewport[2];
	geo_height = pScene->roam_res_for_QTN  * pScene->innerviewport[3];

	//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//赤道上一度经度的距离
	lon_widht = geo_width / 111319.5 ;
	lat_height = geo_height / 111319.5 ;		

	//选取最小的经纬度跨度
	if(lon_widht > lat_height)
	{
		min_screen_geo_width = lat_height;
	}
	else
	{
		min_screen_geo_width = lon_widht;
	}

	/*距离改变量为何选这个值？？*/
	min_screen_geo_width = sqr(min_screen_geo_width) / 16.0;

	/*分三种情况，距离改变min_screen_geo_width度，高度改变1000，方向角改变15度*/
	if((sqr(pScene->pre_cameraLonLatHei[0] - pScene->camParam.m_geoptEye.lon)
		+sqr(pScene->pre_cameraLonLatHei[1] - pScene->camParam.m_geoptEye.lat) ) > min_screen_geo_width)
	{
		/*status为1则告知三维数据加载线程需要重新从数据库中查询地名注记数据*/
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_status = 1;
		pScene->pre_cameraLonLatHei[0] = pScene->camParam.m_geoptEye.lon;
		pScene->pre_cameraLonLatHei[1] = pScene->camParam.m_geoptEye.lat;
	}

	if(
		((pScene->pre_cameraLonLatHei[2] - pScene->camParam.m_geoptEye.height) > 1000.0)
		||((pScene->pre_cameraLonLatHei[2] - pScene->camParam.m_geoptEye.height) < -1000.0)
		)
	{
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_status = 1;
		pScene->pre_cameraLonLatHei[2] = pScene->camParam.m_geoptEye.height;
	}				

	if(	
		((pScene->pre_cameraYPR[0] - pScene->attitude.yaw) > 15.0)
		||((pScene->pre_cameraYPR[0] - pScene->attitude.yaw) < -15.0)
		)
	{
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_status = 1;
		pScene->pre_cameraYPR[0] = pScene->attitude.yaw;

	}

	// 1.根据视点位置和量程大小判断显示的范围
	if(pHandle->qtn_node[pScene->qtn_nodeID].qtn_status == 1)
	{
		geo_width = pScene->roam_res_for_QTN * pScene->innerviewport[2];
		geo_height = pScene->roam_res_for_QTN  * pScene->innerviewport[3];

		//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//赤道上一度经度的距离
		lon_widht = geo_width / 111319.5 ;
		lat_height = geo_height / 111319.5 ;

		//计算显示范围,粗略的计算方法,飞机位置在中间,视口的四个角点即为经纬度范围
		//但实际并不是俯视,是在三维视角下绘制地名,该方法有待优化
		lonlat_max = sqrt(sqr( lon_widht /2.0) + sqr(lat_height /2.0));
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[0] = pScene->camParam.m_geoptEye.lon - lonlat_max;
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[1] = pScene->camParam.m_geoptEye.lon + lonlat_max;
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[2] = pScene->camParam.m_geoptEye.lat - lonlat_max;
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[3] = pScene->camParam.m_geoptEye.lat + lonlat_max;
	}
	
}


/*
	从sqlite数据库中重新查找并更新地名数据
	参数：pHandle -- 全局唯一的树句柄
    返回值：
	      无
*/
void QtnSqlLoad(sMAPHANDLE *pHandle)
{
	f_int32_t i = 0, j = 0;
	f_float64_t lonlat_max = 0.0;		//屏幕范围内的经纬度最大覆盖范围
	f_int32_t qtn_i = 0;

	takeSem(pHandle->qtnthread, FOREVER_WAIT);
	
	//调度每个视口的qtn 数据
	for(qtn_i = 0; qtn_i < SCENE_NUM; qtn_i ++)
	{
		// 0.判断是否需要调度qtn 数据
		if(pHandle->qtn_node[qtn_i].qtn_status == 1)
		{
			printf("读取sql 数据库\n");
#if 0
			// 1.根据视点位置和量程大小判断显示的范围

			geo_width = pHandle->roam_res_for_QTN* pHandle->h_viewport[2];
			geo_height = pHandle->roam_res_for_QTN * pHandle->h_viewport[3];

			//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//赤道上一度经度的距离
			lon_widht = geo_width / 111319.5 ;
			lat_height = geo_height / 111319.5 ;

			//计算显示范围
			lonlat_max = sqrt(sqr( lon_widht /2.0) + sqr(lat_height /2.0));
			lonlat_range[0] = pHandle->cam_geo_pos.lon - lonlat_max;
			lonlat_range[1] = pHandle->cam_geo_pos.lon + lonlat_max;
			lonlat_range[2] = pHandle->cam_geo_pos.lat - lonlat_max;
			lonlat_range[3] = pHandle->cam_geo_pos.lat + lonlat_max;
#endif
			// 2.读取数据库,最多支持5个数据库,每个数据库最多读取1024个地名数据,目前只从机场和地名数据库中读取地名数据		
			GetSqlAirport(pHandle->qtn_node[qtn_i].qtn_lonlat_range, &pHandle->qtn_node[qtn_i].qtn_sql[1 * MAXQTNSQLNUM], &(pHandle->qtn_node[qtn_i].qtn_num[1]));		
			GetSqlPlacename(pHandle->qtn_node[qtn_i].qtn_lonlat_range, &pHandle->qtn_node[qtn_i].qtn_sql[0 * MAXQTNSQLNUM], &(pHandle->qtn_node[qtn_i].qtn_num[0]));	

			// 3.恢复状态
			pHandle->qtn_node[qtn_i].qtn_status = 0;

			// 4.处理qtn结构体,目前只处理机场和地名2个数据库
			for(j=0; j<2; j++)
			{
				for(i = 0; i<pHandle->qtn_node[qtn_i].qtn_num[j]; i++)
				{
					Geo_Pt_D geoPt;
					Obj_Pt_D objPt;
				
					/*数据库中地名高度值不对,用瓦片中查询到的高度+数据库中的高度,后续数据库中高度得重出*/
					geoPt.lon = pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].plon;
					geoPt.lat = pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].plat;
					/* 600.0该值在实际应用中不需要，目前是因为小钟在出地名注记时没有提取地形高度，导致地名显示在地下。后续让小钟提取高度 */
	//				geoPt.height = pHandle->qtn_sql[j * MAXQTNSQLNUM + i].pHei + 600.0;
					{
						double terrainheight = 0.0;
						GetZex(pHandle, geoPt.lon, geoPt.lat, &terrainheight);
						geoPt.height = pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].pHei 	+ terrainheight;
					}
					/*地理坐标转物体坐标,并进行保存*/
					geoDPt2objDPt(&geoPt, &objPt);
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].px = objPt.x;
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].py = objPt.y;
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].pz = objPt.z;
				}

				//已处理完某一个数据库中所有的地名注记,不满1024条,则设置下一条的地名注记长度为0,确保该条数据被设置为无效
				if(i < MAXQTNSQLNUM)
				{
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].textlen = 0;
				}
			}

		}
	}
	
	giveSem(pHandle->qtnthread);	
}
#endif
