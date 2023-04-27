/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: WhiteCity.c
**
** ����: ���ļ�������άģ��������Ϣ�Ķ��뺯������άģ�͵Ļ��ƺ�����
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
**		2015-8-24  16:13  LPF �����ļ���
**		2015-9-6   14:54  LPF �޸Ķ�ȡ���뾶�����ĵ�������Ϊ���س�����㣻
**		2015-9-23  11:39  LPF �޸�����Ϊָ�룻
**		2015-10-12 15:22  LPF ����ͼƬ���ƱȶԺ�����������ͬͼƬ�ظ����룻
**		2015-10-15 11:29  LPF ���������ǻ��������ļ��޸ĳ���
**      2015-12-21 10:13  LPF ͳһ����˳��Ϊ��ʱ�룻
**		2015-12-21 16:46  LPF ���ӱ����޳����ܣ�
**		2015-12-22 16:00  LPF ���ӷ��߼��㡢͸���ȼ��㹦�ܣ�
**		2016-1-5 15:10	  LPF ���Ӹ߶�Ԥ��
**		2016-4-1 10:47    LPF ���ӽ�������Ը߶ȶ�ȡ
**		2016-5-16  15:34  LPF �����ʾ֡��
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
**-----------------------------------------------------------
*/
#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#else
#include <vxWorks.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"
#include "../projection/coord.h"
#include "../geometry/vectorMethods.h"
#include "../mapApi/paramGetApi.h"

#include "loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "WhiteCity.h"
/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/


/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*
#ifdef WIN32
#define WhiteCityPath "../../../../DataFile/model/WhiteCity/"
#else
#define WhiteCityPath "/ahci00/mapData3d/DataFile/model/WhiteCity/"
#endif
*/
/*#define	ColorIndex		0.902, 0.906, 0.886, 0.75				//�Ұ�ɫ+��͸�� 0.5-->0.75*/
//#define ColorIndex		1.0,0.0,0.0, 0.5						//��ɫ
#define	CityRadiusSize				5						//���������İ뾶

#define	ColorIndex					1/0.6					//0.902, 0.906, 0.886	RGB��ֵ
#define AlphaIndex					1.0						//͸����ֵ

#define	Alpha_TopBuilding			0.60						//����ϵ��--����
#define	Alpha_NorthBuiding			0.45  					//����ϵ��--�ϱ���
#define	Alpha_WestBuilding			0.25	    					//����ϵ��--������

//�ɿؿ��ر���
#define Line_BuildingShow				1						//�Ƿ���ʾ���������
#define Polygon_TopFaceRender		0						//�Ƿ���ö���η�ʽ���ƶ���
#define Building_Warning				0						//�Ƿ��������߶�Ԥ��
#define Building_HeiChoose          		1						//�Ƿ�����ĸ߶��Ǻ��θ߶ȣ�1���ǣ�0����
#define Building_TopConcaveDraw		0						//�Ƿ���ð�����λ��ƶ���
/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
static stList_Head list_citydata;
static stList_Head *m_listCityCfgData = &list_citydata;					//�洢���������Ϣ
static PT_3D	g_whitecity_position ;									//������Ļ�׼��
/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** �ڲ���������
**-----------------------------------------------------------
*/
void WhiteCityCallList_Concave(LPSingleCityCfg lpsinglecitycfg_LIST,int lpsinglecitycfg_LIST_ID, sGLRENDERSCENE* pModelScene);
int WhiteCityBmpCompare(const char ComparedBmpName[255], unsigned int* TextureID);
int RenderWhiteCitySingle(sGLRENDERSCENE* pModelScene);
BOOL CalNormalVector_SingleBuilding(LPSingleCityCfg LPSingleCityCfg_Normal);
BOOL CalAlpha_SingleBuilding(LPSingleCityCfg LPSingleCityCfg_Alpha);
int GenerateCityList(stList_Head * List3rd);
int RenderCityListThree(stList_Head * List3rd);
int RenderCityListFour(stList_Head * List3rd);
int RenderCityListLine(stList_Head * List3rd);
BOOL CalNormalVector_SingleFace(LP_PT_3D PointVextor1, LP_PT_3D PointVextor2, LP_PT_3D PointVextor3, LP_PT_3D PointVextor4,LP_PT_3D NormalVector, LP_PT_3D MiddlePoint);
BOOL CalAlpha_SingleFace(LP_PT_3D NormalVector_SingleFace, LP_PT_3D MiddlePoint_SingleFace,double * Alpha_SingleFace);
BOOL CalNormal_NorthAngle(LP_PT_3D NormalVector_AnlgeCal, LP_PT_3D  NorthVector_AngleCal ,double * Normal_NorthAngle);

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/

