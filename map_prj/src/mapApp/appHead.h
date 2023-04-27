/**
 * @file appHead.h
 * @brief ���ļ�������app������ݽṹ
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-26
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
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
*  @brief ��ʾ������
*
*/
typedef struct tagScreenParam{
	f_float32_t phi_width;     /**<  ��ʾ����������(��λ������) */
	f_float32_t phi_height;    /**<  ��ʾ��������߶�(��λ������) */
	f_int32_t   rotate_type;   /**<  ��ʾ������ת��ʽ,0:����,1:˳ʱ��90��,2:˳ʱ��180��,3:˳ʱ��270�� */
	f_int32_t   width;         /**<  ��ʾ���ķֱ��ʿ��(��λ������) */
	f_int32_t   height;        /**<  ��ʾ���ķֱ��ʿ��(��λ������) */
	}sSCREENPARAM;

/** @struct sViewWindowSize
*  @brief �Ӵ�λ�ò���
*
*/	
typedef struct tagViewWindowSize{
    f_int32_t llx;
    f_int32_t lly;              /**< �Ӵ����½ǵ�λ�ã���λ������. */
    f_int32_t width;            /**< �Ӵ��Ŀ�ȣ���λ������. */
    f_int32_t height;           /**< �Ӵ��ĸ߶ȣ���λ������. */
	}sViewWindowSize;
	
/** @struct sViewWindowParam
*  @brief �Ӵ���ؿ���
*
*/
typedef struct tagViewWindowParam
{
    sViewWindowSize outer_view_port;    /**< �ⲿ�Ӵ�     */    
    f_float32_t     alpha;              /**< �Ӵ���͸���� */
    sColor3f        bkcolor;            /**< �Ӵ��ı���ɫ */
    f_float32_t     rtangle;            /**< �Ӵ�����ת�ǣ���λ���ȣ� */
    sViewWindowSize inner_view_port;    /**< �ڲ��Ӵ�     */  
}sViewWindowParam;

/** @struct sCAMERACTRLPARAM
*  @brief ������Ʋ���
*
*/
typedef struct tagCameraCtrlParam
{
	f_float32_t view_near;          /**< ���ý���ľ��루��λ���ף�   */
	f_float32_t view_far;           /**< Զ�ý���ľ��루��λ���ף�   */
	f_float32_t view_angle;         /**< ƽ��ͷ�ӽ�(��λ����)         */
	f_float32_t view_res;           /**< ��ʾ�ֱ��� ����λ����/���أ� */    
	f_int32_t   compass_radius;     /**< �̶Ȼ��뾶����λ�����أ�     */
	f_int32_t   view_type;          /**< �ӽ�����                     */
    PT_2I       rotate_center;      /**< ��ͼ��ת���ģ���λ�����أ�   */
}sCAMERACTRLPARAM;

/** @struct sCameraParam
*  @brief �������
*
*/
typedef struct tagCameraParam
{
	Obj_Pt_D    m_ptEye;		/**< �ӵ�λ��(������) */
	Geo_Pt_D    m_geoptEye;     /**< �ӵ�λ��(��������) */
	PT_3D		m_ptCenter;		/**< �ӵ����ĵ�λ��(�߶�Ϊ0�ĵ����) */
	PT_3D		m_ptUp;			/**< ����Ϸ��� */
	PT_3D		m_ptTo;			/**< ������� */
	f_float64_t	m_lfDistance;	/**< ��������ĵ�(�����)�ľ��� */
	f_float64_t	m_lfNear;		/**< ��׶����ü��� */
	f_float64_t	m_lfFar;		/**< ��׶��Զ�ü��� */
}sCameraParam;

