/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: TerrainPrefile.c
**
** ����: ���ļ�������ֱ��������ͼVSD����صĺ�����
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
**		2017-6-26 9:06 LPF �������ļ�
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#include <math.h>
#include "../utility/tools.h"
#include "../mapApp/bil.h"
#include "../mapApi/mapApi.h"
#include "../mapApp/appHead.h"
#include "../mapApp/mapApp.h"
#include "../mapApi/common.h"
#include "../mapApi/initApi.h"
#include "../mapApi/paramGetApi.h"
//#include "../../vecMapEngine_prj/2dEngine/vecMapFont.h"

/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*/

#ifdef WIN32
#define SPANPICPATH "../��ͼ����/DataFile/SpanPic.bmp"
#else
#define SPANPICPATH "/ahci00/mapData3d/DataFile/SpanPic.bmp"
#endif



/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
extern VOIDPtr pMapDataSubTree;

//��¼��һ�ηɻ��ľ�γ�߼�����,2·�ֱ��¼
static f_float64_t AirPlane_oldpos[2][4] = {{0.0},{0.0}};

//��¼��һ��ˮƽ����ÿ�׶������ء���ֱ����ÿ�׶�������
static f_float64_t s_HWinPerMeter_old[2] = {0.0,0.0};
static f_float64_t s_VWinPerMeter_old[2] = {0.0,0.0};

//�Ƿ�ʹ����Ƭ�߶�,0-��ʹ��,1-ʹ��
static f_int8_t use_hei_tile = 0;  

//��¼��һʱ����Ƭ���ص�����,��ʹ����Ƭ�߶�ʱ,����ݸñ����ж���Ƭ�Ƿ����,�Ӷ������Ƿ��������ͼ�еĸ߶�ֵ
static f_int32_t loadedTileNumber_old = 0;

static f_int32_t listIdNeedRegenerate[2] = {0,0};
/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

extern int textOutPutOnScreen(int fontType,float viewportX, float viewportY, unsigned char* stringOut, sColor4f textColor);

/*
 *	����ͶӰ�����ģ�ͱ任����
 *	��������תΪ��Ļ����
 */
void PushWorldCoord2CliendCoord(f_float64_t left, f_float64_t right, f_float64_t bottom, f_float64_t top)
{
	glMatrixMode(GL_PROJECTION);						// ѡ��ͶӰ����
	glPushMatrix();										// ���浱ǰ��ͶӰ����
	glLoadIdentity();									// ����ͶӰ����
	glOrtho(left, right, bottom, top, -1, 1);			//������ͶӰ�Ŀ�������
	glMatrixMode(GL_MODELVIEW);							// ѡ��ģ�ͱ任����
	glPushMatrix();										// ���浱ǰ��ģ�ͱ任����
	glLoadIdentity();									// ����ģ�ͱ任����	
}

/*
 *	�ָ�ͶӰ�����ģ�ͱ任����
 *	��Ļ����תΪ��������
 */
void PopWorldCoord2CliendCoord()
{
	glMatrixMode(GL_PROJECTION);						// ѡ��ͶӰ����
	glPopMatrix();										// ����Ϊ����ľ���
	glMatrixMode(GL_MODELVIEW);							// ѡ��ģ�;���
	glPopMatrix();										// ����Ϊ����ľ���
}

/*
*	����ɫ�������ݲ�ͬ��ɫ�����
*/
// void static DrawColorQuads(	float color_area[4] , int hei_low, int hei_hi)
// {
// 	glColor4fv(color_area);
// 	glBegin(GL_QUADS);
// 	glVertex2i(s_StartWinPosX, s_StartWinPosY+hei_low);
// 	glVertex2i(s_StartWinPosX + s_WinWidth, s_StartWinPosY+hei_low);
// 	glVertex2i(s_StartWinPosX + s_WinWidth, s_StartWinPosY + hei_hi);
// 	glVertex2i(s_StartWinPosX, s_StartWinPosY + hei_hi);
// 	glEnd();
// 
// }

/*.BH--------------------------------------------------------
**
** ������: DrawQuadsEx
**
** ����:  ����Ļ�ϻ���һ������ɫ��
**
** �������:  hei_low������ɫ���±ߴ�ֱ����������½ǵ�����ֵ
**            hei_hi������ɫ���ϱߴ�ֱ����������½ǵ�����ֵ
**            pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
static void DrawQuadsEx(f_float32_t hei_low, f_float32_t hei_hi, sGLRENDERSCENE* pScene)
{
	/*��VSD�����ṹ���л�ȡVSD������ʼλ�á���Ⱥͳ���(��λΪ����)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	/*���ƾ���ɫ��*/
	glBegin(GL_QUADS);
	glVertex2f(s_StartWinPosX, s_StartWinPosY + hei_low);
	glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + hei_low);
	glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + hei_hi);
	glVertex2f(s_StartWinPosX, s_StartWinPosY + hei_hi);
	glEnd();

}


