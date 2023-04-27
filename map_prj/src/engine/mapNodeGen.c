#include <math.h>
#include "mapNodeGen.h"
#include "mapNodeJudge.h"
#include "../projection/coord.h"
#include "memoryPool.h"

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#ifdef WIN32 	
extern PFNGLACTIVETEXTUREPROC glActiveTexture;	
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;	
extern PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD1FPROC glMultiTexCoord1f;
#endif  
#endif

int gMAXQTNODEINRAM = 0;

#define SWAP32(_X_) (((_X_) >> 24) | (((_X_) & 0x00FF0000) >> 8) | (((_X_) & 0x0000FF00) << 8) | ((_X_) << 24))
static f_float32_t SWAPFLOAT(f_float32_t x)
{
	f_uint32_t _x;
	f_float32_t ret;
	_x = *(f_uint32_t *)&x;
	_x = SWAP32(_x);
	memcpy(&ret, &_x, 4);
	return ret;
}

void geo2obj(f_float64_t lon, f_float64_t lat, f_float64_t ht,
                    f_float64_t *x, f_float64_t *y, f_float64_t *z);


/*
功能：生成瓦片顶点的二维纹理坐标,二维纹理坐标数组的大小为33*(33+4)
输入：
	无
输出：
    无
返回值：
    无
*/
void generateTexBuffer(void)
{
	/* 获取全局顶点的二维纹理坐标数组的指针 */
	f_float32_t* pTex = gTextureBuf;
	f_int32_t i=0, j=0;
	f_float32_t dx = 1.0f / VTXCOUNT;

	/* 设置瓦片内部顶点的二维纹理坐标,顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始,故各个顶点的纹理坐标为 */
	/* x范围为[0,1],一行共33个顶点,故x方向的值依次为0,1/32,2/32,...,31/32,1 */
	/* y范围为[0,1],一列共33个顶点,故y方向的值依次为1,31/32,30/32,...,1/32,0 */
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
			// lower, upper, left, right
			switch(i)
			{
			/* 设置瓦片的下裙边的顶点的二维纹理坐标,与瓦片第二行(从下往上数)顶点的纹理坐标一致 */
			case 0:
				*pTex++ = (j * dx);
				*pTex++ = 1.0-dx;
				break;
			/* 设置瓦片的上裙边的顶点的二维纹理坐标,与瓦片第二行(从上往下数)顶点的纹理坐标一致 */
			case 1:
				*pTex++ = (j * dx);
				*pTex++ = dx;
				break;
			/* 设置瓦片的左裙边的顶点的二维纹理坐标,与瓦片第二列(从左往右数)顶点的纹理坐标一致 */
			case 2:
				*pTex++ = dx;
				*pTex++ = 1.0-(j * dx);
				break;
			/* 设置瓦片的右裙边的顶点的二维纹理坐标,与瓦片第二列(从右往左数)顶点的纹理坐标一致 */
			case 3:
				*pTex++ = (1 - dx);
				*pTex++ = 1.0-(j * dx);
				break;
			}
		}
	}

#define Do_FileTextureVertexData 1	// 将0层数据放入文件并读出
#if Do_FileTextureVertexData // gx test for debug 输出纹理坐标为文件
	{
		/*
			f_float32_t* pTex数据设置代码如下：
		*/
		FILE* fp = fopen("D:\\ImgData\\TextCoord.txt", "w");
		if (fp != NULL) 
		{
			int pos=0;
			// 每个瓦块的二维纹理坐标
			for(i = 0; i <= VTXCOUNT; i++)
			{
				for(j = 0; j <= VTXCOUNT; j++)
				{
					fprintf(fp, "%f ", gTextureBuf[pos++]);
					fprintf(fp, "%f ", gTextureBuf[pos++]);
				}
			}

			// 四条裙边纹理坐标
			for(i = 0; i < 4; i ++)
			{
				for(j = 0; j <= VTXCOUNT; j++)
				{
					fprintf(fp, "%f ", gTextureBuf[pos++]);
					fprintf(fp, "%f ", gTextureBuf[pos++]);
				}
			}
			fclose(fp);
		}
		else 
		{
			printf("无法打开文件！\n");
		}
	}
#endif
}


/*
功能：生成三角形条带化后瓦片顶点的索引,总索引数组的大小为(33*2+2)*3+(33*2)+(32*34*2)=2446
输入：
	无
输出：
    无
返回值：
    无
*/
void generateTileIndices(void)
{
#if 1
	/* 获取全局顶点索引数组的指针 */
	f_uint32_t* pIndex = gindices;
	f_uint32_t i, j, k, m;

	/* 与瓦片内部下面接边,一条边33个顶点,顶点索引值依次为1089,0,1090,1,1091,2,1093,3...1121,32,32,0 */
	/* 由于绘制方式是三角形条带，故第1个三角形的顶点索引为1089,0,1090,第2个三角形的顶点绘制顺序为1090,0,1, */
	/* 第3个三角形的顶点绘制顺序为1090,1,1091,之后以此类推,顶点顺序为顺时针方向 */
	/* 最后增加两个顶点索引32,0,这样就可以和瓦片内部的三角形条带连起来 */
	/* 故第65个三角形的顶点绘制顺序为1121,32,32,第66个三角形的顶点绘制顺序为32,32,0, */
	/* 第67个三角形的顶点绘制顺序为32,0,0,第68个三角形的顶点绘制顺序为0,0,33,之后就与瓦片内部的第1个三角形连起来了 */
	/* 共增加4个三角形,这4个三角形就是一条直线,不影响绘制 */
	/* 共33个顶点,起始点索引序号为1089,最后一个点索引序号为1121,总共顶点索引个数为33*2+2 */
	k = (VTXCOUNT + 1) * (VTXCOUNT + 1);
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = k;
		pIndex[1] = j;
		pIndex += 2;
	}
	pIndex[0] = j - 1;	// 增加一條横线，連接到第一個顶点,以便於內部三角形相连
	pIndex[1] = 0;
	pIndex += 2;

	/* 瓦片内部,每个瓦片33*33个顶点,第0个顶点从左下角开始,顶点索引值依次为0,33,1,34,2,35,3,36...32,65,65,33,33,66...1055,1088,1088,1056 */
	/* 第1个三角形的顶点索引为0,33,1;顶点顺序为顺时针方向,绘制的时候必须设置GL_CW,即顺时针方向为三角形正面 */
	/* 由于绘制方式是三角形条带，故第2个三角形的顶点绘制顺序为1,33,34,第3个三角形的顶点绘制顺序为1,34,2,之后以此类推 */
	/* 为了从第一行的三角形过渡到第二行的三角形,增加2个顶点索引值65,33,故第65个三角形的顶点绘制顺序为32,65,65, */
	/* 第66个三角形的顶点绘制顺序为65,65,33,第67个三角形的顶点绘制顺序为33,65,33,第68个三角形的顶点绘制顺序为33,33,66, */
	/* 共增加4个三角形,这4个三角形就是一条直线,不影响绘制 */
	/* 同理,最后增加2个顶点索引值1088,1056,故倒数第4个三角形的顶点绘制顺序为1055,1088,1088, */
	/* 倒数第3个三角形的顶点绘制顺序为1088,1088,1056,倒数第2个三角形的顶点绘制顺序为1056,1088,1056, */
	/* 倒数第1个三角形的顶点绘制顺序为1056,1056,1122,后面就与上面接边相连 */
	/* 共33*33=1089个点,起始点索引序号为0,最后一个点索引序号为32+33*32=1088,总共顶点索引个数为32*33*2+32*2 */
	for (i = 0, m = 0; i < VTXCOUNT; ++i)	{
		for (j = 0; j < VTXCOUNT1; ++j, ++m) {
			pIndex[0] = m;
			pIndex[1] = m + VTXCOUNT1;
			pIndex += 2;
			// 到了最後一列,增加一條橫線連接到上一行
			if (i != VTXCOUNT - 1 && j == VTXCOUNT) {
				pIndex[0] = m + VTXCOUNT1;
				pIndex[1] = m + 1;
				pIndex += 2;
			}
		}
	}
	pIndex[0] = m + VTXCOUNT;	// 新增一条从尾到头的横线，以便后续连接到新增的最上一行
	pIndex[1] = m;
	pIndex += 2;

	/* 与瓦片内部上面接边,一条边33个顶点,顶点索引值依次为1056,1122,1057,1123,1058,1124,1059,1125...1088,1154,1154,1155 */
	/* 由于绘制方式是三角形条带，故第1个三角形的顶点索引为1056,1122,1057,第2个三角形的顶点绘制顺序为1057,1122,1123, */
	/* 第3个三角形的顶点绘制顺序为1057,1123,1058,之后以此类推,顶点顺序为顺时针方向 */
	/* 最后增加两个顶点索引1154,1155,这样就可以和瓦片内部左边接边连起来 */
	/* 故倒数第4个三角形的顶点绘制顺序为1088,1154,1154,倒数第3个三角形的顶点绘制顺序为1154,1154,1155, */
	/* 倒数第2个三角形的顶点绘制顺序为1154,1155,1155,倒数第1个三角形的顶点绘制顺序为1155,1155,0,之后就与瓦片内部左边接边的第1个三角形连起来了 */
	/* 共增加4个三角形,这4个三角形就是一条直线,倒数第3和第2个三角形都是同一条斜对角线 */
	/* 共33个顶点,起始点索引序号为1122,最后一个点索引序号为1154,总共顶点索引个数为33*2+2 */
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = j + VTXCOUNT * (VTXCOUNT + 1);
		pIndex[1] = k;
		pIndex += 2;
	}
	pIndex[0] = k - 1;	// 新增一条斜角线,从最上行最右侧连接到最下行最左侧,以与新增的左列连接起来
	pIndex[1] = k;
	pIndex += 2;

	/* 与瓦片内部左边接边,一条边33个顶点,顶点索引值依次为1155,0,1156,33,1157,66,1158,99...1187,1056,1056,32 */
	/* 由于绘制方式是三角形条带，故第1个三角形的顶点索引为1155,0,1156,第2个三角形的顶点绘制顺序为1156,0,33, */
	/* 第3个三角形的顶点绘制顺序为1156,33,1157,之后以此类推,顶点顺序为逆时针方向 */
	/* 最后增加两个顶点索引1056,32,这样就可以和瓦片内部右边接边连起来 */
	/* 故倒数第4个三角形的顶点绘制顺序为1187,1056,1056,倒数第3个三角形的顶点绘制顺序为1056,1056,32, */
	/* 倒数第2个三角形的顶点绘制顺序为1056,32,32,倒数第1个三角形的顶点绘制顺序为32,32,1188,之后就与瓦片内部左边接边的第1个三角形连起来了 */
	/* 共增加4个三角形,这4个三角形就是一条直线,倒数第3和第2个三角形都是同一条斜对角线 */
	/* 共33个顶点,起始点索引序号为1155,最后一个点索引序号为1187,总共顶点索引个数为33*2+2 */
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = k;
		pIndex[1] = j * (VTXCOUNT + 1);
		pIndex += 2;
	}
	pIndex[0] = (j - 1) * (VTXCOUNT + 1);
	pIndex[1] = VTXCOUNT;
	pIndex += 2;

	/* 与瓦片内部右边接边,一条边33个顶点,顶点索引值依次为32,1188,65,1189,98,1190,131,1191...1088,1220 */
	/* 由于绘制方式是三角形条带，故第1个三角形的顶点索引为32,1188,65,第2个三角形的顶点绘制顺序为65,1188,1189, */
	/* 第3个三角形的顶点绘制顺序为65,1189,98,之后以此类推,顶点顺序为逆时针方向 */
	/* 最后不增加顶点索引 */
	/* 共33个顶点,起始点索引序号为1188,最后一个点索引序号为1220,总共顶点索引个数为33*2 */
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = j * (VTXCOUNT + 1) + VTXCOUNT;
		pIndex[1] = k;
		pIndex += 2;
	}

