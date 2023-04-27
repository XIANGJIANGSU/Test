#ifdef WIN32
    #include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
    #include "vxworks.h"
#endif
#include <math.h>
//#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include "../projection/project.h"
#include "../../../include_ttf/ttf_api.h"
#include "../geometry/matrixMethods.h"
#include "../projection/coord.h"
#include "filepool.h"
#include "mapRender.h"
#include "qtnfile.h"
#include "qtnsort.h"
#include "textCollision.h"
#include "memoryPool.h"

extern sSCREENPARAM screen_param;

#define QTNFILEKEY  0x74717494
#define MAXQTNLAYER	8
#define VERIFYQTNFILE(qtn) ( (qtn != NULL) && (qtn->skey == QTNFILEKEY) && (qtn->pFileItem != NULL) )
#define MAPNOTECLASS	6
/*
// 序号	名称	       主码	  识别码	字体	颜色	        大小	倾斜	图层名	 列表
// 1			省级	   	50	  		501,502	黑体	50,50, 50	    28	     F	    地名	 0
// 2			地州级	50	  		503,504	黑体	50,50,50	    24	     F	    地名	 1
// 3			县级	   	50	  		505	    黑体	50, 50,50	    20	     F	    地名	 2
// 4			乡镇	   	50	  		506,507	楷体	50,50, 50	    16	     F	    地名	 3,4
// 5			村以下	50	  		其他	    宋体	50, 50, 50	    13	     F	    地名	 5
// 6			其他	   	/	   		/	    宋体	50, 50, 50	    12	     F	    /	     6
*/

typedef struct tagQtnTileInfo{
	f_uint64_t offset;                /* 瓦片在文件中的偏移地址 */
	f_uint64_t length;                /* 瓦片数据的长度(字节数) */
}sQtnTileInfo;

typedef struct tagQTNFile
{
	f_uint64_t skey; /*must be QTNFILEKEY*/
	FILEITEM * pFileItem;
	f_uint64_t ver;
	f_uint64_t layer;
	f_uint64_t piccount;
	f_uint64_t tilestart[MAXQTNLAYER];
	sQtnTileInfo tileinfo[1];
}QTNFile;

typedef struct sQTNINFO
{
	struct sQTNINFO * pNext;
	QTNFILE qtnfile;
	f_int32_t rootlevel;
	f_int32_t rootxidx;
	f_int32_t rootyidx;
}QTNINFO;

typedef struct tagNameList
{
	struct tagNameList * pNext;
	f_uint16_t * pText;
	f_int32_t textlen;
	f_uint16_t code;
	f_float32_t px, py, pz;
}sNameList;

typedef struct tagMapNote
{
	sNameList notelist[MAPNOTECLASS];
	f_int32_t needrefresh;
}sMapNote;

#ifdef USE_SQLITE

const f_uint16_t mapsym[] = 
{
	0x2605,  /* 外国首都 */
    0x2605,  /* 首都     501*/
	0x25c6,  /* 重要城市 502*/
	0x25c6,  /* 重要城市 503*/  
	0x25ce,  /* 主要城市 504*/ 
	0x25c7,  /* 一般城市 505*/
	0x25cb,  /* 其它居民地 506 */
	0x25cb,  /* 其它居民地 507 */
	0x25cb,  /* 其它居民地 508 */
	0x25cb,  /* 其它居民地 509 */
};

#define CHAR_NUM 12
f_int32_t fntwidth[CHAR_NUM]; /* 第0个是汉字的宽度，1~9是与mapsym对应符号的宽度，10是数字的宽度，11是km的宽度 */
f_int32_t fntheight[CHAR_NUM];

/* added by zqx 2016-10-8 14:43 start */
#define MAX_QTNNUM_DISPLAY_PERLAYER_IN_CURFRAME  500    /* 当前帧每层可能要显示的最大注记数（碰撞检测前） */
sQTNText qtntext[MAPNOTECLASS][MAX_QTNNUM_DISPLAY_PERLAYER_IN_CURFRAME];
f_int32_t qtn_count[MAPNOTECLASS];

#define MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME  1000       /* 当前帧需显示的最大注记数（碰撞检测后） */
sQTNText qtndisp[MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME];
f_int32_t qtn_display_count;

/* added by zqx 2016-10-8 14:43 end */

static f_float64_t g_matrix[16];
static QTNINFO qtninfo;

static f_int32_t qtncreatenode(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint64_t *pNodeID);
// static f_int32_t qtndisplaynode(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid,
//                                 Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4]);
static f_int32_t qtndisplaynode(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid,
                                Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4], sSqliteName*  pList, f_int32_t* QtnNum);

static f_int32_t qtndisplaypre(f_int32_t view_type, sTtfFont tfont, f_int32_t viewport[4], f_float64_t matrix[16]);
static f_int32_t qtndisplaypro(f_int32_t view_type, sTtfFont tfont);

/*
	打开一个QTN文件，
	参数：cstrFileName -- 文件名称
	      pQTNFile -- 如果成功则返回文件句柄供后续调用使用
	返回值：
	     0 -- 成功，文件句柄在pQTNFile中
		-1 -- 文件打开失败
		-2 -- 文件格式错误
		-3 -- 内存分配失败
*/
f_int32_t qtnfileOpen(const f_char_t * cstrFileName, QTNFILE * pQTNFile)
{
	FILE * pFile;
	QTNFile * pQtn;
	f_uint8_t filehead[16], buf[8];
	f_uint64_t ver;
	f_uint64_t layer;
	f_uint64_t piccount;
	f_uint64_t i;
	if (pQTNFile == NULL)
		return -1;
	pFile = fopen(cstrFileName, "rb");
	if (pFile == NULL)
		return -1;
	if (fread(filehead, 1, 16, pFile) != 16)
	{
	    fclose(pFile);
    	return -2;
	}
	if ( (filehead[0] != 'q') || (filehead[1] != 't') || (filehead[2] != 'n') )
	{
	    fclose(pFile);
    	return -2;
	}
	ver = filehead[4] + 
		  filehead[5] * 256lu + 
		  filehead[6] * 256lu * 256lu + 
		  filehead[7] * 256lu * 256lu * 256lu;
	/*version must be 0x0000ff04*/
	//if (ver != 0x0000ff04)
	//{
	//    fclose(pFile);
    //	return -2;
	//}

	layer = filehead[8] + 
		  filehead[9] * 256lu + 
		  filehead[10] * 256lu * 256lu + 
		  filehead[11] * 256lu * 256lu * 256lu;
	/*layer count must between 1 to MAXQTNLAYER*/
	if ( (layer == 0) || (layer > MAXQTNLAYER) )
	{
	    fclose(pFile);
    	return -2;
	}
    
	piccount = 0;
	for (i = 0;i < layer;i++)
	{
		piccount += 1 << (i * 2);
	}
	
	pQtn = (QTNFile *)NewAlterableMemory(sizeof(QTNFile) + piccount * sizeof(sQtnTileInfo));
	if (pQtn == NULL)
	{
		fclose(pFile);
		return -3;
	}
	memset(pQtn, 0, sizeof(QTNFile));
	pQtn->skey = QTNFILEKEY;
	pQtn->ver = ver;
	pQtn->layer = layer;
	pQtn->piccount = piccount;
	piccount = 0;
	for (i = 0;i < layer;i++)
	{
		pQtn->tilestart[i] = piccount;
		piccount += 1 << (i * 2);
	}
	for (i = 0;i<piccount;i++)
	{		
		if (fread(buf, 1, 8, pFile) != 8)
		{
			DeleteAlterableMemory(pQtn);
			fclose(pFile);
	        return -2;
		}
		pQtn->tileinfo[i].offset = buf[0] + 
							       buf[1] * 256lu + 
							       buf[2] * 256lu * 256lu + 
							       buf[3] * 256lu * 256lu * 256lu;
        pQtn->tileinfo[i].length = buf[4] + 
							      buf[5] * 256lu + 
							      buf[6] * 256lu * 256lu + 
							      buf[7] * 256lu * 256lu * 256lu;							 
	}
	fclose(pFile);
	pQtn->pFileItem = filepoolOpen(cstrFileName, 1);
	*pQTNFile = pQtn;
	return 0;
}