/*.BH--------------------------------------------------------
**
** ������: DrawQuardTerrain
**
** ����:  ����Ԥ��ĸ澯ɫ����ɫֵ,��VSD�ľ��ο��л��ƶ�������ɫ��
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
static void  DrawQuardTerrain(sGLRENDERSCENE* pScene)
{
	/*��VSD�����ṹ���л�ȡVSD������ʼλ�á���Ⱥͳ���(��λΪ����)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	f_float32_t s_winHeiMeter = 1000.0f;
	f_float32_t prefile_height[5] = {0.0f};
	f_float32_t fly[5] = {0.0f};
	f_int32_t color_num = 6;
	f_int32_t i = 0;

	/*��VSD�����ṹ���л�ȡ��ֱ�����������ʵ�ʾ���(��)��ɫ��������*/
	s_winHeiMeter = pScene->auxnavinfo_param.prefile_param.s_winHeiMeter * 1.0f;
	color_num = pScene->auxnavinfo_param.prefile_param.color_num;

	/*ɫ��������Ч�ж�,[2,6]��Ч*/
	if( (color_num < 2) || (color_num > 6))
	{
		//printf("input color_num error!\n");
		return;
	}

	/*��ȡɫ������ĸ߶�,�߶ȵ�����Ϊɫ������-1*/
	for(i = 0;i < color_num-1; i++)
	{
		prefile_height[i] = pScene->auxnavinfo_param.prefile_param.prefile_height[i];
	}


	/* �澯��ɫ����Ϊ2ʱ������ɫ���ķֽ�߶ȣ�������ɫ�� */
	/*
	  �߶ȷ�Χ				                                RGB						VSD���ڸ߶ȷ�Χ
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight			
	  -s_winHeiMeter -- 	prefile_height[0]			prefile_color[1]		  0	-- fly[0]
	*/
	if(2 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
		(prefile_height[0] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(0, fly[0], pScene);

	}

	/* �澯��ɫ����Ϊ3ʱ������ɫ���ķֽ�߶ȣ�������ɫ�� */
	/*
	  �߶ȷ�Χ				                                RGB						VSD���ڸ߶ȷ�Χ
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]		
	  -s_winHeiMeter -- 	prefile_height[1]			prefile_color[2]		  0	-- fly[1]
	*/
	if(3 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
		    (prefile_height[1] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(0, fly[1], pScene);

	}

	/* �澯��ɫ����Ϊ4ʱ������ɫ���ķֽ�߶ȣ�������ɫ�� */
	/*
	  �߶ȷ�Χ				                                RGB						VSD���ڸ߶ȷ�Χ
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]	
	  prefile_height[2] -- 	prefile_height[1]			prefile_color[2]		  fly[2]--fly[1]	
	  -s_winHeiMeter -- 	prefile_height[2]			prefile_color[3]		  0	-- fly[2]
	*/
	if(4 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
			(prefile_height[2] > prefile_height[1]) ||
		    (prefile_height[2] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(fly[2], fly[1], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
		DrawQuadsEx(0, fly[2], pScene);
	}

	/* �澯��ɫ����Ϊ5ʱ������ɫ���ķֽ�߶ȣ�������ɫ�� */
	/*
	  �߶ȷ�Χ				                                RGB						VSD���ڸ߶ȷ�Χ
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]	
	  prefile_height[2] -- 	prefile_height[1]			prefile_color[2]		  fly[2]--fly[1]
	  prefile_height[3] -- 	prefile_height[2]			prefile_color[3]		  fly[3]--fly[2]	
	  -s_winHeiMeter -- 	prefile_height[3]			prefile_color[4]		  0	-- fly[4]
	*/
	if(5 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
			(prefile_height[2] > prefile_height[1]) ||
			(prefile_height[3] > prefile_height[2]) ||
		    (prefile_height[3] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(fly[2], fly[1], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
		DrawQuadsEx(fly[3], fly[2], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[4][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][3]);
		DrawQuadsEx(0, fly[3], pScene);
	}

	/* �澯��ɫ����Ϊ6ʱ������ɫ���ķֽ�߶ȣ�������ɫ�� */
	/*
	  �߶ȷ�Χ				                                RGB						VSD���ڸ߶ȷ�Χ
	  prefile_height[0] -- 	s_winHeiMeter			    prefile_color[0]		  fly[0]--s_WinHeight	
	  prefile_height[1] -- 	prefile_height[0]			prefile_color[1]		  fly[1]--fly[0]	
	  prefile_height[2] -- 	prefile_height[1]			prefile_color[2]		  fly[2]--fly[1]
	  prefile_height[3] -- 	prefile_height[2]			prefile_color[3]		  fly[3]--fly[2]	
	  prefile_height[4] -- 	prefile_height[3]			prefile_color[4]		  fly[4]--fly[3]
	  -s_winHeiMeter -- 	prefile_height[4]			prefile_color[5]		  0	-- fly[4]
	*/
	if(6 == color_num)
	{
		if( (prefile_height[0] > s_winHeiMeter) || 
			(prefile_height[1] > prefile_height[0]) ||
			(prefile_height[2] > prefile_height[1]) ||
			(prefile_height[3] > prefile_height[2]) ||
			(prefile_height[4] > prefile_height[3]) ||
		    (prefile_height[4] < -s_winHeiMeter) )
		{
			//printf("input prefile_height error!\n");
			return;
		}

		for(i = 0;i < color_num-1; i++)
		{
			fly[i] = (s_winHeiMeter + prefile_height[i]) / s_winHeiMeter * (s_WinHeight / 2);
		}

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
		DrawQuadsEx(fly[0], s_WinHeight, pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
		DrawQuadsEx(fly[1], fly[0], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
		DrawQuadsEx(fly[2], fly[1], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
		DrawQuadsEx(fly[3], fly[2], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[4][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][3]);
		DrawQuadsEx(fly[4], fly[3], pScene);

		glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[5][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][3]);
		DrawQuadsEx(0, fly[4], pScene);
	}


#if 0 //�澯�����̶��Ļ��Ʒ���,�ѷ���
	//����ɫ��,ֱ�ӻ���,����ʹ����ʾ�б�
#if 0
	
	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[5][3]);

	if(pScene->prefile_param_used.s_QuardLISTID[0] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[0] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[0], GL_COMPILE);
		{
			DrawQuadsEx(0, fly1,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[0]);
	}	

	


	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[4][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[1] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[1] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[1], GL_COMPILE);
		{
			DrawQuadsEx(fly1, fly2,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[1]);
	}	

	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[3][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[2] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[2] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[2], GL_COMPILE);
		{
			DrawQuadsEx(fly2, fly3,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[2]);
	}	

	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[2][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[3] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[3] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[3], GL_COMPILE);
		{
			DrawQuadsEx(fly3, fly4,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[3]);
	}	

	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[1][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[4] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[4] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[4], GL_COMPILE);
		{
			DrawQuadsEx(fly4, fly5,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[4]);
	}	
	
	glColor4ub(pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][0], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][1], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][2], 
		pScene->auxnavinfo_param.prefile_param.TerrainPrefileColor[0][3]);
	if(pScene->prefile_param_used.s_QuardLISTID[5] == 0)
	{
		pScene->prefile_param_used.s_QuardLISTID[5] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_QuardLISTID[5], GL_COMPILE);
		{
			DrawQuadsEx(fly5, s_WinHeight,pScene);
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_QuardLISTID[5]);
	}	

#else
	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[5][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[5][3]);
	DrawQuadsEx(0, fly[0], pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[4][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[4][3]);
	DrawQuadsEx(fly1, fly2, pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[3][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[3][3]);
	DrawQuadsEx(fly2, fly3, pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[2][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[2][3]);
	DrawQuadsEx(fly3, fly4, pScene);

	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[1][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[1][3]);
	DrawQuadsEx(fly4, fly5, pScene);
	
	glColor4ub(pScene->auxnavinfo_param.prefile_param.prefile_color[0][0], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][1], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][2], 
		pScene->auxnavinfo_param.prefile_param.prefile_color[0][3]);
	DrawQuadsEx(fly5, s_WinHeight, pScene);

#endif

#endif

}

/*.BH--------------------------------------------------------
**
** ������: DrawDynamicHeightMap
**
** ����:  ����Ļ�ϻ��ƶ�̬�ĵ���
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�,����ģ�建�����ķ���������������
**
**.EH--------------------------------------------------------
*/
static void DrawDynamicHeightMap(sGLRENDERSCENE* pScene)
{
	f_int32_t i;
	f_float32_t cur_posx;

	/*��VSD�����ṹ���л�ȡVSD������ʼλ�á���Ⱥͳ���(��λΪ����)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	f_float32_t s_winHeiMeter = pScene->auxnavinfo_param.prefile_param.s_winHeiMeter * 1.0f;


	/*���������ȵĶ�������,�ܹ�303����,���е�1��Ϊ��ʼ��,��2��������ʼ��ˮƽ����������ͬ����ֱ�������겻ͬ,
	�м������299��,������2�������յ�ˮƽ����������ͬ����ֱ�������겻ͬ,���һ����Ϊ�յ�*/
    /*��ʼ��*/
	pScene->prefile_param_used.s_vDetectHeights_cal[0] = s_StartWinPosX;
	pScene->prefile_param_used.s_vDetectHeights_cal[1] = s_StartWinPosY;

	/*�м�301����*/
	for(i = 0; i <= SAMPLE_NUM; ++i)
	{
		/*����ˮƽ��������,��ÿ���������ʼ���ʵ�ʾ���(m)*ˮƽ����ÿ������ֵ+��ʼ��ˮƽ��������*/
		cur_posx = s_StartWinPosX + (pScene->prefile_param_used.s_HWinPerMeter * i * pScene->prefile_param_used.s_WorldSampleInterval);

		pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2] = cur_posx;

		/*���㴹ֱ��������,�ȼ���ÿ����߶�����ʼ��߶�֮��Ĳ�ֵ(m),��ʼ��߶�Ϊ�ɻ��߶�-1000(���1000�ǹ̶�ֵ,��ʱ�ⲿ�޷��ı�)*/
		/*�ٰ���ֵ*��ֱ����ÿ������ֵ+��ʼ�㴹ֱ��������,���ɵõ����*/
		/*�������ڷ�Χ��ȡ�߿�ֵ����*/
		if((pScene->prefile_param_used.s_VWinPerMeter * (pScene->prefile_param_used.s_vDetectHeights[i] - (pScene->geopt_pos.height - s_winHeiMeter)))> s_WinHeight)
		{
			pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = (s_StartWinPosY + s_WinHeight);	
		}
		else if((pScene->prefile_param_used.s_VWinPerMeter * (pScene->prefile_param_used.s_vDetectHeights[i] - (pScene->geopt_pos.height - s_winHeiMeter)))< 0)
		{
			pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = (s_StartWinPosY + 0);
		}
		else
		{
			pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = s_StartWinPosY + ( pScene->prefile_param_used.s_VWinPerMeter * 
				(pScene->prefile_param_used.s_vDetectHeights[i] - (pScene->geopt_pos.height - s_winHeiMeter)) );
		}
	}

	/*�յ�*/
	pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2] = s_StartWinPosX + s_WinWidth;
	pScene->prefile_param_used.s_vDetectHeights_cal[2 + i * 2 + 1] = s_StartWinPosY;
			
	/*������������,����ģ�建�����ķ�ʽ*/
	/* ����ģ����ԣ�Ĭ�ϲ�������ģ����Է�����͸���Ȳ���֮����Ȳ���֮ǰ*/
    glEnableEx(GL_STENCIL_TEST);
	/* ģ�建�����е�ֵ�������3(0x1)���������,��������2(0x1)���бȽ�,�ȽϺ���Ϊ����1. GL_NEVER��ʾģ�������Զ��ͨ��*/
	/* ģ����Բ�ͨ�����ʾ��Ӧ���ص����ɫֵ�������*/
	glStencilFunc(GL_NEVER, 0x1, 0x1);
	/*��һ������sfail�� ���ģ�����ʧ�ܽ���ȡ�Ķ�����
	  �ڶ�������dpfail�� ���ģ�����ͨ����������Ȳ���ʧ��ʱ��ȡ�Ķ�����
	  ����������dppass�� �����Ȳ��Ժ�ģ����Զ�ͨ��������ȡ�Ķ�����
	  GL_INVERT��ʾģ��ֵ��λȡ��
    */
	glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

	/*Ĭ��ģ��ֵΪ0,ģ����Ա�Ȼ��ͨ��,�������Ȳ�����ʾ���������������ȵ�����ģ��ֵΪ0xff(0��λȡ��)*/
#if 0	
	/*���ƶ��������,���ö�������ķ�ʽ*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_DOUBLE, 0, &pScene->prefile_param_used.s_vDetectHeights_cal[0]);

	glDrawElements(GL_TRIANGLE_FAN, (SAMPLE_NUM + 3), GL_UNSIGNED_INT, &pScene->prefile_param_used.pIndex_tri_fan[0]);	

	glDisableClientState(GL_VERTEX_ARRAY);

#else

#if 1
	{
		/*������������,����ֱ�ӻ��Ƶķ�ʽ,��303����,pc������Ӱ��֡��,7200socʵ��Ӱ�첻��*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];
		glBegin(GL_TRIANGLE_FAN);
		for (i=0; i<SAMPLE_NUM + 3; ++i)
		{
			glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
		}
		glEnd();
	}
#else
	{
		/*������ʾ�б�Ļ��Ʒ�ʽ,7200�����Ƶ������������ʾ�б������ᱨ��pvtxbuf malloc failed*/
	    /*���������ҪƵ��������ʾ�б�ᵼ�����������Ի��ǲ���ֱ�ӻ���*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];

		if(pScene->prefile_param_used.s_TerrainPrefileLISTID[5] == 0)
		{
			pScene->prefile_param_used.s_TerrainPrefileLISTID[5] = glGenLists(1);
		}

		if(listIdNeedRegenerate[pScene->scene_index]){
			glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[5], GL_COMPILE);
			{
				glBegin(GL_TRIANGLE_FAN);
				for (i=0; i<SAMPLE_NUM + 3; ++i)
				{
					glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
				}
				glEnd();
			}
			glEndList();
		}
		else
		{
			glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[5]);
		}

	}
#endif

#endif

	

	/* �ػ���Ļ,ֻ����ģ�建��ֵ��0������,ɫ��ֻ����������������ʾ */

	/* ģ�建�����е�ֵ�������3(0x1)���������,��������2(0x1)���бȽ�,�ȽϺ���Ϊ����1. GL_EQUAL��ʾ������ȱȽ�*/
	/* ģ����Բ�ͨ�����ʾ��Ӧ���ص����ɫֵ�������*/
	glStencilFunc(GL_EQUAL,0x1,0x1);
	/*GL_ZERO��ʾģ��ֵ����Ϊ0*/
	glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO); 

	//����ɫ��
	DrawQuardTerrain(pScene);

	/* ����ģ�������Զͨ�� */
	glStencilFunc(GL_ALWAYS,0,0x1);

}

