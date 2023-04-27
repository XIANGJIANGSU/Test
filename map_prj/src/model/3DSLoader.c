/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2014年
**
** 文件名: 3DSLoader.c
**
** 描述: 本文件包含3DS 文件的读入函数与3DS模型的绘制函数。
**
** 定义的函数:  
**       
**                            
** 设计注记: 
**
** 作者：
**		LPF。
** 
**
** 更改历史:
**      2014-11-12 10:40 		LPF 修改本文----添加打印语句。
**	  	2014-11-14 9:14   		LPF添加ReadVertices中的y和z坐标交换，z取反。
**	    2014-12-12 9:56			LPF取消坐标变换语句。
**	  	2014-11-21 13:40		LPF设置材质ID为i，不能为0.
**      2014-12-1   14:28		LPF添加初始化vSum，修改Div_3D_32(...(f_float64_t)(shared));shared的负号去掉
**      2014-12-10  11:50		LPF删掉了存储无用数据的bulffer，使用fseek语句。
**      2014-12-11  15:51		LPF添加了读取数据大小与存储变量长度的判断语句
**		2015-5-5 19:37			LPF注释了glNormal3f函数
**		2015-9-22 15:36			LPF修改读取多个纹理的函数,注释“包围盒函数”。
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
** 函数名: Init_3DS
**
** 描述: 初始化三维模型的具体数据，即解析*.3ds文件
**
** 输入参数:  pModel--模型具体数据信息结构体指针
**           chFilename--*.3ds文件全路径名
**
** 输出参数: pModel--模型具体数据信息结构体指针
**
** 返回值: TRUE--加载成功
**        FALSE--加载失败         
**
** 设计注记: 无
**
**.EH--------------------------------------------------------
*/
void Init_3DS(LPModel3DS pModel, const f_char_t* chFilename)
{
	//用于解析3ds文件名称字符串
	f_char_t *psz = NULL;
	f_char_t filename[256] = {0};

	f_uint16_t  j;
	
	//用于循环遍历链表
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;

	//模型材质信息结构体
	LPMatInfo pMatInfo = NULL;
	f_char_t filepath[256] = {0};

	/**< 为当前块分配空间 */
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));
	/**< 为临时块分配空间 */
	g_pTempChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));				

	memset(g_pCurrentChunk, 0, sizeof(Chunk));
	memset(g_pTempChunk, 0, sizeof(Chunk));

	//初始化模型具体数据信息结构体指针指向的内存区域
	memset(pModel, 0, sizeof(Model3DS));

	//将*.3ds文件的全路径名复制到filename中
	strcpy(filename, chFilename);

	//初始化模型具体数据中的对象链表和材质链表
	CreateList(&(pModel->m_3DModel.m_listObjects));
	CreateList(&(pModel->m_3DModel.m_listMaterials));
	
	/** 将3ds文件装入到模型具体信息结构体中三维详细模型信息结构体 */
	if(!Import3DS(&pModel->m_3DModel, filename))
		return;

	//获取*.3ds文件所在的路径名
	psz = strrchr(filename,'/');
	if(psz != NULL)
		*psz = 0;

	pstListHead = &(pModel->m_3DModel.m_listMaterials);	
	j = 0;

    //遍历模型的材质信息链表
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)		
	{
		if( NULL != pstTmpList )
		{
			pMatInfo = ASDE_LIST_ENTRY(pstTmpList, MatInfo, stListHead);
			/**< 判断是否是一个文件名 */
			if(strlen(pMatInfo->m_strFile) > 0)                 
			{	
				strncpy(filepath, filename, 256);
				strncat(filepath, "/", 1);
				strncat(filepath, pMatInfo->m_strFile, sizeof(pMatInfo->m_strFile));
				/**< 读取模型材质信息对应的纹理文件(*.bmp)，并生成纹理id */
				LoadTexture(filepath, pModel->m_textures, j);			
				/** 设置材质的纹理ID */
				pMatInfo->m_nTexureId = j;  //LPF add  2015-9-22 15:38
	//			j++;						//LPF modify
			}
			else							//LPF add
			{
				/** 设置材质的纹理ID */
				pMatInfo->m_nTexureId = j;
			}	
	// 		/** 设置材质的纹理ID */			//LPF modify
	// 		pMatInfo->m_nTexureId = j;
			j++;
//			pNode = (LPNode)pNode->m_pNext;
			
		}

	}
	//计算包围盒							//LPF modify
// 	{
// 		LPObject3D pObject = NULL;
// 		/** 遍历模型中所有的对象 */
// 		pNode = (LPNode)pModel->m_3DModel.m_listObjects.m_pFirst;
// 		while (pNode != NULL)
// 		{
// 			/** 获得当前的对象 */
// 			pObject = (LPObject3D)pNode->m_pCur;
// 			/*
// 			 * 遍历对象的所有面
// 			 */
// 			for(i = 0; i < pObject->m_nNumOfFaces; ++i)
// 			{
// 				AddPt2AABB3(&pModel->m_AABB3, &pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[0]]);
// 				AddPt2AABB3(&pModel->m_AABB3, &pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[1]]);
// 				AddPt2AABB3(&pModel->m_AABB3, &pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[2]]);
// 			}
// 			pNode = (LPNode)pNode->m_pNext;
// 		}
// 		GetAABB3Radius(&pModel->m_AABB3);	// 获得AABB组成的包围球半径
// 		GetAABB3Center(&pModel->m_AABB3);	// 获得AABB中心点坐标
// 	}
}



/*
 * 3ds模型, 删除内存
 * pModel：三维模型数据
 */