/** @struct sModeCtrlCmd
*  @brief ��ͼ��ʾģʽ��������
*
*/
typedef struct tagModeCtrlCmd
{
	f_int32_t cor_mode;      /**< ��ɫģʽ */
    f_int32_t dn_mode;       /**< ��ҹģʽ */
    f_int32_t mv_mode;       /**< �˶�ģʽ */
    f_int32_t frz_mode;      /**< ����ģʽ */
    f_int32_t rom_mode;      /**< ����ģʽ */
    f_int32_t terwarn_mode;  /**< ���θ澯ģʽ */
    f_int32_t scene_mode;    /**< ����ģʽ(0:����ģʽ,1:ӥ��ģʽ)  */
    f_int32_t map2dor3d;	/*0:  ʸ����ʾ����1 :����ʾʸ��*/
    f_int32_t mapDisplay_mode;	/*��ͼͼ����ӷ�ʽ0:��Ƭtqs/tqt,1:��Ƭ+ʸ��vqt(��ʱδʹ��),2:����clr,3:����+��Ӱhil(��ʱδʹ��),4:ʸ����դ��,5:��ʸ��*/
	f_int32_t close3d_mode;	/*������ά����0: ������, 1 : ����*/
	f_int32_t rom2d_mode;	/**<�����µ�����ģʽ>*/
	f_int32_t rom2d_mode_old;   /**<�����µ�ǰһ֡������ģʽ��Ϊ�˵���ģʽ���κ��ܻص�ԭλ>*/
	f_int32_t svs_mode;		/**<svs�ľ�γ��������ʾģʽ>*/
	f_int32_t is_3dplane_display;	/**<�Ƿ���ʾ3Dģ��>*/
	f_int32_t plane_symbol_display;	/**<�Ƿ���ʾ�ɻ�����0:����ʾ,1:��ʾ>*/
	f_int32_t color_layermask_cfg_index;	/**<��ɫ��ӵ�����ļ�������>*/
	f_int32_t area_ba_draw;	        /**<�Ƿ����ʸ��������0:������,1:����>*/
	f_int32_t sea_map_draw;	        /**<�Ƿ���ƺ�ͼ0:������,1:����>*/
	f_int32_t text_draw_level;      /* ����ע�Ǻͷ�����ʾ��λ0-����ʾ�κ�ע�Ǻͷ��� 1-ֻ��ʾʡ����к��еȳ��еĵ���ע�Ǻͷ��� 2-������ʾ���е�ע�Ǻͷ���*/
}sMODECTRLCMD;

/** @struct sATTITUDE
*  @brief ����
*
*/
typedef struct tagAttitude
{
    f_float64_t yaw;     /**< ����� */
	f_float64_t mag;     /**< �Ų�� */
    f_float64_t pitch;   /**< ������ */
    f_float64_t roll;    /**< ����� */
}sATTITUDE;

/** @struct sMAPTREE
*  @brief ��
*
*/
typedef struct tagMapTree
{
	f_int32_t rootlevel;      /**< ���ڵ�Ĳ㼶 */
	f_int32_t rootxidx;       /**< ���ڵ��x���������� */
	f_int32_t rootyidx;       /**< ���ڵ��y���������� */
	f_int32_t level;          /**< �������      */
	VOIDPtr   texfile;        /**< ������Ӧ���������� */
	VOIDPtr   terfile;        /**< ������Ӧ�ĵ������� */
	VOIDPtr   infofile;       /**< ������Ӧ�Ľڵ���Ϣ���� */
	VOIDPtr	vqtfile;	    /**<������Ӧ��ʸ����������*/
	VOIDPtr	clrfile;	    /**<������Ӧ����ѣ��������*/
	VOIDPtr	hilfile;	   /**<������Ӧ��shade��������*/
}sMAPTREE;

/** @struct sMAPFILE
*  @brief �����ļ�
*
*/
typedef struct tagMapFile{
	f_int32_t fcount;       /**< �ļ��� */
	sQTFName *fName_list;   /**< tqt �ļ����б� */

//	f_int32_t fcount_vqt;	/*vqt �ļ�����*/
//	sQTFName *fName_list_vqt;	/*vqt �ļ����б�*/
	}sMAPFILE;

