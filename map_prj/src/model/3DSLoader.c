/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2014��
**
** �ļ���: 3DSLoader.c
**
** ����: ���ļ�����3DS �ļ��Ķ��뺯����3DSģ�͵Ļ��ƺ�����
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
**      2014-11-12 10:40 		LPF �޸ı���----��Ӵ�ӡ��䡣
**	  	2014-11-14 9:14   		LPF���ReadVertices�е�y��z���꽻����zȡ����
**	    2014-12-12 9:56			LPFȡ������任��䡣
**	  	2014-11-21 13:40		LPF���ò���IDΪi������Ϊ0.
**      2014-12-1   14:28		LPF��ӳ�ʼ��vSum���޸�Div_3D_32(...(f_float64_t)(shared));shared�ĸ���ȥ��
**      2014-12-10  11:50		LPFɾ���˴洢�������ݵ�bulffer��ʹ��fseek��䡣
**      2014-12-11  15:51		LPF����˶�ȡ���ݴ�С��洢�������ȵ��ж����
**		2015-5-5 19:37			LPFע����glNormal3f����
**		2015-9-22 15:36			LPF�޸Ķ�ȡ�������ĺ���,ע�͡���Χ�к�������
**-----------------------------------------------------------
*/
#include "3DSLoader.h"
//#include "../define3d/MacroDef.h"
#if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#include <unistd.h>
#endif

static LPChunk		g_pCurrentChunk = NULL;
static LPChunk		g_pTempChunk = NULL; 
//extern int red_textureID;
//extern int yellow_textureID;
BOOL CreateNormalsFile(LPModel3D pModel, f_char_t* strFileName);
BOOL ReadNormalsFile(LPModel3D pModel, f_char_t* strFileName);

/*.BH--------------------------------------------------------
**
** ������: Init_3DS
**
** ����: ��ʼ����άģ�͵ľ������ݣ�������*.3ds�ļ�
**
** �������:  pModel--ģ�;���������Ϣ�ṹ��ָ��
**           chFilename--*.3ds�ļ�ȫ·����
**
** �������: pModel--ģ�;���������Ϣ�ṹ��ָ��
**
** ����ֵ: TRUE--���سɹ�
**        FALSE--����ʧ��         
**
** ���ע��: ��
**
**.EH--------------------------------------------------------
*/
void Init_3DS(LPModel3DS pModel, const f_char_t* chFilename)
{
	//���ڽ���3ds�ļ������ַ���
	f_char_t *psz = NULL;
	f_char_t filename[256] = {0};

	f_uint16_t  j;
	
	//����ѭ����������
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;

	//ģ�Ͳ�����Ϣ�ṹ��
	LPMatInfo pMatInfo = NULL;
	f_char_t filepath[256] = {0};

	/**< Ϊ��ǰ�����ռ� */
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));
	/**< Ϊ��ʱ�����ռ� */
	g_pTempChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));				

	memset(g_pCurrentChunk, 0, sizeof(Chunk));
	memset(g_pTempChunk, 0, sizeof(Chunk));

	//��ʼ��ģ�;���������Ϣ�ṹ��ָ��ָ����ڴ�����
	memset(pModel, 0, sizeof(Model3DS));

	//��*.3ds�ļ���ȫ·�������Ƶ�filename��
	strcpy(filename, chFilename);

	//��ʼ��ģ�;��������еĶ�������Ͳ�������
	CreateList(&(pModel->m_3DModel.m_listObjects));
	CreateList(&(pModel->m_3DModel.m_listMaterials));
	
	/** ��3ds�ļ�װ�뵽ģ�;�����Ϣ�ṹ������ά��ϸģ����Ϣ�ṹ�� */
	if(!Import3DS(&pModel->m_3DModel, filename))
		return;

	//��ȡ*.3ds�ļ����ڵ�·����
	psz = strrchr(filename,'/');
	if(psz != NULL)
		*psz = 0;

	pstListHead = &(pModel->m_3DModel.m_listMaterials);	
	j = 0;

    //����ģ�͵Ĳ�����Ϣ����
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)		
	{
		if( NULL != pstTmpList )
		{
			pMatInfo = ASDE_LIST_ENTRY(pstTmpList, MatInfo, stListHead);
			/**< �ж��Ƿ���һ���ļ��� */
			if(strlen(pMatInfo->m_strFile) > 0)                 
			{	
				strncpy(filepath, filename, 256);
				strncat(filepath, "/", 1);
				strncat(filepath, pMatInfo->m_strFile, sizeof(pMatInfo->m_strFile));
				/**< ��ȡģ�Ͳ�����Ϣ��Ӧ�������ļ�(*.bmp)������������id */
				LoadTexture(filepath, pModel->m_textures, j);			
				/** ���ò��ʵ�����ID */
				pMatInfo->m_nTexureId = j;  //LPF add  2015-9-22 15:38
	//			j++;						//LPF modify
			}
			else							//LPF add
			{
				/** ���ò��ʵ�����ID */
				pMatInfo->m_nTexureId = j;
			}	
	// 		/** ���ò��ʵ�����ID */			//LPF modify
	// 		pMatInfo->m_nTexureId = j;
			j++;
//			pNode = (LPNode)pNode->m_pNext;
			
		}

	}
	//�����Χ��							//LPF modify
// 	{
// 		LPObject3D pObject = NULL;
// 		/** ����ģ�������еĶ��� */
// 		pNode = (LPNode)pModel->m_3DModel.m_listObjects.m_pFirst;
// 		while (pNode != NULL)
// 		{
// 			/** ��õ�ǰ�Ķ��� */
// 			pObject = (LPObject3D)pNode->m_pCur;
// 			/*
// 			 * ���������������
// 			 */
// 			for(i = 0; i < pObject->m_nNumOfFaces; ++i)
// 			{
// 				AddPt2AABB3(&pModel->m_AABB3, &pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[0]]);
// 				AddPt2AABB3(&pModel->m_AABB3, &pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[1]]);
// 				AddPt2AABB3(&pModel->m_AABB3, &pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[2]]);
// 			}
// 			pNode = (LPNode)pNode->m_pNext;
// 		}
// 		GetAABB3Radius(&pModel->m_AABB3);	// ���AABB��ɵİ�Χ��뾶
// 		GetAABB3Center(&pModel->m_AABB3);	// ���AABB���ĵ�����
// 	}
}



/*
 * 3dsģ��, ɾ���ڴ�
 * pModel����άģ������
 */
void Delete_3DS(LPModel3DS pModel)
{
	LPObject3D pObject = NULL;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	
	// ����
	glDeleteTextures(MAX_TEXTURES, pModel->m_textures);	
	// ��ʾ�б�
	if(pModel->m_glLists != 0)
	{
	#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
		glDeleteLists(pModel->m_glLists, pModel->m_3DModel.m_nNumOfObjects);
	#else
		//ES��esCommon����ʵ�ֵ�glDeleteLists������һ��ֻ��ɾ��1����ʾ�б�
		int i = 0, m = GetNodeNum(&pModel->m_3DModel.m_listObjects);;
		for (i = 0; i < m;i++)    //pModel->m_3DModel.m_nNumOfObjects
		    glDeleteLists(pModel->m_glLists[i], 1);
		free(pModel->m_glLists);
	#endif
	}
	// ɾ��ÿ��Object3D�е���Ϣ
//	pNode = (LPNode)pModel->m_3DModel.m_listObjects.m_pFirst;
//	while (pNode != NULL)
	pstListHead = &(pModel->m_3DModel.m_listObjects);

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)		
	{
//		pObject = (LPObject3D)pNode->m_pCur;

		if( NULL != pstTmpList )
		{
			pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);
				
			if (pObject->m_nNumOfFaces != 0)
			{
				DeleteAlterableMemory(pObject->m_pFaces);
			}
			if (pObject->m_nNumOfVerts != 0)
			{
				DeleteAlterableMemory(pObject->m_pVerts);
				DeleteAlterableMemory(pObject->m_pNormals);
			}
			if (pObject->m_nNumTexVertex != 0)
			{
				DeleteAlterableMemory(pObject->m_pTexVerts);
			}
//		pNode = (LPNode)pNode->m_pNext;		
		}
	}
}


