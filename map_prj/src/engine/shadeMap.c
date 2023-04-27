
#include <math.h>

#include "../define/macrodefine.h"
#include "terWarning.h"
#include "../mapApp/mapApp.h"
#include "memoryPool.h"
#include "../define/macrodefine.h"

#include "../projection/coord.h"
#include "../../../vecMapEngine_prj/src/mathematical/Mathematical.h"
#include "imageBmp.h"

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32	
extern PFNGLACTIVETEXTUREPROC glActiveTexture;	
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;	
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f;
#endif                                     /* Ӱ���������������� */
#endif

//��Ӱ������صĽṹ��----------------
typedef struct tagsRGBFLOATCOLOR
{
	float r_step;
	float g_step;
	float b_step;
}sRGBFLOATCOLOR;

typedef struct tagsRGBCOLOR
{
	int r;
	int g;
	int b;
}sRGBCOLOR;

typedef struct tagsPARAMS_FOR_OUTPUT
{
	int height;
	int shade;
}sPARAMS_FOR_OUTPUT;


#define ai 0.5
#define ka 0.7
#define li 0.8
#define kd 0.8
//ɫ����
#define stripeDemImageWidth 	8192	//5500	//1000    //8192	modify to 1000
#define stripeVecImageWidth 	4096    //256->4096������߶�ֵ������ϸ�����ºܶ�ط�������һ����ɫ����½�ֽ粻����;JM7200���������ߴ�Ϊ4096*4096
#define ColorNumber				23

static unsigned char stripeDemImage[stripeDemImageWidth*4];               //SVS��ͼɫ��
static unsigned char stripeVecDemImage[stripeVecImageWidth*4];            //VecShade��ͼɫ��

static int TagSum = 0;
static int heiMax = 0;

static f_uint32_t shadeMapTexture[2] = {0};	// 0:��������VecShadeģʽ��ͼ��һά���ά����ID��1:��γ���Ķ�ά����ID

extern BOOL objDPt2geoDPt(const LP_Obj_Pt_D objPt, LP_Geo_Pt_D geoPt);

void makeStripeDemImage(unsigned char  stripedemimage[]);

f_uint32_t  InitGridTexture(f_uint32_t width);