void Delete_3DS(LPModel3DS pModel)
{
	LPObject3D pObject = NULL;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	
	// 纹理
	glDeleteTextures(MAX_TEXTURES, pModel->m_textures);	
	// 显示列表
	if(pModel->m_glLists != 0)
	{
	#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
		glDeleteLists(pModel->m_glLists, pModel->m_3DModel.m_nNumOfObjects);
	#else
		//ES下esCommon库中实现的glDeleteLists函数：一次只能删除1个显示列表
		int i = 0, m = GetNodeNum(&pModel->m_3DModel.m_listObjects);;
		for (i = 0; i < m;i++)    //pModel->m_3DModel.m_nNumOfObjects
		    glDeleteLists(pModel->m_glLists[i], 1);
		free(pModel->m_glLists);
	#endif
	}
	// 删除每个Object3D中的信息
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
** 函数名: Import3DS
**
** 描述: 从*.3ds文件读取具体模型信息到三维详细模型信息结构体
**
** 输入参数:  p3DModel--三维详细模型信息结构体指针
**           chFilename--*.3ds文件全路径名
**
** 输出参数: p3DModel--三维详细模型信息结构体指针
**
** 返回值: TRUE--加载成功
**        FALSE--加载失败         
**
** 设计注记: 无
**
**.EH--------------------------------------------------------
*/
BOOL Import3DS(LPModel3D p3DModel, f_char_t* strFileName)
{
	FILE* t_pFilePointer = NULL;
	/** 打开一个3ds文件 */
	t_pFilePointer = fopen(strFileName, "rb");
	/**< 检查文件指针 */
	if(!t_pFilePointer)
	{	
		return FALSE;
	}
	/** 将文件的第一块读出并判断是否是3ds文件 */
	ReadChunk(g_pCurrentChunk, t_pFilePointer);
	/** 确保是3ds文件 */
	if (g_pCurrentChunk->m_nID != PRIMARY)
	{	
		return FALSE;
	}
	/** 递归读出对象数据 */
	ReadNextChunk(p3DModel, g_pCurrentChunk, t_pFilePointer);


#if 0
	/** 计算顶点的法线 */
#ifdef WIN32	
	ComputeNormals(p3DModel);   //LPF				
#else	
	ReadNormalsFile(p3DModel, strFileName);		//lpf 2017-2-16 17:02直接读取法线记录文件
#endif	
	
#if 0	
	CreateNormalsFile(p3DModel, strFileName);		//lpf 2017-2-16 17:01 生成法线记录文件
#endif
#endif


#if	1  
	// 读取顶点的法线文件（*.3dsNormal）
 	if(FALSE == ReadNormalsFile(p3DModel, strFileName))
	{
		// 读取失败的话，计算法线并生成法线文件
		ComputeNormals(p3DModel);   //LPF				
		/** 生成对象顶点的法向量记录文件 */
		CreateNormalsFile(p3DModel, strFileName);	

	}
#endif

#if 0
	{
		ReadNormalsFile(p3DModel, strFileName);
		
		//比较两种法线
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
					printf("法线存储错误\n%f,%f\n%f,%f\n%f,%f\n",
							pObject->m_pNormals[i].x , pObject->m_pNormals2[i].x,
							pObject->m_pNormals[i].y , pObject->m_pNormals2[i].y,
							pObject->m_pNormals[i].z , pObject->m_pNormals2[i].z);
				}


			}

			// 下一个对象
			pNode = (LPNode)pNode->m_pNext;
		}

	}

#endif	



	/** 释放内存空间 */
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
 *	创建纹理
 *	filepath: 纹理路径
 *	textureArray：返回绑定纹理数组
 *	textureID：	
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

	/** 载入位图 */
	pImageData = ReadBMP(filepath, &width, &height, &size, &style);

	if(pImageData == NULL)
	{
		printf("3DS模型-%s:载入位图失败!\n", filepath);
		textureArray[textureID] = 0;
		return;
	}
	glGenTextures(1, &textureArray[textureID]);
	textures[i].texid = textureArray[textureID];

	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

#if 1
	/** 控制滤波 */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/** 创建纹理 */
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
 * 读取一个块 
 */
void ReadChunk(LPChunk pChunk, FILE* pFile)
{
	f_uint16_t tempConvUS;
	f_uint32_t tempConvUI;
	/* 读入块的ID号，并记录已读取的字节数 */
	pChunk->m_nBytesRead = fread(&tempConvUS, 1, 2, pFile);
#ifdef _LITTLE_ENDIAN_
	pChunk->m_nID = tempConvUS;
#else
	pChunk->m_nID = ConvertL2B_ushort(tempConvUS);
#endif
	/** 读入块占用的长度，并记录已读取的字节数 */
	pChunk->m_nBytesRead += fread(&tempConvUI, 1, 4, pFile);
	
#ifdef _LITTLE_ENDIAN_
	pChunk->m_nLength = tempConvUI;
#else
	pChunk->m_nLength = ConvertL2B_uint(tempConvUI);
#endif
}

