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
���ܣ�������Ƭ����Ķ�ά��������,��ά������������Ĵ�СΪ33*(33+4)
���룺
	��
�����
    ��
����ֵ��
    ��
*/
void generateTexBuffer(void)
{
	/* ��ȡȫ�ֶ���Ķ�ά�������������ָ�� */
	f_float32_t* pTex = gTextureBuf;
	f_int32_t i=0, j=0;
	f_float32_t dx = 1.0f / VTXCOUNT;

	/* ������Ƭ�ڲ�����Ķ�ά��������,��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ,�ʸ����������������Ϊ */
	/* x��ΧΪ[0,1],һ�й�33������,��x�����ֵ����Ϊ0,1/32,2/32,...,31/32,1 */
	/* y��ΧΪ[0,1],һ�й�33������,��y�����ֵ����Ϊ1,31/32,30/32,...,1/32,0 */
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
			// lower, upper, left, right
			switch(i)
			{
			/* ������Ƭ����ȹ�ߵĶ���Ķ�ά��������,����Ƭ�ڶ���(����������)�������������һ�� */
			case 0:
				*pTex++ = (j * dx);
				*pTex++ = 1.0-dx;
				break;
			/* ������Ƭ����ȹ�ߵĶ���Ķ�ά��������,����Ƭ�ڶ���(����������)�������������һ�� */
			case 1:
				*pTex++ = (j * dx);
				*pTex++ = dx;
				break;
			/* ������Ƭ����ȹ�ߵĶ���Ķ�ά��������,����Ƭ�ڶ���(����������)�������������һ�� */
			case 2:
				*pTex++ = dx;
				*pTex++ = 1.0-(j * dx);
				break;
			/* ������Ƭ����ȹ�ߵĶ���Ķ�ά��������,����Ƭ�ڶ���(����������)�������������һ�� */
			case 3:
				*pTex++ = (1 - dx);
				*pTex++ = 1.0-(j * dx);
				break;
			}
		}
	}

#define Do_FileTextureVertexData 1	// ��0�����ݷ����ļ�������
#if Do_FileTextureVertexData // gx test for debug �����������Ϊ�ļ�
	{
		/*
			f_float32_t* pTex�������ô������£�
		*/
		FILE* fp = fopen("D:\\ImgData\\TextCoord.txt", "w");
		if (fp != NULL) 
		{
			int pos=0;
			// ÿ���߿�Ķ�ά��������
			for(i = 0; i <= VTXCOUNT; i++)
			{
				for(j = 0; j <= VTXCOUNT; j++)
				{
					fprintf(fp, "%f ", gTextureBuf[pos++]);
					fprintf(fp, "%f ", gTextureBuf[pos++]);
				}
			}

			// ����ȹ����������
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
			printf("�޷����ļ���\n");
		}
	}
#endif
}


