#include <stdio.h>
#include "../engine/filepool.h"
#include "../engine/mapRender.h"
#include "../engine/tqtfile.h"
#include "../engine/tqsfile.h"
#include "../engine/vqtfile.h"
#include "../engine/terfile.h"
#include "../engine/qtnfile.h"
#include "../engine/infofile.h"
#include "../engine/atmosphere.h"
#include "appHead.h"
#include "mapApp.h"
#include "initData.h"
#include "../../../include_ttf/ttf_api.h"
#include "../engine/Roam.h"
#include "../engine/memoryPool.h"


#define __tqtfileGetLevels      	terfileGetLevels

#ifdef WIN32					//windowsƽ̨
#define __TERFILEEXT        	"*.ter"
#else						//��windowsƽ̨
#define __TERFILEEXT        	".ter"
#endif

#ifdef USE_DDS_TEXTURE  		/*ʹ��DDS��ʽ������*/

#define __imgTqtIsExist         	imgTqsIsExist

#define __tqtfileOpen           	tqsfileOpen
#define __loadtqt               		loadtqs      
#define __tqt2tex               		tqs2tex      

#define __TQTFILEEXT        	".tqs"
#define __CLRFILEEXT            	".crh"

#else

#define __imgTqtIsExist         	imgTqtIsExist

#define __tqtfileOpen           	tqtfileOpen
#define __loadtqt               		loadtqt      
#define __tqt2tex               		tqt2tex      

#define __TQTFILEEXT        	".tqt"
#define __CLRFILEEXT            	".clr"

#endif


extern sSCREENPARAM screen_param;