/** 读取下一个块 */
void ReadNextChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile)
{
	/**< 用来添加到对象链表 */
	LPObject3D pNewObject = NULL;	
	/**< 用来添加到材质链表 */
	LPMatInfo  pNewTexture = NULL;	
	/**< 保存文件版本 */
	f_uint32_t version = 0;	
	/**< 用来跳过不需要的数据 */
	//f_int32_t buffer[50000] = {0};											
	f_uint32_t tempConvUI = 0;
	/**< 为新的块分配空间*/
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));			

	/** 如果已经读取字节数小于总的字节数，继续读入子块 */
	while (pPreChunk->m_nBytesRead < pPreChunk->m_nLength)
	{	
		/** 读入下一个块 */
		ReadChunk(g_pCurrentChunk, pFile);
		/** 判断块的ID号 */
		switch (g_pCurrentChunk->m_nID)
		{
			/** 文件版本号 */
		case VERSION:							
			//需要做大小端转换（unsighed int）
			/** 读入文件的版本号 */
			/**需要对读入的内容大小进行判断，是否与存储变量相符 */
			if((g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead) <= sizeof(tempConvUI))
			{
				g_pCurrentChunk->m_nBytesRead += fread(&tempConvUI, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
#ifdef _LITTLE_ENDIAN_
				version = tempConvUI;
#else
				version = ConvertL2B_int(tempConvUI);
#endif
				/** 如果文件版本号大于3，给出一个警告信息 */
				if (version > 0x03)
				{
					//throw(" 该3DS文件版本大于3.0,可能不能正确读取");
					printf("该3DS文件版本大于3.0,可能不能正确读取!\n");
				}
			}
			else
			{
				//throw("文件内容损坏");
				printf("文件内容损坏!\n");
			}
			break;

			/** 网格版本信息 */
		case OBJECTINFO:						
			/** 读入下一个块 */
			ReadChunk(g_pTempChunk, pFile);
			//需要做大小端转换 (单位int)
			/** 获得网格的版本号 */
			/**需要对读入的内容大小进行判断，是否与存储变量相符 */
			if((g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead) <= sizeof(tempConvUI))
			{
				g_pTempChunk->m_nBytesRead += fread(&tempConvUI, 1, g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead, pFile);
#ifdef _LITTLE_ENDIAN_
				version = tempConvUI;
#else
				version = ConvertL2B_uint(tempConvUI);
#endif
				/** 增加读入的字节数 */
				g_pCurrentChunk->m_nBytesRead += g_pTempChunk->m_nBytesRead;
				/** 递归进入下一个块 */
				ReadNextChunk(pModel, g_pCurrentChunk, pFile);
			}
			else
			{
				//throw("文件内容损坏");
				printf("文件内容损坏!\n");
			}
			break;

			/** 材质信息 */
		case MATERIAL:							
			/** 材质的数目递增 */
			pModel->m_nNumOfMaterials++;
			/** 在纹理链表中添加一个空白纹理结构 */
		    {
				LPMatInfo  pNewTexture = NewFixedMemory(sizeof(MatInfo));	
				
		    	/**挂到头结点之后**/
				stList_Head *pstListHead = NULL;
				pstListHead = &(pModel->m_listMaterials);
				LIST_ADD(&pNewTexture->stListHead, pstListHead);	

				/** 进入材质装入函数 */
				ReadNextMatChunk(pModel, g_pCurrentChunk, pFile);	
		    }
			break;
			
			/** 对象名称 */
		case OBJECT:							
			/** 对象数递增 */
			pModel->m_nNumOfObjects++;
			/** 添加一个新的tObject节点到对象链表中 */
			{
				LPObject3D pNewObject = NewFixedMemory(sizeof(Object3D));	
				
		    	/**挂到头结点之后**/
				stList_Head *pstListHead = NULL;
				pstListHead = &(pModel->m_listObjects);
				LIST_ADD(&pNewObject->stListHead, pstListHead);	

				/*
				 * 获得并保存对象的名称，然后增加读入的字节数
				 */
				g_pCurrentChunk->m_nBytesRead += GetString(pNewObject->m_strName, pFile);
				/** 进入其余的对象信息的读入 */
				ReadNextObjChunk(pModel, pNewObject, g_pCurrentChunk, pFile);
			}
			break;

			/** 关键帧 */
		//case EDITKEYFRAME:
			//需要做大小端转换 （unsigned int）
			/** 跳过关键帧块的读入 */
			//g_pCurrentChunk->m_nBytesRead += fread(buffer, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			//break;

		default: 
			//需要做大小端转换 (unsigned int)
			/**  跳过所有忽略的块的内容的读入，包含关键帧块*/
			fseek( pFile, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead,SEEK_CUR);
			g_pCurrentChunk->m_nBytesRead += (g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead);
			break;
		}
		/** 增加从最后块读入的字节数 */
		pPreChunk->m_nBytesRead += g_pCurrentChunk->m_nBytesRead;
	}

	/** 释放当前块的内存空间 */
	DeleteAlterableMemory(g_pCurrentChunk);
	g_pCurrentChunk = pPreChunk;
}

/** 处理所有的文件中对象的信息 */
void ReadNextObjChunk(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	//f_int32_t buffer[50000] = {0};					/** 用于读入不需要的数据 */
	/** 对新的块分配存储空间 */
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));
	/*
	 * 继续读入块的内容直至本子块结束 
	 */
	while(pPreChunk->m_nBytesRead < pPreChunk->m_nLength)
	{	
		/** 读入下一个块 */
		ReadChunk(g_pCurrentChunk, pFile);

		/** 区别读入是哪种块 */
		switch (g_pCurrentChunk->m_nID)
		{
		/*
		 *< 正读入的是一个新块 
		 */
		case OBJ_MESH:					
			/** 使用递归函数调用，处理该新块 */
			ReadNextObjChunk(pModel, pObject, g_pCurrentChunk, pFile);
			break;
		case OBJ_VERTICES:				/**< 读入是对象顶点 */
			ReadVertices(pObject, g_pCurrentChunk, pFile);
			break;
		case OBJ_FACES:					/**< 读入的是对象的面 */
			ReadVertexIndices(pObject, g_pCurrentChunk, pFile);
			break;
		case OBJ_MATERIAL:				/**< 读入的是对象的材质名称 */
			/** 读入对象的材质名称 */
			ReadObjMat(pModel, pObject, g_pCurrentChunk, pFile);		
			break;
		case OBJ_UV:						/**< 读入对象的UV纹理坐标 */
			/** 读入对象的UV纹理坐标 */
			ReadUVCoordinates(pObject, g_pCurrentChunk, pFile);
			break;
		default:  
			//需要做大小端转换 (unsigned int)
			/** 略过不需要读入的块 */
			//g_pCurrentChunk->m_nBytesRead += fread(buffer, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			fseek( pFile, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead,SEEK_CUR);
			g_pCurrentChunk->m_nBytesRead += (g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead);
			break;
		}
		/** 添加从最后块中读入的字节数到前面的读入的字节中 */
		pPreChunk->m_nBytesRead += g_pCurrentChunk->m_nBytesRead;
	}
	/** 释放当前块的内存空间，并把当前块设置为前面块 */
	DeleteAlterableMemory(g_pCurrentChunk);
	g_pCurrentChunk = pPreChunk;
}