/*
���ܣ���������������������Ƭ���������,����������Ĵ�СΪ(33*2+2)*3+(33*2)+(32*34*2)=2446
���룺
	��
�����
    ��
����ֵ��
    ��
*/
void generateTileIndices(void)
{
#if 1
	/* ��ȡȫ�ֶ������������ָ�� */
	f_uint32_t* pIndex = gindices;
	f_uint32_t i, j, k, m;

	/* ����Ƭ�ڲ�����ӱ�,һ����33������,��������ֵ����Ϊ1089,0,1090,1,1091,2,1093,3...1121,32,32,0 */
	/* ���ڻ��Ʒ�ʽ���������������ʵ�1�������εĶ�������Ϊ1089,0,1090,��2�������εĶ������˳��Ϊ1090,0,1, */
	/* ��3�������εĶ������˳��Ϊ1090,1,1091,֮���Դ�����,����˳��Ϊ˳ʱ�뷽�� */
	/* �������������������32,0,�����Ϳ��Ժ���Ƭ�ڲ������������������� */
	/* �ʵ�65�������εĶ������˳��Ϊ1121,32,32,��66�������εĶ������˳��Ϊ32,32,0, */
	/* ��67�������εĶ������˳��Ϊ32,0,0,��68�������εĶ������˳��Ϊ0,0,33,֮�������Ƭ�ڲ��ĵ�1���������������� */
	/* ������4��������,��4�������ξ���һ��ֱ��,��Ӱ����� */
	/* ��33������,��ʼ���������Ϊ1089,���һ�����������Ϊ1121,�ܹ�������������Ϊ33*2+2 */
	k = (VTXCOUNT + 1) * (VTXCOUNT + 1);
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = k;
		pIndex[1] = j;
		pIndex += 2;
	}
	pIndex[0] = j - 1;	// ����һ�l���ߣ��B�ӵ���һ������,�Ա�춃Ȳ�����������
	pIndex[1] = 0;
	pIndex += 2;

	/* ��Ƭ�ڲ�,ÿ����Ƭ33*33������,��0����������½ǿ�ʼ,��������ֵ����Ϊ0,33,1,34,2,35,3,36...32,65,65,33,33,66...1055,1088,1088,1056 */
	/* ��1�������εĶ�������Ϊ0,33,1;����˳��Ϊ˳ʱ�뷽��,���Ƶ�ʱ���������GL_CW,��˳ʱ�뷽��Ϊ���������� */
	/* ���ڻ��Ʒ�ʽ���������������ʵ�2�������εĶ������˳��Ϊ1,33,34,��3�������εĶ������˳��Ϊ1,34,2,֮���Դ����� */
	/* Ϊ�˴ӵ�һ�е������ι��ɵ��ڶ��е�������,����2����������ֵ65,33,�ʵ�65�������εĶ������˳��Ϊ32,65,65, */
	/* ��66�������εĶ������˳��Ϊ65,65,33,��67�������εĶ������˳��Ϊ33,65,33,��68�������εĶ������˳��Ϊ33,33,66, */
	/* ������4��������,��4�������ξ���һ��ֱ��,��Ӱ����� */
	/* ͬ��,�������2����������ֵ1088,1056,�ʵ�����4�������εĶ������˳��Ϊ1055,1088,1088, */
	/* ������3�������εĶ������˳��Ϊ1088,1088,1056,������2�������εĶ������˳��Ϊ1056,1088,1056, */
	/* ������1�������εĶ������˳��Ϊ1056,1056,1122,�����������ӱ����� */
	/* ��33*33=1089����,��ʼ���������Ϊ0,���һ�����������Ϊ32+33*32=1088,�ܹ�������������Ϊ32*33*2+32*2 */
	for (i = 0, m = 0; i < VTXCOUNT; ++i)	{
		for (j = 0; j < VTXCOUNT1; ++j, ++m) {
			pIndex[0] = m;
			pIndex[1] = m + VTXCOUNT1;
			pIndex += 2;
			// ��������һ��,����һ�l�M���B�ӵ���һ��
			if (i != VTXCOUNT - 1 && j == VTXCOUNT) {
				pIndex[0] = m + VTXCOUNT1;
				pIndex[1] = m + 1;
				pIndex += 2;
			}
		}
	}
	pIndex[0] = m + VTXCOUNT;	// ����һ����β��ͷ�ĺ��ߣ��Ա�������ӵ�����������һ��
	pIndex[1] = m;
	pIndex += 2;

	/* ����Ƭ�ڲ�����ӱ�,һ����33������,��������ֵ����Ϊ1056,1122,1057,1123,1058,1124,1059,1125...1088,1154,1154,1155 */
	/* ���ڻ��Ʒ�ʽ���������������ʵ�1�������εĶ�������Ϊ1056,1122,1057,��2�������εĶ������˳��Ϊ1057,1122,1123, */
	/* ��3�������εĶ������˳��Ϊ1057,1123,1058,֮���Դ�����,����˳��Ϊ˳ʱ�뷽�� */
	/* �������������������1154,1155,�����Ϳ��Ժ���Ƭ�ڲ���߽ӱ������� */
	/* �ʵ�����4�������εĶ������˳��Ϊ1088,1154,1154,������3�������εĶ������˳��Ϊ1154,1154,1155, */
	/* ������2�������εĶ������˳��Ϊ1154,1155,1155,������1�������εĶ������˳��Ϊ1155,1155,0,֮�������Ƭ�ڲ���߽ӱߵĵ�1���������������� */
	/* ������4��������,��4�������ξ���һ��ֱ��,������3�͵�2�������ζ���ͬһ��б�Խ��� */
	/* ��33������,��ʼ���������Ϊ1122,���һ�����������Ϊ1154,�ܹ�������������Ϊ33*2+2 */
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = j + VTXCOUNT * (VTXCOUNT + 1);
		pIndex[1] = k;
		pIndex += 2;
	}
	pIndex[0] = k - 1;	// ����һ��б����,�����������Ҳ����ӵ������������,����������������������
	pIndex[1] = k;
	pIndex += 2;

	/* ����Ƭ�ڲ���߽ӱ�,һ����33������,��������ֵ����Ϊ1155,0,1156,33,1157,66,1158,99...1187,1056,1056,32 */
	/* ���ڻ��Ʒ�ʽ���������������ʵ�1�������εĶ�������Ϊ1155,0,1156,��2�������εĶ������˳��Ϊ1156,0,33, */
	/* ��3�������εĶ������˳��Ϊ1156,33,1157,֮���Դ�����,����˳��Ϊ��ʱ�뷽�� */
	/* �������������������1056,32,�����Ϳ��Ժ���Ƭ�ڲ��ұ߽ӱ������� */
	/* �ʵ�����4�������εĶ������˳��Ϊ1187,1056,1056,������3�������εĶ������˳��Ϊ1056,1056,32, */
	/* ������2�������εĶ������˳��Ϊ1056,32,32,������1�������εĶ������˳��Ϊ32,32,1188,֮�������Ƭ�ڲ���߽ӱߵĵ�1���������������� */
	/* ������4��������,��4�������ξ���һ��ֱ��,������3�͵�2�������ζ���ͬһ��б�Խ��� */
	/* ��33������,��ʼ���������Ϊ1155,���һ�����������Ϊ1187,�ܹ�������������Ϊ33*2+2 */
	for(j = 0; j < VTXCOUNT1; j++, k++) {
		pIndex[0] = k;
		pIndex[1] = j * (VTXCOUNT + 1);
		pIndex += 2;
	}
	pIndex[0] = (j - 1) * (VTXCOUNT + 1);
	pIndex[1] = VTXCOUNT;
	pIndex += 2;

	/* ����Ƭ�ڲ��ұ߽ӱ�,һ����33������,��������ֵ����Ϊ32,1188,65,1189,98,1190,131,1191...1088,1220 */
	/* ���ڻ��Ʒ�ʽ���������������ʵ�1�������εĶ�������Ϊ32,1188,65,��2�������εĶ������˳��Ϊ65,1188,1189, */
	/* ��3�������εĶ������˳��Ϊ65,1189,98,֮���Դ�����,����˳��Ϊ��ʱ�뷽�� */
	/* ������Ӷ������� */
	/* ��33������,��ʼ���������Ϊ1188,���һ�����������Ϊ1220,�ܹ�������������Ϊ33*2 */
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

	for(i = 0; i < 4; i++, k++)/* ����ȹ�� */
	{
		for(j = 0; j < VTXCOUNT; j++, k++)
		{
			switch(i)
			{
			case 0:								// ����ӱ�
				pIndex[0] = j;
				pIndex[1] = k;
				pIndex[2] = j + 1;
				pIndex += 3;
				pIndex[0] = k + 1;
				pIndex[1] = j+1;
				pIndex[2] = k;
				pIndex += 3;
				break;
			case 1:								// �±߽ӱ�
				pIndex[0] = j + 1 + VTXCOUNT*(VTXCOUNT+1);
				pIndex[1] = k;
				pIndex[2] = j + VTXCOUNT*(VTXCOUNT+1);
				pIndex += 3;
				pIndex[0] = k;
				pIndex[1] = j+1 + VTXCOUNT*(VTXCOUNT+1);
				pIndex[2] = k + 1;
				pIndex += 3;
				break;
			case 2:						// ��߽ӱ�
				pIndex[0] = (j + 1) * (VTXCOUNT+1);
				pIndex[1] = k;
				pIndex[2] = j * (VTXCOUNT+1);
				pIndex += 3;
				pIndex[0] = k;
				pIndex[1] = (j+1)*(VTXCOUNT+1);
				pIndex[2] = k + 1;
				pIndex += 3;
				break;

			case 3:						// �ұ߽ӱ�	
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

#define Do_FileTextureVertexData 1	// ��0�����ݷ����ļ�������
#if Do_FileTextureVertexData	//gx test for Debug �߿鶥������ 
	{
		{
			//ʹ��C���� �������newImageд������(D:\level_xidx_yidx.txt)�ļ���
			// ƴ���ļ�·��
			char filePath[256]={0};
			FILE *fp = NULL;
			int pos = 0;
			sprintf(filePath, "D:\\ImgData\\vertexIdx.txt");
			fp = fopen(filePath, "wb");
			// �����ļ�����
			if (fp == NULL) {
				printf("Failed to create file!\n");
				return 1;
			}

			/* ����Ƭ�ڲ�����ӱ�,һ����33������,��������ֵ����Ϊ1089,0,1090,1,1091,2,1093,3...1121,32,32,0 */
			for(j = 0; j < VTXCOUNT1; j++, k++) {
				//pIndex[0] = k;
				//pIndex[1] = j;
				//pIndex += 2;
				fprintf(fp, "%d ", gindices[pos++]);
				fprintf(fp, "%d ", gindices[pos++]);

			}
			fprintf(fp, "%d ", gindices[pos++]);
			fprintf(fp, "%d ", gindices[pos++]);
			//pIndex[0] = j - 1;	// ����һ�l���ߣ��B�ӵ���һ������,�Ա�춃Ȳ�����������
			//pIndex[1] = 0;
			//pIndex += 2;

			/* ��Ƭ�ڲ�,ÿ����Ƭ33*33������,��0����������½ǿ�ʼ,��������ֵ����Ϊ0,33,1,34,2,35,3,36...32,65,65,33,33,66...1055,1088,1088,1056 */
			for (i = 0, m = 0; i < VTXCOUNT; ++i)	{
				for (j = 0; j < VTXCOUNT1; ++j, ++m) {
					fprintf(fp, "%d ", gindices[pos++]);
					fprintf(fp, "%d ", gindices[pos++]);
					//pIndex[0] = m;
					//pIndex[1] = m + VTXCOUNT1;
					//pIndex += 2;

					// ��������һ��,����һ�l�M���B�ӵ���һ��
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
			//pIndex[0] = m + VTXCOUNT;	// ����һ����β��ͷ�ĺ��ߣ��Ա�������ӵ�����������һ��
			//pIndex[1] = m;
			//pIndex += 2;


			/* ����Ƭ�ڲ�����ӱ�,һ����33������,��������ֵ����Ϊ1056,1122,1057,1123,1058,1124,1059,1125...1088,1154,1154,1155 */
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
			//pIndex[0] = k - 1;	// ����һ��б����,�����������Ҳ����ӵ������������,����������������������
			//pIndex[1] = k;
			//pIndex += 2;

			/* ����Ƭ�ڲ���߽ӱ�,һ����33������,��������ֵ����Ϊ1155,0,1156,33,1157,66,1158,99...1187,1056,1056,32 */
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

			/* ����Ƭ�ڲ��ұ߽ӱ�,һ����33������,��������ֵ����Ϊ32,1188,65,1189,98,1190,131,1191...1088,1220 */
			/* ���ڻ��Ʒ�ʽ���������������ʵ�1�������εĶ�������Ϊ32,1188,65,��2�������εĶ������˳��Ϊ65,1188,1189, */
			/* ��3�������εĶ������˳��Ϊ65,1189,98,֮���Դ�����,����˳��Ϊ��ʱ�뷽�� */
			/* ������Ӷ������� */
			/* ��33������,��ʼ���������Ϊ1188,���һ�����������Ϊ1220,�ܹ�������������Ϊ33*2 */
			for(j = 0; j < VTXCOUNT1; j++, k++) {
				fprintf(fp, "%d ", gindices[pos++]);
				fprintf(fp, "%d ", gindices[pos++]);
				//pIndex[0] = j * (VTXCOUNT + 1) + VTXCOUNT;
				//pIndex[1] = k;
				//pIndex += 2;
			}
			// �ر��ļ�
			fclose(fp);
		}
	}
#endif
}

/*����ӥ��ģʽ��������*/
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

/* ����ӥ��ģʽ�������������󶥵������ */
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

/* ���ܣ���������ģ�͵����񶥵�����(����ģʽ) 
	terrain: ��ǰ�߿�33*33��������߳�����
*/
f_int32_t qtmapNodeGenGlobalGrid(sQTMAPNODE *pNode, f_int16_t *terrain)
{
	f_int32_t i, j, i_lon, i_lat;
	f_float32_t hv[2], hvsin[VTXCOUNT1][2], hvcos[VTXCOUNT1][2];	
	f_float32_t hstep = 0.0f, vstep = 0.0f, lon = 0.0f, lat = 0.0f;
	f_int16_t *pTerrainLine = NULL;
	/*JM7200��֧��short���ͣ���fterrain�ڶ�ȡ��ʱ��ת��Ϊfloat����*/
	f_float32_t *pZ = NULL;
	f_float32_t *temp_fvertex = NULL;
	
	// �����ǰ�߿��Ѿ����ɶ�������,���ٴ���
	if(pNode->fvertex != NULL)
	    return 0;
	// �������Ƕ�������
	else
	{
		/* index_count���㣬ÿ��������꣨x,y,z��,ÿ������4���ֽ� */		
		pNode->fvertex = (f_float32_t *)NewAlterableMemory(vertexNum * 3 * sizeof(f_float32_t));
		if(pNode->fvertex == NULL)
		    return -1;
		pNode->fterrain = (f_float32_t *)NewAlterableMemory(vertexNum * sizeof(f_float32_t));
		if(pNode->fterrain == NULL)
		    return -1;
	}	
	temp_fvertex = pNode->fvertex;
	pZ = pNode->fterrain;


	// Ԥ�ȼ���ÿ���������cos|sin�����ں������ټ��㶥���������
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
	

	// ���������Ӧ��ÿ�������������(�������¶����Ų�)
	for(i_lat = 0; i_lat < VTXCOUNT1; i_lat++)		// ����ĳ��
	{
		pTerrainLine = (f_int16_t *)(&terrain[VTXCOUNT1 * (VTXCOUNT1 -1 - i_lat)]);	// terrain�߳����϶���,��������������·�ת        	
	    for(i_lon = 0; i_lon < VTXCOUNT1; i_lon++)	// ����ĳ��
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


	// ��33*33������Χ����������һ�У���������һ��
	for(i = 0; i < 4; i++)	
	{
	    for(j = 0; j < VTXCOUNT1; j++)	// ����ĳ��
	    {
	    	f_int32_t pos0, pos1, ter_pos0,  lat_i, lon_i;
			f_float32_t ht0; 
	    	switch(i)
	    	{
	    		case 0: /*������������һ��*/
	    			pos0 = ((VTXCOUNT1) * VTXCOUNT1 + j) * 3;	
					ter_pos0 = (VTXCOUNT1) * VTXCOUNT1 + j;
	    			pos1 =  (VTXCOUNT1 - 1) * VTXCOUNT1 + j;
					// �����������һ�б���һ��
	    			lon_i = j;
	    			lat_i = 0;
	    			break;
	    		
	    		case 1: /*������������һ��*/
	    			pos0 = ((VTXCOUNT1 + 1) * VTXCOUNT1 + j) * 3;	// +1:ÿһ��������һ������Ķ���
					ter_pos0 = (VTXCOUNT1 + 1) * VTXCOUNT1 + j;
					pos1 = j; //(VTXCOUNT1 - 1) * VTXCOUNT1 + j;	
					// �������������һ�б���һ��
					lon_i = j;
	    			lat_i = VTXCOUNT;	
	    			break;
	    		
	    		case 2: /*��������*/
	    			pos0 = ((VTXCOUNT1 + 2) * VTXCOUNT1 + j) * 3;	// +2:ÿһ��������һ������������Ķ���
					ter_pos0 = (VTXCOUNT1 + 2) * VTXCOUNT1 + j;
					pos1 =(VTXCOUNT1 - 1 -j) * VTXCOUNT1;// j * VTXCOUNT1;
					// �����������һ�б���һ��
					lon_i = 0;
	    			lat_i = j;
	    			break;
	    		
	    		case 3: /*��������*/
	    			pos0 = ((VTXCOUNT1 + 3) * VTXCOUNT1 + j) * 3;
					ter_pos0 = (VTXCOUNT1 + 3) * VTXCOUNT1 + j;
					pos1 = (VTXCOUNT1 - 1 -j)  * VTXCOUNT1 + VTXCOUNT1 - 1;// j * VTXCOUNT1 + VTXCOUNT1 - 1;
					// �������������һ�б���һ��
					lon_i = VTXCOUNT;
	    			lat_i = j;
	    			break;
	    	}

		    ht0 = terrain[pos1]*1.0f - 999.0f + EARTH_RADIUS;	
		    temp_fvertex[pos0 + 0] = ht0 * hvcos[lat_i][1] * hvcos[lon_i][0];
		    temp_fvertex[pos0 + 1] = ht0 * hvcos[lat_i][1] * hvsin[lon_i][0];
		    temp_fvertex[pos0 + 2] = ht0 * hvsin[lat_i][1];
		    pZ[ter_pos0] = terrain[pos1]*1.0f - 999.0f;	// �߶��³�999, Ϊ�˱������߿�Ӳ��ϵ���¶��
	    }
	}

	return 0;
}


/*
���ܣ�����ƽ��ģʽ����Ƭ�������꣬���ڼ�����Ƭ�����εķ��������Ӷ�����SVSģʽ�����������ɫ������x,y��������Ͻǵ�
ʵ�ʾ���(��λ��m)ȷ����zֵΪ��Ƭ����ʵ�ʸ߶�(��λ��m)
      
���룺
	pNode       ��ǰ��Ƭ�ڵ�ָ��
	terrain     ��ǰ��Ƭ�ڵ�Ķ���߶�ָ��

���������
	��

�����
    ��

����ֵ��
    -1 ƽ��ģʽ��Ƭ��������ָ����Ч
    0  ƽ��ģʽ��Ƭ�����������ɳɹ�
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
		/* index_count���㣬ÿ��������꣨x,y,z����ÿ������4���ֽ� */		
		pNode->fvertex_flat = (f_float32_t *)NewAlterableMemory(vertexNum * 3 * sizeof(f_float32_t));
		if(pNode->fvertex_flat == NULL)
		    return -1;

	}	
	temp_fvertex = pNode->fvertex_flat;
	//111319.5mΪ������1���ȿ�ȵ�ʵ�ʾ��룬�����Ƭ��γ�ȷ���ʵ�ʾ��룬�ٳ��Զ��������õ�ÿ������ľ�����
    hstep = pNode->hscale / VTXCOUNT * 111319.5 ;
    vstep = pNode->vscale / VTXCOUNT * 111319.5 ;
	/*������Ƭ���ж����ƽ�涥������*/	
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
	/*������Ƭ4��ȹ�ߵ�ƽ�涥������*/
	for(i = 0; i < 4; i++)
	{
	    for(j = 0; j < VTXCOUNT1; j++)	
	    {
	    	f_int32_t pos0, pos1, ter_pos0,  lat_i, lon_i;
			f_float32_t ht0; 
	    	switch(i)
	    	{
	    		case 0: /*��*/
	    		pos0 = ((VTXCOUNT1) * VTXCOUNT1 + j) * 3;	
				ter_pos0 = (VTXCOUNT1) * VTXCOUNT1 + j;
	    		pos1 =  (VTXCOUNT1 - 1) * VTXCOUNT1 + j;//j;	
	    		lon_i = j;
	    		lat_i = 0;
	    		break;
	    		
	    		case 1: /*��*/
	    		pos0 = ((VTXCOUNT1 + 1) * VTXCOUNT1 + j) * 3;
				ter_pos0 = (VTXCOUNT1 + 1) * VTXCOUNT1 + j;
		        pos1 = j; //(VTXCOUNT1 - 1) * VTXCOUNT1 + j;	
		        lon_i = j;
	    		lat_i = VTXCOUNT;
	    		break;
	    		
	    		case 2: /*��*/
	    		pos0 = ((VTXCOUNT1 + 2) * VTXCOUNT1 + j) * 3;
				ter_pos0 = (VTXCOUNT1 + 2) * VTXCOUNT1 + j;
		        pos1 =(VTXCOUNT1 - 1 -j) * VTXCOUNT1;// j * VTXCOUNT1;
		        lon_i = 0;
	    		lat_i = j;
	    		break;
	    		
	    		case 3: /*��*/
	    		pos0 = ((VTXCOUNT1 + 3) * VTXCOUNT1 + j) * 3;
				ter_pos0 = (VTXCOUNT1 + 3) * VTXCOUNT1 + j;
		        pos1 = (VTXCOUNT1 - 1 -j)  * VTXCOUNT1 + VTXCOUNT1 - 1;// j * VTXCOUNT1 + VTXCOUNT1 - 1;
		        lon_i = VTXCOUNT;
	    		lat_i = j;
	    		break;
	    	}
			/*ȹ�߶���߶ȱ�ʵ�ʵ�999m����ֹ����¶��*/
		    ht0 = terrain[pos1] - 999.0f;		
		    temp_fvertex[pos0 + 0] =  i_lon * hstep;
		    temp_fvertex[pos0 + 1] =  i_lat * vstep;
		    temp_fvertex[pos0 + 2] = ht0 ;
	    }
	}

	return 0;
}


/* ���ܣ���������ģ�͵���������(ӥ��ģʽ) */
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
		/* index_count���㣬ÿ��������꣨x,y,z����ÿ������4���ֽ� */		
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
���ܣ�����Ƭ�����ɸ�����ת��Ϊ����,ת���ķ�ʽ������Ƭ��һ��������Ϊ��׼�㣬�������㱣����Ե�һ�����������ƫ��ֵ����.
      ����ƫ��ֵ�ļ��㷽�����󸡵���֮��ٳ��ԷŴ������ʻ�׼�㻹�豣������ϵ��
      
���룺
	pNode       ��ǰ��Ƭ�ڵ�ָ��

���������
	��

�����
    ��

����ֵ��
    -1 �����ͻ����ε���Ƭ��������ָ����Ч
    0  ���ε���Ƭ�����������ɳɹ�
*/
f_int32_t qtmapNodeGenPartGrid(sQTMAPNODE *pNode)
{
	f_int32_t i = 0, j = 0, i_lat = 0, i_lon = 0;
	/* �����͵���Ƭ��������ָ�� */
	f_float32_t *temp_fvertex = NULL;
	/* ���͵���Ƭ��������ָ�� */
	f_int32_t *temp_fvertex_part = NULL;
	/* ��������ת�������εķŴ���,1073741824.0=2^30,Ϊ�������ֵ */
	double size =  0.1 * 1073741824.0 / pNode->nodeRange.radius; //100.0;
	
	if(pNode->fvertex == NULL)
	    return -1;
	
	/* index_count���㣬ÿ��������꣨x,y,z����ÿ������4���ֽ� */		
	pNode->fvertex_part = NewAlterableMemory(vertexNum * 3 * sizeof(f_int32_t));
	if(pNode->fvertex_part == NULL)
	    return -1;

	temp_fvertex = pNode->fvertex;
	temp_fvertex_part = pNode->fvertex_part;

	/* ��Ƭ�ĵ�һ������������Ϊ��׼���xyz */
	pNode->point_vertex_xyz[0] = temp_fvertex[0];
	pNode->point_vertex_xyz[1] = temp_fvertex[1];
	pNode->point_vertex_xyz[2] = temp_fvertex[2];
	/* �Ŵ����ĵ�����Ϊ��׼�������ϵ�� */
	pNode->point_vertex_xyz[3] = 1.0 / size ;
	
		
	/* ���μ�����Ƭ�и�����������ڻ�׼���ƫ�ƣ������ԷŴ�����ת��Ϊ���� */
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

	/* ���μ�����Ƭ4���ӱ��ϸ�����������ڻ�׼���ƫ�ƣ������ԷŴ�����ת��Ϊ���� */
	for(i = 0; i < 4; i++)
	{
	    for(j = 0; j < VTXCOUNT1; j++)	
	    {
	    	f_int32_t pos;

	    	switch(i)
	    	{
	    		case 0: /*��*/
	    		pos = ((VTXCOUNT1) * VTXCOUNT1 + j) * 3;	

	    		break;
	    		
	    		case 1: /*��*/
	    		pos = ((VTXCOUNT1 + 1) * VTXCOUNT1 + j) * 3;

	    		break;
	    		
	    		case 2: /*��*/
	    		pos = ((VTXCOUNT1 + 2) * VTXCOUNT1 + j) * 3;

	    		break;
	    		
	    		case 3: /*��*/
	    		pos = ((VTXCOUNT1 + 3) * VTXCOUNT1 + j) * 3;

	    		break;
	    	}

	
		temp_fvertex_part[pos + 0] = (f_int32_t)((temp_fvertex[pos + 0]  - pNode->point_vertex_xyz[0]) * size);
		temp_fvertex_part[pos + 1] = (f_int32_t)((temp_fvertex[pos + 1]  - pNode->point_vertex_xyz[1]) * size);
		temp_fvertex_part[pos + 2] = (f_int32_t)((temp_fvertex[pos + 2]  - pNode->point_vertex_xyz[2]) * size);

	    }
	}

	/* ������ÿ����Ƭֻ����8���㣬ǰ4��������Ƭ��4���ǵ�(�߶�Ϊ0)����4��������Ƭ��4���ǵ�(�߶�Ϊ-1000)��ʵ��ֻ����ǰ4���� */
	/* ���μ��㸩������Ƭ8����������ڻ�׼���ƫ�ƣ������ԷŴ�����ת��Ϊ���� */
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

/*�����ڲ������н�ֹĿǰ�Ľڵ�����ֵ��Ϊ������*/
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

/* ���ص�ǰ�������� */
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
** �����µ���Ƭ�ڵ� 
** 
** ������
** ��ʼ�����е�ͼ���ݣ���ʼ����ͼ����
** 
** 
** ���� ��
** ���������    pHandle      ��ͼ���������,ȫ��Ψһ
**               level        ��Ƭ�ڵ���ʼ�㼶
**               xidx         ��Ƭ�ڵ�x��������
**               yidx         ��Ƭ�ڵ�y��������
**               pSubTree     ��Ƭ�ڵ����ڵ�����ָ��
**               emptyOrload  �Ƿ񽫸���Ƭ�ڵ����ݼ��ص��ڴ�
**        
** ���������    ��
** 
** ������������� ppNode       ��Ƭ�ڵ�ָ���ָ��,���ڷ���ָ���´�������Ƭ�ڵ��ָ���ַ
** 
** ����ֵ��0 �ɹ�
**        -1 ʧ��
** 
*/
/// <summary>
/// LODÿ���������Ƭ�߳����ȣ�
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
// �к�-�к�-�㼶
static void CalGeoCoordinateViaTileLoactionAndPixelLocation(int x, int y, int z,
	int m, int n, double* longitude, double* latitude)
{
	double mm = m / 256;
	double nn = n / 256;

	double currentTileWidth = TileSizeInDegree[z];

	*longitude = currentTileWidth * x + currentTileWidth * nn;
	*latitude = currentTileWidth * y + currentTileWidth * mm;

	//�����Ͻ��������������ĵ���������ϵ
	*longitude = *longitude - 180.0f;
	*latitude = 90.0f - *latitude;
}
f_int32_t qtmapnodeCreate(sMAPHANDLE *pHandle, f_int32_t level, f_int32_t xidx, f_int32_t yidx, 
                          sQTMAPSUBTREE * pSubTree, sQTMAPNODE **ppNode, BOOL emptyOrload)
{
	/* ����������Ƭ�ڵ������ͷ�ڵ� */
	stList_Head *pstListHead = NULL;
	/* ���ڱ�������������Ƭ�ڵ������е����нڵ� */
	stList_Head *pstTmpList = NULL;
	/* ָ����Ƭ�ڵ�ָ�� */
	sQTMAPNODE  *pRet = NULL;
	/* ����������Ƭ�ڵ���������û�п��õĽڵ�,Ĭ��û�� */
	BOOL is_dnst_empty = FALSE;

	/* ��ʼ��ָ���´�������Ƭ�ڵ�ָ��Ϊ�� */
	*ppNode = 0;

	takeSem(pHandle->rendscenelock, FOREVER_WAIT);

	/* ���һ���������Ƭ�ڵ���������û�п��õĽڵ� */
	pstListHead = &(pHandle->nodehead.stListHead);
	/**�������**/
	LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
	{
		if( NULL != pstTmpList )
		{
			pRet = ASDE_LIST_ENTRY(pstTmpList, sQTMAPNODE, stListHead);
			/* �ҵ����õ���Ƭ�ڵ�,ֱ������ѭ�� */
			if(DNST_EMPTY == qtmapnodeGetStatus(pRet, __LINE__))
			{
				is_dnst_empty = TRUE;
			    break;
			}			
		}
	}

	giveSem(pHandle->rendscenelock);
	/* �ҵ����õ���Ƭ�ڵ�,�����ٴ����½ڵ�,ֱ��ʹ�øÿ��ýڵ㼴�� */
	if(is_dnst_empty)
	{
		pRet->nodekey = MAPDATANODEKEY;
		pRet->createtime = tickCountGet();
		qtmapnodeSetStatus(pRet, DNST_EMPTY, __LINE__);			
	}	
	else  /* û���ҵ��սڵ㣬�Ǿ���Ҫ����һ���½ڵ� */
	{
#if 1
#define MAXQTNODE_INRAM	  480//(320)//(160)	//��·��160
		/* ���ƻ�����������Ƭ�ڵ����������趨�����ֵ����������Ͳ������½ڵ� */
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
		/* ������Ƭ�ڵ��ڴ�,�����½ڵ� */
		pRet = (sQTMAPNODE *)NewAlterableMemory(sizeof(sQTMAPNODE));
		if (pRet == NULL)
			return -1;

		/* ����������Ƭ�ڵ�������1 */
		pHandle->nodecount++;

		/* �����´�����Ƭ�ڵ��key������ʱ�䡢�ź��� */
		memset(pRet, 0, sizeof(sQTMAPNODE));	
		pRet->nodekey = MAPDATANODEKEY;
		pRet->createtime = tickCountGet();
		pRet->semlock = createBSem(Q_FIFO_SEM, FULL_SEM);
		giveSem(pRet->semlock);

		/* �����´�����Ƭ�ڵ������״̬Ϊ���� */
		qtmapnodeSetStatus(pRet, DNST_EMPTY, __LINE__);	

		/*���´�����Ƭ�ڵ�嵽�������*/
#if 1
		{
			stList_Head *pstListHead = NULL;
			pstListHead = &(pHandle->nodehead.stListHead);
			LIST_ADD(&pRet->stListHead, pstListHead);
		}	
#else
		{
			int i = 0;
			// ���´�����Ƭ�ڵ�嵽��������һλ
			stList_Head *pstListHead = NULL;
			pstListHead = &(pHandle->nodehead.stListHead);
			while(1)
			{
				if(&(pHandle->nodehead.stListHead) == pstListHead->pNext)
					break;
				else
					pstListHead = pstListHead->pNext;i++;
			}
			///printf("�����е�λ�� = %d\n",i);
			LIST_ADD(&pRet->stListHead, pstListHead);
		}
		#endif		
		
		giveSem(pHandle->rendscenelock);
		
	}
	
	/*������Ƭ�ڵ�ľ������*/
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
	
	/* ��ȡ�ڵ��λ����Ϣ�����µ���Ƭ�ڵ�ṹ���� */
	#ifdef USE_FAULT_Tile
	{
		double lonMin,lonMax,latMin,latMax=0.0f;
		double lon_leftUp=0.0f,lat_LeftUp=0.0f,lon_rightBottom=0.0,lat_rightBottom=0.0;
		f_float32_t nodeinfo[9];
		
		// �����߿����� 
		CalGeoCoordinateViaTileLoactionAndPixelLocation(pRet->xidx,pRet->yidx,pRet->level,0,0,&lon_leftUp,&lat_LeftUp);
		CalGeoCoordinateViaTileLoactionAndPixelLocation(pRet->xidx,pRet->yidx,pRet->level,256,256,&lon_rightBottom,&lat_rightBottom);
		lonMin = lon_leftUp;lonMax = lon_rightBottom;
		latMin = lat_rightBottom;latMax = lat_LeftUp;

		/* �����ݽ��д�С��ת�� */
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

		/* ��������5������������� */
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
		/* ˳���ǣ����½Ǿ��ȡ����½�γ�ȡ����ȿ�ȡ�γ�ȿ�ȡ�
		           ���ϽǸ߶ȡ����½Ǹ߶ȡ����ϽǸ߶ȡ����½Ǹ߶ȡ����ĵ�߶� */
		f_float32_t nodeinfo[9];

		/* ����loadinfofile����,������info�ļ��ж�ȡ���� */
		pRet->pSubTree->infoloader(pRet->level, pRet->xidx, pRet->yidx, 
					               pRet->pSubTree->infoparam, &nodeinfo[0]);
		/* �����ݽ��д�С��ת�� */
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

		/* ��������5������������� */
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


	/* �ͷ��ź���,��֪��ά���ݼ����������´�������Ƭ�ڵ� */
    giveSem(pHandle->anyrequire);

	/* ���´�������Ƭ�ڵ㷵�� */
	*ppNode = pRet;
	
	if(emptyOrload == TRUE)
	{
		/* ���´�������Ƭ�ڵ���ʱ����������ʱ,�ڵ�״̬��ΪSLEEP , �ٴμ���ʱ�ı�״̬ */
		qtmapnodeSetStatus(pRet, DNST_SLEEP, __LINE__);	
	}
	else
	{
		/* ���´�������Ƭ�ڵ���Ҫ��������ʱ,�ڵ�״̬��ΪDNST_WAITFORLOAD, ��֪��ά���ݼ���������ظýڵ������ */
		qtmapnodeSetStatus(pRet, DNST_WAITFORLOAD, __LINE__);
	}
        
	return 0;
}


/*
���ܣ�������Ƭ��������Ķ����������꼰��������
���룺
	pNode     ��Ƭ�ڵ�ָ��
	pTex      ��Ƭ�����ƶ������������
	ind       ��Ƭ�����ƶ��������ֵ

���������
	��

�����
    ��

����ֵ��
    ��
*/
static void qtmapSetPoint(sQTMAPNODE * pNode, f_float32_t* pTex, f_int32_t ind)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
#if 1
	/* ���ö�������,��������0��Ӧ�ĸ������ƶ������������,����0Ϊ����Ӱ��������ò�������� */
	glMultiTexCoord2f(GL_TEXTURE0, pTex[2*ind], pTex[2*ind+1]);		

	/* ���ö�������,��������1��Ӧ�ĸ������ƶ������������,����1Ϊ���θ澯���� */
#if 1				
	glMultiTexCoord1f(GL_TEXTURE1, pNode->fterrain[ind]);            /* ���θ澯��һά���� */		
#else
    glMultiTexCoord2f(GL_TEXTURE1, pNode->fterrain[ind], 1.0f);		/* ���θ澯�ö�ά���� */		
#endif

	/* ���ö�������,��������2��Ӧ�ĸ������ƶ������������,����2Ϊʸ���������������,��ʱδʹ�� */
	//glMultiTexCoord2f(GL_TEXTURE2, pTex[2*ind], pTex[2*ind+1]);		

	/* ���ø������ƶ���Ķ�������,ʹ�����ͻ���Ķ���ֵ */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);

#else
	/* ���õ�һ����,ֱ�����ø���������������꼰�������� */
	glTexCoord2f(pTex[2*ind], pTex[2*ind+1]);	//һ������	
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
	
#endif
#endif
}


/*
���ܣ�������Ƭ�����»��Ƶ���ʾ�б�(��������Ƭ��4�����㡢�ĸ��ӱ��ı���)
���룺
	pNode                ��Ƭ�ڵ�ָ��
	is_need_terwarn      �Ƿ����߶�Ԥ��
	scene_mode           ����ģʽ

���������
	��

�����
    ��

����ֵ��
    0   ��ʾ�б����ɹ���֮ǰ�Ѵ���
	-1  ��ʾ�б���ʧ��
*/
f_int32_t qtmapNodeCreateListOverlook(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �����Ƭ�����»��Ƶ���ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->tileListOverlook) != 0)
	{
		return 0;
	}

	/* ������Ƭ�����»��Ƶ���ʾ�б� */
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
	
	// �ϲ�ģʽ,��������Ӹ��������
	#ifdef USE_FAULT_Tile
	{
		// ʹ�����������λ���
		{
			float startTxtX=0.0, endTxtX=1.0;
			float startTxtY=0.0, endTxtY=1.0;
			
			// ������������
			{
				// ʹ���˸�����,�����ڸ������е�λ��
				if (pNode->tqsInFile == 0)
				{
					sQTMAPNODE* parentNode = pNode->tqsParentInFile;
					if(parentNode != NULL)
					{
						int childZoomScale = pow(2,pNode->level - parentNode->level);	// һ�����߿�Ŵ���(ͬʱ�����˵�ǰ��һ�е����߿���)
						float txtStep = 1.0f/(childZoomScale*1.0f);						// ������
						int childStartX = parentNode->xidx * childZoomScale;			// ��ǰ����ʼ���߿��
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

			/* ���Ƶ�һ�������� */
			{
				glBegin(GL_TRIANGLES);
				/* ��������,��������0��һ���������������,��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ */
				glMultiTexCoord2f(GL_TEXTURE0, startTxtX, endTxtY);	// �������Ͻ�		
				/* ���õ�һ����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

				/* ��������,��������0�ڶ����������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,�ڶ�������ʹ�����ϽǵĶ���,�������½� */
				glMultiTexCoord2f(GL_TEXTURE0, endTxtX, startTxtY);	// �������½�
				/* ���õڶ�����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

				/* ��������,��������0�������������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,����������ʹ�����½ǵĶ���,�������Ͻ� */
				glMultiTexCoord2f(GL_TEXTURE0, endTxtX, endTxtY);			// �������Ͻ�
				/* ���õ�������������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[1].x, 
					pNode->fvertex_part_overlook[1].y, 
					pNode->fvertex_part_overlook[1].z);

				glEnd();		
			}

			/* ���Ƶڶ��������� */
			{
				glBegin(GL_TRIANGLES);
				/* ��������,��������0��һ���������������,��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ */
				glMultiTexCoord2f(GL_TEXTURE0, startTxtX, endTxtY);	// �������Ͻ�
				/* ���õ�һ����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

				/* ��������,��������0�ڶ����������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,�ڶ�������ʹ�����ϽǵĶ��� */
				glMultiTexCoord2f(GL_TEXTURE0, startTxtX, startTxtY);// �������½�	
				/* ���õڶ�����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[3].x, 
					pNode->fvertex_part_overlook[3].y, 
					pNode->fvertex_part_overlook[3].z);

				/* ��������,��������0�������������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,����������ʹ�����ϽǵĶ��� */
				glMultiTexCoord2f(GL_TEXTURE0, endTxtX, startTxtY);			// �������½�
				/* ���õ�������������Ͷ������� */
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
			/* ��Ƭ��1���ı��������л��� */
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

			/* ��Ƭ��2�������������л��ƣ����һ���ı��� */
			/* ���Ƶ�һ�������� */
			glBegin(GL_TRIANGLES);
				/* �����ö�������,���õ�һ��������������꣬Ŀǰ�����˶������� */
				//glTexCoord2f(0.0, 1.0);	
				/* ��������,��������0��һ���������������,��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);		
				/* ��������,��������2��һ��������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);		
				/* ���õ�һ����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);

				/* �����ö�������,���õڶ���������������꣬Ŀǰ�����˶������� */
				//glTexCoord2f(1.0, 0.0);	
				/* ��������,��������0�ڶ����������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,�ڶ�������ʹ�����ϽǵĶ��� */
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);	
				/* ��������,��������2�ڶ���������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);
				/* ���õڶ�����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

				/* �����ö�������,���õ�����������������꣬Ŀǰ�����˶������� */
				//glTexCoord2f(1.0, 1.0);	
				/* ��������,��������0�������������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,����������ʹ�����½ǵĶ��� */
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);
				/* ��������,��������2������������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0);	
				/* ���õ�������������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[1].x, 
					pNode->fvertex_part_overlook[1].y, 
					pNode->fvertex_part_overlook[1].z);
		
			glEnd();		

			/* ���Ƶڶ��������� */
			glBegin(GL_TRIANGLES);
				/* ��������,��������0��һ���������������,��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);
				/* ��������,��������2��һ��������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);			
				/* ���õ�һ����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[0].x, 
					pNode->fvertex_part_overlook[0].y, 
					pNode->fvertex_part_overlook[0].z);
		
				/* ��������,��������0�ڶ����������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,�ڶ�������ʹ�����ϽǵĶ��� */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);	
				/* ��������,��������2�ڶ���������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 0.0);	
				/* ���õڶ�����������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[3].x, 
					pNode->fvertex_part_overlook[3].y, 
					pNode->fvertex_part_overlook[3].z);

				/* ��������,��������0�������������������,Ϊ�˱�֤�����ζ���˳ʱ�뷽��Ϊ����,����������ʹ�����ϽǵĶ��� */
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);
				/* ��������,��������2������������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);			
				/* ���õ�������������Ͷ������� */
				glVertex3i(
					pNode->fvertex_part_overlook[2].x, 
					pNode->fvertex_part_overlook[2].y, 
					pNode->fvertex_part_overlook[2].z);

			glEnd();

		#endif

			/* ������Ƭ�������������ӱ�,���ı����������� */
			glBegin(GL_QUAD_STRIP);
			{
				int k = 0;
				f_float32_t dx = 1.0f / VTXCOUNT;
		
				/* �����ö�������,���õ�һ��������������꣬Ŀǰ�����˶������� */
				//glTexCoord2f(0.0, 1.0);		
				/* ��������,��������0��һ������(��Ƭ���½�)����������,��������˳���Ǵ����½ǿ�ʼ,�����������Ǵ����Ͻǿ�ʼ */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);
				/* ��������,��������2��һ��������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);	
				/* ���õ�һ����������Ͷ������� */
				k = 0;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* �����ö�������,���õڶ���������������꣬Ŀǰ�����˶������� */
				//glTexCoord2f(0.0, 1.0 - dx);		
				/* ��������,��������0�ڶ�������(��Ƭ�½ӱߵ��󶥵�)����������,�������������Ƭ�ڲ������ڶ��ж������������ */
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0 - dx);	
				/* ��������,��������2�ڶ���������������꣬Ŀǰδʹ������2 */
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0 - dx);	
				/* ���õڶ�����������Ͷ������� */
				k = 4;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õ���������(��Ƭ���½�)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(1.0, 1.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0);	
				k = 1;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õ��ĸ�����(��Ƭ�½ӱߵ��Ҷ���)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(1.0, 1.0 - dx);	
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 1.0 - dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 1.0 - dx);	
				k = 5;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

		#if 1
				/* ���õ��������(��Ƭ���Ͻ�)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(1.0, 0.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, 0.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, 0.0);	
				k = 2;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õ���������(��Ƭ�ҽӱߵ��϶���)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(1.0, dx);		
				glMultiTexCoord2f(GL_TEXTURE0, 1.0, dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 1.0, dx);	
				k = 6;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õ��߸�����(��Ƭ���Ͻ�)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(0.0, 0.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 0.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 0.0);	
				k = 3;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õڰ˸�����(��Ƭ�Ͻӱߵ��󶥵�)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(0.0, dx);		
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, dx);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, dx);	
				k = 7;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õھŸ�����(��Ƭ���½�)������0���ꡢ���Ͷ������� */
				//glTexCoord2f(0.0, 1.0);		
				glMultiTexCoord2f(GL_TEXTURE0, 0.0, 1.0);		
				//glMultiTexCoord2f(GL_TEXTURE2, 0.0, 1.0);	
				k = 0;
				glVertex3i(
					pNode->fvertex_part_overlook[k].x, 
					pNode->fvertex_part_overlook[k].y, 
					pNode->fvertex_part_overlook[k].z);

				/* ���õ�ʮ������(��Ƭ��ӱߵ��¶���)������0���ꡢ���Ͷ������� */
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
			/* ����Ǿ����ṩ��M9����,�������������������ʾ�б�,���ʸ��� */
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
���ܣ�������Ƭ������������ʾ�б�(����������Ƭ����)
���룺
	pNode                ��Ƭ�ڵ�ָ��
	is_need_terwarn      �Ƿ����߶�Ԥ��
	scene_mode           ����ģʽ

���������
	��

�����
    ��

����ֵ��
    0   ��ʾ�б����ɹ���֮ǰ�Ѵ���
	-1  ��ʾ�б���ʧ��
*/
f_int32_t qtmapNodeCreateList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;
	f_float32_t* pTex = NULL;
	
	/* �����Ƭ������������ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->tileList) != 0)
	{
		return 0;
	}

	/* ������Ƭ������������ʾ�б� */
	pNode->tileList = glGenLists(1);
	if((pNode->tileList) == 0)
	{
		printf("create list failed \n");
		return -1;
	}
	
	/* �жϵ�ǰ��Ⱦ�����ĳ���ģʽ */
	switch(scene_mode)
	{
		/* ���������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_normal:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
		/* �����ӥ��ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�뼰����������������ָ�� */
	case eScene_eyebird:
		count = index_count_eyebird;
		pIndex = indices_eyebird;
		pTex = gTextureBuf_eyebird;
		break;	
		/* Ĭ��Ϊ����ģʽ */
	default:
		count = index_count;
		pIndex = gindices;
		pTex = gTextureBuf;
		break;
	}

	glNewList(pNode->tileList, GL_COMPILE);
#if 1
	/* ��Ƭ�����������������л��� */
	glBegin(GL_TRIANGLE_STRIP);
	/* ������Ƭ����ʱ�������ƶ�������������ֵ */
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
���ܣ�������Ƭ�������ӽǻ���VBO(��������/����߶�)�͸����ӽǻ��Ƶ�VBO(�������꣬��4���ǵ������)��ÿ����Ƭ��VBO�е����ݲ�һ��
���룺
	pNode                ��Ƭ�ڵ�ָ��

���������
	��

�����
    ��

����ֵ��
    0   VBO�����ɹ���֮ǰ�Ѵ���
*/
f_int32_t qtmapNodeCreateVboEs(sQTMAPNODE * pNode)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	int i=0;
	//������Ƭ�ĸ����ӽǻ��Ƶ�VBO(�������꣬��4���ǵ������)
	if (0 == pNode->tileOverlookVertexVBO)
	{
		glGenBuffers(1, &(pNode->tileOverlookVertexVBO));
		/*ֻȡ��Ƭ�и߶�Ϊ0���ĸ��ǵ�*/
		{
#ifdef VERTEX_USE_FLOAT
			f_float32_t verCoord[12] = { 0 };
			for (i = 0; i < 4; i++)
			{
				/*ʹ�ø����͵Ķ�������*/
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
				/*ʹ�����εĶ�������*/
				verCoord[3 * i] = pNode->fvertex_part_overlook[i].x;
				verCoord[3 * i + 1] = pNode->fvertex_part_overlook[i].y;
				verCoord[3 * i + 2] = pNode->fvertex_part_overlook[i].z;
			}

			glBindBuffer(GL_ARRAY_BUFFER, pNode->tileOverlookVertexVBO);
			glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(f_int32_t), verCoord, GL_STATIC_DRAW);
#endif
		}
	}

	//������Ƭ�������ӽǻ���VBO(��������)
	if (0 == pNode->tileVertexVBO)
	{
		glGenBuffers(1, &pNode->tileVertexVBO);
		/*�����Ƭ�ĸ�����������*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)* vertexNum, pNode->fvertex, GL_STATIC_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLint)* vertexNum, pNode->fvertex_part, GL_STATIC_DRAW);
#endif
	}
	//������Ƭ�������ӽǻ���VBO(����߶�)
	if (0 == pNode->tileTerrainVBO)
	{
		glGenBuffers(1, &pNode->tileTerrainVBO);
		/*�����Ƭ�ĸ�������߶ȣ��������ɸ澯����*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* vertexNum, pNode->fterrain, GL_STATIC_DRAW);
	}
#endif
	return 0;
}



/*
���ܣ�������Ƭ�Ļ���VBO(��������/����߶�/������ɫ/���㷨��/���㷽��������)��ÿ����Ƭ��VBO�е����ݲ�һ����SVSģʽ�����ӽǶ�����33*33������
���룺
	pNode                ��Ƭ�ڵ�ָ��

���������
	��

�����
    ��

����ֵ��
    0   VBO�����ɹ���֮ǰ�Ѵ���
*/
f_int32_t qtmapNodeCreateSvsVboEs(sQTMAPNODE * pNode)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	int i=0;

	//������Ƭ�Ļ���VBO(��������)
	if (0 == pNode->tileVertexVBO)
	{
		glGenBuffers(1, &pNode->tileVertexVBO);
		/*�����Ƭ�ĸ�����������*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileVertexVBO);
#ifdef VERTEX_USE_FLOAT
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)* vertexNum, pNode->fvertex, GL_STATIC_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLint)* vertexNum, pNode->fvertex_part, GL_STATIC_DRAW);
#endif
	}
	//������Ƭ�Ļ���VBO(����߶�)
	if (0 == pNode->tileTerrainVBO)
	{
		glGenBuffers(1, &pNode->tileTerrainVBO);
		/*�����Ƭ�ĸ�������߶ȣ��������ɸ澯����*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileTerrainVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* vertexNum, pNode->fterrain, GL_STATIC_DRAW);
	}
	//������Ƭ�Ļ���VBO(������ɫ)
	if (0 == pNode->tileColorVBO)
	{
		glGenBuffers(1, &pNode->tileColorVBO);
		/*�����Ƭ�ĸ�����������*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileColorVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat)* vertexNum, pNode->m_pColor, GL_STATIC_DRAW);
	}
	//������Ƭ�Ļ���VBO(���㷨��)
	if (0 == pNode->tileNormalVBO)
	{
		glGenBuffers(1, &pNode->tileNormalVBO);
		/*�����Ƭ�ĸ�����������*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileNormalVBO);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLbyte)* vertexNum, pNode->pNormal, GL_STATIC_DRAW);
	}
	//������Ƭ�Ļ���VBO(���㷽��������)
	if (0 == pNode->tileGridTexVBO)
	{
		glGenBuffers(1, &pNode->tileGridTexVBO);
		/*�����Ƭ�ĸ�����������*/
		glBindBuffer(GL_ARRAY_BUFFER, pNode->tileGridTexVBO);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* vertexNum, pNode->m_pVertexTex, GL_STATIC_DRAW);
	}
#endif
	return 0;
}