#else
	f_int32_t i, j, k = 0;
	f_uint32_t* pIndex = indices;
	for(i = 0; i < VTXCOUNT; i++, k++)
	{
		for(j = 0; j < VTXCOUNT; j++, k++)
		{
			pIndex[0] = k;
			pIndex[1] = k + 1;
			pIndex[2] = k + VTXCOUNT + 1;
			pIndex += 3;
			pIndex[0] = k+1;
			pIndex[1] = k+ VTXCOUNT +2;
			pIndex[2] = k+ VTXCOUNT +1;
			pIndex += 3;
		}
	}
	k = (VTXCOUNT + 1) * (VTXCOUNT + 1);

	for(i = 0; i < 4; i++, k++)/* 四条裙边 */
	{
		for(j = 0; j < VTXCOUNT; j++, k++)
		{
			switch(i)
			{
			case 0:								// 上面接边
				pIndex[0] = j;
				pIndex[1] = k;
				pIndex[2] = j + 1;
				pIndex += 3;
				pIndex[0] = k + 1;
				pIndex[1] = j+1;
				pIndex[2] = k;
				pIndex += 3;
				break;
			case 1:								// 下边接边
				pIndex[0] = j + 1 + VTXCOUNT*(VTXCOUNT+1);
				pIndex[1] = k;
				pIndex[2] = j + VTXCOUNT*(VTXCOUNT+1);
				pIndex += 3;
				pIndex[0] = k;
				pIndex[1] = j+1 + VTXCOUNT*(VTXCOUNT+1);
				pIndex[2] = k + 1;
				pIndex += 3;
				break;
			case 2:						// 左边接边
				pIndex[0] = (j + 1) * (VTXCOUNT+1);
				pIndex[1] = k;
				pIndex[2] = j * (VTXCOUNT+1);
				pIndex += 3;
				pIndex[0] = k;
				pIndex[1] = (j+1)*(VTXCOUNT+1);
				pIndex[2] = k + 1;
				pIndex += 3;
				break;

			case 3:						// 右边接边	
				pIndex[0] = j*(VTXCOUNT+1) + VTXCOUNT;
				pIndex[1] = k;
				pIndex[2] = (j+1)*(VTXCOUNT+1) + VTXCOUNT;
				pIndex += 3;
				pIndex[0] = k + 1;
				pIndex[1] = (j+1)*(VTXCOUNT+1) + VTXCOUNT;
				pIndex[2] = k;
				pIndex += 3;
				break;
			}
		}
	}
#endif

#define Do_FileTextureVertexData 1	// 将0层数据放入文件并读出
#if Do_FileTextureVertexData	//gx test for Debug 瓦块顶点索引 
	{
		{
			//使用C语言 把下面的newImage写到本地(D:\level_xidx_yidx.txt)文件里
			// 拼接文件路径
			char filePath[256]={0};
			FILE *fp = NULL;
			int pos = 0;
			sprintf(filePath, "D:\\ImgData\\vertexIdx.txt");
			fp = fopen(filePath, "wb");
			// 创建文件并打开
			if (fp == NULL) {
				printf("Failed to create file!\n");
				return 1;
			}

			/* 与瓦片内部下面接边,一条边33个顶点,顶点索引值依次为1089,0,1090,1,1091,2,1093,3...1121,32,32,0 */
			for(j = 0; j < VTXCOUNT1; j++, k++) {
				//pIndex[0] = k;
				//pIndex[1] = j;
				//pIndex += 2;
				fprintf(fp, "%d ", gindices[pos++]);
				fprintf(fp, "%d ", gindices[pos++]);

			}
			fprintf(fp, "%d ", gindices[pos++]);
			fprintf(fp, "%d ", gindices[pos++]);
			//pIndex[0] = j - 1;	// 增加一條横线，連接到第一個顶点,以便於內部三角形相连
			//pIndex[1] = 0;
			//pIndex += 2;

			/* 瓦片内部,每个瓦片33*33个顶点,第0个顶点从左下角开始,顶点索引值依次为0,33,1,34,2,35,3,36...32,65,65,33,33,66...1055,1088,1088,1056 */
			for (i = 0, m = 0; i < VTXCOUNT; ++i)	{
				for (j = 0; j < VTXCOUNT1; ++j, ++m) {
					fprintf(fp, "%d ", gindices[pos++]);
					fprintf(fp, "%d ", gindices[pos++]);
					//pIndex[0] = m;
					//pIndex[1] = m + VTXCOUNT1;
					//pIndex += 2;

					// 到了最後一列,增加一條橫線連接到上一行
					if (i != VTXCOUNT - 1 && j == VTXCOUNT) 
					{
						fprintf(fp, "%d ", gindices[pos++]);
						fprintf(fp, "%d ", gindices[pos++]);
						//pIndex[0] = m + VTXCOUNT1;
						//pIndex[1] = m + 1;
						//pIndex += 2;
					}
				}
			}
			fprintf(fp, "%d ", gindices[pos++]);
			fprintf(fp, "%d ", gindices[pos++]);
			//pIndex[0] = m + VTXCOUNT;	// 新增一条从尾到头的横线，以便后续连接到新增的最上一行
			//pIndex[1] = m;
			//pIndex += 2;


			/* 与瓦片内部上面接边,一条边33个顶点,顶点索引值依次为1056,1122,1057,1123,1058,1124,1059,1125...1088,1154,1154,1155 */
			for(j = 0; j < VTXCOUNT1; j++, k++) 
			{
				fprintf(fp, "%d ", gindices[pos++]);
				fprintf(fp, "%d ", gindices[pos++]);
				//pIndex[0] = j + VTXCOUNT * (VTXCOUNT + 1);
				//pIndex[1] = k;
				//pIndex += 2;
			}
			fprintf(fp, "%d ", gindices[pos++]);
			fprintf(fp, "%d ", gindices[pos++]);
			//pIndex[0] = k - 1;	// 新增一条斜角线,从最上行最右侧连接到最下行最左侧,以与新增的左列连接起来
			//pIndex[1] = k;
			//pIndex += 2;

			/* 与瓦片内部左边接边,一条边33个顶点,顶点索引值依次为1155,0,1156,33,1157,66,1158,99...1187,1056,1056,32 */
			for(j = 0; j < VTXCOUNT1; j++, k++) 
			{
				fprintf(fp, "%d ", gindices[pos++]);
				fprintf(fp, "%d ", gindices[pos++]);
				//pIndex[0] = k;
				//pIndex[1] = j * (VTXCOUNT + 1);
				//pIndex += 2;
			}
			fprintf(fp, "%d ", gindices[pos++]);
			fprintf(fp, "%d ", gindices[pos++]);
			//pIndex[0] = (j - 1) * (VTXCOUNT + 1);
			//pIndex[1] = VTXCOUNT;
			//pIndex += 2;

			/* 与瓦片内部右边接边,一条边33个顶点,顶点索引值依次为32,1188,65,1189,98,1190,131,1191...1088,1220 */
			/* 由于绘制方式是三角形条带，故第1个三角形的顶点索引为32,1188,65,第2个三角形的顶点绘制顺序为65,1188,1189, */
			/* 第3个三角形的顶点绘制顺序为65,1189,98,之后以此类推,顶点顺序为逆时针方向 */
			/* 最后不增加顶点索引 */
			/* 共33个顶点,起始点索引序号为1188,最后一个点索引序号为1220,总共顶点索引个数为33*2 */
			for(j = 0; j < VTXCOUNT1; j++, k++) {
				fprintf(fp, "%d ", gindices[pos++]);
				fprintf(fp, "%d ", gindices[pos++]);
				//pIndex[0] = j * (VTXCOUNT + 1) + VTXCOUNT;
				//pIndex[1] = k;
				//pIndex += 2;
			}
			// 关闭文件
			fclose(fp);
		}
	}
#endif
}

/*生成鹰眼模式纹理坐标*/
void generateTexBufferEyeBird(void)
{
	f_float32_t* pTex = gTextureBuf_eyebird;
	f_int32_t i=0, j=0;
	f_float32_t dx = 1.0f / VTXCOUNT_EYEBIRD;
	for(i = 0; i <= VTXCOUNT_EYEBIRD; i++)
	{
		for(j = 0; j <= VTXCOUNT_EYEBIRD; j++)
		{
			*pTex++ = (j*dx);
			*pTex++ = 1.0-(i*dx);
		}
	}
}

/* 生成鹰眼模式三角形条带化后顶点的索引 */
void generateTileIndicesEyeBird(void)
{
	f_int32_t i, j, k = 0;
	f_uint32_t* pIndex = indices_eyebird;
	for(i = 0; i < VTXCOUNT_EYEBIRD; i++, k++)
	{
		for(j = 0; j < VTXCOUNT_EYEBIRD; j++, k++)
		{
			pIndex[0] = k;
			pIndex[1] = k + 1;
			pIndex[2] = k + VTXCOUNT_EYEBIRD + 1;
			pIndex += 3;
			pIndex[0] = k+1;
			pIndex[1] = k+ VTXCOUNT_EYEBIRD +2;
			pIndex[2] = k+ VTXCOUNT_EYEBIRD +1;
			pIndex += 3;
		}
	}
}