/*.BH--------------------------------------------------------
**
** ������: InitVecShadedReliefMap
**
** ����:  ����2��������������SVSģʽ��ͼ��һά���ά����ID����γ���Ķ�ά����ID
**
** �������:  ��
**
** �����������
**
** ����ֵ����;
**          
**
** ���ע��: 
**
**.EH--------------------------------------------------------
*/
void InitVecShadedReliefMap()
{
	f_uint16_t width, height;
	f_uint32_t size;
	f_uint8_t style; 
	f_uint8_t* pImageData = NULL;
	f_uint32_t textureid = 0;

	/*��vecShade.bmp�ж�ȡ��ɫ����*/
	pImageData = ReadBMPEX(VECSHADEPICPATH, &width, &height, &size, &style);
	if(pImageData == NULL)
	{
		printf("%s:����λͼʧ��!\n", VECSHADEPICPATH);
		return ;
	}

	{
	#define color_num 32

		int i = 0,j = 0;
		int texX[color_num] = {0};
		sRGBFLOATCOLOR colorStep[color_num];
		float level[color_num] = {0};
		sRGBCOLOR color[color_num] ={ 0};

		/* �Ӷ�ȡ����ɫ������ѡ��32����ɫ
		* 124 148 212
		191 221 195
		211 232 213
		225 239 226
		239 244 240
		241 244 237
		242 243 235
		245 242 233
		246 242 231
		247 243 234
		248 243 237
		249 244 238
		252 245 239
		251 242 235
		252 239 231
		250 237 228
		249 235 226
		250 232 222
		250 230 219
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		250 228 215
		* */
		for(i = 0; i < color_num; i++)
		{
			int index = style *( i*1024/color_num );
		
			color[i].r = pImageData[index ];
			color[i].g = pImageData[index + 1];
			color[i].b = pImageData[index + 2];

		}
		/*������Ϊ4096�����ֳ�32�Σ�ÿ�γ���Ϊ128*/
		for(i=0; i < color_num; i++)
		{
			level[i] = (float)(i) / color_num;
		}
		for(i = 0; i < color_num; i++)
			texX[i] = level[i] * stripeVecImageWidth;  

		/*������ɫ����ϵ����Ŀ������32����ɫ֮����ɽ��䣬ʵ�ʲ�δʹ��*/
		for(i = 0; i < color_num; i++){
			if(0 == i){
				colorStep[i].r_step = 0.0f;
				colorStep[i].g_step = 0.0f;
				colorStep[i].b_step = 0.0f;
			}
			else if(i < color_num - 1){
				colorStep[i].r_step = (color[i + 1].r - color[i].r) * 1.0f / texX[i];
				colorStep[i].g_step = (color[i + 1].g - color[i].g) * 1.0f / texX[i];
				colorStep[i].b_step = (color[i + 1].b - color[i].b) * 1.0f / texX[i];
			}
			else{
				colorStep[i].r_step = 0.0f;
				colorStep[i].g_step = 0.0f;
				colorStep[i].b_step = 0.0f;
			}
		}
		/*�����������ݣ��ܳ���Ϊ4096�����ֳ�32�Σ�ÿ�γ���Ϊ128��ÿ�ε���ɫ��Ӧ֮ǰ��32����ɫ��͸����ʼ��Ϊ255������͸��*/
		for (i = 0; i < stripeVecImageWidth ; i++){
			for(j = 0; j < color_num; j++){
				if(i <= texX[j]){
					stripeVecDemImage[4*i]   = (unsigned char)(color[j].r /*+ colorStep[j].r_step * (i-texX[j])*/);
					stripeVecDemImage[4*i+1] = (unsigned char)(color[j].g /*+ colorStep[j].g_step * (i-texX[j])*/);
					stripeVecDemImage[4*i+2] = (unsigned char)(color[j].b /*+ colorStep[j].b_step * (i-texX[j])*/);
					stripeVecDemImage[4*i+3] = 255;
					break;
				}
			}
		}
	}

	//OpenGL ES��֧��GL_TEXTURE_1D���޸�ΪGL_TEXTURE_2D
#if 0
	glDisableEx(GL_TEXTURE_1D);
	glGenTextures(1, &shadeMapTexture[0]);
	glBindTexture(GL_TEXTURE_1D, shadeMapTexture[0]);
	//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeVecImageWidth,  0, GL_RGBA, GL_UNSIGNED_BYTE, &stripeVecDemImage[0]);

	glBindTexture(GL_TEXTURE_1D,0);

#else

	glDisableEx(GL_TEXTURE_2D);
	glGenTextures(1, &shadeMapTexture[0]);
	glBindTexture(GL_TEXTURE_2D, shadeMapTexture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, stripeVecImageWidth, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &stripeVecDemImage[0]);
	
	glBindTexture(GL_TEXTURE_2D, 0);
#endif

	free(pImageData);

	//���ɾ�γ��������ID
	shadeMapTexture[1] = InitGridTexture(TILESIZE);
}

/*.BH--------------------------------------------------------
**
** ������: RenderShadeMapPre
**
** ����:  ����VecShadeģʽǰ����OpenGL����״̬���������
**
** �������:  ��
**
** �����������
**
** ����ֵ���ɹ�������TRUE;
**		   ʧ�ܣ�����FALSE;
**          
**
** ���ע��: ��
**
**.EH--------------------------------------------------------
*/
BOOL RenderShadeMapPre(sGLRENDERSCENE *pHandle)
{

	float raio = 1.0f / (8192.0f);  // 1.0/8192.0;

	if(NULL == pHandle)
	    return FALSE;

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	glEnableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, shadeMapTexture[0]);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMatrixMode(GL_TEXTURE);								// ����������ţ�
	glLoadIdentity();
//	glScalef(0.0002f,0.0002f,0.0002f);						// ��������ͼ����ϵ������������͵����ߵ����õ�
	glScalef(raio, 1.0, 1.0);						// ��������ͼ����ϵ������������͵����ߵ����õ�

	glMatrixMode(GL_MODELVIEW);
#else
	{
		ESMatrix mat;
		int prog = 0;
		int loc = 0;

		glActiveTextureARB(GL_TEXTURE0);
		glEnableEx(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, shadeMapTexture[0]);

		esMatrixLoadIdentity(&mat);
		esScale(&mat, raio, 1.0, 1.0);

		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		loc = glGetUniformLocation(prog, "u_tMatrix");

		glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&mat);
	}
