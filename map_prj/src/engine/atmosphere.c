/********************************************************************
	created:	2015/12/15
	created:	15:12:2015   15:27
	filename: 	Atmosphere.c
	file path:	$��ͼ���Ŀ¼$\VxWorks_VecMap\3dMapEngine_prj\src
	file base:	Atmosphere
	file ext:	c
	author:		������
	
	purpose:	���ɴ���(���)Ч��
*********************************************************************/
#include <math.h>
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/vectorMethods.h"
#include "../projection/coord.h"
#include "atmosphere.h"
#include "../mapApp/mapApp.h"

#define TEST_GRAY    //ʹ�ûҶ�ͼЧ��(��άҹ��ģʽʹ��)����RGBת���ɻҶ�ͼ�����ü�Ȩƽ������0.2989R+0.5870G+0.1140B
//#undef TEST_GRAY

#define ATMS_TEX    //ʹ�������������ֱ�Ӹ�Բ׶������������ɫֵ
//#undef ATMS_TEX

// �ڲ��ӿ�
// ���������ɫ�Ĺؼ���
static void setTexKeyColor(sTexKeyColor *kc, float pp, int R, int G, int B);

static void setGadientTex(GLubyte texData[][4], int texLenth, sTexKeyColor kc[], int keyColorLength);


// �ڲ��ӿ�ʵ��
static void setTexKeyColor(sTexKeyColor *kc, float pp, int R, int G, int B)
{
	kc->posPercent = pp;
	kc->colorR = R;
	kc->colorG = G;
	kc->colorB = B;
}


static void setGadientTex(GLubyte texData[][4], int texLenth, sTexKeyColor kc[], int keyColorLength)
{
	//ע�⣬������Ŀǰֻ֧��kc��posPercent��0.00~1.00��С����˳�����У�����0.00��1.00��������ֵ
	int i, j, k;
	
	//��ʼ����ȫ����Ϊ(0,0,0)
	for(i = 0; i<texLenth; i++)
	{
		for(j = 0; j<3; j++)
		{
			texData[i][j] = 0;
		}
	}
		
		//kc[0]ΪposPercentΪ0�Ĺؼ�ɫ
		texData[0][0] = kc[0].colorR;
		texData[0][1] = kc[0].colorG;
		texData[0][2] = kc[0].colorB;
		
		for (k = 1; k<keyColorLength; k++)
		{
			int i0 = (int)(kc[k - 1].posPercent * (texLenth - 1));	//��һ�����λ��
			int p = (int)(kc[k].posPercent * (texLenth - 1));	//�����λ��
			for (i = i0+1; i<p; i++)
			{
				texData[i][0] = (kc[k].colorR - kc[k-1].colorR) * (i-i0) / (p-i0) + kc[k-1].colorR;
				texData[i][1] = (kc[k].colorG - kc[k-1].colorG) * (i-i0) / (p-i0) + kc[k-1].colorG;
				texData[i][2] = (kc[k].colorB - kc[k-1].colorB) * (i-i0) / (p-i0) + kc[k-1].colorB;
			}
			texData[p][0] = kc[k].colorR;
			texData[p][1] = kc[k].colorG;
			texData[p][2] = kc[k].colorB;
		}
}