/** 读取下一个材质块 */
void ReadNextMatChunk(LPModel3D pModel, LPChunk pPreChunk, FILE* pFile)
{
	//f_int32_t buffer[50000] = {0};					/**< 用于读入不需要的数据 */
	LPMatInfo pModelMatNow = ASDE_LIST_ENTRY(pModel->m_listMaterials.pNext, MatInfo, stListHead);	/*当前存入的材质内存块*/
	
	/** 给当前块分配存储空间 */
	g_pCurrentChunk = (LPChunk)NewAlterableMemory(sizeof(Chunk));
	/** 继续读入这些块 */
	while (pPreChunk->m_nBytesRead < pPreChunk->m_nLength)
	{	
		/*
		 * 读入下一块
		 */
		ReadChunk(g_pCurrentChunk, pFile);
		/** 判断读入的是什么块 */
		switch (g_pCurrentChunk->m_nID)
		{
		case MATNAME:							/**< 材质的名称 */
			/** 读入材质的名称 */
                     	/**需要对读入的内容大小进行判断，是否与存储变量相符 */
			if((g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead) <=  256)
			{
	
//				g_pCurrentChunk->m_nBytesRead += fread(((LPMatInfo)(pModel->m_listMaterials.m_pLast->m_pCur))->m_strName, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
				g_pCurrentChunk->m_nBytesRead += 
					fread(pModelMatNow->m_strName, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			
			}
			else
			{
				printf("文件内容损坏!\n");
				//throw("文件内容损坏");
			}
			break;
		case MATDIFFUSE:						/**< 对象的R G B颜色 */
//			ReadColor((LPMatInfo)(pModel->m_listMaterials.m_pLast->m_pCur), g_pCurrentChunk, pFile);
			ReadColor(pModelMatNow, g_pCurrentChunk, pFile);
			
			break;
		case MATMAP:							/**< 纹理信息的头部 */
			/** 下一个材质块信息 */
			ReadNextMatChunk(pModel, g_pCurrentChunk, pFile);
			break;
		case MATMAPFILE:						/**< 材质文件的名称 */
			/** 读入材质的文件名称 */
                     	/**需要对读入的内容大小进行判断，是否与存储变量相符 */
			if((g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead) <=  256)
			{
//				g_pCurrentChunk->m_nBytesRead += fread(((LPMatInfo)(pModel->m_listMaterials.m_pLast->m_pCur))->m_strFile, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
				g_pCurrentChunk->m_nBytesRead += fread(((LPMatInfo)(pModelMatNow))->m_strFile, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);

			}
			else
			{
				//throw("文件内容损坏");
				printf("文件内容损坏!\n");
			}
			break;
		default:  
			/** 跳过不需要读入的块 */
			//g_pCurrentChunk->m_nBytesRead += fread(buffer, 1, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead, pFile);
			fseek( pFile, g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead,SEEK_CUR);
			g_pCurrentChunk->m_nBytesRead += (g_pCurrentChunk->m_nLength - g_pCurrentChunk->m_nBytesRead);
			break;
		}
		/** 添加从最后块中读入的字节数 */
		pPreChunk->m_nBytesRead += g_pCurrentChunk->m_nBytesRead;
	}
	/** 删除当前块，并将当前块设置为前面的块 */
	DeleteAlterableMemory(g_pCurrentChunk);
	g_pCurrentChunk = pPreChunk;
}
/** 读取对象颜色的RGB值 */
void ReadColor(LPMatInfo pMaterial, LPChunk pChunk, FILE* pFile)
{
	/** 读入颜色块信息 */
	ReadChunk(g_pTempChunk, pFile);
	/** 读入RGB颜色 */
       /**需要对读入的内容大小进行判断，是否与存储变量相符 */
	if((g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead) <= sizeof(pMaterial->m_nColor))
	{
		g_pTempChunk->m_nBytesRead += fread(pMaterial->m_nColor, 1, g_pTempChunk->m_nLength - g_pTempChunk->m_nBytesRead, pFile);
		/** 增加读入的字节数 */
		pChunk->m_nBytesRead += g_pTempChunk->m_nBytesRead;
	}
	else
	{
		//throw("颜色块出错")	;
		printf("颜色块出错!\n");
	}
	/** 增加读入的字节数 */
	pChunk->m_nBytesRead += g_pTempChunk->m_nBytesRead;
}


/** 读取对象的顶点信息 */
void ReadVertices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_uint16_t tempConvUS = 0;

	/** 读入顶点的数目 */
	pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, 2, pFile);
	
#ifdef _LITTLE_ENDIAN_
	pObject->m_nNumOfVerts = tempConvUS;
#else
	pObject->m_nNumOfVerts = ConvertL2B_ushort(tempConvUS);