#endif
	
	return TRUE;	
}


/*.BH--------------------------------------------------------
**
** ������: RenderShadeMapPro
**
** ����:  ����VecShadeģʽǰ�ָ�OpenGL����״̬���������
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:��
**
**.EH--------------------------------------------------------
*/
void RenderShadeMapPro()
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	//glActiveTextureARB(GL_TEXTURE1_ARB);					// ��������Ԫ1
	glActiveTexture(GL_TEXTURE0);					        // ��������Ԫ0
	glMatrixMode(GL_TEXTURE);								// תΪ����������
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// �ָ������ջΪģ����ͼ����

	glDisable(GL_TEXTURE_1D);

	//glActiveTextureARB(GL_TEXTURE0_ARB);					// ��������Ԫ0
	glActiveTexture(GL_TEXTURE0);					// ��������Ԫ0

#else
	glActiveTextureARB(GL_TEXTURE0);					    // ��������Ԫ0
	glMatrixMode(GL_TEXTURE);								// תΪ����������
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// �ָ������ջΪģ����ͼ����

	glDisableEx(GL_TEXTURE_2D);

#endif
}


/*.BH--------------------------------------------------------
**
** ������: RenderShadeMap
**
** ����:  ����VecShadeģʽ
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע��:��
**
**.EH--------------------------------------------------------
*/
void RenderShadeMap(sGLRENDERSCENE *pHandle, sQTMAPNODE *pNode)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)

	if(NULL == pHandle || NULL == pNode || NULL == pNode->fterrain)
	    return;

#if 0
#ifdef _JM7200_
	/* JM7200��glTexCoordPointer��֧��GL_SHORT���޸�ΪGL_FLOAT*/
	{    
		int i = 0;
		float tt[vertexNum] = {0};
		for (i = 0; i< vertexNum; i++)
		{
			tt[i] = pNode->fterrain[i] * 1.0f;
		}
		glClientActiveTexture(GL_TEXTURE0);                
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_FLOAT, 0, tt);	
	}
#else
	{              
		glClientActiveTexture(GL_TEXTURE0);                
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(1, GL_SHORT, 0, pNode->fterrain);
	}
#endif
#endif

	/* ��������Ӱ��֡�ʣ���fterrain�ڶ�ȡ��ʱ�����short����ת����float���� */              
	glClientActiveTexture(GL_TEXTURE0);                
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(1, GL_FLOAT, 0, pNode->fterrain);
	
#else
	/* ʵ����mapRenderVecImgEs������ֱ�ӻ��� */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(1, GL_FLOAT, 0, pNode->fterrain);	

#endif

}

