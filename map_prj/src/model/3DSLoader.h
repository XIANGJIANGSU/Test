// ����3DSģ��
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


/** �����Ƕ���һЩ���ID�� */
/**  ������(Primary Chunk)��λ���ļ��Ŀ�ʼ */
#define PRIMARY       0x4D4D

/**  ����(Main Chunks) */
#define OBJECTINFO    0x3D3D		/**< �������İ汾�� */
#define VERSION       0x0002		/**< .3ds�ļ��İ汾 */
#define EDITKEYFRAME  0xB000		/**< ���йؼ�֡��Ϣ��ͷ�� */

/**<  ����Ĵμ����� */
#define MATERIAL	  0xAFFF		/** ������Ϣ */
#define OBJECT		  0x4000		/** ������桢�������Ϣ */

/**  ���ʵĴμ����� */
#define MATNAME       0xA000		/**< �������� */
#define MATDIFFUSE    0xA020		/**< ����/���ʵ���ɫ */
#define MATMAP        0xA200		/**< �²��ʵ�ͷ�� */
#define MATMAPFILE    0xA300		/**< ����������ļ��� */
#define OBJ_MESH	  0x4100		/**< �µ�������� */


/** �������Ĵμ����� */
#define OBJ_VERTICES  0x4110		/**< ���󶥵� */
#define OBJ_FACES	  0x4120		/**< ������� */
#define OBJ_MATERIAL  0x4130		/**< ����Ĳ��� */
#define OBJ_UV		  0x4140		/**< �����UV�������� */

/** ��Ľṹ���� */
typedef struct tagFace		
{	
	f_int32_t m_nVertIndex[3];			    /**< �������� */
	f_int32_t m_nCoordIndex[3];			    /**< ������������ */
} Face, *LPFace;

/** ������Ϣ�ṹ�� */
typedef struct tagMatInfo
{	
	f_char_t		m_strName[255];			   /**< �������� */
	f_char_t		m_strFile[255];			   /**< �����ļ����� */
	f_int8_t		m_nColor[3];				   /**< �����RGB��ɫ */		
	f_int32_t		m_nTexureId;				   /**< ����ID */
	f_float32_t	m_uTile;					   /**< u �ظ� */
	f_float32_t	m_vTile;				       /**< v �ظ� */
	f_float32_t   m_uOffset;			       /**< u ����ƫ�� */
	f_float32_t   m_vOffset;				   /**< v ����ƫ�� */

	stList_Head  stListHead;					//����ڵ�
} MatInfo, *LPMatInfo;

/** ������Ϣ�ṹ�� */
typedef struct tagObject3D
{	
	f_int32_t  m_nNumOfVerts;		          /**< ģ���ж������Ŀ */
	f_int32_t  m_nNumOfFaces;			      /**< ģ���������Ŀ */
	f_int32_t  m_nNumTexVertex;			  /**< ģ���������������Ŀ */
	f_int32_t  m_nMaterialID;				  /**< ����ID */
	BOOL     m_bHasTexture;				  /**< �Ƿ��������ӳ�� */
	f_char_t   m_strName[255];			  /**< ��������� */
	LP_PT_3F m_pVerts;				  /**< ����Ķ��� */
	LP_PT_3F m_pNormals;				  /**< ����ķ����� */
	LP_PT_2F m_pTexVerts;				  /**< ����UV���� */
	LPFace   m_pFaces;					  /**< ���������Ϣ */

	stList_Head  stListHead;					//����ڵ�
}Object3D, *LPObject3D;

/** ����Ϣ�Ľṹ */
typedef struct tagChunk	
{	
	f_uint16_t	m_nID;					 /**< ���ID */
	f_uint32_t	m_nLength;				 /**< ��ĳ��� */
	f_uint32_t	m_nBytesRead;			 /**< �Ѿ����Ŀ����ݵ��ֽ��� */
} Chunk, *LPChunk;

/** ģ����Ϣ�ṹ�� */
typedef struct tagModel3D
{	
	f_int32_t	m_nNumOfObjects;			  /**< ģ���ж������Ŀ */
	f_int32_t	m_nNumOfMaterials;			  /**< ģ���в��ʵ���Ŀ */
	stList_Head		m_listMaterials;			  /**< ����������Ϣ */
	stList_Head		m_listObjects;				  /**< ģ���ж���������Ϣ */
} Model3D, *LPModel3D;

#define MAX_TEXTURES  100

typedef struct tagModel3DS
{
	f_uint32_t   m_textures[MAX_TEXTURES];	/**< ���� */
	Model3D		 m_3DModel;					/**< ģ�� */
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_uint32_t	 m_glLists;					/**< ��ʾ�б�*/
#else
	f_uint32_t*	 m_glLists;					/**< ��ʾ�б�*/
#endif
// 	AABB3		 m_AABB3;
} Model3DS, *LPModel3DS;

/*
 * ��ʼ����άģ��
 * pModel: ��άģ������
 * chFilename: ģ��·��
 */
extern void Init_3DS(LPModel3DS pModel, const f_char_t* chFilename); 
/*
 * 3dsģ��
 * pModel����άģ������
 */
extern void Draw_3DS(LPModel3DS pModel);

/*
 *	ʹ����ʾ�б�ķ���������άģ��
 */
//extern void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds);
//extern void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds,PT_4D colorModel3ds);
extern void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds,PT_4D colorModel3ds, int pictureID);


/*
 * 3dsģ��, ɾ���ڴ�
 * pModel����άģ������
 */
extern void Delete_3DS(LPModel3DS pModel);

/*
 *	װ��3ds�ļ���ģ�ͽṹ��
 */
BOOL Import3DS(LPModel3D p3DModel, f_char_t* strFileName);

/*
 *	��������
 *	filepath: ����·��
 *	textureArray�����ذ���������
 *	textureID��	
 */
void LoadTexture(const f_char_t* filepath, f_uint32_t* textureArray, f_uint16_t textureID);
/*
*  LPChunk�ṹ�鸳ֵ
*/
//void  LPChunkDefine(f_uint16_t define_m_nID,f_uint32_t define_m_nLength,f_uint32_t define_m_nBytesRead,LPChunk define_chunk);
/*
 * ��ȡһ���� 
 */
void ReadChunk(LPChunk pChunk, FILE* pFile);		

/** ��ȡ��һ���� */
void ReadNextChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile);

/** ��ȡ��һ������ */
void ReadNextObjChunk(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** ��ȡ��һ�����ʿ� */
void ReadNextMatChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile);	

/** ��ȡ������ɫ��RGBֵ */
void ReadColor(LPMatInfo pMaterial, LPChunk pChunk, FILE* pFile);

/** ��ȡ����Ķ�����Ϣ */
void ReadVertices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** ��ȡ���������Ϣ */
void ReadVertexIndices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** ��ȡ������������� */
void ReadUVCoordinates(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** ��ȡ�������Ĳ��� */
void ReadObjMat(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile);

/** ������󶥵�ķ����� */
void ComputeNormals(LPModel3D pModel);	

/** ��һ���ַ��� */
f_int32_t  GetString(f_char_t* pBuffer, FILE* pFile);

/** �ͷ��ڴ�,�ر��ļ� */
void CleanUp(LPModel3D pModel);	

int GetNodeNum(const stList_Head *pstListHead);

#endif
