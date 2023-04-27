#ifdef WIN32
#include <memory.h>
#endif
#include "bil.h"
#include "../engine/bildata.h"

#define USE_BLOCK_BIL_DATA_LEVEL 3

/*���в㼶�߳����ݹ����ܽṹ��ָ��*/
BILDATA bil_data_handle;

/**
 * @fn BOOL createBilHandle(f_char_t *data_path)
 *  @brief ����BIL���.
 *  @param[in] data_path BIL����·���������ļ�����.
 *  @exception void
 *  @return BOOL,BIL��������ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
 * @see 
 * @note 
*/
BOOL createBilHandle(f_char_t *data_path)
{
	//f_char_t buf[256] = {0};
	/*��������ʼ�����в㼶�߳����ݹ����ܽṹ��*/
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

	/*����ָ���㼶�ĸ߳�����Ŀ¼���߳�ͼ�����ȿ�ȡ��߳�ͼ��γ�ȿ��*/
	/*ʹ�õ�����,�����㾭�Ⱥ�γ�ȿ�Ⱦ�Ϊ0.25��,ÿ���߳�ͼ������256*256����,�ʵ�����ÿ���̵߳�֮��ļ��Ϊ0.0009765625��*/
	/*��ԭʼSRTM-3���ݼ��3���,3���Ӧ�Ķ�����0.000833333333��*/
	if( bildataSetDataPath(bil_data_handle, USE_BLOCK_BIL_DATA_LEVEL, data_path, 3600 * 128 / 4, 3600 * 128 / 4) < 0 )
	{
		DEBUG_PRINT("BILDATA set data path failed\n");
		return FALSE;
	}

	return TRUE;

}

/*
���ܣ����ݾ�γ�Ȼ�ȡ�߳�ֵ,�����ϵ���߷���������壬8848.86��������ߵ㣻������͵��������ɺ���������11034��
      �������õ����ֵΪ10000,���ֵΪ-9999
���룺
    lon ����
	lat γ��
�����
    pzvalue ��ѯ�õ��ĸ߶�ֵ�������ָ��
����ֵ��
    TRUE  ��ѯ�ɹ�,*pAltΪ��ѯ�õ��ĸ߳�ֵ(
	      >-9999: �����߳�
		   -9999: ԭʼ�߳����ݲ�����)
	FALSE ��ѯʧ��,*pAltΪһ���쳣ֵ(
	      -10001: ԭʼ�߳����ݳ�����Χ
		  -10002: ���ݾ����Ч
		  -10003: ������Ч
		  -10004: ����ռ����)
*/
BOOL getAltByGeoPos(const f_float64_t lon, const f_float64_t lat, f_int16_t *pAlt)
{
	f_int16_t view_z = 0;
	/* ���Ⱥ�γ���Ƿ�����Ч��Χ��,����ֱ�ӷ��ز�ѯʧ�� */
	if( (lon < -180.0f) || (lon > 360.0f) || (lat < -90.f) || (lat > 90.f) )
	{
		*pAlt = -10003;
		return FALSE;
	}
	/* �̴߳������ָ��Ϊ��,���߸߳����ݹ����ܽṹ��ָ��Ϊ��,�޷����и̲߳�ѯ,ֱ�ӷ��ز�ѯʧ�� */
	if( (pAlt == NULL) || (bil_data_handle == NULL) )
	{
		*pAlt = -10003;
		return FALSE;
	}

	/* ����ָ����γ�ȵĸ߳� */
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
���ܣ����ݾ�γ�ȡ�����뾶����ȡ�����η�Χ(Ҳ���Կ���Բ)�����߳�ֵ
���룺
    lon ����
	lat γ��
	radius �����η�Χ�ڽ�Բ�뾶
�����
    ���ظ߳�ֵ
*/
#define ELEVATION_DATA_RESOLUTION 90/*�߳����ݾ���*/
f_int16_t getMaxAltInRect(const f_float64_t lon, const f_float64_t lat, f_float64_t radius)
{
    /* Ŀǰʹ�õĸ߳����ݾ���ԼΪ90m��Ҳ���Գ�Ϊ3���룬����Ļ�����ǽ��룬1��=60��=3600��
     * �����1���ȶ�Ӧ�ľ���ԼΪ111km��ʵ���ϸ߳����ݵ����֮��ļ��ԼΪ0.000833333��
     * ���������㷨�Ǵ���ģ�1����=57.3�ȣ�1����=3600���룬1����=0.01592��*/
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
            //��ȡʧ�ܣ��߶�����Ϊ�쳣ֵ-9999
            if(FALSE == ret)
            {
                valueZ = -9999;
            }
            //�õ���Χ�����߶�ֵ
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
���ܣ�����̻߳�ȡ���ܻ���
���룺
    ��
�����
    ��
����ֵ��
	TRUE ����ɹ�
	FALSE ����ʧ��
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

/* ��ʾ��ǰ�ڴ����������ͼ���߳��������Ѵ򿪵�ͼ�������ļ����� */
void printBilFileInMem()
{
	bildataPrintCacheCount(bil_data_handle, USE_BLOCK_BIL_DATA_LEVEL);
}