void initAtmosphereTex(VOIDPtr map_andle)
{
#if 1
	int i = 0;
#ifdef TEST_GRAY
	GLubyte texData[128][4];
	GLubyte texDataGray[128][4];
#else
	GLubyte texData[128][4];
#endif        
	sTexKeyColor keyColor[3];

	sGLRENDERSCENE *pScene = NULL;
	pScene = scenePtrValid(map_andle);
	if(NULL == pScene)
	{
	    return;
	}

	setTexKeyColor(&keyColor[0], 0.00f, 255, 255, 255);
	setTexKeyColor(&keyColor[1], 0.80f, 33, 110, 191);
	setTexKeyColor(&keyColor[2], 1.00f, 17, 64, 146);

	//setTexKeyColor(&keyColor[0], 0.00f, 255, 255, 255);
	//setTexKeyColor(&keyColor[1], 0.80f, 116, 115, 255);
	//setTexKeyColor(&keyColor[2], 1.00f, 0, 0, 255);

	//Ϊ��άҹ��ģʽ��������һ����ɫ��ƫ��ɫ
	//setTexKeyColor(&keyColor[0], 0.00f, 17, 64, 146);
	//setTexKeyColor(&keyColor[1], 0.80f, 11, 43, 96);
	//setTexKeyColor(&keyColor[2], 1.00f, 0, 0, 0);
	
	setGadientTex(texData, 128, keyColor, 3);
	
	//������յ�RGB����,һά�����ά
	//glEnableEx(GL_TEXTURE_1D);
	glEnableEx(GL_TEXTURE_2D);
	glDisableEx(GL_DITHER);
	
	glGenTextures(1, &(pScene->atms_text));
	//glBindTexture(GL_TEXTURE_1D, pHandle->atms_text);	
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, pScene->atms_text);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
#ifdef TEST_GRAY	
	for (i = 0; i<128; i++)
	{
		//������յ�RGB�������ݵ�Alphaֵ
		texData[i][3] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		//texData[i][3] = (GLubyte)(255);

		//������յĻҶ��������ݵ�r,g,b,aֵ
		texDataGray[i][0] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		texDataGray[i][1] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		texDataGray[i][2] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		texDataGray[i][3] = texData[i][3];
	}	
	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

	//������յĻҶ�����,һά�����ά
	glGenTextures(1, &(pScene->atms_textgray));

	//glBindTexture(GL_TEXTURE_1D, pHandle->atms_textgray);
	glBindTexture(GL_TEXTURE_2D, pScene->atms_textgray);
	
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, texDataGray);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texDataGray);
#else

	for (i = 0; i<128; i++)
	{
		//������յ�RGB�������ݵ�Alphaֵ
		texData[i][3] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		//texData[i][3] = (GLubyte)(255);
	}

	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	
#endif
	//glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);
#endif

}

//#define HEIGHT_ATMOSPHERE 20000 //������߶�(��������������ɫ��)

// �����ά��ʹ�õ���գ������ӵ��ϵ�һ��Բ׶(Զ��Ϊ�����ڵ��������е�)
void renderAtmosphere(sGLRENDERSCENE * pHandle)
{
	PT_3D sphereDir;//���ĵ��ɻ��ķ���
	f_float64_t hgtAtmos;//���ǽ�ĸ߶�
	PT_3D ptCenterTop;//�ɻ�����xyz����
	PT_3D planeDir1;//׶̨����Բ�ϵ�һ������
	PT_3D farAtmos0, farAtmos1, farAtmos2;//��յ�3��Զ��(��0�����ڴ��棬��1��2������ѭ��)
	PT_3D ptTemp;
	f_float64_t angle_alpha = 0.0;//׶̨��ѭ��������ʱ���õ��Ľ�
	Geo_Pt_D geoptTemp;
	
	double disFarPorj;//���Զ���ڷɻ�-���������ϵ�ͶӰ�㵽���ľ���
	PT_3D pointFarPorj;//Զ���ڷɻ�-���������ϵ�ͶӰ��
	double disFarProj2;//���Զ�㵽�ɻ�-���������ϵ�ͶӰ�㵽���ľ���	
	
	if (pHandle == NULL)
	{
		return ;
	}
	
	//��ʼ����պ�����
#ifdef ATMS_TEX
	if( 0 == pHandle->atms_text)
	{
		initAtmosphereTex(pHandle);
		printf("atms_text=%d\n",pHandle->atms_text);
	}
#endif	
    glDepthMask(GL_FALSE); //��ֹ��պн�ע�ǵ�ס
	
	// 1.׼������-����Զ���ڷɻ���������ߵ�ͶӰ��(�õ���Ϊ���Զ��(�ײ�)��������)
	vector3DSetValue(&sphereDir, &(pHandle->camParam.m_ptEye));
	vector3DNormalize(&sphereDir);
	objDPt2geoDPt(&(pHandle->camParam.m_ptEye), &geoptTemp);
	disFarPorj = EARTH_RADIUS * EARTH_RADIUS / (EARTH_RADIUS + geoptTemp.height);
	vector3DMul(&pointFarPorj, &sphereDir, disFarPorj);
	hgtAtmos = (float)geoptTemp.height + 20000.0;//geoptTemp.height > 10000 ? geoptTemp.height : 10000;
	geoptTemp.height = hgtAtmos;
	geoDPt2objDPt(&geoptTemp, &ptCenterTop);

	// 2.������յײ�һ��
	disFarProj2 = sqrt(EARTH_RADIUS * EARTH_RADIUS - disFarPorj * disFarPorj);
	// Ѱ��һ����ֱ�ķ���
	vector3DMemSet(&ptTemp, 0, 1, 0);
	vector3DCrossProduct(&planeDir1,	&sphereDir, &ptTemp);
	// ���(0,1,0)����ѡ�������ĵ��ɻ��ķ���̫���ڿ��������˽������Ϊ0����ʱ�ͻ�һ���෴�ķ�����в��
	if(vector3DLength(&planeDir1) < 0.0001)
	{
		vector3DMemSet(&ptTemp, 1, 0, 0);
		vector3DCrossProduct(&planeDir1,	&sphereDir, &ptTemp);
	}
	vector3DNormalize(&planeDir1);
	vector3DMul(&farAtmos1, &planeDir1, disFarProj2);
	vector3DAdd(&farAtmos1, &farAtmos1, &pointFarPorj);
	vector3DSetValue(&farAtmos0, &farAtmos1);//��ʼ�㴢������
	
	// ��ն�����Ϊ�ӵ�����һ������ĵ�
	// 3.������׼������
#ifdef ATMS_TEX

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	glActiveTextureARB(GL_TEXTURE0);
#endif

#ifdef TEST_GRAY
	if(eDN_nighttime == pHandle->mdctrl_cmd.dn_mode){
		//glEnableEx(GL_TEXTURE_1D);
		//glBindTexture(GL_TEXTURE_1D, pHandle->atms_textgray);
		glEnableEx(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pHandle->atms_textgray);
	}else{
		//glEnableEx(GL_TEXTURE_1D);
		//glBindTexture(GL_TEXTURE_1D, pHandle->atms_text);
		glEnableEx(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, pHandle->atms_text);
	}
#else
	//glEnableEx(GL_TEXTURE_1D);
	//glBindTexture(GL_TEXTURE_1D, pHandle->atms_text);
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pHandle->atms_text);
#endif