/*.BH--------------------------------------------------------
**
** ������: UpdataTerrainProfile
**
** ����:  ���´�ֱ��������ͼ���趯̬����Ĳ���,��ȡ�ɻ�������ǰ��̽������ڵĸ���������߶�ֵ
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�, ��ȡ�ɻ�������ǰ��̽������ڵĸ���������߶�ֵ
**
**.EH--------------------------------------------------------
*/
void UpdataTerrainProfile(sGLRENDERSCENE* pScene)
{
	f_int32_t i;
	f_uint32_t index;
//	f_float64_t sample_step;				            //Ŀ��Ĳ�ֵ�����
	f_float64_t lon_dest, lat_dest; 	                //Ŀ���ľ�γ��
	f_int16_t hei_dest;                                 //Ŀ���ĸ߶�
//	f_float64_t hei_dest_dom;					        //��Ƭ��ĸ߶�
	f_float64_t lon_final = 0.0, lat_final = 0.0;	    //�յ�ľ�γ��
	f_float64_t lon_step, lat_step;						//��γ�ȵļ��
	f_float32_t s_WinWidth =  0;
	f_float32_t s_WinHeight = 0;
	sMAPHANDLE * pHandle = NULL;                        //���������,ȫ��Ψһ
	
	if(pMapDataSubTree == NULL)
	{
		return;
	}
	pHandle = (sMAPHANDLE *)pMapDataSubTree;

	index = pScene->scene_index;
	if (index > 1)
	{
		return;
	}

	/*��ȡ�ⲿ���õ�VSD���ڵĿ�Ⱥ͸߶�*/
	s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	if(pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance < 1000.0)
	{
		//printf("input s_WorldDetectDistance must > 1km!\n");
		return;
	}

	if(pScene->auxnavinfo_param.prefile_param.s_winHeiMeter <= 0)
	{
		//printf("input s_winHeiMeter must > 0!\n");
		return;
	}

	/*Ĭ�ϲ�������������ʾ�б�*/
	listIdNeedRegenerate[index] = 0;

	/*�����ⲿ���õ�VSD���������㲢���µ��ڲ�ʹ�õ�VSD�����ṹ��prefile_param_used��*/
	/*����VSD����ˮƽ���������̶���֮��Ŀ��(��λΪ����)*/
	pScene->prefile_param_used.s_WinInterval = s_WinWidth / pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum;
	/*����VSD����ˮƽ����Ĳ������(��λΪm),Ĭ��300��������,���һ��������Ϊ�յ�*/
	pScene->prefile_param_used.s_WorldSampleInterval = pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance / SAMPLE_NUM;
	/*����VSD����ˮƽ�������ű���,��ʵ�ʾ���1m������ٸ�����*/
	pScene->prefile_param_used.s_HWinPerMeter = (f_float64_t)s_WinWidth / pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance;
	//���㴹ֱ�������ű���,��ʵ�ʾ���1�״����������
	pScene->prefile_param_used.s_VWinPerMeter =(f_float64_t)( (s_WinHeight/2.0) / (pScene->auxnavinfo_param.prefile_param.s_winHeiMeter*1.0) );
	
	/*�����ʹ����Ƭ�߶�*/
	if(!use_hei_tile){
		/* ���ݾ�γ�Ȼ�ȡ�߳�ֵ,���Ȳ�ѯԭʼDEM����,δ��ѯ����ʹ����Ƭ�߶� */
		if(FALSE == getAltByGeoPos( pScene->geopt_pos.lon, pScene->geopt_pos.lat, &hei_dest))
		{
			/*ʹ����Ƭ�߶�*/
			use_hei_tile = 1;
		}
	}

	//ÿ��5km���¼���һ�Ρ��ɻ��߶ȷ����仯(����0.1)���߷ɻ�����Ƿ����仯(����0.01)ʱ���¼���һ��
	if(	(fabs(s_HWinPerMeter_old[index] - pScene->prefile_param_used.s_HWinPerMeter) < FLT_EPSILON)
		&&(fabs(s_VWinPerMeter_old[index] - pScene->prefile_param_used.s_VWinPerMeter) < FLT_EPSILON)
		&&(fabs(AirPlane_oldpos[index][2] - pScene->geopt_pos.height) < 0.1)
		&&(fabs(AirPlane_oldpos[index][3] - pScene->attitude.yaw) < 0.01)
		&&(caculateGreatCircleDis(
			AirPlane_oldpos[index][0], 
			AirPlane_oldpos[index][1], 
			(f_float32_t)pScene->geopt_pos.lon, 
			(f_float32_t)pScene->geopt_pos.lat) < 5000) )
	{
		/*���ʹ����Ƭ�߶�,����Ƭ�������������仯ʱҲҪ���²�ѯ�߶�*/
		if(use_hei_tile)
		{
			/*��Ƭ��������δ�仯,���������²�ѯ�߶�,�����¼��ʱ�̵���Ƭ��������,�����²�ѯ�߶�*/
			if((loadedTileNumber_old == pHandle->loadedNumber))
			{
				return;
			}else{
				loadedTileNumber_old = pHandle->loadedNumber;
			}
		}else{
			return;
		}
	}
	else
	{
		/*��¼��ǰ�ɻ���γ�߼�����*/
		AirPlane_oldpos[index][0] = pScene->geopt_pos.lon;
		AirPlane_oldpos[index][1] = pScene->geopt_pos.lat;	
		AirPlane_oldpos[index][2] = pScene->geopt_pos.height;
		AirPlane_oldpos[index][3] = pScene->attitude.yaw;	 

		/*��¼��ǰˮƽ�������ű�������ֱ�������ű���*/
		s_HWinPerMeter_old[index] = pScene->prefile_param_used.s_HWinPerMeter;
		s_VWinPerMeter_old[index] = pScene->prefile_param_used.s_VWinPerMeter;
	}

	/*����������ʾ�б�*/
	listIdNeedRegenerate[index] = 1;

	//printf("index=%d %f %f %f %f\n",index,AirPlane_oldpos[index][0],AirPlane_oldpos[index][1],AirPlane_oldpos[index][2],AirPlane_oldpos[index][3]);
	//printf("%f %f\n",s_HWinPerMeter_old[index],s_VWinPerMeter_old[index]);

//	printf("���´�ֱ��������ͼ�ĸ߳�����\n");

	//�����յ㾭γ��
//	rhumbLineDistanceAzimuth(
//		(f_float32_t)pScene->geopt_pos.lon, 
//		(f_float32_t)pScene->geopt_pos.lat,
//		(f_float32_t)(360.0 - pScene->attitude.yaw),
//		(f_float32_t)pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance,
//		&lon_final, &lat_final);

	{
		double temp_lon = 0.0, temp_lat = 0.0;
		//���ݵ�ǰ�ɻ���γ�Ⱥͺ��򣬼�����Զ̽�����Ķ�Ӧ��ľ�γ��
		caculateLonLatOnGreateCircle(pScene->geopt_pos.lon, 
			pScene->geopt_pos.lat, 
			pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance,
			(360.0 - pScene->attitude.yaw), &temp_lon, &temp_lat);

		lon_final = temp_lon;
		lat_final = temp_lat;
		
		//printf("%f,%f,%f\n", lon_final, temp_lon, temp_lon - lon_final);
		//printf("%f,%f,%f\n", lat_final, temp_lat, temp_lat - lat_final);

	}

	//���������֮��ľ�γ�ȼ��
	lon_step = (lon_final - pScene->geopt_pos.lon)/SAMPLE_NUM;
	lat_step = (lat_final - pScene->geopt_pos.lat)/SAMPLE_NUM;

	// ��ȡ�ɻ�ǰ��̽�ⷽ���ϵĸ���������ľ�γ��,
	// ��301����,��1���������뵱ǰ�ɻ�λ�þ�γ����ͬ���߶�Ϊ���θ߶ȡ��м�299�������㡢
	// ���1�����������յ�λ�þ�γ����ͬ���߶�Ϊ���θ߶�
	for(i = 0; i <= SAMPLE_NUM; ++i)
	{
//		sample_step = i * s_WorldSampleInterval;
//		rhumbLineDistanceAzimuth(g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLon, 
//			g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLat,
//			g_pAirPlanes[g_nSelectPlane].m_Entity.m_nYaw,
//			sample_step,
//			&lon_dest, &lat_dest);

		// ��ȡ�ɻ�ǰ��̽�ⷽ���ϵĸ���������ľ�γ,ֱ�Ӳ���ƽ���ֵķ���,
		// �ü��㷽��������ʵ�Ĵ�Բ����,����׼ȷ
		lon_dest = (pScene->geopt_pos.lon + i * lon_step);
		lat_dest = (pScene->geopt_pos.lat + i *lat_step);


		/* ���ݾ�γ�Ȼ�ȡ�߳�ֵ,���Ȳ�ѯԭʼDEM����,δ��ѯ����ʹ����Ƭ�߶�(���ڽϴ����) */
		if(FALSE == getAltByGeoPos( lon_dest, lat_dest, &hei_dest))
		{
			double pz = 0.0;

			/*��ѯ��Ƭ�߶�,��ѯʧ��,�߶�Ĭ��Ϊ0,���ֵѡȡ������,0��һ����ʵ���ڵĸ߶�*/
			if(FALSE == GetZ((sGLRENDERSCENE*)pScene, lon_dest, lat_dest, &pz))
			{
				hei_dest = 0;
			}
			else
			{
				hei_dest = (short)pz;
			}
			
			//��ȡ�ĸ߶�Ϊ��Ƭ�߶�ʱ,״̬����Ϊ1,�߶ȵ���Ƶ���ɫΪ��ɫ
			pScene->prefile_param_used.s_vDetectHeights_state[i] = 1;	

			pScene->prefile_param_used.s_vDetectHeight_color[4*i] = 1.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 1] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 2] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 3] = 1.0;
			
		}
		else
		{
			//��ȡ�ĸ߶�ΪԭʼDEM�߶�ʱ,״̬����Ϊ0,�߶ȵ���Ƶ���ɫΪ��ɫ
			pScene->prefile_param_used.s_vDetectHeights_state[i] = 0;

			pScene->prefile_param_used.s_vDetectHeight_color[4*i] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 1] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 2] = 0.0;
			pScene->prefile_param_used.s_vDetectHeight_color[4*i + 3] = 1.0;
		}

		//�����ѯ���ĸ���������ĸ߶�
		pScene->prefile_param_used.s_vDetectHeights[i] = (double)hei_dest;

	}

}

