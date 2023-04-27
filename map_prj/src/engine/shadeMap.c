
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
#endif                                     /* 影像数据生成纹理函数 */
#endif

//阴影计算相关的结构体----------------
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
//色带长
#define stripeDemImageWidth 	8192	//5500	//1000    //8192	modify to 1000
#define stripeVecImageWidth 	4096    //256->4096，解决高度值不够精细，导致很多地方跟海洋一个颜色，海陆分界不清晰;JM7200的最大纹理尺寸为4096*4096
#define ColorNumber				23

static unsigned char stripeDemImage[stripeDemImageWidth*4];               //SVS底图色带
static unsigned char stripeVecDemImage[stripeVecImageWidth*4];            //VecShade底图色带

static int TagSum = 0;
static int heiMax = 0;

static f_uint32_t shadeMapTexture[2] = {0};	// 0:用于生成VecShade模式底图的一维或二维纹理ID，1:经纬网的二维纹理ID

extern BOOL objDPt2geoDPt(const LP_Obj_Pt_D objPt, LP_Geo_Pt_D geoPt);

void makeStripeDemImage(unsigned char  stripedemimage[]);

f_uint32_t  InitGridTexture(f_uint32_t width);


/*.BH--------------------------------------------------------
**
** 函数名: InitVecShadedReliefMap
**
** 描述:  生成2个纹理，用于生成SVS模式底图的一维或二维纹理ID，经纬网的二维纹理ID
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无;
**          
**
** 设计注记: 
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

	/*从vecShade.bmp中读取颜色数据*/
	pImageData = ReadBMPEX(VECSHADEPICPATH, &width, &height, &size, &style);
	if(pImageData == NULL)
	{
		printf("%s:载入位图失败!\n", VECSHADEPICPATH);
		return ;
	}

	{
	#define color_num 32

		int i = 0,j = 0;
		int texX[color_num] = {0};
		sRGBFLOATCOLOR colorStep[color_num];
		float level[color_num] = {0};
		sRGBCOLOR color[color_num] ={ 0};

		/* 从读取的颜色数据中选出32个颜色
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
		/*纹理长度为4096，均分成32段，每段长度为128*/
		for(i=0; i < color_num; i++)
		{
			level[i] = (float)(i) / color_num;
		}
		for(i = 0; i < color_num; i++)
			texX[i] = level[i] * stripeVecImageWidth;  

		/*设置颜色渐变系数，目的是让32个颜色之间过渡渐变，实际并未使用*/
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
		/*设置纹理数据，总长度为4096，均分成32段，每段长度为128，每段的颜色对应之前的32种颜色，透明度始终为255，即不透明*/
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

	//OpenGL ES不支持GL_TEXTURE_1D，修改为GL_TEXTURE_2D
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

	//生成经纬网的纹理ID
	shadeMapTexture[1] = InitGridTexture(TILESIZE);
}

/*.BH--------------------------------------------------------
**
** 函数名: RenderShadeMapPre
**
** 描述:  绘制VecShade模式前设置OpenGL纹理状态及纹理矩阵
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：成功，返回TRUE;
**		   失败，返回FALSE;
**          
**
** 设计注记: 无
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
	glMatrixMode(GL_TEXTURE);								// 纹理矩阵（缩放）
	glLoadIdentity();
//	glScalef(0.0002f,0.0002f,0.0002f);						// 地形晕渲图缩放系数，可设置最低点和最高点计算得到
	glScalef(raio, 1.0, 1.0);						// 地形晕渲图缩放系数，可设置最低点和最高点计算得到

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
** 函数名: RenderShadeMapPro
**
** 描述:  绘制VecShade模式前恢复OpenGL纹理状态及纹理矩阵
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:无
**
**.EH--------------------------------------------------------
*/
void RenderShadeMapPro()
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	//glActiveTextureARB(GL_TEXTURE1_ARB);					// 操作纹理单元1
	glActiveTexture(GL_TEXTURE0);					        // 操作纹理单元0
	glMatrixMode(GL_TEXTURE);								// 转为纹理矩阵操作
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// 恢复矩阵堆栈为模型视图矩阵

	glDisable(GL_TEXTURE_1D);

	//glActiveTextureARB(GL_TEXTURE0_ARB);					// 操作纹理单元0
	glActiveTexture(GL_TEXTURE0);					// 操作纹理单元0

