/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: textCollision.c
**
** 描述: 
**
** 定义的函数: 
**      
**                            
** 设计注记: 与二维的vecMapCollosion.c中的碰撞算法一致
**
** 作者：
**     邓秀剑(DXJ)，2015年8月开始编写本文件。
** 
**
** 更改历史:
**     2015年8月11日 创建本文件。
**     xxxx年x月xx日 初始版本发布 
**
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 头文件引用
**-----------------------------------------------------------
*/
#include <string.h>
#include <math.h>
#include "textCollision.h"
#include "../mapApp/appHead.h"
#include "../define/macrodefine.h"

/*-----------------------------------------------------------
** 文字量和宏声明
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 类型声明
**-----------------------------------------------------------
*/
typedef struct
{
	float midX;
	float midY;
	float halfWidth;
	float halfHeight;
}sMIDRECT3D;

/*-----------------------------------------------------------
** 全局变量定义
**-----------------------------------------------------------
*/
extern sSCREENPARAM screen_param;

//宏为1则使用老的碰撞检测算法(屏幕就是一块，块中矩形进行碰撞)，宏为0则使用新的碰撞检测算法(屏幕最多分成3*64块，块中矩形分别进行碰撞)
#define USE_COLLISION_3D 0

#if USE_COLLISION_3D
static sMIDRECT3D sRectArray_3D[MAX_NO_COLLISION_POINT_ELEMENT_NUMBER];
static int sRectNumberInArray_3D = 0;
#else
//#define SCRWIDTH			800
//#define SCRHEIGHT			800
//#define BLOCKWIDTH			512		//块的宽度
//#define BLOCKHEIGHT			32		//块的高度
//#define BLOCKCOLS			((SCRWIDTH/BLOCKWIDTH)+1)
//#define BLOCKROWS			((SCRHEIGHT/BLOCKHEIGHT)+3)
//#define COLLISION_ARRAY_SIZE	64	//每一块中可保存的矩形数量

//static sMIDRECT3D g_RectExist[BLOCKCOLS][BLOCKROWS][COLLISION_ARRAY_SIZE]={0};
//static int g_RectNum[BLOCKCOLS][BLOCKROWS]={0};

static int BLOCKWIDTH_3D = 512;		//块的宽度
static int BLOCKHEIGHT_3D	= 32;		//块的高度
static int BLOCKCOLS_3D = 3;     //((SCRWIDTH/BLOCKWIDTH)+1)
static int BLOCKROWS_3D = 27;    //((SCRHEIGHT/BLOCKHEIGHT)+3)
#define COLLISION_ARRAY_SIZE_3D	64	//每一块中可保存的矩形数量

static sMIDRECT3D g_RectExist_3D[3][64][COLLISION_ARRAY_SIZE_3D]={0};  //所有块存放的外接矩形参数
static int g_RectNum_3D[3][64]={0}; //将屏幕分成3列64行，记录每个块的注记外接矩形个数，每个块最多64个
#endif
/*-----------------------------------------------------------
** 外部变量声明
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 函数定义
**-----------------------------------------------------------
*/

/*.BH--------------------------------------------------------
**
** 函数名: 
**
** 描述: 
**
** 输入参数:  视口宽度和高度。
**
** 输出参数： 无。
**
** 返回值： 无。
**
** 设计注记:  
**
**.EH--------------------------------------------------------
*/
void collisionInfoInit3d(int view_width, int view_height)
{
#if USE_COLLISION_3D
	/*清空当前帧参与碰撞检测的注记外接矩形*/
	sRectNumberInArray_3D = 0;
#else
	/*根据屏幕旋转方向，把屏幕分成一个个矩形块，设置块的宽度和高度，默认为512和32*/
    switch(screen_param.rotate_type)
	{   /* 屏幕旋转 */
	    default:
	    case eROTATE_CW_0:
	    case eROTATE_CW_180:
	        BLOCKWIDTH_3D  = 512;
	        BLOCKHEIGHT_3D = 32;
	    break;
	    case eROTATE_CW_90:
	    case eROTATE_CW_270:
	        BLOCKWIDTH_3D  = 32;
	        BLOCKHEIGHT_3D = 512;
	    break;
	}
	
	/*根据视口宽度设置块的宽度，为视口的一半*/
    BLOCKWIDTH_3D = view_width / 2;
	/*将视口分成3列，为何是3？？*/
    BLOCKCOLS_3D = 3;
	/*根据视口高度将视口分成(h/32+3)行,最多为64行，为何加3？？*/
    BLOCKROWS_3D = view_height / BLOCKHEIGHT_3D + 3;
    if(BLOCKROWS_3D > 64)
        BLOCKROWS_3D = 64;

	/*清空视口的所有的块中注记外接矩形的个数*/
    memset(g_RectNum_3D, 0, sizeof(g_RectNum_3D));

#endif
}