/*.BH--------------------------------------------------------
**
** ������: Import3DS
**
** ����: ��*.3ds�ļ���ȡ����ģ����Ϣ����ά��ϸģ����Ϣ�ṹ��
**
** �������:  p3DModel--��ά��ϸģ����Ϣ�ṹ��ָ��
**           chFilename--*.3ds�ļ�ȫ·����
**
** �������: p3DModel--��ά��ϸģ����Ϣ�ṹ��ָ��
**
** ����ֵ: TRUE--���سɹ�
**        FALSE--����ʧ��         
**
** ���ע��: ��
**
**.EH--------------------------------------------------------
*/
BOOL Import3DS(LPModel3D p3DModel, f_char_t* strFileName)
{
	FILE* t_pFilePointer = NULL;
	/** ��һ��3ds�ļ� */
	t_pFilePointer = fopen(strFileName, "rb");
	/**< ����ļ�ָ�� */
	if(!t_pFilePointer)
	{	
		return FALSE;
	}
	/** ���ļ��ĵ�һ��������ж��Ƿ���3ds�ļ� */
	ReadChunk(g_pCurrentChunk, t_pFilePointer);
	/** ȷ����3ds�ļ� */
	if (g_pCurrentChunk->m_nID != PRIMARY)
	{	
		return FALSE;
	}
	/** �ݹ������������ */
	ReadNextChunk(p3DModel, g_pCurrentChunk, t_pFilePointer);


#if 0
	/** ���㶥��ķ��� */
#ifdef WIN32	
	ComputeNormals(p3DModel);   //LPF				
#else	
	ReadNormalsFile(p3DModel, strFileName);		//lpf 2017-2-16 17:02ֱ�Ӷ�ȡ���߼�¼�ļ�
#endif	
	
#if 0	
	CreateNormalsFile(p3DModel, strFileName);		//lpf 2017-2-16 17:01 ���ɷ��߼�¼�ļ�
#endif
#endif


#if	1  
	// ��ȡ����ķ����ļ���*.3dsNormal��
 	if(FALSE == ReadNormalsFile(p3DModel, strFileName))
	{
		// ��ȡʧ�ܵĻ������㷨�߲����ɷ����ļ�
		ComputeNormals(p3DModel);   //LPF				
		/** ���ɶ��󶥵�ķ�������¼�ļ� */
		CreateNormalsFile(p3DModel, strFileName);	

	}
#endif

#if 0
	{
		ReadNormalsFile(p3DModel, strFileName);
		
		//�Ƚ����ַ���
		LPNode pNode = p3DModel->m_listObjects.m_pFirst;
		LPObject3D pObject = NULL;
		int i = 0;
		while(pNode != NULL)
		{
			pObject = (LPObject3D)pNode->m_pCur;

			for(i=0; i<pObject->m_nNumOfVerts; i++)
			{
				if(
						(fabs(pObject->m_pNormals[i].x - pObject->m_pNormals2[i].x) > PRECISION)
				||(fabs(pObject->m_pNormals[i].y - pObject->m_pNormals2[i].y) > PRECISION)
				||(fabs(pObject->m_pNormals[i].z - pObject->m_pNormals2[i].z) > PRECISION))
				{
					printf("���ߴ洢����\n%f,%f\n%f,%f\n%f,%f\n",
							pObject->m_pNormals[i].x , pObject->m_pNormals2[i].x,
							pObject->m_pNormals[i].y , pObject->m_pNormals2[i].y,
							pObject->m_pNormals[i].z , pObject->m_pNormals2[i].z);
				}


			}

			// ��һ������
			pNode = (LPNode)pNode->m_pNext;
		}

	}

#endif	



	/** �ͷ��ڴ�ռ� */
	fclose(t_pFilePointer);
	CleanUp(p3DModel);
	return TRUE;
}

#include <assert.h>
#include <string.h>

#define TEXMAX		1600
struct tagTEXTUREINFO
{
	f_char_t	filename[200];
	f_uint32_t	texid;
} textures[TEXMAX] = {{0}};


/*
 *	��������
 *	filepath: ����·��
 *	textureArray�����ذ���������
 *	textureID��	
 */
void LoadTexture(const f_char_t* filepath, f_uint32_t* textureArray, f_uint16_t textureID)
{
	f_uint32_t		i;
	f_uint16_t width, height;
	f_uint32_t size;
	f_uint8_t style; 
	f_uint8_t* pImageData = NULL;


	if(!filepath)
		return;
		
	for (i=0; i<TEXMAX; ++i)
	{
		if ( 0 == textures[i].filename[0] )
			break;
		if ( strcmp(textures[i].filename, filepath) == 0 )
		{
			textureArray[textureID] = textures[i].texid;
			return;
		}
	}
	assert(i < TEXMAX);
		
	//printf("LoadTexture %s...\n", filepath);	
	strcpy(textures[i].filename, filepath);
	
#if 1

	/** ����λͼ */
	pImageData = ReadBMP(filepath, &width, &height, &size, &style);

	if(pImageData == NULL)
	{
		printf("3DSģ��-%s:����λͼʧ��!\n", filepath);
		textureArray[textureID] = 0;
		return;
	}
	glGenTextures(1, &textureArray[textureID]);
	textures[i].texid = textureArray[textureID];

	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

#if 1
	/** �����˲� */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/** �������� */
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, 
					  GL_UNSIGNED_BYTE, pImageData);
#else
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
	gluBuild2DMipmapsExt(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
#endif

#else
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);

#endif

#endif
	
	DeleteAlterableMemory(pImageData);
}

/*
 * ��ȡһ���� 
 */
void ReadChunk(LPChunk pChunk, FILE* pFile)
{
	f_uint16_t tempConvUS;
	f_uint32_t tempConvUI;
	/* ������ID�ţ�����¼�Ѷ�ȡ���ֽ��� */
	pChunk->m_nBytesRead = fread(&tempConvUS, 1, 2, pFile);
#ifdef _LITTLE_ENDIAN_
	pChunk->m_nID = tempConvUS;
#else
	pChunk->m_nID = ConvertL2B_ushort(tempConvUS);
#endif
	/** �����ռ�õĳ��ȣ�����¼�Ѷ�ȡ���ֽ��� */
	pChunk->m_nBytesRead += fread(&tempConvUI, 1, 4, pFile);
	
#ifdef _LITTLE_ENDIAN_
	pChunk->m_nLength = tempConvUI;
#else
	pChunk->m_nLength = ConvertL2B_uint(tempConvUI);
#endif
}

