/********************************************************************
	created:	2015/12/15
	created:	15:12:2015   15:27
	filename: 	Atmosphere.c
	file path:	$地图软件目录$\VxWorks_VecMap\3dMapEngine_prj\src
	file base:	Atmosphere
	file ext:	c
	author:		张钰鹏
	
	purpose:	生成大气(天空)效果
*********************************************************************/
#include <math.h>
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/vectorMethods.h"
#include "../projection/coord.h"
#include "atmosphere.h"
#include "../mapApp/mapApp.h"

#define TEST_GRAY    //使用灰度图效果(三维夜间模式使用)，将RGB转换成灰度图，采用加权平均法，0.2989R+0.5870G+0.1140B
//#undef TEST_GRAY

#define ATMS_TEX    //使用天空纹理，否则直接给圆锥体各个顶点的颜色值
//#undef ATMS_TEX

// 内部接口
// 设置天空颜色的关键点
static void setTexKeyColor(sTexKeyColor *kc, float pp, int R, int G, int B);

static void setGadientTex(GLubyte texData[][4], int texLenth, sTexKeyColor kc[], int keyColorLength);


// 内部接口实现
static void setTexKeyColor(sTexKeyColor *kc, float pp, int R, int G, int B)
{
	kc->posPercent = pp;
	kc->colorR = R;
	kc->colorG = G;
	kc->colorB = B;
}


static void setGadientTex(GLubyte texData[][4], int texLenth, sTexKeyColor kc[], int keyColorLength)
{
	//注意，本函数目前只支持kc的posPercent从0.00~1.00从小到大按顺序排列，并且0.00和1.00处必须有值
	int i, j, k;
	
	//初始化，全部设为(0,0,0)
	for(i = 0; i<texLenth; i++)
	{
		for(j = 0; j<3; j++)
		{
			texData[i][j] = 0;
		}
	}
		
		//kc[0]为posPercent为0的关键色
		texData[0][0] = kc[0].colorR;
		texData[0][1] = kc[0].colorG;
		texData[0][2] = kc[0].colorB;
		
		for (k = 1; k<keyColorLength; k++)
		{
			int i0 = (int)(kc[k - 1].posPercent * (texLenth - 1));	//上一个点的位置
			int p = (int)(kc[k].posPercent * (texLenth - 1));	//本点的位置
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

	//为三维夜间模式设置另外一种配色，偏黑色
	//setTexKeyColor(&keyColor[0], 0.00f, 17, 64, 146);
	//setTexKeyColor(&keyColor[1], 0.80f, 11, 43, 96);
	//setTexKeyColor(&keyColor[2], 1.00f, 0, 0, 0);
	
	setGadientTex(texData, 128, keyColor, 3);
	
	//生成天空的RGB纹理,一维或则二维
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
		//设置天空的RGB纹理数据的Alpha值
		texData[i][3] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		//texData[i][3] = (GLubyte)(255);

		//设置天空的灰度纹理数据的r,g,b,a值
		texDataGray[i][0] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		texDataGray[i][1] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		texDataGray[i][2] = (GLubyte)(0.299 * texData[i][0] + 0.587 * texData[i][1] + 0.114 * texData[i][2]);
		texDataGray[i][3] = texData[i][3];
	}	
	//glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

	//生成天空的灰度纹理,一维或则二维
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
		//设置天空的RGB纹理数据的Alpha值
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

//#define HEIGHT_ATMOSPHERE 20000 //大气层高度(外面就是宇宙的颜色了)

// 配合三维雾化使用的天空，罩在视点上的一个圆锥(远端为视线在地球表面的切点)
void renderAtmosphere(sGLRENDERSCENE * pHandle)
{
	PT_3D sphereDir;//球心到飞机的方向
	f_float64_t hgtAtmos;//天空墙的高度
	PT_3D ptCenterTop;//飞机顶部xyz坐标
	PT_3D planeDir1;//锥台底面圆上的一个方向
	PT_3D farAtmos0, farAtmos1, farAtmos2;//天空的3个远点(第0点用于储存，第1、2点用于循环)
	PT_3D ptTemp;
	f_float64_t angle_alpha = 0.0;//锥台面循环计算点的时候用到的角
	Geo_Pt_D geoptTemp;
	
	double disFarPorj;//天空远点在飞机-地心连线上的投影点到球心距离
	PT_3D pointFarPorj;//远点在飞机-地心连线上的投影点
	double disFarProj2;//天空远点到飞机-地心连线上的投影点到球心距离	
	
	if (pHandle == NULL)
	{
		return ;
	}
	
	//初始化天空盒纹理
#ifdef ATMS_TEX
	if( 0 == pHandle->atms_text)
	{
		initAtmosphereTex(pHandle);
		printf("atms_text=%d\n",pHandle->atms_text);
	}
#endif	
    glDepthMask(GL_FALSE); //防止天空盒将注记挡住
	
	// 1.准备工作-计算远点在飞机与地心连线的投影点(该点作为天空远端(底部)的生发点)
	vector3DSetValue(&sphereDir, &(pHandle->camParam.m_ptEye));
	vector3DNormalize(&sphereDir);
	objDPt2geoDPt(&(pHandle->camParam.m_ptEye), &geoptTemp);
	disFarPorj = EARTH_RADIUS * EARTH_RADIUS / (EARTH_RADIUS + geoptTemp.height);
	vector3DMul(&pointFarPorj, &sphereDir, disFarPorj);
	hgtAtmos = (float)geoptTemp.height + 20000.0;//geoptTemp.height > 10000 ? geoptTemp.height : 10000;
	geoptTemp.height = hgtAtmos;
	geoDPt2objDPt(&geoptTemp, &ptCenterTop);

	// 2.计算天空底部一点
	disFarProj2 = sqrt(EARTH_RADIUS * EARTH_RADIUS - disFarPorj * disFarPorj);
	// 寻找一个垂直的方向
	vector3DMemSet(&ptTemp, 0, 1, 0);
	vector3DCrossProduct(&planeDir1,	&sphereDir, &ptTemp);
	// 如果(0,1,0)方向选择与球心到飞机的方向太过于靠近，则叉乘结果可能为0，此时就换一个相反的方向进行叉乘
	if(vector3DLength(&planeDir1) < 0.0001)
	{
		vector3DMemSet(&ptTemp, 1, 0, 0);
		vector3DCrossProduct(&planeDir1,	&sphereDir, &ptTemp);
	}
	vector3DNormalize(&planeDir1);
	vector3DMul(&farAtmos1, &planeDir1, disFarProj2);
	vector3DAdd(&farAtmos1, &farAtmos1, &pointFarPorj);
	vector3DSetValue(&farAtmos0, &farAtmos1);//起始点储存下来
	
	// 天空顶部点为视点向上一定距离的点
	// 3.接下来准备绘制
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
	default:                            // RGB显示模式
		break;		
	case eCOR_gray:	// 灰度图显示模式
		break;		
	case eCOR_green: // 绿色显示模式
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
				// 第一个天空远(低)点
#ifdef ATMS_TEX
				//glTexCoord1f(0.0f);
				glTexCoord2f(0.0f,1.0f);
#else
				glColor4f( 255/255.0f, 255/255.0f, 255/255.0f,0.80f);
#endif
				glVertex3d(farAtmos1.x, farAtmos1.y, farAtmos1.z);

			// 绕中心线旋转0.1弧度得到下一个天空近点
			farAtmos2 = vector3DRotate(&farAtmos1, &sphereDir, -0.1);
			vector3DSetValue(&farAtmos1, &farAtmos2);
			angle_alpha -= 0.1;
		}
		// 和起始点连起来
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
