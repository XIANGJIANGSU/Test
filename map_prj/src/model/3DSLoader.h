// 导入3DS模型
#ifndef __3DSLOADER_H__
#define __3DSLOADER_H__
#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#else
#include <vxWorks.h>
#endif

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
#include "ModelreadTxtFile.h"

extern void CreateList(stList_Head *plist );


/** 下面是定义一些块的ID号 */
/**  基本块(Primary Chunk)，位于文件的开始 */
#define PRIMARY       0x4D4D

/**  主块(Main Chunks) */
#define OBJECTINFO    0x3D3D		/**< 网格对象的版本号 */
#define VERSION       0x0002		/**< .3ds文件的版本 */
#define EDITKEYFRAME  0xB000		/**< 所有关键帧信息的头部 */

/**<  对象的次级定义 */
#define MATERIAL	  0xAFFF		/** 纹理信息 */
#define OBJECT		  0x4000		/** 对象的面、顶点等信息 */

/**  材质的次级定义 */
#define MATNAME       0xA000		/**< 材质名称 */
#define MATDIFFUSE    0xA020		/**< 对象/材质的颜色 */
#define MATMAP        0xA200		/**< 新材质的头部 */
#define MATMAPFILE    0xA300		/**< 保存纹理的文件名 */
#define OBJ_MESH	  0x4100		/**< 新的网格对象 */


/** 网格对象的次级定义 */
#define OBJ_VERTICES  0x4110		/**< 对象顶点 */
#define OBJ_FACES	  0x4120		/**< 对象的面 */
#define OBJ_MATERIAL  0x4130		/**< 对象的材质 */
#define OBJ_UV		  0x4140		/**< 对象的UV纹理坐标 */

/** 面的结构定义 */
typedef struct tagFace		
{	
	f_int32_t m_nVertIndex[3];			    /**< 顶点索引 */
	f_int32_t m_nCoordIndex[3];			    /**< 纹理坐标索引 */
} Face, *LPFace;

/** 材质信息结构体 */
typedef struct tagMatInfo
{	
	f_char_t		m_strName[255];			   /**< 纹理名称 */
	f_char_t		m_strFile[255];			   /**< 纹理文件名称 */
	f_int8_t		m_nColor[3];				   /**< 对象的RGB颜色 */		
	f_int32_t		m_nTexureId;				   /**< 纹理ID */
	f_float32_t	m_uTile;					   /**< u 重复 */
	f_float32_t	m_vTile;				       /**< v 重复 */
	f_float32_t   m_uOffset;			       /**< u 纹理偏移 */
	f_float32_t   m_vOffset;				   /**< v 纹理偏移 */

	stList_Head  stListHead;					//链表节点
} MatInfo, *LPMatInfo;

/** 对象信息结构体 */
typedef struct tagObject3D
{	
	f_int32_t  m_nNumOfVerts;		          /**< 模型中顶点的数目 */
	f_int32_t  m_nNumOfFaces;			      /**< 模型中面的数目 */
	f_int32_t  m_nNumTexVertex;			  /**< 模型中纹理坐标的数目 */
	f_int32_t  m_nMaterialID;				  /**< 纹理ID */
	BOOL     m_bHasTexture;				  /**< 是否具有纹理映射 */
	f_char_t   m_strName[255];			  /**< 对象的名称 */
	LP_PT_3F m_pVerts;				  /**< 对象的顶点 */
	LP_PT_3F m_pNormals;				  /**< 对象的法向量 */
	LP_PT_2F m_pTexVerts;				  /**< 纹理UV坐标 */
	LPFace   m_pFaces;					  /**< 对象的面信息 */

	stList_Head  stListHead;					//链表节点
}Object3D, *LPObject3D;

/** 块信息的结构 */
typedef struct tagChunk	
{	
	f_uint16_t	m_nID;					 /**< 块的ID */
	f_uint32_t	m_nLength;				 /**< 块的长度 */
	f_uint32_t	m_nBytesRead;			 /**< 已经读的块数据的字节数 */
} Chunk, *LPChunk;

/** 模型信息结构体 */
typedef struct tagModel3D
{	
	f_int32_t	m_nNumOfObjects;			  /**< 模型中对象的数目 */
	f_int32_t	m_nNumOfMaterials;			  /**< 模型中材质的数目 */
	stList_Head		m_listMaterials;			  /**< 材质链表信息 */
	stList_Head		m_listObjects;				  /**< 模型中对象链表信息 */
} Model3D, *LPModel3D;

#define MAX_TEXTURES  100

typedef struct tagModel3DS
{
	f_uint32_t   m_textures[MAX_TEXTURES];	/**< 纹理 */
	Model3D		 m_3DModel;					/**< 模型 */
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_uint32_t	 m_glLists;					/**< 显示列表*/
#else
	f_uint32_t*	 m_glLists;					/**< 显示列表*/
#endif
// 	AABB3		 m_AABB3;
} Model3DS, *LPModel3DS;

/*
 * 初始化三维模型
 * pModel: 三维模型数据
 * chFilename: 模型路径
 */
extern void Init_3DS(LPModel3DS pModel, const f_char_t* chFilename); 
/*
 * 3ds模型
 * pModel：三维模型数据
 */
extern void Draw_3DS(LPModel3DS pModel);

/*
 *	使用显示列表的方法绘制三维模型
 */
//extern void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds);
//extern void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds,PT_4D colorModel3ds);
extern void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds,PT_4D colorModel3ds, int pictureID);


/*
 * 3ds模型, 删除内存
 * pModel：三维模型数据
 */
extern void Delete_3DS(LPModel3DS pModel);

/*
 *	装载3ds文件到模型结构中
 */
BOOL Import3DS(LPModel3D p3DModel, f_char_t* strFileName);

/*
 *	创建纹理
 *	filepath: 纹理路径
 *	textureArray：返回绑定纹理数组
 *	textureID：	
 */
void LoadTexture(const f_char_t* filepath, f_uint32_t* textureArray, f_uint16_t textureID);
/*
*  LPChunk结构块赋值
*/
//void  LPChunkDefine(f_uint16_t define_m_nID,f_uint32_t define_m_nLength,f_uint32_t define_m_nBytesRead,LPChunk define_chunk);
/*
 * 读取一个块 
 */
void ReadChunk(LPChunk pChunk, FILE* pFile);		

/** 读取下一个块 */
void ReadNextChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile);

/** 读取下一个对象 */
void ReadNextObjChunk(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** 读取下一个材质块 */
void ReadNextMatChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile);	

/** 读取对象颜色的RGB值 */
void ReadColor(LPMatInfo pMaterial, LPChunk pChunk, FILE* pFile);

/** 读取对象的顶点信息 */
void ReadVertices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** 读取对象的面信息 */
void ReadVertexIndices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** 读取对象的纹理坐标 */
void ReadUVCoordinates(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** 读取赋予对象的材质 */
void ReadObjMat(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** 计算对象顶点的法向量 */
void ComputeNormals(LPModel3D pModel);	

/** 读一个字符串 */
f_int32_t  GetString(f_char_t* pBuffer, FILE* pFile);

/** 释放内存,关闭文件 */
void CleanUp(LPModel3D pModel);	

int GetNodeNum(const stList_Head *pstListHead);

#endif