/*.BH--------------------------------------------------------
**
** ������: InitWhiteCityData
**
** ����:  ������н���Ⱥ�ĵ������ݺ͸߳�����
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ⲿ�ӿ�1, ��һ�㺯��
**
**.EH--------------------------------------------------------
*/
int InitWhiteCityData()
{
    LPSingleCityCfg pData = NULL;
    f_char_t file_path[1024] = {0};
    FILE *pFile = NULL;
    f_int32_t ret=0;
    f_int32_t TempI = 0;
    int i, j;

    //���ļ�
    memset(file_path, 0, 1024);
    strncpy(file_path, WhiteCityPath, 512);
    strncat(file_path, "WhiteCityData.cfg", 17);
    pFile = fopen(file_path,"rb");
    if(pFile == NULL)
    {
        printf("open %s failed.\n",pFile);
        return FALSE;
    }

    //��ʼ������
    CreateList(m_listCityCfgData);


    //��ȡ������Ϣ

    while(1)
    {
        ret = readtxtfile_getInt(pFile);
        if (ret == 0)
        {
            break;
        }
        pData = (LPSingleCityCfg)NewFixedMemory(sizeof(SingleCityCfg));

        pData->IdNum = ret;

        pData->PointNum = readtxtfile_getInt(pFile);

        (pData->Lon) = NewFixedMemory(sizeof(double)*(pData->PointNum) );
        (pData->Lat) = NewFixedMemory(sizeof(double)*(pData->PointNum) );
        for (i = 0; i<pData->PointNum;i++)
        {
            *(pData->Lon+i) = readtxtfile_getFloat(pFile) - 16.530377f;
            *(pData->Lat+i) = readtxtfile_getFloat(pFile) + 8.113f;
        }

        pData->Hei = readtxtfile_getInt(pFile) + 200;		//lpf add �ֶ�����300m

#if 0	//Ŀǰ�ڵ��������������ʱ�����ڵò������Ը߶ȣ����������ڻ��س����ڼ�������4��
        pData->radius = readtxtfile_getFloat(pFile);
        pData->AvgLonLat[0] = readtxtfile_getFloat(pFile);
        pData->AvgLonLat[1] = readtxtfile_getFloat(pFile);
        pData->AvgLonLat[2] = readtxtfile_getFloat(pFile);

#endif
#if 1
        //����4 �У����뾶+ ���ĵ�����
        readtxtfile_getFloat(pFile);
        readtxtfile_getFloat(pFile);
        readtxtfile_getFloat(pFile);
        readtxtfile_getFloat(pFile);

#endif
        //��ȡ������Ϣ
        pData->m_bHasTexture = readtxtfile_getInt(pFile);
        if (pData->m_bHasTexture != FALSE)
        {
            pData->m_TextureCity = NewFixedMemory(sizeof(SingleCityCfg_Texture)* (pData->m_bHasTexture));
            for(j=0;j<pData->m_bHasTexture;j++)
            {
                (*(pData->m_TextureCity+j)).m_PosID = readtxtfile_getInt(pFile);
                readtxtfile_getStr(pFile, (pData->m_TextureCity+j)->m_strname, 255);
                TempI = readtxtfile_getInt(pFile);

                (*(pData->m_TextureCity+j)).m_pTexverts = NewFixedMemory(sizeof(PT_2D)*TempI);
                for (i = 0; i<TempI;i++)
                {
                    (*((*(pData->m_TextureCity+j)).m_pTexverts + i )).x = readtxtfile_getFloat(pFile);
                    (*((*(pData->m_TextureCity+j)).m_pTexverts + i )).y = readtxtfile_getFloat(pFile);
                }

                //�ж�ͼƬ�Ƿ��Ѿ�����
                if(TRUE == WhiteCityBmpCompare((pData->m_TextureCity+j)->m_strname,&((*(pData->m_TextureCity+j)).TextureID)) )
                {

                }
                else
                {
                    //���������ļ�, ��������ID
                    LoadWhiteCityTexture((*(pData->m_TextureCity+j)).m_strname,&((*(pData->m_TextureCity+j)).TextureID));
                }
            }
        }

        //��ȡ����������ǻ�������
        pData->BottomPointNum = readtxtfile_getInt(pFile);

        (pData->BottomTriangle_LonLat) = NewFixedMemory(sizeof(PT_2D)*pData->BottomPointNum);

        for (i = 0; i<pData->BottomPointNum;i++)
        {
            (*(pData->BottomTriangle_LonLat+i)).x = readtxtfile_getFloat(pFile) - 16.530377f;
            (*(pData->BottomTriangle_LonLat+i)).y = readtxtfile_getFloat(pFile) + 8.113f;
        }

        //���䷨�������Ŀռ�
        pData->NormalVector = NewFixedMemory(sizeof(PT_3D)* pData->PointNum);
        pData->AlpherVector = NewFixedMemory(sizeof(double)*pData->PointNum);
        pData->MiddlePointPerFace = NewFixedMemory(sizeof(PT_3D)* pData->PointNum);

        //�������洢�ڵ���Ϣ
        readtxtfile_nextline(pFile);
        {
                /**�ҵ�ͷ���֮��**/
            stList_Head *pstListHead = NULL;
            pstListHead = (m_listCityCfgData);
            LIST_ADD(&pData->stListHead, pstListHead);
        }

    }
    fclose(pFile);

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: LoadWhiteCityTexture
**
** ����:  �����ģ���е�����ͼ��
**
** �������:  ����·��
**
** �������������ID
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڶ��㺯��
**
**.EH--------------------------------------------------------
*/
int LoadWhiteCityTexture(const f_char_t* filepath, f_uint32_t* textureID)
{
    f_uint16_t width, height;
    f_uint32_t size;
    f_uint8_t style;
    f_uint8_t* pImageData = NULL;
    f_char_t file_path[1024] = {0};
    f_uint32_t textureid = 0;

    if(!filepath)
        return FALSE;

    /** ����λͼ */
    memset(file_path, 0, 1024);
    strncpy(file_path, WhiteCityPath, 512);
    strcat(file_path, filepath);
    strncat(file_path, ".bmp",4);

    pImageData = ReadBMP(file_path, &width, &height, &size, &style);

    if(pImageData == NULL)
    {
        printf("����ģ��-%s:����λͼʧ��!\n", file_path);
        return FALSE;
    }
    glGenTextures(1, &textureid);

    glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
    glBindTexture(GL_TEXTURE_2D, textureid);

    /** �����˲� */
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    /** �������� */
//	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB,
//					  GL_UNSIGNED_BYTE, pImageData);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
    DeleteAlterableMemory(pImageData);
    *textureID = textureid;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: RenderWhiteCity
**
** ����:  ����������
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  ��һ�㺯�����ⲿ�ӿ�2
**
**.EH--------------------------------------------------------
*/
int RenderWhiteCity(sGLRENDERSCENE* pModelScene)
{
    //��������
    LPSingleCityCfg pData = NULL;
    f_float64_t AvgLon = 0;
    f_float64_t AvgLat = 0;
    int i=0;

    f_float64_t radius = CityRadiusSize;
    f_float64_t dis = 0;
    f_float64_t dislon = 0;
    f_float64_t dislat = 0;

    //�жϿ��Ʊ����Ƿ�Ϊtrue
    if(CheckModelRender() == FALSE)
    {
//		printf("���ƿ���δ�򿪣�\n");
        return FALSE;
    }

    //�жϷɻ��߶��Ƿ�С��5000m
    if( GetPlaneHei(pModelScene) > 5000)				//LPF modify 5000 -> 450000
    {
//		printf("�ɻ��߶ȴ���5000��\n");
        return FALSE;
    }

    //�жϽ������Ƿ����Ӿ�����
    {
        stList_Head *pstListHead = (m_listCityCfgData);
        LPSingleCityCfg lpsinglemodel =NULL;
        lpsinglemodel = ASDE_LIST_ENTRY(pstListHead->pNext, SingleCityCfg, stListHead);

        if (CheckBulidingInFrustum( lpsinglemodel , pModelScene) == FALSE)
        {
            return FALSE;
        }

    }


#if 1
    //����������
    //���ƽ�����
    if (FALSE == RenderWhiteCitySingle(pModelScene))
    {
        return FALSE;
    }
#endif

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: RenderWhiteCitySingle
**
** ����:  ���ƽ����ﺯ��
**
** �������:  LPSingleCityCfg�ṹ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڶ��㺯��
**
**.EH--------------------------------------------------------
*/
//int RenderWhiteCitySingle(LPSingleCityCfg lpsinglecitycfg2rd)

//LPF 2017-2-16 14:26:44 �ж��Ƿ���Ҫ���㽨��������
//�ж��Ƿ���Ҫ���¼��㽨��������
//����ֵ��TRUE,��Ҫ��FALSE,����Ҫ
BOOL ReCalBuilding(LPSingleCityCfg lpsinglecitycfg_ex)
{
    f_int16_t pZ = 0.0;
    PT_3D CurPosXyz = {0};

    // 0.�ж��Ƿ��һ�μ���
    if (lpsinglecitycfg_ex->WhiteCity_XYZ == NULL)
    {
        return TRUE;
    }

    // 1.�ж��Ƿ���Ҫ���¼��㣬Ӧ�ø��ݽ��������ĺ��θ߶�,���漰������ľ��Ժ��θ߶�
	if(FALSE == getAltByGeoPos(*lpsinglecitycfg_ex->Lon, *lpsinglecitycfg_ex->Lat, &pZ) )	
    {
        return TRUE;
    }

//	EarthToXYZ(DEGREE_TO_RADIAN(*lpsinglecitycfg_ex->Lon), DEGREE_TO_RADIAN(*lpsinglecitycfg_ex->Lat), pZ,
//		&CurPosXyz.x, &CurPosXyz.y, &CurPosXyz.z);

    {
        Geo_Pt_D geo_pt;

        geo_pt.lat = *lpsinglecitycfg_ex->Lat;
        geo_pt.lon = *lpsinglecitycfg_ex->Lon;
        geo_pt.height = pZ;

        geoDPt2objDPt(&geo_pt, &CurPosXyz);
    }






    //	if (CurPosXyz.x == lpsinglecitycfg2rd->WhiteCity_XYZ->x)
    if (CurPosXyz.x != 	(((*(lpsinglecitycfg_ex->WhiteCity_XYZ+1)).x)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//lpf 2017-2-16 15:24 ���㽨��������
BOOL CalBuildingData(/*LPNode pNode2nd*/)
{
    f_int16_t pZ = 0;
    f_int16_t pZ2 = 0;
    int i = 0;
    PT_3D CurPosXyz = {0};
    LPSingleCityCfg lpsinglecitycfg2rd = NULL;
    int ReList = FALSE;			//�Ƿ����¼�����ʾ�б�
    int FirstCal = FALSE;		//�Ƿ��һ�μ��㽨������

    stList_Head *pstListHead = (m_listCityCfgData);
    stList_Head *pstTmpList = NULL;
    LPSingleCityCfg lpsinglemodel =NULL;

    // 1.׼������Ⱥ������
    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
        if( NULL != pstTmpList )
        {
            lpsinglecitycfg2rd = ASDE_LIST_ENTRY(pstTmpList, SingleCityCfg, stListHead);

            // 1.1.0 �״μ��㾭γ�ȡ����ǻ����ݡ�������Alpha���ж��Ƿ��һ�μ���		//�ж��Ƿ��һ�μ��㣬��һ�εĻ���Ҫ�����ڴ�
            if ((lpsinglecitycfg2rd->WhiteCity_XYZ == NULL)|| (lpsinglecitycfg2rd->BottomTriangle_XYZ == NULL)|| (lpsinglecitycfg2rd->WhiteCity_XYZ_Integer == NULL))
            {
                lpsinglecitycfg2rd->WhiteCity_XYZ = NewFixedMemory(sizeof(PT_3D)*(lpsinglecitycfg2rd->PointNum+1)*2);
                lpsinglecitycfg2rd->BottomTriangle_XYZ = NewFixedMemory(sizeof(PT_3D)*(lpsinglecitycfg2rd->BottomPointNum));

                lpsinglecitycfg2rd->WhiteCity_XYZ_Integer = NewFixedMemory(sizeof(PT_3I)*(lpsinglecitycfg2rd->PointNum+1)*2);
                lpsinglecitycfg2rd->BottomTriangle_XYZ_Integer = NewFixedMemory(sizeof(PT_3I)*(lpsinglecitycfg2rd->BottomPointNum));

                FirstCal = TRUE;

            }

            // 1.1.1 ������ľ�γ�ȵ�
            //����XYZ: ���㽨����ĵر�߶ȣ������о�γ�ȸ߶�ת��XYZ		//����ĳһ��ĸ߶���Ϊ������ĸ߶�
			if(FALSE == getAltByGeoPos(*lpsinglecitycfg2rd->Lon, *lpsinglecitycfg2rd->Lat, &pZ))
            {	return FALSE;}

            for(i = 0; i<(lpsinglecitycfg2rd->PointNum+1)*2;i++)
            {
                if(lpsinglecitycfg2rd->PointNum*2 == i)
                {
                    //����һ���������������뵽��������һλ�������ڻ���ʱʹ��

                    //���뽨����ľ��Ը߶ȼ���
                    #if Building_HeiChoose
                    {
                        Geo_Pt_D geo_pt;

                        geo_pt.lat = *lpsinglecitycfg2rd->Lat;
                        geo_pt.lon = *lpsinglecitycfg2rd->Lon;
                        geo_pt.height = lpsinglecitycfg2rd->Hei;

                        geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->WhiteCity_XYZ+i)));
                    }

                    #else
                    {
                        Geo_Pt_D geo_pt;

                        geo_pt.lat = *lpsinglecitycfg2rd->Lat;
                        geo_pt.lon = *lpsinglecitycfg2rd->Lon;
                        geo_pt.height = pZ+lpsinglecitycfg2rd->Hei;

                        geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->WhiteCity_XYZ+i)));
                    }
                    #endif

                    i++;
                    {
                        Geo_Pt_D geo_pt;

                        geo_pt.lat = *lpsinglecitycfg2rd->Lat;
                        geo_pt.lon = *lpsinglecitycfg2rd->Lon;
                        geo_pt.height = pZ;

                        geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->WhiteCity_XYZ+i)));
                    }

                    break;
                }
                else
                {

                    //���뽨����ľ��Ը߶ȼ���
                    #if Building_HeiChoose
                    {
                        Geo_Pt_D geo_pt;

                        geo_pt.lat = *(lpsinglecitycfg2rd->Lat+i/2);
                        geo_pt.lon = *(lpsinglecitycfg2rd->Lon+i/2);
                        geo_pt.height = lpsinglecitycfg2rd->Hei;

                        geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->WhiteCity_XYZ+i)));
                    }
                    #else
                    {
                        Geo_Pt_D geo_pt;

                        geo_pt.lat =*(lpsinglecitycfg2rd->Lat+i/2);
                        geo_pt.lon = *(lpsinglecitycfg2rd->Lon+i/2);
                        geo_pt.height = pZ + lpsinglecitycfg2rd->Hei;

                        geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->WhiteCity_XYZ+i)));
                    }

                    #endif

                    i++;

                    {
                        Geo_Pt_D geo_pt;

                        geo_pt.lat =*(lpsinglecitycfg2rd->Lat+i/2);
                        geo_pt.lon = *(lpsinglecitycfg2rd->Lon+i/2);
                        geo_pt.height = pZ ;

                        geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->WhiteCity_XYZ+i)));
                    }


                }


            }

            // 1.1.2 ����������ǻ�������
			if(FALSE == getAltByGeoPos((*(lpsinglecitycfg2rd->BottomTriangle_LonLat)).x, (*(lpsinglecitycfg2rd->BottomTriangle_LonLat)).y, &pZ2))
            {	return FALSE;}

            for(i = 0; i<(lpsinglecitycfg2rd->BottomPointNum);i++)
            {
                //���뽨����ľ��Ը߶ȼ���
                #if Building_HeiChoose
                {
                    Geo_Pt_D geo_pt;

                    geo_pt.lat =(*(lpsinglecitycfg2rd->BottomTriangle_LonLat+i)).y;
                    geo_pt.lon = (*(lpsinglecitycfg2rd->BottomTriangle_LonLat+i)).x;
                    geo_pt.height = lpsinglecitycfg2rd->Hei ;

                    geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)));
                }
                #else
                {
                    Geo_Pt_D geo_pt;

                    geo_pt.lat =(*(lpsinglecitycfg2rd->BottomTriangle_LonLat+i)).y;
                    geo_pt.lon = (*(lpsinglecitycfg2rd->BottomTriangle_LonLat+i)).x;
                    geo_pt.height = pZ2+lpsinglecitycfg2rd->Hei ;

                    geoDPt2objDPt(&geo_pt, &(*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)));
                }
                #endif
            }

            // 1.1.3 ���������ķ�������,ֻ�е�һ����Ҫ����
            if(TRUE == FirstCal)
            {
                //	if(lpsinglecitycfg3rd->NormalVector == NULL)
                CalNormalVector_SingleBuilding(lpsinglecitycfg2rd);
                //����ÿ�����Alphaֵ
                CalAlpha_SingleBuilding(lpsinglecitycfg2rd);
            }



            // 1.1.4 ƫ�Ƽ���
                // 1.1.4.1 �洢��׼��
                if((g_whitecity_position.x == 0)&&(g_whitecity_position.y == 0)&&(g_whitecity_position.z == 0))
                {
                    g_whitecity_position.x = lpsinglecitycfg2rd->WhiteCity_XYZ->x;
                    g_whitecity_position.y = lpsinglecitycfg2rd->WhiteCity_XYZ->y;
                    g_whitecity_position.z = lpsinglecitycfg2rd->WhiteCity_XYZ->z;
                }

                // 1.1.4.2 ƫ�Ƽ���
                for(i = 0; i<(lpsinglecitycfg2rd->PointNum+1)*2;i++)
                {
                    ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->x -=  g_whitecity_position.x;
                    ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->y -=  g_whitecity_position.y;
                    ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->z -=  g_whitecity_position.z;

                    // 1.1.4.3 �Ŵ���
                    ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->x *= 100000.0;
                    ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->y *= 100000.0;
                    ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->z *= 100000.0;

//					printf("%f,%f,%f\n",
//						((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->x,
//						((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->y,
//						((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->z);

                    ((LP_PT_3I)(lpsinglecitycfg2rd->WhiteCity_XYZ_Integer + i ))->x
                        = ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->x;
                    ((LP_PT_3I)(lpsinglecitycfg2rd->WhiteCity_XYZ_Integer + i ))->y
                        = ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->y;
                    ((LP_PT_3I)(lpsinglecitycfg2rd->WhiteCity_XYZ_Integer + i ))->z
                        = ((LP_PT_3D)(lpsinglecitycfg2rd->WhiteCity_XYZ+i))->z;
                }



                // 1.1.4.3.���ǻ�������ƫ�Ƽ���
                {
                    for(i = 0; i<(lpsinglecitycfg2rd->BottomPointNum);i++)
                    {
                        //���뽨����ľ��Ը߶ȼ���
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).x -=  g_whitecity_position.x;
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).y -=  g_whitecity_position.y;
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).z -=  g_whitecity_position.z;


                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).x *= 100000.0;
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).y *= 100000.0;
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).z *= 100000.0;

                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ_Integer+i)).x
                            = (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).x;
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ_Integer+i)).y
                            = (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).y;
                        (*(lpsinglecitycfg2rd->BottomTriangle_XYZ_Integer+i)).z
                            = (*(lpsinglecitycfg2rd->BottomTriangle_XYZ+i)).z;
                    }


                }

        }
    }
    return TRUE;
}

