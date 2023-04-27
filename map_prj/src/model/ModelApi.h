/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: ModelApi.h
**
** ����: ���ļ�����ģ�ͳ�ʼ����ģ�ͻ��ƵĽӿڵ��ú�����ͷ�ļ���
**
** ����ĺ���:  
**
**                            
** ���ע��: 
**
** ���ߣ�
**		LPF��
** 
**
** ������ʷ:
**		2016-10-24 9:47 LPF �������ļ�
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#ifndef _MODELAPI_H_
#define _MODELAPI_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"

// #include "Loadmodel.h"
// #include "Model.h"
// #include "ModelApi.h"
#include "../model/3DSLoader.h"
#include "../mapApp/appHead.h"
/*-----------------------------------------------------------
** �ṹ������
**-----------------------------------------------------------
*/
#define FLYTRACE_NODENUM_MAX	100		// ÿ���ɻ����洢100�����й켣��
typedef struct tagTraceNode
{
	BOOL	m_bUse;		// �ýڵ��ǹ���ʹ��
	PT_3D	m_ptUp;		// ����ڷɻ����ϵĵ�
	PT_3D	m_ptDown;   // ����ڷɻ����µĵ�
} TraceNode, *LPTraceNode;

//ģ����Ϣ�ṹ��
typedef struct tagModelRender{
	f_int32_t	m_modelFlag;		//��Ǵ˻�������ģ��
	f_int32_t m_modeltype;		//model type in config.txt
	f_int32_t m_modelID;			//model ID used outside
	BOOL	m_flagshield;			//model �Ƿ����α�־
	
	PT_3D	m_modelPos;			//lat, lon, hei
	PT_3D	m_modelAngle;		//yaw, pitch, roll
	f_float64_t	m_modelScale;	//scale index
	f_float64_t	m_modelSize;	//size
	f_int32_t	m_modelDisplaytype;	//display type
	PT_3D     m_modelPoxWorld;	//x,y,z in world vertex
	f_int32_t m_heimode;			//��ظ߶�0�����Ը߶�1, ���θ߶�2
	PT_4D m_color[4];			//color used in draw,  �߿�,��Ļ��,�⻷,����ɫ
	f_int32_t m_colorNum_use;		//Ӧ��ʹ�õ���ɫ�������
	f_int32_t m_colorPicture;		//��ɫģʽ��ʹ�õ�����ID
	f_int32_t m_colorPictureIsNeedRebuild;	//��ɫģʽ��ʹ�õ�����ID �Ƿ���Ҫ��������:  0 : ��,1 : ��
	LPModel3DS m_pModel3ds;		//model 3ds ����
	BOOL	m_selectMode;		//select mode Renderѡ��ʽ�Ļ���ģ��
	f_float64_t m_modelMatrix[16];	//��ת�ƶ�����
	BOOL	m_MatrixChange;		//��ת�����Ƿ�ı�

	Matrix44	m_mxRotate;			// �ֲ��������̬��ת����
	Matrix44	m_mxWorld;			// �ֲ����굽��������任����

	f_int32_t m_nodeIndex;		//�ڵ����к�

	BOOL		m_flagUser[4];			//�û�ͼ����ʾ��־,�ֱ����β�������֣������壬�û��Զ���
//	BOOL		m_flagTrace;							//�Ƿ���ʾ�켣,����
	
	TraceNode	m_vFlyTrace[FLYTRACE_NODENUM_MAX];	// �ɻ����й켣��
	f_uint16_t 	s_pTraceIndex;						// ��¼��ǰ���õĵ�
	f_uint16_t 	s_pTraceCount;						// �ڵ��ʱ��	

	f_int32_t m_childNum;						//child model number
//	struct tagModelRender * m_childModel;		//child model struct
	stList_Head 		m_childlist;						//child model list

	f_uint32_t	 m_glLists;							/**��糤������ʾ�б�ID*/
	
	f_uint16_t  m_pString[4][16];					//ģ�͵�UNICOD���ַ���,���֧��4��
	f_int32_t   m_stringNum[4];					//ÿ���ַ������ַ�����
	f_uint32_t	m_stringList;					/**��ʶ�ŵĳ����θ���ʾ�б�ID*/
	Matrix44	m_stringWorld;					//ģ�ͱ�ʶ�ŵľֲ�����ϵ����������ϵ�ı任����
	BOOL		m_stringWorldChange;			//�ñ任�����Ƿ����¼���
	
	BOOL		m_drawOrNot;					//�Ƿ���Ƹ�ģ�ͣ�TRUE,����,�������ڸ��ڵ�

	//��С��ͼ�й�
	BOOL	m_flagModelScreen;				//ģ�ͷ����Ƿ���С��ͼ����
	PT_2D	m_ModelScreenXY;				//ģ�ͷ�����С��ͼ��λ��
	f_int32_t m_ModelScreenID;				//ģ����С��ͼ�ķ���ID
	//��С��ͼ�й�
	stList_Head	stListHead;					//����Ľڵ�ͷ
}ModelRender,*LPModelRender;