/*.BH--------------------------------------------------------
**
** ������: InitTriangleFanAndQuadIndex
**
** ����:  ��ʼ��VSD���ڻ��Ƶ��������������ı�������
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ڲ��ӿ�
**
**.EH--------------------------------------------------------
*/
void InitTriangleFanAndQuadIndex(sGLRENDERSCENE* pScene)
{
	int k = 0;
	/*�����ȶ�������,��303����,����301��������*/
	for(k=0; k<(SAMPLE_NUM + 3); k++)
	{
		pScene->prefile_param_used.pIndex_tri_fan[k] = k;
	}
	/*�ı��ζ�������,��4����,����1���ı���*/
	for(k=0; k<4; k++)
	{
		pScene->prefile_param_used.pIndex_quad[k] = k;
	}

}




/*.BH--------------------------------------------------------
**
** ������: InitTerrainProfile
**
** ����:  ��ʼ����ֱ��������ͼ(VSD-Vertical Situation Display)(����ֻ��Ҫ��ʼ��һ�εĲ���)
**
** �������:  render_scene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�
**
**.EH--------------------------------------------------------
*/
void InitTerrainProfile(VOIDPtr render_scene)
{
    sGLRENDERSCENE *pScene = NULL;

	/*�жϳ�������Ƿ���Ч,��Чֱ�ӷ���*/
	pScene = scenePtrValid(render_scene);
	if(NULL == pScene)
	    return;

	/*��ʼ��VSD���ڻ��ƵĶ�����������*/
	InitTriangleFanAndQuadIndex(pScene);

	//��ʼ�����̵�λͼƬ
	//LoadParticlesTexture(SPANPICPATH, &SpantextureID[0]);
}
 