#else
	glActiveTextureARB(GL_TEXTURE0);					    // 操作纹理单元0
	glMatrixMode(GL_TEXTURE);								// 转为纹理矩阵操作
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);								// 恢复矩阵堆栈为模型视图矩阵

	glDisableEx(GL_TEXTURE_2D);

#endif
}


/*.BH--------------------------------------------------------
**
** 函数名: RenderShadeMap
**
** 描述:  绘制VecShade模式
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记:无
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
	/* JM7200的glTexCoordPointer不支持GL_SHORT，修改为GL_FLOAT*/
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

	/* 上述方法影响帧率，将fterrain在读取的时候就由short类型转换成float类型 */              
	glClientActiveTexture(GL_TEXTURE0);                
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(1, GL_FLOAT, 0, pNode->fterrain);
	
#else
	/* 实际在mapRenderVecImgEs函数中直接绘制 */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(1, GL_FLOAT, 0, pNode->fterrain);	

#endif

}

/*.BH--------------------------------------------------------
**
** 函数名: GenerateTexture
**
** 描述:   生成方里网的MipMap纹理
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记：无
**
**.EH--------------------------------------------------------
*/
void GenerateTexture(f_uint32_t* texture, f_uint32_t width, f_uint8_t* texBuf)
{
	glGenTextures(1, texture);
	if (*texture == 0)
	{
		printf("!!!报错-纹理对象生成失败\n");
		// 报错-纹理对象生成失败
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
** 函数名: InitGridTexture
**
** 描述:   生成方里网的纹理数据并生成该纹理，网格线的距离是固定的，就是第16层级瓦片的边长所表示的实际距离
**
** 输入参数:  width -- 纹理大小（width*width）
**
** 输出参数：无
**
** 返回值：纹理ID
**          
**
** 设计注记：无
**
**.EH--------------------------------------------------------
*/
f_uint32_t InitGridTexture(f_uint32_t width)
{
	f_uint8_t* texBuf = NULL;
	f_uint32_t i = 0, j = 0;
	f_uint32_t texture_id = 0;
	texBuf = (f_uint8_t*)malloc(sizeof(f_uint8_t) * sqr(width) * 4);
	//  1.生成方里网纹理的数据(从左下角开始存放)，大小为256*256，纹理是一个十字，颜色都是黑色，由字形的地方不透明，其他地方透明
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

	// 2.生成纹理ID
	GenerateTexture(&texture_id, width, texBuf);

	free(texBuf);

	return( texture_id);
}

/*.BH--------------------------------------------------------
**
** 函数名: GetGridTextureID
**
** 描述:   绑定方里网的纹理，准备使用该纹理
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：无
**          
**
** 设计注记：无
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
** 函数名: GenerateTileNormal
**
** 描述:   生成瓦片各个顶点的法向量，光照开启时使用
**
** 输入参数:  mapNode -- 瓦片节点句柄
**
** 输出参数：无
**
** 返回值：成功，返回TRUE;
**		   失败，返回FALSE;
**
** 设计注记：无
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
** 函数名: GenerateTexCoord
**
** 描述:   生成瓦片各个顶点的方里网纹理，必须先调用GenerateNodePVertexTex申请顶点纹理坐标内存
**
** 输入参数:  pNode -- 瓦片节点句柄
**
** 输出参数：无
**
** 返回值：成功，返回TRUE;
**		   失败，返回FALSE;
**
** 设计注记：无
**
**.EH--------------------------------------------------------
*/
BOOL GenerateTexCoord(sQTMAPNODE  *pNode)
{
	f_uint32_t i,j;
	f_float32_t* pTex = NULL;
	f_uint8_t level = pNode->level;
	/*默认16级显示一个方里网纹理，纹理模式为重复，通过设置纹理坐标，使15级显示4个方里网纹理，依次类推*/
	f_float32_t dx = (float)(1.0 / VTXCOUNT * pow(2.0, MAX_TILE_LEVEL - level));
//	f_float32_t dx = (float)(1.0 / VTXCOUNT );

	// create tile index buffer
	// texture 
	pTex = pNode->m_pVertexTex;
	/*顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始*/
	for(i = 0; i <= VTXCOUNT; i++)
	{
		for(j = 0; j <= VTXCOUNT; j++)
		{
			*pTex++ = (j*dx);
			*pTex++ = 1.0-(i*dx);
		}
	}
	/* 设置瓦片的四条裙边的顶点的二维纹理坐标,四条裙边顶点索引顺序是从下->上->左->右 */
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
** 函数名: GenerateNodePVertexTex
**
** 描述:   申请瓦片各个顶点的纹理坐标内存空间(用于每个瓦片的纹理坐标不同时，影像底图时每个瓦片的纹理坐标相同，无需浪费内存空间)
**         ，传入顶点纹理坐标时必须先调用该函数
**
** 输入参数:  pNode -- 瓦片节点句柄
**
** 输出参数：无
**
** 返回值：成功，返回TRUE;
**		   失败，返回FALSE;
**
** 设计注记：无
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
			printf("！！错误！！pNode->m_pVertexTex == NULL\n");
			return FALSE;			
		}
	}
	
	return TRUE;
}