static void qtnDeInitFont()
{

}

/*
	关闭一个已经打开的QTN文件，释放文件占用的资源
	参数：qtnfile -- 用qtnfileOpen返回的QTN文件句柄
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
*/
f_int32_t qtnfileClose(QTNFILE qtnfile)
{
	QTNFile * pQtn;
	pQtn = (QTNFile *)qtnfile;
	if(!VERIFYQTNFILE(pQtn))
	{
		DEBUG_PRINT("pQtn error.");
	    return -1;
	}
	if (pQtn->pFileItem != NULL)
		filepoolClose(pQtn->pFileItem);
	memset(pQtn, 0, sizeof(QTNFile));
	DeleteAlterableMemory(pQtn);
	return 0;
}

/*
	得到一个已经打开的QTN文件中树的层数
	参数：qtnfile -- 用qtnfileOpen返回的QTN文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
f_int32_t qtnfileGetLayerCount(QTNFILE qtnfile)
{
	QTNFile * pQtn;
	pQtn = (QTNFile *)qtnfile;
	if(!VERIFYQTNFILE(pQtn))
	{
		DEBUG_PRINT("pQtn error.");
	    return -1;
	}
	return pQtn->layer;
}

/*
	得到一个QTN文件中的基本图块数据
	参数：qtnfile -- 用qtnfileOpen返回的QTN文件句柄
	      layer -- 指定的层，从0开始
		  xidx, yidx -- 基本图块在层中的编号
		  pData -- 返回的数据，如果成功返回
	返回值：
	     1 -- 数据正在调入
	     0 -- 成功
		-1 -- 文件句柄无效
		-2 -- 层号无效
		-3 -- xidx或yidx无效
		-4 -- 内存分配错误
		-5 -- 数据解码错误
*/
f_int32_t qtnfileGetData(QTNFILE qtnfile, f_int32_t layer, f_int32_t xidx, f_int32_t yidx, void ** pData)
{
	f_uint64_t offset;
	f_uint8_t * pD;
	QTNFile * pQtn;
	FILE *pFile;
	if (pData == NULL)
		return -4;
	*pData = NULL;
	pQtn = (QTNFile *)qtnfile;
	if(!VERIFYQTNFILE(pQtn))
	{
		DEBUG_PRINT("pQtn error.");
	    return -1;
	}

	/*全局编号转成局部编号*/
	if ( (layer < 0) || (layer >= (f_int32_t)pQtn->layer) )
		return -2;
	if ( (xidx < 0 ) || (yidx < 0) || (xidx >= (1 << layer)) || (yidx >= (1 << layer)) )
		return -2;
	offset = pQtn->tilestart[layer] + yidx * (1 << layer) + xidx;
	
	pFile = filepoolGetFile(pQtn->pFileItem);
	if (pFile == 0)
		return -1;
	
	if(0 == pQtn->tileinfo[offset].length)
	{
		filepoolReleaseFile(pQtn->pFileItem);
		return -1;
	}
	
	if (fseek(pFile, pQtn->tileinfo[offset].offset, SEEK_SET) != 0)
	{
		filepoolReleaseFile(pQtn->pFileItem);
		return -1;
	}
	
	pD = (f_uint8_t *)NewAlterableMemory(pQtn->tileinfo[offset].length + 4);
	if (pD == NULL)
	{
		filepoolReleaseFile(pQtn->pFileItem);
		return -4;
	}
	fread(pD, 1, pQtn->tileinfo[offset].length, pFile);
	//*(f_uint64_t *)pD = fread(pD, 1, pQtn->tileinfo[offset].length, pFile);
	*pData = pD;
	filepoolReleaseFile(pQtn->pFileItem);
	return( (f_int32_t)(pQtn->tileinfo[offset].length));
}

static f_int32_t namelistDestroy(sNameList * pList)
{
	if (pList == NULL)
		return 0;
	namelistDestroy(pList->pNext);
	if (pList->pText != NULL)
		DeleteAlterableMemory(pList->pText);
	DeleteAlterableMemory(pList);
	return 0;
}

static f_int32_t namelistAddName(sNameList * pList, f_float32_t x, f_float32_t y, f_float32_t z, 
                                 f_int32_t str_len, const f_char_t *noteS, unsigned short code)
{
	sNameList * pItem;
	Geo_Pt_D geoPt;
	Obj_Pt_D objPt;
	
	if (noteS == NULL)
		return -2;
	if (str_len == 0)
		return -2;
	pItem = (sNameList *)NewAlterableMemory(sizeof(sNameList));
	if (pItem == NULL)
		return -1;
//	pItem->pText = (unsigned short *)malloc(str_len + 2);
	pItem->pText = (unsigned short *)NewAlterableMemory(256);	
	if (pItem->pText == NULL)
	{
		DeleteAlterableMemory(pItem);
		return -1;
	}
    /* 将注记符号的unicode码与注记合并，在显示时一起显示 changed start by zqx 2016-10-13 14:45*/
	if(code > 0 && code < 10)
	    pItem->pText[0] = mapsym[code];
	else
	    pItem->pText[0] = mapsym[6];
	ttfAnsi2Unicode((unsigned short *)(&(pItem->pText[1])), noteS, &pItem->textlen);
	pItem->textlen++;
	/* 将注记符号的unicode码与注记合并，在显示时一起显示 changed end by zqx 2016-10-13 14:45*/
	//ttfAnsi2Unicode(pItem->pText, noteS, &pItem->textlen); //old
	
	geoPt.lon = x;
	geoPt.lat = y;
	geoPt.height = z + 600.0;/* 600.0该值在实际应用中不需要，目前是因为小钟在出地名注记时没有提取地形高度，导致地名显示在地下。后续让小钟提取高度 */
	geoDPt2objDPt(&geoPt, &objPt);
	pItem->px = objPt.x;
	pItem->py = objPt.y;
	pItem->pz = objPt.z;
	pItem->code = code;
	pItem->pNext = pList->pNext;
	pList->pNext = pItem;
	return 0;
}