/* 功能：生成球面模型的网格顶点数据(正常模式) 
	terrain: 当前瓦块33*33格网顶点高程数组
*/
f_int32_t qtmapNodeGenGlobalGrid(sQTMAPNODE *pNode, f_int16_t *terrain)
{
	f_int32_t i, j, i_lon, i_lat;
	f_float32_t hv[2], hvsin[VTXCOUNT1][2], hvcos[VTXCOUNT1][2];	
	f_float32_t hstep = 0.0f, vstep = 0.0f, lon = 0.0f, lat = 0.0f;
	f_int16_t *pTerrainLine = NULL;
	/*JM7200不支持short类型，将fterrain在读取的时候转换为float类型*/
	f_float32_t *pZ = NULL;
	f_float32_t *temp_fvertex = NULL;
	
	// 如果当前瓦块已经生成顶点数组,则不再处理
	if(pNode->fvertex != NULL)
	    return 0;
	// 创建三角顶点数组
	else
	{
		/* index_count个点，每个点个坐标（x,y,z）,每个坐标4个字节 */		
		pNode->fvertex = (f_float32_t *)NewAlterableMemory(vertexNum * 3 * sizeof(f_float32_t));
		if(pNode->fvertex == NULL)
		    return -1;
		pNode->fterrain = (f_float32_t *)NewAlterableMemory(vertexNum * sizeof(f_float32_t));
		if(pNode->fterrain == NULL)
		    return -1;
	}	
	temp_fvertex = pNode->fvertex;
	pZ = pNode->fterrain;


	// 预先计算每个格网点的cos|sin，便于后续快速计算顶点的球坐标
    hstep = pNode->hscale / VTXCOUNT;
    vstep = pNode->vscale / VTXCOUNT;
	for (i = 0; i < VTXCOUNT1; i++)
	{
		lon = pNode->hstart + i * hstep;
		lat = pNode->vstart + (i) * vstep;
		hv[0] = (f_float32_t)(lon * DE2RA);
		hv[1] = (f_float32_t)(lat * DE2RA);
		hvcos[i][0] = (f_float32_t)cos(hv[0]);
		hvcos[i][1] = (f_float32_t)cos(hv[1]);
		hvsin[i][0] = (f_float32_t)sin(hv[0]);
		hvsin[i][1] = (f_float32_t)sin(hv[1]);
	}
	

	// 计算网格对应的每个顶点的球坐标(顶点自下而上排布)
	for(i_lat = 0; i_lat < VTXCOUNT1; i_lat++)		// 对于某行
	{
		pTerrainLine = (f_int16_t *)(&terrain[VTXCOUNT1 * (VTXCOUNT1 -1 - i_lat)]);	// terrain高程自上而下,所以这里进行上下翻转        	
	    for(i_lon = 0; i_lon < VTXCOUNT1; i_lon++)	// 对于某列
	    {
	    	f_int32_t pos, ter_pos;
	    	f_float32_t ht0; 
			ter_pos = (i_lat * VTXCOUNT1 + i_lon) ;	
		    pos = (i_lat * VTXCOUNT1 + i_lon) * 3;
		    
		    ht0 = ((f_float32_t)pTerrainLine[i_lon]) + EARTH_RADIUS;
			temp_fvertex[pos + 0] = ht0 * hvcos[i_lat][1] * hvcos[i_lon][0];
			temp_fvertex[pos + 1] = ht0 * hvcos[i_lat][1] * hvsin[i_lon][0];
			temp_fvertex[pos + 2] = ht0 * hvsin[i_lat][1];
			pZ[ter_pos] = pTerrainLine[i_lon]*1.0f;
	    }
	}


	// 在33*33格网外围上下左扩充一行，左右扩充一列
	for(i = 0; i < 4; i++)	
	{
	    for(j = 0; j < VTXCOUNT1; j++)	// 对于某列
	    {
	    	f_int32_t pos0, pos1, ter_pos0,  lat_i, lon_i;
			f_float32_t ht0; 
	    	switch(i)
	    	{
	    		case 0: /*扩充网格下面一行*/
	    			pos0 = ((VTXCOUNT1) * VTXCOUNT1 + j) * 3;	
					ter_pos0 = (VTXCOUNT1) * VTXCOUNT1 + j;
	    			pos1 =  (VTXCOUNT1 - 1) * VTXCOUNT1 + j;
					// 顶点坐标与第一行保持一致
	    			lon_i = j;
	    			lat_i = 0;
	    			break;
	    		
	    		case 1: /*扩充网格上面一行*/
	    			pos0 = ((VTXCOUNT1 + 1) * VTXCOUNT1 + j) * 3;	// +1:每一列增加了一个上面的顶点
					ter_pos0 = (VTXCOUNT1 + 1) * VTXCOUNT1 + j;
					pos1 = j; //(VTXCOUNT1 - 1) * VTXCOUNT1 + j;	
					// 顶点坐标与最后一行保持一致
					lon_i = j;
	    			lat_i = VTXCOUNT;	
	    			break;
	    		
	    		case 2: /*扩充左列*/
	    			pos0 = ((VTXCOUNT1 + 2) * VTXCOUNT1 + j) * 3;	// +2:每一列增加了一个上面与下面的顶点
					ter_pos0 = (VTXCOUNT1 + 2) * VTXCOUNT1 + j;
					pos1 =(VTXCOUNT1 - 1 -j) * VTXCOUNT1;// j * VTXCOUNT1;
					// 顶点坐标与第一列保持一致
					lon_i = 0;
	    			lat_i = j;
	    			break;
	    		
	    		case 3: /*扩充右列*/
	    			pos0 = ((VTXCOUNT1 + 3) * VTXCOUNT1 + j) * 3;
					ter_pos0 = (VTXCOUNT1 + 3) * VTXCOUNT1 + j;
					pos1 = (VTXCOUNT1 - 1 -j)  * VTXCOUNT1 + VTXCOUNT1 - 1;// j * VTXCOUNT1 + VTXCOUNT1 - 1;
					// 顶点坐标与最后一列保持一致
					lon_i = VTXCOUNT;
	    			lat_i = j;
	    			break;
	    	}

		    ht0 = terrain[pos1]*1.0f - 999.0f + EARTH_RADIUS;	
		    temp_fvertex[pos0 + 0] = ht0 * hvcos[lat_i][1] * hvcos[lon_i][0];
		    temp_fvertex[pos0 + 1] = ht0 * hvcos[lat_i][1] * hvsin[lon_i][0];
		    temp_fvertex[pos0 + 2] = ht0 * hvsin[lat_i][1];
		    pZ[ter_pos0] = terrain[pos1]*1.0f - 999.0f;	// 高度下沉999, 为了避免临瓦块接不上导致露白
	    }
	}

	return 0;
}


/*
功能：计算平面模式的瓦片顶点坐标，用于计算瓦片三角形的法向量，从而生成SVS模式各个顶点的颜色。其中x,y用相对左上角的
实际距离(单位是m)确定，z值为瓦片顶点实际高度(单位是m)
      
输入：
	pNode       当前瓦片节点指针
	terrain     当前瓦片节点的顶点高度指针

输入输出：
	无

输出：
    无

返回值：
    -1 平面模式瓦片顶点数据指针无效
    0  平面模式瓦片顶点数据生成成功
*/
f_int32_t qtmapNodeGenGlobalGridFlat(sQTMAPNODE *pNode, f_int16_t *terrain)
{
	f_int32_t i, j, i_lon, i_lat;
	f_float32_t hstep = 0.0f, vstep = 0.0f, lon = 0.0f, lat = 0.0f;
	f_int16_t *pTerrainLine = NULL;
	f_int16_t *pZ = NULL;
	f_float32_t *temp_fvertex = NULL;
	
	if(pNode->fvertex_flat != NULL)
	{
	    return 0;
	}
	else
	{
		/* index_count个点，每个点个坐标（x,y,z），每个坐标4个字节 */		
		pNode->fvertex_flat = (f_float32_t *)NewAlterableMemory(vertexNum * 3 * sizeof(f_float32_t));
		if(pNode->fvertex_flat == NULL)
		    return -1;

	}	
	temp_fvertex = pNode->fvertex_flat;
	//111319.5m为地球上1经度跨度的实际距离，求出瓦片经纬度方向实际距离，再除以顶点数，得到每个顶点的距离间隔
    hstep = pNode->hscale / VTXCOUNT * 111319.5 ;
    vstep = pNode->vscale / VTXCOUNT * 111319.5 ;
	/*计算瓦片所有顶点的平面顶点坐标*/	
	for(i_lat = 0; i_lat < VTXCOUNT1; i_lat++)
	{
		pTerrainLine = (f_int16_t *)(&terrain[VTXCOUNT1 * (VTXCOUNT1 -1 - i_lat)]);  	        	
	    for(i_lon = 0; i_lon < VTXCOUNT1; i_lon++)	
	    {
	    	f_int32_t pos;
	    	f_float32_t ht0; 
			
		    pos = (i_lat * VTXCOUNT1 + i_lon) * 3;
		    
		    ht0 = ((f_float32_t)pTerrainLine[i_lon]) ;
			temp_fvertex[pos + 0] = i_lon * hstep; 
			temp_fvertex[pos + 1] = i_lat * vstep;
			temp_fvertex[pos + 2] = ht0 ;
	    }
	}
	/*计算瓦片4条裙边的平面顶点坐标*/
	for(i = 0; i < 4; i++)
	{
	    for(j = 0; j < VTXCOUNT1; j++)	
	    {
	    	f_int32_t pos0, pos1, ter_pos0,  lat_i, lon_i;
			f_float32_t ht0; 
	    	switch(i)
	    	{
	    		case 0: /*上*/
	    		pos0 = ((VTXCOUNT1) * VTXCOUNT1 + j) * 3;	
				ter_pos0 = (VTXCOUNT1) * VTXCOUNT1 + j;
	    		pos1 =  (VTXCOUNT1 - 1) * VTXCOUNT1 + j;//j;	
	    		lon_i = j;
	    		lat_i = 0;
	    		break;
	    		
	    		case 1: /*下*/
	    		pos0 = ((VTXCOUNT1 + 1) * VTXCOUNT1 + j) * 3;
				ter_pos0 = (VTXCOUNT1 + 1) * VTXCOUNT1 + j;
		        pos1 = j; //(VTXCOUNT1 - 1) * VTXCOUNT1 + j;	
		        lon_i = j;
	    		lat_i = VTXCOUNT;
	    		break;
	    		
	    		case 2: /*左*/
	    		pos0 = ((VTXCOUNT1 + 2) * VTXCOUNT1 + j) * 3;
				ter_pos0 = (VTXCOUNT1 + 2) * VTXCOUNT1 + j;
		        pos1 =(VTXCOUNT1 - 1 -j) * VTXCOUNT1;// j * VTXCOUNT1;
		        lon_i = 0;
	    		lat_i = j;
	    		break;
	    		
	    		case 3: /*右*/
	    		pos0 = ((VTXCOUNT1 + 3) * VTXCOUNT1 + j) * 3;
				ter_pos0 = (VTXCOUNT1 + 3) * VTXCOUNT1 + j;
		        pos1 = (VTXCOUNT1 - 1 -j)  * VTXCOUNT1 + VTXCOUNT1 - 1;// j * VTXCOUNT1 + VTXCOUNT1 - 1;
		        lon_i = VTXCOUNT;
	    		lat_i = j;
	    		break;
	    	}
			/*裙边顶点高度比实际低999m，防止穿地露白*/
		    ht0 = terrain[pos1] - 999.0f;		
		    temp_fvertex[pos0 + 0] =  i_lon * hstep;
		    temp_fvertex[pos0 + 1] =  i_lat * vstep;
		    temp_fvertex[pos0 + 2] = ht0 ;
	    }
	}

	return 0;
}


/* 功能：生成球面模型的网格数据(鹰眼模式) */
f_int32_t qtmapNodeGenGlobalGridForEyeBird(sQTMAPNODE *pNode)
{
	f_int32_t i, i_lon, i_lat;
	f_float32_t hstep = 0.0f, vstep = 0.0f, lon = 0.0f, lat = 0.0f;
	f_float32_t hv[2], hvsin[VTXCOUNT1_EYEBIRD][2], hvcos[VTXCOUNT1_EYEBIRD][2];	
	f_float32_t *temp_fvertex = NULL;
	
	if(pNode->fvertex != NULL)
	    return 0;
	else
	{
		/* index_count个点，每个点个坐标（x,y,z），每个坐标4个字节 */		
		pNode->fvertex = NewAlterableMemory(vertexNum_eyebird * 3 * sizeof(f_float32_t));
		if(pNode->fvertex == NULL)
		    return -1;
		pNode->fterrain = NULL;
	}	
	temp_fvertex = pNode->fvertex;

	hstep = pNode->hscale / VTXCOUNT_EYEBIRD;
	vstep = pNode->vscale / VTXCOUNT_EYEBIRD;
	for (i = 0; i < VTXCOUNT1_EYEBIRD; i++)
	{
		lon = pNode->hstart + hstep * i;
		lat = pNode->vstart + vstep * (i);
		hv[0] = (f_float32_t)(lon * DE2RA);
		hv[1] = (f_float32_t)(lat * DE2RA);
		hvcos[i][0] = (f_float32_t)cos(hv[0]);
		hvcos[i][1] = (f_float32_t)cos(hv[1]);
		hvsin[i][0] = (f_float32_t)sin(hv[0]);
		hvsin[i][1] = (f_float32_t)sin(hv[1]);
	}
	
	for(i_lat = 0; i_lat < VTXCOUNT1_EYEBIRD; i_lat++)
	{	        	
	    for(i_lon = 0; i_lon < VTXCOUNT1_EYEBIRD; i_lon++)	
	    {
	    	f_int32_t pos, ter_pos;
	    	f_float32_t ht0; 
			ter_pos = (i_lat * VTXCOUNT1_EYEBIRD + i_lon) ;
		    pos = (i_lat * VTXCOUNT1_EYEBIRD + i_lon) * 3;
		    
		    ht0 = EARTH_RADIUS;
			temp_fvertex[pos + 0] = ht0 * hvcos[i_lat][1] * hvcos[i_lon][0];
			temp_fvertex[pos + 1] = ht0 * hvcos[i_lat][1] * hvsin[i_lon][0];
			temp_fvertex[pos + 2] = ht0 * hvsin[i_lat][1];
	    }
	}
	
	return 0;	
}