/** ��ȡ��һ���� */
void ReadNextChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile)
{
	/**< ������ӵ��������� */
	LPObject3D pNewObject = NULL;	
	/**< ������ӵ��������� */
	LPMatInfo  pNewTexture = NULL;	
	/**< �����ļ��汾 */
	f_uint32_t version = 0;	
	/**< ������������Ҫ������ */
	//f_int32_t buffer[50000] = {0};											
	f_uint32_t tempConvUI = 0;
	/**< Ϊ�µĿ����ռ�*/
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));			

	/** ����Ѿ���ȡ�ֽ���С���ܵ��ֽ��������������ӿ� */
	while (pPreChunk->m_nBytesRead < pPreChunk->m_nLength)
	{	
		/** ������һ���� */
		ReadChunk(g_pCurrentChunk, pFile);
		/** �жϿ��ID�� */
		switch (g_pCurrentChunk->m_nID)
		{
			/** �ļ��汾�� */
		case VERSION:							
			//��Ҫ����С��ת����unsighed int��
			/** �����ļ��İ汾�� */
			/**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
			if((g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead) <= sizeof(tempConvUI))
			{
				g_pCurrentChunk->m_nBytesRead += fread(&tempConvUI, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
#ifdef _LITTLE_ENDIAN_
				version = tempConvUI;
#else
				version = ConvertL2B_int(tempConvUI);
#endif
				/** ����ļ��汾�Ŵ���3������һ��������Ϣ */
				if (version > 0x03)
				{
					//throw(" ��3DS�ļ��汾����3.0,���ܲ�����ȷ��ȡ");
					printf("��3DS�ļ��汾����3.0,���ܲ�����ȷ��ȡ!\n");
				}
			}
			else
			{
				//throw("�ļ�������");
				printf("�ļ�������!\n");
			}
			break;

			/** ����汾��Ϣ */
		case OBJECTINFO:						
			/** ������һ���� */
			ReadChunk(g_pTempChunk, pFile);
			//��Ҫ����С��ת�� (��λint)
			/** �������İ汾�� */
			/**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
			if((g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead) <= sizeof(tempConvUI))
			{
				g_pTempChunk->m_nBytesRead += fread(&tempConvUI, 1, g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead, pFile);
#ifdef _LITTLE_ENDIAN_
				version = tempConvUI;
#else
				version = ConvertL2B_uint(tempConvUI);
#endif
				/** ���Ӷ�����ֽ��� */
				g_pCurrentChunk->m_nBytesRead += g_pTempChunk->m_nBytesRead;
				/** �ݹ������һ���� */
				ReadNextChunk(pModel, g_pCurrentChunk, pFile);
			}
			else
			{
				//throw("�ļ�������");
				printf("�ļ�������!\n");
			}
			break;

			/** ������Ϣ */
		case MATERIAL:							
			/** ���ʵ���Ŀ���� */
			pModel->m_nNumOfMaterials++;
			/** ���������������һ���հ�����ṹ */
		    {
				LPMatInfo  pNewTexture = NewFixedMemory(sizeof(MatInfo));	
				
		    	/**�ҵ�ͷ���֮��**/
				stList_Head *pstListHead = NULL;
				pstListHead = &(pModel->m_listMaterials);
				LIST_ADD(&pNewTexture->stListHead, pstListHead);	

				/** �������װ�뺯�� */
				ReadNextMatChunk(pModel, g_pCurrentChunk, pFile);	
		    }
			break;
			
			/** �������� */
		case OBJECT:							
			/** ���������� */
			pModel->m_nNumOfObjects++;
			/** ���һ���µ�tObject�ڵ㵽���������� */
			{
				LPObject3D pNewObject = NewFixedMemory(sizeof(Object3D));	
				
		    	/**�ҵ�ͷ���֮��**/
				stList_Head *pstListHead = NULL;
				pstListHead = &(pModel->m_listObjects);
				LIST_ADD(&pNewObject->stListHead, pstListHead);	

				/*
				 * ��ò������������ƣ�Ȼ�����Ӷ�����ֽ���
				 */
				g_pCurrentChunk->m_nBytesRead += GetString(pNewObject->m_strName, pFile);
				/** ��������Ķ�����Ϣ�Ķ��� */
				ReadNextObjChunk(pModel, pNewObject, g_pCurrentChunk, pFile);
			}
			break;

			/** �ؼ�֡ */
		//case EDITKEYFRAME:
			//��Ҫ����С��ת�� ��unsigned int��
			/** �����ؼ�֡��Ķ��� */
			//g_pCurrentChunk->m_nBytesRead += fread(buffer, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			//break;

		default: 
			//��Ҫ����С��ת�� (unsigned int)
			/**  �������к��ԵĿ�����ݵĶ��룬�����ؼ�֡��*/
			fseek( pFile, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead,SEEK_CUR);
			g_pCurrentChunk->m_nBytesRead += (g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead);
			break;
		}
		/** ���Ӵ����������ֽ��� */
		pPreChunk->m_nBytesRead += g_pCurrentChunk->m_nBytesRead;
	}

	/** �ͷŵ�ǰ����ڴ�ռ� */
	DeleteAlterableMemory(g_pCurrentChunk);
	g_pCurrentChunk = pPreChunk;
}

/** �������е��ļ��ж������Ϣ */
void ReadNextObjChunk(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	//f_int32_t buffer[50000] = {0};					/** ���ڶ��벻��Ҫ������ */
	/** ���µĿ����洢�ռ� */
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));
	/*
	 * ��������������ֱ�����ӿ���� 
	 */
	while(pPreChunk->m_nBytesRead < pPreChunk->m_nLength)
	{	
		/** ������һ���� */
		ReadChunk(g_pCurrentChunk, pFile);

		/** ������������ֿ� */
		switch (g_pCurrentChunk->m_nID)
		{
		/*
		 *< ���������һ���¿� 
		 */
		case OBJ_MESH:					
			/** ʹ�õݹ麯�����ã�������¿� */
			ReadNextObjChunk(pModel, pObject, g_pCurrentChunk, pFile);
			break;
		case OBJ_VERTICES:				/**< �����Ƕ��󶥵� */
			ReadVertices(pObject, g_pCurrentChunk, pFile);
			break;
		case OBJ_FACES:					/**< ������Ƕ������ */
			ReadVertexIndices(pObject, g_pCurrentChunk, pFile);
			break;
		case OBJ_MATERIAL:				/**< ������Ƕ���Ĳ������� */
			/** �������Ĳ������� */
			ReadObjMat(pModel, pObject, g_pCurrentChunk, pFile);		
			break;
		case OBJ_UV:						/**< ��������UV�������� */
			/** ��������UV�������� */
			ReadUVCoordinates(pObject, g_pCurrentChunk, pFile);
			break;
		default:  
			//��Ҫ����С��ת�� (unsigned int)
			/** �Թ�����Ҫ����Ŀ� */
			//g_pCurrentChunk->m_nBytesRead += fread(buffer, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			fseek( pFile, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead,SEEK_CUR);
			g_pCurrentChunk->m_nBytesRead += (g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead);
			break;
		}
		/** ��Ӵ������ж�����ֽ�����ǰ��Ķ�����ֽ��� */
		pPreChunk->m_nBytesRead += g_pCurrentChunk->m_nBytesRead;
	}
	/** �ͷŵ�ǰ����ڴ�ռ䣬���ѵ�ǰ������Ϊǰ��� */
	DeleteAlterableMemory(g_pCurrentChunk);
	g_pCurrentChunk = pPreChunk;
}

