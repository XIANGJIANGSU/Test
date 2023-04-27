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

#ifdef WIN32					//windows平台
#define __TERFILEEXT        	"*.ter"
#else						//非windows平台
#define __TERFILEEXT        	".ter"
#endif

#ifdef USE_DDS_TEXTURE  		/*使用DDS格式的纹理*/

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
 *  @brief 3d数据初始化.
 *  @param[in] pScene 地图数据场景
 *  @param[in] path   视口对应3d数据路径的根目录.
 *  @param[in] mfile   地图数据文件列表.
 *  @exception void.
 *  @return f_int32_t,成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
 * @see 
 * @note
*/
static f_int32_t set3dData(sMAPHANDLE *pScene, f_char_t *pathinput, sMAPFILE mfile)
{
	f_int32_t i = 0;
	char path[256] = {0};
	
	if((NULL == pScene))
	    return -1;
 
	/* 根据ter文件个数，动态申请对应个数的子树 */
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

		/* 获取ter文件所在的文件夹路径，存放在path中 */
#ifdef WIN32
		strcpy(path , pathinput);
		strcat(path, "/");
		strcat(path, mfile.fName_list[i].pathname);
		strcat(path, "/");
#else
		//strcat(path, "/");
		strcpy(path, mfile.fName_list[i].pathname);
#endif
		
		/* 获取ter文件的文件名(包含路径)，存放在buf中 */
        sprintf(buf, "%s%d_%d_%d_%d.ter", path, 
                pScene->map_trees[i].rootlevel, 
				pScene->map_trees[i].rootyidx,
                pScene->map_trees[i].rootxidx,                 
                pScene->map_trees[i].level);	
		/* 读取ter文件，解析ter文件的头信息，ter文件存放的是子树中各个瓦片的高程信息 */
		pScene->map_trees[i].terfile = terfileOpen(pScene->map_trees[i].rootlevel, 
                                                   pScene->map_trees[i].rootxidx, 
                                                   pScene->map_trees[i].rootyidx, 
			                                       buf);
		/* 读取ter文件失败，打印错误信息，跳过该子树其他类型文件的读取，继续读取下一棵子树 */
        if(NULL == pScene->map_trees[i].terfile)                                      
		{
			f_char_t tbuf[256] = {0};
			sprintf(tbuf, "!!Cant Find %s.", buf);
			DEBUG_PRINT(tbuf)
			continue;
		}

		/* 目前不再使用qtn文件，三维地名改用sqlite数据库 */
#if 0
		/* 获取qtn文件的文件名(包含路径)，存放在buf中 */
		sprintf(buf, "%s%d_%d_%d_%d.qtn", path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);
		/* 读取qtn文件，解析qtn文件的头信息，qtn文件存放的是子树中各个瓦片的地名信息 */
		qtnAddFile(buf, pScene->map_trees[i].rootlevel, pScene->map_trees[i].rootxidx, pScene->map_trees[i].rootyidx);
#endif

#ifdef USE_VQT
		/* 获取vqt文件的文件名(包含路径)，存放在buf中 */
		sprintf(buf, "%s%d_%d_%d_%d.vqt", path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);
		/* 读取vqt文件，解析vqt文件的头信息，vqt文件存放的是子树中各个瓦片的矢量栅格化纹理 */
		pScene->map_trees[i].vqtfile = vqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);
#endif	
		
#ifdef USE_CRH
		/* 获取crh文件的文件名(包含路径)，存放在buf中 */
		sprintf(buf, "%s%d_%d_%d_%d"__CLRFILEEXT, path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);
		/* 读取crh文件，解析crh文件的头信息，crh文件存放的是子树中各个瓦片的地形晕渲纹理 */
		pScene->map_trees[i].clrfile = __tqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);
#endif

#ifdef USE_HIL
		/* 获取hil文件的文件名(包含路径)，存放在buf中 */
		sprintf(buf, "%s\\%d_%d_%d_%d.hil", path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx, 
			pScene->map_trees[i].rootxidx,               
			pScene->map_trees[i].level);

		/* 读取hil文件，解析hil文件的头信息，hil文件存放的是子树中各个瓦片的其他纹理 */
		pScene->map_trees[i].hilfile = __tqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);