/*
功能：将瓦片顶点由浮点型转换为整形,转换的方式是以瓦片第一个顶点作为基准点，后续顶点保存相对第一个顶点的整形偏移值即可.
      整形偏移值的计算方法是求浮点数之差，再乘以放大倍数，故基准点还需保存缩放系数
      
输入：
	pNode       当前瓦片节点指针

输入输出：
	无

输出：
    无

返回值：
    -1 浮点型或整形的瓦片顶点数据指针无效
    0  整形的瓦片顶点数据生成成功
*/
f_int32_t qtmapNodeGenPartGrid(sQTMAPNODE *pNode)
{
	f_int32_t i = 0, j = 0, i_lat = 0, i_lon = 0;
	/* 浮点型的瓦片顶点数据指针 */
	f_float32_t *temp_fvertex = NULL;
	/* 整型的瓦片顶点数据指针 */
	f_int32_t *temp_fvertex_part = NULL;
	/* 将浮点型转换成整形的放大倍数,1073741824.0=2^30,为何是这个值 */
	double size =  0.1 * 1073741824.0 / pNode->nodeRange.radius; //100.0;
	
	if(pNode->fvertex == NULL)
	    return -1;
	
	/* index_count个点，每个点个坐标（x,y,z），每个坐标4个字节 */		
	pNode->fvertex_part = NewAlterableMemory(vertexNum * 3 * sizeof(f_int32_t));
	if(pNode->fvertex_part == NULL)
	    return -1;

	temp_fvertex = pNode->fvertex;
	temp_fvertex_part = pNode->fvertex_part;

	/* 瓦片的第一个顶点坐标作为基准点的xyz */
	pNode->point_vertex_xyz[0] = temp_fvertex[0];
	pNode->point_vertex_xyz[1] = temp_fvertex[1];
	pNode->point_vertex_xyz[2] = temp_fvertex[2];
	/* 放大倍数的倒数作为基准点的缩放系数 */
	pNode->point_vertex_xyz[3] = 1.0 / size ;
	
		
	/* 依次计算瓦片中各个顶点相对于基准点的偏移，并乘以放大倍数，转换为整形 */
	for(i_lat = 0; i_lat < VTXCOUNT1; i_lat++)
	{
    	
	    for(i_lon = 0; i_lon < VTXCOUNT1; i_lon++)	
	    {
	        f_int32_t pos;
		    pos = (i_lat * VTXCOUNT1 + i_lon) * 3;

			temp_fvertex_part[pos + 0] = (f_int32_t)((temp_fvertex[pos + 0]  - pNode->point_vertex_xyz[0]) * size);
			temp_fvertex_part[pos + 1] = (f_int32_t)((temp_fvertex[pos + 1]  - pNode->point_vertex_xyz[1]) * size);
			temp_fvertex_part[pos + 2] = (f_int32_t)((temp_fvertex[pos + 2]  - pNode->point_vertex_xyz[2]) * size);

	    }
	}

	/* 依次计算瓦片4条接边上各个顶点相对于基准点的偏移，并乘以放大倍数，转换为整形 */
	for(i = 0; i < 4; i++)
	{
	    for(j = 0; j < VTXCOUNT1; j++)	
	    {
	    	f_int32_t pos;

	    	switch(i)
	    	{
	    		case 0: /*上*/
	    		pos = ((VTXCOUNT1) * VTXCOUNT1 + j) * 3;	

	    		break;
	    		
	    		case 1: /*下*/
	    		pos = ((VTXCOUNT1 + 1) * VTXCOUNT1 + j) * 3;

	    		break;
	    		
	    		case 2: /*左*/
	    		pos = ((VTXCOUNT1 + 2) * VTXCOUNT1 + j) * 3;

	    		break;
	    		
	    		case 3: /*右*/
	    		pos = ((VTXCOUNT1 + 3) * VTXCOUNT1 + j) * 3;

	    		break;
	    	}

	
		temp_fvertex_part[pos + 0] = (f_int32_t)((temp_fvertex[pos + 0]  - pNode->point_vertex_xyz[0]) * size);
		temp_fvertex_part[pos + 1] = (f_int32_t)((temp_fvertex[pos + 1]  - pNode->point_vertex_xyz[1]) * size);
		temp_fvertex_part[pos + 2] = (f_int32_t)((temp_fvertex[pos + 2]  - pNode->point_vertex_xyz[2]) * size);

	    }
	}

	/* 俯视下每个瓦片只需用8个点，前4个点是瓦片的4个角点(高度为0)，后4个点是瓦片的4个角点(高度为-1000)，实际只用了前4个点 */
	/* 依次计算俯视下瓦片8个顶点相对于基准点的偏移，并乘以放大倍数，转换为整形 */
	{
		double x = 0.0, y =0.0, z=0.0;
		int i = 0;

		for(i = 0 ; i < 8; i ++)
		{
			x = pNode->fvertex_overlook[3 * i  + 0];
			y = pNode->fvertex_overlook[3 * i  + 1];
			z = pNode->fvertex_overlook[3 * i  + 2];
			
			pNode->fvertex_part_overlook[i].x = (f_int32_t)((x  - pNode->point_vertex_xyz[0]) * size);
			pNode->fvertex_part_overlook[i].y = (f_int32_t)((y  - pNode->point_vertex_xyz[1]) * size);
			pNode->fvertex_part_overlook[i].z = (f_int32_t)((z - pNode->point_vertex_xyz[2]) * size);
		}


	}



	return 0;
}


f_uint64_t qtmapnodeGetStatus(sQTMAPNODE *pNode, f_int32_t line)
{
	f_uint64_t ret;
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeGetStatus with error node %d\n", line);
		return 0;
	}
	takeSem(pNode->semlock, FOREVER_WAIT);
	ret = pNode->datastatus;
	giveSem(pNode->semlock);
	return ret;
}

f_uint64_t qtmapnodeSetStatus(sQTMAPNODE * pNode, f_uint64_t status, f_int32_t line)
{
	if (!ISMAPDATANODE(pNode))
	{
		printf("call qtmapnodeSetStatus with error node %d\n", line);
		return 0;
	}
	takeSem(pNode->semlock, FOREVER_WAIT);
	pNode->datastatus = status;
	giveSem(pNode->semlock);
	return status;
}

void geo2obj(f_float64_t lon, f_float64_t lat, f_float64_t ht,
                    f_float64_t *x, f_float64_t *y, f_float64_t *z)
{
	Geo_Pt_D geoPt;
	Obj_Pt_D objPt;
	geoPt.lon = lon;
	geoPt.lat = lat;
	geoPt.height = ht;
	if(geoDPt2objDPt(&geoPt, &objPt))
	{
		*x = objPt.x;
	    *y = objPt.y;
	    *z = objPt.z;
	}
	else
	{
		*x = 0.0f;
	    *y = 0.0f;
	    *z = 0.0f;
	}
}

/*返回内部链表中截止目前的节点数峰值，为调试用*/
f_int32_t qtmapGetNodePeakValue(VOIDPtr map_handle)
{
	f_int32_t ret;
	sMAPHANDLE * phandle;
	if (!ISMAPHANDLEVALID(map_handle))
		return -1;
	phandle = (sMAPHANDLE *)map_handle;
	takeSem(phandle->rendscenelock, FOREVER_WAIT);
	ret = phandle->nodecount;
	giveSem(phandle->rendscenelock);
	return ret;
}

/* 返回当前缓冲区数 */
f_int32_t qtmapGetBufferCount(VOIDPtr map_handle)
{
	f_int32_t ret;
	sMAPHANDLE * phandle;
	if (!ISMAPHANDLEVALID(map_handle))
		return -1;
	phandle = (sMAPHANDLE *)map_handle;
	takeSem(phandle->rendscenelock, FOREVER_WAIT);
	ret = phandle->loadedNumber - phandle->delNodeNumber;
	giveSem(phandle->rendscenelock);
	return ret;
}            