/*
计算阴影，子函数			
输入：3个点坐标、光照向量；
输出：rbg值；
*/

void GetShadowRGB(LP_PT_3F point[3], LP_PT_3D light, f_float32_t* rgb)
{
	sVECOTR3 v1, v2,v3,vlight;
	f_float64_t dotRet,II ;
	f_int16_t shade;
	sPARAMS_FOR_OUTPUT  paramsForOutputBuf;

	//初始化DEM色带
	if(TagSum == 0)
	{	
		makeStripeDemImage(stripeDemImage);
		TagSum = 1;
	}

	/*计算顶点三角形面的法向量*/
	v1.x = point[1]->x - point[0]->x;
	v1.y = point[1]->y - point[0]->y;
	v1.z = point[1]->z - point[0]->z;
	v2.x = point[2]->x - point[0]->x;
	v2.y = point[2]->y - point[0]->y;
	v2.z = point[2]->z - point[0]->z;
	//v2 crossProduct v1;
	vector3Cross(&v3,v1,v2);
	vector3Normalize(&v3);
	
	/*法向量与光照方向向量进行向量点乘*/
	setVector3_xyz(&vlight, light->x, light->y, light->z);
	dotRet = vector3Dot(v3, vlight);

	//计算顶点阴影颜色的系数因子
	II = ai*ka + li*kd*dotRet;
	if( II > 1.0 )
		II = 1.0;
	if( II < 0.0)
		II = 0.0;
	shade = (short)(II * 255);
	shade = shade * 1.0;				
	paramsForOutputBuf.shade = shade;

	/*获取顶点的高度，取最小值为0，最大值为8191，并计算所有顶点高度的最大值*/
	paramsForOutputBuf.height = point[0]->z;
	if(paramsForOutputBuf.height > heiMax)
	{
		heiMax = paramsForOutputBuf.height;
		//printf("%d-",paramsForOutputBuf.height);
	}
	if(paramsForOutputBuf.height < 0)
		paramsForOutputBuf.height = 0;
	//防止数组越界
	if(paramsForOutputBuf.height >= stripeDemImageWidth)
	{
		paramsForOutputBuf.height = (stripeDemImageWidth - 1);
	}

#if 1
	//根据顶点高度获取顶点颜色，并与阴影系数因子相乘，模拟光照效果
	rgb[0] = ((unsigned char)(stripeDemImage[4*paramsForOutputBuf.height] * (1.0 - paramsForOutputBuf.shade/255.0)))/255.0f;
	rgb[1] = ((unsigned char)(stripeDemImage[4*paramsForOutputBuf.height+1] * (1.0 -  paramsForOutputBuf.shade/255.0)))/255.0f;
	rgb[2] = ((unsigned char)(stripeDemImage[4*paramsForOutputBuf.height+2] * (1.0 - paramsForOutputBuf.shade/255.0)))/255.0f;
	//aBuf[j]  = (unsigned char)(0.299 * rBuf[j] + 0.587 * gBuf[j] + 0.114 * bBuf[j]) ;
	//rgb[2] = rgb[1] = rgb[0] = 255 - (unsigned char)paramsForOutputBuf.shade;
#else
	//根据顶点高度获取顶点颜色，不启用阴影
	rgb[0] = (unsigned char)stripeDemImage[4*paramsForOutputBuf.height]/255.0f;
	rgb[1] = (unsigned char)stripeDemImage[4*paramsForOutputBuf.height+1]/255.0f;
	rgb[2] = (unsigned char)stripeDemImage[4*paramsForOutputBuf.height+2]/255.0f;
	//aBuf[j]  = (unsigned char)(0.299 * rBuf[j] + 0.587 * gBuf[j] + 0.114 * bBuf[j]) ;
	//printf("(%f %f %f)\t",rgb[0],rgb[1],rgb[2]);
#endif
	
}