int RenderWhiteCitySingle(sGLRENDERSCENE* pModelScene)
{
    double pZ = 0.0;
    double pZ2 = 0.0;
    int i = 0;
    PT_3D CurPosXyz = {0};
//	LPSingleCityCfg lpsinglecitycfg2rd = NULL;
    static  int ReList = FALSE;			//�Ƿ����¼�����ʾ�б�


        stList_Head *pstListHead = (m_listCityCfgData);
        LPSingleCityCfg lpsinglemodel =NULL;
        lpsinglemodel = ASDE_LIST_ENTRY(pstListHead->pNext, SingleCityCfg, stListHead);





    // 0.�Ƿ��ж�
    if(lpsinglemodel == NULL)
        return FALSE;

    // 0.�ж��Ƿ���Ҫ���¼��㽨��������
//	ReList = ReCalBuilding(lpsinglemodel);
    if(ReList == FALSE )
    {
        printf("���㽨��������\n");
        //1.׼������Ⱥ������
        if(FALSE == CalBuildingData(/*pNode2nd*/))
        {
            return FALSE;
        }

        ReList = TRUE;

    }
#if 1







    //2.���ƽ���Ⱥ.�ж���ʾ�б��Ƿ�Ϊ���ڣ������ڻ�����Ҫ���¼��㣬��������ʾ�б�����Ļ���ֱ�ӵ�����ʾ�б�
    if( ( lpsinglemodel->DrawListID[0] == 0)
        ||(lpsinglemodel->DrawListID[1] == 0)
        ||(ReList == TRUE)
        )
    {
        if (FALSE == GenerateCityList(m_listCityCfgData)	)
        {
            printf("���ɽ�������ʾ�б�ʧ��\n");
            return FALSE;
        }
// 		else
// 		{
// #if Building_TopConcaveDraw
// 			WhiteCityCallList_Concave(lpsinglemodel,lpsinglemodel->DrawListID[0], pModelScene);
// #else
// 			WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[0], pModelScene);
// #endif
// //			WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[1], pModelScene);
// //			WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[2], pModelScene);
//
// 		}

    }
/*	else*/
    {
//				WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[1], pModelScene);
#if Building_TopConcaveDraw
        WhiteCityCallList_Concave(lpsinglemodel,lpsinglemodel->DrawListID[0], pModelScene);
#else
        WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[0], pModelScene);

#endif

        WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[1], pModelScene);
        WhiteCityCallList(lpsinglemodel,lpsinglemodel->DrawListID[2], pModelScene);
//			printf("���ư�ģ����%d\n",lpsinglecitycfg2rd->IdNum);
    }


#endif
    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������:
**
** ����:  ���ɽ��������ʾ�б�
**
** �������:  SingleCityCfg
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �����㺯��
**
**.EH--------------------------------------------------------
*/
int GenerateCityList(stList_Head * List3rd)
{
//	int * CityList = NewAlterableMemory(sizeof(int));

// 	//���������ķ�������
// //	if(lpsinglecitycfg3rd->NormalVector == NULL)
// 	CalNormalVector_SingleBuilding(lpsinglecitycfg3rd);
// 	//����ÿ�����Alphaֵ
// 	CalAlpha_SingleBuilding(lpsinglecitycfg3rd);

    //OpenGL����
    if (FALSE == RenderCityListThree(List3rd))
    {
        return FALSE;
    }

    if (FALSE == RenderCityListFour(List3rd))
    {
        return FALSE;
    }

    if (FALSE == RenderCityListLine(List3rd))
    {
        return FALSE;
    }

// 	//�洢��ʾ�б��id��
// 	lpsinglecitycfg3rd->DrawListID = (*CityList);
//
// 	//�ͷ��ڴ�
// 	DeleteAlterableMemory(CityList);
// 	CityList = NULL;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: RenderCityListThree
**
** ����:  ������ʾ�б�--������
**
** �������:  ����
**
** �����������ʾ�б�ID
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  ���Ĳ㺯��
**
**.EH--------------------------------------------------------
*/
int RenderCityListThree(stList_Head * List3rd)
{
    int drawlistid = 0;
    int i = 0;
    int texture_i = 0;
    LPSingleCityCfg_Texture tempTexture = NULL;
    int j = 0;
// 	LPNode pNode = (LPNode)List3rd.m_pFirst;
// 	LPSingleCityCfg lpsinglecitycfg4rd = (LPSingleCityCfg)pNode->m_pCur;


    stList_Head *pstListHead = (m_listCityCfgData);
    stList_Head *pstTmpList = NULL;
    LPSingleCityCfg lpsinglecitycfg4rd =ASDE_LIST_ENTRY(m_listCityCfgData->pNext, SingleCityCfg, stListHead);
    f_float64_t pZ = 0;



    // 1.�ж���ʾ�б��Ƿ���ڣ����ڵĻ�����
    if (lpsinglecitycfg4rd->DrawListID[0] != 0)
    {
        glDeleteLists(lpsinglecitycfg4rd->DrawListID[0],1);
    }

    // 2.������ʾ�б�ID
    drawlistid = glGenLists(1);
    glNewList(drawlistid, GL_COMPILE);

//	//3.�������������޳�����
//	glFrontFace(GL_CCW);
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_BACK);
//	glDisable(GL_CULL_FACE);

//	glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */

    // 4.��ȡ����
// 	while(pNode != NULL)
// 	{
// 		lpsinglecitycfg4rd = (LPSingleCityCfg)pNode->m_pCur;



        LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
        {
            if( NULL != pstTmpList )
            {
            lpsinglecitycfg4rd = ASDE_LIST_ENTRY(pstTmpList, SingleCityCfg, stListHead);


        // 4.1 ���ƶ���								//���㼯ֻ����͹�����
//		glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */
#if 1		//lpf add
        //	glFrontFace(GL_CW);						//temp lpf add
        //	glCullFace(GL_FRONT);					//temp lpf add

#if Polygon_TopFaceRender
            glBegin(GL_POLYGON);
        {
//			glLineWidth(10);		//temp lpf add
            texture_i = 0;

            //�ж��Ƿ���ڶ�������
            for(j=0;j<lpsinglecitycfg4rd->m_bHasTexture;j++)
            {
                if ((*(lpsinglecitycfg4rd->m_TextureCity+j)).m_PosID == lpsinglecitycfg4rd->PointNum)
                {
                    tempTexture = lpsinglecitycfg4rd->m_TextureCity+j;
                    break;
                }

            }

            for(i = 0;i<(lpsinglecitycfg4rd->PointNum*2);i++)
            {
                //��ȡ��������
                if(tempTexture != NULL)
                {
                    glTexCoord2f(
                        (*(tempTexture->m_pTexverts+texture_i)).x,
                        (*(tempTexture->m_pTexverts+texture_i)).y);
                    texture_i++;
                }
                else
                {
                    //glColor4f(ColorIndex,Alpha_TopBuilding);
                    glColor4f(ColorIndex*Alpha_TopBuilding,
                        ColorIndex*Alpha_TopBuilding,
                        ColorIndex*Alpha_TopBuilding,
                        AlphaIndex);
                }
                //����һ����
                glVertex3i(
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+i)).x,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+i)).y,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+i)).z );
                i++;
            }
        }
        glEnd();