/*.BH--------------------------------------------------------
**
** ������: GenerateTexture
**
** ����:   ���ɷ�������MipMap����
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
void GenerateTexture(f_uint32_t* texture, f_uint32_t width, f_uint8_t* texBuf)
{
	glGenTextures(1, texture);
	if (*texture == 0)
	{
		printf("!!!����-�����������ʧ��\n");
		// ����-�����������ʧ��
		return ;
	}
	glDisableEx(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, *texture);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32
	{
		float paraTexMaxAnis;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &paraTexMaxAnis);
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, paraTexMaxAnis);	
	}
#endif
#endif
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	//glTexImage2D( GL_TEXTURE_2D, 0, 4, g_SphereDataEx.m_nDOMWidth*2, g_SphereDataEx.m_nDOMWidth*2,0,GL_RGBA,GL_UNSIGNED_BYTE, pRT->m_lpBuf);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, width, GL_RGBA, GL_UNSIGNED_BYTE, texBuf);
#else
	gluBuild2DMipmapsExt(GL_TEXTURE_2D, GL_RGBA, width, width, GL_RGBA, GL_UNSIGNED_BYTE, texBuf);
#endif

	glBindTexture(GL_TEXTURE_2D,0);
}

/*.BH--------------------------------------------------------
**
** ������: InitGridTexture
**
** ����:   ���ɷ��������������ݲ����ɸ����������ߵľ����ǹ̶��ģ����ǵ�16�㼶��Ƭ�ı߳�����ʾ��ʵ�ʾ���
**
** �������:  width -- �����С��width*width��
**
** �����������
**
** ����ֵ������ID
**          
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
f_uint32_t InitGridTexture(f_uint32_t width)
{
	f_uint8_t* texBuf = NULL;
	f_uint32_t i = 0, j = 0;
	f_uint32_t texture_id = 0;
	texBuf = (f_uint8_t*)malloc(sizeof(f_uint8_t) * sqr(width) * 4);
	//  1.���ɷ��������������(�����½ǿ�ʼ���)����СΪ256*256��������һ��ʮ�֣���ɫ���Ǻ�ɫ�������εĵط���͸���������ط�͸��
	for (i = 0; i < width; ++i)
	{
		for (j = 0; j < width; ++j)
		{
			f_uint32_t index = (i * width + j) * 4;
			if( ((i%(width/2) == 0) || (j%(width/2) == 0)) && (i != 0)&&(j != 0) && (i != width)&&(j != width) )
			{
				texBuf[index] = 0;
				texBuf[index + 1] = 0;
				texBuf[index + 2] = 0;
				texBuf[index + 3] = 255;
			}
			else
			{
				texBuf[index] = 0;
				texBuf[index + 1] = 0;
				texBuf[index + 2] = 0;
				texBuf[index + 3] = 0;
			}
		}
 	}

	// 2.��������ID
	GenerateTexture(&texture_id, width, texBuf);

	free(texBuf);

	return( texture_id);
}

/*.BH--------------------------------------------------------
**
** ������: GetGridTextureID
**
** ����:   �󶨷�����������׼��ʹ�ø�����
**
** �������:  ��
**
** �����������
**
** ����ֵ����
**          
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
void GetGridTextureID()
{
	glBindTexture(GL_TEXTURE_2D, shadeMapTexture[1]);	
}


f_float32_t FastInvSqrt(f_float32_t x)
{
	float xhalf = 0.5f * x;
	int i = *(int *)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x*(1.5f - (xhalf*x*x));

	return x;
}

void GetNormal(const f_float32_t x0, const f_float32_t y0, const f_float32_t z0, const f_float32_t x1, const f_float32_t y1, const f_float32_t z1, f_char_t* pNormal) {
	f_float32_t x, y, z, w;
	x = y0 * z1 - z0 * y1;
	y = -x0 * z1 + z0 * x1;
	z = x0 * y1 - y0 * x1;

	w = (x * x + y * y + z * z);
	w = FastInvSqrt(w);
	w = 127 * w;

	pNormal[0] = x * w;
	pNormal[1] = y * w;
	pNormal[2] = z * w;
}

/*.BH--------------------------------------------------------
**
** ������: GenerateTileNormal
**
** ����:   ������Ƭ��������ķ����������տ���ʱʹ��
**
** �������:  mapNode -- ��Ƭ�ڵ���
**
** �����������
**
** ����ֵ���ɹ�������TRUE;
**		   ʧ�ܣ�����FALSE;
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
BOOL GenerateTileNormal(sQTMAPNODE* mapNode) 
{
	f_uint32_t i, j, k;
	LP_PT_3F p00 = NULL, p01 = NULL, p10 = NULL, p11 = NULL;
	LP_PT_3F pPt = NULL;
	f_char_t* pNormal = NULL;
	f_int32_t nLineWidth;

	if (mapNode == NULL) {
		return FALSE;
	}

	if (mapNode->pNormal == NULL)
	{
		mapNode->pNormal = NewAlterableMemory(vertexNum * 3 * sizeof(f_char_t));
		if (mapNode->pNormal == NULL)
			return - 1;
	}

	nLineWidth = VTXCOUNT + 1;
	pNormal = mapNode->pNormal;
	pPt = (LP_PT_3F)mapNode->fvertex;

#if 1
	for (i = 0, k = 0; i <= VTXCOUNT; ++i) {
		for (j = 0; j <= VTXCOUNT; ++j, ++pPt, ++k) {
			if (0 == j) {
				p00 = pPt;
				p01 = pPt + 1;
			}
			else if (VTXCOUNT == j) {
				p00 = pPt - 1;
				p01 = pPt;
			}
			else {
				p00 = pPt - 1;
				p01 = pPt + 1;
			}

			if (0 == i) {
				p10 = pPt;
				p11 = pPt + nLineWidth;
			}
			else if (VTXCOUNT == i) {
				p10 = pPt - nLineWidth;
				p11 = pPt;
			}
			else {
				p10 = pPt - nLineWidth;
				p11 = pPt + nLineWidth;
			}
			GetNormal(p01->x - p00->x, p01->y - p00->y, p01->z - p00->z,
				p11->x - p10->x, p11->y - p10->y, p11->z - p10->z,
				pNormal);
			//if (mapNode->nodeRange.z > 0) {
			//	pNormal[0] *= -1;
			//	pNormal[1] *= -1;
			//	pNormal[2] *= -1;
			//}
			pNormal += 3;
		}
	}
#else
	for (i = 0, k = 0; i <= VTXCOUNT; ++i) {
		for (j = 0; j <= VTXCOUNT; ++j, ++pPt, ++k) {
			pNormal[0] = pPt->x / 6378137.0 * 127;
			pNormal[1] = pPt->y / 6378137.0 * 127;
			pNormal[2] = pPt->z / 6378137.0 * 127;

			pNormal += 3;
		}
	}
#endif 
	for (i = 0; i < 4; ++i) {
		for (j = 0; j <= VTXCOUNT; ++j) {
			switch (i) {
			case 0:
				memcpy(pNormal, mapNode->pNormal + j * 3, 3);
				break;
			case 1:
				memcpy(pNormal, mapNode->pNormal + (j + VTXCOUNT * (VTXCOUNT + 1)) * 3, 3);
				break;
			case 2:
				memcpy(pNormal, mapNode->pNormal + j * (VTXCOUNT + 1) * 3, 3);
				break;
			case 3:
				memcpy(pNormal, mapNode->pNormal + (j * (VTXCOUNT + 1) + VTXCOUNT) * 3, 3);
				break;
			}
			pNormal += 3;
		}
	}
	return TRUE;
}



#define MAX_TILE_LEVEL 16
/*.BH--------------------------------------------------------
**
** ������: GenerateTexCoord
**
** ����:   ������Ƭ��������ķ��������������ȵ���GenerateNodePVertexTex���붥�����������ڴ�
**
** �������:  pNode -- ��Ƭ�ڵ���
**
** �����������
**
** ����ֵ���ɹ�������TRUE;
**		   ʧ�ܣ�����FALSE;
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
BOOL GenerateTexCoord(sQTMAPNODE  *pNode)
{
	f_uint32_t i,j;
	f_float32_t* pTex = NULL;
	f_uint8_t level = pNode->level;
	/*Ĭ��16����ʾһ����������������ģʽΪ�ظ���ͨ�������������꣬ʹ15����ʾ4��������������������*/
	f_float32_t dx = (float)(1.0 / VTXCOUNT * pow(2.0, MAX_TILE_LEVEL - level));