/*.BH--------------------------------------------------------
**
** ������: RenderTerrainProfile
**
** ����:  ��Ⱦ��ֱ��������ͼVSD
**
** �������:  pScene����Ⱦ�������
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�
**
**.EH--------------------------------------------------------
*/
BOOL RenderTerrainProfile(sGLRENDERSCENE* pScene)
{
	f_int32_t viewport[4];
	f_int32_t i;
	f_char_t chNum[32] = {0};
	f_int16_t left, right, bottom, top;
	f_int16_t width, height;

	f_char_t textInfo[32] = {0};
	f_int32_t textLength = 0;
	sColor4f textColor = {0};
	f_float64_t hIntervalDistance = 0.0;

	/*ÿ���������ڻ�ͨ������setEventParam����VSD��������auxnavinfo_param_input�ṹ����µ�auxnavinfo_param�ṹ��*/
	/*��VSD�����ṹ���л�ȡVSD������ʼλ�á���Ⱥͳ���(��λΪ����)*/
	f_float32_t s_StartWinPosX = pScene->auxnavinfo_param.prefile_param.x;					
	f_float32_t s_StartWinPosY = pScene->auxnavinfo_param.prefile_param.y;
	f_float32_t s_WinWidth = pScene->auxnavinfo_param.prefile_param.width;
	f_float32_t s_WinHeight = pScene->auxnavinfo_param.prefile_param.height;

	/*�жϳ�������Ƿ���Ч,��Чֱ�ӷ���0*/
	if(NULL == pScene)
		return FALSE;	

	/*��ʼ��VSD��ز���,�����ʼ����ʱ���ѳ�ʼ��,ֻ��ʼ��һ��,�˴�����*/
	//InitTerrainProfile(pScene);

	/*����ͼ��λ�����ӿ����λ�ø�Ϊ��Ļ���λ��,��ΪVSD�п��ܲ������ڵ�ͼ�ӿ���*/
#if 1
	
	if(!getInnerViewPort(pScene, viewport))
	    return FALSE;

	//startX = viewPort[0]; startY = viewPort[1]; width = viewPort[2]; height = viewPort[3];
#else
	{
		f_int32_t width_screen = 0, height_screen = 0;
		getScreenWindowSize(&width_screen, &height_screen);

		viewport[0] = 0;
		viewport[1] = 0;
		viewport[2] = width_screen;
		viewport[3] = height_screen;	
	}

#endif

	/*������������(�ǵ�ͼ�ӿ�)�Ŀ��ߡ����½����꼰���Ͻ�����(��λΪ����)*/
	width = viewport[2];
	height = viewport[3];
	left = viewport[0];
	right = viewport[0] + viewport[2];
	bottom = viewport[1];
	top = viewport[1] + viewport[3];
	
	/*���´�ֱ��������ͼ���趯̬����Ĳ���*/
	UpdataTerrainProfile(pScene);

	/*�����ӿڴ�С*/
	glViewport(left, bottom, width, height);
	/*����ͶӰ����(��ͶӰ)��ģʽ�任����*/
	//PushWorldCoord2CliendCoord(left*1.0, width*1.0, bottom*1.0, height*1.0);
	PushWorldCoord2CliendCoord(0.0, width*1.0, bottom*1.0, height*1.0);

	glEnableEx(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisableEx(GL_CULL_FACE);
	glDisableEx(GL_DEPTH_TEST);

	/*ʹ����ͼԪ��ƽ����ɫ,�����߶γ��־��״*/
	glEnableEx(GL_LINE_SMOOTH);

	/*����VSD�Ľ����Ĭ��Ϊ����*/
	glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
#if 0
	/*Ϊ��ʵ�ֿ��λ�úʹ�С��̬�ɱ䣬������ʾ�б�ķ�ʽ���ƣ���֡��Ӱ��ϴ�*/
	glBegin(GL_QUADS);
		glVertex2f(s_StartWinPosX, s_StartWinPosY);
		glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY);
		glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + s_WinHeight);
		glVertex2f(s_StartWinPosX, s_StartWinPosY + s_WinHeight);
	glEnd();
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[0] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[0] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[0], GL_COMPILE);
		{
			glBegin(GL_QUADS);
			glVertex2f(s_StartWinPosX, s_StartWinPosY);
			glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY);
			glVertex2f(s_StartWinPosX + s_WinWidth, s_StartWinPosY + s_WinHeight);
			glVertex2f(s_StartWinPosX, s_StartWinPosY + s_WinHeight);
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[0]);
	}