/** ��ȡ��һ�����ʿ� */
void ReadNextMatChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile)
{
	//f_int32_t buffer[50000] = {0};					/**< ���ڶ��벻��Ҫ������ */
	LPMatInfo pModelMatNow = ASDE_LIST_ENTRY(pModel->m_listMaterials.pNext, MatInfo, stListHead);	/*��ǰ����Ĳ����ڴ��*/
	
	/** ����ǰ�����洢�ռ� */
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));
	/** ����������Щ�� */
	while (pPreChunk->m_nBytesRead < pPreChunk->m_nLength)
	{	
		/*
		 * ������һ��
		 */
		ReadChunk(g_pCurrentChunk, pFile);
		/** �ж϶������ʲô�� */
		switch (g_pCurrentChunk->m_nID)
		{
		case MATNAME:							/**< ���ʵ����� */
			/** ������ʵ����� */
                     	/**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
			if((g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead) <=  256)
			{
	
//				g_pCurrentChunk->m_nBytesRead += fread(((LPMatInfo)(pModel->m_listMaterials.m_pLast->m_pCur))->m_strName, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
				g_pCurrentChunk->m_nBytesRead += 
					fread(pModelMatNow->m_strName, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			
			}
			else
			{
				printf("�ļ�������!\n");
				//throw("�ļ�������");
			}
			break;
		case MATDIFFUSE:						/**< �����R G B��ɫ */
//			ReadColor((LPMatInfo)(pModel->m_listMaterials.m_pLast->m_pCur), g_pCurrentChunk, pFile);
			ReadColor(pModelMatNow, g_pCurrentChunk, pFile);
			
			break;
		case MATMAP:							/**< ������Ϣ��ͷ�� */
			/** ��һ�����ʿ���Ϣ */
			ReadNextMatChunk(pModel, g_pCurrentChunk, pFile);
			break;
		case MATMAPFILE:						/**< �����ļ������� */
			/** ������ʵ��ļ����� */
                     	/**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
			if((g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead) <=  256)
			{
//				g_pCurrentChunk->m_nBytesRead += fread(((LPMatInfo)(pModel->m_listMaterials.m_pLast->m_pCur))->m_strFile, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
				g_pCurrentChunk->m_nBytesRead += fread(((LPMatInfo)(pModelMatNow))->m_strFile, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);

			}
			else
			{
				//throw("�ļ�������");
				printf("�ļ�������!\n");
			}
			break;
		default:  
			/** ��������Ҫ����Ŀ� */
			//g_pCurrentChunk->m_nBytesRead += fread(buffer, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			fseek( pFile, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead,SEEK_CUR);
			g_pCurrentChunk->m_nBytesRead += (g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead);
			break;
		}
		/** ��Ӵ������ж�����ֽ��� */
		pPreChunk->m_nBytesRead += g_pCurrentChunk->m_nBytesRead;
	}
	/** ɾ����ǰ�飬������ǰ������Ϊǰ��Ŀ� */
	DeleteAlterableMemory(g_pCurrentChunk);
	g_pCurrentChunk = pPreChunk;
}
/** ��ȡ������ɫ��RGBֵ */
void ReadColor(LPMatInfo pMaterial, LPChunk pChunk, FILE* pFile)
{
	/** ������ɫ����Ϣ */
	ReadChunk(g_pTempChunk, pFile);
	/** ����RGB��ɫ */
       /**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
	if((g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead) <= sizeof(pMaterial->m_nColor))
	{
		g_pTempChunk->m_nBytesRead += fread(pMaterial->m_nColor, 1, g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead, pFile);
		/** ���Ӷ�����ֽ��� */
		pChunk->m_nBytesRead += g_pTempChunk->m_nBytesRead;
	}
	else
	{
		//throw("��ɫ�����")	;
		printf("��ɫ�����!\n");
	}
	/** ���Ӷ�����ֽ��� */
	pChunk->m_nBytesRead += g_pTempChunk->m_nBytesRead;
}


/** ��ȡ����Ķ�����Ϣ */
void ReadVertices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_uint16_t tempConvUS = 0;

	/** ���붥�����Ŀ */
	pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, 2, pFile);
	
#ifdef _LITTLE_ENDIAN_
	pObject->m_nNumOfVerts = tempConvUS;
#else
	pObject->m_nNumOfVerts = ConvertL2B_ushort(tempConvUS);
#endif

	/** ���䶥��Ĵ洢�ռ䣬Ȼ���ʼ���ṹ�� */
	pObject->m_pVerts = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfVerts);
	/** ���붥������ */
    /**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
	if((pPreChunk->m_nLength - pPreChunk->m_nBytesRead) <= sizeof(PT_3F) * pObject->m_nNumOfVerts)
	{

//Ƕ��ʽ����Ҫ�ȶ�ȡchar���飬�ٴ�С��ת��, lpf add 2017-4-27 10:53:11
#ifndef _LITTLE_ENDIAN_

		{

			int i =0;
			PT_3F temp_convert = {0};
			float * pt = (float *)&pObject->m_pVerts[0];
			char read_convert[4] = {0};
			
			//��С��ת��
			for(i=0; i< pObject->m_nNumOfVerts * 3; i++)
			{
				//pc�²���ֱ�Ӷ�ȡ��˵�float����
				if (fread(read_convert, sizeof(float) ,1,pFile) != 1)
				{

					printf("the model verterts file read error!\n");
					fclose(pFile);	
					return ;
				}

				//��С��ת��
				{
					char temp_store;
					int i = 0;
				
					temp_store =	read_convert[0 + i];
					read_convert[0 + i] = read_convert[3 + i];
					read_convert[3 + i] = temp_store;

					temp_store =	read_convert[1 + i];
					read_convert[1 + i] = read_convert[2 + i];
					read_convert[2 + i] = temp_store;		

				}
				
				*pt = *(float *)read_convert;

				pt++;


//				temp_convert.x = ConvertL2B_float(pObject->m_pNormals2[i].x);
//				temp_convert.y = ConvertL2B_float(pObject->m_pNormals2[i].y);
//				temp_convert.z = ConvertL2B_float(pObject->m_pNormals2[i].z);

//				SetValue_3D_32(&pObject->m_pNormals2[i], &temp_convert);	

			}

			pt  = NULL;
		}


		pPreChunk->m_nBytesRead += ( pPreChunk->m_nLength - pPreChunk->m_nBytesRead);

#else
		//PC ��ֱ�Ӷ�ȡ
		pPreChunk->m_nBytesRead += fread(pObject->m_pVerts, 1, pPreChunk->m_nLength - pPreChunk->m_nBytesRead, pFile);

#endif
	}
	else
	{
		//throw("���붨�����г���");
		printf("���붥�����г���!\n");
	}
}

/** ��ȡ���������Ϣ */
void ReadVertexIndices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_uint16_t tempConvUS = 0;
	/*
	 *< ���ڶ��뵱ǰ������� 
	 */
	f_uint16_t index = 0;		
	f_int32_t i, j;
	/** ����ö����������Ŀ */
	pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, 2, pFile);
#ifdef _LITTLE_ENDIAN_
	pObject->m_nNumOfFaces = tempConvUS;
#else
	pObject->m_nNumOfFaces = ConvertL2B_ushort(tempConvUS);
#endif
	/*
	 * ����������Ĵ洢�ռ䣬����ʼ���ṹ
	 */
	pObject->m_pFaces = (LPFace)NewAlterableMemory(sizeof(Face) * pObject->m_nNumOfFaces);
	/** �������������е��� */
	for(i = 0; i < pObject->m_nNumOfFaces; ++i)
	{	
		for(j = 0; j < 4; ++j)
		{	
			/** ���뵱ǰ��ĵ�һ����  */
			pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, sizeof(f_uint16_t), pFile);
#ifdef _LITTLE_ENDIAN_
			index = tempConvUS;
#else
			index = ConvertL2B_ushort(tempConvUS);
#endif
			if(j < 3)
			{	
				/*
				 * ��������������Ľṹ��
				 */
				pObject->m_pFaces[i].m_nVertIndex[j] = index;
			}
		}
	}
}

/*
 * ��ȡ������������� 
 */
void ReadUVCoordinates(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_uint16_t tempConvUS = 0;
//	f_int32_t i;
	/** ����UV��������� */
	pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, 2, pFile);
#ifdef _LITTLE_ENDIAN_
	pObject->m_nNumTexVertex = tempConvUS;