//	f_float32_t dx = (float)(1.0 / VTXCOUNT );

	// create tile index buffer
	// texture 
	pTex = pNode->m_pVertexTex;
	/*��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ*/
	for(i = 0; i <= VTXCOUNT; i++)
	{
		for(j = 0; j <= VTXCOUNT; j++)
		{
			*pTex++ = (j*dx);
			*pTex++ = 1.0-(i*dx);
		}
	}
	/* ������Ƭ������ȹ�ߵĶ���Ķ�ά��������,����ȹ�߶�������˳���Ǵ���->��->��->�� */
	for(i = 0; i < 4; i ++)
	{
		for(j = 0; j <= VTXCOUNT; j++)
		{
			switch(i)
			{
			case 0:
				*pTex++ = (j * dx);
				*pTex++ = 1.0-dx;
				break;

			case 1:
				*pTex++ = (j * dx);
				*pTex++ = dx;
				break;

			case 2:
				*pTex++ = dx;
				*pTex++ = 1.0-(j * dx);
				break;

			case 3:
				*pTex++ = (1 - dx);
				*pTex++ = 1.0-(j * dx);
				break;
			}
		}
	}

	return TRUE;
}
#undef MAX_TILE_LEVEL


/*.BH--------------------------------------------------------
**
** ������: GenerateNodePVertexTex
**
** ����:   ������Ƭ������������������ڴ�ռ�(����ÿ����Ƭ���������겻ͬʱ��Ӱ���ͼʱÿ����Ƭ������������ͬ�������˷��ڴ�ռ�)
**         �����붥����������ʱ�����ȵ��øú���
**
** �������:  pNode -- ��Ƭ�ڵ���
**
** �����������
**
** ����ֵ���ɹ�������TRUE;
**		   ʧ�ܣ�����FALSE;
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
BOOL GenerateNodePVertexTex(sQTMAPNODE* pNode)
{
	if(pNode->m_pVertexTex == NULL)
	{
		pNode->m_pVertexTex = (f_float32_t*)NewAlterableMemory(sizeof(f_float32_t) * vertexNum * 2);
		if(pNode->m_pVertexTex == NULL)
		{
			printf("�������󣡣�pNode->m_pVertexTex == NULL\n");
			return FALSE;			
		}
	}
	
	return TRUE;
}



/*
������Ӱ���Ӻ���			
���룺3�������ꡢ����������
�����rbgֵ��
*/