/**************************************************************************/
/*
** 创建新的瓦片节点 
** 
** 描述：
** 初始化所有地图数据，初始化地图引擎
** 
** 
** 参数 ：
** 输入参数：    pHandle      地图场景树句柄,全局唯一
**               level        瓦片节点起始层级
**               xidx         瓦片节点x方向索引
**               yidx         瓦片节点y方向索引
**               pSubTree     瓦片节点所在的子树指针
**               emptyOrload  是否将该瓦片节点数据加载到内存
**        
** 输出参数：    无
** 
** 输入输出参数： ppNode       瓦片节点指针的指针,用于返回指向新创建的瓦片节点的指针地址
** 
** 返回值：0 成功
**        -1 失败
** 
*/
/// <summary>
/// LOD每个级别的瓦片边长（度）
/// </summary>
static double TileSizeInDegree[] = 
{
	90.0f,              //level - 0
	45.0f,              //level - 1
	22.5f,              //level - 2
	11.25f,             //level - 3
	5.625f,             //level - 4
	2.8125f,            //level - 5
	1.40625f,           //level - 6
	0.703125f,          //level - 7
	0.3515625f,         //level - 8
	0.17578125f,        //level - 9
	0.087890625f,       //level - 10
	0.0439453125f,      //level - 11
	0.02197265625f,     //level - 12
	0.010986328125f,    //level - 13
	0.0054931640625f,   //level - 14
	0.00274658203125f,  //level - 15
	0.001373291015625f, //level - 16
	0.0006866455078125f //level - 17
};
// 列号-行号-层级
static void CalGeoCoordinateViaTileLoactionAndPixelLocation(int x, int y, int z,
	int m, int n, double* longitude, double* latitude)
{
	double mm = m / 256;
	double nn = n / 256;

	double currentTileWidth = TileSizeInDegree[z];

	*longitude = currentTileWidth * x + currentTileWidth * nn;
	*latitude = currentTileWidth * y + currentTileWidth * mm;

	//从左上角零零修正到中心点零零坐标系
	*longitude = *longitude - 180.0f;
	*latitude = 90.0f - *latitude;
}
f_int32_t qtmapnodeCreate(sMAPHANDLE *pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                          sQTMAPSUBTREE * pSubTree, sQTMAPNODE **ppNode, BOOL emptyOrload)
{
	/* 缓冲区中瓦片节点链表的头节点 */
	stList_Head *pstListHead = NULL;
	/* 用于遍历缓冲区中瓦片节点链表中的所有节点 */
	stList_Head *pstTmpList = NULL;
	/* 指向瓦片节点指针 */
	sQTMAPNODE  *pRet = NULL;
	/* 缓冲区中瓦片节点链表里有没有空置的节点,默认没有 */
	BOOL is_dnst_empty = FALSE;

	/* 初始化指向新创建的瓦片节点指针为空 */
	*ppNode = 0;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/* 查找缓冲区中瓦片节点链表里有没有空置的节点 */
	pstListHead = &(pHandle->nodehead.stListHead);
	/**正向遍历**/
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pRet = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
			/* 找到空置的瓦片节点,直接跳出循环 */
			if(DNST_EMPTY == qtmapnodeGetStatus(pRet, __LINE__))
			{
				is_dnst_empty = TRUE;
			    break;
			}			
		}
	}

	giveSem(pHandle->rendscenelock);
	/* 找到空置的瓦片节点,无需再创建新节点,直接使用该空置节点即可 */
	if(is_dnst_empty)
	{
		pRet->nodekey = MAPDATANODEKEY;
		pRet->createtime = tickCountGet();
		qtmapnodeSetStatus(pRet, DNST_EMPTY, __LINE__);			
	}	
	else  /* 没有找到空节点，那就需要创建一个新节点 */
	{
#if 1
#define MAXQTNODE_INRAM	  480//(320)//(160)	//单路是160
		/* 控制缓冲区中总瓦片节点数不超过设定的最大值，如果超过就不创建新节点 */
		if(qtmapGetBufferCount(pHandle) > MAXQTNODE_INRAM)
		{
			//printf("node num > max \n");
			return (-1);
		}
		else
		{
			/* NULL */
		}
		gMAXQTNODEINRAM = MAXQTNODE_INRAM;
#undef	MAXQTNODE_INRAM
#endif

		takeSem(pHandle->rendscenelock, FOREVER_WAIT);
		/* 申请瓦片节点内存,创建新节点 */
		pRet = (sQTMAPNODE *)NewAlterableMemory(sizeof(sQTMAPNODE));
		if (pRet == NULL)
			return -1;

		/* 缓冲区中瓦片节点数增加1 */
		pHandle->nodecount++;

		/* 设置新创建瓦片节点的key、创建时间、信号量 */
		memset(pRet, 0, sizeof(sQTMAPNODE));	
		pRet->nodekey = MAPDATANODEKEY;
		pRet->createtime = tickCountGet();
		pRet->semlock = createBSem(Q_FIFO_SEM, FULL_SEM);
		giveSem(pRet->semlock);

		/* 设置新创建瓦片节点的数据状态为空置 */
		qtmapnodeSetStatus(pRet, DNST_EMPTY, __LINE__);	

		/*将新创建瓦片节点插到链表队首*/
#if 1
		{
			stList_Head *pstListHead = NULL;
			pstListHead = &(pHandle->nodehead.stListHead);
			LIST_ADD(&pRet->stListHead, pstListHead);
		}	
#else
		{
			int i = 0;
			// 将新创建瓦片节点插到链表的最后一位
			stList_Head *pstListHead = NULL;
			pstListHead = &(pHandle->nodehead.stListHead);
			while(1)
			{
				if(&(pHandle->nodehead.stListHead) == pstListHead->pNext)
					break;
				else
					pstListHead = pstListHead->pNext;i++;
			}
			///printf("链表中的位置 = %d\n",i);
			LIST_ADD(&pRet->stListHead, pstListHead);
		}
		#endif		
		
		giveSem(pHandle->rendscenelock);
		
	}
	
	/*设置瓦片节点的具体参数*/
	pRet->level = level;
	pRet->xidx = xidx;
	pRet->yidx = yidx;
	pRet->texid = 0;
	pRet->vtexid = 0;
	pRet->pSubTree = pSubTree;
	pRet->parent = NULL;
	pRet->children[0]= NULL;
	pRet->children[1]= NULL;
	pRet->children[2]= NULL;
	pRet->children[3]= NULL;
	pRet->drawnumber = 0;
	pRet->lastdraw   = 0;
	
	/* 读取节点的位置信息并更新到瓦片节点结构体中 */
	#ifdef USE_FAULT_Tile
	{
		double lonMin,lonMax,latMin,latMax=0.0f;
		double lon_leftUp=0.0f,lat_LeftUp=0.0f,lon_rightBottom=0.0,lat_rightBottom=0.0;
		f_float32_t nodeinfo[9];
		
		// 计算瓦块四至 
		CalGeoCoordinateViaTileLoactionAndPixelLocation(pRet->xidx,pRet->yidx,pRet->level,0,0,&lon_leftUp,&lat_LeftUp);
		CalGeoCoordinateViaTileLoactionAndPixelLocation(pRet->xidx,pRet->yidx,pRet->level,256,256,&lon_rightBottom,&lat_rightBottom);
		lonMin = lon_leftUp;lonMax = lon_rightBottom;
		latMin = lat_rightBottom;latMax = lat_LeftUp;

		/* 对数据进行大小端转换 */
#ifdef _LITTLE_ENDIAN_
		pRet->hstart = lonMin;//nodeinfo[0];
		pRet->vstart = latMin;//nodeinfo[1];
		pRet->hscale = TileSizeInDegree[pRet->level];//nodeinfo[2];
		pRet->vscale = TileSizeInDegree[pRet->level];//nodeinfo[3];
		
		pRet->height_lt = 0;//nodeinfo[4];
		pRet->height_lb = 0;//nodeinfo[5];
		pRet->height_rt = 0;//nodeinfo[6];
		pRet->height_rb = 0;//nodeinfo[7];
		pRet->height_ct = 0;//nodeinfo[8];	
#else					              
		pRet->hstart    = SWAPFLOAT(lonMin);
		pRet->vstart    = SWAPFLOAT(latMin);
		pRet->hscale    = SWAPFLOAT(TileSizeInDegree[pRet->level]);
		pRet->vscale    = SWAPFLOAT(TileSizeInDegree[pRet->level]);
		
		pRet->height_lt = SWAPFLOAT(0);
		pRet->height_lb = SWAPFLOAT(0);
		pRet->height_rt = SWAPFLOAT(0);
		pRet->height_rb = SWAPFLOAT(0);
		pRet->height_ct = SWAPFLOAT(0);
#endif

		/* 计算上述5个点的物体坐标 */
		geo2obj(pRet->hstart + pRet->hscale * 0.5, pRet->vstart + pRet->vscale * 0.5, pRet->height_ct, 
		        &(pRet->obj_cent.x), &(pRet->obj_cent.y), &(pRet->obj_cent.z)); 
		geo2obj(pRet->hstart, pRet->vstart + pRet->vscale, pRet->height_lt, 
		        &(pRet->obj_lt.x), &(pRet->obj_lt.y), &(pRet->obj_lt.z));	
        geo2obj(pRet->hstart + pRet->hscale, pRet->vstart + pRet->vscale, pRet->height_rt, 
                &(pRet->obj_rt.x), &(pRet->obj_rt.y), &(pRet->obj_rt.z));		
        geo2obj(pRet->hstart + pRet->hscale, pRet->vstart, pRet->height_rb, 
                &(pRet->obj_rb.x), &(pRet->obj_rb.y), &(pRet->obj_rb.z));	
		geo2obj(pRet->hstart, pRet->vstart, pRet->height_lb, 
		        &(pRet->obj_lb.x), &(pRet->obj_lb.y), &(pRet->obj_lb.z));				          	          	          	          
	}
	#else
	{
		/* 顺序是：左下角经度、左下角纬度、经度跨度、纬度跨度、
		           左上角高度、左下角高度、右上角高度、右下角高度、中心点高度 */
		f_float32_t nodeinfo[9];

		/* 调用loadinfofile函数,真正从info文件中读取数据 */
		pRet->pSubTree->infoloader(pRet->level, pRet->xidx, pRet->yidx, 
					               pRet->pSubTree->infoparam, &nodeinfo[0]);
		/* 对数据进行大小端转换 */
#ifdef _LITTLE_ENDIAN_
		pRet->hstart = nodeinfo[0];
		pRet->vstart = nodeinfo[1];
		pRet->hscale = nodeinfo[2];
		pRet->vscale = nodeinfo[3];
		
		pRet->height_lt = nodeinfo[4];
		pRet->height_lb = nodeinfo[5];
		pRet->height_rt = nodeinfo[6];
		pRet->height_rb = nodeinfo[7];
		pRet->height_ct = nodeinfo[8];		
#else					              

		pRet->hstart    = SWAPFLOAT(nodeinfo[0]);
		pRet->vstart    = SWAPFLOAT(nodeinfo[1]);
		pRet->hscale    = SWAPFLOAT(nodeinfo[2]);
		pRet->vscale    = SWAPFLOAT(nodeinfo[3]);
		
		pRet->height_lt = SWAPFLOAT(nodeinfo[4]);
		pRet->height_lb = SWAPFLOAT(nodeinfo[5]);
		pRet->height_rt = SWAPFLOAT(nodeinfo[6]);
		pRet->height_rb = SWAPFLOAT(nodeinfo[7]);
		pRet->height_ct = SWAPFLOAT(nodeinfo[8]);
#endif

		/* 计算上述5个点的物体坐标 */
		geo2obj(pRet->hstart + pRet->hscale * 0.5, pRet->vstart + pRet->vscale * 0.5, pRet->height_ct, 
		        &(pRet->obj_cent.x), &(pRet->obj_cent.y), &(pRet->obj_cent.z)); 
		geo2obj(pRet->hstart, pRet->vstart + pRet->vscale, pRet->height_lt, 
		        &(pRet->obj_lt.x), &(pRet->obj_lt.y), &(pRet->obj_lt.z));	
        geo2obj(pRet->hstart + pRet->hscale, pRet->vstart + pRet->vscale, pRet->height_rt, 
                &(pRet->obj_rt.x), &(pRet->obj_rt.y), &(pRet->obj_rt.z));		
        geo2obj(pRet->hstart + pRet->hscale, pRet->vstart, pRet->height_rb, 
                &(pRet->obj_rb.x), &(pRet->obj_rb.y), &(pRet->obj_rb.z));	
		geo2obj(pRet->hstart, pRet->vstart, pRet->height_lb, 
		        &(pRet->obj_lb.x), &(pRet->obj_lb.y), &(pRet->obj_lb.z));				          	          	          	          
	}
	#endif


	/* 释放信号量,告知三维数据加载任务有新创建的瓦片节点 */
    giveSem(pHandle->anyrequire);

	/* 将新创建的瓦片节点返回 */
	*ppNode = pRet;
	
	if(emptyOrload == TRUE)
	{
		/* 当新创建的瓦片节点暂时不加载数据时,节点状态置为SLEEP , 再次加载时改变状态 */
		qtmapnodeSetStatus(pRet, DNST_SLEEP, __LINE__);	
	}
	else
	{
		/* 当新创建的瓦片节点需要加载数据时,节点状态置为DNST_WAITFORLOAD, 告知三维数据加载任务加载该节点的数据 */
		qtmapnodeSetStatus(pRet, DNST_WAITFORLOAD, __LINE__);
	}
        
	return 0;
}