#else
	pObject->m_nNumTexVertex = ConvertL2B_ushort(tempConvUS);
#endif

	/** ���䱣��UV������ڴ�ռ� */
	pObject->m_pTexVerts = (LP_PT_2F)NewAlterableMemory(sizeof(PT_2F) * pObject->m_nNumTexVertex);
	/** ������������ */
       /**��Ҫ�Զ�������ݴ�С�����жϣ��Ƿ���洢������� */
	if((pPreChunk->m_nLength - pPreChunk->m_nBytesRead) <= sizeof(PT_2F) * pObject->m_nNumTexVertex)
	{



#if 0		//lpf delete 2017-4-27 10:59:23
	
		pPreChunk->m_nBytesRead += fread(pObject->m_pTexVerts, 1, pPreChunk->m_nLength - pPreChunk->m_nBytesRead, pFile);
#ifndef _LITTLE_ENDIAN_
		for(i = 0; i < pObject->m_nNumTexVertex; ++i)
		{
			pObject->m_pTexVerts[i].x = ConvertL2B_float(pObject->m_pTexVerts[i].x);
			pObject->m_pTexVerts[i].y = ConvertL2B_float(pObject->m_pTexVerts[i].y);
		}
#endif

#else








#ifndef _LITTLE_ENDIAN_
//Ƕ��ʽ����Ҫ�ȶ�ȡchar���飬�ٴ�С��ת��, lpf add 2017-4-27 10:53:11
		{
			// PC�¶�ȡ������ݣ���Ҫ��ȡ����д�С��ת��

			int i =0;
			PT_3F temp_convert = {0};
			float * pt = (float *)&pObject->m_pTexVerts[0];
			float		temp_pt = 0.0f;
			char read_convert[4] = {0};
			
			//��С��ת��
			for(i=0; i< pObject->m_nNumTexVertex * 2; i++)
			{
				//pc�²���ֱ�Ӷ�ȡ��˵�float����
				if (fread(read_convert, sizeof(float) ,1,pFile) != 1)
				{

					printf("the model normal file read error!\n");
					fclose(pFile);	
					return ;
				}

				//��С��ת��
				{
					char temp_store;
					int i = 0;
				
					temp_store =	read_convert[0 + i];
					read_convert[0 + i] = read_convert[3 + i];
					read_convert[3 + i] = temp_store;

					temp_store =	read_convert[1 + i];
					read_convert[1 + i] = read_convert[2 + i];
					read_convert[2 + i] = temp_store;		

				}
				
				*pt = *(float *)read_convert;

				pt++;

			}

			pt  = NULL;
		}



		pPreChunk->m_nBytesRead += (pPreChunk->m_nLength - pPreChunk->m_nBytesRead);












#else
//PC ��ֱ�Ӷ�ȡ

		pPreChunk->m_nBytesRead += fread(pObject->m_pTexVerts, 1, pPreChunk->m_nLength - pPreChunk->m_nBytesRead, pFile);



#endif




#endif













	}
	else
	{
		//throw("��������鱨��");
		printf("��������鱨��!\n");
	}
}

/*
 * ��ȡ�������Ĳ��� 
 */
void ReadObjMat(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_char_t strMaterial[255] = {0};			/**< �����������Ĳ������� */
	//f_int32_t buffer[50000] = {0};				/**< �������벻��Ҫ������ */
	f_uint32_t i;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	LPMatInfo pMatInfo = NULL;
	/** ���븳�赱ǰ����Ĳ������� */
	pPreChunk->m_nBytesRead += GetString(strMaterial, pFile);
	/*
	 * �������е����� 
	 */
	//for(i = 0; i < pModel->m_nNumOfMaterials; ++i)
	//{	
//		pNode = pModel->m_listMaterials.m_pFirst;
		pstListHead = &(pModel->m_listMaterials);	
		i = 0;
//		while((pNode != NULL)&&(i<pModel->m_nNumOfMaterials))

		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
		{

			if(i >= (f_uint32_t)pModel->m_nNumOfMaterials)
			{
				break;
			}

//			pMatInfo = (LPMatInfo)pNode->m_pCur;			
			if( NULL != pstTmpList )
			{
				pMatInfo = ASDE_LIST_ENTRY(pstTmpList, MatInfo, stListHead);

				/*
				 * �������������뵱ǰ����������ƥ�� 
				 */
				if(strncmp(strMaterial, pMatInfo->m_strName, 255) == 0)
				{	
					/** ���ò���ID */
					pObject->m_nMaterialID = i;	//2015-3-30 13:46  LPF
					//pObject->m_nMaterialID = 0;
					/** �ж��Ƿ�������ӳ�� */
					if(strlen(pMatInfo->m_strFile) > 0) 
					{
						/** ���ö��������ӳ���־ */
						pObject->m_bHasTexture = TRUE;
					}	
					break;
				}
				else
				{	
					/** ����ö���û�в��ʣ�������IDΪ-1 */
					pObject->m_nMaterialID = -1;
				
				}
				// next
//				pNode = (LPNode)pNode->m_pNext;
				i++;
			
			}

		}
	//}
	//pPreChunk->m_nBytesRead += fread(buffer, 1, pPreChunk->m_nLength - pPreChunk->m_nBytesRead, pFile);
	fseek( pFile,pPreChunk->m_nLength - pPreChunk->m_nBytesRead,SEEK_CUR);
	pPreChunk->m_nBytesRead += (pPreChunk->m_nLength - pPreChunk->m_nBytesRead);
}

/** ������󶥵�ķ����� */
void ComputeNormals(LPModel3D pModel)
{
	PT_3F vVector1, vVector2, vNormal, vPoly[3];
	f_int32_t i, j;
	LPObject3D pObject = NULL;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	LP_PT_3F pTempNormals = NULL;
	LP_PT_3F pNormals = NULL;
	PT_3F vSum;
	f_int32_t shared = 0;
	//��ʼ��vSum
	vector3FMemSet(&vSum, 0, 0, 0);	
	/** ���ģ����û�ж����򷵻� */
	if(pModel->m_nNumOfObjects <= 0)
		return;
	/** ����ģ�������еĶ��� */
	//for(index = 0; index < pModel->m_nNumOfObjects; ++index)
	//{	
//		pNode = (LPNode)pModel->m_listObjects.m_pFirst;
		pstListHead = &(pModel->m_listObjects);
//		while (pNode != NULL)

		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
		{
			/** ��õ�ǰ�Ķ��� */
//			pObject = (LPObject3D)pNode->m_pCur;
			if( NULL != pstTmpList )
			{
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);

				/** ������Ҫ�Ĵ洢�ռ� */
				pNormals = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfFaces);
				pTempNormals = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfFaces);
				pObject->m_pNormals	= (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfVerts);
				/*
				 * ���������������
				 */
				for(i = 0; i < pObject->m_nNumOfFaces; ++i)
				{
					vPoly[0] = pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[0]];
					vPoly[1] = pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[1]];
					vPoly[2] = pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[2]];

					//m_AABB3.add(vPoly[0]);
					//m_AABB3.add(vPoly[1]);
					//m_AABB3.add(vPoly[2]);

					/** ������ķ����� */
					//vVector1 = vPoly[0] - vPoly[2];		        /**< ��ö���ε�ʸ�� */
					vector3FSub(&vVector1, &vPoly[0], &vPoly[2]);
					//vVector2 = vPoly[2] - vPoly[1];		        /**< ��ö���εĵڶ���ʸ�� */
					vector3FSub(&vVector2, &vPoly[2], &vPoly[1]);
					//vNormal  = vVector1.crossProduct(vVector2);	/**< ��������ʸ���Ĳ�� */
					vector3FCrossProduct(&vNormal, &vVector1, &vVector2);
					//pTempNormals[i] = vNormal;
					vector3FSetValue(&pTempNormals[i], &vNormal);
					//vNormal  = vNormal.normalize();					/**< ��һ����� */
					vector3FNormalize(&vNormal);
					//pNormals[i] = vNormal;							/**< ����������ӵ��������б��� */
					vector3FSetValue(&pNormals[i], &vNormal);
				}
				/** ���㶥�㷨���� */
				
				//Vector3 vZero = vSum;
				/** �������еĶ��� */
				for (i = 0; i < pObject->m_nNumOfVerts; ++i)			
				{	
					for (j = 0; j < pObject->m_nNumOfFaces; ++j)	/**< �������е��������� */
					{													/**< �жϸõ��Ƿ����������湲�� */
						if (pObject->m_pFaces[j].m_nVertIndex[0] == i || 
							pObject->m_pFaces[j].m_nVertIndex[1] == i || 
							pObject->m_pFaces[j].m_nVertIndex[2] == i)
						{	
							//vSum = vSum + pTempNormals[j];
							vector3FAdd(&vSum, &vSum, &pTempNormals[j]);
							shared++;								
						}
					}      
					//pObject->m_pNormals[i] = vSum / float(-shared);
					vector3FDiv(&pObject->m_pNormals[i], &vSum, (f_float64_t)(shared));
					/** ��һ�����㷨�� */
					//pObject->m_pNormals[i] = pObject->m_pNormals[i].normalize();
					vector3FNormalize(&pObject->m_pNormals[i]);
					vector3FMemSet(&vSum, 0, 0, 0);							
					shared = 0;										
				}
				/** �ͷŴ洢�ռ䣬��ʼ��һ������ */
				DeleteAlterableMemory(pTempNormals);
				DeleteAlterableMemory(pNormals);