/**
 * @fn f_int32_t set3dData(sMAPHANDLE *pScene, f_char_t *path, sMAPFILE mfile)
 *  @brief 3d���ݳ�ʼ��.
 *  @param[in] pScene ��ͼ���ݳ���
 *  @param[in] path   �ӿڶ�Ӧ3d����·���ĸ�Ŀ¼.
 *  @param[in] mfile   ��ͼ�����ļ��б�.
 *  @exception void.
 *  @return f_int32_t,�ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ʧ��.
 * @see 
 * @note
*/
static f_int32_t set3dData(sMAPHANDLE *pScene, f_char_t *pathinput, sMAPFILE mfile)
{
	f_int32_t i = 0;
	char path[256] = {0};
	
	if((NULL == pScene))
	    return -1;
 
	/* ����ter�ļ���������̬�����Ӧ���������� */
	pScene->map_trees = (sMAPTREE *)NewFixedMemory(sizeof(sMAPTREE) * mfile.fcount);
	if(NULL == pScene->map_trees)
	{
	    DEBUG_PRINT("malloc error.")
	    return -1;
	}
	
	for (i = 0; i < mfile.fcount; i++)
	{
		f_char_t buf[256] = {0};		
		pScene->map_trees[i].rootlevel = mfile.fName_list[i].rootlevel;
		pScene->map_trees[i].rootxidx  = mfile.fName_list[i].rootxidx;
		pScene->map_trees[i].rootyidx  = mfile.fName_list[i].rootyidx;
		pScene->map_trees[i].level     = mfile.fName_list[i].level;

		/* ��ȡter�ļ����ڵ��ļ���·���������path�� */
#ifdef WIN32
		strcpy(path , pathinput);
		strcat(path, "/");
		strcat(path, mfile.fName_list[i].pathname);
		strcat(path, "/");
#else
		//strcat(path, "/");
		strcpy(path, mfile.fName_list[i].pathname);
#endif
		
		/* ��ȡter�ļ����ļ���(����·��)�������buf�� */
        sprintf(buf, "%s%d_%d_%d_%d.ter", path, 
                pScene->map_trees[i].rootlevel, 
				pScene->map_trees[i].rootyidx,
                pScene->map_trees[i].rootxidx,                 
                pScene->map_trees[i].level);	
		/* ��ȡter�ļ�������ter�ļ���ͷ��Ϣ��ter�ļ���ŵ��������и�����Ƭ�ĸ߳���Ϣ */
		pScene->map_trees[i].terfile = terfileOpen(pScene->map_trees[i].rootlevel, 
                                                   pScene->map_trees[i].rootxidx, 
                                                   pScene->map_trees[i].rootyidx, 
			                                       buf);
		/* ��ȡter�ļ�ʧ�ܣ���ӡ������Ϣ���������������������ļ��Ķ�ȡ��������ȡ��һ������ */
        if(NULL == pScene->map_trees[i].terfile)                                      
		{
			f_char_t tbuf[256] = {0};
			sprintf(tbuf, "!!Cant Find %s.", buf);
			DEBUG_PRINT(tbuf)
			continue;
		}

		/* Ŀǰ����ʹ��qtn�ļ�����ά��������sqlite���ݿ� */
#if 0
		/* ��ȡqtn�ļ����ļ���(����·��)�������buf�� */
		sprintf(buf, "%s%d_%d_%d_%d.qtn", path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);
		/* ��ȡqtn�ļ�������qtn�ļ���ͷ��Ϣ��qtn�ļ���ŵ��������и�����Ƭ�ĵ�����Ϣ */
		qtnAddFile(buf, pScene->map_trees[i].rootlevel, pScene->map_trees[i].rootxidx, pScene->map_trees[i].rootyidx);
#endif

#ifdef USE_VQT
		/* ��ȡvqt�ļ����ļ���(����·��)�������buf�� */
		sprintf(buf, "%s%d_%d_%d_%d.vqt", path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);
		/* ��ȡvqt�ļ�������vqt�ļ���ͷ��Ϣ��vqt�ļ���ŵ��������и�����Ƭ��ʸ��դ������ */
		pScene->map_trees[i].vqtfile = vqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);
#endif	
		
#ifdef USE_CRH
		/* ��ȡcrh�ļ����ļ���(����·��)�������buf�� */
		sprintf(buf, "%s%d_%d_%d_%d"__CLRFILEEXT, path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);
		/* ��ȡcrh�ļ�������crh�ļ���ͷ��Ϣ��crh�ļ���ŵ��������и�����Ƭ�ĵ����������� */
		pScene->map_trees[i].clrfile = __tqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);
#endif

#ifdef USE_HIL
		/* ��ȡhil�ļ����ļ���(����·��)�������buf�� */
		sprintf(buf, "%s\\%d_%d_%d_%d.hil", path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);

		/* ��ȡhil�ļ�������hil�ļ���ͷ��Ϣ��hil�ļ���ŵ��������и�����Ƭ���������� */
		pScene->map_trees[i].hilfile = __tqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);