#endif

	/*����VSD�Ĺ켣��*/
	DrawDynamicHeightMap(pScene);

	/*����VSD�����ܱ߿���*/
	glColor3f(1.0f, 1.0f, 1.0f);
#if 0
	/*�������1������*/
	glBegin(GL_LINE_LOOP);
		glVertex2f(s_StartWinPosX-1, s_StartWinPosY-1);
		glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY -1);
		glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY + s_WinHeight + 1);
		glVertex2f(s_StartWinPosX - 1, s_StartWinPosY + s_WinHeight + 1);
	glEnd();
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[1] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[1] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[1], GL_COMPILE);
		{
			/*�������1������*/
			glBegin(GL_LINE_LOOP);
			glVertex2f(s_StartWinPosX-1, s_StartWinPosY-1);
			glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY -1);
			glVertex2f(s_StartWinPosX + s_WinWidth + 1, s_StartWinPosY + s_WinHeight + 1);
			glVertex2f(s_StartWinPosX - 1, s_StartWinPosY + s_WinHeight + 1);
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[1]);
	}
	
#endif

	/*����VSD��ˮƽ����̶���,�̶���Ϊ����,�߶�Ϊ10������*/
#if 0
	for (i = 1; i < pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum; ++i)
	{
		glBegin(GL_LINES);
		glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY);
		glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY + 10);
		glEnd();
	}
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[2] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[2] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[2], GL_COMPILE);
		{
			for (i = 1; i < pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum; ++i)
			{
				glBegin(GL_LINES);
				glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY);
				glVertex2f((s_StartWinPosX + (i) * pScene->prefile_param_used.s_WinInterval), s_StartWinPosY + 10);
				glEnd();
			}
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[2]);
	}
