/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: WhiteCity.h
**
** ����: ���ļ���WhiteCity.c ��ͷ�ļ�
**
** ����ĺ���:  
**                            
** ���ע��: 
**
** ���ߣ�
**		LPF��
** 
**
** ������ʷ:
**		2015.8.24 16:09  LPF �����ļ���
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#ifndef _WHITECITY_H_
#define _WHITECITY_H_

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

//������Ϣ�ṹ��
typedef struct tagSingleCityCfg_Texture{
	int m_PosID;
	char m_strname[255];			//�����ļ�����
	LP_PT_2D m_pTexverts;		//�������꼯
	unsigned int TextureID;			//����ID
}SingleCityCfg_Texture,*LPSingleCityCfg_Texture;

//������������Ϣ�ṹ��
typedef struct tagSingleCityCfg{
	double* Lon;
	double* Lat;
	double Hei;
	int	   PointNum;
	int    IdNum;
	int    DrawListID[3];				//��ʾ�б�ID,��Ϊ�����Ρ��ı��Ρ��߿�
	double radius;					//���뾶
	double AvgLonLat[3];			//���ĵ�����
	int	m_bHasTexture;				//������ڵ�������0��������
	LPSingleCityCfg_Texture m_TextureCity;
	LP_PT_3D WhiteCity_XYZ;
	LP_PT_3I	WhiteCity_XYZ_Integer;	//�������ľֲ�����ֵ
	int		 BottomPointNum;			//�������κ�ĵ�������
	LP_PT_2D BottomTriangle_LonLat;		//�������κ�ĵ�������Lat��Lon
	LP_PT_3D BottomTriangle_XYZ;		//�������κ�ĵ�������ת��Ϊ���������XYZ
	LP_PT_3I  BottomTriangle_XYZ_Integer;	//�������ľֲ�����ֵ
	LP_PT_3D NormalVector;			//������ķ�������
	double* AlpherVector;				//�������͸����ֵ
	LP_PT_3D MiddlePointPerFace;		//����������ĵ�����

	stList_Head stListHead;


}SingleCityCfg,*LPSingleCityCfg;

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/
int CheckBulidingInFrustum(LPSingleCityCfg lpsinglecitycfg_used, sGLRENDERSCENE* pModelScene);
//void InitList(LPList pList);
int LoadWhiteCityTexture(const f_char_t* filepath, f_uint32_t* textureID);
void WhiteCityCallList(LPSingleCityCfg lpsinglecitycfg_LIST,int lpsinglecitycfg_LIST_ID, sGLRENDERSCENE* pModelScene);
int InitWhiteCityData();
int RenderWhiteCity(sGLRENDERSCENE* pModelScene);
void CalModelMatrix(sGLRENDERSCENE* pModelScene, double x, double y, double z, double size);

/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/
extern void CreateList(stList_Head *plist );



#endif