/*
功能：设置瓦片各个顶点的多重纹理坐标及顶点坐标
输入：
	pNode     瓦片节点指针
	pTex      瓦片各绘制顶点的纹理坐标
	ind       瓦片各绘制顶点的索引值

输入输出：
	无

输出：
    无

返回值：
    无
*/
static void qtmapSetPoint(sQTMAPNODE * pNode, f_float32_t* pTex, f_int32_t ind)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#if 1
	/* 采用多重纹理,设置纹理0对应的各个绘制顶点的纹理坐标,纹理0为卫星影像纹理或地貌晕渲纹理 */
	glMultiTexCoord2f(GL_TEXTURE0, pTex[2*ind], pTex[2*ind+1]);		

	/* 采用多重纹理,设置纹理1对应的各个绘制顶点的纹理坐标,纹理1为地形告警纹理 */
#if 1				
	glMultiTexCoord1f(GL_TEXTURE1, pNode->fterrain[ind]);            /* 地形告警用一维纹理 */		
#else
    glMultiTexCoord2f(GL_TEXTURE1, pNode->fterrain[ind], 1.0f);		/* 地形告警用二维纹理 */		
#endif

	/* 采用多重纹理,设置纹理2对应的各个绘制顶点的纹理坐标,纹理2为矢量纹理或其他纹理,暂时未使用 */
	//glMultiTexCoord2f(GL_TEXTURE2, pTex[2*ind], pTex[2*ind+1]);		

	/* 设置各个绘制顶点的顶点坐标,使用整型化后的顶点值 */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);

#else
	/* 采用单一纹理,直接设置各个顶点的纹理坐标及顶点坐标 */
	glTexCoord2f(pTex[2*ind], pTex[2*ind+1]);	//一重纹理	
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
	
#endif
#endif
}


/*
功能：创建瓦片俯视下绘制的显示列表(仅绘制瓦片的4个顶点、四个接边四边形)
输入：
	pNode                瓦片节点指针
	is_need_terwarn      是否开启高度预警
	scene_mode           场景模式

输入输出：
	无

输出：
    无

返回值：
    0   显示列表创建成功或之前已创建
	-1  显示列表创建失败
*/
f_int32_t qtmapNodeCreateListOverlook(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 如果瓦片俯视下绘制的显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->tileListOverlook) != 0)
	{
		return 0;
	}

	/* 创建瓦片俯视下绘制的显示列表 */
	pNode->tileListOverlook = glGenLists(1);
	if((pNode->tileListOverlook) == 0)
	{
		printf("create tileListOverlook failed \n");
		return -1;
	}
	
     count = 4;
     pIndex = gindices;
     pTex = gTextureBuf;

	glNewList(pNode->tileListOverlook, GL_COMPILE);
	
	// 断层模式,纹理坐标从父纹理计算
	#ifdef USE_FAULT_Tile
	{
		// 使用两个三角形绘制
		{
			float startTxtX=0.0, endTxtX=1.0;
			float startTxtY=0.0, endTxtY=1.0;
			
			// 计算纹理坐标
			{
				// 使用了父纹理,计算在父纹理中的位置
				if (pNode->tqsInFile == 0)
				{
					sQTMAPNODE* parentNode = pNode->tqsParentInFile;
					if(parentNode != NULL)
					{
						int childZoomScale = pow(2,pNode->level - parentNode->level);	// 一行子瓦块放大倍数(同时代表了当前层一行的子瓦块数)
						float txtStep = 1.0f/(childZoomScale*1.0f);						// 纹理步长
						int childStartX = parentNode->xidx * childZoomScale;			// 当前层起始子瓦块号
						int childStartY = parentNode->yidx * childZoomScale;
						startTxtX = (pNode->xidx - childStartX)*txtStep;
						endTxtX = startTxtX+txtStep;
						startTxtY = (pNode->yidx - childStartY)*txtStep;
						endTxtY = startTxtY+txtStep;
					}
					else
						printf("[qtmapNodeCreateListOverlook] Error! Can't Find ParentNode. \n");
				} 
			}

			/* 绘制第一个三角形 */
			{
				glBegin(GL_TRIANGLES);
				/* 多重纹理,设置纹理0第一个顶点的纹理坐标,顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始 */
				glMultiTexCoord2f(GL_TEXTURE0, startTxtX, endTxtY);	// 纹理左上角		
				/* 设置第一个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

				/* 多重纹理,设置纹理0第二个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第二个顶点使用右上角的顶点,纹理右下角 */
				glMultiTexCoord2f(GL_TEXTURE0, endTxtX, startTxtY);	// 纹理右下角
				/* 设置第二个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

				/* 多重纹理,设置纹理0第三个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第三个顶点使用右下角的顶点,纹理右上角 */
				glMultiTexCoord2f(GL_TEXTURE0, endTxtX, endTxtY);			// 纹理右上角
				/* 设置第三个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[1].x, 
					pNode->fvertex_part_overlook[1].y, 
					pNode->fvertex_part_overlook[1].z);

				glEnd();		
			}

			/* 绘制第二个三角形 */
			{
				glBegin(GL_TRIANGLES);
				/* 多重纹理,设置纹理0第一个顶点的纹理坐标,顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始 */
				glMultiTexCoord2f(GL_TEXTURE0, startTxtX, endTxtY);	// 纹理左上角
				/* 设置第一个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

				/* 多重纹理,设置纹理0第二个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第二个顶点使用左上角的顶点 */
				glMultiTexCoord2f(GL_TEXTURE0, startTxtX, startTxtY);// 纹理左下角	
				/* 设置第二个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[3].x, 
					pNode->fvertex_part_overlook[3].y, 
					pNode->fvertex_part_overlook[3].z);

				/* 多重纹理,设置纹理0第三个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第三个顶点使用右上角的顶点 */
				glMultiTexCoord2f(GL_TEXTURE0, endTxtX, startTxtY);			// 纹理右下角
				/* 设置第三个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);
				glEnd();
			}
		}
	}
	#else
	{
		#if 1

		#if 0
			/* 瓦片用1个四边形来进行绘制 */
			glBegin(GL_QUADS);
		//	for(i=0; i<count; i++)
			{
	
		//		glTexCoord2f(0.0, 1.0);		

				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);		
				glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);			

				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

		//		glTexCoord2f(1.0, 1.0);		

				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);		
				glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0);	

				glVertex3i(
					pNode->fvertex_part_overlook[1].x, 
					pNode->fvertex_part_overlook[1].y, 
					pNode->fvertex_part_overlook[1].z);
		
		//		glTexCoord2f(1.0, 0.0);	
		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);		
				glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);			
		
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

		
		//		glTexCoord2f(0.0, 0.0 );		

				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);		
				glMultiTexCoord2f(GL_TEXTURE2, 0.0, 0.0);	

				glVertex3i(
					pNode->fvertex_part_overlook[3].x, 
					pNode->fvertex_part_overlook[3].y, 
					pNode->fvertex_part_overlook[3].z);
		
			}
			glEnd();

		#else

			/* 瓦片用2个三角形来进行绘制，组成一个四边形 */
			/* 绘制第一个三角形 */
			glBegin(GL_TRIANGLES);
				/* 不启用多重纹理,设置第一个顶点的纹理坐标，目前启用了多重纹理 */
				//glTexCoord2f(0.0, 1.0);	
				/* 多重纹理,设置纹理0第一个顶点的纹理坐标,顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始 */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);		
				/* 多重纹理,设置纹理2第一个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);		
				/* 设置第一个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

				/* 不启用多重纹理,设置第二个顶点的纹理坐标，目前启用了多重纹理 */
				//glTexCoord2f(1.0, 0.0);	
				/* 多重纹理,设置纹理0第二个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第二个顶点使用右上角的顶点 */
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);	
				/* 多重纹理,设置纹理2第二个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);
				/* 设置第二个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

				/* 不启用多重纹理,设置第三个顶点的纹理坐标，目前启用了多重纹理 */
				//glTexCoord2f(1.0, 1.0);	
				/* 多重纹理,设置纹理0第三个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第三个顶点使用右下角的顶点 */
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);
				/* 多重纹理,设置纹理2第三个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0);	
				/* 设置第三个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[1].x, 
					pNode->fvertex_part_overlook[1].y, 
					pNode->fvertex_part_overlook[1].z);
		
			glEnd();		

			/* 绘制第二个三角形 */
			glBegin(GL_TRIANGLES);
				/* 多重纹理,设置纹理0第一个顶点的纹理坐标,顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始 */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);
				/* 多重纹理,设置纹理2第一个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);			
				/* 设置第一个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);
		
				/* 多重纹理,设置纹理0第二个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第二个顶点使用左上角的顶点 */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);	
				/* 多重纹理,设置纹理2第二个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 0.0);	
				/* 设置第二个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[3].x, 
					pNode->fvertex_part_overlook[3].y, 
					pNode->fvertex_part_overlook[3].z);

				/* 多重纹理,设置纹理0第三个顶点的纹理坐标,为了保证三角形顶点顺时针方向为正面,第三个顶点使用右上角的顶点 */
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);
				/* 多重纹理,设置纹理2第三个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);			
				/* 设置第三个顶点的整型顶点坐标 */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

			glEnd();

		#endif

			/* 绘制瓦片上下左右四条接边,用四边形条带绘制 */
			glBegin(GL_QUAD_STRIP);
			{
				int k = 0;
				f_float32_t dx = 1.0f / VTXCOUNT;
		
				/* 不启用多重纹理,设置第一个顶点的纹理坐标，目前启用了多重纹理 */
				//glTexCoord2f(0.0, 1.0);		
				/* 多重纹理,设置纹理0第一个顶点(瓦片左下角)的纹理坐标,顶点索引顺序是从左下角开始,而纹理坐标是从左上角开始 */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);
				/* 多重纹理,设置纹理2第一个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);	
				/* 设置第一个顶点的整型顶点坐标 */
				k = 0;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 不启用多重纹理,设置第二个顶点的纹理坐标，目前启用了多重纹理 */
				//glTexCoord2f(0.0, 1.0 - dx);		
				/* 多重纹理,设置纹理0第二个顶点(瓦片下接边的左顶点)的纹理坐标,纹理坐标采用瓦片内部倒数第二行顶点的纹理坐标 */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0 - dx);	
				/* 多重纹理,设置纹理2第二个顶点的纹理坐标，目前未使用纹理2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0 - dx);	
				/* 设置第二个顶点的整型顶点坐标 */
				k = 4;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第三个顶点(瓦片右下角)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(1.0, 1.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0);	
				k = 1;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第四个顶点(瓦片下接边的右顶点)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(1.0, 1.0 - dx);	
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0 - dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0 - dx);	
				k = 5;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

		#if 1
				/* 设置第五个顶点(瓦片右上角)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(1.0, 0.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);	
				k = 2;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第六个顶点(瓦片右接边的上顶点)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(1.0, dx);		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, dx);	
				k = 6;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第七个顶点(瓦片左上角)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(0.0, 0.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 0.0);	
				k = 3;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第八个顶点(瓦片上接边的左顶点)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(0.0, dx);		
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, dx);	
				k = 7;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第九个顶点(瓦片左下角)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(0.0, 1.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);	
				k = 0;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* 设置第十个顶点(瓦片左接边的下顶点)的纹理0坐标、整型顶点坐标 */
				//glTexCoord2f(0.0, 1.0-dx);	
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0-dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0-dx);	
				k = 4;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);
		#endif
		
		
			}
			glEnd();

	
		#else
			/* 如果是景嘉提供的M9驱动,可以用这个函数创建显示列表,速率更快 */
			gljDrawTriangleByStrip(GL_TRIANGLE_STRIP, count, pIndex, 
					3,pNode->fvertex_part, 
					2,pTex, 
					1,pNode->fterrain );
		#endif
			}
		#endif


	glEndList();
#endif

	return 0;
}



