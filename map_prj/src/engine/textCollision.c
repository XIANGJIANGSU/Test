/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: textCollision.c
**
** ����: 
**
** ����ĺ���: 
**      
**                            
** ���ע��: ���ά��vecMapCollosion.c�е���ײ�㷨һ��
**
** ���ߣ�
**     ���㽣(DXJ)��2015��8�¿�ʼ��д���ļ���
** 
**
** ������ʷ:
**     2015��8��11�� �������ļ���
**     xxxx��x��xx�� ��ʼ�汾���� 
**
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#include <string.h>
#include <math.h>
#include "textCollision.h"
#include "../mapApp/appHead.h"
#include "../define/macrodefine.h"

/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** ��������
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
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
extern sSCREENPARAM screen_param;

//��Ϊ1��ʹ���ϵ���ײ����㷨(��Ļ����һ�飬���о��ν�����ײ)����Ϊ0��ʹ���µ���ײ����㷨(��Ļ���ֳ�3*64�飬���о��ηֱ������ײ)
#define USE_COLLISION_3D 0

#if USE_COLLISION_3D
static sMIDRECT3D sRectArray_3D[MAX_NO_COLLISION_POINT_ELEMENT_NUMBER];
static int sRectNumberInArray_3D = 0;
#else
//#define SCRWIDTH			800
//#define SCRHEIGHT			800
//#define BLOCKWIDTH			512		//��Ŀ��
//#define BLOCKHEIGHT			32		//��ĸ߶�
//#define BLOCKCOLS			((SCRWIDTH/BLOCKWIDTH)+1)
//#define BLOCKROWS			((SCRHEIGHT/BLOCKHEIGHT)+3)
//#define COLLISION_ARRAY_SIZE	64	//ÿһ���пɱ���ľ�������

//static sMIDRECT3D g_RectExist[BLOCKCOLS][BLOCKROWS][COLLISION_ARRAY_SIZE]={0};
//static int g_RectNum[BLOCKCOLS][BLOCKROWS]={0};

static int BLOCKWIDTH_3D = 512;		//��Ŀ��
static int BLOCKHEIGHT_3D	= 32;		//��ĸ߶�
static int BLOCKCOLS_3D = 3;     //((SCRWIDTH/BLOCKWIDTH)+1)
static int BLOCKROWS_3D = 27;    //((SCRHEIGHT/BLOCKHEIGHT)+3)
#define COLLISION_ARRAY_SIZE_3D	64	//ÿһ���пɱ���ľ�������

static sMIDRECT3D g_RectExist_3D[3][64][COLLISION_ARRAY_SIZE_3D]={0};  //���п��ŵ���Ӿ��β���
static int g_RectNum_3D[3][64]={0}; //����Ļ�ֳ�3��64�У���¼ÿ�����ע����Ӿ��θ�����ÿ�������64��
#endif
/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