static void namelistUpdateCoordParam(f_float64_t *matrix)
{
	f_int32_t i = 0;
	for(i = 0; i < 16; i++)
	    g_matrix[i] = matrix[i];
}


static void drawTextLegLine(double sx, double sy, double sz, double nsx, double nsy, double nsz)
{
	glBegin(GL_LINES);
		glVertex3f(sx, sy, sz);
		glVertex3f(nsx, nsy, nsz);
	glEnd();
}

static void drawBordLine(f_float32_t lb_x, f_float32_t lb_y, f_float32_t lb_z, 
                         f_float32_t rt_x, f_float32_t rt_y, f_float32_t rt_z )
{
	//glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	    glVertex3f(lb_x, lb_y, lb_z);
	    glVertex3f(rt_x, lb_y, rt_z);
	    glVertex3f(rt_x, rt_y, rt_z);
	    glVertex3f(lb_x, rt_y, lb_z);
	glEnd();	
}

static void drawBordBackground(f_float32_t lb_x, f_float32_t lb_y, f_float32_t lb_z, 
                               f_float32_t rt_x, f_float32_t rt_y, f_float32_t rt_z )
{
	//glColor4f(0.3216f, 0.6353f, 0.9804f, 0.3f);
	glBegin(GL_QUADS);
	    glVertex3f(lb_x, lb_y, lb_z);
	    glVertex3f(rt_x, lb_y, rt_z);
	    glVertex3f(rt_x, rt_y, rt_z);
	    glVertex3f(lb_x, rt_y, lb_z);
	glEnd();
}

/*
	将注记加入注记绘制数组中
	参数：code_index--当前处理的注记等级,0-5
	      code--待加入的注记的等级,0-4
	      sx,sy,sz--注记的屏幕坐标1,即标杆与地面的交点
	      nsx,nsy,nsz--注记的屏幕坐标2,即标杆与标牌的交点,注记绘制在标牌上
		  dis--注记与视点的距离
		  textlen--注记长度
		  pString--注记内容
    返回值：无
*/
static void addQtntextIntoArray(f_int32_t code_index, f_int32_t code, f_float64_t sx, f_float64_t sy, f_float64_t sz, 
                                f_float64_t nsx, f_float64_t nsy, f_float64_t nsz, f_float32_t dis, 
                                f_int32_t   textlen, const f_uint16_t * pString)
{
	f_int32_t i = 0;
	/*一帧中待绘制各等级注记的个数最多有500个*/
	if(qtn_count[code_index] < MAX_QTNNUM_DISPLAY_PERLAYER_IN_CURFRAME)
	{
		/*将注记加入到各等级待绘制的注记数组中,注记长度最多为12*/
		qtntext[code_index][qtn_count[code_index]].code = code;
		qtntext[code_index][qtn_count[code_index]].sx  = sx;
		qtntext[code_index][qtn_count[code_index]].sy  = sy;
		qtntext[code_index][qtn_count[code_index]].sz  = sz;
		qtntext[code_index][qtn_count[code_index]].nsx = nsx;
		qtntext[code_index][qtn_count[code_index]].nsy = nsy;
		qtntext[code_index][qtn_count[code_index]].nsz = nsz;
		qtntext[code_index][qtn_count[code_index]].dis = dis;
		if(textlen > 12)
		    textlen = 12;
		qtntext[code_index][qtn_count[code_index]].textlen = textlen;
		for(i = 0; i < textlen; i++)
		    qtntext[code_index][qtn_count[code_index]].text[i] = pString[i];
		    
		qtn_count[code_index]++;
	}
}

#define NOTE_DISPLAY_FARST 150000.0     //注记显示的最远距离（与视点间的距离）
static void namelistOutput(f_int32_t view_type, sTtfFont tfont, f_int32_t code_index, 
                           sNameList * pList, Obj_Pt_D view_pos, f_int32_t viewport[4])
{
	f_int32_t size = tfont.size;
	f_float32_t hfsize = size * 0.5f;
	BOOL ret = FALSE;
	f_float64_t viewx, viewy, viewz, dis;/*注记点的屏幕坐标*/ 
	f_float64_t nviewx, nviewy, nviewz, nobjx, nobjy, nobjz, ratio; 	
		
	while (pList != NULL)
	{
		if (pList->code < MAPNOTECLASS)
		if ( (pList->textlen > 0) && (pList->pText != NULL) )
		{
			
		    dis = 0.0f;			
			dis = (f_float32_t)sqrt((view_pos.x - pList->px) * (view_pos.x - pList->px) + 
					                (view_pos.y - pList->py) * (view_pos.y - pList->py) + 
								    (view_pos.z - pList->pz) * (view_pos.z - pList->pz) );
		    if( view_type != eVM_OVERLOOK_VIEW )
			{
				/* 俯视时，不用距离来限制，透视时才用距离来限制 */
			    if(dis > NOTE_DISPLAY_FARST)
			    {
				    pList = pList->pNext;
				    continue;
			    }
			}
			
			/*转换点物体坐标到屏幕坐标*/
			ret = avicglProject(pList->px, pList->py, pList->pz, g_matrix, viewport, &viewx, &viewy, &viewz);
			if(FALSE == ret)
			{
				pList = pList->pNext;
				continue;
			}

			if(viewz > 1.0f)
			{
				pList = pList->pNext;
				continue;
			}

			ret = isPointInViewport(viewx, viewy, viewport);
			if(0 != ret)
			{
				pList = pList->pNext;
				continue;
			}
			
			if(view_type != eVM_OVERLOOK_VIEW )
		    {
		    	ratio = 1.0 + (dis + 5000.0) / NOTE_DISPLAY_FARST * 0.002;
			    nobjx = ratio * pList->px;
			    nobjy = ratio * pList->py;
			    nobjz = ratio * pList->pz;
			    ret = avicglProject(nobjx, nobjy, nobjz, g_matrix, viewport, &nviewx, &nviewy, &nviewz);
			    if(FALSE == ret)
			    {
				    pList = pList->pNext;
				    continue;
			    }
		    }
		    
		    addQtntextIntoArray(code_index, pList->code, viewx, viewy, viewz, nviewx, nviewy, nviewz, dis, pList->textlen, pList->pText);		    	
		}
		pList = pList->pNext;
	}
}