/*.BH--------------------------------------------------------
**
** 函数名: makeStripeDemImage
**
** 描述:   创建DEM色带，用于计算顶点阴影颜色
**
** 输入参数:  stripeDemImage - DEM色带数组
**
** 输出参数：无
**
** 返回值：无
**
** 设计注记：无
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
	//原配色方案
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

	//黄色配色方案
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

	//地貌晕眩的配色方案
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

	//降低颜色幅值
	for(i = 0;i<ColorNumber;i++)
	{
//		color[i].r *= 0.5; 
//		color[i].g *= 0.5; 
//		color[i].b *= 0.5; 
	}

	/*纹理长度为8192，按设置的系数level[i]分成23段，每段长度不同*/
	for(i = 0; i < ColorNumber; i++)
		texX[i] = level[i] * stripeDemImageWidth;  

	/*设置各段颜色之间的渐变系数，实际未使用*/
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
	/*设置各段颜色依次为color[ColorNumber]数组中的ColorNumber种颜色，透明度均为不透明*/
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
** 函数名: GenerateVertexColor
**
** 描述:   生成瓦片各个顶点的阴影颜色，用于生成SVS模式底图；阴影颜色根据每个顶点高度生成
**
** 输入参数:  mapNode -- 瓦片节点句柄
**
** 输出参数：无
**
** 返回值：成功，返回TRUE;
**		   失败，返回FALSE;
**
** 设计注记：无
**
**.EH--------------------------------------------------------
*/
BOOL GenerateVertexColor(sQTMAPNODE* mapNode)
{
	f_int32_t i, j , k;
	LP_PT_3F p012[3] ;		// 3个点的坐标
	LP_PT_3D vlight = NULL;		// 光照向量	
	LP_PT_3F pPt = NULL;
	//存储颜色指针，es3.0模拟器不支持传入UCHAR，修改为float
	f_float32_t* pColor = NULL;		
	f_int32_t nLineWidth = 0;
	p012[0] = NULL; p012[1] = NULL; p012[2] = NULL; 
	
	if(mapNode == NULL )
	{
		return FALSE;
	}

	//获取光照向量
	vlight = GetLightPos();

	// 生成颜色数组的内存，es3.0模拟器不支持传入UCHAR，修改为float
	if(mapNode->m_pColor== NULL)
	{
		mapNode->m_pColor = (f_float32_t *)NewAlterableMemory(sizeof(sColor3f) * vertexNum);
		if(mapNode->m_pColor == NULL)
		{
			printf("！！错误！！mapNode->m_pColor == NULL\n");
			return FALSE;
		}		
	}
	
	
	//  生成阴影颜色向量start
	nLineWidth = VTXCOUNT + 1;
	pColor = (f_float32_t*)mapNode->m_pColor;
	pPt = (LP_PT_3F)mapNode->fvertex_flat;
	for( i = 0, k = 0; i <= VTXCOUNT; i ++ )
	{
		for( j = 0; j <= VTXCOUNT; j ++,pPt++,k++ )
		{
			//准备组成瓦片三角形的3个点
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

			//根据瓦片三角形和光照向量计算顶点颜色
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
	//  生成阴影颜色向量end	
	return TRUE;
}