void GetShadowRGB(LP_PT_3F point[3], LP_PT_3D light, f_float32_t* rgb)
{
	sVECOTR3 v1, v2,v3,vlight;
	f_float64_t dotRet,II ;
	f_int16_t shade;
	sPARAMS_FOR_OUTPUT  paramsForOutputBuf;

	//��ʼ��DEMɫ��
	if(TagSum == 0)
	{	
		makeStripeDemImage(stripeDemImage);
		TagSum = 1;
	}

	/*���㶥����������ķ�����*/
	v1.x = point[1]->x - point[0]->x;
	v1.y = point[1]->y - point[0]->y;
	v1.z = point[1]->z - point[0]->z;
	v2.x = point[2]->x - point[0]->x;
	v2.y = point[2]->y - point[0]->y;
	v2.z = point[2]->z - point[0]->z;
	//v2 crossProduct v1;
	vector3Cross(&v3,v1,v2);
	vector3Normalize(&v3);
	
	/*����������շ������������������*/
	setVector3_xyz(&vlight, light->x, light->y, light->z);
	dotRet = vector3Dot(v3, vlight);

	//���㶥����Ӱ��ɫ��ϵ������
	II = ai*ka + li*kd*dotRet;
	if( II > 1.0 )
		II = 1.0;
	if( II < 0.0)
		II = 0.0;
	shade = (short)(II * 255);
	shade = shade * 1.0;				
	paramsForOutputBuf.shade = shade;

	/*��ȡ����ĸ߶ȣ�ȡ��СֵΪ0�����ֵΪ8191�����������ж���߶ȵ����ֵ*/
	paramsForOutputBuf.height = point[0]->z;
	if(paramsForOutputBuf.height > heiMax)
	{
		heiMax = paramsForOutputBuf.height;
		//printf("%d-",paramsForOutputBuf.height);
	}
	if(paramsForOutputBuf.height < 0)
		paramsForOutputBuf.height = 0;
	//��ֹ����Խ��
	if(paramsForOutputBuf.height >= stripeDemImageWidth)
	{
		paramsForOutputBuf.height = (stripeDemImageWidth - 1);
	}

#if 1
	//���ݶ���߶Ȼ�ȡ������ɫ��������Ӱϵ��������ˣ�ģ�����Ч��
	rgb[0] = ((unsigned char)(stripeDemImage[4*paramsForOutputBuf.height] * (1.0 - paramsForOutputBuf.shade/255.0)))/255.0f;
	rgb[1] = ((unsigned char)(stripeDemImage[4*paramsForOutputBuf.height+1] * (1.0 -  paramsForOutputBuf.shade/255.0)))/255.0f;
	rgb[2] = ((unsigned char)(stripeDemImage[4*paramsForOutputBuf.height+2] * (1.0 - paramsForOutputBuf.shade/255.0)))/255.0f;
	//aBuf[j]  = (unsigned char)(0.299 * rBuf[j] + 0.587 * gBuf[j] + 0.114 * bBuf[j]) ;
	//rgb[2] = rgb[1] = rgb[0] = 255 - (unsigned char)paramsForOutputBuf.shade;
#else
	//���ݶ���߶Ȼ�ȡ������ɫ����������Ӱ
	rgb[0] = (unsigned char)stripeDemImage[4*paramsForOutputBuf.height]/255.0f;
	rgb[1] = (unsigned char)stripeDemImage[4*paramsForOutputBuf.height+1]/255.0f;
	rgb[2] = (unsigned char)stripeDemImage[4*paramsForOutputBuf.height+2]/255.0f;
	//aBuf[j]  = (unsigned char)(0.299 * rBuf[j] + 0.587 * gBuf[j] + 0.114 * bBuf[j]) ;
	//printf("(%f %f %f)\t",rgb[0],rgb[1],rgb[2]);
#endif
	
}