#endif

	/** 分配顶点的存储空间，然后初始化结构体 */
	pObject->m_pVerts = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfVerts);
	/** 读入顶点序列 */
    /**需要对读入的内容大小进行判断，是否与存储变量相符 */
	if((pPreChunk->m_nLength - pPreChunk->m_nBytesRead) <= sizeof(PT_3F) * pObject->m_nNumOfVerts)
	{

//嵌入式下需要先读取char数组，再大小端转换, lpf add 2017-4-27 10:53:11
#ifndef _LITTLE_ENDIAN_

		{

			int i =0;
			PT_3F temp_convert = {0};
			float * pt = (float *)&pObject->m_pVerts[0];
			char read_convert[4] = {0};
			
			//大小端转换
			for(i=0; i< pObject->m_nNumOfVerts * 3; i++)
			{
				//pc下不能直接读取大端的float数据
				if (fread(read_convert, sizeof(float) ,1,pFile) != 1)
				{

					printf("the model verterts file read error!\n");
					fclose(pFile);	
					return ;
				}

				//大小端转换
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
		//PC 下直接读取
		pPreChunk->m_nBytesRead += fread(pObject->m_pVerts, 1, pPreChunk->m_nLength - pPreChunk->m_nBytesRead, pFile);

#endif
	}
	else
	{
		//throw("读入定点序列出错");
		printf("读入顶点序列出错!\n");
	}
}

/** 读取对象的面信息 */
void ReadVertexIndices(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_uint16_t tempConvUS = 0;
	/*
	 *< 用于读入当前面的索引 
	 */
	f_uint16_t index = 0;		
	f_int32_t i, j;
	/** 读入该对象中面的数目 */
	pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, 2, pFile);
#ifdef _LITTLE_ENDIAN_
	pObject->m_nNumOfFaces = tempConvUS;
#else
	pObject->m_nNumOfFaces = ConvertL2B_ushort(tempConvUS);
#endif
	/*
	 * 分配所有面的存储空间，并初始化结构
	 */
	pObject->m_pFaces = (LPFace)NewAlterableMemory(sizeof(Face) * pObject->m_nNumOfFaces);
	/** 遍历对象中所有的面 */
	for(i = 0; i < pObject->m_nNumOfFaces; ++i)
	{	
		for(j = 0; j < 4; ++j)
		{	
			/** 读入当前面的第一个点  */
			pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, sizeof(f_uint16_t), pFile);
#ifdef _LITTLE_ENDIAN_
			index = tempConvUS;
#else
			index = ConvertL2B_ushort(tempConvUS);
#endif
			if(j < 3)
			{	
				/*
				 * 将索引保存在面的结构中
				 */
				pObject->m_pFaces[i].m_nVertIndex[j] = index;
			}
		}
	}
}

/*
 * 读取对象的纹理坐标 
 */
void ReadUVCoordinates(LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_uint16_t tempConvUS = 0;
//	f_int32_t i;
	/** 读入UV坐标的数量 */
	pPreChunk->m_nBytesRead += fread(&tempConvUS, 1, 2, pFile);
#ifdef _LITTLE_ENDIAN_
	pObject->m_nNumTexVertex = tempConvUS;
#else
	pObject->m_nNumTexVertex = ConvertL2B_ushort(tempConvUS);
#endif

	/** 分配保存UV坐标的内存空间 */
	pObject->m_pTexVerts = (LP_PT_2F)NewAlterableMemory(sizeof(PT_2F) * pObject->m_nNumTexVertex);
	/** 读入纹理坐标 */
       /**需要对读入的内容大小进行判断，是否与存储变量相符 */
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
//嵌入式下需要先读取char数组，再大小端转换, lpf add 2017-4-27 10:53:11
		{
			// PC下读取大端数据，需要读取后进行大小端转换

			int i =0;
			PT_3F temp_convert = {0};
			float * pt = (float *)&pObject->m_pTexVerts[0];
			float		temp_pt = 0.0f;
			char read_convert[4] = {0};
			
			//大小端转换
			for(i=0; i< pObject->m_nNumTexVertex * 2; i++)
			{
				//pc下不能直接读取大端的float数据
				if (fread(read_convert, sizeof(float) ,1,pFile) != 1)
				{

					printf("the model normal file read error!\n");
					fclose(pFile);	
					return ;
				}

				//大小端转换
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
//PC 下直接读取

		pPreChunk->m_nBytesRead += fread(pObject->m_pTexVerts, 1, pPreChunk->m_nLength - pPreChunk->m_nBytesRead, pFile);



#endif




#endif













	}
	else
	{
		//throw("纹理坐标块报错");
		printf("纹理坐标块报错!\n");
	}
}

/*
 * 读取赋予对象的材质 
 */
void ReadObjMat(LPModel3D pModel, LPObject3D pObject, LPChunk pPreChunk, FILE* pFile)
{
	f_char_t strMaterial[255] = {0};			/**< 用来保存对象的材质名称 */
	//f_int32_t buffer[50000] = {0};				/**< 用来读入不需要的数据 */
	f_uint32_t i;
//	LPNode pNode = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;	
	LPMatInfo pMatInfo = NULL;
	/** 读入赋予当前对象的材质名称 */
	pPreChunk->m_nBytesRead += GetString(strMaterial, pFile);
	/*
	 * 遍历所有的纹理 
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
				 * 如果读入的纹理与当前的纹理名称匹配 
				 */
				if(strncmp(strMaterial, pMatInfo->m_strName, 255) == 0)
				{	
					/** 设置材质ID */
					pObject->m_nMaterialID = i;	//2015-3-30 13:46  LPF
					//pObject->m_nMaterialID = 0;
					/** 判断是否是纹理映射 */
					if(strlen(pMatInfo->m_strFile) > 0) 
					{
						/** 设置对象的纹理映射标志 */
						pObject->m_bHasTexture = TRUE;
					}	
					break;
				}
				else
				{	
					/** 如果该对象没有材质，则设置ID为-1 */
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

/** 计算对象顶点的法向量 */
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
	//初始化vSum
	vector3FMemSet(&vSum, 0, 0, 0);	
	/** 如果模型中没有对象，则返回 */
	if(pModel->m_nNumOfObjects <= 0)
		return;
	/** 遍历模型中所有的对象 */
	//for(index = 0; index < pModel->m_nNumOfObjects; ++index)
	//{	
//		pNode = (LPNode)pModel->m_listObjects.m_pFirst;
		pstListHead = &(pModel->m_listObjects);
//		while (pNode != NULL)

		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
		{
			/** 获得当前的对象 */
//			pObject = (LPObject3D)pNode->m_pCur;
			if( NULL != pstTmpList )
			{
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);

				/** 分配需要的存储空间 */
				pNormals = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfFaces);
				pTempNormals = (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfFaces);
				pObject->m_pNormals	= (LP_PT_3F)NewAlterableMemory(sizeof(PT_3F) * pObject->m_nNumOfVerts);
				/*
				 * 遍历对象的所有面
				 */
				for(i = 0; i < pObject->m_nNumOfFaces; ++i)
				{
					vPoly[0] = pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[0]];
					vPoly[1] = pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[1]];
					vPoly[2] = pObject->m_pVerts[pObject->m_pFaces[i].m_nVertIndex[2]];

					//m_AABB3.add(vPoly[0]);
					//m_AABB3.add(vPoly[1]);
					//m_AABB3.add(vPoly[2]);

					/** 计算面的法向量 */
					//vVector1 = vPoly[0] - vPoly[2];		        /**< 获得多边形的矢量 */
					vector3FSub(&vVector1, &vPoly[0], &vPoly[2]);
					//vVector2 = vPoly[2] - vPoly[1];		        /**< 获得多边形的第二个矢量 */
					vector3FSub(&vVector2, &vPoly[2], &vPoly[1]);
					//vNormal  = vVector1.crossProduct(vVector2);	/**< 计算两个矢量的叉积 */
					vector3FCrossProduct(&vNormal, &vVector1, &vVector2);
					//pTempNormals[i] = vNormal;
					vector3FSetValue(&pTempNormals[i], &vNormal);
					//vNormal  = vNormal.normalize();					/**< 规一化叉积 */
					vector3FNormalize(&vNormal);
					//pNormals[i] = vNormal;							/**< 将法向量添加到法向量列表中 */
					vector3FSetValue(&pNormals[i], &vNormal);
				}
				/** 计算顶点法向量 */
				
				//Vector3 vZero = vSum;
				/** 遍历所有的顶点 */
				for (i = 0; i < pObject->m_nNumOfVerts; ++i)			
				{	
					for (j = 0; j < pObject->m_nNumOfFaces; ++j)	/**< 遍历所有的三角形面 */
					{													/**< 判断该点是否与其它的面共享 */
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
					/** 规一化顶点法向 */
					//pObject->m_pNormals[i] = pObject->m_pNormals[i].normalize();
					vector3FNormalize(&pObject->m_pNormals[i]);
					vector3FMemSet(&vSum, 0, 0, 0);							
					shared = 0;										
				}
				/** 释放存储空间，开始下一个对象 */
				DeleteAlterableMemory(pTempNormals);
				DeleteAlterableMemory(pNormals);
//				pNode = (LPNode)pNode->m_pNext;


				
			
			}




			

		}
	//}
}