/*
功能：创建瓦片的正常绘制显示列表(绘制所有瓦片顶点)
输入：
	pNode                瓦片节点指针
	is_need_terwarn      是否开启高度预警
	scene_mode           场景模式

输入输出：
	无

输出：
    无

返回值：
    0   显示列表创建成功或之前已创建
	-1  显示列表创建失败
*/
f_int32_t qtmapNodeCreateList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* 如果瓦片的正常绘制显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->tileList) != 0)
	{
		return 0;
	}

	/* 创建瓦片的正常绘制显示列表 */
	pNode->tileList = glGenLists(1);
	if((pNode->tileList) == 0)
	{
		printf("create list failed \n");
		return -1;
	}
	
	/* 判断当前渲染场景的场景模式 */
	switch(scene_mode)
	{
		/* 如果是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* 如果是鹰眼模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针及顶点纹理坐标数组指针 */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* 默认为正常模式 */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	glNewList(pNode->tileList, GL_COMPILE);
#if 1
	/* 瓦片用三角形条带来进行绘制 */
	glBegin(GL_TRIANGLE_STRIP);
	/* 设置瓦片绘制时各个绘制顶点的纹理和索引值 */
	for(i=0; i<count; i++)
	{
		qtmapSetPoint(pNode, pTex, pIndex[i]);
	}
	glEnd();
#else
	gljDrawTriangleByStrip(GL_TRIANGLE_STRIP, count, pIndex, 
			3,pNode->fvertex_part, 
			2,pTex, 
			1,pNode->fterrain );
#endif
	glEndList();

	return 0;
}


/*
功能：创建瓦片的座舱视角绘制VBO(顶点坐标/顶点高度)和俯视视角绘制的VBO(顶点坐标，即4个角点的坐标)，每个瓦片的VBO中的数据不一样
输入：
	pNode                瓦片节点指针

输入输出：
	无

输出：
    无

返回值：
    0   VBO创建成功或之前已创建
*/
f_int32_t qtmapNodeCreateVboEs(sQTMAPNODE * pNode)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	int i=0;
	//创建瓦片的俯视视角绘制的VBO(顶点坐标，即4个角点的坐标)
	if (0 == pNode->tileOverlookVertexVBO)
	{
		glGenBuffers(1, &(pNode->tileOverlookVertexVBO));
		/*只取瓦片中高度为0的四个角点*/
		{
#ifdef VERTEX_USE_FLOAT
			f_float32_t verCoord[12] = { 0 };
			for (i = 0; i < 4; i++)
			{
				/*使用浮点型的顶点数据*/
				verCoord[3 * i] = pNode->fvertex_overlook[3 * i + 0];
				verCoord[3 * i + 1] = pNode->fvertex_overlook[3 * i + 1];
				verCoord[3 * i + 2] = pNode->fvertex_overlook[3 * i + 2];
			}

			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
			glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(f_float32_t), verCoord, GL_STATIC_DRAW);
#else

			f_int32_t verCoord[12] = { 0 };
			for (i = 0; i < 4; i++)
			{
				/*使用整形的顶点数据*/
				verCoord[3 * i] = pNode->fvertex_part_overlook[i].x;
				verCoord[3 * i + 1] = pNode->fvertex_part_overlook[i].y;
				verCoord[3 * i + 2] = pNode->fvertex_part_overlook[i].z;
			}

			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
			glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(f_int32_t), verCoord, GL_STATIC_DRAW);
#endif
		}
	}

	//创建瓦片的座舱视角绘制VBO(顶点坐标)
	if (0 == pNode->tileVertexVBO)
	{
		glGenBuffers(1, &pNode->tileVertexVBO);
		/*存放瓦片的各个顶点坐标*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)* vertexNum, pNode->fvertex, GL_STATIC_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLint)* vertexNum, pNode->fvertex_part, GL_STATIC_DRAW);
#endif
	}
	//创建瓦片的座舱视角绘制VBO(顶点高度)
	if (0 == pNode->tileTerrainVBO)
	{
		glGenBuffers(1, &pNode->tileTerrainVBO);
		/*存放瓦片的各个顶点高度，用于生成告警纹理*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* vertexNum, pNode->fterrain, GL_STATIC_DRAW);
	}
#endif
	return 0;
}



/*
功能：创建瓦片的绘制VBO(顶点坐标/顶点高度/顶点颜色/顶点法线/顶点方里网纹理)，每个瓦片的VBO中的数据不一样，SVS模式所有视角都绘制33*33个顶点
输入：
	pNode                瓦片节点指针

输入输出：
	无

输出：
    无

返回值：
    0   VBO创建成功或之前已创建
*/
f_int32_t qtmapNodeCreateSvsVboEs(sQTMAPNODE * pNode)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	int i=0;

	//创建瓦片的绘制VBO(顶点坐标)
	if (0 == pNode->tileVertexVBO)
	{
		glGenBuffers(1, &pNode->tileVertexVBO);
		/*存放瓦片的各个顶点坐标*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)* vertexNum, pNode->fvertex, GL_STATIC_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLint)* vertexNum, pNode->fvertex_part, GL_STATIC_DRAW);
#endif
	}
	//创建瓦片的绘制VBO(顶点高度)
	if (0 == pNode->tileTerrainVBO)
	{
		glGenBuffers(1, &pNode->tileTerrainVBO);
		/*存放瓦片的各个顶点高度，用于生成告警纹理*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* vertexNum, pNode->fterrain, GL_STATIC_DRAW);
	}
	//创建瓦片的绘制VBO(顶点颜色)
	if (0 == pNode->tileColorVBO)
	{
		glGenBuffers(1, &pNode->tileColorVBO);
		/*存放瓦片的各个顶点坐标*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileColorVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)* vertexNum, pNode->m_pColor, GL_STATIC_DRAW);
	}
	//创建瓦片的绘制VBO(顶点法线)
	if (0 == pNode->tileNormalVBO)
	{
		glGenBuffers(1, &pNode->tileNormalVBO);
		/*存放瓦片的各个顶点坐标*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileNormalVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLbyte)* vertexNum, pNode->pNormal, GL_STATIC_DRAW);
	}
	//创建瓦片的绘制VBO(顶点方里网纹理)
	if (0 == pNode->tileGridTexVBO)
	{
		glGenBuffers(1, &pNode->tileGridTexVBO);
		/*存放瓦片的各个顶点坐标*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileGridTexVBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* vertexNum, pNode->m_pVertexTex, GL_STATIC_DRAW);
	}
#endif
	return 0;
}


/*
功能：创建瓦片的座舱视角绘制VBO(顶点索引/顶点纹理坐标)和俯视视角绘制的VBO(顶点索引/顶点纹理坐标)，所有瓦片的这些VBO中的数据均一样，故不放到结构体sQTMAPNODE中定义，
      用静态变量存储即可，该函数在正常模式下使用。
输入：
	无

输入输出：
	pibo                座舱视角瓦片顶点索引VBO指针
	ptvbo               座舱视角瓦片顶点纹理坐标VBO指针
	ptvbo_overlook      俯视视角瓦片顶点索引VBO指针
	pibo_overlook       俯视视角瓦片顶点纹理坐标VBO指针

输出：
    无

返回值：
    0   VBO创建成功或之前已创建
*/
f_int32_t genConstVBOs(unsigned int* pibo, unsigned int* ptvbo, unsigned int* ptvbo_overlook, unsigned int* pibo_overlook)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = index_count;
	f_uint32_t* pIndex = gindices;
	f_float32_t* pTex = gTextureBuf;

	GLuint ibo = *pibo;
	GLuint tvbo = *ptvbo;
	GLuint tvbo_overlook = *ptvbo_overlook;
	GLuint ibo_overlook = *pibo_overlook;

	/*座舱视角瓦片顶点纹理坐标VBO*/
	if (tvbo == 0)
	{
		glGenBuffers(1, &tvbo);
		glBindBuffer(GL_ARRAY_BUFFER, tvbo);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* vertexNum, pTex, GL_STATIC_DRAW);

		*ptvbo = tvbo;
	}
	/*座舱视角瓦片顶点索引VBO*/
	if (ibo == 0)
	{
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* count, pIndex, GL_STATIC_DRAW);

		*pibo = ibo;
	}
	/*俯视视角瓦片顶点纹理坐标VBO*/
	if (tvbo_overlook == 0)
	{
		f_float32_t texCoord[8] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glGenBuffers(1, &tvbo_overlook);
		glBindBuffer(GL_ARRAY_BUFFER, tvbo_overlook);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* 4, texCoord, GL_STATIC_DRAW);

		*ptvbo_overlook = tvbo_overlook;
	}
	/*俯视视角瓦片顶点索引VBO*/
	if (ibo_overlook == 0)
	{
		f_uint16_t indices[6] = { 0, 1, 2, 0, 2, 3 };
		glGenBuffers(1, &ibo_overlook);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_overlook);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)* 6, indices, GL_STATIC_DRAW);

		*pibo_overlook = ibo_overlook;
	}
#endif
	return 0;
}


/*
功能：创建瓦片的顶点绘制VBO(顶点索引)，所有瓦片的这些VBO中的数据均一样，故不放到结构体sQTMAPNODE中定义，
      用静态变量存储即可，该函数在SVS模式下使用。
输入：
	无

输入输出：
	pibo                瓦片顶点索引VBO指针

输出：
    无

返回值：
    0   VBO创建成功或之前已创建
*/
f_int32_t genSvsConstVBOs(unsigned int* pibo)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = index_count;
	f_uint32_t* pIndex = gindices;

	GLuint ibo = *pibo;

	/*瓦片顶点索引VBO*/
	if (ibo == 0)
	{
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* count, pIndex, GL_STATIC_DRAW);

		*pibo = ibo;
	}
#endif	
	return 0;
}

/*
功能：创建瓦片的顶点绘制VBO(顶点索引/顶点纹理坐标)，所有瓦片的这些VBO中的数据均一样，故不放到结构体sQTMAPNODE中定义，
      用静态变量存储即可，该函数在VecShade模式下使用。
输入：
	无

输入输出：
	pibo                瓦片顶点索引VBO指针

输出：
    无

返回值：
    0   VBO创建成功或之前已创建
*/
f_int32_t genVecShadeConstVBOs(unsigned int* pibo, unsigned int* ptvbo)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = index_count;
	f_uint32_t* pIndex = gindices;
	f_float32_t* pTex = gTextureBuf;

	GLuint ibo = *pibo;
		GLuint tvbo = *ptvbo;

	/*瓦片顶点索引VBO*/
	if (ibo == 0)
	{
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* count, pIndex, GL_STATIC_DRAW);

		*pibo = ibo;
	}

	/*瓦片顶点纹理坐标VBO*/
	if (tvbo == 0)
	{
		glGenBuffers(1, &tvbo);
		glBindBuffer(GL_ARRAY_BUFFER, tvbo);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* vertexNum, pTex, GL_STATIC_DRAW);

		*ptvbo = tvbo;
	}
#endif
	return 0;
}