/*
	计算注记与视点的距离、注记绘制相关的屏幕坐标,若在视口范围内则将其加入注记绘制数组中
	参数：view_type--视角类型
	      tfont--字体句柄
		  code_index--注记等级,0-4
		  pList--数据库中符合条件的所有注记的指针
		  QtnNum--数据库中符合条件的所有注记的数量
		  view_pos--视点位置
		  viewport--视口大小
    返回值：无
*/
static void namelistOutputSql(f_int32_t view_type, sTtfFont tfont, f_int32_t code_index, 
                           sSqliteName * pList, f_int32_t QtnNum, Obj_Pt_D view_pos, f_int32_t viewport[4])
{
	BOOL ret = FALSE;
	/*注记点的屏幕坐标,即标杆与地面的交点*/ 
    f_float64_t viewx = 0.0, viewy = 0.0, viewz = 0.0, dis = 0.0;
	/*标杆与标牌的交点的物体坐标、屏幕坐标*/
    f_float64_t nviewx = 0.0, nviewy = 0.0, nviewz = 0.0, nobjx = 0.0, nobjy = 0.0, nobjz = 0.0, ratio = 0.0;  
	f_int32_t i = 0;

	/*注记指针为空,说明无注记需要处理,直接返回*/
	if (pList == NULL)
	{
		return;
	}

	/*循环处理各个注记*/
	for(i=0; i<QtnNum; i++)
	{
		/*判断注记的code值与当前处理的code值是否一致,不一致则该注记不处理*/
		if(code_index != pList[i].code)
		{
			continue;
		}
		/*注记code值有效,长度大于0,内容不为空时,处理该注记,否则不处理*/
		if( (pList[i].code < MAPNOTECLASS) &&
		    (pList[i].textlen > 0) && (pList[i].pText != NULL) )
		{
			/*计算视点到该注记位置之间的距离*/
		    dis = 0.0f;			
			dis = (f_float32_t)sqrt((view_pos.x - pList[i].px) * (view_pos.x - pList[i].px) + 
					                (view_pos.y - pList[i].py) * (view_pos.y - pList[i].py) + 
								    (view_pos.z - pList[i].pz) * (view_pos.z - pList[i].pz) );
		    /* 俯视时，不用距离来限制，透视时才用距离来限制 */
			if( view_type != eVM_OVERLOOK_VIEW )
			{
				/* 透视时,距离大于150km,该注记不处理 */
			    if(dis > NOTE_DISPLAY_FARST)
			    {
				    //pList[i] = pList[i]->pNext;
				    continue;
			    }
			}
			
			/*注记点的物体坐标转换成屏幕坐标,转换失败或转换后的z值大于1.0f,则该注记不处理*/
			ret = avicglProject(pList[i].px, pList[i].py, pList[i].pz, g_matrix, viewport, &viewx, &viewy, &viewz);
			if(FALSE == ret)
			{
				//pList[i] = pList[i]->pNext;
				continue;
			}
			if(viewz > 1.0f)
			{
				//pList[i] = pList[i]->pNext;
				continue;
			}
			/*判断屏幕坐标是否在视口范围内,不在该注记也不处理*/
			ret = isPointInViewport(viewx, viewy, viewport);
			if(0 != ret)
			{
				//pList[i] = pList[i]->pNext;
				continue;
			}
			
			/*座舱视角下,注记以标杆加标牌的形式绘制,注记点的屏幕坐标实际是标杆与地面的交点*/
			/*还需计算标杆与标牌的交点的屏幕坐标*/
			if( view_type != eVM_OVERLOOK_VIEW )
		    {
				/*控制标杆的长度,即在地心与注记的连线上扩大一定的比例,得到标杆与标牌的交点的物体坐标*/
		    	ratio = 1.0 + (dis + 5000.0) / NOTE_DISPLAY_FARST * 0.002;
			    nobjx = ratio * pList[i].px;
			    nobjy = ratio * pList[i].py;
			    nobjz = ratio * pList[i].pz;
				/*标杆与标牌的交点的物体坐标转换成屏幕坐标*/
			    ret = avicglProject(nobjx, nobjy, nobjz, g_matrix, viewport, &nviewx, &nviewy, &nviewz);
			    if(FALSE == ret)
			    {
				    //pList[i] = pList[i]->pNext;
				    continue;
			    }
		    }
		    /*将该注记加入到待绘制的注记数组中*/
		    addQtntextIntoArray(code_index, pList[i].code, viewx, viewy, viewz, nviewx, nviewy, nviewz, dis, pList[i].textlen, pList[i].pText);		    	
		}
		//pList = pList->pNext;
	}
}


static f_int32_t qtncreatenode(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint64_t *pNodeID)
{
	sMapNote * pNote;
	QTNINFO * pInfo;
	f_uint8_t * pData;
	f_uint8_t * pDataBuf;
	f_int32_t datalen;
	f_int32_t llevel, lxidx, lyidx;
	pData = NULL;
	pNote = NULL;
	*pNodeID = 0;
	/*
		在表中找到对应的文件
	*/
	pInfo = (QTNINFO *)param;
	while (pInfo != NULL)
	{
		if ( (level >= pInfo->rootlevel) && (level < pInfo->rootlevel + qtnfileGetLayerCount(pInfo->qtnfile)) )
		{
			llevel = level - pInfo->rootlevel;
			lxidx  = xidx  - pInfo->rootxidx * (1 << llevel);
			lyidx  = yidx  - pInfo->rootyidx * (1 << llevel);
			if ( (lxidx >= 0) && (lxidx < (1 << llevel)) && (lyidx >= 0) && (lyidx < (1 << llevel)) )
			{
				break;
			}
		}
		pInfo = pInfo->pNext;
	}
	if (pInfo == NULL)
		return -1;
	
	datalen = qtnfileGetData(pInfo->qtnfile, llevel, lxidx, lyidx, &pData);	
	if (datalen < 0)
	{
	    return -1;
	}

	pNote = (sMapNote *)NewAlterableMemory(sizeof(sMapNote));
	if (pNote == NULL)
	{
	    if (pData != NULL)
		    DeleteAlterableMemory(pData);
		return -1;
	}

	memset(pNote, 0, sizeof(sMapNote));
	pDataBuf = pData;

	while (datalen > 14)
	{
		f_int32_t code, len;
		f_float32_t pos[3], fsize;
		sNameList * slist;
		f_char_t noteS[40];
		f_uint8_t * pD;
		code = pDataBuf[0];
		pD = (f_uint8_t *)&pos[0];
#ifndef _LITTLE_ENDIAN_
		pD[0] = pDataBuf[1];
		pD[1] = pDataBuf[2];
		pD[2] = pDataBuf[3];
		pD[3] = pDataBuf[4];
#else
		pD[0] = pDataBuf[4];
		pD[1] = pDataBuf[3];
		pD[2] = pDataBuf[2];
		pD[3] = pDataBuf[1];
#endif
		pD = (f_uint8_t *)&pos[1];
#ifndef _LITTLE_ENDIAN_
		pD[0] = pDataBuf[5];
		pD[1] = pDataBuf[6];
		pD[2] = pDataBuf[7];
		pD[3] = pDataBuf[8];
#else
		pD[0] = pDataBuf[8];
		pD[1] = pDataBuf[7];
		pD[2] = pDataBuf[6];
		pD[3] = pDataBuf[5];
#endif
		pD = (f_uint8_t *)&pos[2];
#ifndef _LITTLE_ENDIAN_
		pD[0] = pDataBuf[9];
		pD[1] = pDataBuf[10];
		pD[2] = pDataBuf[11];
		pD[3] = pDataBuf[12];
#else
		pD[0] = pDataBuf[12];
		pD[1] = pDataBuf[11];
		pD[2] = pDataBuf[10];
		pD[3] = pDataBuf[9];
#endif

		len = pDataBuf[13];
		if (len > 30)
			len = 30;
		memcpy(noteS, pDataBuf + 14, len);
		noteS[len] = 0;
		fsize = 0.0f;
		
		switch(code)
		{
			case 1:
			case 2:			/*/ 省、直辖市 */
				fsize = 0.5f;
				slist = &pNote->notelist[0];
				/*strcat(noteS, "⊙");*/
				break;
			case 3:			/*/ 州驻地*/ 
			case 4:			/*/ 地级市*/ 
				fsize = 0.5f;
				slist = &pNote->notelist[1];
				/*strcat(noteS, "○");*/
				break;
			case 5:			/*/ 县 */
				fsize = 0.5f;
				slist = &pNote->notelist[2];
				/*strcat(noteS, "○");*/
				break;
			case 6:			/*/ 镇 */
			case 7:			/*/ 乡 */
				fsize = 0.5f;
				slist = &pNote->notelist[code==6?3:4];
				/*strcat(noteS, "○");*/
				break;
			default:
				fsize = 0.5f;
				slist = &pNote->notelist[5];
				/*strcat(noteS, "○");*/
				break;
		}
		
		if (fsize > 0.0001f)
			namelistAddName(slist, pos[0], pos[1], pos[2], len, noteS, (unsigned short)code);
		datalen -=  14 + pDataBuf[13];
		pDataBuf += 14 + pDataBuf[13];
	}
	pNote->needrefresh = 1;
	*pNodeID = (f_uint64_t)pNote;
	DeleteAlterableMemory(pData);
	return 0;
}