/** @struct sTerWarningData
*  @brief ���θ澯��ɫ����
*
*/
typedef struct tagTerWarningData{
    f_int32_t   index;        /**< �㼶���� */
	f_float32_t height_min;   /**< �澯�߶���������ֵ */
	f_float32_t height_max;   /**< �澯�߶���������ֵ */
	f_float32_t red;          /**< ��ɫ���� */
	f_float32_t green;        /**< ��ɫ���� */
	f_float32_t blue;         /**< ��ɫ���� */
}sTerWarningData;

/** @struct sTerWarningData
*  @brief ���θ澯��ɫ���ü���
*
*/
typedef struct sTerWarningSet{
    f_int32_t   layers;              /**< �ܼ��� */
	f_float32_t height_max;          /**< �澯�߶ȵ����ֵ */
	f_int32_t   height_max_index;    /**< �澯�߶����ֵ��Ӧ�Ĳ㼶���� */
	f_float32_t height_min;          /**< �澯�߶ȵ���Сֵ */
	f_int32_t   height_min_index;    /**< �澯�߶���Сֵ��Ӧ�Ĳ㼶���� */
	sTerWarningData ter_warning_data[TER_WARNING_MAX_LAYERS]; /**< �澯���� */
}sTerWarningSet;

/** @struct sAuxNavInfoDisSwitch
*  @brief ����������Ϣ��ʾ����
*
*/
typedef struct sAuxNavInfoParam{
	sZplParam zpl_param;   /**< 0�������߲��� */
	sCpsParam cps_param;   /**< ���̲���      */
	sPrefileParam		prefile_param;			/**����ͼ�Ĳ������ñ�*/
	BOOL is_pos_dis;       /**< λ����Ϣ�Ƿ���ʾ���� FALSE������ʾ��TRUE����ʾ */
	}sAuxNavInfoParam;

typedef struct tagDisList{
	f_int32_t displaylist_id;
	f_int32_t texture_id;
	}sDisList;	
	