#endif

	/*����VSD�ķɻ��߶���,ˮƽ���������,����Ϊ30����,���10����,���ڽ�����м�*/
	glLineWidth(1.0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#if 0
	glBegin(GL_LINES);
	for (i = 0; i < s_WinWidth; i += 40)
	{
		glVertex2f(s_StartWinPosX + (i) , 		s_StartWinPosY + s_WinHeight /2);
		glVertex2f(s_StartWinPosX + (i)  + 30, 	s_StartWinPosY + s_WinHeight /2);
	}
	glEnd();
#else
	if(pScene->prefile_param_used.s_TerrainPrefileLISTID[3] == 0)
	{
		pScene->prefile_param_used.s_TerrainPrefileLISTID[3] = glGenLists(1);
		glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[3], GL_COMPILE);
		{
			glBegin(GL_LINES);
			for (i = 0; i < s_WinWidth; i += 40)
			{
				glVertex2i(s_StartWinPosX + (i) , 		s_StartWinPosY + s_WinHeight /2);
				glVertex2i(s_StartWinPosX + (i)  + 30, 	s_StartWinPosY + s_WinHeight /2);
			}
			glEnd();
		}
		glEndList();
	}
	else
	{
		glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[3]);
	}
#endif

	/*�رջ��,�������ƾ���͸��*/
	glDisableEx(GL_BLEND);
	/*����301�������������*/
	glLineWidth(1.0);
	
#if 0	
	/*���ö�������ķ�ʽ,֡��Ӱ��ϴ�*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, pScene->prefile_param_used.s_vDetectHeight_color);

	//�����Ƶ�һ��������һ����
	glVertexPointer(2, GL_DOUBLE, 0, &pScene->prefile_param_used.s_vDetectHeights_cal[2]);
	glDrawElements(GL_LINE_STRIP, (SAMPLE_NUM + 1), GL_UNSIGNED_INT, &pScene->prefile_param_used.pIndex_tri_fan[0]);	

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
#else

#if 1
	{
		/*����ֱ�ӻ��Ƶķ�ʽ,֡��Ӱ��ϴ�*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLfloat * pCol = &pScene->prefile_param_used.s_vDetectHeight_color[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];
		glBegin(GL_LINE_STRIP);
		for (i=0; i<SAMPLE_NUM + 1; ++i)
		{
			glColor4f(pCol[4 * pIndex[i]], pCol[4 * pIndex[i] + 1], pCol[4 * pIndex[i] + 2], pCol[4 * pIndex[i] + 3]);
			glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
		}
		glEnd();
	}
#else

	{
		/*������ʾ�б�Ļ��Ʒ�ʽ�����������ҪƵ��������ʾ�б�ᵼ�����������Ի��ǲ���ֱ�ӻ���*/
		int			i;
		GLdouble	*pVtx = &pScene->prefile_param_used.s_vDetectHeights_cal[0];
		GLfloat     *pCol = &pScene->prefile_param_used.s_vDetectHeight_color[0];
		GLint		*pIndex = &pScene->prefile_param_used.pIndex_tri_fan[0];

		if(pScene->prefile_param_used.s_TerrainPrefileLISTID[4] == 0)
		{
			pScene->prefile_param_used.s_TerrainPrefileLISTID[4] = glGenLists(1);
		}

		if(listIdNeedRegenerate[pScene->scene_index]){
			glNewList(pScene->prefile_param_used.s_TerrainPrefileLISTID[4], GL_COMPILE);
			{
				glBegin(GL_LINE_STRIP);
				for (i=0; i<SAMPLE_NUM + 1; ++i)
				{
					glColor4f(pCol[4 * pIndex[i]], pCol[4 * pIndex[i] + 1], pCol[4 * pIndex[i] + 2], pCol[4 * pIndex[i] + 3]);
					glVertex2d(pVtx[2 * pIndex[i]], pVtx[2 * pIndex[i] + 1]);
				}
				glEnd();
			}
			glEndList();
		}
		else
		{
			glCallList(pScene->prefile_param_used.s_TerrainPrefileLISTID[4]);
		}

	}
	
#endif

#endif

	
	// ����VSD��ֱ������3���߶�ע��,�ֱ��ǣ�����Ϊ�ɻ���ǰ�߶�,����Ϊ�ɻ��߶�-s_winHeiMeter,
	// ����Ϊ�ɻ��߶�+s_winHeiMeter