/*
	处理各个数据库中符合条件的所有注记,将其加入注记绘制数组中
	参数：view_type--视角类型
	      tfont--字体句柄
		  nodeid--暂时无用,始终给0
		  view_pos--视点位置
		  height--视点高度
		  viewport--视口大小
		  pList--2个数据库中符合条件的所有注记数组的指针
		  QtnNum--2个数据库中符合条件的所有注记的数量数组指针
    返回值：0--成功
*/
static f_int32_t qtndisplaynode(f_int32_t view_type, sTtfFont tfont, f_uint64_t nodeid,
                                Obj_Pt_D view_pos, f_float32_t height, f_int32_t viewport[4], sSqliteName*  pList, f_int32_t* QtnNum)
{
#if 0
	sMapNote * pNote;
	f_int32_t i, toplevel;
	pNote = (sMapNote *)nodeid;

	if(height >= 80000.0f)
		toplevel = 0; 
	else if(height >= 50000.0f)
		toplevel = 1;
	else if(height >= 20000.0f)
		toplevel = 2;
	else if(height >= 10000.0f)
		toplevel = 3;
	else if(height >= 5000.0f)
		toplevel = 4;
	else if(height >= 2000.0f)
		toplevel = 5;
	else
		toplevel = 5;
		
    toplevel = 5;
	
	if (pNote != NULL)
	{
		/*for (i = MAPNOTECLASS - 1;i>=0;i--)*/
		for(i = 0; i <= toplevel; i++)
		{
			namelistOutput(view_type, tfont, i, pNote->notelist[i].pNext, view_pos, viewport);	
		}
		pNote->needrefresh = 0;
	}

#else
	f_int32_t i, j, toplevel;

	/*根据不同高度,选择显示的注记等级,toplevel=4表示显示所有0-4级的注记*/
	if(height >= 2000000.0f)
		toplevel = 1; 
	else if(height >= 1000000.0f)
		toplevel = 2; 
	else if(height >= 250000.0f)
		toplevel = 3; 
	else
		toplevel = 4;

	/*依次处理2个数据库中符合条件的注记,0-地名数据库,1-机场数据库*/
	for(j=0; j<2; j++)
	{
		/*依次处理0-toplevel级的注记数据*/
		/*for (i = MAPNOTECLASS - 1;i>=0;i--)*/
		for(i = 0; i <= toplevel; i++)
		{
			namelistOutputSql(view_type, tfont,  i,  &pList[j * MAXQTNSQLNUM],  QtnNum[j], view_pos,  viewport);
		}
	}
	
#endif

	return 0;
}

/*
	计算标牌注记的左下角的位置(相对于标杆与标牌交点屏幕坐标的偏移)、宽度和高度,单位是像素
	参数：fntwidth--注记等级符号的宽度
	      fntheight--注记等级符号的高度
		  qtntextwidth--注记总的宽度
	      qtntextheight--注记总的高度
		  xoffset,yoffset--标牌的左下角的位置(相对于标杆与标牌交点屏幕坐标的偏移)
		  width,height--标牌的宽度和高度
    返回值：无
*/
static void getPtCollisionParam(f_float32_t fntwidth, f_float32_t fntheight, f_float32_t qtntextwidth, f_float32_t qtntextheight,
	                            f_float32_t *xoffset, f_float32_t *yoffset, f_float32_t *width, f_float32_t *height)
{
	if((NULL == xoffset) || (NULL == yoffset) || (NULL == width) || (NULL == height))
	    return;
	
	/* 考虑4种屏幕旋转的情况 */
	switch(screen_param.rotate_type)
	{   
	    default:
		/*屏幕不旋转,xoffset为向左偏移0.5个注记等级符号的宽度,yoffset为向上偏移0.5个注记等级符号的高度*/
	    case eROTATE_CW_0:
	        *xoffset = -fntwidth * 0.5f;
	        *yoffset = fntheight * 0.5f;
			*width   = qtntextwidth;
			*height  = qtntextheight;
	    break;
	    case eROTATE_CW_90: 
	        *xoffset = -fntheight * 0.5f;
	        *yoffset = -fntwidth * 0.5f;
			*width   = qtntextheight;
			*height  = qtntextwidth;
	    break;
	    case eROTATE_CW_180:
	        *xoffset = fntwidth * 0.5f;
	        *yoffset = 0.0f;
			*width   = qtntextwidth;
			*height  = qtntextheight;
	    break;
	    case eROTATE_CW_270:
	        *xoffset = fntheight * 0.5f;
	        *yoffset = fntwidth * 0.5f;
			*width   = qtntextheight;
			*height  = qtntextwidth;
	    break;
    }
}

