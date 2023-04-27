/**
 * @file macrodefine.h
 * @brief ���ļ��ṩ��һЩ�궨�弰ö���ͱ���
 * @author 615��ͼ�Ŷ� ��Ǫ��
 * @date 2016-05-08
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
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

#define PRECISION		0.0000001    /* ����ȽϾ��� */
#define RA2DE 57.295779513082320876798154814105
#define DE2RA 0.017453292519943295769236907684883
#define EARTH_RADIUS	6378137.0f                  /* ����뾶����λ:�� */
#define MAPLOGSIZE      20037508.342789             /* ī����ͶӰ����θ߶ȵ�һ�룬��λ���ף� */

#define TILESIZE           256     /* ��Ƭ�ߴ磨���ص����� */
#define VIEW_ANGLE 60.0

#define TER_WARNING_MAX_LAYERS 	128// 16     /* ���θ澯��ɫ���õ������ */
#define TER_WARNING_USE_TEXTURE_1D     /* ���θ澯����1ά����ı�־ */
#undef TER_WARNING_USE_TEXTURE_1D

#define sqr(a)  (a)*(a)
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b)) 
#define DEGREE_TO_RADIAN(x)		((x) * 0.017453292519943295769236907684883)
#define RADIAN_TO_DEGREE(x)		((x) * 57.295779513082320876798154814105)


#define QTNSQLNUM 		5		//QTN ��ʽ��sql ���ݿ����͸���

#define MAPHANDLEKEY  0x121833EF
#define ISMAPHANDLEVALID(_d) ( ((_d) != NULL) && ( ((sMAPHANDLE *)(_d))->key == MAPHANDLEKEY ) )
#define MAPDATANODEKEY	(0x07314227)
#define ISMAPDATANODE(_node) ( (_node != 0) && (_node->nodekey == MAPDATANODEKEY) )


/** @enum eMatrixMode
* @brief ��������
*/
typedef enum
{
    eMatrix_Mdview,			/**< ģ�Ӿ��� */
    eMatrix_Mdprj		    /**< ͶӰ���� */
}eMatrixMode;

/** @enum eRotateType
* @brief �����ӽ�ģʽ
*/
typedef enum
{
    eROTATE_CW_0,			/**< ����ģʽ,����ת */
    eROTATE_CW_90,		    /**< ˳ʱ����ת90��  */
    eROTATE_CW_180,		    /**< ˳ʱ����ת180�� */
    eROTATE_CW_270          /**< ˳ʱ����ת270�� */
}eRotateType;

/** @enum eColorMode
* @brief ��ɫģʽ
*/
typedef enum
{
	eCOR_normal,	/**< ����ɫ */
	eCOR_green, 	/**< ��ɫ   */
	eCOR_gray   	/**< ��ɫ   */
}eColorMode;

/** @enum eDNmode
* @brief ��ҹģʽ
*/
typedef enum
{
	eDN_daytime,	/**< �� */
	eDN_nighttime	/**< ҹ */
}eDNmode;

/** @enum eMoveMode
* @brief ��ͼ�˶�ģʽ
*/
typedef enum
{
	eMV_headup,	/**< ������ */
	eMV_northupmapmove,	/**< �����ϣ���ͼ�� */
	eMV_northupmapstatic  /**< �����ϣ���ͼ��ֹ */
}eMoveMode;

/** @enum eFreezeMode
* @brief ��ͼ����ģʽ
*/
typedef enum
{
	eFRZ_outfreeze,	/**< �ⶳģʽ */
	eFRZ_infreeze	/**< ����ģʽ */
}eFreezeMode;

/** @enum eRoamMode
* @brief ��ͼ����ģʽ
*/
typedef enum
{
	eROM_outroam,	/**< ������ģʽ */
	eROM_inroam  	/**< ����ģʽ */
}eRoamMode;

/** @enum eSceneMode
* @brief ��ͼ������ʾģʽ
*/
typedef enum
{
	eScene_normal,	/**< ����ģʽ */
	eScene_eyebird 	/**< ӥ��ģʽ */
}eSceneMode;

/** @enum eIsRoamed
* @brief ��ͼ���α�־
*/
typedef enum
{
	eROM_notroamed,	/**< ��û���� */
	eROM_roamed  	/**< ������ */
}eIsRoamed;

/** @enum eIsRoamHomed
* @brief ��ͼ���ι�λ��־
*/
typedef enum
{
	eROM_notroamhomed,	/**< ��û���ι�λ */
	eROM_roamhomed  	/**< �����ι�λ */
}eIsRoamHomed;

/** @enum eTerwarnMode
* @brief ���θ澯ģʽ
*/
typedef enum
{
	eTERWARN_outwarn,	/**< �ǵ��θ澯ģʽ */
	eTERWARN_inwarn  	/**< ���θ澯ģʽ */
}eTerwarnMode;

/** @enum eViewMode
* @brief �����ӽ�ģʽ
*/
typedef enum
{
	eVM_DEFAULT_VIEW,           /**< Ĭ���ӽ�(�����ӽ�) */
    eVM_FIXED_VIEW,			    /**< ����۲��ӽ�  */
    eVM_COCKPIT_VIEW,		    /**< �����ӽ�      */
    eVM_FOLLOW_VIEW,		    /**< β���ӽ�      */
    eVM_OVERLOOK_VIEW,	        /**< �����ӽ�      */
    eVM_SCALE_VIEW              /**< ������ʾ�ֱ��ʶԵ�ͼ�������ŵ��ӽ�,��30/45/75�������ӽ�*/
}eViewMode;

/** @enum eProjectMode
* @brief ͶӰģʽ
*/
typedef enum
{
	ePRJ_mercator,	    /**< ī���� */
	ePRJ_lambert,      	   /**< ������ */
	ePRJ_ball		   /**���� */
}eProjectMode;

/** @enum eMap23dMode
* @brief ����άģʽ
*/
typedef enum
{
	eMAPMode_2D,	    /**< ��άģʽ */
	eMAPMode_3D      	/**< ��άģʽ */
}eMap23dMode;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