/*.BH--------------------------------------------------------
**
** 函数名: calcMidRect3d
**
** 描述: 获取注记的外接矩形参数
**
** 输入参数:  sx,sy--地名注记绘制起始点位置
**            symbolSize--地名注记等级符号大小
**            textwidth--地名注记总宽度
**            fontheight--地名注记总高度
**            alignType--地名注记对其方式
**
** 输出参数： 无。
**
** 返回值： 注记的外接矩形参数。
**
** 设计注记:  
**
**.EH--------------------------------------------------------
*/
sMIDRECT3D calcMidRect3d(float sx, float sy, int symbolSize, float textLen, int fontheight, int alignType)
{
	float deltax, deltay;
	float startx, starty, width, height;
	sMIDRECT3D rect;

	/*根据不同的对齐方式调整注记起始点的位置*/
	switch(alignType)
	{
		case TEXT_ALIGN_LEFT:
			deltax = -textLen - symbolSize;
			deltay = 0.0f;
			break;
		case TEXT_ALIGN_TOP:
			deltax = -textLen/2.0f;
			deltay = 10.0f + symbolSize;
			break;			
		case TEXT_ALIGN_BOTTOM:
			deltax = -textLen/2.0f;
			deltay = (float)(-fontheight - symbolSize);
			break;
		case TEXT_ALIGN_MIDDLE:
			deltax = -textLen/2.0f;
			deltay = 0.0f;
			break;
		default:
			deltax = (float)(symbolSize);
			deltay = 0.0f;
			break;			
	}
	/*计算得到注记起始点位置、注记总宽度和总高度*/
	startx = sx + deltax;
	starty = sy + deltay;
	width  = textLen;
	height = (float)fontheight;
	/*计算得到注记外接矩形的中心点坐标、矩形的半高和半宽*/
	rect.halfWidth  = width/2.0f;
	rect.halfHeight = height/2.0f;
	rect.midX = startx + rect.halfWidth;
	rect.midY = starty + rect.halfHeight;
	rect.halfWidth += COLLISION_EXTERNSION_3D;
	rect.halfHeight += COLLISION_EXTERNSION_3D;
	
	return rect;
}

/*.BH--------------------------------------------------------
**
** 函数名: isRectCollide3d
**
** 描述: 判断两个矩形是否相交
**
** 输入参数:  psrcRect--矩形1指针
**            pdstRect--矩形2指针。
**
** 输出参数： 无。
**
** 返回值： 1-相交,0-不相交。
**
** 设计注记:  
**
**.EH--------------------------------------------------------
*/
int isRectCollide3d(const sMIDRECT3D *psrcRect, const sMIDRECT3D *pdstRect)
{
	double	dx, dy;
	dx = fabs(psrcRect->midX - pdstRect->midX);
	dy = fabs(psrcRect->midY - pdstRect->midY);
	if (   (dx < (psrcRect->halfWidth + pdstRect->halfWidth))
		&& (dy < (psrcRect->halfHeight + pdstRect->halfHeight))
		)
	{
		return 1;
	}
	return 0;
}