/*
���ܣ�������Ƭ�������ӽǻ���VBO(��������/������������)�͸����ӽǻ��Ƶ�VBO(��������/������������)��������Ƭ����ЩVBO�е����ݾ�һ�����ʲ��ŵ��ṹ��sQTMAPNODE�ж��壬
      �þ�̬�����洢���ɣ��ú���������ģʽ��ʹ�á�
���룺
	��

���������
	pibo                �����ӽ���Ƭ��������VBOָ��
	ptvbo               �����ӽ���Ƭ������������VBOָ��
	ptvbo_overlook      �����ӽ���Ƭ��������VBOָ��
	pibo_overlook       �����ӽ���Ƭ������������VBOָ��

�����
    ��

����ֵ��
    0   VBO�����ɹ���֮ǰ�Ѵ���
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

	/*�����ӽ���Ƭ������������VBO*/
	if (tvbo == 0)
	{
		glGenBuffers(1, &tvbo);
		glBindBuffer(GL_ARRAY_BUFFER, tvbo);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* vertexNum, pTex, GL_STATIC_DRAW);

		*ptvbo = tvbo;
	}
	/*�����ӽ���Ƭ��������VBO*/
	if (ibo == 0)
	{
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* count, pIndex, GL_STATIC_DRAW);

		*pibo = ibo;
	}
	/*�����ӽ���Ƭ������������VBO*/
	if (tvbo_overlook == 0)
	{
		f_float32_t texCoord[8] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glGenBuffers(1, &tvbo_overlook);
		glBindBuffer(GL_ARRAY_BUFFER, tvbo_overlook);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat)* 4, texCoord, GL_STATIC_DRAW);

		*ptvbo_overlook = tvbo_overlook;
	}
	/*�����ӽ���Ƭ��������VBO*/
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
���ܣ�������Ƭ�Ķ������VBO(��������)��������Ƭ����ЩVBO�е����ݾ�һ�����ʲ��ŵ��ṹ��sQTMAPNODE�ж��壬
      �þ�̬�����洢���ɣ��ú�����SVSģʽ��ʹ�á�
���룺
	��

���������
	pibo                ��Ƭ��������VBOָ��

�����
    ��

����ֵ��
    0   VBO�����ɹ���֮ǰ�Ѵ���
*/
f_int32_t genSvsConstVBOs(unsigned int* pibo)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = index_count;
	f_uint32_t* pIndex = gindices;

	GLuint ibo = *pibo;

	/*��Ƭ��������VBO*/
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
���ܣ�������Ƭ�Ķ������VBO(��������/������������)��������Ƭ����ЩVBO�е����ݾ�һ�����ʲ��ŵ��ṹ��sQTMAPNODE�ж��壬
      �þ�̬�����洢���ɣ��ú�����VecShadeģʽ��ʹ�á�
���룺
	��

���������
	pibo                ��Ƭ��������VBOָ��

�����
    ��

����ֵ��
    0   VBO�����ɹ���֮ǰ�Ѵ���
*/
f_int32_t genVecShadeConstVBOs(unsigned int* pibo, unsigned int* ptvbo)
{
#if defined(OPENGL_ES_2) || defined(OPENGL_ES_3)
	f_int32_t count = index_count;
	f_uint32_t* pIndex = gindices;
	f_float32_t* pTex = gTextureBuf;

	GLuint ibo = *pibo;
		GLuint tvbo = *ptvbo;

	/*��Ƭ��������VBO*/
	if (ibo == 0)
	{
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* count, pIndex, GL_STATIC_DRAW);

		*pibo = ibo;
	}

	/*��Ƭ������������VBO*/
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
���ܣ�������Ƭ�����»���ʱ���ߵĻ�����ʾ�б�
���룺
	pNode                ��Ƭ�ڵ�ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��ʾ�б����ɹ���֮ǰ�Ѵ���
	-1  ��ʾ�б���ʧ��
*/
f_int32_t qtmapNodeCreateEdgeListOverlook(sQTMAPNODE * pNode)
{
	/* �����Ƭ�����µı��߻�����ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->edgeListOverlook) != 0)
	{
		return 0;
	}

	/* ������Ƭ�����µı��߻�����ʾ�б� */	
	pNode->edgeListOverlook = glGenLists(1);
	if((pNode->edgeListOverlook) == 0)
	{
		return -1;
	}
		
	glNewList(pNode->edgeListOverlook, GL_COMPILE);
	/* ���λ�����������,�ܹ�4���� */
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
���ܣ�������Ƭ��������ʱ���ߵĻ�����ʾ�б�
���룺
	pNode                ��Ƭ�ڵ�ָ��

���������
	��

�����
    ��

����ֵ��
    0   ��ʾ�б����ɹ���֮ǰ�Ѵ���
	-1  ��ʾ�б���ʧ��
*/
f_int32_t qtmapNodeCreateEdgeList(sQTMAPNODE * pNode)
{
	f_int32_t i, j, index0/*, index1*/;
	
	/* �����Ƭ�ı��߻�����ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->edgeList) != 0)
	{
		return 0;
	}
	
	/* ������Ƭ�ı��߻�����ʾ�б� */
	pNode->edgeList = glGenLists(1);
	if((pNode->edgeList) == 0)
	{
		return -1;
	}
		
	glNewList(pNode->edgeList, GL_COMPILE);
	/* ���λ�����������,ÿ������33���� */
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

			/* ���ø�������,�������� */
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


/*����vecShadeģʽ�µ���ʾ�б��ж�����������Ͷ�������*/
static void qtmapSetVecShadePoint(sQTMAPNODE * pNode, f_int32_t ind)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* JM7200���֧�ֶ��������ʲ��ö�������GL_TEXTURE0��ӦvecShade�� GL_TEXTURE1��Ӧ�澯����*/
	/* ���ö�������,��������0��Ӧ�ĸ������ƶ������������,����0ΪvecShade���� */
#if 0				
	glMultiTexCoord1f(GL_TEXTURE0, pNode->fterrain[ind]);            /* vecShade��һά���� */		
#else
	glMultiTexCoord2f(GL_TEXTURE0, pNode->fterrain[ind], 1.0f);		/* vecShade�ö�ά���� */		
#endif

	/* ���ö�������,��������1��Ӧ�ĸ������ƶ������������,����1Ϊ���θ澯���� */
#if 0				
	glMultiTexCoord1f(GL_TEXTURE1, pNode->fterrain[ind]);            /* ���θ澯��һά���� */		
#else
	glMultiTexCoord2f(GL_TEXTURE1, pNode->fterrain[ind], 1.0f);		/* ���θ澯�ö�ά���� */		
#endif

	/* ���ø������ƶ���Ķ�������,ʹ�����ͻ���Ķ���ֵ */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
#endif
}

/*����vecShadeģʽ�¸����ӽ��µ���ʾ�б�*/
f_int32_t qtmapNodeCreateVecShadeList(sQTMAPNODE * pNode, BOOL is_need_terwarn, f_int32_t scene_mode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;

	/* �����Ƭ��vecShadeģʽ����ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->tileVecShadeList) != 0)
	{
		return 0;
	}
	
	/* ����vecShade����ʾ�б������ӽ�����ͬһ����ʾ�б� */
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

	/* Ĭ��������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�롣ӥ��ģʽ�������ǡ� */
	count = index_count;
	pIndex = gindices;

	glNewList(pNode->tileVecShadeList, GL_COMPILE);
#if 1
	/* ��Ƭ�����������������л��� */
	glBegin(GL_TRIANGLE_STRIP);
	/* ������Ƭ����ʱ�������ƶ��������(ʵ�ʴ���߶�ֵ)������ֵ */
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

/*����SVS���θ澯ģʽ�µ���ʾ�б��ж����������ꡢ������ɫֵ�Ͷ�������*/
static void qtmapSetSVSPoint(sQTMAPNODE * pNode, f_int32_t ind)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* JM7200���֧�ֶ��������ʲ��ö�������GL_TEXTURE0�������ã���Ϊ�������glColor�� GL_TEXTURE1��Ӧ�澯����*/
	//glColor3ub(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);
	glColor3f(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);

#if 1				
	glMultiTexCoord1f(GL_TEXTURE1, pNode->fterrain[ind]);            /* ���θ澯��һά���� */		
#else
	glMultiTexCoord2f(GL_TEXTURE1, pNode->fterrain[ind], 1.0f);		/* ���θ澯�ö�ά���� */		
#endif

	/* ���ø������ƶ���Ķ�������,ʹ�����ͻ���Ķ���ֵ */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
#endif
}

/*����SVS���θ澯ģʽ�¸����ӽ��µ���ʾ�б�*/
f_int32_t qtmapNodeCreateSVSList(sQTMAPNODE * pNode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;

	/* �����Ƭ��SVS���θ澯ģʽ����ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->tileSVSList) != 0)
	{
		return 0;
	}

	
	//����SVS���θ澯ģʽ����ʾ�б������ӽ�����ͬһ����ʾ�б�
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

	/* Ĭ��������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�롣ӥ��ģʽ�������ǡ� */
	count = index_count;
	pIndex = gindices;

	glNewList(pNode->tileSVSList, GL_COMPILE);
#if 1
	/* ��Ƭ�����������������л��� */
	glBegin(GL_TRIANGLE_STRIP);
	/* ������Ƭ����ʱ�������ƶ���Ķ����������ꡢ������ɫֵ�Ͷ������� */
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


/*����SVS������ģʽ�µ���ʾ�б��ж����������ꡢ������ɫֵ�Ͷ�������*/
static void qtmapSetSVSGridPoint(sQTMAPNODE * pNode, f_int32_t ind, f_int32_t count)
{
#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
	/* JM7200���֧�ֶ��������ʲ��ö�������GL_TEXTURE0�������ã���Ϊ�������glColor�� GL_TEXTURE1��Ӧ����������*/
	//glColor3ub(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);
	// es3.0ģ������֧�ִ���UCHAR�����������޸��ˣ��ʸ���glColor3f
	glColor3f(pNode->m_pColor[3*ind], pNode->m_pColor[3*ind+1], pNode->m_pColor[3*ind+2]);

    /* JM7200�Դ���ֵ������������㲻��ȷ������������ʱ���ڶ��������ķ�λ���ֶ϶������������ֶ�ָ����������ֵ�����Է����������
	����Ϊ4.0fʱ�ܱ��ֺ�֮ǰһ����Ч�� */
#ifdef _JM7200_
	if( (count % 4) == 0)glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
	if( (count % 4) == 1)glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 4.0f);
	if( (count % 4) == 2)glMultiTexCoord2f(GL_TEXTURE1, 4.0f, 0.0f);
	if( (count % 4) == 3)glMultiTexCoord2f(GL_TEXTURE1, 4.0f, 4.0f);
#else
	glMultiTexCoord2f(GL_TEXTURE1, pNode->m_pVertexTex[2*ind], pNode->m_pVertexTex[2*ind+1]);/* �����������ö�ά���� */
#endif
	/* ���ø������ƶ���Ķ�������,ʹ�����ͻ���Ķ���ֵ */
	glVertex3i(pNode->fvertex_part[3*ind], pNode->fvertex_part[3*ind+1], pNode->fvertex_part[3*ind+2]);
#endif
	
}

/*����SVS������ģʽ�¸����ӽ��µ���ʾ�б�*/
f_int32_t qtmapNodeCreateSVSGridList(sQTMAPNODE * pNode)
{
	f_int32_t i = 0, j=0;
	f_int32_t count = 0;
	f_uint32_t* pIndex = NULL;

	/* �����Ƭ��SVS������ģʽ����ʾ�б��Ѵ����������ظ�������ֱ�ӷ��� */
	if((pNode->tileSVSGridList) != 0)
	{
		return 0;
	}

	
	//����SVS������ģʽ����ʾ�б������ӽ�����ͬһ����ʾ�б�
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

	/* Ĭ��������ģʽ,������Ƭ������������Ԫ�ظ�����������������ָ�롣ӥ��ģʽ�������ǡ� */
	count = index_count;
	pIndex = gindices;

	glNewList(pNode->tileSVSGridList, GL_COMPILE);
#if 1
	/* ��Ƭ�����������������л��� */
	glBegin(GL_TRIANGLE_STRIP);
	/* ������Ƭ����ʱ�������ƶ���Ķ����������ꡢ������ɫֵ�Ͷ������� */
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