//				pNode = (LPNode)pNode->m_pNext;


				
			
			}




			

		}
	//}
}

/** ��һ���ַ��� */
f_int32_t  GetString(f_char_t* pBuffer, FILE* pFile)
{
	f_int32_t index = 0;
	/** ����һ���ֽڵ����� */
	fread(pBuffer, 1, 1, pFile);
	/** ֱ������ */
	while(*(pBuffer + index++) != 0) 
	{
		/** ����һ���ַ�ֱ��NULL */
		fread(pBuffer + index, 1, 1, pFile);
	}
	/** �����ַ����ĳ��� */
	return strlen(pBuffer) + 1;
}

/** �ͷ��ڴ�,�ر��ļ� */
void CleanUp(LPModel3D pModel)
{
	
	//delete m_CurrentChunk;		     /**< �ͷŵ�ǰ�� */
	DeleteAlterableMemory(g_pCurrentChunk);
	//delete m_TempChunk;				 /**< �ͷ���ʱ�� */
	DeleteAlterableMemory(g_pTempChunk);
}

//ͨ�������ѯ����ṹ�壬��������ͷ������ID���������ṹ��
void* MatNodeAt(const stList_Head *pstListHead ,const f_uint16_t index)
{
	stList_Head *pstTmpList = NULL;
	LPMatInfo pNode = NULL;

	if(LIST_IS_EMPTY(pstListHead))
	{
		return NULL;
	}


	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pNode = ASDE_LIST_ENTRY(pstTmpList, MatInfo, stListHead);
			
			if(index == pNode->m_nTexureId)
			{
				return pNode;
			}			
		}
	}	
	return NULL;	
}

//��ȡ����Ľڵ����������ÿ���ڵ��Ƿ�Ϊ���ж�
int GetNodeNum(const stList_Head *pstListHead)
{
	stList_Head *pstTmpList = NULL;
	LPMatInfo pNode = NULL;
	int i = 0;
	
	//ȷ�������ͷ�ڵ�ǿ�
	if(LIST_IS_EMPTY(pstListHead))
	{
		return 0;
	}
	
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{

		if(LIST_IS_EMPTY(pstTmpList))
		{
			break;
		}

		i ++;	
	}	
	
	return i;	
}
/*
 * 3dsģ��
 * pModel����άģ������
 */
void Draw_3DS(LPModel3DS pModel)
{
	f_int32_t j, index, tex;
	LPObject3D pObject = NULL;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	LPMatInfo pMatInfo = NULL;
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glPushAttrib(GL_CURRENT_BIT);									/**< ����������ɫ��ʵ�� */
#endif
	/**< ����ģ�������еĶ��� */
//	pNode = pModel->m_3DModel.m_listObjects.m_pFirst;
	pstListHead = &(pModel->m_3DModel.m_listObjects);	
//	while (pNode != NULL)

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
	{
//		pObject = (LPObject3D)pNode->m_pCur;
		
			if( NULL != pstTmpList )
			{
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);
				
				if (pObject->m_bHasTexture)									/**< �жϸö����Ƿ�������ӳ�� */
				{
					glBindTexture(GL_TEXTURE_2D, pModel->m_textures[pObject->m_nMaterialID]);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				/** ��ʼ���� */
				glBegin(GL_TRIANGLES);
				for(j = 0; j < pObject->m_nNumOfFaces; ++j)					/**< �������е��� */
				{
					for(tex = 0; tex < 3; tex++)							/**< ���������ε����е� */
					{
						index = pObject->m_pFaces[j].m_nVertIndex[tex];		/**< ������ÿ��������� */
		 				glNormal3f(	pObject->m_pNormals[index].x,
		 							pObject->m_pNormals[index].y,  
		 							pObject->m_pNormals[index].z );			/**< ���������� */
						//printf("pObject->m_pNormals[index].x %f\n",pObject->m_pNormals[index].x);
						if(pObject->m_bHasTexture)							/**< �������������� */
						{	
							if(pObject->m_pTexVerts)						/**< ȷ���Ƿ���UVW�������� */
							{
								glTexCoord2f(pObject->m_pTexVerts[index].x,pObject->m_pTexVerts[index].y);
							}
						}
						else												/**< ���û���������������ɫ */
						{	
							if(pModel->m_3DModel.m_nNumOfMaterials && pObject->m_nMaterialID >= 0) 
							{	
//								pMatInfo = (LPMatInfo)(((LPNode)NodeAt(&pModel->m_3DModel.m_listMaterials, pObject->m_nMaterialID))->m_pCur);
								pMatInfo = (LPMatInfo)MatNodeAt(&pModel->m_3DModel.m_listMaterials, (f_uint16_t)pObject->m_nMaterialID);

								glColor3ub(	pMatInfo->m_nColor[0],
											pMatInfo->m_nColor[1],
											pMatInfo->m_nColor[2]);
							}
						}
						glVertex3f(	pObject->m_pVerts[index].x,
									pObject->m_pVerts[index].y,
									pObject->m_pVerts[index].z );
					}
				}
				glEnd(); /**< ���ƽ��� */
//				pNode = (LPNode)pNode->m_pNext;
				
			
			}


		

	}
	glPopAttrib();   /**< �ָ�ǰһ���� */
}

/*
 *	ʹ����ʾ�б�ķ���������άģ��
 *  displaytype3ds  0��������ʾ��1��������2���߿�(������С,��ɫĬ��Ϊ��ɫ)��
 *	3��ģ��+�߿�(1.02����С,��ɫĬ��Ϊ��ɫ)��4��ģ��+������(������С,�ߴ�3��,��ɫĬ��Ϊ��ɫ)��
 *  5���⻷Ч��,��ģ��+��ɫ(1.05����С,��ɫĬ��Ϊ���ɫ)��6����ɫ����(��ɫĬ��Ϊ��ɫ)
 */
