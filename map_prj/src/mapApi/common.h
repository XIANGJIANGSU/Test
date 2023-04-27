/**
 * @file common.h
 * @brief ���ļ��ṩ��ͼ�������ϲ�Ӧ�ó���Ҫ�õ���һЩ��Դ
 * @author 615��ͼ�Ŷ�
 * @date 2016-05-05
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
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
#define	SAMPLE_NUM	300//60								//����ͼ�Ĳ��������������л��Ƶķֱ������ɶ��ٸ������

/** @enum eStatus
* @brief ״̬
*/
typedef enum
{
	sFail,	    /**< ʧ�� */
	sSuccess	/**< �ɹ� */
}eStatus;

/** @struct sColor3f
*  @brief 3������ɫ
*
*/
typedef struct tagColor3f{
	f_float32_t red;     /**< ��ɫ. */
	f_float32_t green;   /**< ��ɫ. */
	f_float32_t blue;    /**< ��ɫ. */
	}sColor3f;

/** @enum sColor4f
* @brief ��ɫֵ
*/
typedef struct tagColor4f
{
	f_float32_t red;     /**< ��ɫ���� */
	f_float32_t green;   /**< ��ɫ���� */
	f_float32_t blue;    /**< ��ɫ���� */
	f_float32_t alpha;   /**< ͸������ */
}sColor4f;

/** @enum sColor3uc
* @brief ��ɫֵ
*/
typedef struct tagColor3uc
{
	f_uint8_t red;     /**< ��ɫ���� */
	f_uint8_t green;   /**< ��ɫ���� */
	f_uint8_t blue;    /**< ��ɫ���� */
}sColor3uc;

/** @enum sColor4uc
* @brief ��ɫֵ
*/
typedef struct tagColor4uc
{
	f_uint8_t red;     /**< ��ɫ���� */
	f_uint8_t green;   /**< ��ɫ���� */
	f_uint8_t blue;    /**< ��ɫ���� */
	f_uint8_t alpha;   /**< ͸������ */
}sColor4uc;


/** @enum sZplParam
* @brief 0�������߲���
*/
typedef struct tagZplParam{
	BOOL     is_display;   /**< �Ƿ���ʾ���أ�TRUE��ʾ��FALSE����ʾ */
	sColor3f color;        /**< ��ʾ����ɫֵ */
	}sZplParam;
	
/** @enum sCpsParam
* @brief ���̲���
*/
typedef struct tagCpsParam{
	BOOL        is_display;   /**< �Ƿ���ʾ���أ�TRUE��ʾ��FALSE����ʾ */
	sColor3f    color;        /**< ��ʾ����ɫֵ */
	f_float32_t cent_x;       /**< �������ĵ�����꣬������ӿ����½� */
	f_float32_t cent_y;       /**< �������ĵ������꣬������ӿ����½� */
	f_float32_t width;        /**< ���̿�� */
	f_float32_t height;       /**< ���̸߶� */
	}sCpsParam;	


/** @enum sPrefileParam
* @brief����ͼ����
*/
typedef struct tagPreParam{
	BOOL        is_display;				/**< �Ƿ���ʾ���أ�TRUE��ʾ��FALSE����ʾ */
	f_float32_t x;						/**< ���½Ǻ����꣬������ⲿ��Ļ���½� */
	f_float32_t y;						/**< ���½������꣬������ⲿ��Ļ���½� */
	f_float32_t width;					/**< ����ͼ��� */
	f_float32_t height;					/**< ����ͼ�߶� */

	f_float64_t s_WorldDetectDistance;						//ˮƽ����̽����루�ף���̽��ɻ�ǰ��XX�׵ľ���
	f_int32_t s_WinIntervalNum;								//ˮƽ����̶ȵļ������ֻ���ڴ����л��Ƽ�����ã�
	f_int32_t s_winHeiMeter;								//��ֱ�����������ʵ�ʾ���(��)
	f_int32_t color_num;                                    //ɫ��������,���6��ɫ��
	f_float32_t prefile_height[5];                          //ɫ������ĸ߶�,ÿ��ɫ����Ӧһ�����䷶Χ,5��ֵ��Ӧ���6������,
	f_uint8_t prefile_color[6][4];					        //ɫ������ɫ
}sPrefileParam;	

