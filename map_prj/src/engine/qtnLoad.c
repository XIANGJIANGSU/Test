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
	�ж��Ƿ���Ҫ���µ���qtn ����
	������pHandle -- ȫ��Ψһ�������
	      pScene  -- ���Ƴ������
    ����ֵ��
	      ��
*/
void QtnSqlJudge(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene)
{
	f_float64_t geo_width, geo_height;	//��Ļ��Χ�ڵĵ���Χ
	f_float64_t lon_widht, lat_height;	//��Ļ��Χ�ڵľ�γ�ȷ�Χ
	f_float64_t min_screen_geo_width;	//������ϵ���Ļ��С����
	f_float64_t lonlat_max = 0.0;		//��Ļ��Χ�ڵľ�γ����󸲸Ƿ�Χ
//	f_float32_t lonlat_range[4];//lon_low, lon_hi, lat_low, lat_hi;


	//��Ļ��Χ�仯1/4 �����µ�������
	geo_width = pScene->roam_res_for_QTN * pScene->innerviewport[2];
	geo_height = pScene->roam_res_for_QTN  * pScene->innerviewport[3];

	//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//�����һ�Ⱦ��ȵľ���
	lon_widht = geo_width / 111319.5 ;
	lat_height = geo_height / 111319.5 ;		

	//ѡȡ��С�ľ�γ�ȿ��
	if(lon_widht > lat_height)
	{
		min_screen_geo_width = lat_height;
	}
	else
	{
		min_screen_geo_width = lon_widht;
	}

	/*����ı���Ϊ��ѡ���ֵ����*/
	min_screen_geo_width = sqr(min_screen_geo_width) / 16.0;

	/*���������������ı�min_screen_geo_width�ȣ��߶ȸı�1000������Ǹı�15��*/
	if((sqr(pScene->pre_cameraLonLatHei[0] - pScene->camParam.m_geoptEye.lon)
		+sqr(pScene->pre_cameraLonLatHei[1] - pScene->camParam.m_geoptEye.lat) ) > min_screen_geo_width)
	{
		/*statusΪ1���֪��ά���ݼ����߳���Ҫ���´����ݿ��в�ѯ����ע������*/
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

	// 1.�����ӵ�λ�ú����̴�С�ж���ʾ�ķ�Χ
	if(pHandle->qtn_node[pScene->qtn_nodeID].qtn_status == 1)
	{
		geo_width = pScene->roam_res_for_QTN * pScene->innerviewport[2];
		geo_height = pScene->roam_res_for_QTN  * pScene->innerviewport[3];

		//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//�����һ�Ⱦ��ȵľ���
		lon_widht = geo_width / 111319.5 ;
		lat_height = geo_height / 111319.5 ;

		//������ʾ��Χ,���Եļ��㷽��,�ɻ�λ�����м�,�ӿڵ��ĸ��ǵ㼴Ϊ��γ�ȷ�Χ
		//��ʵ�ʲ����Ǹ���,������ά�ӽ��»��Ƶ���,�÷����д��Ż�
		lonlat_max = sqrt(sqr( lon_widht /2.0) + sqr(lat_height /2.0));
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[0] = pScene->camParam.m_geoptEye.lon - lonlat_max;
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[1] = pScene->camParam.m_geoptEye.lon + lonlat_max;
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[2] = pScene->camParam.m_geoptEye.lat - lonlat_max;
		pHandle->qtn_node[pScene->qtn_nodeID].qtn_lonlat_range[3] = pScene->camParam.m_geoptEye.lat + lonlat_max;
	}
	
}


/*
	��sqlite���ݿ������²��Ҳ����µ�������
	������pHandle -- ȫ��Ψһ�������
    ����ֵ��
	      ��
*/
void QtnSqlLoad(sMAPHANDLE *pHandle)
{
	f_int32_t i = 0, j = 0;
	f_float64_t lonlat_max = 0.0;		//��Ļ��Χ�ڵľ�γ����󸲸Ƿ�Χ
	f_int32_t qtn_i = 0;

	takeSem(pHandle->qtnthread, FOREVER_WAIT);
	
	//����ÿ���ӿڵ�qtn ����
	for(qtn_i = 0; qtn_i < SCENE_NUM; qtn_i ++)
	{
		// 0.�ж��Ƿ���Ҫ����qtn ����
		if(pHandle->qtn_node[qtn_i].qtn_status == 1)
		{
			printf("��ȡsql ���ݿ�\n");
#if 0
			// 1.�����ӵ�λ�ú����̴�С�ж���ʾ�ķ�Χ

			geo_width = pHandle->roam_res_for_QTN* pHandle->h_viewport[2];
			geo_height = pHandle->roam_res_for_QTN * pHandle->h_viewport[3];

			//111319.5 = caculateGreatCircleDis(0.0, 0.0, 1.0, 0.0);	//�����һ�Ⱦ��ȵľ���
			lon_widht = geo_width / 111319.5 ;
			lat_height = geo_height / 111319.5 ;

			//������ʾ��Χ
			lonlat_max = sqrt(sqr( lon_widht /2.0) + sqr(lat_height /2.0));
			lonlat_range[0] = pHandle->cam_geo_pos.lon - lonlat_max;
			lonlat_range[1] = pHandle->cam_geo_pos.lon + lonlat_max;
			lonlat_range[2] = pHandle->cam_geo_pos.lat - lonlat_max;
			lonlat_range[3] = pHandle->cam_geo_pos.lat + lonlat_max;
#endif
			// 2.��ȡ���ݿ�,���֧��5�����ݿ�,ÿ�����ݿ�����ȡ1024����������,Ŀǰֻ�ӻ����͵������ݿ��ж�ȡ��������		
			GetSqlAirport(pHandle->qtn_node[qtn_i].qtn_lonlat_range, &pHandle->qtn_node[qtn_i].qtn_sql[1 * MAXQTNSQLNUM], &(pHandle->qtn_node[qtn_i].qtn_num[1]));		
			GetSqlPlacename(pHandle->qtn_node[qtn_i].qtn_lonlat_range, &pHandle->qtn_node[qtn_i].qtn_sql[0 * MAXQTNSQLNUM], &(pHandle->qtn_node[qtn_i].qtn_num[0]));	

			// 3.�ָ�״̬
			pHandle->qtn_node[qtn_i].qtn_status = 0;

			// 4.����qtn�ṹ��,Ŀǰֻ��������͵���2�����ݿ�
			for(j=0; j<2; j++)
			{
				for(i = 0; i<pHandle->qtn_node[qtn_i].qtn_num[j]; i++)
				{
					Geo_Pt_D geoPt;
					Obj_Pt_D objPt;
				
					/*���ݿ��е����߶�ֵ����,����Ƭ�в�ѯ���ĸ߶�+���ݿ��еĸ߶�,�������ݿ��и߶ȵ��س�*/
					geoPt.lon = pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].plon;
					geoPt.lat = pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].plat;
					/* 600.0��ֵ��ʵ��Ӧ���в���Ҫ��Ŀǰ����ΪС���ڳ�����ע��ʱû����ȡ���θ߶ȣ����µ�����ʾ�ڵ��¡�������С����ȡ�߶� */
	//				geoPt.height = pHandle->qtn_sql[j * MAXQTNSQLNUM + i].pHei + 600.0;
					{
						double terrainheight = 0.0;
						GetZex(pHandle, geoPt.lon, geoPt.lat, &terrainheight);
						geoPt.height = pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].pHei 	+ terrainheight;
					}
					/*��������ת��������,�����б���*/
					geoDPt2objDPt(&geoPt, &objPt);
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].px = objPt.x;
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].py = objPt.y;
					pHandle->qtn_node[qtn_i].qtn_sql[j * MAXQTNSQLNUM + i].pz = objPt.z;
				}

				//�Ѵ�����ĳһ�����ݿ������еĵ���ע��,����1024��,��������һ���ĵ���ע�ǳ���Ϊ0,ȷ���������ݱ�����Ϊ��Ч
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