/*.BH--------------------------------------------------------
**
** ������: makeStripeDemImage
**
** ����:   ����DEMɫ�������ڼ��㶥����Ӱ��ɫ
**
** �������:  stripeDemImage - DEMɫ������
**
** �����������
**
** ����ֵ����
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
void makeStripeDemImage(unsigned char  stripeDemImage[])
{
	int i = 0,j = 0;
	int texX[ColorNumber] = {0};
	sRGBFLOATCOLOR colorStep[ColorNumber];
//	float level[8] = {0.0f, 0.083f, 0.166f, 0.332f, 0.43f, 0.60f, 0.75f, 1.000f};
	float level[ColorNumber] = {
		0.0033f, 0.0066f, 0.0167f, 0.0333f, 
		0.0500f, 0.0667f, 0.1000f, 0.1333f, 0.1667f,
		0.2000f, 0.2667f, 0.3333f, 0.4000f, 0.4667f,
		0.5333f, 0.6000f, 0.6667f, 0.7333f, 0.8000f,
		0.8667f, 0.9333f, 1.0000f, 2.0000f
	};
#if 1
	//ԭ��ɫ����
	sRGBCOLOR color[ColorNumber] = 
	{ 
		{217, 234, 227},	
		{37, 244, 227},
		{225, 236, 214},
		{214, 228, 202}, 
		{201, 218, 188}, 
		{188, 209, 176},
		{175, 199, 162}, 
		{162, 188, 150},
		{242, 239, 210},
		{251, 232, 176},
		{244, 218, 162},
		{241, 210, 154},
		{238, 202, 149},
		{236, 189, 142},
		{232, 180, 132},
		{228, 163, 117}, 
		{218, 148, 100}, 
		{204, 139, 73},
		{207, 110, 36}, 
		{167, 89, 29}, 
		{178, 133, 100},
		{230, 230, 230}, 
		{240, 240, 240}
	};
#endif

	//��ɫ��ɫ����
/*	sRGBCOLOR color[8] ={
	{90,74,50},
	{90,74,50},
	{90,74,50},
	{90,74,50},
	{90,74,50},
	{90,74,50},
	{90,74,50},
	{90,74,50}
	};

	//��ò��ѣ����ɫ����
	sRGBCOLOR color[8] = {		
	{223,231,196},
	{207,222,174},
	{255,244,219},
	{247,232,184},
	{242,213,139},
	{228,191,155},
	{238,180,127},	
	{180,142,63}
	};*/

	for (i = 1; i < ColorNumber - 1; ++i)
		level[i] /= 2.0;

	//������ɫ��ֵ
	for(i = 0;i<ColorNumber;i++)
	{
//		color[i].r *= 0.5; 
//		color[i].g *= 0.5; 
//		color[i].b *= 0.5; 
	}

	/*������Ϊ8192�������õ�ϵ��level[i]�ֳ�23�Σ�ÿ�γ��Ȳ�ͬ*/
	for(i = 0; i < ColorNumber; i++)
		texX[i] = level[i] * stripeDemImageWidth;  

	/*���ø�����ɫ֮��Ľ���ϵ����ʵ��δʹ��*/
	for(i = 0; i < ColorNumber; i++){
		if(0 == i){
			colorStep[i].r_step = 0.0f;
			colorStep[i].g_step = 0.0f;
			colorStep[i].b_step = 0.0f;
		}
		else if(i < ColorNumber - 1){
			colorStep[i].r_step = (color[i + 1].r - color[i].r) * 1.0f / texX[i];
			colorStep[i].g_step = (color[i + 1].g - color[i].g) * 1.0f / texX[i];
			colorStep[i].b_step = (color[i + 1].b - color[i].b) * 1.0f / texX[i];
		}
		else{
			colorStep[i].r_step = 0.0f;
			colorStep[i].g_step = 0.0f;
			colorStep[i].b_step = 0.0f;
		}
	}
	/*���ø�����ɫ����Ϊcolor[ColorNumber]�����е�ColorNumber����ɫ��͸���Ⱦ�Ϊ��͸��*/
	for (i = 0; i < stripeDemImageWidth ; i++){
		for(j = 0; j < ColorNumber; j++){
			if(i <= texX[j]){
				stripeDemImage[4*i]   = (unsigned char)(color[j].r /*+ colorStep[j].r_step * (i-texX[j])*/);
				stripeDemImage[4*i+1] = (unsigned char)(color[j].g /*+ colorStep[j].g_step * (i-texX[j])*/);
				stripeDemImage[4*i+2] = (unsigned char)(color[j].b /*+ colorStep[j].b_step * (i-texX[j])*/);
				stripeDemImage[4*i+3] = 255;
				break;
			}
		}
	}
}