/** @enum sPrefileParam
* @brief����ͼ����
*/
typedef struct tagPreParamUsed{
	//�������ڲ�����ʹ�õ�ֵ
	f_float64_t s_WorldSampleInterval;						//�������(��)
	f_float64_t s_WinInterval ;                             //ˮƽ����ÿ���̶ȵĳ���(����)
	f_float64_t s_VWinPerMeter;								//��ֱ���ű���
	f_float64_t s_HWinPerMeter;								//ˮƽ���ű���
	f_float64_t s_vDetectHeights[SAMPLE_NUM + 1];			// �ɻ��߶ȵ㣨�ڵ��������У�
	int s_vDetectHeights_state[SAMPLE_NUM + 1];				// �ɻ��߶ȵ��״̬: 0:�߶�ֵ���ڣ�1 :������
	f_float32_t s_vDetectHeight_color[(SAMPLE_NUM + 1)*4];	//���и߶ȵ����ɫ����
	int s_TerrainPrefileLISTID[10] ;						//�洢����ʾ�б�
	int pIndex_tri_fan[SAMPLE_NUM + 3];						//�����ȵĶ�����������
	int pIndex_quad[4];										//�ı��εĶ�������
	f_float64_t s_vDetectHeights_cal[(SAMPLE_NUM + 3) * 2];	//�����εĶ�������ֵ 
	f_float64_t s_vDetectQuads[8][10];						//�ı��εĶ�������ֵ���������10���������
	int s_QuardLISTID[6];									//�洢ɫ������ʾ�б�ID
	f_uint32_t  SpantextureID[2];							//��������ͼƬ
}sPrefileParamUsed;	



/** @enum sTtfFont
* @brief ����
*/
typedef struct tagTtfFont
{
	TTFONT      font;   /**< ������ */
	f_int32_t   size;   /**< ����ߴ磨���ص㣩 */
	f_int32_t   edge;   /**< ����߽��ȣ����ص㣩 */
	f_float32_t color_font[4]; /**< ������ɫ */
	f_float32_t color_edge[4]; /**< �߿���ɫ */
}sTtfFont;

/** @struct sMCTPARAM
*  @brief ī����ͶӰ����
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
*  @brief ������ͶӰ����
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
*  @brief ���ڳߴ�
*
*/	
typedef struct tagWDSINZE_2I 
{
	f_int32_t width;   /**< ���(���ص�) */
	f_int32_t height;  /**< �߶�(���ص�) */
} WDSINZE_2I, *LP_WDSINZE_2I;

/** @struct PT_2I
*  @brief ��ά��(32λ����)
*
*/	
typedef struct tagPT_2I 
{
	f_int32_t x;  /**< x���� */
	f_int32_t y;  /**< y���� */
} PT_2I, *LP_PT_2I;

/** @struct PT_2F
*  @brief ��ά��(32λ������)
*
*/	
typedef struct tagPT_2F 
{
	f_float32_t x;  /**< x���� */
	f_float32_t y;  /**< y���� */
} PT_2F, *LP_PT_2F;

/** @struct PT_2D
*  @brief ��ά��(64λ������)
*
*/	
typedef struct tagPT_2D 
{
	f_float64_t x;  /**< x���� */
	f_float64_t y;  /**< y���� */
} PT_2D, *LP_PT_2D;

/** @struct PT_3I
*  @brief ��ά��/����(32λ����)
*
*/	
typedef struct tagPT_3I 
{
	f_int32_t x;  /**< x���� */
	f_int32_t y;  /**< y���� */
	f_int32_t z;  /**< z���� */
} PT_3I, *LP_PT_3I, VECOTR_3I, *LP_VECOTR_3I;

/** @struct PT_3F
*  @brief ��ά��/����(32λ������)
*
*/	
typedef struct tagPT_3F 
{
	f_float32_t x;  /**< x���� */
	f_float32_t y;  /**< y���� */
	f_float32_t z;  /**< z���� */
} PT_3F, *LP_PT_3F, VECOTR_3F, *LP_VECOTR_3F;

/** @struct PT_3D
*  @brief ��ά��/����/��������(64λ������)
*
*/	
typedef struct tagPT_3D 
{
	f_float64_t x;  /**< x���� */
	f_float64_t y;  /**< y���� */
	f_float64_t z;  /**< y���� */
} PT_3D, *LP_PT_3D, VECOTR_3D, *LP_VECOTR_3D, Obj_Pt_D, *LP_Obj_Pt_D;

typedef struct _PT_3D_8
{
	f_uint8_t x;
	f_uint8_t y;
	f_uint8_t z;
}PT_3D_8, *LP_PT_3D_8;

/** @struct Geo_Pt_D
*  @brief ��ά��(�������꣬64λ������)
*
*/
typedef struct tagGeo_Pt_D
{
    f_float64_t lon;        /**< ���� */
    f_float64_t lat;        /**< γ�� */
    f_float64_t height;     /**< �߶ȣ����θ߶ȣ� */
} Geo_Pt_D, *LP_Geo_Pt_D;

/** @struct PT_4D
*  @brief ��ά��(64λ������)
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
*  @brief 33����(64λ������)
*
*/
typedef struct tagMatrix33{
	    f_float64_t m[9];
	} Matrix33, *LPMatrix33;

/** @struct Matrix44
*  @brief 4x4����(64λ������)
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