/** @struct sAuxNavInfoList
*  @brief ����������Ϣlist
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
}PLANE, *LPPLANE;	// �Ӿ�����

typedef struct tagFRUSTUM
{
	PLANE plane[6];
}FRUSTUM, *LPFRUSTUM;	// �Ӿ���6����

/** @struct sGLRENDERSCENE
*  @brief �����ṹ
*
*/
typedef struct tagGLRenderScene{
//	VOIDPtr          map_handle;           /**< ��ͼ���   */
	f_uint32_t       scene_index;          /**< ��������     */
	f_int32_t        project_mode;         /**< ͶӰ����  0ī���� 1������ 2������*/
	
	FUNCTION         func2dMap;            /**< ��ά��ͼ���ܵ���ں��� */
	FUNCTION         func3dMap;            /**< ��ά��ͼ���ܵ���ں��� */
	USERLAYERFUNCTION  drawScreenUserLayer;   /**< �û���Ļͼ�㹦�ܵ���ں��� */
	USERLAYERFUNCTION  drawGeoUserLayer;      /**< �û�����ͼ�㹦�ܵ���ں��� */
	FUNCTION         funcScreenPt2GeoPt;   /**< ��Ļ���굽�������깦�ܵ���ں��� */
	FUNCTION         funcGeoPt2ScreenPt;   /**< �������굽��Ļ���깦�ܵ���ں��� */
	
//	sMAPTREE         *map_trees;           /**< ��ͼ��     */
//	f_int32_t        data3d_init_status;   /**< 3D���ݳ�ʼ��״̬     */	
//	sTtfFont         ttf_font;             /**< ttf ���� */
	
	SEMID            periodparaminput_sem; /**< �������ڲ���(Ӧ�ò�������뵽param_input)���ź���     */
	SEMID            periodparamset_sem;   /**< �������ڲ���(��param_input�������ڲ�ʹ�õ�param)���ź���     */	
	SEMID            eventparaminput_sem;  /**< �����¼�����(Ӧ�ò�������뵽param_input)���ź���     */
	SEMID            eventparamset_sem;    /**< �����¼�����(��param_input�������ڲ�ʹ�õ�param)���ź���     */
	
	sCAMERACTRLPARAM camctrl_param_input;  /**< �ⲿ���յ���������Ʋ��� */
	sMODECTRLCMD     mdctrl_cmd_input;     /**< �ⲿ���յ���ģʽ�������� */
	Geo_Pt_D         geopt_pos_input;          /**< �ⲿ���յ���λ����Ϣ���������ӽ�ʱ��������ǽ���λ�ã�
	                                           �������ӽ�ʱ����������ӵ�λ�ã��ڸ�������£��ӵ�߶��Ǹ������̷�������ģ��� */	
    PT_2I            rom_pt_input;         /**< �ⲿ���յ�������λ�� */	                                           
	sATTITUDE        attitude_input;       /**< �ⲿ���յ��ĺ�����Ϣ */
	sTerWarningSet   m_terwarning_set_input;  /**< �ⲿ���յ��ĵ��θ澯�������� */
	sAuxNavInfoParam auxnavinfo_param_input;   /**< �ⲿ���յ��ĸ���������Ϣ��ʾ���� */

	f_int32_t is_romed;          /**< �Ƿ��Ѿ����ι��ı�־0:��û����,1:������  */
	f_int32_t is_rom_homed;      /**< �Ƿ��Ѿ����ι�λ���ı�־0:��û���ι�λ,1:�����ι�λ  */
	
	sCAMERACTRLPARAM camctrl_param;        /**< �����ڲ�ʹ�õ�������Ʋ��� */
	sMODECTRLCMD     mdctrl_cmd;           /**< �����ڲ�ʹ�õ�ģʽ�������� */
	Geo_Pt_D         geopt_pos;            /**< �����ڲ�ʹ�õķ��в�����γ����Ϣ */
	Geo_Pt_D         planesymbol_pos;      /**< �����ڲ�ʹ�õķɻ����ž�γ����Ϣ */
	f_float64_t      planesymbol_yaw;      /**< �����ڲ�ʹ�õķɻ����ź�����Ϣ */
	PT_2I            rom_pt;               /**< �ⲿ���յ�������λ�� */	
	sATTITUDE        attitude;             /**< �����ڲ�ʹ�õĺ�����Ϣ */
	sTerWarningSet   m_terwarning_set;     /**< �����ڲ�ʹ�õĵ��θ澯�������� */
	Obj_Pt_D         objpt_pos;            /**< ���в�����γ�߶�Ӧ����������� */
	sAuxNavInfoParam auxnavinfo_param;     /**< �����ڲ�ʹ�õĸ���������Ϣ��ʾ���� */
	sAuxNavInfoList  auxnav_list;         /**< ����������Ϣ����ʾ�б� */
	
	//Geo_Pt_D         cam_geo_pos;          /**< ����ĵ�������ϵ�µ�λ��  */
	//Obj_Pt_D         cam_obj_pos;          /**< �������������ϵ�µ�λ��  */
	
	sCameraParam     camParam;             /**< �������                */
	Matrix44	     m_mxRotate;           /**< �ֲ��������̬��ת����  */
	Matrix44	     m_mxWorld;	           /**< �ֲ����굽��������任����  */
	f_float64_t	     m_lfProjMatrix[16];   /**< ͶӰ����  */
	f_float64_t	     m_lfModelMatrix[16];  /**< ģ�Ӿ���  */
	f_int32_t        outerviewport[4];     /**< �ⲿ�ӿڲ���  */
	f_int32_t        innerviewport[4];     /**< �ڲ��ӿڲ���  */
	f_float64_t		 matrix[16];           /**< �ü�����  */
	f_float64_t      invmat[16];           /**< �ü����������  */
	
	//f_float64_t      m_lfDelta;		   /**< ��Ļ�ߴ絽��ά�ռ�ߴ�ı��� */
	PT_2D            rotate_opengl_pt;     /**< ����ģʽ����ת���ĵ�openGL���� */
	
	RoamT            roam;  //DXJ
	stList_Head 	pScene_ModelList;	/*�����ڵ�ģ������ڵ�ͷ��ַlpf add */	
	f_int32_t		airplane_id;			/*�����ڵķɻ�ģ��id lpf add */

//    Geo_Pt_D                cam_geo_pos; 
//    Obj_Pt_D                cam_obj_pos;
	
	FRUSTUM                 frustum;     /*�Ӿ������*/

	BOOL                    is_need_terwarning;
	f_int32_t				warn_texid;          // ���θ澯һά�������
	f_int32_t				is_need_regenerate_warn_texid;          // ���θ澯һά��������Ƿ�����������
	f_float32_t             terwaring_height;
	f_float32_t             s_nTexCoordPerMeter;  // ÿ������Ҫ������������
	f_float64_t             cur_plane_height;
//	sTerWarningSet          m_terwarning_set;
	BOOL                    is_warn_data_set_success;  // �������óɹ���־
	f_uint32_t              atms_text;            /**< ���ڴ������һά���� */
   	f_uint32_t              atms_textgray;        /**< ���ڴ�����ĻҶ�һά���� */

	f_float64_t		roam_res;			//��������µķ������̷ֱ���
	f_float64_t		roam_res_for_QTN;	//���������������QTN�ķ������̷ֱ���
	
	stList_Head		scene_draw_list;	//��Ҫ���ƵĽڵ�����

	f_float64_t 		pre_cameraLonLatHei[3];	//��һ�ε��ӵ㾭γ��
	f_float64_t 		pre_cameraYPR[3];	//��һ�ε��ӵ㾭γ��
	f_uint32_t		qtn_nodeID;				//��Ҫ���õĵ���qtn �����ID ��

	sPrefileParamUsed	prefile_param_used;		//�ڲ�ʹ�õĴ�ֱ��������ͼϸ������

	TerrainMapContext termap_param_input;		/**<�ⲿ���յĺ�·���������ͼ����>*/
	TerrainMapContext termap_param;			    /**<�ڲ�ʹ�õĺ�·���������ͼ����>*/
	TerrainMapUsed	terrainMap_param_used;	    //�ڲ�����ʱʹ�õĺ�·���������ͼϸ������
	f_uint32_t       set_center_pos;          /**< ����ģʽ���Ƿ���Ӧ�������ĵ㹦�ܣ�0-����Ӧ��1-��Ӧ */
	Geo_Pt_D         centerpt_pos;            /**< ����ģʽ��ʹ�õ��������ĵ㾭γ����Ϣ */
	Geo_Pt_D         roam_start_pt_pos;       /**< �ɷ����ν�������ʱ����¼��ǰ���ĵ㾭γ����Ϣ���������ι�λ */
	}sGLRENDERSCENE;

/** @struct sViewWindow
*  @brief �Ӵ��ṹ
*
*/	
typedef struct tagViewWindow{
	SEMID              viewwdparaminput_sem;     /**< ���ò���(Ӧ�ò�������뵽param_input)���ź���     */
	SEMID              viewwdparamset_sem;       /**< ���ò���(��param_input�������ڲ�ʹ�õ�param)���ź���     */
	sViewWindowParam   viewwd_param_input;       /**< �ⲿ���յ����Ӵ����Ʋ���     */
	sViewWindowParam   viewwd_param;             /**< �����ڲ�ʹ�õ��Ӵ����Ʋ���     */
	}sViewWindow;

/** @struct sPAINTUNIT
*  @brief ���Ƶ�Ԫ�ṹ.
*  @note  ���Ƶ�Ԫ��һ�������Ӵ���һ����Ⱦ������������ɣ�\n
          �ڻ���ǰ����Ҫ�������Ƶ�Ԫ��\n
          Ȼ��Ҫ�����Ӵ��ͳ��������Դ�������Ӵ��Ͷ��������\n
          ����Ҫ���Ӵ��ͳ���������һ���Ӵ�ֻ�ܹ���һ�������������Ա�������ϵ.
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