extern LP_PT_3D GetLightPos();

/*.BH--------------------------------------------------------
**
** ������: GenerateVertexColor
**
** ����:   ������Ƭ�����������Ӱ��ɫ����������SVSģʽ��ͼ����Ӱ��ɫ����ÿ������߶�����
**
** �������:  mapNode -- ��Ƭ�ڵ���
**
** �����������
**
** ����ֵ���ɹ�������TRUE;
**		   ʧ�ܣ�����FALSE;
**
** ���ע�ǣ���
**
**.EH--------------------------------------------------------
*/
BOOL GenerateVertexColor(sQTMAPNODE* mapNode)
{
	f_int32_t i, j , k;
	LP_PT_3F p012[3] ;		// 3���������
	LP_PT_3D vlight = NULL;		// ��������	
	LP_PT_3F pPt = NULL;
	//�洢��ɫָ�룬es3.0ģ������֧�ִ���UCHAR���޸�Ϊfloat
	f_float32_t* pColor = NULL;		
	f_int32_t nLineWidth = 0;
	p012[0] = NULL; p012[1] = NULL; p012[2] = NULL; 
	
	if(mapNode == NULL )
	{
		return FALSE;
	}

	//��ȡ��������
	vlight = GetLightPos();

	// ������ɫ������ڴ棬es3.0ģ������֧�ִ���UCHAR���޸�Ϊfloat
	if(mapNode->m_pColor== NULL)
	{
		mapNode->m_pColor = (f_float32_t *)NewAlterableMemory(sizeof(sColor3f) * vertexNum);
		if(mapNode->m_pColor == NULL)
		{
			printf("�������󣡣�mapNode->m_pColor == NULL\n");
			return FALSE;
		}		
	}
	
	
	//  ������Ӱ��ɫ����start
	nLineWidth = VTXCOUNT + 1;
	pColor = (f_float32_t*)mapNode->m_pColor;
	pPt = (LP_PT_3F)mapNode->fvertex_flat;
	for( i = 0, k = 0; i <= VTXCOUNT; i ++ )
	{
		for( j = 0; j <= VTXCOUNT; j ++,pPt++,k++ )
		{
			//׼�������Ƭ�����ε�3����
			if(j == nLineWidth -1 || i == nLineWidth -1)
			{
				if(j == nLineWidth -1)
				{
					if(i != nLineWidth -1)
					{
						p012[0] = pPt;
						p012[1] = pPt + nLineWidth;
						p012[2] = pPt - 1;
					}
					else
					{
						p012[0] = pPt;
						p012[1] = pPt - 1;
						p012[2] = pPt - nLineWidth;
					}
				}
				else
				{
					p012[0] = pPt;
					p012[1] = pPt - nLineWidth;
					p012[2] = pPt + 1;
				}
			}
			else
			{			
				p012[0] = pPt;
				p012[1] = pPt + 1;
				p012[2] = pPt + nLineWidth;
			}

			//������Ƭ�����κ͹����������㶥����ɫ
			GetShadowRGB(p012, vlight, pColor);
			pColor += 3;
		}
	}
	for(i = 0; i < 4; i++)
	{
		for( j = 0; j <= VTXCOUNT; j++)
		{
			switch( i )
			{
			case 0:
				memcpy(pColor, mapNode->m_pColor+ j*3, 4*3);
				break;
			case 1:
				memcpy(pColor, mapNode->m_pColor + (j + VTXCOUNT*(VTXCOUNT+1))*3, 4*3);
				break;

			case 2:
				memcpy(pColor, mapNode->m_pColor + j*(VTXCOUNT+1)*3, 4*3);
				break;
			case 3:
				memcpy(pColor, mapNode->m_pColor + (j*(VTXCOUNT+1)+VTXCOUNT)*3, 4*3);
				break;
			}
			pColor += 3;
		}
	}
	//  ������Ӱ��ɫ����end	
	return TRUE;
}