void Draw_3DS_GLList(LPModel3DS pModel, int displaytype3ds,PT_4D colorModel3ds, int pictureID)
{
	f_int32_t i, j, index, tex;
	LPObject3D pObject = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;		
	LPMatInfo pMatInfo = NULL;
	f_int32_t object_size = 0;			// object size in list 
	
	f_float32_t mat_color[4] = {0.0f, 0.0f, 0.0f, 0.0f}; 
	//�ⲿ�������ɫ�����
	mat_color[0] = colorModel3ds.x;
	mat_color[1] = colorModel3ds.y;
	mat_color[2] = colorModel3ds.z;
	mat_color[3] = colorModel3ds.w;
	
//	f_float32_t mat_amb[] = {0.2,0.2,0.2,1.0};			//default
//	f_float32_t mat_diff[] = {0.8,0.8,0.8,1.0};			//default
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glPushAttrib(GL_CURRENT_BIT);									/**< ����������ɫ��ʵ�� */
#endif
	//�߿�ģʽ
	if(displaytype3ds == 2)	
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//Ϊģ�͵�ÿ�����󴴽�һ����ʾ�б�
	if(pModel->m_glLists == 0)
	{
		if(!LIST_IS_EMPTY(&pModel->m_3DModel.m_listObjects))	
		{
			// ���ɷ���,��������Ῠ��
	//		printf("���ɷ���...");
	//		ComputeNormals(&pModel->m_3DModel);
	//		printf("���ɷ��߳ɹ�\n");

			// Ϊÿ��ģ�Ͷ���������ʾ�б�
			object_size = GetNodeNum(&pModel->m_3DModel.m_listObjects);
		#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
			pModel->m_glLists = glGenLists(object_size);
		#else
			pModel->m_glLists = malloc(object_size*sizeof(f_uint32_t));
			memset(pModel->m_glLists, 0, object_size*sizeof(f_uint32_t));
			for (i = 0; i < object_size;i++)
			    pModel->m_glLists[i] = glGenLists(1);
		#endif
			/**< ����ģ�������еĶ��� */
			pstListHead = &(pModel->m_3DModel.m_listObjects);	
			i = 0;
			{
				LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
				{
					if( NULL != pstTmpList )
					{
						pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);
					#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
						glNewList(pModel->m_glLists + i, GL_COMPILE);
					#else
						glNewList(pModel->m_glLists[i], GL_COMPILE);
					#endif
						/** ��ʼ���� */
						glBegin(GL_TRIANGLES);
						for(j = 0; j < pObject->m_nNumOfFaces; ++j)					/**< �����������е��� */
						{
							for(tex = 0; tex < 3; tex++)							/**< ���������ε����е� */
							{
								index = pObject->m_pFaces[j].m_nVertIndex[tex];		/**< ������ÿ��������� */
								glNormal3f(	pObject->m_pNormals[index].x,
				 							pObject->m_pNormals[index].y,  
				 							pObject->m_pNormals[index].z );			/**< ���������� */					
								//printf("pObject->m_pNormals[index].x %f\n",pObject->m_pNormals[index].x);
								if(pObject->m_bHasTexture)							/**< �������������� */
								{	
									if(pObject->m_pTexVerts)						/**< ȷ���Ƿ���UVW�������� */
									{
										glTexCoord2f(pObject->m_pTexVerts[index].x,pObject->m_pTexVerts[index].y);
									}
								}
								else												/**< ���û���������������ɫ */
								{	
								}
								glVertex3f(	pObject->m_pVerts[index].x,             /**< �������󶥵� */	
											pObject->m_pVerts[index].y,
											pObject->m_pVerts[index].z );
							}
						}
						glEnd();	
						glEndList();
	
						i++;
					}
				}		
			}
		}
	}

	/**< ����ģ�������еĶ���,���ݲ�ͬ��ʾģʽ������,������ʾ�б����ģ�� */
	pstListHead = &(pModel->m_3DModel.m_listObjects);		
	i = 0;
	{
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)		
		{
			if( NULL != pstTmpList )
			{
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);

				if(displaytype3ds == 2)				//�߿�ģʽ,�����߿���ɫΪ�ⲿ�������ɫ
				{
					glBindTexture(GL_TEXTURE_2D, 0);
					glColor4fv(mat_color);			
				}		
				else if(displaytype3ds == 5)			//�⻷ģʽ�£���ʹ������
				{
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				else if(displaytype3ds == 6)			//��ɫģʽ,ʹ�ô�ɫ��ͼ
				{
//		//			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_color);
					glBindTexture(GL_TEXTURE_2D, pictureID);
			
				}
				else								//����ģʽ
				{			
					if (pObject->m_bHasTexture)			/**< �жϸö����Ƿ�������ӳ�� */
					{
						/*������������*/
						glBindTexture(GL_TEXTURE_2D, pModel->m_textures[pObject->m_nMaterialID]);
					}
					else
					{
						/*���û�У����ж�ģ�Ͷ����Ƿ��в�����Ϣ���������ʹ�ò��ʵ���ɫ*/
						glBindTexture(GL_TEXTURE_2D, 0);
						if(pModel->m_3DModel.m_nNumOfMaterials && pObject->m_nMaterialID >= 0) 
						{	
	//						pMatInfo = (LPMatInfo)(((LPNode)NodeAt(&pModel->m_3DModel.m_listMaterials, pObject->m_nMaterialID))->m_pCur);
							pMatInfo = (LPMatInfo)MatNodeAt(&pModel->m_3DModel.m_listMaterials, (f_uint16_t)pObject->m_nMaterialID);

							//���ư����ߵ�ʱ��ʹ�ò��ʵ���ɫ
							if(displaytype3ds != 4)
							{
								glColor3ub(pMatInfo->m_nColor[0],pMatInfo->m_nColor[1],pMatInfo->m_nColor[2]);
							}
						}
					}
				}

				//������ʾ�б�
			#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
				glCallList(pModel->m_glLists + i);
			#else
				glCallList(pModel->m_glLists[i]);
			#endif

				i++;
			}
		}	

	
	}
	
	//�ָ��߿�ģʽ
	if(displaytype3ds == 2)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//��ɫģʽ,ȡ��֮ǰ�������
	if(displaytype3ds == 6)		
	{
//		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_amb);
//		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diff);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glPopAttrib();   /**< �ָ�ǰһ���� */
}