/*
	计算标牌的左下角和右上角的位置(相对于标杆与标牌交点屏幕坐标的偏移),单位是像素
	参数：fntwidth--注记等级符号的宽度
	      fntheight--注记等级符号的高度
		  qtntextwidth--注记总的宽度
	      qtntextheight--注记总的高度
		  xloffset,yloffset--标牌的左下角的位置(相对于标杆与标牌交点屏幕坐标的偏移)
		  xroffset,yroffset--标牌的右上角的位置(相对于标杆与标牌交点屏幕坐标的偏移)
    返回值：无
*/
static void getQtnRect(f_float32_t fntwidth, f_float32_t fntheight, f_float32_t qtntextwidth, f_float32_t qtntextheight,
	                   f_float32_t *xlboffset, f_float32_t *ylboffset, f_float32_t *xrtoffset, f_float32_t *yrtoffset)
{
	if((NULL == xlboffset) || (NULL == ylboffset) || (NULL == xrtoffset) || (NULL == yrtoffset))
	    return;
	
	/* 考虑4种屏幕旋转的情况 */
	switch(screen_param.rotate_type)
	{   
	    default:
		/*屏幕不旋转*/
	    case eROTATE_CW_0:
			/*xlboffset为向左偏移1个注记等级符号的宽度,ylboffset为0*/
	        *xlboffset = -fntwidth*0.5 - 3.0f;
	        *ylboffset = 0.0f;//-qtntextheight * 0.5f - 1.0f;  //DXJ
			/*xrtoffset为向右偏移总注记的宽度-1个注记等级符号的宽度+3.0,yrtoffset为汉字注记高度的1.5倍*/
	        *xrtoffset = qtntextwidth - fntwidth*0.5f + 3.0f;
	        *yrtoffset = qtntextheight*1.5f; //DXJ
	    break;
	    case eROTATE_CW_90:
	        *xlboffset = -qtntextheight;
	        *ylboffset = -fntwidth;
	        *xrtoffset = qtntextheight * 0.5f + 1.0f;
	        *yrtoffset = qtntextwidth - fntwidth * 0.5f + 3.0f;
	    break;
	    case eROTATE_CW_180:
	        *xlboffset = -qtntextwidth - fntwidth * 0.5f + 3.0f;
	        *ylboffset = -qtntextheight;
	        *xrtoffset = fntwidth;
	        *yrtoffset = qtntextheight * 0.5f - 1.0f;
	    break;
	    case eROTATE_CW_270:
	        *xlboffset = -qtntextheight * 0.5f + 1.0f;
	        *ylboffset = -qtntextwidth - fntwidth * 0.5f + 3.0f;
	        *xrtoffset = qtntextheight;
	        *yrtoffset = fntwidth;
	    break;
	}
}	                                     

/*
	计算标牌注记的左下角和右上角的位置(相对于标杆与标牌交点屏幕坐标的偏移),单位是像素
	参数：fntwidth--注记等级符号的宽度
	      fntheight--注记等级符号的高度
		  qtntextwidth--注记总的宽度
	      qtntextheight--注记总的高度
		  xloffset,yloffset--标牌的左下角的位置(相对于标杆与标牌交点屏幕坐标的偏移)
    返回值：无
*/
static void getQtnOffset(f_float32_t fntwidth, f_float32_t fntheight, f_float32_t *xoffset, f_float32_t *yoffset)
{
	if((NULL == xoffset) || (NULL == yoffset))
	    return;
	
	/* 考虑4种屏幕旋转的情况 */
	switch(screen_param.rotate_type)
	{
	    default:
		/*屏幕不旋转*/
	    case eROTATE_CW_0:
			/*屏幕不旋转,xoffset为向左偏移0.5个注记等级符号的宽度,yoffset为向上偏移0.5个注记等级符号的高度*/
	        *xoffset = -fntwidth * 0.5f;
	        *yoffset = fntheight * 0.5f;
	    break;
	    case eROTATE_CW_90:
	        *xoffset = -fntheight * 0.5f;
	        *yoffset = -fntwidth * 0.5f;
	    break;
	    case eROTATE_CW_180:
	        *xoffset = fntwidth * 0.5f;
	        *yoffset = -fntheight * 0.5f;
	    break;
	    case eROTATE_CW_270:
	        *xoffset = fntheight * 0.5f;
	        *yoffset = fntwidth * 0.5f;
	    break;
	}
}