/** 读一个字符串 */
f_int32_t  GetString(f_char_t* pBuffer, FILE* pFile)
{
	f_int32_t index = 0;
	/** 读入一个字节的数据 */
	fread(pBuffer, 1, 1, pFile);
	/** 直到结束 */
	while(*(pBuffer + index++) != 0) 
	{
		/** 读入一个字符直到NULL */
		fread(pBuffer + index, 1, 1, pFile);
	}
	/** 返回字符串的长度 */
	return strlen(pBuffer) + 1;
}

/** 释放内存,关闭文件 */
void CleanUp(LPModel3D pModel)
{
	
	//delete m_CurrentChunk;		     /**< 释放当前块 */
	DeleteAlterableMemory(g_pCurrentChunk);
	//delete m_TempChunk;				 /**< 释放临时块 */
	DeleteAlterableMemory(g_pTempChunk);
}

//通过链表查询纹理结构体，输入链表头和纹理ID，输出纹理结构体
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

//获取链表的节点个数，根据每个节点是否为空判断
int GetNodeNum(const stList_Head *pstListHead)
{
	stList_Head *pstTmpList = NULL;
	LPMatInfo pNode = NULL;
	int i = 0;
	
	//确保链表的头节点非空
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
 * 3ds模型
 * pModel：三维模型数据
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
	glPushAttrib(GL_CURRENT_BIT);									/**< 保存现有颜色属实性 */
#endif
	/**< 遍历模型中所有的对象 */
//	pNode = pModel->m_3DModel.m_listObjects.m_pFirst;
	pstListHead = &(pModel->m_3DModel.m_listObjects);	
//	while (pNode != NULL)

	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)	
	{
//		pObject = (LPObject3D)pNode->m_pCur;
		
			if( NULL != pstTmpList )
			{
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);
				
				if (pObject->m_bHasTexture)									/**< 判断该对象是否有纹理映射 */
				{
					glBindTexture(GL_TEXTURE_2D, pModel->m_textures[pObject->m_nMaterialID]);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				/** 开始绘制 */
				glBegin(GL_TRIANGLES);
				for(j = 0; j < pObject->m_nNumOfFaces; ++j)					/**< 遍历所有的面 */
				{
					for(tex = 0; tex < 3; tex++)							/**< 遍历三角形的所有点 */
					{
						index = pObject->m_pFaces[j].m_nVertIndex[tex];		/**< 获得面对每个点的索引 */
		 				glNormal3f(	pObject->m_pNormals[index].x,
		 							pObject->m_pNormals[index].y,  
		 							pObject->m_pNormals[index].z );			/**< 给出法向量 */
						//printf("pObject->m_pNormals[index].x %f\n",pObject->m_pNormals[index].x);
						if(pObject->m_bHasTexture)							/**< 如果对象具有纹理 */
						{	
							if(pObject->m_pTexVerts)						/**< 确定是否有UVW纹理坐标 */
							{
								glTexCoord2f(pObject->m_pTexVerts[index].x,pObject->m_pTexVerts[index].y);
							}
						}
						else												/**< 如果没有纹理，则赋予材质颜色 */
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
				glEnd(); /**< 绘制结束 */
//				pNode = (LPNode)pNode->m_pNext;
				
			
			}


		

	}
	glPopAttrib();   /**< 恢复前一属性 */
}

/*
 *	使用显示列表的方法绘制三维模型
 *  displaytype3ds  0：正常显示，1：闪动，2：线框(正常大小,颜色默认为绿色)，
 *	3：模型+线框(1.02倍大小,颜色默认为绿色)，4：模型+包络线(正常大小,线粗3倍,颜色默认为红色)，
 *  5：光环效果,即模型+纯色(1.05倍大小,颜色默认为金黄色)，6：纯色纹理(颜色默认为红色)
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
	//外部输入的颜色或材质
	mat_color[0] = colorModel3ds.x;
	mat_color[1] = colorModel3ds.y;
	mat_color[2] = colorModel3ds.z;
	mat_color[3] = colorModel3ds.w;
	
//	f_float32_t mat_amb[] = {0.2,0.2,0.2,1.0};			//default
//	f_float32_t mat_diff[] = {0.8,0.8,0.8,1.0};			//default
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glPushAttrib(GL_CURRENT_BIT);									/**< 保存现有颜色属实性 */
#endif
	//线框模式
	if(displaytype3ds == 2)	
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//为模型的每个对象创建一个显示列表
	if(pModel->m_glLists == 0)
	{
		if(!LIST_IS_EMPTY(&pModel->m_3DModel.m_listObjects))	
		{
			// 生成法线,放在这里会卡顿
	//		printf("生成法线...");
	//		ComputeNormals(&pModel->m_3DModel);
	//		printf("生成法线成功\n");

			// 为每个模型对象生成显示列表
			object_size = GetNodeNum(&pModel->m_3DModel.m_listObjects);
		#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
			pModel->m_glLists = glGenLists(object_size);
		#else
			pModel->m_glLists = malloc(object_size*sizeof(f_uint32_t));
			memset(pModel->m_glLists, 0, object_size*sizeof(f_uint32_t));
			for (i = 0; i < object_size;i++)
			    pModel->m_glLists[i] = glGenLists(1);
		#endif
			/**< 遍历模型中所有的对象 */
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
						/** 开始绘制 */
						glBegin(GL_TRIANGLES);
						for(j = 0; j < pObject->m_nNumOfFaces; ++j)					/**< 遍历对象所有的面 */
						{
							for(tex = 0; tex < 3; tex++)							/**< 遍历三角形的所有点 */
							{
								index = pObject->m_pFaces[j].m_nVertIndex[tex];		/**< 获得面对每个点的索引 */
								glNormal3f(	pObject->m_pNormals[index].x,
				 							pObject->m_pNormals[index].y,  
				 							pObject->m_pNormals[index].z );			/**< 给出法向量 */					
								//printf("pObject->m_pNormals[index].x %f\n",pObject->m_pNormals[index].x);
								if(pObject->m_bHasTexture)							/**< 如果对象具有纹理 */
								{	
									if(pObject->m_pTexVerts)						/**< 确定是否有UVW纹理坐标 */
									{
										glTexCoord2f(pObject->m_pTexVerts[index].x,pObject->m_pTexVerts[index].y);
									}
								}
								else												/**< 如果没有纹理，则赋予材质颜色 */
								{	
								}
								glVertex3f(	pObject->m_pVerts[index].x,             /**< 给出对象顶点 */	
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

	/**< 遍历模型中所有的对象,根据不同显示模式绑定纹理,调用显示列表绘制模型 */
	pstListHead = &(pModel->m_3DModel.m_listObjects);		
	i = 0;
	{
		LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)		
		{
			if( NULL != pstTmpList )
			{
				pObject = ASDE_LIST_ENTRY(pstTmpList, Object3D, stListHead);

				if(displaytype3ds == 2)				//线框模式,设置线框颜色为外部输入的颜色
				{
					glBindTexture(GL_TEXTURE_2D, 0);
					glColor4fv(mat_color);			
				}		
				else if(displaytype3ds == 5)			//光环模式下，不使用纹理
				{
					glBindTexture(GL_TEXTURE_2D, 0);
				}
				else if(displaytype3ds == 6)			//纯色模式,使用纯色贴图
				{
//		//			glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_color);
					glBindTexture(GL_TEXTURE_2D, pictureID);
			
				}
				else								//正常模式
				{			
					if (pObject->m_bHasTexture)			/**< 判断该对象是否有纹理映射 */
					{
						/*如果有则绑定纹理*/
						glBindTexture(GL_TEXTURE_2D, pModel->m_textures[pObject->m_nMaterialID]);
					}
					else
					{
						/*如果没有，则判断模型对象是否有材质信息，如果有则使用材质的颜色*/
						glBindTexture(GL_TEXTURE_2D, 0);
						if(pModel->m_3DModel.m_nNumOfMaterials && pObject->m_nMaterialID >= 0) 
						{	
	//						pMatInfo = (LPMatInfo)(((LPNode)NodeAt(&pModel->m_3DModel.m_listMaterials, pObject->m_nMaterialID))->m_pCur);
							pMatInfo = (LPMatInfo)MatNodeAt(&pModel->m_3DModel.m_listMaterials, (f_uint16_t)pObject->m_nMaterialID);

							//绘制包络线的时候不使用材质的颜色
							if(displaytype3ds != 4)
							{
								glColor3ub(pMatInfo->m_nColor[0],pMatInfo->m_nColor[1],pMatInfo->m_nColor[2]);
							}
						}
					}
				}

				//调用显示列表
			#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
				glCallList(pModel->m_glLists + i);
			#else
				glCallList(pModel->m_glLists[i]);
			#endif

				i++;
			}
		}	

	
	}
	
	//恢复线框模式
	if(displaytype3ds == 2)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	//纯色模式,取消之前的纹理绑定
	if(displaytype3ds == 6)		
	{
//		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_amb);
//		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diff);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glPopAttrib();   /**< 恢复前一属性 */
}

/** 生成对象顶点的法向量记录文件 */
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
	
	// 0./** 如果模型中没有对象，则返回 */
	if(pModel->m_nNumOfObjects <= 0)
		return FALSE;
	
	// 1.生成文件
	memcpy(NormalFile, strFileName, 1024);	
#ifndef _LITTLE_ENDIAN_
	strncat(NormalFile, "NormalPPC", 9);
#else
	strncat(NormalFile, "Normal", 6);
#endif
	// 1.1.删除旧文件
#if defined (WIN32) || defined(SYLIXOS) || defined(ACOREOS)
	//tm3、翼辉下没有rm函数，换成remove函数
	remove(NormalFile);
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
    rmdir(NormalFile);
#else
	rm(NormalFile);
#endif

	// 1.2.生成新文件
	fp = fopen(NormalFile,"wb");
	
	// 2.设置返回值
	if(fp == NULL)
	{
		ret = FALSE;
		printf("生成法线记录文件失败\n");
		return ret;
	}
	else
	{
		ret = TRUE;
		printf("生成法线记录文件成功\n");
	}

	// 3.0.写入开始标志
	fwrite(Flag_begin,sizeof(Flag_begin),1,fp);
	
	// 3.写入法线信息
	// 3.1遍历模型中所有的对象 
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

				for(j = 0; j < pObject->m_nNumOfVerts; ++j)					/**< 遍历对象所有的面 */
				{
		//			for(tex = 0; tex < 3; tex++)							/**< 遍历三角形的所有点 */
					{
		//				index = pObject->m_pFaces[j].m_nVertIndex[tex];		/**< 获得面对每个点的索引 */
		//				glNormal3f(	pObject->m_pNormals[index].x,
		// 							pObject->m_pNormals[index].y,  
		// 							pObject->m_pNormals[index].z );			/**< 给出法向量 */	
			 											
						//printf("pObject->m_pNormals[index].x %f\n",pObject->m_pNormals[index].x);
							
						// 3.2 写入每个点的法线
#ifdef WIN32		
#if 1
						//PC下需要大小端转换后再写入文件			
						{
							char store_normal[12] = {0};
							int i = 0;
							char temp_store = 0;
							
							memcpy(store_normal, &pObject->m_pNormals[j].x, 12);
							//大小端转换, 每4个字节倒序1234 - >4321
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

			// 下一个对象
	//		pNode = (LPNode)pNode->m_pNext;
			}

		}			
	}


	
	// 3.3写入结束标志
	fwrite(Flag_end,sizeof(Flag_end),1,fp);
	
	// 4.关闭文件
	fclose(fp);

	printf("...结束写入法线记录文件\n\n");
	
	return ret;

}

//lpf 2017-2-16 16:44 make
/** 读取对象顶点的法向量记录文件 大端文件*/
BOOL ReadNormalsFile(LPModel3D pModel, f_char_t* strFileName)
{
	LPObject3D pObject = NULL;
	stList_Head *pstListHead = NULL;
	stList_Head *pstTmpList = NULL;
	char NormalFile[1024] = {0};
	FILE* fp = NULL;
	char readflag[8] = {0}; 
	BOOL ret = FALSE;
	
	// 0./** 如果模型中没有对象，则返回 */
	if(pModel->m_nNumOfObjects <= 0)
		return FALSE;
	
	// 1.打开文件
	memcpy(NormalFile, strFileName, 1024);	
#ifndef _LITTLE_ENDIAN_
	strncat(NormalFile, "NormalPPC", 9);
#else
	strncat(NormalFile, "Normal", 6);
#endif
	fp = fopen(NormalFile,"rb");
	
	// 2.设置返回值
	if(fp == NULL)
	{
		printf("打开法线记录文件%s失败\n",NormalFile);
		return FALSE;
	}
	else
	{
//		printf("打开法线记录文件成功\n");
	}

	// 3.0.读入开始标志,根据前8个字节判断是否非法
	fread(&readflag, sizeof(readflag), 1, fp);

	if(strncmp(readflag, "AABB",4) != 0)
	{
		printf("read normal file begin failed \n");
		fclose(fp);
		return FALSE;

	}
	
	// 3.读取法线信息
	// 3.1遍历模型中所有的对象 
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
				
			// 3.2 读取每个对象的法线

#ifdef _LITTLE_ENDIAN_
#if 1
			{
				// PC下读取大端数据，需要读取后进行大小端转换

				int i =0;
				PT_3F temp_convert = {0};
				float * pt =(float *) &(pObject->m_pNormals[0]);
				float		temp_pt = 0.0f;
				char read_convert[4] = {0};
				
				//大小端转换
				for(i=0; i< pObject->m_nNumOfVerts * 3; i++)
				{
					//pc下不能直接读取大端的float数据
					if (fread(read_convert, sizeof(float) ,1,fp) != 1)
					{

						printf("the model normal file read error!\n");
						fclose(fp);	
						return FALSE;
					}

					//大小端转换
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
			// 下一个对象
	//		pNode = (LPNode)pNode->m_pNext;
				}
		}	




		
	}
	
	
	// 3.3写入结束标志
	fread(&readflag, sizeof(readflag), 1, fp);

	if(strncmp(readflag, "BBAA",4) != 0)
	{
		printf("read normal file end failed \n");
		fclose(fp);
		return FALSE;
	}
	
	// 4.关闭文件
	fclose(fp);

//	printf("...结束读取法线记录文件\n\n");
	
	return TRUE;

}