/*
功能：创建瓦片俯视下绘制时边线的绘制显示列表
输入：
	pNode                瓦片节点指针

输入输出：
	无

输出：
    无

返回值：
    0   显示列表创建成功或之前已创建
	-1  显示列表创建失败
*/
f_int32_t qtmapNodeCreateEdgeListOverlook(sQTMAPNODE * pNode)
{
	/* 如果瓦片俯视下的边线绘制显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->edgeListOverlook) != 0)
	{
		return 0;
	}

	/* 创建瓦片俯视下的边线绘制显示列表 */	
	pNode->edgeListOverlook = glGenLists(1);
	if((pNode->edgeListOverlook) == 0)
	{
		return -1;
	}
		
	glNewList(pNode->edgeListOverlook, GL_COMPILE);
	/* 依次绘制四条边线,总共4个点 */
	glBegin(GL_LINE_STRIP);

		glVertex3i(
			pNode->fvertex_part_overlook[0].x, 
			pNode->fvertex_part_overlook[0].y, 
			pNode->fvertex_part_overlook[0].z);

		glVertex3i(
			pNode->fvertex_part_overlook[1].x, 
			pNode->fvertex_part_overlook[1].y, 
			pNode->fvertex_part_overlook[1].z);
		
		glVertex3i(
			pNode->fvertex_part_overlook[2].x, 
			pNode->fvertex_part_overlook[2].y, 
			pNode->fvertex_part_overlook[2].z);

		glVertex3i(
			pNode->fvertex_part_overlook[3].x, 
			pNode->fvertex_part_overlook[3].y, 
			pNode->fvertex_part_overlook[3].z);
		
	glEnd();

	glEndList();

	return 0;
}


/*
功能：创建瓦片正常绘制时边线的绘制显示列表
输入：
	pNode                瓦片节点指针

输入输出：
	无

输出：
    无

返回值：
    0   显示列表创建成功或之前已创建
	-1  显示列表创建失败
*/
f_int32_t qtmapNodeCreateEdgeList(sQTMAPNODE * pNode)
{
	f_int32_t i, j, index0/*, index1*/;
	
	/* 如果瓦片的边线绘制显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->edgeList) != 0)
	{
		return 0;
	}
	
	/* 创建瓦片的边线绘制显示列表 */
	pNode->edgeList = glGenLists(1);
	if((pNode->edgeList) == 0)
	{
		return -1;
	}
		
	glNewList(pNode->edgeList, GL_COMPILE);
	/* 依次绘制四条边线,每条边线33个点 */
	for(i=0; i<4; i++)
	{
		glBegin(GL_LINE_STRIP);
		for(j = 0; j <= VTXCOUNT; j++)
		{
			switch(i)
			{
			case 0:	
				index0 = j;
				//index1 = j + 1;
				break;
			case 1:
				index0 = j + VTXCOUNT*(VTXCOUNT+1);
				//index1 = j + 1 + VTXCOUNT*(VTXCOUNT+1);
				break;
			case 2:
				index0 = j * (VTXCOUNT+1);
				//index1 = (j + 1) * (VTXCOUNT+1);
				break;
			case 3:
				index0 = j*(VTXCOUNT+1) + VTXCOUNT;
				//index1 = (j+1)*(VTXCOUNT+1) + VTXCOUNT;
				break;
			}
			index0 *= 3;
			//index1 *= 3;

			/* 设置各个顶点,采用整型 */
			glVertex3i(pNode->fvertex_part[index0], pNode->fvertex_part[index0+1], pNode->fvertex_part[index0+2]);
			//glVertex3i(pNode->fvertex_part[index1], pNode->fvertex_part[index1+1], pNode->fvertex_part[index1+2]);

			//glVertex3f(pNode->fvertex[index0], pNode->fvertex[index0+1], pNode->fvertex[index0+2] + 10.0f);
			//glVertex3f(pNode->fvertex[index1], pNode->fvertex[index1+1], pNode->fvertex[index1+2] + 10.0f);
		}
		glEnd();
	}		
		
	glEndList();

	return 0;
}


/*设置vecShade模式下的显示列表中多重纹理坐标和顶点坐标*/
static void qtmapSetVecShadePoint(sQTMAPNODE * pNode, f_int32_t ind)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* JM7200最多支持二重纹理，故采用二重纹理，GL_TEXTURE0对应vecShade， GL_TEXTURE1对应告警纹理*/
	/* 采用多重纹理,设置纹理0对应的各个绘制顶点的纹理坐标,纹理0为vecShade纹理 */
#if 0				
	glMultiTexCoord1f(GL_TEXTURE0, pNode->fterrain[ind]);            /* vecShade用一维纹理 */		
#else
	glMultiTexCoord2f(GL_TEXTURE0, pNode->fterrain[ind], 1.0f);		/* vecShade用二维纹理 */		
#endif

	/* 采用多重纹理,设置纹理1对应的各个绘制顶点的纹理坐标,纹理1为地形告警纹理 */
#if 0				
	glMultiTexCoord1f(GL_TEXTURE1, pNode->fterrain[ind]);            /* 地形告警用一维纹理 */		
#else
	glMultiTexCoord2f(GL_TEXTURE1, pNode->fterrain[ind], 1.0f);		/* 地形告警用二维纹理 */		
#endif

	/* 设置各个绘制顶点的顶点坐标,使用整型化后的顶点值 */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
#endif
}

/*创建vecShade模式下各个视角下的显示列表*/
f_int32_t qtmapNodeCreateVecShadeList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;

	/* 如果瓦片的vecShade模式的显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->tileVecShadeList) != 0)
	{
		return 0;
	}
	
	/* 创建vecShade的显示列表，所有视角下用同一个显示列表 */
	pNode->tileVecShadeList = glGenLists(1);
	if((pNode->tileVecShadeList) == 0)
	{
		printf("create list failed \n");
		return -1;
	}
	else
	{
		//printf("create list succeed \n");
	}

	/* 默认是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针。鹰眼模式后续考虑。 */
	count = index_count;
	pIndex = gindices;

	glNewList(pNode->tileVecShadeList, GL_COMPILE);
#if 1
	/* 瓦片用三角形条带来进行绘制 */
	glBegin(GL_TRIANGLE_STRIP);
	/* 设置瓦片绘制时各个绘制顶点的纹理(实际传入高度值)和索引值 */
	for(i=0; i<count; i++)
	{
		qtmapSetVecShadePoint(pNode, pIndex[i]);
	}
	glEnd();
#else
	gljDrawTriangleByStrip(GL_TRIANGLE_STRIP, count, pIndex, 
		3,pNode->fvertex_part, 
		2,pTex, 
		1,pNode->fterrain );
#endif

	glEndList();

	return 0;
}



#ifdef _JM7200_

/*设置SVS地形告警模式下的显示列表中多重纹理坐标、顶点颜色值和顶点坐标*/
static void qtmapSetSVSPoint(sQTMAPNODE * pNode, f_int32_t ind)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* JM7200最多支持二重纹理，故采用二重纹理，GL_TEXTURE0不能设置，因为顶点给了glColor， GL_TEXTURE1对应告警纹理*/
	//glColor3ub(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);
	glColor3f(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);

#if 1				
	glMultiTexCoord1f(GL_TEXTURE1, pNode->fterrain[ind]);            /* 地形告警用一维纹理 */		
#else
	glMultiTexCoord2f(GL_TEXTURE1, pNode->fterrain[ind], 1.0f);		/* 地形告警用二维纹理 */		
#endif

	/* 设置各个绘制顶点的顶点坐标,使用整型化后的顶点值 */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
#endif
}

/*创建SVS地形告警模式下各个视角下的显示列表*/
f_int32_t qtmapNodeCreateSVSList(sQTMAPNODE * pNode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;

	/* 如果瓦片的SVS地形告警模式的显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->tileSVSList) != 0)
	{
		return 0;
	}

	
	//创建SVS地形告警模式的显示列表，所有视角下用同一个显示列表
	pNode->tileSVSList = glGenLists(1);
	if((pNode->tileSVSList) == 0)
	{
		printf("create SVS list failed \n");
		return -1;
	}
	else
	{
		//printf("create list succeed \n");
	}

	/* 默认是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针。鹰眼模式后续考虑。 */
	count = index_count;
	pIndex = gindices;

	glNewList(pNode->tileSVSList, GL_COMPILE);
#if 1
	/* 瓦片用三角形条带来进行绘制 */
	glBegin(GL_TRIANGLE_STRIP);
	/* 设置瓦片绘制时各个绘制顶点的多重纹理坐标、顶点颜色值和顶点坐标 */
	for(i=0; i<count; i++)
	{
		qtmapSetSVSPoint(pNode, pIndex[i]);
	}
	glEnd();
#else
	gljDrawTriangleByStrip(GL_TRIANGLE_STRIP, count, pIndex, 
		3,pNode->fvertex_part, 
		2,pTex, 
		1,pNode->fterrain );
#endif

	glEndList();

	return 0;
}


/*设置SVS方里网模式下的显示列表中多重纹理坐标、顶点颜色值和顶点坐标*/
static void qtmapSetSVSGridPoint(sQTMAPNODE * pNode, f_int32_t ind, f_int32_t count)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* JM7200最多支持二重纹理，故采用二重纹理，GL_TEXTURE0不能设置，因为顶点给了glColor， GL_TEXTURE1对应方里网纹理*/
	//glColor3ub(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);
	// es3.0模拟器不支持传入UCHAR，整个类型修改了，故改用glColor3f
	glColor3f(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);

    /* JM7200对大数值的纹理坐标计算不正确；方里网绘制时会在东南西北的方位出现断断续续的现象，手动指定纹理坐标值，测试发现最大纹理
	坐标为4.0f时能保持和之前一样的效果 */
#ifdef _JM7200_
	if( (count % 4) == 0)glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
	if( (count % 4) == 1)glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 4.0f);
	if( (count % 4) == 2)glMultiTexCoord2f(GL_TEXTURE1, 4.0f, 0.0f);
	if( (count % 4) == 3)glMultiTexCoord2f(GL_TEXTURE1, 4.0f, 4.0f);
#else
	glMultiTexCoord2f(GL_TEXTURE1, pNode->m_pVertexTex[2*ind], pNode->m_pVertexTex[2*ind+1]);/* 方里网纹理用二维纹理 */
#endif
	/* 设置各个绘制顶点的顶点坐标,使用整型化后的顶点值 */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
#endif
	
}

/*创建SVS方里网模式下各个视角下的显示列表*/
f_int32_t qtmapNodeCreateSVSGridList(sQTMAPNODE * pNode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;

	/* 如果瓦片的SVS方里网模式的显示列表已创建，则不再重复创建，直接返回 */
	if((pNode->tileSVSGridList) != 0)
	{
		return 0;
	}

	
	//创建SVS方里网模式的显示列表，所有视角下用同一个显示列表
	pNode->tileSVSGridList = glGenLists(1);
	if((pNode->tileSVSGridList) == 0)
	{
		printf("create SVS Grid list failed \n");
		return -1;
	}
	else
	{
		//printf("create list succeed \n");
	}

	/* 默认是正常模式,设置瓦片顶点索引数组元素个数、顶点索引数组指针。鹰眼模式后续考虑。 */
	count = index_count;
	pIndex = gindices;

	glNewList(pNode->tileSVSGridList, GL_COMPILE);
#if 1
	/* 瓦片用三角形条带来进行绘制 */
	glBegin(GL_TRIANGLE_STRIP);
	/* 设置瓦片绘制时各个绘制顶点的多重纹理坐标、顶点颜色值和顶点坐标 */
	for(i=0; i<count; i++)
	{
		qtmapSetSVSGridPoint(pNode, pIndex[i], i);
	}
	glEnd();
#else
	gljDrawTriangleByStrip(GL_TRIANGLE_STRIP, count, pIndex, 
		3,pNode->fvertex_part, 
		2,pTex, 
		1,pNode->fterrain );
#endif

	glEndList();

	return 0;
}

#endif