typedef void (*USER_MODEL_DRAW_FUNC)(void *userLayerParam, void *userLayerParam1);

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

//��ʼ��ģ�����ݡ���ģ���С�ģ�����弰�����ж�ģ���Ƿ�ɼ��ĵ�������
BOOL InitModel();

//����ģ��
// �������:  SelectModel; TRUE-ѡ��ģ�͵Ļ��Ʒ�ʽ,FALSE-����ģʽ����ģ��
//            pModelScene  ��Ⱦ�������
BOOL RenderModel(BOOL SelectModel , sGLRENDERSCENE* pModelScene);

//void RenderScreenModelFlag();

/*�û�����õ�API*/

//����ģ������(modeltype)����ģ�ͣ���ӵ�ģ�ͻ�������������ģ��ID(��ģ�ͽṹ��ָ��)
f_int32_t NewModel(f_int32_t modeltype);
//����ģ��ID(��ģ�ͽṹ��ָ��)ɾ��ģ�ͣ�����ģ�ͻ���������ɾ��
BOOL DestroyModel(f_int32_t modelID);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�lat, lon, height
BOOL SetModelPos(f_int32_t modelID, PT_3D latlonhei);
//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�lat, lon, height
BOOL GetModelPos(f_int32_t modelid, LP_PT_3D model_pos);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�yaw,pitch,roll
BOOL SetModelAngle(f_int32_t modelID, PT_3D yawpitchroll);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�����ϵ��
BOOL SetModelScale(f_int32_t modelID, f_float64_t modelscale);
//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�����ϵ��
BOOL GetModelScale(f_int32_t modelid, f_float64_t* model_scale);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵���ʾ��ʽ
//0��������ʾ��1��������2���߿�(������С,��ɫĬ��Ϊ��ɫ)��
//3��ģ��+�߿�(1.02����С,��ɫĬ��Ϊ��ɫ)��4��ģ��+������(������С,�ߴ�3��,��ɫĬ��Ϊ��ɫ)��
//5���⻷Ч��,��ģ��+��ɫ(1.05����С,��ɫĬ��Ϊ���ɫ)��6����ɫ����(��ɫĬ��Ϊ��ɫ)
BOOL SetModelDisplay(f_int32_t modelID, f_int32_t modeldisplaytype);
//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵���ʾ��ʽ
BOOL GetModelType(f_int32_t modelid, f_int32_t *model_type);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵ĸ߶�ֵ���ͣ���Ը߶�0�����Ը߶�1, ���θ߶�2
BOOL SetModelHeiMode(f_int32_t modelID,f_int32_t HeiMode);

