/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: WhiteCity.h
**
** 描述: 本文件是WhiteCity.c 的头文件
**
** 定义的函数:  
**                            
** 设计注记: 
**
** 作者：
**		LPF。
** 
**
** 更改历史:
**		2015.8.24 16:09  LPF 创建文件；
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 头文件引用
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
** 结构体声明
**-----------------------------------------------------------
*/

//纹理信息结构体
typedef struct tagSingleCityCfg_Texture{
	int m_PosID;
	char m_strname[255];			//纹理文件名称
	LP_PT_2D m_pTexverts;		//纹理坐标集
	unsigned int TextureID;			//纹理ID
}SingleCityCfg_Texture,*LPSingleCityCfg_Texture;

//单个建筑物信息结构体
typedef struct tagSingleCityCfg{
	double* Lon;
	double* Lat;
	double Hei;
	int	   PointNum;
	int    IdNum;
	int    DrawListID[3];				//显示列表ID,分为三角形、四边形、边框
	double radius;					//最大半径
	double AvgLonLat[3];			//中心点坐标
	int	m_bHasTexture;				//纹理存在的数量；0：不存在
	LPSingleCityCfg_Texture m_TextureCity;
	LP_PT_3D WhiteCity_XYZ;
	LP_PT_3I	WhiteCity_XYZ_Integer;	//整数化的局部坐标值
	int		 BottomPointNum;			//切三角形后的底面点个数
	LP_PT_2D BottomTriangle_LonLat;		//切三角形后的底面数据Lat，Lon
	LP_PT_3D BottomTriangle_XYZ;		//切三角形后的底面数据转换为世界坐标的XYZ
	LP_PT_3I  BottomTriangle_XYZ_Integer;	//整数化的局部坐标值
	LP_PT_3D NormalVector;			//各个面的法线向量
	double* AlpherVector;				//各个面的透明度值
	LP_PT_3D MiddlePointPerFace;		//各个面的中心点坐标

	stList_Head stListHead;


}SingleCityCfg,*LPSingleCityCfg;

/*-----------------------------------------------------------
** 函数声明
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
** 外部变量声明
**-----------------------------------------------------------
*/
extern void CreateList(stList_Head *plist );



#endif