#endif

		/* ��ȡtqs�ļ����ļ���(����·��)�������buf�� */
		sprintf(buf, "%s%d_%d_%d_%d"__TQTFILEEXT, path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx,
			pScene->map_trees[i].rootxidx,                 
			pScene->map_trees[i].level);
		/* ��ȡtqs�ļ�������tqs�ļ���ͷ��Ϣ��tqs�ļ���ŵ��������и�����Ƭ������Ӱ������ */
		pScene->map_trees[i].texfile = __tqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);
        
		/* ��ȡinfo�ļ����ļ���(����·��)�������buf�� */
		sprintf(buf, "%s%d_%d_%d_%d.info", path, 
			pScene->map_trees[i].rootlevel,
			pScene->map_trees[i].rootyidx,
			pScene->map_trees[i].rootxidx,                 
			pScene->map_trees[i].level);
		/* ��ȡinfo�ļ�������info�ļ���ͷ��Ϣ��info�ļ���ŵ��������и�����Ƭ���ĸ��ǵ�����ĵ�ľ�γ�� */
		/* ��Ҫ���������ʵ�ʰ����Ĳ㼶�� */
		pScene->map_trees[i].infofile = infofileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			__tqtfileGetLevels(pScene->map_trees[i].terfile), 
			buf);
		
		/* �������������������ʽ�������������ȫ���γ�8���� */
	    createMapSubtreeList(pScene, 
	                 pScene->map_trees[i].rootlevel, 
                     pScene->map_trees[i].rootxidx, 
                     pScene->map_trees[i].rootyidx, 
                     pScene->map_trees[i].level, 
                     (NODETERRAINLOADFUNC)loadterfile, (f_uint64_t)(pScene->map_trees[i].terfile),
                     (NODEINFOLOADFUNC)loadinfofile, (f_uint64_t)(pScene->map_trees[i].infofile),
			         (NODEIMGISEXISTFUNC)imgTerIsExist, 
			         
					 (NODEIMGLOADFUNC)__loadtqt, (NODEIMG2TEXFUNC)__tqt2tex, (f_uint64_t)(pScene->map_trees[i].texfile),
					 
					 (NODEIMGLOADFUNC)__loadtqt, (NODEIMG2TEXFUNC)tqs2texRGBA, (f_uint64_t)(pScene->map_trees[i].vqtfile),
					 
					 (NODEIMGLOADFUNC)__loadtqt, (NODEIMG2TEXFUNC)__tqt2tex, (f_uint64_t)(pScene->map_trees[i].clrfile),

#ifdef USE_DDS_TEXTURE
					 (NODEIMGLOADFUNC)NULL, (NODEIMG2TEXFUNC)__tqt2tex, (f_uint64_t)(pScene->map_trees[i].hilfile)
#else
					 (NODEIMGLOADFUNC)NULL, (NODEIMG2TEXFUNC)vqt2tex, (f_uint64_t)(pScene->map_trees[i].hilfile)
#endif
			);
					         
	}

#ifdef USE_SQLITE
	/*���ó�����ͼ��0�ĸ������ܺ���*/
	qtnSetLayerFunc(pScene, 0);
#endif
	 
	return 0;
}

/**
 * @fn f_int32_t init3dData(sMAPHANDLE *pScene, f_char_t *path)
 *  @brief 3d���ݳ�ʼ��.
 *  @param[in] pScene ��ͼ���ݳ���
 *  @param[in] path   �ӿڶ�Ӧ3d����·���ĸ�Ŀ¼.  
 *  @exception void.
 *  @return f_int32_t,��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1 ʧ��.
 * @see 
 * @note
*/
static f_int32_t init3dData(sMAPHANDLE* pScene, f_char_t *path)
{
	f_int32_t ret = -1;
	sMAPFILE mfile;    /* ��ͼ�����ļ��б� */

	if((NULL == pScene) || (NULL == path))
	    return -1;

	/* ��3d����·���²��Һ�׺��Ϊter���ļ������������ļ�����ŵ���ͼ�����ļ��б��� */
#ifdef WIN32
	mfile.fcount = getSortedFile(path, &(mfile.fName_list), __TERFILEEXT);
#else
	mfile.fcount = getSortedFile(path, &(mfile.fName_list), __TERFILEEXT);
#endif

	/* ���û���ҵ���ֱ�ӷ��� */
	if(mfile.fcount <= 0)
		return -1;
	
	/* Ԥ��ȡ3d���ݣ������������������ø��������ľ�����Ϣ������ȫ���������� */
	ret = set3dData(pScene, path, mfile);
	
	return(ret);
}

/**
 * @fn f_int32_t initMapData(f_char_t *data_path)
 *  @brief ��ͼ���ݳ�ʼ��.
 *  @param[in] render_scene ��ͼ���ݳ���
 *  @param[in] data_path ��ͼ����·���ĸ�Ŀ¼.
 *  @exception void
 *  @return f_int32_t, ��ʼ���ɹ���־.
 *  @retval 0 �ɹ�.
 *  @retval -1: �������Ϊ��,ʧ��.
 * @see 
 * @note
*/
extern VOIDPtr pMapDataSubTree;			/*��ͼ������*/