//BOOL SetModelTraceFlag(int modelid, BOOL flag);
//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵����α�־
BOOL SetModelShield(f_int32_t modelID, BOOL shieldflag);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�ָ���û�ͼ����ʾ��־
//�������: ģ�͵�ID����ʾ��־flag����ʾͼ��mode:0-β����1-���֡�2-�����壬3-�Զ���
BOOL SetModelUser(f_int32_t modelID, BOOL userflag, int mode);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵������û�ͼ����ʾ��־
BOOL SetModelUserAll(f_int32_t modelID, BOOL userflag);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵ĸ�����ʾģʽ����ɫ
BOOL SetModelColor(int modelid, int color_type, PT_4D colorset);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵���С��ͼ��Ļ������ʾ��־����ʽ
BOOL SetModelScreenFlagType(f_int32_t modelID, int  type);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵ĸ�������
BOOL SetModelText(f_int32_t modelID, unsigned char *textString, int line);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�lat, lon, height������,��ģ���ƶ�
//�������: ģ��id,ģ��λ������ֵ,�ƶ��ĸ�����lat :0,lon:1,hei:2
BOOL SetModelPosAdd(f_int32_t modelid, f_float64_t model_dpos, f_int32_t mode);

//����ģ��ID(��ģ�ͽṹ��ָ��)����ģ�͵�yaw,pitch,roll������,��ģ����̬�����仯
//�������: ģ��id,ģ��λ������ֵ,�ƶ��ĸ�����lat :0,lon:1,hei:2
BOOL SetModelAngleAdd(f_int32_t modelid, f_float64_t model_dagnle, f_int32_t mode);


BOOL NewChildModel(f_int32_t modelid,  f_int32_t model_childID, f_int32_t modeltype);
BOOL SetChildModelPos(f_int32_t modelid, f_int32_t childmodelID, PT_3D xyz);
BOOL SetChildModelAngle(f_int32_t modelid, f_int32_t childmodelID, PT_3D yawpitchroll);
BOOL SetChildModelScale(f_int32_t modelid, f_int32_t childmodelID, f_float64_t scale);
BOOL SetChildModelPosAdd(f_int32_t modelid, f_int32_t childmodelID, f_float64_t model_dpos, f_int32_t mode);
BOOL SetChildModelAngleAdd(f_int32_t modelid, f_int32_t childmodelID, f_float64_t model_dangle, f_int32_t mode);
BOOL SetChildNum(f_int32_t modelid, f_int32_t num);

BOOL NewChildModelDynamic(f_int32_t parent_id,  f_int32_t child_id);
BOOL DeleteChildModelDynamic(f_int32_t parent_id, f_int32_t child_number, f_int32_t* child_id);

void AttachpScene2Model(sGLRENDERSCENE* pModelScene);

//�û�����õ���ײ���API
f_int32_t CheckModelcollisionScreen(PT_3D collisionPoint, f_float64_t collisionRadius, sGLRENDERSCENE* pModelScene);
//f_int32_t CheckModelcollisionScreen(PT_3D collisionPoint, f_float64_t collisionRadius);

BOOL RenderFlyTrace(int nonecon, LPModelRender pModelSelect);
void RenderModelRectangle(int nonecon, LPModelRender pModelSelect);
void RenderModelTxt(char * text_out, LPModelRender pModelSelect, sGLRENDERSCENE* pModelScene);
void ModelUserDraw(int none, LPModelRender pModelUser);

//������ͼ������ʹ�õ�API
void set_model_displaytype(LPModelRender model_select,f_int32_t displaytype);
BOOL set_model_colorNumUse(LPModelRender model_select,f_int32_t color_use);
BOOL set_model_color(LPModelRender model_select,int color_type, PT_4D colorset);
void CreateList(stList_Head *plist );

int userFuncModelRegedit(USER_MODEL_DRAW_FUNC userLayerDrawFunc, void *userLayerParam);

//���Ժ���
BOOL TestModelAppMake(int scene);
BOOL AddModelToScene(int scene);
void TestModelAppSet(int scene);

double GetWorldMapSizeX();
double GetWorldMapSizeY();

extern void Maxtrix4x4_Mul(Matrix44* pOut, Matrix44* pIn0, Matrix44 *pIn1);

extern int LoadParticlesTexture(const f_char_t* filepath, f_uint32_t* textureID);
/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/



#endif