#endif

		/* 获取tqs文件的文件名(包含路径)，存放在buf中 */
		sprintf(buf, "%s%d_%d_%d_%d"__TQTFILEEXT, path, 
			pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootyidx,
			pScene->map_trees[i].rootxidx,                 
			pScene->map_trees[i].level);
		/* 读取tqs文件，解析tqs文件的头信息，tqs文件存放的是子树中各个瓦片的卫星影像纹理 */
		pScene->map_trees[i].texfile = __tqtfileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			buf);
        
		/* 获取info文件的文件名(包含路径)，存放在buf中 */
		sprintf(buf, "%s%d_%d_%d_%d.info", path, 
			pScene->map_trees[i].rootlevel,
			pScene->map_trees[i].rootyidx,
			pScene->map_trees[i].rootxidx,                 
			pScene->map_trees[i].level);
		/* 读取info文件，解析info文件的头信息，info文件存放的是子树中各个瓦片的四个角点加中心点的经纬高 */
		/* 需要传入该子树实际包含的层级数 */
		pScene->map_trees[i].infofile = infofileOpen(pScene->map_trees[i].rootlevel, 
			pScene->map_trees[i].rootxidx, 
			pScene->map_trees[i].rootyidx, 
			__tqtfileGetLevels(pScene->map_trees[i].terfile), 
			buf);
		
		/* 将各个子树以链表的形式组合起来，最终全球形成8棵树 */
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
	/*设置场景中图层0的各个功能函数*/
	qtnSetLayerFunc(pScene, 0);
#endif
	 
	return 0;
}

/**
 * @fn f_int32_t init3dData(sMAPHANDLE *pScene, f_char_t *path)
 *  @brief 3d数据初始化.
 *  @param[in] pScene 地图数据场景
 *  @param[in] path   视口对应3d数据路径的根目录.  
 *  @exception void.
 *  @return f_int32_t,初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1 失败.
 * @see 
 * @note
*/
static f_int32_t init3dData(sMAPHANDLE* pScene, f_char_t *path)
{
	f_int32_t ret = -1;
	sMAPFILE mfile;    /* 地图数据文件列表 */

	if((NULL == pScene) || (NULL == path))
	    return -1;

	/* 在3d数据路径下查找后缀名为ter的文件数量，并把文件名存放到地图数据文件列表中 */
#ifdef WIN32
	mfile.fcount = getSortedFile(path, &(mfile.fName_list), __TERFILEEXT);
#else
	mfile.fcount = getSortedFile(path, &(mfile.fName_list), __TERFILEEXT);
#endif

	/* 如果没查找到，直接返回 */
	if(mfile.fcount <= 0)
		return -1;
	
	/* 预读取3d数据，创建各个子树，设置各个子树的具体信息，构建全局子树链表 */
	ret = set3dData(pScene, path, mfile);
	
	return(ret);
}

/**
 * @fn f_int32_t initMapData(f_char_t *data_path)
 *  @brief 地图数据初始化.
 *  @param[in] render_scene 地图数据场景
 *  @param[in] data_path 地图数据路径的根目录.
 *  @exception void
 *  @return f_int32_t, 初始化成功标志.
 *  @retval 0 成功.
 *  @retval -1: 场景句柄为空,失败.
 * @see 
 * @note
*/
extern VOIDPtr pMapDataSubTree;			/*地图场景树*/

f_int32_t initMapData(f_char_t *data_path)
{
	f_int32_t ret = 0;
	sMAPHANDLE * pMapHandle = NULL;

	/* 判断地图场景树是否为空 */
	pMapHandle = maphandlePtrValid(pMapDataSubTree);
	if(NULL == pMapHandle)
	    return(-1);
	
	/* 文件池初始化 */
	filepoolInit();    

	/* 3d数据初始化 */
	ret = init3dData(pMapHandle, data_path);

	return(ret);
}

/**
 * @fn BOOL createTtfFont(f_char_t *font_path, f_int32_t font_size, f_int32_t edge_size,\n
                          f_float32_t color_font[4], f_float32_t color_edge[4])
 *  @brief 字体创建.
 *  @param[in] scene 地图数据场景
 *  @param[in] font_path 字库文件路径（不含文件名）.
 *  @param[in] font_file 字库文件名.
 *  @param[in] font_size 字体大小.
 *  @param[in] edge_size 字体边缘大小.
 *  @param[in] color_font 字体颜色.
 *  @param[in] color_edge 边框颜色.
 *  @exception void
 *  @return BOOL,字体创建成功标志.
 *  @retval TRUE 成功.
 *  @retval FALSE 失败.
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
	/*全局qtn信息链表初始化*/
    qtnInit();
#endif
    
    return(TRUE);
}