f_int32_t initMapData(f_char_t *data_path)
{
	f_int32_t ret = 0;
	sMAPHANDLE * pMapHandle = NULL;

	/* �жϵ�ͼ�������Ƿ�Ϊ�� */
	pMapHandle = maphandlePtrValid(pMapDataSubTree);
	if(NULL == pMapHandle)
	    return(-1);
	
	/* �ļ��س�ʼ�� */
	filepoolInit();    

	/* 3d���ݳ�ʼ�� */
	ret = init3dData(pMapHandle, data_path);

	return(ret);
}

/**
 * @fn BOOL createTtfFont(f_char_t *font_path, f_int32_t font_size, f_int32_t edge_size,\n
                          f_float32_t color_font[4], f_float32_t color_edge[4])
 *  @brief ���崴��.
 *  @param[in] scene ��ͼ���ݳ���
 *  @param[in] font_path �ֿ��ļ�·���������ļ�����.
 *  @param[in] font_file �ֿ��ļ���.
 *  @param[in] font_size �����С.
 *  @param[in] edge_size �����Ե��С.
 *  @param[in] color_font ������ɫ.
 *  @param[in] color_edge �߿���ɫ.
 *  @exception void
 *  @return BOOL,���崴���ɹ���־.
 *  @retval TRUE �ɹ�.
 *  @retval FALSE ʧ��.
 * @see 
 * @note 
*/
BOOL createTtfFont(f_char_t *font_path, f_char_t *font_file, 
                   f_int32_t font_size, f_int32_t edge_size, f_float32_t color_font[4], f_float32_t color_edge[4])
{
	f_int32_t i = 0;
	static BOOL is_lib_loaded = FALSE;
//	sGLRENDERSCENE *pScene = NULL;
	f_char_t buf[256] = {0};
	f_char_t fontbuf[256] = {0};
	sMAPHANDLE * pMapHandle = NULL;
	
//	pScene = scenePtrValid(scene);

	pMapHandle = maphandlePtrValid(pMapDataSubTree);

	
	if(NULL == pMapHandle)
	    return(FALSE);
	    
	if(FALSE == is_lib_loaded)
	{
				
#ifdef _LITTLE_ENDIAN_		
    {
        #if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
                sprintf(buf, "%s/%s", font_path, "ansi2unicode_win.lib");
        #else
                sprintf(buf, "%s\\%s", font_path, "ansi2unicode_win.lib");
        #endif
    }
#else
    sprintf(buf, "%s/%s", font_path, "ansi2unicode_ppc.lib");
#endif        
	    if(ttfLoadUnicodeLib(buf) < 0)
        {
    	    DEBUG_PRINT("ansi2unicodelib load failed.")
    	    return(FALSE);
        }
        else	
            is_lib_loaded = TRUE;
    }

    #if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        sprintf(fontbuf, "%s/%s", font_path, font_file);
    #else
         sprintf(fontbuf, "%s\\%s", font_path, font_file);
    #endif

    pMapHandle->ttf_font.size = font_size;
    pMapHandle->ttf_font.edge = edge_size;
	for(i = 0; i < 4; i++)
	{
        pMapHandle->ttf_font.color_font[i] = color_font[i];
        pMapHandle->ttf_font.color_edge[i] = color_edge[i];
	}
    if(ttfCreateFontSU(fontbuf, pMapHandle->ttf_font.size, pMapHandle->ttf_font.edge, screen_param.rotate_type, &(pMapHandle->ttf_font.font))!=0)
	{
		DEBUG_PRINT("freetype font create failed!")
		return(FALSE); 
	}
	
#ifdef USE_SQLITE
	/*ȫ��qtn��Ϣ�����ʼ��*/
    qtnInit();
#endif
    
    return(TRUE);
}