/*
	注记碰撞及注记绘制显示
	参数：view_type--视角类型
	      tfont--字体句柄
    返回值：无
*/
static void qtnTextDisplay(sTtfFont tfont, f_int32_t view_type)
{
	f_int32_t i = 0;
	f_int32_t j = 0;
	f_int32_t ret = 0;
	f_float32_t size = 0;
	f_float32_t hfsize = 0.0f;
	f_float32_t extsize = ((tfont.edge > 0) ? (tfont.size * 1.5f):(0));
	f_float32_t xoffset = 0.0f, yoffset = 0.0f;
	f_float32_t xlboffset = 0.0f, ylboffset = 0.0f, xrtoffset = 0.0f, yrtoffset = 0.0f;

	size = ((tfont.edge > 0) ? (tfont.size + 4.0f):(tfont.size));
	hfsize = size * 0.5f;
	
	/* 同一层级的注记按距离(注记与视点的距离)由近到远进行快速排序 */
	for(i = 0; i < MAPNOTECLASS; i++)
	{
		if(qtn_count[i] > 1)
		{
			qtnQuickSortByDis(qtntext[i], 0, qtn_count[i] - 1);
		}
	}

	/* 非俯视视角,所有层级的注记之后加上注记到视点的距离,单位是km */
	if( view_type != eVM_OVERLOOK_VIEW )
	{
	    for(i = 0; i < MAPNOTECLASS; i++) 
	    {
		    for(j = 0; j < qtn_count[i]; j++)
		    {
		        f_char_t disbuf[12] = {0};
		        f_int32_t k, strlength;
		        f_uint16_t uCode[12];
		        sprintf(disbuf, "(%d )", (f_int32_t)(qtntext[i][j].dis * 0.001));
		        ttfAnsi2Unicode(uCode, disbuf, &strlength);
				/*注记的总宽度等于注记符号的宽度+所有注记字符的宽度+距离数字的宽度+距离单位的宽度-extsize(??)*/
		        qtntext[i][j].textwidth = fntwidth[qtntext[i][j].code] + fntwidth[0] * qtntext[i][j].textlen + fntwidth[10] * strlength + fntwidth[11] - extsize;
		        /*注记的总高度等于单个注记字符的高度*/
				qtntext[i][j].textheight = fntheight[0];
		        /* km的unicode码 */
		        uCode[strlength - 2] = 0x008a;
				/* 在地名注记之后增加距离注记 */
                for(k = 0; k < strlength; k++)
		            qtntext[i][j].text[qtntext[i][j].textlen + k] = uCode[k];
		        qtntext[i][j].textlen += strlength;
	        }//end 	for(j = 0; j < qtn_count[i]; j++)
	    }//end for(i = 0; i < MAPNOTECLASS; i++) 
    }//end if
    else
    {
    	for(i = 0; i < MAPNOTECLASS; i++) 
	    {
		    for(j = 0; j < qtn_count[i]; j++)
		    {
		        qtntext[i][j].textwidth = fntwidth[qtntext[i][j].code] + fntwidth[0] * qtntext[i][j].textlen;
		        qtntext[i][j].textheight = fntheight[0];		        
	        }//end 	for(j = 0; j < qtn_count[i]; j++)
	    }//end for(i = 0; i < MAPNOTECLASS; i++) 
    }
	
	/* 所有层级的注记按重要程度、距离由近到远进行碰撞检测 */
	if(qtn_display_count < MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME)
	{
		f_float32_t width = 0.0f, height = 0.0f;
		/*依次处理各个层级中的各个注记*/
	    for(i = 0; i < MAPNOTECLASS; i++)
	    {
		    for(j = 0; j < qtn_count[i]; j++)
		    {   
				/*计算标牌注记的左下角的位置(相对于标杆与标牌交点屏幕坐标的偏移)、宽度和高度,单位是像素*/
	            getPtCollisionParam(fntwidth[qtntext[i][j].code], fntheight[qtntext[i][j].code], 
	                                qtntext[i][j].textwidth, qtntext[i][j].textheight,
	                                &xoffset, &yoffset, &width, &height);	
				/*标牌注记外接矩形进行碰撞检测,code值越小,距离越近的地名先被绘制,之后同一code等级距离远的参与碰撞,再试不同code等级参与碰撞*/
			    if( view_type != eVM_OVERLOOK_VIEW )
			    {
			    	ret = isPointCollision3d(qtntext[i][j].nsx + xoffset, qtntext[i][j].nsy + yoffset, 0, width, height, 0);
				}
			    else
			    {
			    	ret = isPointCollision3d(qtntext[i][j].sx + xoffset, qtntext[i][j].sy + yoffset, 0, width, height, 0);
				}
		        
				/*如果碰撞,不绘制*/
		        if(0 != ret)
		            continue;
		        else
		        {
					/*如果不碰撞,则加入到待绘制的数组中,待绘制的注记数量+1*/
		    	    qtndisp[qtn_display_count] = qtntext[i][j];
		    	    qtn_display_count++;
		        }
		    }
		}//end for(i = 0; i < MAPNOTECLASS; i++)
	}//end if(qtn_display_count < MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME)
	
	/* 碰撞检测后所有需要绘制的各层级的注记再进行统一排序（由近到远,按深度值从小到大） */
	if(qtn_display_count > 1)
	     qtnQuickSortByDepth(qtndisp, 0, qtn_display_count - 1);	
	
	glDisableEx(GL_TEXTURE_2D);	
	/* 为了让背景框的透明起效果，注记输出时必需按从远到近的顺序输出 */
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	for(i = qtn_display_count - 1; i >= 0; i--)
	{
		/*计算标牌的左下角和右上角的位置(相对于标杆与标牌交点屏幕坐标的偏移),单位是像素*/
	    getQtnRect(fntwidth[qtndisp[i].code], fntheight[qtndisp[i].code], qtndisp[i].textwidth, qtndisp[i].textheight,
	               &xlboffset, &ylboffset, &xrtoffset, &yrtoffset);	
	            
		/*非俯视视角时画注记标牌矩形线框及标杆*/
		if( view_type != eVM_OVERLOOK_VIEW )
		{
			/*绘制注记标牌矩形线框*/
			drawBordLine(qtndisp[i].nsx + xlboffset, qtndisp[i].nsy + ylboffset, qtndisp[i].nsz, 
                         qtndisp[i].nsx + xrtoffset, qtndisp[i].nsy + yrtoffset, qtndisp[i].nsz ); 
			/*绘制标杆*/
            drawTextLegLine(qtndisp[i].sx, qtndisp[i].sy, qtndisp[i].nsz, qtndisp[i].nsx, qtndisp[i].nsy, qtndisp[i].nsz);
        }
	}        

	glColor4f(0.3216f, 0.6353f, 0.9804f, 0.35f);
	for(i = qtn_display_count - 1; i >= 0; i--)
	{
		/*计算标牌的左下角和右上角的位置(相对于标杆与标牌交点屏幕坐标的偏移),单位是像素*/
	    getQtnRect(fntwidth[qtndisp[i].code], fntheight[qtndisp[i].code], qtndisp[i].textwidth, qtndisp[i].textheight,
	               &xlboffset, &ylboffset, &xrtoffset, &yrtoffset);
		/*非俯视视角时画注记矩形背景框*/
		if( view_type != eVM_OVERLOOK_VIEW )
		{
			drawBordBackground(qtndisp[i].nsx + xlboffset, qtndisp[i].nsy + ylboffset, qtndisp[i].nsz, 
                               qtndisp[i].nsx + xrtoffset, qtndisp[i].nsy + yrtoffset, qtndisp[i].nsz);
        }
	}	
	glEnableEx(GL_TEXTURE_2D);
	/* 设置字体的颜色 */
	glColor4fv(tfont.color_font);
	for(i = qtn_display_count - 1; i >= 0; i--)
	{
		/*计算标牌注记的左下角的位置(相对于标杆与标牌交点屏幕坐标的偏移),单位是像素*/
	    getQtnOffset(fntwidth[qtndisp[i].code], fntheight[qtndisp[i].code], &xoffset, &yoffset);
		/*在标牌中绘制注记*/
	    if( view_type != eVM_OVERLOOK_VIEW )
		{
			ttfDrawStringDepth(tfont.font, qtndisp[i].nsx + xoffset, qtndisp[i].nsy + yoffset, 
			              (f_float32_t)qtndisp[i].nsz, qtndisp[i].text, qtndisp[i].textlen);
		}
		else
		{
			ttfDrawString(tfont.font, qtndisp[i].sx + xoffset, qtndisp[i].sy + yoffset, 
			              (f_float32_t)(qtndisp[i].sz), qtndisp[i].text, qtndisp[i].textlen);			              
		}
	}
}

/*
	查询不同的单个注记符号在字体中的宽度和高度,单位是像素
	参数：tfont--字体句柄
    返回值：无
*/
static void getFntWidthHeight(sTtfFont tfont)
{
	f_int32_t ret = -1;
	f_uint16_t unicode[CHAR_NUM] = {0x51dd,  /* 字库中的某个汉字 凝 代表所有汉字的宽度和高度 */
		                      0x2605,  /* 首都     501符号为实心五角星*/	
		                      0x25c6,  /* 重要城市 502符号为实心菱形*/	
		                      0x25c6,  /* 重要城市 503*/  	
		                      0x25ce,  /* 主要城市 504符号为实心圆形*/ 	
		                      0x25c7,  /* 一般城市 505符号为空心菱形*/	
		                      0x25cb,  /* 其它居民地 506符号为空心圆形 */	
		                      0x25cb,  /* 其它居民地 507 */	
		                      0x25cb,  /* 其它居民地 508 */	
		                      0x25cb,  /* 其它居民地 509 */
		                      0x0038,  /* 8 代表所有数字的宽度和高度*/
							  0x008a   /* km 字体中的特有字符,上半部是k,下半部是m*/
		                      };
	/*查询12种类型的注记在字体库中的宽度和高度*/
	ret = ttfGetFontWidthHeight(tfont.font, unicode, CHAR_NUM, fntwidth, fntheight);
	if(-1 == ret)
	{ 
		/*查询失败则给一个默认值即可,区分有描边和无描边*/
		f_int32_t i = 0;
		if(tfont.edge > 0)
		{
			for(i = 0; i < 5; i++)
		        fntwidth[i]	= tfont.size + 4;
			for(i = 5; i < CHAR_NUM; i++)
				fntwidth[i] = tfont.size / 2 + 4;

			for(i = 0; i < 4; i++)
		        fntheight[i]	= tfont.size;
			for(i = 4; i < CHAR_NUM; i++)
				fntheight[i] = tfont.size / 2 + 4;
		}
		else
		{
		    fntwidth[0]	= tfont.size;
			for(i = 1; i < 4; i++)
		        fntwidth[i]	= tfont.size - 4;
			for(i = 4; i < CHAR_NUM; i++)
				fntwidth[i] = tfont.size / 2;
			
			fntheight[0]	= tfont.size;	
			for(i = 1; i < 4; i++)
		        fntheight[i]	= tfont.size - 4;
			for(i = 4; i < CHAR_NUM; i++)
				fntheight[i] = tfont.size / 2 ;
		}
	}
}