/*.BH--------------------------------------------------------
**
** ������: 
**
** ����: 
**
** �������:  �ӿڿ�Ⱥ͸߶ȡ�
**
** ��������� �ޡ�
**
** ����ֵ�� �ޡ�
**
** ���ע��:  
**
**.EH--------------------------------------------------------
*/
void collisionInfoInit3d(int view_width, int view_height)
{
#if USE_COLLISION_3D
	/*��յ�ǰ֡������ײ����ע����Ӿ���*/
	sRectNumberInArray_3D = 0;
#else
	/*������Ļ��ת���򣬰���Ļ�ֳ�һ�������ο飬���ÿ�Ŀ�Ⱥ͸߶ȣ�Ĭ��Ϊ512��32*/
    switch(screen_param.rotate_type)
	{   /* ��Ļ��ת */
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
	
	/*�����ӿڿ�����ÿ�Ŀ�ȣ�Ϊ�ӿڵ�һ��*/
    BLOCKWIDTH_3D = view_width / 2;
	/*���ӿڷֳ�3�У�Ϊ����3����*/
    BLOCKCOLS_3D = 3;
	/*�����ӿڸ߶Ƚ��ӿڷֳ�(h/32+3)��,���Ϊ64�У�Ϊ�μ�3����*/
    BLOCKROWS_3D = view_height / BLOCKHEIGHT_3D + 3;
    if(BLOCKROWS_3D > 64)
        BLOCKROWS_3D = 64;

	/*����ӿڵ����еĿ���ע����Ӿ��εĸ���*/
    memset(g_RectNum_3D, 0, sizeof(g_RectNum_3D));

#endif
}


/*.BH--------------------------------------------------------
**
** ������: calcMidRect3d
**
** ����: ��ȡע�ǵ���Ӿ��β���
**
** �������:  sx,sy--����ע�ǻ�����ʼ��λ��
**            symbolSize--����ע�ǵȼ����Ŵ�С
**            textwidth--����ע���ܿ��
**            fontheight--����ע���ܸ߶�
**            alignType--����ע�Ƕ��䷽ʽ
**
** ��������� �ޡ�
**
** ����ֵ�� ע�ǵ���Ӿ��β�����
**
** ���ע��:  
**
**.EH--------------------------------------------------------
*/
sMIDRECT3D calcMidRect3d(float sx, float sy, int symbolSize, float textLen, int fontheight, int alignType)
{
	float deltax, deltay;
	float startx, starty, width, height;
	sMIDRECT3D rect;

	/*���ݲ�ͬ�Ķ��뷽ʽ����ע����ʼ���λ��*/
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
	/*����õ�ע����ʼ��λ�á�ע���ܿ�Ⱥ��ܸ߶�*/
	startx = sx + deltax;
	starty = sy + deltay;
	width  = textLen;
	height = (float)fontheight;
	/*����õ�ע����Ӿ��ε����ĵ����ꡢ���εİ�ߺͰ��*/
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
** ������: isRectCollide3d
**
** ����: �ж����������Ƿ��ཻ
**
** �������:  psrcRect--����1ָ��
**            pdstRect--����2ָ�롣
**
** ��������� �ޡ�
**
** ����ֵ�� 1-�ཻ,0-���ཻ��
**
** ���ע��:  
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
** ������: isPointCollision3d
**
** ����: �ж�ע�ǵ���Ӿ����Ƿ���Ҫ��ײ
**
** �������:  sx,sy--����ע�ǻ�����ʼ��λ��
**            symbolSize--����ע�ǵȼ����Ŵ�С
**            textwidth--����ע���ܿ��
**            fontheight--����ע���ܸ߶�
**            alignType--����ע�Ƕ��䷽ʽ
**
** ��������� �ޡ�
**
** ����ֵ�� 0-����ײ,1-��ײ��
**
** ���ע��:  
**
**.EH--------------------------------------------------------
*/
int isPointCollision3d(float sx, float sy, int symbolSize, float textwidth, int fontheight, int alignType)
{
	int ret = 0;
	int i = 0;
	sMIDRECT3D rect;

	/*��ȡע�ǵ����Ӿ��β���*/
	rect = calcMidRect3d(sx, sy, symbolSize, textwidth, fontheight, alignType);

#if USE_COLLISION
	/*ѭ�����Ѿ����Ƶ�ע�ǵ���Ӿ��ν����ཻ�Ƚ�*/
	for(i=0; i<sRectNumberInArray_3D; i++)
	{
		/*���������һ�������ཻ����ֱ���˳���������������αȽ�*/
		ret = isRectCollide3d(&rect, &sRectArray[i]);
		if(1 == ret)
		{
			break;
		}
	}
	/*����ཻ�����ע�ǵ㷢����ײ��ֱ�ӷ��أ������л���*/
	if(i != sRectNumberInArray_3D)
	{
		return 1;
	}
	
	/*������ཻ���жϾ��������Ƿ��ܴ�ţ��޷���ţ����ע�ǲ���ײ��Ҳ��������ײ����������*/
	if(sRectNumberInArray_3D >= MAX_NO_COLLISION_POINT_ELEMENT_NUMBER)
	{
		/*������������ֹ�������������ӡһ����������һ��*/
		//_DEBUG_PRINT_ERROR_
		return 0;
	}
	/*������ཻ���Ҿ������黹�ܴ�ţ��򽫸�ע�Ƿ�����ײ���������У���ײ���θ�������*/
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

	/*ÿ����Ӿ����������4������*/
	/*�ж���Ӿ��������������е���һ�л���������*/
	col[0] = (int)((rect.midX - rect.halfWidth) / BLOCKWIDTH_3D);
	col[1] = (int)((rect.midX + rect.halfWidth) / BLOCKWIDTH_3D);
	/*�ж���Ӿ���������һ���У����ǿ�Խ����*/
	if (col[0]<0 || col[0]>=BLOCKCOLS_3D )		col[0] = 0;
	if (col[1]<0 || col[1]>=BLOCKCOLS_3D )		col[1] = 0;
	/*1��ʾ����Ӿ�������һ���У�2��ʾ����Ӿ��ο�Խ����*/
	cols = (col[0] == col[1]) ? 1 : 2;

	/*�ж���Ӿ��������������е���һ�л���������*/
	row[0] = (int)((rect.midY - rect.halfHeight) / BLOCKHEIGHT_3D);
	row[1] = (int)((rect.midY + rect.halfHeight) / BLOCKHEIGHT_3D);
	/*�ж���Ӿ���������һ���У����ǿ�Խ����*/
	if (row[0]<0 || row[0]>=BLOCKROWS_3D) row[0]=0;
	if (row[1]<0 || row[1]>=BLOCKROWS_3D) row[1]=0;
	/*1��ʾ����Ӿ�������һ���У�2��ʾ����Ӿ��ο�Խ����*/
	rows = (row[0] == row[1]) ? 1 : 2;

	/*NX��ʾ����Ӿ�������4����(���)���кţ�NX��ʾ����Ӿ�������4����(���)���к�*/
#define NX		col[i]
#define NY		row[j]
	/*���б�������Ӿ�������Խ�Ŀ�*/
	for (j=0; j<rows; j++)
	{
		/*���б�������Ӿ�������Խ�Ŀ�*/
		for (i=0; i<cols; i++)
		{
			/*���α�����ǰ���е�������Ӿ���*/
			for (k=0; k<g_RectNum_3D[NX][NY]; k++)
			{
				/*�жϵ�ǰ��Ӿ����Ƿ������еľ��η����ཻ�����ཻ����ֱ������*/
				if ( isRectCollide3d(&rect, &g_RectExist_3D[NX][NY][k]) )
				{
					bCollide = 1;
					break;
				}
			}
			/*���ཻ��ֱ������*/
			if ( bCollide )
				break;
		}
		/*���ཻ��ֱ������*/
		if ( bCollide )
			break;
	}

	/* ���û����ײ�����µľ��μ�����У�һ�������п���������4������ */
	if ( !bCollide )
	{
		for (j=0; j<rows; j++)
		{
			for (i=0; i<cols; i++)
			{
				/*ÿ���������64�����Σ�������������ע���������ƣ���������ײ*/
				if ( g_RectNum_3D[NX][NY] < COLLISION_ARRAY_SIZE_3D )
				{
					/*�����μ�¼��Ӧ���У���Ӧ���еľ�����������1*/
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