#if 1
	
	memset(textInfo,0,sizeof(textInfo));

	textColor.red = 1.0; textColor.blue=1.0; textColor.green = 1.0; textColor.alpha = 1.0;
		
	sprintf(textInfo, "%d", (int)pScene->geopt_pos.height);
	textLength = strlen(textInfo);
	textOutPutOnScreen(0, s_StartWinPosX - textLength * 15, s_StartWinPosY + s_WinHeight /2.0 - 7.5, (unsigned char*)textInfo, textColor);		

	sprintf(textInfo, "%d", (int)pScene->geopt_pos.height  - pScene->auxnavinfo_param.prefile_param.s_winHeiMeter);
	textLength = strlen(textInfo);
	textOutPutOnScreen(0, s_StartWinPosX - textLength * 15, s_StartWinPosY, (unsigned char*)textInfo, textColor);		

	sprintf(textInfo, "%d", (int)pScene->geopt_pos.height + pScene->auxnavinfo_param.prefile_param.s_winHeiMeter);
	textLength = strlen(textInfo);	
	textOutPutOnScreen(0, s_StartWinPosX - textLength * 15, s_StartWinPosY + s_WinHeight - 7.5, (unsigned char*)textInfo, textColor);	
	
#else

//	if ((g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei > -1000)&&(g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei < 9000))
	{
		// 1.������������
		f_float32_t low_coord = 0.0, hi_coord = 0.0;

		//��Ϊ����������-1000 ��9000, ��ʾ���̷�Χ��-1000 + �ɻ���+1000 + �ɻ�

		//���ȼ���ɻ����ڵ���������, ���µ���������ֱ�Ӽ�0.1
		
		low_coord = (g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei + 1000)/10000 - 0.1 /*+ 0.013*/;
		hi_coord = (g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei + 1000)/10000 + 0.1 /*+ 0.013*/;

		// 2.���ƴ�����ĳ�����
		glEnableEx(GL_TEXTURE_2D);
		glEnableEx(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, SpantextureID[0]);

		
		glBegin(GL_QUADS);
			glTexCoord2f(0.1,low_coord);		glVertex2i(s_StartWinPosX-35, s_StartWinPosY);
			glTexCoord2f(0.8,low_coord);		glVertex2i(s_StartWinPosX-5 , s_StartWinPosY);
			glTexCoord2f(0.8,hi_coord);			glVertex2i(s_StartWinPosX-5 , s_StartWinPosY + s_WinHeight);
			glTexCoord2f(0.1,hi_coord);			glVertex2i(s_StartWinPosX-35,s_StartWinPosY + s_WinHeight);
		glEnd();


		glDisableEx(GL_BLEND);
		glDisableEx(GL_TEXTURE_2D);


	}
#endif


	/*����VSDˮƽ����̶��ߵ�����ע��,�̶�ֵΪ�̶���λ�õ���ʼλ�õ�ʵ�ʾ���(km),�յ㴦���ƾ��뵥λע��km*/
	for (i = 0; i < pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum; ++i)
	{
		//�������ע������
		memset(textInfo,0,sizeof(textInfo));
		//��������ע����ɫ,ʼ��Ϊ��ɫ
		textColor.red = 1.0; textColor.blue=1.0; textColor.green = 1.0; textColor.alpha = 1.0;
		//����ˮƽ����̶��߶�Ӧ�ľ���
		hIntervalDistance = (pScene->auxnavinfo_param.prefile_param.s_WorldDetectDistance/pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum * i / 1000);
		//������ת��������ע��,����С���ֱ����ִ���
		if( (fabs(hIntervalDistance-(int)hIntervalDistance)) < FLT_EPSILON )
		{
			sprintf(textInfo, "%d", (int)hIntervalDistance);
		}else{
			sprintf(textInfo, "%.1f", hIntervalDistance);
		}
		//��ȡ����ע�ǳ���
		textLength = strlen(textInfo);
		//��������ע��,�������Ͻ���ԭ��
		textOutPutOnScreen(0, 
			(s_StartWinPosX +  i * pScene->prefile_param_used.s_WinInterval - textLength/2.0 * 15.0), 
			(s_StartWinPosY - 20), (unsigned char*)textInfo, textColor);	
		
// 		printf("%d,%d,%d,%s,%f\n",i,(int)(s_StartWinPosX + ( i ) * pScene->prefile_param_used.s_WinInterval - s_WinWidth * 0.02), 
// 			(int)(s_StartWinPosY - s_WinHeight * 0.10),timeInfo,pScene->prefile_param_used.s_WinInterval);
	}	
	//���ƾ��뵥λ������ע��,Ĭ��Ϊkm
	memset(textInfo,0,sizeof(textInfo));
	strcpy(textInfo,"(km)");
	textLength = strlen(textInfo);
	textOutPutOnScreen(0, 
		(s_StartWinPosX + ( pScene->auxnavinfo_param.prefile_param.s_WinIntervalNum ) * pScene->prefile_param_used.s_WinInterval - textLength/2.0 * 15), 
		(s_StartWinPosY - 20), (unsigned char*)textInfo, textColor);		

	//glEnableEx(GL_CULL_FACE);
	//glEnableEx(GL_DEPTH_TEST);	


	/*�ָ�ͶӰ�����ģ�ͱ任����*/
	PopWorldCoord2CliendCoord();

	/*�ָ��ӿھ���*/
	if(!getInnerViewPort(pScene, viewport))
		return FALSE;

	width = viewport[2];
	height = viewport[3];
	left = viewport[0];
	right = viewport[0] + viewport[2];
	bottom = viewport[1];
	top = viewport[1] + viewport[3];
		
	glViewport(left, bottom, width, height);

	return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: GetHandleFont
**
** ����:  ��ȡ��ά������������
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:  �ⲿ�ӿ�
**
**.EH--------------------------------------------------------
*/
TTFONT GetHandleFont()
{
	if(pMapDataSubTree == NULL)
		return NULL;
	else
	{
		sMAPHANDLE * pHandle = (sMAPHANDLE *)pMapDataSubTree;
		return pHandle->ttf_font.font;
	}
		
}