/*.BH--------------------------------------------------------
**
** 函数名: isPointCollision3d
**
** 描述: 判断注记的外接矩形是否需要碰撞
**
** 输入参数:  sx,sy--地名注记绘制起始点位置
**            symbolSize--地名注记等级符号大小
**            textwidth--地名注记总宽度
**            fontheight--地名注记总高度
**            alignType--地名注记对其方式
**
** 输出参数： 无。
**
** 返回值： 0-不碰撞,1-碰撞。
**
** 设计注记:  
**
**.EH--------------------------------------------------------
*/
int isPointCollision3d(float sx, float sy, int symbolSize, float textwidth, int fontheight, int alignType)
{
	int ret = 0;
	int i = 0;
	sMIDRECT3D rect;

	/*获取注记点的外接矩形参数*/
	rect = calcMidRect3d(sx, sy, symbolSize, textwidth, fontheight, alignType);

#if USE_COLLISION
	/*循环与已经绘制的注记点外接矩形进行相交比较*/
	for(i=0; i<sRectNumberInArray_3D; i++)
	{
		/*如果与其中一个矩形相交，则直接退出，不再与其余矩形比较*/
		ret = isRectCollide3d(&rect, &sRectArray[i]);
		if(1 == ret)
		{
			break;
		}
	}
	/*如果相交，则该注记点发生碰撞，直接返回，不进行绘制*/
	if(i != sRectNumberInArray_3D)
	{
		return 1;
	}
	
	/*如果不相交，判断矩形数组是否还能存放，无法存放，则该注记不碰撞，也不放入碰撞矩形数组中*/
	if(sRectNumberInArray_3D >= MAX_NO_COLLISION_POINT_ELEMENT_NUMBER)
	{
		/*不做操作，防止数组溢出，但打印一个错误，提醒一下*/
		//_DEBUG_PRINT_ERROR_
		return 0;
	}
	/*如果不相交，且矩形数组还能存放，则将该注记放入碰撞矩形数组中，碰撞矩形个数自增*/
	sRectArray_3D[sRectNumberInArray] = rect;
	sRectNumberInArray_3D++;

	return 0;
#else
{
	#define OFFSET_X	40
	int		col[2], row[2];
	int		cols, rows;
	int		j, k;
	int		bCollide = 0;

	/*每个外接矩形最多落在4个块内*/
	/*判断外接矩形落在所有列中的哪一列或则哪两列*/
	col[0] = (int)((rect.midX - rect.halfWidth) / BLOCKWIDTH_3D);
	col[1] = (int)((rect.midX + rect.halfWidth) / BLOCKWIDTH_3D);
	/*判断外接矩形是落在一列中，还是跨越两列*/
	if (col[0]<0 || col[0]>=BLOCKCOLS_3D )		col[0] = 0;
	if (col[1]<0 || col[1]>=BLOCKCOLS_3D )		col[1] = 0;
	/*1表示该外接矩形落在一列中，2表示该外接矩形跨越二列*/
	cols = (col[0] == col[1]) ? 1 : 2;

	/*判断外接矩形落在所有行中的哪一行或则哪两行*/
	row[0] = (int)((rect.midY - rect.halfHeight) / BLOCKHEIGHT_3D);
	row[1] = (int)((rect.midY + rect.halfHeight) / BLOCKHEIGHT_3D);
	/*判断外接矩形是落在一行中，还是跨越两行*/
	if (row[0]<0 || row[0]>=BLOCKROWS_3D) row[0]=0;
	if (row[1]<0 || row[1]>=BLOCKROWS_3D) row[1]=0;
	/*1表示该外接矩形落在一行中，2表示该外接矩形跨越二行*/
	rows = (row[0] == row[1]) ? 1 : 2;

	/*NX表示该外接矩形所处4个块(最多)的列号，NX表示该外接矩形所处4个块(最多)的行号*/
#define NX		col[i]
#define NY		row[j]
	/*按行遍历该外接矩形所跨越的块*/
	for (j=0; j<rows; j++)
	{
		/*按列遍历该外接矩形所跨越的块*/
		for (i=0; i<cols; i++)
		{
			/*依次遍历当前块中的所有外接矩形*/
			for (k=0; k<g_RectNum_3D[NX][NY]; k++)
			{
				/*判断当前外接矩形是否与已有的矩形发生相交，若相交，则直接跳出*/
				if ( isRectCollide3d(&rect, &g_RectExist_3D[NX][NY][k]) )
				{
					bCollide = 1;
					break;
				}
			}
			/*若相交，直接跳出*/
			if ( bCollide )
				break;
		}
		/*若相交，直接跳出*/
		if ( bCollide )
			break;
	}

	/* 如果没有碰撞，则新的矩形加入块中，一个矩形有可能最多加入4个块中 */
	if ( !bCollide )
	{
		for (j=0; j<rows; j++)
		{
			for (i=0; i<cols; i++)
			{
				/*每个块中最多64个矩形，如果超过，则该注记正常绘制，不进行碰撞*/
				if ( g_RectNum_3D[NX][NY] < COLLISION_ARRAY_SIZE_3D )
				{
					/*将矩形记录对应块中，对应块中的矩形数量自增1*/
					g_RectExist_3D[NX][NY][g_RectNum_3D[NX][NY]] = rect;
					g_RectNum_3D[NX][NY] += 1;
				}
			}
		}
	}

	return bCollide;
}
#endif
	
}