#else
        //	glBegin(GL_TRIANGLE_STRIP);
        glBegin(GL_TRIANGLES);
        {
            for(i = 0;i<(lpsinglecitycfg4rd->BottomPointNum);i++)
            {
                // 			//��ȡ��������
                // 			if(tempTexture != NULL)
                // 			{
                // 				glTexCoord2f(
                // 					(*(tempTexture->m_pTexverts+texture_i)).x,
                // 					(*(tempTexture->m_pTexverts+texture_i)).y);
                // 				texture_i++;
                // 			}
                // 			else
                // 			{
                glColor4f(ColorIndex*Alpha_TopBuilding,
                    ColorIndex*Alpha_TopBuilding,
                    ColorIndex*Alpha_TopBuilding,
                    AlphaIndex);
                // 			}
                //����һ����
                glVertex3i(
                    (*(lpsinglecitycfg4rd->BottomTriangle_XYZ_Integer+i)).x,
                    (*(lpsinglecitycfg4rd->BottomTriangle_XYZ_Integer+i)).y,
                    (*(lpsinglecitycfg4rd->BottomTriangle_XYZ_Integer+i)).z );

            }
        }
        glEnd();
#endif

#endif
//		glPopAttrib();   /**< �ָ�ǰһ���� */

//		pNode = pNode->m_pNext;
            }
    }

//	glPopAttrib();   /**< �ָ�ǰһ���� */

    // 5.������ʾ�б�
    glEndList();

//	// 6.������ʾ�б�
//	WhiteCityCallList(lpsinglecitycfg4rd,drawlistid);
    //	printf("�״λ��ư�ģ����\n");

    // 7.�洢��ʾ�б�ID
    lpsinglecitycfg4rd =ASDE_LIST_ENTRY(m_listCityCfgData->pNext, SingleCityCfg, stListHead);
    lpsinglecitycfg4rd->DrawListID[0] = drawlistid;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: RenderCityListFour
**
** ����:  ������ʾ�б�--�ı���
**
** �������:  ��������
**
** �����������ʾ�б�ID
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  ���Ĳ㺯��
**
**.EH--------------------------------------------------------
*/
int RenderCityListFour(stList_Head * List3rd)
{
    int drawlistid = 0;
    int i = 0;
    int texture_i = 0;
    LPSingleCityCfg_Texture tempTexture = NULL;
    int j = 0;
// 	LPNode pNode = (LPNode)List3rd.m_pFirst;
// 	LPSingleCityCfg lpsinglecitycfg4rd = (LPSingleCityCfg)pNode->m_pCur;
    double temp_RGBA = 0;

    stList_Head *pstListHead = (m_listCityCfgData);
    stList_Head *pstTmpList = NULL;
    LPSingleCityCfg lpsinglecitycfg4rd =ASDE_LIST_ENTRY(m_listCityCfgData->pNext, SingleCityCfg, stListHead);



    //1.�ж���ʾ�б��Ƿ���ڣ����ڵĻ�����
    if (lpsinglecitycfg4rd->DrawListID[1] != 0)
    {
        glDeleteLists(lpsinglecitycfg4rd->DrawListID[1],1);
    }

    //2.������ʾ�б�ID
    drawlistid = glGenLists(1);
    glNewList(drawlistid, GL_COMPILE);

    //3.�������������޳�����
//	glFrontFace(GL_CCW);
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_BACK);
//	glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */

    //4.��ȡ����
// 	while(pNode != NULL)
// 	{
// 		lpsinglecitycfg4rd = (LPSingleCityCfg)pNode->m_pCur;

        LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
        {
            if( NULL != pstTmpList )
            {
            lpsinglecitycfg4rd = ASDE_LIST_ENTRY(pstTmpList, SingleCityCfg, stListHead);

        /** ���Ʋ��� */
#if 1
//		glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */
        /*	glColor4f(ColorIndex);*/
        glBegin(GL_QUADS);
        {
            //�ֱ����ÿ����
            for(i = 0;i<(lpsinglecitycfg4rd->PointNum);i++)
            {
                temp_RGBA = ColorIndex *(*(lpsinglecitycfg4rd->AlpherVector+i));	//ÿ�����RGBֵ
                //�ж��Ƿ��������
                for(j=0;j<lpsinglecitycfg4rd->m_bHasTexture;j++)
                {
                    if ((*(lpsinglecitycfg4rd->m_TextureCity+j)).m_PosID == (i))
                    {
                        tempTexture = lpsinglecitycfg4rd->m_TextureCity+j;
                        break;
                    }
                }

                //����ǰ������
                for (j=0;j<2;j++)
                {
                    //��ȡ��������
                    if(tempTexture != NULL)
                    {
                        glTexCoord2f(
                            (*(tempTexture->m_pTexverts+j)).x,
                            (*(tempTexture->m_pTexverts+j)).y);
                    }
                    else
                    {
                        //glColor4f(ColorIndex,*(lpsinglecitycfg4rd->AlpherVector+i));
                        //glColor4f(ColorIndex*(*(lpsinglecitycfg4rd->AlpherVector+i)),0.75*(*(lpsinglecitycfg4rd->AlpherVector+i)));

                        glColor4f(temp_RGBA,temp_RGBA,temp_RGBA,AlphaIndex);
                    }
                    glVertex3i(
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).x,
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).y,
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).z );

                }
                //  2��3�ĵ�˳��ߵ�
                {
                    //����3�ŵ�
                    j=3;
                    //��ȡ��������
                    if(tempTexture != NULL)
                    {
                        glTexCoord2f(
                            (*(tempTexture->m_pTexverts+j)).x,
                            (*(tempTexture->m_pTexverts+j)).y);
                    }
                    else
                    {
                        //glColor4f(ColorIndex,*(lpsinglecitycfg4rd->AlpherVector+i));
                        //glColor4f(ColorIndex*(*(lpsinglecitycfg4rd->AlpherVector+i)),0.75*(*(lpsinglecitycfg4rd->AlpherVector+i)));
                        glColor4f(temp_RGBA,temp_RGBA,temp_RGBA,AlphaIndex);
                    }
                    glVertex3i(
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).x,
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).y,
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).z );

                    //����2�ŵ�
                    j=2;
                    //��ȡ��������
                    if(tempTexture != NULL)
                    {
                        glTexCoord2f(
                            (*(tempTexture->m_pTexverts+j)).x,
                            (*(tempTexture->m_pTexverts+j)).y);
                    }
                    else
                    {
                        //glColor4f(ColorIndex,*(lpsinglecitycfg4rd->AlpherVector+i));
                        //glColor4f(ColorIndex*(*(lpsinglecitycfg4rd->AlpherVector+i)),0.75*(*(lpsinglecitycfg4rd->AlpherVector+i)));
                        glColor4f(temp_RGBA,temp_RGBA,temp_RGBA,AlphaIndex);
                    }
                    glVertex3i(
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).x,
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).y,
                        (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+j)).z );

                }


                tempTexture = NULL;
            }
        }
        glEnd();
//		glPopAttrib();							/**< �ָ�ǰһ���� */
#else

{
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_QUADS);
        glVertex3i(0, 0, 0 );
        glVertex3i(1000, 0, 0 );
        glVertex3i(1000, 1000, 0 );
        glVertex3i(0, 1000, 0 );
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_QUADS);
        glVertex3i(0, 0, 0 );
        glVertex3i(0, 1000, 0 );
        glVertex3i(0, 1000, 1000);
        glVertex3i(0, 0, 1000);
    glEnd();

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
        glVertex3i(0, 0, 0 );
        glVertex3i(1000, 0, 0);
        glVertex3i(1000, 0, 1000 );
        glVertex3i(0, 0, 1000 );
    glEnd();
}

#endif

//		pNode = pNode->m_pNext;
    }
    }



//	glPopAttrib();							/**< �ָ�ǰһ���� */
//	glDisable(GL_CULL_FACE);

    //5.������ʾ�б�
    glEndList();

//	//6.������ʾ�б�
//	WhiteCityCallList(lpsinglecitycfg4rd,drawlistid);
    //	printf("�״λ��ư�ģ����\n");

    //7.�洢��ʾ�б�ID
//	((LPSingleCityCfg)m_listCityCfgData.m_pFirst->m_pCur)->DrawListID[1] = drawlistid;
    lpsinglecitycfg4rd =ASDE_LIST_ENTRY(m_listCityCfgData->pNext, SingleCityCfg, stListHead);
    lpsinglecitycfg4rd->DrawListID[1] = drawlistid;
    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: RenderCityListLine
**
** ����:  ������ʾ�б�--�߿�
**
** �������:  ����
**
** �����������ʾ�б�ID
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  ���Ĳ㺯��
**
**.EH--------------------------------------------------------
*/
int RenderCityListLine(stList_Head * List3rd)
{
    int drawlistid = 0;
    int i = 0;
    int texture_i = 0;
    LPSingleCityCfg_Texture tempTexture = NULL;
    int j = 0;
// 	LPNode pNode = (LPNode)List3rd.m_pFirst;
// 	LPSingleCityCfg lpsinglecitycfg4rd = (LPSingleCityCfg)pNode->m_pCur;

    stList_Head *pstListHead = (m_listCityCfgData);
    stList_Head *pstTmpList = NULL;
    LPSingleCityCfg lpsinglecitycfg4rd =ASDE_LIST_ENTRY(m_listCityCfgData->pNext, SingleCityCfg, stListHead);


    //1.�ж���ʾ�б��Ƿ���ڣ����ڵĻ�����
    if (lpsinglecitycfg4rd->DrawListID[2] != 0)
    {
        glDeleteLists(lpsinglecitycfg4rd->DrawListID[2],1);
    }

    //2.������ʾ�б�ID
    drawlistid = glGenLists(1);
    glNewList(drawlistid, GL_COMPILE);

    //3.�������������޳�����
//	glFrontFace(GL_CCW);
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_BACK);
//	glDisable(GL_CULL_FACE);

//	glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */

    //4.��ȡ����
// 	while(pNode != NULL)
// 	{
// 		lpsinglecitycfg4rd = (LPSingleCityCfg)pNode->m_pCur;
    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
        if( NULL != pstTmpList )
        {
            lpsinglecitycfg4rd = ASDE_LIST_ENTRY(pstTmpList, SingleCityCfg, stListHead);

        //4.1 �������
#if Line_BuildingShow
        //		glPushAttrib(GL_CURRENT_BIT);

        // 	glColor4f(ColorIndex*Alpha_TopBuilding,
        // 			  ColorIndex*Alpha_TopBuilding,
        // 			  ColorIndex*Alpha_TopBuilding,
        // 			  AlphaIndex);
        glColor4f(0.3,0.3,0.3,AlphaIndex);
        glLineWidth(1.0);

        // 	//�������¿�
        // 	glBegin(GL_LINE_LOOP);
        // 	{
        // 		//�����¿�
        // 		for(i = 0;i<(lpsinglecitycfg4rd->PointNum*2);i++)
        // 		{
        // 			glVertex3f(
        // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).x,
        // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).y,
        // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).z );
        // 			i++;
        // 		}
        // 	}
        // 	glEnd();
        // 	glBegin(GL_LINE_LOOP);
        // 	{
        // 		//�����Ͽ�
        // 		for(i = 1;i<(lpsinglecitycfg4rd->PointNum*2);i++)
        // 		{
        // 			glVertex3f(
        // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).x,
        // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).y,
        // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).z );
        // 			i++;
        // 		}
        // 	}
        // 	glEnd();
        //���Ʊ߿�
        for (i=0;i<lpsinglecitycfg4rd->PointNum;i++)
        {
            //		glBegin(GL_LINE_LOOP);
            glBegin(GL_LINE_STRIP);
            {
                //����0��1��3��2 modify to 1��0��2��3
                glVertex3i(
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+1)).x,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+1)).y,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+1)).z );
                glVertex3i(
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+0)).x,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+0)).y,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+0)).z );
                glVertex3i(
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+2)).x,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+2)).y,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+2)).z );
                glVertex3i(
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+3)).x,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+3)).y,
                    (*(lpsinglecitycfg4rd->WhiteCity_XYZ_Integer+2*i+3)).z );

            }

            glEnd();
        }
        //		glPopAttrib();

#endif

//		pNode = pNode->m_pNext;
    }
    }

//	glPopAttrib();   /**< �ָ�ǰһ���� */

    //5.������ʾ�б�
    glEndList();

//	//6.������ʾ�б�
//	WhiteCityCallList(lpsinglecitycfg4rd,drawlistid);
    //	printf("�״λ��ư�ģ����\n");

    //7.�洢��ʾ�б�ID
//	((LPSingleCityCfg)m_listCityCfgData.m_pFirst->m_pCur)->DrawListID[2] = drawlistid;
    lpsinglecitycfg4rd =ASDE_LIST_ENTRY(m_listCityCfgData->pNext, SingleCityCfg, stListHead);
    lpsinglecitycfg4rd->DrawListID[2] = drawlistid;

    return TRUE;
}


/*.BH--------------------------------------------------------
// **
// ** ������: RenderCityList
// **
// ** ����:  ������ʾ�б�
// **
// ** �������:  ��������
// **
// ** �����������ʾ�б�ID
// **
// ** ����ֵ��TRUE �ɹ�or FALSE ʧ��
// **
// **
// ** ���ע��:  ���Ĳ㺯��
// **
// **.EH--------------------------------------------------------
// */
// int RenderCityList(int* ListID,LPSingleCityCfg lpsinglecitycfg4rd)
// {
// 	int drawlistid = 0;
// 	int i = 0;
// 	int texture_i = 0;
// 	LPSingleCityCfg_Texture tempTexture = NULL;
// 	int j = 0;
//
// 	//lpf add temp
// 	double temp_RGBA = 0;
//
// 	//�ж���ʾ�б��Ƿ���ڣ����ڵĻ�����
// 	if (lpsinglecitycfg4rd->DrawListID != 0)
// 	{
// 		glDeleteLists(lpsinglecitycfg4rd->DrawListID,1);
// 	}
// 	// ������ʾ�б�ID
// 	drawlistid = glGenLists(1);
// 	glNewList(drawlistid, GL_COMPILE);
//
// 	//�������������޳�����
// 	glFrontFace(GL_CCW);
// 	glEnable(GL_CULL_FACE);
// 	glCullFace(GL_BACK);
//
// 	/** ���Ʋ��� */
// #if 1
//
// 	glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */
// /*	glColor4f(ColorIndex);*/
// 	glBegin(GL_QUADS);
// 	{
// 		//�ֱ����ÿ����
// 		for(i = 0;i<(lpsinglecitycfg4rd->PointNum);i++)
// 		{
// 			temp_RGBA = ColorIndex *(*(lpsinglecitycfg4rd->AlpherVector+i));	//ÿ�����RGBֵ
// 			//�ж��Ƿ��������
// 			for(j=0;j<lpsinglecitycfg4rd->m_bHasTexture;j++)
// 			{
// 				if ((*(lpsinglecitycfg4rd->m_TextureCity+j)).m_PosID == (i))
// 				{
// 					tempTexture = lpsinglecitycfg4rd->m_TextureCity+j;
// 					break;
// 				}
// 			}
//
// 			//����ǰ������
// 			for (j=0;j<2;j++)
// 			{
// 				//��ȡ��������
// 				if(tempTexture != NULL)
// 				{
// 					glTexCoord2f(
// 						(*(tempTexture->m_pTexverts+j)).x,
// 						(*(tempTexture->m_pTexverts+j)).y);
// 				}
// 				else
// 				{
// 					//glColor4f(ColorIndex,*(lpsinglecitycfg4rd->AlpherVector+i));
// 					//glColor4f(ColorIndex*(*(lpsinglecitycfg4rd->AlpherVector+i)),0.75*(*(lpsinglecitycfg4rd->AlpherVector+i)));
//
// 					glColor4f(temp_RGBA,temp_RGBA,temp_RGBA,AlphaIndex);
// 				}
// 				glVertex3f(
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).x,
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).y,
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).z );
//
// 			}
// 			//  2��3�ĵ�˳��ߵ�
// 			{
// 				//����3�ŵ�
// 				j=3;
// 				//��ȡ��������
// 				if(tempTexture != NULL)
// 				{
// 					glTexCoord2f(
// 						(*(tempTexture->m_pTexverts+j)).x,
// 						(*(tempTexture->m_pTexverts+j)).y);
// 				}
// 				else
// 				{
// 					//glColor4f(ColorIndex,*(lpsinglecitycfg4rd->AlpherVector+i));
// 					//glColor4f(ColorIndex*(*(lpsinglecitycfg4rd->AlpherVector+i)),0.75*(*(lpsinglecitycfg4rd->AlpherVector+i)));
// 						glColor4f(temp_RGBA,temp_RGBA,temp_RGBA,AlphaIndex);
// 				}
// 				glVertex3f(
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).x,
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).y,
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).z );
//
// 				//����2�ŵ�
// 				j=2;
// 				//��ȡ��������
// 				if(tempTexture != NULL)
// 				{
// 					glTexCoord2f(
// 						(*(tempTexture->m_pTexverts+j)).x,
// 						(*(tempTexture->m_pTexverts+j)).y);
// 				}
// 				else
// 				{
// 					//glColor4f(ColorIndex,*(lpsinglecitycfg4rd->AlpherVector+i));
// 					//glColor4f(ColorIndex*(*(lpsinglecitycfg4rd->AlpherVector+i)),0.75*(*(lpsinglecitycfg4rd->AlpherVector+i)));
// 					glColor4f(temp_RGBA,temp_RGBA,temp_RGBA,AlphaIndex);
// 				}
// 				glVertex3f(
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).x,
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).y,
// 						(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+j)).z );
//
// 			}
//
//
// 			tempTexture = NULL;
// 		}
// 	}
// 	glEnd();
// 	glPopAttrib();							/**< �ָ�ǰһ���� */
//
// #endif
// 	glDisable(GL_CULL_FACE);
//
// 	// ���ƶ���								//���㼯ֻ����͹�����
// 	glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */
// #if 1		//lpf add
// //	glFrontFace(GL_CW);						//temp lpf add
// //	glCullFace(GL_FRONT);					//temp lpf add
//
// #if Polygon_TopFaceRender
// //	glBegin(GL_POLYGON);		//temp lpf delete
// 	glBegin(GL_LINE_LOOP);		//temp lpf add
// //	glBegin(GL_POINT);			//temp lpf add
// //	glBegin(GL_TRIANGLES);		//temp lpf add
// 	{
// 		glLineWidth(10);		//temp lpf add
// 		texture_i = 0;
//
// 		//�ж��Ƿ���ڶ�������
// 		for(j=0;j<lpsinglecitycfg4rd->m_bHasTexture;j++)
// 		{
// 			if ((*(lpsinglecitycfg4rd->m_TextureCity+j)).m_PosID == lpsinglecitycfg4rd->PointNum)
// 			{
// 				tempTexture = lpsinglecitycfg4rd->m_TextureCity+j;
// 				break;
// 			}
//
// 		}
//
// 		for(i = 0;i<(lpsinglecitycfg4rd->PointNum*2);i++)
// 		{
// 			//��ȡ��������
// 			if(tempTexture != NULL)
// 			{
// 				glTexCoord2f(
// 					(*(tempTexture->m_pTexverts+texture_i)).x,
// 					(*(tempTexture->m_pTexverts+texture_i)).y);
// 				texture_i++;
// 			}
// 			else
// 			{
// 				//glColor4f(ColorIndex,Alpha_TopBuilding);
// 				glColor4f(ColorIndex*Alpha_TopBuilding,
// 						  ColorIndex*Alpha_TopBuilding,
// 						  ColorIndex*Alpha_TopBuilding,
// 						  AlphaIndex);
// 			}
// 			//����һ����
// 			glVertex3f(
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).x,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).y,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).z );
// 			i++;
// 		}
// 	}
// 	glEnd();
// #else
// //	glBegin(GL_TRIANGLE_STRIP);
// 	glBegin(GL_TRIANGLES);
// 	{
// 		for(i = 0;i<(lpsinglecitycfg4rd->BottomPointNum);i++)
// 		{
// // 			//��ȡ��������
// // 			if(tempTexture != NULL)
// // 			{
// // 				glTexCoord2f(
// // 					(*(tempTexture->m_pTexverts+texture_i)).x,
// // 					(*(tempTexture->m_pTexverts+texture_i)).y);
// // 				texture_i++;
// // 			}
// // 			else
// // 			{
// 			glColor4f(ColorIndex*Alpha_TopBuilding,
// 					  ColorIndex*Alpha_TopBuilding,
// 					  ColorIndex*Alpha_TopBuilding,
// 					  AlphaIndex);
// // 			}
// 			//����һ����
// 			glVertex3f(
// 				(*(lpsinglecitycfg4rd->BottomTriangle_XYZ+i)).x,
// 				(*(lpsinglecitycfg4rd->BottomTriangle_XYZ+i)).y,
// 				(*(lpsinglecitycfg4rd->BottomTriangle_XYZ+i)).z );
//
// 		}
// 	}
// 	glEnd();
// #endif
//
// #endif
// 	glPopAttrib();   /**< �ָ�ǰһ���� */
//
// 	//�������
// #if Line_BuildingShow
// 	glPushAttrib(GL_CURRENT_BIT);
//
// // 	glColor4f(ColorIndex*Alpha_TopBuilding,
// // 			  ColorIndex*Alpha_TopBuilding,
// // 			  ColorIndex*Alpha_TopBuilding,
// // 			  AlphaIndex);
// 	glColor4f(0.3,0.3,0.3,AlphaIndex);
// 	glLineWidth(1.0);
//
// // 	//�������¿�
// // 	glBegin(GL_LINE_LOOP);
// // 	{
// // 		//�����¿�
// // 		for(i = 0;i<(lpsinglecitycfg4rd->PointNum*2);i++)
// // 		{
// // 			glVertex3f(
// // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).x,
// // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).y,
// // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).z );
// // 			i++;
// // 		}
// // 	}
// // 	glEnd();
// // 	glBegin(GL_LINE_LOOP);
// // 	{
// // 		//�����Ͽ�
// // 		for(i = 1;i<(lpsinglecitycfg4rd->PointNum*2);i++)
// // 		{
// // 			glVertex3f(
// // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).x,
// // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).y,
// // 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+i)).z );
// // 			i++;
// // 		}
// // 	}
// // 	glEnd();
// 	//���Ʊ߿�
// 	for (i=0;i<lpsinglecitycfg4rd->PointNum;i++)
// 	{
// //		glBegin(GL_LINE_LOOP);
// 		glBegin(GL_LINE_STRIP);
// 		{
// 			//����0��1��3��2 modify to 1��0��2��3
// 			glVertex3f(
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+1)).x,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+1)).y,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+1)).z );
// 			glVertex3f(
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+0)).x,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+0)).y,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+0)).z );
// 			glVertex3f(
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+2)).x,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+2)).y,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+2)).z );
// 			glVertex3f(
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+3)).x,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+3)).y,
// 				(*(lpsinglecitycfg4rd->WhiteCity_XYZ+2*i+3)).z );
//
// 		}
//
// 		glEnd();
// 	}
// 	glPopAttrib();
//
// #endif
//
// 	//������ʾ�б�
// 	glEndList();
//
// 	//������ʾ�б�
// 	WhiteCityCallList(lpsinglecitycfg4rd,drawlistid);
//
// //	printf("�״λ��ư�ģ����\n");
//
// 	*ListID = drawlistid;
// 	return TRUE;
// }


