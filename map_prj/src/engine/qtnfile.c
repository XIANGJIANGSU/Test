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
// ���	����	       ����	  ʶ����	����	��ɫ	        ��С	��б	ͼ����	 �б�
// 1			ʡ��	   	50	  		501,502	����	50,50, 50	    28	     F	    ����	 0
// 2			���ݼ�	50	  		503,504	����	50,50,50	    24	     F	    ����	 1
// 3			�ؼ�	   	50	  		505	    ����	50, 50,50	    20	     F	    ����	 2
// 4			����	   	50	  		506,507	����	50,50, 50	    16	     F	    ����	 3,4
// 5			������	50	  		����	    ����	50, 50, 50	    13	     F	    ����	 5
// 6			����	   	/	   		/	    ����	50, 50, 50	    12	     F	    /	     6
*/

typedef struct tagQtnTileInfo{
	f_uint64_t offset;                /* ��Ƭ���ļ��е�ƫ�Ƶ�ַ */
	f_uint64_t length;                /* ��Ƭ���ݵĳ���(�ֽ���) */
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
	0x2605,  /* ����׶� */
    0x2605,  /* �׶�     501*/
	0x25c6,  /* ��Ҫ���� 502*/
	0x25c6,  /* ��Ҫ���� 503*/  
	0x25ce,  /* ��Ҫ���� 504*/ 
	0x25c7,  /* һ����� 505*/
	0x25cb,  /* ��������� 506 */
	0x25cb,  /* ��������� 507 */
	0x25cb,  /* ��������� 508 */
	0x25cb,  /* ��������� 509 */
};

#define CHAR_NUM 12
f_int32_t fntwidth[CHAR_NUM]; /* ��0���Ǻ��ֵĿ�ȣ�1~9����mapsym��Ӧ���ŵĿ�ȣ�10�����ֵĿ�ȣ�11��km�Ŀ�� */
f_int32_t fntheight[CHAR_NUM];

/* added by zqx 2016-10-8 14:43 start */
#define MAX_QTNNUM_DISPLAY_PERLAYER_IN_CURFRAME  500    /* ��ǰ֡ÿ�����Ҫ��ʾ�����ע��������ײ���ǰ�� */
sQTNText qtntext[MAPNOTECLASS][MAX_QTNNUM_DISPLAY_PERLAYER_IN_CURFRAME];
f_int32_t qtn_count[MAPNOTECLASS];

#define MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME  1000       /* ��ǰ֡����ʾ�����ע��������ײ���� */
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
	��һ��QTN�ļ���
	������cstrFileName -- �ļ�����
	      pQTNFile -- ����ɹ��򷵻��ļ��������������ʹ��
	����ֵ��
	     0 -- �ɹ����ļ������pQTNFile��
		-1 -- �ļ���ʧ��
		-2 -- �ļ���ʽ����
		-3 -- �ڴ����ʧ��
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
	�ر�һ���Ѿ��򿪵�QTN�ļ����ͷ��ļ�ռ�õ���Դ
	������qtnfile -- ��qtnfileOpen���ص�QTN�ļ����
	����ֵ��
	     0 -- �ɹ�
		-1 -- �ļ������Ч
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
	�õ�һ���Ѿ��򿪵�QTN�ļ������Ĳ���
	������qtnfile -- ��qtnfileOpen���ص�QTN�ļ����
	����ֵ��
	    >0 -- ���Ĳ���
		-1 -- �ļ������Ч
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
	�õ�һ��QTN�ļ��еĻ���ͼ������
	������qtnfile -- ��qtnfileOpen���ص�QTN�ļ����
	      layer -- ָ���Ĳ㣬��0��ʼ
		  xidx, yidx -- ����ͼ���ڲ��еı��
		  pData -- ���ص����ݣ�����ɹ�����
	����ֵ��
	     1 -- �������ڵ���
	     0 -- �ɹ�
		-1 -- �ļ������Ч
		-2 -- �����Ч
		-3 -- xidx��yidx��Ч
		-4 -- �ڴ�������
		-5 -- ���ݽ������
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

	/*ȫ�ֱ��ת�ɾֲ����*/
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
    /* ��ע�Ƿ��ŵ�unicode����ע�Ǻϲ�������ʾʱһ����ʾ changed start by zqx 2016-10-13 14:45*/
	if(code > 0 && code < 10)
	    pItem->pText[0] = mapsym[code];
	else
	    pItem->pText[0] = mapsym[6];
	ttfAnsi2Unicode((unsigned short *)(&(pItem->pText[1])), noteS, &pItem->textlen);
	pItem->textlen++;
	/* ��ע�Ƿ��ŵ�unicode����ע�Ǻϲ�������ʾʱһ����ʾ changed end by zqx 2016-10-13 14:45*/
	//ttfAnsi2Unicode(pItem->pText, noteS, &pItem->textlen); //old
	
	geoPt.lon = x;
	geoPt.lat = y;
	geoPt.height = z + 600.0;/* 600.0��ֵ��ʵ��Ӧ���в���Ҫ��Ŀǰ����ΪС���ڳ�����ע��ʱû����ȡ���θ߶ȣ����µ�����ʾ�ڵ��¡�������С����ȡ�߶� */
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
	��ע�Ǽ���ע�ǻ���������
	������code_index--��ǰ�����ע�ǵȼ�,0-5
	      code--�������ע�ǵĵȼ�,0-4
	      sx,sy,sz--ע�ǵ���Ļ����1,����������Ľ���
	      nsx,nsy,nsz--ע�ǵ���Ļ����2,���������ƵĽ���,ע�ǻ����ڱ�����
		  dis--ע�����ӵ�ľ���
		  textlen--ע�ǳ���
		  pString--ע������
    ����ֵ����
*/
static void addQtntextIntoArray(f_int32_t code_index, f_int32_t code, f_float64_t sx, f_float64_t sy, f_float64_t sz, 
                                f_float64_t nsx, f_float64_t nsy, f_float64_t nsz, f_float32_t dis, 
                                f_int32_t   textlen, const f_uint16_t * pString)
{
	f_int32_t i = 0;
	/*һ֡�д����Ƹ��ȼ�ע�ǵĸ��������500��*/
	if(qtn_count[code_index] < MAX_QTNNUM_DISPLAY_PERLAYER_IN_CURFRAME)
	{
		/*��ע�Ǽ��뵽���ȼ������Ƶ�ע��������,ע�ǳ������Ϊ12*/
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

#define NOTE_DISPLAY_FARST 150000.0     //ע����ʾ����Զ���루���ӵ��ľ��룩
static void namelistOutput(f_int32_t view_type, sTtfFont tfont, f_int32_t code_index, 
                           sNameList * pList, Obj_Pt_D view_pos, f_int32_t viewport[4])
{
	f_int32_t size = tfont.size;
	f_float32_t hfsize = size * 0.5f;
	BOOL ret = FALSE;
	f_float64_t viewx, viewy, viewz, dis;/*ע�ǵ����Ļ����*/ 
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
				/* ����ʱ�����þ��������ƣ�͸��ʱ���þ��������� */
			    if(dis > NOTE_DISPLAY_FARST)
			    {
				    pList = pList->pNext;
				    continue;
			    }
			}
			
			/*ת�����������굽��Ļ����*/
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
	����ע�����ӵ�ľ��롢ע�ǻ�����ص���Ļ����,�����ӿڷ�Χ���������ע�ǻ���������
	������view_type--�ӽ�����
	      tfont--������
		  code_index--ע�ǵȼ�,0-4
		  pList--���ݿ��з�������������ע�ǵ�ָ��
		  QtnNum--���ݿ��з�������������ע�ǵ�����
		  view_pos--�ӵ�λ��
		  viewport--�ӿڴ�С
    ����ֵ����
*/
static void namelistOutputSql(f_int32_t view_type, sTtfFont tfont, f_int32_t code_index, 
                           sSqliteName * pList, f_int32_t QtnNum, Obj_Pt_D view_pos, f_int32_t viewport[4])
{
	BOOL ret = FALSE;
	/*ע�ǵ����Ļ����,����������Ľ���*/ 
    f_float64_t viewx = 0.0, viewy = 0.0, viewz = 0.0, dis = 0.0;
	/*�������ƵĽ�����������ꡢ��Ļ����*/
    f_float64_t nviewx = 0.0, nviewy = 0.0, nviewz = 0.0, nobjx = 0.0, nobjy = 0.0, nobjz = 0.0, ratio = 0.0;  
	f_int32_t i = 0;

	/*ע��ָ��Ϊ��,˵����ע����Ҫ����,ֱ�ӷ���*/
	if (pList == NULL)
	{
		return;
	}

	/*ѭ���������ע��*/
	for(i=0; i<QtnNum; i++)
	{
		/*�ж�ע�ǵ�codeֵ�뵱ǰ�����codeֵ�Ƿ�һ��,��һ�����ע�ǲ�����*/
		if(code_index != pList[i].code)
		{
			continue;
		}
		/*ע��codeֵ��Ч,���ȴ���0,���ݲ�Ϊ��ʱ,�����ע��,���򲻴���*/
		if( (pList[i].code < MAPNOTECLASS) &&
		    (pList[i].textlen > 0) && (pList[i].pText != NULL) )
		{
			/*�����ӵ㵽��ע��λ��֮��ľ���*/
		    dis = 0.0f;			
			dis = (f_float32_t)sqrt((view_pos.x - pList[i].px) * (view_pos.x - pList[i].px) + 
					                (view_pos.y - pList[i].py) * (view_pos.y - pList[i].py) + 
								    (view_pos.z - pList[i].pz) * (view_pos.z - pList[i].pz) );
		    /* ����ʱ�����þ��������ƣ�͸��ʱ���þ��������� */
			if( view_type != eVM_OVERLOOK_VIEW )
			{
				/* ͸��ʱ,�������150km,��ע�ǲ����� */
			    if(dis > NOTE_DISPLAY_FARST)
			    {
				    //pList[i] = pList[i]->pNext;
				    continue;
			    }
			}
			
			/*ע�ǵ����������ת������Ļ����,ת��ʧ�ܻ�ת�����zֵ����1.0f,���ע�ǲ�����*/
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
			/*�ж���Ļ�����Ƿ����ӿڷ�Χ��,���ڸ�ע��Ҳ������*/
			ret = isPointInViewport(viewx, viewy, viewport);
			if(0 != ret)
			{
				//pList[i] = pList[i]->pNext;
				continue;
			}
			
			/*�����ӽ���,ע���Ա�˼ӱ��Ƶ���ʽ����,ע�ǵ����Ļ����ʵ���Ǳ�������Ľ���*/
			/*�������������ƵĽ������Ļ����*/
			if( view_type != eVM_OVERLOOK_VIEW )
		    {
				/*���Ʊ�˵ĳ���,���ڵ�����ע�ǵ�����������һ���ı���,�õ��������ƵĽ������������*/
		    	ratio = 1.0 + (dis + 5000.0) / NOTE_DISPLAY_FARST * 0.002;
			    nobjx = ratio * pList[i].px;
			    nobjy = ratio * pList[i].py;
			    nobjz = ratio * pList[i].pz;
				/*�������ƵĽ������������ת������Ļ����*/
			    ret = avicglProject(nobjx, nobjy, nobjz, g_matrix, viewport, &nviewx, &nviewy, &nviewz);
			    if(FALSE == ret)
			    {
				    //pList[i] = pList[i]->pNext;
				    continue;
			    }
		    }
		    /*����ע�Ǽ��뵽�����Ƶ�ע��������*/
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
		�ڱ����ҵ���Ӧ���ļ�
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
			case 2:			/*/ ʡ��ֱϽ�� */
				fsize = 0.5f;
				slist = &pNote->notelist[0];
				/*strcat(noteS, "��");*/
				break;
			case 3:			/*/ ��פ��*/ 
			case 4:			/*/ �ؼ���*/ 
				fsize = 0.5f;
				slist = &pNote->notelist[1];
				/*strcat(noteS, "��");*/
				break;
			case 5:			/*/ �� */
				fsize = 0.5f;
				slist = &pNote->notelist[2];
				/*strcat(noteS, "��");*/
				break;
			case 6:			/*/ �� */
			case 7:			/*/ �� */
				fsize = 0.5f;
				slist = &pNote->notelist[code==6?3:4];
				/*strcat(noteS, "��");*/
				break;
			default:
				fsize = 0.5f;
				slist = &pNote->notelist[5];
				/*strcat(noteS, "��");*/
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
	����������ݿ��з�������������ע��,�������ע�ǻ���������
	������view_type--�ӽ�����
	      tfont--������
		  nodeid--��ʱ����,ʼ�ո�0
		  view_pos--�ӵ�λ��
		  height--�ӵ�߶�
		  viewport--�ӿڴ�С
		  pList--2�����ݿ��з�������������ע�������ָ��
		  QtnNum--2�����ݿ��з�������������ע�ǵ���������ָ��
    ����ֵ��0--�ɹ�
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

	/*���ݲ�ͬ�߶�,ѡ����ʾ��ע�ǵȼ�,toplevel=4��ʾ��ʾ����0-4����ע��*/
	if(height >= 2000000.0f)
		toplevel = 1; 
	else if(height >= 1000000.0f)
		toplevel = 2; 
	else if(height >= 250000.0f)
		toplevel = 3; 
	else
		toplevel = 4;

	/*���δ���2�����ݿ��з���������ע��,0-�������ݿ�,1-�������ݿ�*/
	for(j=0; j<2; j++)
	{
		/*���δ���0-toplevel����ע������*/
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
	�������ע�ǵ����½ǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��)����Ⱥ͸߶�,��λ������
	������fntwidth--ע�ǵȼ����ŵĿ��
	      fntheight--ע�ǵȼ����ŵĸ߶�
		  qtntextwidth--ע���ܵĿ��
	      qtntextheight--ע���ܵĸ߶�
		  xoffset,yoffset--���Ƶ����½ǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��)
		  width,height--���ƵĿ�Ⱥ͸߶�
    ����ֵ����
*/
static void getPtCollisionParam(f_float32_t fntwidth, f_float32_t fntheight, f_float32_t qtntextwidth, f_float32_t qtntextheight,
	                            f_float32_t *xoffset, f_float32_t *yoffset, f_float32_t *width, f_float32_t *height)
{
	if((NULL == xoffset) || (NULL == yoffset) || (NULL == width) || (NULL == height))
	    return;
	
	/* ����4����Ļ��ת����� */
	switch(screen_param.rotate_type)
	{   
	    default:
		/*��Ļ����ת,xoffsetΪ����ƫ��0.5��ע�ǵȼ����ŵĿ��,yoffsetΪ����ƫ��0.5��ע�ǵȼ����ŵĸ߶�*/
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
	������Ƶ����½Ǻ����Ͻǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��),��λ������
	������fntwidth--ע�ǵȼ����ŵĿ��
	      fntheight--ע�ǵȼ����ŵĸ߶�
		  qtntextwidth--ע���ܵĿ��
	      qtntextheight--ע���ܵĸ߶�
		  xloffset,yloffset--���Ƶ����½ǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��)
		  xroffset,yroffset--���Ƶ����Ͻǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��)
    ����ֵ����
*/
static void getQtnRect(f_float32_t fntwidth, f_float32_t fntheight, f_float32_t qtntextwidth, f_float32_t qtntextheight,
	                   f_float32_t *xlboffset, f_float32_t *ylboffset, f_float32_t *xrtoffset, f_float32_t *yrtoffset)
{
	if((NULL == xlboffset) || (NULL == ylboffset) || (NULL == xrtoffset) || (NULL == yrtoffset))
	    return;
	
	/* ����4����Ļ��ת����� */
	switch(screen_param.rotate_type)
	{   
	    default:
		/*��Ļ����ת*/
	    case eROTATE_CW_0:
			/*xlboffsetΪ����ƫ��1��ע�ǵȼ����ŵĿ��,ylboffsetΪ0*/
	        *xlboffset = -fntwidth*0.5 - 3.0f;
	        *ylboffset = 0.0f;//-qtntextheight * 0.5f - 1.0f;  //DXJ
			/*xrtoffsetΪ����ƫ����ע�ǵĿ��-1��ע�ǵȼ����ŵĿ��+3.0,yrtoffsetΪ����ע�Ǹ߶ȵ�1.5��*/
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
	�������ע�ǵ����½Ǻ����Ͻǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��),��λ������
	������fntwidth--ע�ǵȼ����ŵĿ��
	      fntheight--ע�ǵȼ����ŵĸ߶�
		  qtntextwidth--ע���ܵĿ��
	      qtntextheight--ע���ܵĸ߶�
		  xloffset,yloffset--���Ƶ����½ǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��)
    ����ֵ����
*/
static void getQtnOffset(f_float32_t fntwidth, f_float32_t fntheight, f_float32_t *xoffset, f_float32_t *yoffset)
{
	if((NULL == xoffset) || (NULL == yoffset))
	    return;
	
	/* ����4����Ļ��ת����� */
	switch(screen_param.rotate_type)
	{
	    default:
		/*��Ļ����ת*/
	    case eROTATE_CW_0:
			/*��Ļ����ת,xoffsetΪ����ƫ��0.5��ע�ǵȼ����ŵĿ��,yoffsetΪ����ƫ��0.5��ע�ǵȼ����ŵĸ߶�*/
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
	ע����ײ��ע�ǻ�����ʾ
	������view_type--�ӽ�����
	      tfont--������
    ����ֵ����
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
	
	/* ͬһ�㼶��ע�ǰ�����(ע�����ӵ�ľ���)�ɽ���Զ���п������� */
	for(i = 0; i < MAPNOTECLASS; i++)
	{
		if(qtn_count[i] > 1)
		{
			qtnQuickSortByDis(qtntext[i], 0, qtn_count[i] - 1);
		}
	}

	/* �Ǹ����ӽ�,���в㼶��ע��֮�����ע�ǵ��ӵ�ľ���,��λ��km */
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
				/*ע�ǵ��ܿ�ȵ���ע�Ƿ��ŵĿ��+����ע���ַ��Ŀ��+�������ֵĿ��+���뵥λ�Ŀ��-extsize(??)*/
		        qtntext[i][j].textwidth = fntwidth[qtntext[i][j].code] + fntwidth[0] * qtntext[i][j].textlen + fntwidth[10] * strlength + fntwidth[11] - extsize;
		        /*ע�ǵ��ܸ߶ȵ��ڵ���ע���ַ��ĸ߶�*/
				qtntext[i][j].textheight = fntheight[0];
		        /* km��unicode�� */
		        uCode[strlength - 2] = 0x008a;
				/* �ڵ���ע��֮�����Ӿ���ע�� */
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
	
	/* ���в㼶��ע�ǰ���Ҫ�̶ȡ������ɽ���Զ������ײ��� */
	if(qtn_display_count < MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME)
	{
		f_float32_t width = 0.0f, height = 0.0f;
		/*���δ�������㼶�еĸ���ע��*/
	    for(i = 0; i < MAPNOTECLASS; i++)
	    {
		    for(j = 0; j < qtn_count[i]; j++)
		    {   
				/*�������ע�ǵ����½ǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��)����Ⱥ͸߶�,��λ������*/
	            getPtCollisionParam(fntwidth[qtntext[i][j].code], fntheight[qtntext[i][j].code], 
	                                qtntext[i][j].textwidth, qtntext[i][j].textheight,
	                                &xoffset, &yoffset, &width, &height);	
				/*����ע����Ӿ��ν�����ײ���,codeֵԽС,����Խ���ĵ����ȱ�����,֮��ͬһcode�ȼ�����Զ�Ĳ�����ײ,���Բ�ͬcode�ȼ�������ײ*/
			    if( view_type != eVM_OVERLOOK_VIEW )
			    {
			    	ret = isPointCollision3d(qtntext[i][j].nsx + xoffset, qtntext[i][j].nsy + yoffset, 0, width, height, 0);
				}
			    else
			    {
			    	ret = isPointCollision3d(qtntext[i][j].sx + xoffset, qtntext[i][j].sy + yoffset, 0, width, height, 0);
				}
		        
				/*�����ײ,������*/
		        if(0 != ret)
		            continue;
		        else
		        {
					/*�������ײ,����뵽�����Ƶ�������,�����Ƶ�ע������+1*/
		    	    qtndisp[qtn_display_count] = qtntext[i][j];
		    	    qtn_display_count++;
		        }
		    }
		}//end for(i = 0; i < MAPNOTECLASS; i++)
	}//end if(qtn_display_count < MAX_QTNNUM_NEED_DISPLAY_IN_CURFRAME)
	
	/* ��ײ����������Ҫ���Ƶĸ��㼶��ע���ٽ���ͳһ�����ɽ���Զ,�����ֵ��С���� */
	if(qtn_display_count > 1)
	     qtnQuickSortByDepth(qtndisp, 0, qtn_display_count - 1);	
	
	glDisableEx(GL_TEXTURE_2D);	
	/* Ϊ���ñ������͸����Ч����ע�����ʱ���谴��Զ������˳����� */
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	for(i = qtn_display_count - 1; i >= 0; i--)
	{
		/*������Ƶ����½Ǻ����Ͻǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��),��λ������*/
	    getQtnRect(fntwidth[qtndisp[i].code], fntheight[qtndisp[i].code], qtndisp[i].textwidth, qtndisp[i].textheight,
	               &xlboffset, &ylboffset, &xrtoffset, &yrtoffset);	
	            
		/*�Ǹ����ӽ�ʱ��ע�Ǳ��ƾ����߿򼰱��*/
		if( view_type != eVM_OVERLOOK_VIEW )
		{
			/*����ע�Ǳ��ƾ����߿�*/
			drawBordLine(qtndisp[i].nsx + xlboffset, qtndisp[i].nsy + ylboffset, qtndisp[i].nsz, 
                         qtndisp[i].nsx + xrtoffset, qtndisp[i].nsy + yrtoffset, qtndisp[i].nsz ); 
			/*���Ʊ��*/
            drawTextLegLine(qtndisp[i].sx, qtndisp[i].sy, qtndisp[i].nsz, qtndisp[i].nsx, qtndisp[i].nsy, qtndisp[i].nsz);
        }
	}        

	glColor4f(0.3216f, 0.6353f, 0.9804f, 0.35f);
	for(i = qtn_display_count - 1; i >= 0; i--)
	{
		/*������Ƶ����½Ǻ����Ͻǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��),��λ������*/
	    getQtnRect(fntwidth[qtndisp[i].code], fntheight[qtndisp[i].code], qtndisp[i].textwidth, qtndisp[i].textheight,
	               &xlboffset, &ylboffset, &xrtoffset, &yrtoffset);
		/*�Ǹ����ӽ�ʱ��ע�Ǿ��α�����*/
		if( view_type != eVM_OVERLOOK_VIEW )
		{
			drawBordBackground(qtndisp[i].nsx + xlboffset, qtndisp[i].nsy + ylboffset, qtndisp[i].nsz, 
                               qtndisp[i].nsx + xrtoffset, qtndisp[i].nsy + yrtoffset, qtndisp[i].nsz);
        }
	}	
	glEnableEx(GL_TEXTURE_2D);
	/* �����������ɫ */
	glColor4fv(tfont.color_font);
	for(i = qtn_display_count - 1; i >= 0; i--)
	{
		/*�������ע�ǵ����½ǵ�λ��(����ڱ������ƽ�����Ļ�����ƫ��),��λ������*/
	    getQtnOffset(fntwidth[qtndisp[i].code], fntheight[qtndisp[i].code], &xoffset, &yoffset);
		/*�ڱ����л���ע��*/
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
	��ѯ��ͬ�ĵ���ע�Ƿ����������еĿ�Ⱥ͸߶�,��λ������
	������tfont--������
    ����ֵ����
*/
static void getFntWidthHeight(sTtfFont tfont)
{
	f_int32_t ret = -1;
	f_uint16_t unicode[CHAR_NUM] = {0x51dd,  /* �ֿ��е�ĳ������ �� �������к��ֵĿ�Ⱥ͸߶� */
		                      0x2605,  /* �׶�     501����Ϊʵ�������*/	
		                      0x25c6,  /* ��Ҫ���� 502����Ϊʵ������*/	
		                      0x25c6,  /* ��Ҫ���� 503*/  	
		                      0x25ce,  /* ��Ҫ���� 504����Ϊʵ��Բ��*/ 	
		                      0x25c7,  /* һ����� 505����Ϊ��������*/	
		                      0x25cb,  /* ��������� 506����Ϊ����Բ�� */	
		                      0x25cb,  /* ��������� 507 */	
		                      0x25cb,  /* ��������� 508 */	
		                      0x25cb,  /* ��������� 509 */
		                      0x0038,  /* 8 �����������ֵĿ�Ⱥ͸߶�*/
							  0x008a   /* km �����е������ַ�,�ϰ벿��k,�°벿��m*/
		                      };
	/*��ѯ12�����͵�ע����������еĿ�Ⱥ͸߶�*/
	ret = ttfGetFontWidthHeight(tfont.font, unicode, CHAR_NUM, fntwidth, fntheight);
	if(-1 == ret)
	{ 
		/*��ѯʧ�����һ��Ĭ��ֵ����,��������ߺ������*/
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
	ע�ǻ���ǰ��׼������,����ģ��ͶӰ��������á�ȫ��ע���������㡢��ͬ����ע��������ָ�������еĿ�Ⱥ͸߶�
	������view_type--�ӽ�����
	      tfont--������
		  viewport--�ӿڴ�С
		  matrix--ԭ�������Ƶ�ģ��ͶӰ����
    ����ֵ��0--�ɹ�
*/
static f_int32_t qtndisplaypre(f_int32_t view_type, sTtfFont tfont, f_int32_t viewport[4], f_float64_t matrix[16])
{
	/*�Ǹ����ӽ�������Ȳ���,�����ӽǹر���Ȳ���*/
	if( view_type != eVM_OVERLOOK_VIEW )
	{
	    glEnableEx(GL_DEPTH_TEST);
	}
	else
	{
	    glDisableEx(GL_DEPTH_TEST);
	}

	glDisableEx(GL_CULL_FACE);
		
	/*����ԭ����ͶӰģ�Ӿ���*/
	namelistUpdateCoordParam(matrix);

	/*����ͶӰ����,��ͶӰ*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	/*2016-9-20 15:51 ��Ǫ�� ��֪ʲôԭ�򣨵��㽣���Գ����ģ���Զ���ý���������ó�(0.0f, -1.0f)��
	  ������Ļ������ƶ���ʱ(��ע�Ǳ����)���ڵ������ã������ڵ���������*/
	glOrtho(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, -1.0f); 

	/*����ģ�Ӿ���*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 	
	/*ע�����ǰ��OpenGL״̬����,ʹ����ά����Ŀ¼�е�����*/
	ttfDrawStringPre(tfont.font);
	/* ���ñ߿����ɫ */
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tfont.color_edge); 
	{
	    f_int32_t i = 0;
		/*�ܵ���ʾ��ע��������ʼ��Ϊ0*/
	    qtn_display_count = 0;
		/*ÿһ���ע��������ʼ��Ϊ0,�ܹ���6��*/
	    for(i = 0; i < MAPNOTECLASS; i++)	
	        qtn_count[i] = 0;
	}
	
	{
	    static f_int32_t tempcnt = 0;
		/*��ѯ��ͬ�ĵ���ע�Ƿ����������еĿ�Ⱥ͸߶�,��λ������*/
	    if(0 == tempcnt)	
	    {
	        getFntWidthHeight(tfont);
	        tempcnt = 1;	
	    }
	}
	
	return 0;
}

/*
	ע����ײ��ע�ǻ�����ʾ�����ƺ��״̬�ָ�
	������view_type--�ӽ�����
	      tfont--������
    ����ֵ��0--�ɹ�
*/
static f_int32_t qtndisplaypro(f_int32_t view_type, sTtfFont tfont)
{
	/*ע����ײ��ע�ǻ�����ʾ*/
	qtnTextDisplay(tfont, view_type);
	
	/*ע��������OpenGL״̬�ָ�*/
	ttfDrawStringPro(tfont.font);
	/*�Ǹ����ӽǹر���Ȳ���*/
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
	ȫ��qtn��Ϣ�����ʼ��
	��������
    ����ֵ����
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
	����ָ��ͼ��Ĺ��ܺ���
	������map_andle -- ��createMapHandle���ص�VOIDPtr�����ȫ��Ψһ����
	      layer -- ͼ���, 0..MAXMAPLAYER-1,Ŀǰֻ��һ��ͼ��,��0
    ����ֵ��
	      ��
*/
void qtnSetLayerFunc(VOIDPtr map_handle, f_int32_t layer)
{
	/*����ͼ��0�ĸ������ܺ���,����Ϊȫ��qtn��Ϣ����*/
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