/** ���ɶ��󶥵�ķ�������¼�ļ� */
//lpf 2017-2-16 16:44 
BOOL CreateNormalsFile(LPModel3D pModel, f_char_t* strFileName)
{
	LPObject3D pObject = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	char NormalFile[1024] = {0};
	FILE* fp = NULL;
	char Flag_begin[8] = "AABB";
	char Flag_end[8] = "BBAA";
	BOOL ret = FALSE;
	int j;
	
	// 0./** ���ģ����û�ж����򷵻� */
	if(pModel->m_nNumOfObjects <= 0)
		return FALSE;
	
	// 1.�����ļ�
	memcpy(NormalFile, strFileName, 1024);	
#ifndef _LITTLE_ENDIAN_
	strncat(NormalFile, "NormalPPC", 9);
#else
	strncat(NormalFile, "Normal", 6);
#endif
	// 1.1.ɾ�����ļ�
#if defined (WIN32) || defined(SYLIXOS) || defined(ACOREOS)
	//tm3�������û��rm����������remove����
	remove(NormalFile);
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
    rmdir(NormalFile);
#else
	rm(NormalFile);
#endif

	// 1.2.�������ļ�
	fp = fopen(NormalFile,"wb");
	
	// 2.���÷���ֵ
	if(fp == NULL)
	{
		ret = FALSE;
		printf("���ɷ��߼�¼�ļ�ʧ��\n");
		return ret;
	}
	else
	{
		ret = TRUE;
		printf("���ɷ��߼�¼�ļ��ɹ�\n");
	}

	// 3.0.д�뿪ʼ��־
	fwrite(Flag_begin,sizeof(Flag_begin),1,fp);
	
	// 3.д�뷨����Ϣ
	// 3.1����ģ�������еĶ��� 
//	pNode = (LPNode)pModel->m_listObjects.m_pFirst;
	pstListHead = &(pModel->m_listObjects);
//	while(pNode != NULL)
//	{

	{
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{

		
		//		pObject = (LPObject3D)pNode->m_pCur;
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);

				for(j = 0; j < pObject->m_nNumOfVerts; ++j)					/**< �����������е��� */
				{
		//			for(tex = 0; tex < 3; tex++)							/**< ���������ε����е� */
					{
		//				index = pObject->m_pFaces[j].m_nVertIndex[tex];		/**< ������ÿ��������� */
		//				glNormal3f(	pObject->m_pNormals[index].x,
		// 							pObject->m_pNormals[index].y,  
		// 							pObject->m_pNormals[index].z );			/**< ���������� */	
			 											
						//printf("pObject->m_pNormals[index].x %f\n",pObject->m_pNormals[index].x);
							
						// 3.2 д��ÿ����ķ���
#ifdef WIN32		
#if 1
						//PC����Ҫ��С��ת������д���ļ�			
						{
							char store_normal[12] = {0};
							int i = 0;
							char temp_store = 0;
							
							memcpy(store_normal, &pObject->m_pNormals[j].x, 12);
							//��С��ת��, ÿ4���ֽڵ���1234 - >4321
							for(i=0; i<12; )
							{
								temp_store =	store_normal[0 + i];
								store_normal[0 + i] = store_normal[3 + i];
								store_normal[3 + i] = temp_store;

								temp_store =	store_normal[1 + i];
								store_normal[1 + i] = store_normal[2 + i];
								store_normal[2 + i] = temp_store;		

								i += 4;

							}
							
							if (fwrite(&store_normal,sizeof(store_normal),1,fp) != 1)
							{
								printf("the model path file write error!\n");
								fclose(fp);
								return FALSE;	
							}	

						}
#else

						if (fwrite(&pObject->m_pNormals[j],sizeof(PT_3D_32),1,fp) != 1)
						{
							printf("the model path file write error!\n");
							fclose(fp);
							return FALSE;	
						}




#endif


						
#else
						if (fwrite(&pObject->m_pNormals[j],sizeof(PT_3F),1,fp) != 1)
						{
							printf("the model path file write error!\n");
							fclose(fp);
							return FALSE;	
						}

#endif		
					}
				}

			// ��һ������
	//		pNode = (LPNode)pNode->m_pNext;
			}

		}			
	}


	
	// 3.3д�������־
	fwrite(Flag_end,sizeof(Flag_end),1,fp);
	
	// 4.�ر��ļ�
	fclose(fp);

	printf("...����д�뷨�߼�¼�ļ�\n\n");
	
	return ret;

}

//lpf 2017-2-16 16:44 make
/** ��ȡ���󶥵�ķ�������¼�ļ� ����ļ�*/
BOOL ReadNormalsFile(LPModel3D pModel, f_char_t* strFileName)
{
	LPObject3D pObject = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	char NormalFile[1024] = {0};
	FILE* fp = NULL;
	char readflag[8] = {0}; 
	BOOL ret = FALSE;
	
	// 0./** ���ģ����û�ж����򷵻� */
	if(pModel->m_nNumOfObjects <= 0)
		return FALSE;
	
	// 1.���ļ�
	memcpy(NormalFile, strFileName, 1024);	
#ifndef _LITTLE_ENDIAN_
	strncat(NormalFile, "NormalPPC", 9);
#else
	strncat(NormalFile, "Normal", 6);
#endif
	fp = fopen(NormalFile,"rb");
	
	// 2.���÷���ֵ
	if(fp == NULL)
	{
		printf("�򿪷��߼�¼�ļ�%sʧ��\n",NormalFile);
		return FALSE;
	}
	else
	{
//		printf("�򿪷��߼�¼�ļ��ɹ�\n");
	}

	// 3.0.���뿪ʼ��־,����ǰ8���ֽ��ж��Ƿ�Ƿ�
	fread(&readflag, sizeof(readflag), 1, fp);

	if(strncmp(readflag, "AABB",4) != 0)
	{
		printf("read normal file begin failed \n");
		fclose(fp);
		return FALSE;

	}
	
	// 3.��ȡ������Ϣ
	// 3.1����ģ�������еĶ��� 
//	pNode = (LPNode)pModel->m_listObjects.m_pFirst;
	pstListHead = &(pModel->m_listObjects);

	{
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
		{
			if( NULL != pstTmpList )
			{
	//		pObject = (LPObject3D)pNode->m_pCur;
			pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);

			pObject->m_pNormals	= (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfVerts);
				
			// 3.2 ��ȡÿ������ķ���

#ifdef _LITTLE_ENDIAN_
#if 1
			{
				// PC�¶�ȡ������ݣ���Ҫ��ȡ����д�С��ת��

				int i =0;
				PT_3F temp_convert = {0};
				float * pt =(float *) &(pObject->m_pNormals[0]);
				float		temp_pt = 0.0f;
				char read_convert[4] = {0};
				
				//��С��ת��
				for(i=0; i< pObject->m_nNumOfVerts * 3; i++)
				{
					//pc�²���ֱ�Ӷ�ȡ��˵�float����
					if (fread(read_convert, sizeof(float) ,1,fp) != 1)
					{

						printf("the model normal file read error!\n");
						fclose(fp);	
						return FALSE;
					}

					//��С��ת��
					{
						char temp_store;
						int i = 0;
					
						temp_store =	read_convert[0 + i];
						read_convert[0 + i] = read_convert[3 + i];
						read_convert[3 + i] = temp_store;

						temp_store =	read_convert[1 + i];
						read_convert[1 + i] = read_convert[2 + i];
						read_convert[2 + i] = temp_store;		

					}
					
					*pt = *(float *)read_convert;

					pt++;


	//				temp_convert.x = ConvertL2B_float(pObject->m_pNormals2[i].x);
	//				temp_convert.y = ConvertL2B_float(pObject->m_pNormals2[i].y);
	//				temp_convert.z = ConvertL2B_float(pObject->m_pNormals2[i].z);

	//				SetValue_3D_32(&pObject->m_pNormals2[i], &temp_convert);	

				}

				pt  = NULL;
			}



#else
			if (fread(&pObject->m_pNormals2[0],sizeof(PT_3D_32) * pObject->m_nNumOfVerts,1,fp) != 1)
			{

				printf("the model normal file read error!\n");
				fclose(fp);	
				return FALSE;
			}



#endif



#else
			if (fread(&pObject->m_pNormals[0],sizeof(PT_3F) * pObject->m_nNumOfVerts,1,fp) != 1)
			{

				printf("the model normal file read error!\n");
				fclose(fp);	
				return FALSE;
			}



#endif
			// ��һ������
	//		pNode = (LPNode)pNode->m_pNext;
				}
		}	




		
	}
	
	
	// 3.3д�������־
	fread(&readflag, sizeof(readflag), 1, fp);

	if(strncmp(readflag, "BBAA",4) != 0)
	{
		printf("read normal file end failed \n");
		fclose(fp);
		return FALSE;
	}
	
	// 4.�ر��ļ�
	fclose(fp);

//	printf("...������ȡ���߼�¼�ļ�\n\n");
	
	return TRUE;

}