/*.BH--------------------------------------------------------
**
** ������: CheckBulidingInFrustum
**
** ����:  �ж�ĳһ�������Ƿ����Ӿ�����
**
** �������:  LPSingleCityCfg����
**
** ������������ĵ����ꡢ�뾶
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  ���ߺ���
**
**.EH--------------------------------------------------------
*/
int CheckBulidingInFrustum(LPSingleCityCfg lpsinglecitycfg_used, sGLRENDERSCENE* pModelScene)
{
//	LPNode pNode = NULL;
    f_float64_t AvgLon = 0;
    f_float64_t AvgLat = 0;
    int i;
    f_float64_t ModelX = 0;
    f_float64_t ModelY = 0;
    f_float64_t ModelZ = 0;
    f_float64_t radius = CityRadiusSize;
    f_float64_t dis = 0;
    f_float64_t dislon = 0;
    f_float64_t dislat = 0;
    f_float64_t ModelXdis = 0;
    f_float64_t ModelYdis = 0;
    f_float64_t ModelZdis = 0;
    f_int16_t pZ = 0;

    //�������ĵ�����
    if (lpsinglecitycfg_used->AvgLonLat[0] == 0
        || lpsinglecitycfg_used->AvgLonLat[1]  == 0
        || lpsinglecitycfg_used->AvgLonLat[2]  == 0
        )
    {



        {
            stList_Head *pstListHead = (m_listCityCfgData);
            stList_Head *pstTmpList = NULL;
            LPSingleCityCfg lpsinglemodel =NULL;
            f_float64_t pZ = 0;

            // 1.���ҽڵ�
            LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
            {
                if( NULL != pstTmpList )
                {
                    lpsinglemodel = ASDE_LIST_ENTRY(pstTmpList, SingleCityCfg, stListHead);

                    //���ĵ������
                    for(i=0;i<lpsinglemodel->PointNum;i++)
                    {
                        AvgLat += *(lpsinglemodel->Lat+i);
                        AvgLon += *(lpsinglemodel->Lon+i);
                    }
                    AvgLat = AvgLat/i;
                    AvgLon = AvgLon/i;

                }
            }






        }













        //���㵱�ظ߶�
		if(FALSE == getAltByGeoPos(AvgLon,AvgLat,&pZ))
            return FALSE;
        //ת��Ϊ��������
#if Building_HeiChoose
//		SphereToXYZ(DEGREE_TO_RADIAN(AvgLon),
//			DEGREE_TO_RADIAN(AvgLat),
//			lpsinglecitycfg_used->Hei,
//			&ModelX, &ModelY, &ModelZ, EARTH_RADIUS);



            {
                Geo_Pt_D geo_pt;
                Obj_Pt_D  xyz_t;

                geo_pt.lat =AvgLat;
                geo_pt.lon = AvgLon;
                geo_pt.height = lpsinglecitycfg_used->Hei;

                geoDPt2objDPt(&geo_pt, &xyz_t);


                ModelX = xyz_t.x;
                ModelY = xyz_t.y;
                ModelZ = xyz_t.z;



            }




#else
//		SphereToXYZ(DEGREE_TO_RADIAN(AvgLon),
//			DEGREE_TO_RADIAN(AvgLat),
//			pZ+lpsinglecitycfg_used->Hei,
//			&ModelX, &ModelY, &ModelZ, EARTH_RADIUS);





            {
                Geo_Pt_D geo_pt;
                Obj_Pt_D  xyz_t;

                geo_pt.lat =AvgLat;
                geo_pt.lon = AvgLon;
                geo_pt.height = pZ+lpsinglecitycfg_used->Hei;

                geoDPt2objDPt(&geo_pt, &xyz_t);


                ModelX = xyz_t.x;
                ModelY = xyz_t.y;
                ModelZ = xyz_t.z;



            }









#endif

        lpsinglecitycfg_used->AvgLonLat[0] = ModelX;
        lpsinglecitycfg_used->AvgLonLat[1] = ModelY;
        lpsinglecitycfg_used->AvgLonLat[2] = ModelZ;
    }

    //�������뾶
    if (lpsinglecitycfg_used->radius == 0 )
    {
        //����������뾶
        for (i=0;i<lpsinglecitycfg_used->PointNum;i++)
        {
            if ((dislat*dislat + dislon*dislon) < ((*(lpsinglecitycfg_used->Lat+i) - AvgLat)*(*(lpsinglecitycfg_used->Lat+i) - AvgLat)
                +(*(lpsinglecitycfg_used->Lon+i) - AvgLon)*(*(lpsinglecitycfg_used->Lon+i) - AvgLon)))
            {
                dislat = *(lpsinglecitycfg_used->Lat+i) - AvgLat;
                dislon = *(lpsinglecitycfg_used->Lon+i) - AvgLon;
            }
        }



        //���㵱�ظ߶�
		if(FALSE == getAltByGeoPos(AvgLon+dislon,AvgLat+dislat,&pZ))
            return FALSE;
        //ת��Ϊ��������
#if Building_HeiChoose
//		SphereToXYZ(DEGREE_TO_RADIAN(AvgLon+dislon),
//			DEGREE_TO_RADIAN(AvgLat+dislat),
//			lpsinglecitycfg_used->Hei,
//			&ModelXdis, &ModelYdis, &ModelZdis, EARTH_RADIUS);




            {
                Geo_Pt_D geo_pt;
                Obj_Pt_D  xyz_t;

                geo_pt.lat =AvgLat+dislat;
                geo_pt.lon = AvgLon+dislon;
                geo_pt.height = lpsinglecitycfg_used->Hei;

                geoDPt2objDPt(&geo_pt, &xyz_t);


                ModelXdis = xyz_t.x;
                ModelYdis = xyz_t.y;
                ModelZdis = xyz_t.z;



            }














#else
//		SphereToXYZ(DEGREE_TO_RADIAN(AvgLon+dislon),
//			DEGREE_TO_RADIAN(AvgLat+dislat),
//			pZ+lpsinglecitycfg_used->Hei,
//			&ModelXdis, &ModelYdis, &ModelZdis, EARTH_RADIUS);





            {
                Geo_Pt_D geo_pt;
                Obj_Pt_D  xyz_t;

                geo_pt.lat =AvgLat+dislat;
                geo_pt.lon = AvgLon+dislon;
                geo_pt.height =pZ+ lpsinglecitycfg_used->Hei;

                geoDPt2objDPt(&geo_pt, &xyz_t);


                ModelXdis = xyz_t.x;
                ModelYdis = xyz_t.y;
                ModelZdis = xyz_t.z;



            }












#endif

        //����뾶
        lpsinglecitycfg_used->radius =sqrt((ModelXdis-ModelX)*(ModelXdis-ModelX)
            +(ModelYdis-ModelY)*(ModelYdis-ModelY)
            +(ModelZdis-ModelZ)*(ModelZdis-ModelZ));
//		printf("%f\n",lpsinglecitycfg_used->radius);
    }


    if(InFrustum(GetView_near(pModelScene), GetView_far(pModelScene),
                lpsinglecitycfg_used->AvgLonLat[0],
                 lpsinglecitycfg_used->AvgLonLat[1],
                 lpsinglecitycfg_used->AvgLonLat[2],
                 lpsinglecitycfg_used->radius, GetFrustumModel(pModelScene), &dis) == FALSE)
    {
//		printf("���������Ӿ��巶Χ%d\n",lpsinglecitycfg_used->IdNum);
        return FALSE;
    }

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: InitList
**
** ����:  ��ʼ������
**
** �������:  LPListָ��
**
** �����������
**
** ����ֵ����
**
**
** ���ע��:  ���ߺ���
**
**.EH--------------------------------------------------------
*
static void InitList(LPList pList)
{
    if(pList != NULL)
    {
        pList->m_pFirst = NULL;
        pList->m_pLast = NULL;
        pList->m_nSize = 0;
    }
}
*/
/*.BH--------------------------------------------------------
**
** ������: WhiteCityBmpCompare
**
** ����:  �ȶ�ͼƬ�Ƿ���ͬ
**
** �������:  ͼƬ����char[255]
**
** �����������
**
** ����ֵ��TRUE:��ͬ
**		   	      FALSE:��ͬ
**
**
** ���ע��:  ���ߺ���
**
**.EH--------------------------------------------------------
*/
int WhiteCityBmpCompare(const char ComparedBmpName[255], unsigned int* TextureID)
{

    stList_Head *pstListHead = (m_listCityCfgData);
    stList_Head *pstTmpList = NULL;
//	LPSingleCityCfg lpsinglemodel =NULL;

    LPSingleCityCfg pData = NULL;
    int i=0;

    //���������
// 	pNode = (LPNode)m_listCityCfgData.m_pFirst;
// 	while(pNode != NULL)
// 	{
// 		pData = (LPSingleCityCfg)pNode->m_pCur;

        // 1.���ҽڵ�
        LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
        {
            if( NULL != pstTmpList )
            {
            pData = ASDE_LIST_ENTRY(pstTmpList, SingleCityCfg, stListHead);




        for (i=0;i<pData->m_bHasTexture;i++)
        {
            //�ж������Ƿ���ͬ
            if ( 0 == strcmp((*(pData->m_TextureCity+i)).m_strname , ComparedBmpName))
            {
                *TextureID = (*(pData->m_TextureCity+i)).TextureID;
                return TRUE;
            }
        }
//		pNode = pNode->m_pNext;
    }
        }
    return FALSE;

}

/*.BH--------------------------------------------------------
**
** ������: CalNormalVector_SingleBuilding
**
** ����: ���㷨�ߺ͸�������ĵ�
**
** �������:  ÿ�������Ľṹ��
**
** �����������������, ���ĵ�����
**
** ����ֵ��TRUE:�ɹ�
**		   	      FALSE:ʧ��
**
**
** ���ע��:  ���߼��㺯��1
**
**.EH--------------------------------------------------------
*/
BOOL CalNormalVector_SingleBuilding(LPSingleCityCfg LPSingleCityCfg_Normal)
{
    int i = 0;
    int ret = TRUE;

    //���������棬���������ķ�����
    for(i=0;i<LPSingleCityCfg_Normal->PointNum*2;)
    {
        if(FALSE == CalNormalVector_SingleFace(LPSingleCityCfg_Normal->WhiteCity_XYZ+i,
                                               LPSingleCityCfg_Normal->WhiteCity_XYZ+1+i,
                                               LPSingleCityCfg_Normal->WhiteCity_XYZ+2+i,
                                               LPSingleCityCfg_Normal->WhiteCity_XYZ+3+i,
                                               LPSingleCityCfg_Normal->NormalVector+ i/2,
                                               LPSingleCityCfg_Normal->MiddlePointPerFace+ i/2))
        {
             return ret = FALSE;
        }

        i++;
        i++;

    }

    return ret;

}



/*.BH--------------------------------------------------------
**
** ������: CalNormalVector_SingleFace
**
** ����: ����ÿ����ķ��ߺ����ĵ�����
**
** �������:  ��������������
**
** ����������������������ĵ�����
**
** ����ֵ��TRUE:�ɹ�
**		   	      FALSE:ʧ��
**
**
** ���ע��:  ���߼��㺯��2
**
**.EH--------------------------------------------------------
*/
BOOL CalNormalVector_SingleFace(LP_PT_3D PointVextor1, LP_PT_3D PointVextor2, LP_PT_3D PointVextor3, LP_PT_3D PointVextor4,LP_PT_3D NormalVector, LP_PT_3D MiddlePoint)
{
//	PT_3D vVector1 = {0}, vVector2={0}, vNormal={0};
#if WIN32
    LP_PT_3D vVector = (LP_PT_3D)malloc(sizeof(PT_3D)*3);
    LP_PT_3D vVector2 = (LP_PT_3D)malloc(sizeof(PT_3D)*4);
#else
    LP_PT_3D vVector = (LP_PT_3D)NewAlterableMemory(sizeof(PT_3D)*3);
    LP_PT_3D vVector2 = (LP_PT_3D)NewAlterableMemory(sizeof(PT_3D)*4);
#endif


    /** ������ķ����� */
     /**< ��ö���ε�ʸ�� */
    vector3DSub(vVector, PointVextor1, PointVextor2);
    /**< ��ö���εĵڶ���ʸ�� */
    vector3DSub(vVector+1, PointVextor2, PointVextor3);
    /**< ��������ʸ���Ĳ�� */
    vector3DCrossProduct(vVector+2, vVector+1, vVector);
    /**< ��һ����� */
    vector3DNormalize(vVector+2);
    /**< ����������ӵ��������б��� */
    vector3DSetValue(NormalVector, vVector+2);

    //�������ĵ�����
    vector3DAdd(vVector2, PointVextor1, PointVextor2);
    vector3DAdd(vVector2+1, PointVextor3, PointVextor4);
    vector3DAdd(vVector2+2, vVector2,vVector2+1);
    vector3DMul(vVector2+3, vVector2+2, 0.25);
    vector3DSetValue(MiddlePoint, vVector2+3);
#if WIN32
    free(vVector);
    free(vVector2);
#else
    DeleteAlterableMemory(vVector);
    DeleteAlterableMemory(vVector2);
#endif


    return TRUE;
}


/*.BH--------------------------------------------------------
**
** ������: CalAlpha_SingleBuilding
**
** ����: ����ÿ���������͸����ֵ
**
** �������:  ����������ṹ��
**
** ���������͸����ֵ��
**
** ����ֵ��TRUE:�ɹ�
**		   	      FALSE:ʧ��
**
**
** ���ע��:  ͸����ֵ���㺯��1
**
**.EH--------------------------------------------------------
*/
BOOL CalAlpha_SingleBuilding(LPSingleCityCfg LPSingleCityCfg_Alpha)
{
    int i=0;
    //����ÿ�����Alphaֵ
    for(i=0;i<LPSingleCityCfg_Alpha->PointNum;i++)
    {
        CalAlpha_SingleFace(LPSingleCityCfg_Alpha->NormalVector+i,
                        LPSingleCityCfg_Alpha->MiddlePointPerFace+i,
                        LPSingleCityCfg_Alpha->AlpherVector+i);
    }

    return TRUE;
}





/*.BH--------------------------------------------------------
**
** ������: CalAlpha_SingleFace
**
** ����: ����ÿ�����͸����ֵ
**
** �������:  �������������ĵ�����
**
** ���������͸����ֵ
**
** ����ֵ��TRUE:�ɹ�
**		   	      FALSE:ʧ��
**
**
** ���ע��:  ͸����ֵ���㺯��2
**
**.EH--------------------------------------------------------
*/
BOOL CalAlpha_SingleFace(LP_PT_3D NormalVector_SingleFace, LP_PT_3D MiddlePoint_SingleFace,double * Alpha_SingleFace)
{
    double  Angle_NormalNorth = 0.0;
    double   Alpha_North = Alpha_NorthBuiding;						//�ϱ����Alphaֵ
    double  Alpha_East = Alpha_WestBuilding;							//�������Alphaֵ
    LP_PT_3D NorthVector_SingleFace = NewAlterableMemory(sizeof(PT_3D));

    //����������������
    NorthVector_SingleFace->x = -(MiddlePoint_SingleFace->x) *(MiddlePoint_SingleFace->z);
    NorthVector_SingleFace->y = -(MiddlePoint_SingleFace->y) *(MiddlePoint_SingleFace->z);
    NorthVector_SingleFace->z =  (MiddlePoint_SingleFace->x) *(MiddlePoint_SingleFace->x)
                                +(MiddlePoint_SingleFace->y)*(MiddlePoint_SingleFace->y);

    //���㷨������������ļн�
    CalNormal_NorthAngle(NormalVector_SingleFace,NorthVector_SingleFace,&Angle_NormalNorth);

    //ȷ��Alphaֵ
    *Alpha_SingleFace = Alpha_North - (Alpha_North-Alpha_East)* sqrt(sin(Angle_NormalNorth)*sin(Angle_NormalNorth)) ;

    DeleteAlterableMemory(NorthVector_SingleFace);

    return TRUE;
}


/*.BH--------------------------------------------------------
**
** ������: CalNormal_NorthAngle
**
** ����: ���㷨������������ļн�
**
** �������:  ��������, ��������
**
** ����������н�
**
** ����ֵ��TRUE:�ɹ�
**		   	      FALSE:ʧ��
**
**
** ���ע��:  ͸����ֵ���㺯��3
**
**.EH--------------------------------------------------------
*/
BOOL CalNormal_NorthAngle(LP_PT_3D NormalVector_AnlgeCal, LP_PT_3D  NorthVector_AngleCal ,double * Normal_NorthAngle)
{
/*
    double k = 0.0f;

    //�ж�x�Ƿ����0
    if(NormalVector_AnlgeCal->x == 0)
    {
        if(NormalVector_AnlgeCal->y >0)
            *Normal_NorthAngle = 0;
        else
            *Normal_NorthAngle = PI;
    }
    else
    {

    //����б����н�
    *Normal_NorthAngle = atan2(NormalVector_AnlgeCal->y,NormalVector_AnlgeCal->x)+PI;

    }
*/

    f_float64_t dot_result = 0;
    f_float64_t cos_result = 0;
    f_float64_t module_vector_result1 = 0;
    f_float64_t module_vector_result2 = 0;


    //���
    dot_result = vector3DDotProduct(NormalVector_AnlgeCal,  NorthVector_AngleCal);

    //������������ģ
    module_vector_result1 = vector3DLength(NormalVector_AnlgeCal);
    module_vector_result2 = vector3DLength(NorthVector_AngleCal);

    //������ֵ
    cos_result = dot_result/(module_vector_result1*module_vector_result2);

    //��н�
    if (cos_result > 1)
    {
        *Normal_NorthAngle = 0;
    }
    else if (cos_result < -1)
    {
        *Normal_NorthAngle = PI;
    }
    else
        *Normal_NorthAngle = acos(cos_result);



    return TRUE;
}

void CalModelMatrix(sGLRENDERSCENE* pModelScene, double x, double y, double z, double size)
{
//	f_float64_t size = 0.00001;
    Matrix44 MxLoad;
//	double x = g_whitecity_position.x;
//	double y = g_whitecity_position.y;
//	double z = g_whitecity_position.z;
    f_float64_t temp[16] = {0};

    memcpy((&temp[0]) , Get_MODELVIEW(pModelScene), sizeof(double) * 16);

#if 0
{
    Matrix44 MxWorld = {0};
    Matrix44 MxModel, MxModel2;
    Matrix44 MxTranslate, MxScale;

    dMatrix44MemSet(&MxWorld, temp[0],  temp[1],  temp[2],  temp[3],
                              temp[4],  temp[5],  temp[6],  temp[7],
                            //  g_whitecity_position.x,  g_whitecity_position.y,  g_whitecity_position.z,  temp[11],
                              temp[8],  temp[9],  temp[10],  temp[11],
                              temp[12], temp[13],  temp[14],  temp[15]);

    dMatrix44MemSet(&MxTranslate, 1.0, 0.0, 0.0, 0.0,
                                  0.0, 1.0, 0.0, 0.0,
                                  0.0, 0.0, 1.0, 0.0,
                                  g_whitecity_position.x, g_whitecity_position.y, g_whitecity_position.z, 1.0);

    dMatrix44MemSet(&MxScale, size, 0.0, 0.0, 0.0,
                              0.0, size, 0.0, 0.0,
                              0.0, 0.0, size, 0.0,
                              0.0, 0.0, 0.0, 1.0);


    Maxtrix4x4_Mul(&MxModel, &MxTranslate, &MxWorld);

    Maxtrix4x4_Mul(&MxModel2, &MxScale, &MxModel);

    glLoadMatrixd(MxModel2.m);
}


#endif
    dMatrix44MemSet(&MxLoad, size * temp[0],  size * temp[1],  size * temp[2],  size * temp[3],
                             size * temp[4],  size * temp[5],  size * temp[6],  size * temp[7],
                             size * temp[8],  size * temp[9],  size * temp[10], size *  temp[11],
                            temp[0] * x + temp[4] * y + temp[8] * z + temp[12],
                            temp[1] * x + temp[5] * y + temp[9] * z + temp[13],
                            temp[2] * x + temp[6] * y + temp[10] * z + temp[14],
                            temp[3] * x + temp[7] * y + temp[11] * z + temp[15]);

    glLoadMatrixd(MxLoad.m);

}



/*.BH--------------------------------------------------------
**
** ������: WhiteCityCallList
**
** ����: ������ʾ�б�
**
** �������:  ��ʾ�б�ID
**
** �����������
**
** ����ֵ����
** ���ע��:  ���ƹ��ߺ���
**
**.EH--------------------------------------------------------
*/
void WhiteCityCallList(LPSingleCityCfg lpsinglecitycfg_LIST,int lpsinglecitycfg_LIST_ID,sGLRENDERSCENE* pModelScene)
{

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixd(Get_PROJECTION(pModelScene));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
//	glLoadMatrixd(Get_MODELVIEW(pModelScene));
//	glTranslated(g_whitecity_position.x, g_whitecity_position.y, g_whitecity_position.z);
//	glScaled(0.00001,0.00001,0.00001);
    CalModelMatrix(pModelScene, g_whitecity_position.x, g_whitecity_position.y, g_whitecity_position.z, 0.00001);

    //������ʾ�б�
    if(lpsinglecitycfg_LIST->m_bHasTexture != FALSE)
    {
        glEnableEx(GL_TEXTURE_2D);
        glEnableEx(GL_LINE_SMOOTH);
        //glEnable(GL_BLEND);
		//glEnableEx(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, lpsinglecitycfg_LIST->m_TextureCity->TextureID);
        glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnableEx(GL_DEPTH_TEST);



        //2016-1-5 15:11 lpf add �߶�Ԥ��
#if Building_Warning
        RenderTerrainWarningStripeColor();
#endif

        glCallList(lpsinglecitycfg_LIST_ID);

#if Building_Warning
        RenderTerrainWarningStripeColor3();
#endif


        glDisableEx(GL_CULL_FACE);
        glDisableEx(GL_BLEND);
        glDisableEx(GL_LINE_SMOOTH);
        glDisableEx(GL_TEXTURE_2D);
        glDisableEx(GL_DEPTH_TEST);
    }
    else
    {
        //���޸Ĵ˴�����״̬������
//		glFrontFace(GL_CCW);

        glEnableEx(GL_LINE_SMOOTH);
        glEnableEx(GL_BLEND);
//		glEnableEx(GL_CULL_FACE);
        glEnableEx(GL_DEPTH_TEST);
        //glDisableEx(GL_CULL_FACE);

//		glCullFace(GL_BACK);
        glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

#if !defined(OPENGL_ES_2) && !defined(OPENGL_ES_3)
		glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */
#endif

    //2016-1-5 15:12 lpf add �߶�Ԥ��
#if Building_Warning
        RenderTerrainWarningStripeColor();
#endif

        glCallList(lpsinglecitycfg_LIST_ID);

#if Building_Warning
        RenderTerrainWarningStripeColor3();
#endif

        glPopAttrib();							/**< �ָ�ǰһ���� */

//		glDisableEx(GL_CULL_FACE);
        glDisableEx(GL_BLEND);
        glDisableEx(GL_LINE_SMOOTH);
        glDisableEx(GL_DEPTH_TEST);



    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

/*.BH--------------------------------------------------------
**
** ������: WhiteCityCallList_Concave
**
** ����: ������ʾ�б���ư���
**
** �������:  ��ʾ�б�ID
**
** �����������
**
** ����ֵ����
** ���ע��:  ���ƹ��ߺ���2
**
**.EH--------------------------------------------------------
*/
void WhiteCityCallList_Concave(LPSingleCityCfg lpsinglecitycfg_LIST,int lpsinglecitycfg_LIST_ID, sGLRENDERSCENE* pModelScene)
{
//	double xSemiMask,ySemiMask;//x��y�����ϰ�͸���ɰ�İ�ߴ�

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfProjMatrix.m);
    glLoadMatrixd(Get_PROJECTION(pModelScene));


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
//	glLoadMatrixd(g_SphereRender.m_Render.m_lfModelMatrix.m);
//	glLoadMatrixd(Get_MODELVIEW(pModelScene));

//	glTranslated(g_whitecity_position.x, g_whitecity_position.y, g_whitecity_position.z);

    CalModelMatrix(pModelScene, g_whitecity_position.x, g_whitecity_position.y, g_whitecity_position.z, 0.00001);


    //������ʾ�б�
    if(lpsinglecitycfg_LIST->m_bHasTexture != FALSE)
    {
        glEnableEx(GL_TEXTURE_2D);
        glEnableEx(GL_LINE_SMOOTH);
        glEnableEx(GL_BLEND);
		//glEnableEx(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, lpsinglecitycfg_LIST->m_TextureCity->TextureID);
        glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnableEx(GL_DEPTH_TEST);


        //2016-1-5 15:11 lpf add �߶�Ԥ��
#if Building_Warning
        RenderTerrainWarningStripeColor();
#endif

        glCallList(lpsinglecitycfg_LIST_ID);

#if Building_Warning
        RenderTerrainWarningStripeColor3();
#endif


        glDisableEx(GL_CULL_FACE);
        glDisableEx(GL_BLEND);
        glDisableEx(GL_LINE_SMOOTH);
        glDisableEx(GL_TEXTURE_2D);
    }
    else
    {
    //���޸Ĵ˴�Ϊ���ư���
    #if 1
//		glEnableEx(GL_LINE_SMOOTH);
//		glEnableEx(GL_DEPTH_TEST);
        glEnableEx(GL_BLEND);
//		glEnableEx(GL_CULL_FACE);
        glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//		glEnableEx(GL_STENCIL_TEST);
        glEnableEx(GL_DEPTH_TEST);

    #else


		glFrontFace(GL_CCW);
		glEnableEx(GL_LINE_SMOOTH);
		glEnableEx(GL_CULL_FACE);
		glEnableEx(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnableEx(GL_BLEND);
		glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
//		glEnableEx(GL_STENCIL_TEST);
    #endif





//		glPushAttrib(GL_CURRENT_BIT);			/**< ����������ɫ��ʵ�� */

        glStencilFunc(GL_NEVER, 0x1, 0x1);
        //glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

    //2016-1-5 15:12 lpf add �߶�Ԥ��
#if Building_Warning
        RenderTerrainWarningStripeColor();
#endif

        // 3.�������������޳�����
//		glFrontFace(GL_CCW);
//		glCullFace(GL_BACK);


        glCallList(lpsinglecitycfg_LIST_ID);


        // �ػ���Ļ,ֻ����ģ�建��ֵ��0������
        glStencilFunc(GL_NOTEQUAL,0,0x1);
        glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);	//lpf add
        //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glCallList(lpsinglecitycfg_LIST_ID);

#if 0

        ySemiMask = (g_SphereRender.m_Render.m_lfNear + 5) * tan(DEGREE_TO_RADIAN(g_Fovy/2));
        xSemiMask = (g_SphereRender.m_Render.m_rcView.right - g_SphereRender.m_Render.m_rcView.left) * ySemiMask / (g_SphereRender.m_Render.m_rcView.top - g_SphereRender.m_Render.m_rcView.bottom);

        #ifdef HIGH_PRECISE_MATRIX
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadMatrixd(g_SphereRender.m_Render.m_lfProjMatrix.m);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadMatrixd(g_SphereRender.m_Render.m_lfModelMatrix.m);
        #else
            glPushMatrix();
            glLoadIdentity();
        #endif

        glDepthMask(GL_FALSE);
        glColor4f(ColorIndex*Alpha_TopBuilding,
                        ColorIndex*Alpha_TopBuilding,
                        ColorIndex*Alpha_TopBuilding,
                        AlphaIndex);
        glBegin(GL_POLYGON);
            glVertex3d(-xSemiMask, -ySemiMask, -(g_SphereRender.m_Render.m_lfNear+5));
            glVertex3d(xSemiMask, -ySemiMask, -(g_SphereRender.m_Render.m_lfNear+5));
            glVertex3d(xSemiMask, ySemiMask, -(g_SphereRender.m_Render.m_lfNear+5));
            glVertex3d(-xSemiMask, ySemiMask, -(g_SphereRender.m_Render.m_lfNear+5));
        glEnd();

        #ifdef HIGH_PRECISE_MATRIX
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
        #else
            glPopMatrix();
        #endif

#endif

#if Building_Warning
        RenderTerrainWarningStripeColor3();
#endif

//		glPopAttrib();   /**< �ָ�ǰһ���� */

//		glDepthMask(GL_TRUE);
//		glDisableEx(GL_STENCIL_TEST);
//		glDisableEx(GL_CULL_FACE);
		glDisableEx(GL_BLEND);	
		glStencilFunc(GL_ALWAYS,0,0x1);

		
//		glDisableEx(GL_LINE_SMOOTH);
		glDisableEx(GL_DEPTH_TEST);



    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}