#else

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	glColorArrayEnable(1);
#endif

#endif

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	switch(pHandle->mdctrl_cmd.cor_mode)
	{
	default:                            // RGB��ʾģʽ
		break;		
	case eCOR_gray:	// �Ҷ�ͼ��ʾģʽ
		break;		
	case eCOR_green: // ��ɫ��ʾģʽ
		break;			
	}

	glBegin(GL_TRIANGLE_FAN);
#ifdef ATMS_TEX
		//glTexCoord1f(1.0f);
		glTexCoord2f(1.0f,1.0f);
#else
		glColor4f( 33/255.0f, 110/255.0f, 191/255.0f,0.80f);
#endif
		glVertex3d(ptCenterTop.x, ptCenterTop.y, ptCenterTop.z);
		//20181122 modified by fw for third-party test
		//for (angle_alpha = 2.0f*PI;angle_alpha>=0.0f;angle_alpha -= 0.1)
		angle_alpha = 2.0*PI;
		while( angle_alpha > 0.0 )
		{
				// ��һ�����Զ(��)��
#ifdef ATMS_TEX
				//glTexCoord1f(0.0f);
				glTexCoord2f(0.0f,1.0f);
#else
				glColor4f( 255/255.0f, 255/255.0f, 255/255.0f,0.80f);
#endif
				glVertex3d(farAtmos1.x, farAtmos1.y, farAtmos1.z);

			// ����������ת0.1���ȵõ���һ����ս���
			farAtmos2 = vector3DRotate(&farAtmos1, &sphereDir, -0.1);
			vector3DSetValue(&farAtmos1, &farAtmos2);
			angle_alpha -= 0.1;
		}
		// ����ʼ��������
#ifdef ATMS_TEX
		//glTexCoord1f(0.0f);
		glTexCoord2f(0.0f,1.0f);
#else
		glColor4f( 255/255.0f, 255/255.0f, 255/255.0f,0.80f);
#endif
		glVertex3d(farAtmos0.x, farAtmos0.y, farAtmos0.z);
	glEnd();
#ifdef ATMS_TEX
	//glDisableEx(GL_TEXTURE_1D);
	glDisableEx(GL_TEXTURE_2D);
#else

#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	glColorArrayEnable(0);
#endif

#endif
	
	glDepthMask(GL_TRUE);	
}