static f_int32_t qtndestroynode(f_uint64_t nodeid)
{
	sMapNote * pNote;
	f_int32_t i;
	pNote = (sMapNote *)nodeid;
	if (pNote != NULL)
	{
		for (i = 0; i < MAPNOTECLASS; i++)
		{
			namelistDestroy(pNote->notelist[i].pNext);
		}
	}
	DeleteAlterableMemory(pNote);
	return 0;
}

/*
	注记绘制前的准备工作,包括模视投影矩阵的设置、全局注记数量清零、不同单个注记类型在指定字体中的宽度和高度
	参数：view_type--视角类型
	      tfont--字体句柄
		  viewport--视口大小
		  matrix--原场景绘制的模视投影矩阵
    返回值：0--成功
*/
static f_int32_t qtndisplaypre(f_int32_t view_type, sTtfFont tfont, f_int32_t viewport[4], f_float64_t matrix[16])
{
	/*非俯视视角则开启深度测试,俯视视角关闭深度测试*/
	if( view_type != eVM_OVERLOOK_VIEW )
	{
	    glEnableEx(GL_DEPTH_TEST);
	}
	else
	{
	    glDisableEx(GL_DEPTH_TEST);
	}

	glDisableEx(GL_CULL_FACE);
		
	/*保存原来的投影模视矩阵*/
	namelistUpdateCoordParam(matrix);

	/*设置投影矩阵,正投影*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	/*2016-9-20 15:51 张仟新 不知什么原因（邓秀剑调试出来的），远近裁截面必需设置成(0.0f, -1.0f)，
	  按照屏幕坐标绘制东西时(如注记标记线)，遮挡才有用，否则遮挡不起作用*/
	glOrtho(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, -1.0f); 

	/*设置模视矩阵*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 	
	/*注记输出前的OpenGL状态设置,使用三维数据目录中的字体*/
	ttfDrawStringPre(tfont.font);
	/* 设置边框的颜色 */
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tfont.color_edge); 
	{
	    f_int32_t i = 0;
		/*总的显示的注记数量初始化为0*/
	    qtn_display_count = 0;
		/*每一层的注记数量初始化为0,总共有6层*/
	    for(i = 0; i < MAPNOTECLASS; i++)	
	        qtn_count[i] = 0;
	}
	
	{
	    static f_int32_t tempcnt = 0;
		/*查询不同的单个注记符号在字体中的宽度和高度,单位是像素*/
	    if(0 == tempcnt)	
	    {
	        getFntWidthHeight(tfont);
	        tempcnt = 1;	
	    }
	}
	
	return 0;
}

/*
	注记碰撞、注记绘制显示及绘制后的状态恢复
	参数：view_type--视角类型
	      tfont--字体句柄
    返回值：0--成功
*/
static f_int32_t qtndisplaypro(f_int32_t view_type, sTtfFont tfont)
{
	/*注记碰撞及注记绘制显示*/
	qtnTextDisplay(tfont, view_type);
	
	/*注记输出后的OpenGL状态恢复*/
	ttfDrawStringPro(tfont.font);
	/*非俯视视角关闭深度测试*/
	if( view_type != eVM_OVERLOOK_VIEW )
	    glDisableEx(GL_DEPTH_TEST);

	return 0;
}

void qtnAddFile(const f_char_t * pFileName, f_int32_t rootlevel, f_int32_t rootxidx, f_int32_t rootyidx)
{
	QTNFILE qtnfile;
	QTNINFO * pInfo;
	if (qtnfileOpen(pFileName, &qtnfile) != 0)
		return ;
	pInfo = (QTNINFO *)NewAlterableMemory(sizeof(QTNINFO));
	if (pInfo == NULL)
	{
		qtnfileClose(qtnfile);
		return ;
	}
	pInfo->pNext     = qtninfo.pNext;
	pInfo->qtnfile   = qtnfile;
	pInfo->rootlevel = rootlevel;
	pInfo->rootxidx  = rootxidx;
	pInfo->rootyidx  = rootyidx;
	qtninfo.pNext    = pInfo;
}

/*
	全局qtn信息链表初始化
	参数：无
    返回值：无
*/
void qtnInit(void)
{
    memset(&qtninfo, 0, sizeof(qtninfo));
}

void qtnDeInit(void)
{	
	QTNINFO * pInfo;
	qtnDeInitFont();
	pInfo = qtninfo.pNext;
	while (pInfo != NULL)
	{
		QTNINFO * pTemp;
		pTemp = pInfo->pNext;
		qtnfileClose(pInfo->qtnfile);
		DeleteAlterableMemory(pInfo);
		pInfo = pTemp;
	}
}

/*///////////////////////////////////////////////////////*/


/*
	设置指定图层的功能函数
	参数：map_andle -- 用createMapHandle返回的VOIDPtr句柄，全局唯一的树
	      layer -- 图层号, 0..MAXMAPLAYER-1,目前只有一个图层,即0
    返回值：
	      无
*/
void qtnSetLayerFunc(VOIDPtr map_handle, f_int32_t layer)
{
	/*设置图层0的各个功能函数,参数为全局qtn信息链表*/
	qtmapSetLayerFunc(map_handle, 
	                  layer, 
	                  (f_uint64_t)qtninfo.pNext, 
		              qtncreatenode, 
		              qtndestroynode, 
		              qtndisplaynode,
		              qtndisplaypre, 
		              qtndisplaypro);
}
#endif



int isPointInViewport(double sx, double sy, int* viewport)
{
	double startx=0, starty=0, width=0, height=0;
	if(NULL == viewport)
	{
		DEBUG_PRINT("NULL = viewport.")
			return -1;
	}
	startx = (double)viewport[0];
	starty = (double)viewport[1];
	width  = (double)viewport[2];
	height = (double)viewport[3];
	if( (sx < 0.0)//startx)
		|| (sx > width)//(startx+width))
		|| (sy < 0.0)//starty)
		|| (sy > height)//(starty+height))
		)
	{
		return -2;
	}
	return 0;
}
