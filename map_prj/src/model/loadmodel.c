/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: loadmodel.c
**
** ����: ���ļ�������άģ��������Ϣ�Ķ��뺯������άģ�͵Ļ��ƺ�����
**
** ����ĺ���:
**       LoadAirFeatureFile();                      -----------����ģ�͵�������Ϣ�ļ�
**
** ���ע��:
**
** ���ߣ�
**		LPF��
**
**
** ������ʷ:
**		2015-4-16  15:22 LPF �����ļ���
**		2015-5-13  8:45  LPF ���ñ�����size
**    	2015-5-19  9:05  LPF �޸Ķ�ȡ�����ļ��ĳ���
**		2015-7-3   9:17  LPF ɾ���� �����ļ��еĹ�����Ϣ������id��ȡ�ӿڣ���Ӷ�̬��ʾģ�ͣ�
**		2015-9-10  15:13 LPF ���� ģ�����ܳ���
**		2015-10-13 10:32 LPF ���Ӷ�ȡ������������Ϣ�ļ��ĳ��������ɶ������ļ��ĳ���
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
#include "../engine/libList.h"
#include "../engine/memoryPool.h"
#include "../engine/osAdapter.h"
#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../mapApi/common.h"
#include "../geometry/convertEddian.h"
#include "../geometry/matrixMethods.h"
#include "../geometry/vectorMethods.h"

#include "loadmodel.h"
#include "Model.h"
#include "ModelApi.h"
#include "ModelreadTxtFile.h"

/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/


/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*
#ifdef WIN32
#define modelpath "../../../../DataFile/model/"
#else
#define modelpath "/ahci00/mapData3d/DataFile/model/"
#endif
*/
#define RenderModelRange 500000
#define FlagMakeBiConfig 0			//�Ƿ����ɶ����������ļ��ı�־
#define FlagReadConfig   1			// 0����ȡ�����������ļ���1����ȡtxt�����ļ�
#define ElementArg  2*PI/10			// ģ�����ܵ�ƽ���ǶȲ�
/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
static double CurModelPosLonlat[3][10]
        ={{120.4},
          {22.5},
        {ElementArg,ElementArg*2,ElementArg*3,ElementArg*4,
        ElementArg*5,ElementArg*6,ElementArg*7,ElementArg*8,ElementArg*9}};		//����ȫ�ֱ���
static double CurModelPosLonlat2[3][10]
        ={{120.4},
        {22.5},
        {ElementArg,ElementArg*2,ElementArg*3,ElementArg*4,
        ElementArg*5,ElementArg*6,ElementArg*7,ElementArg*8,ElementArg*9}};		//����ȫ�ֱ���

//static f_uint32_t s_nTwinkleCount = 0;			// ������˸����������֡Ϊ��λ��
//static f_uint32_t s_nTwinkleStep = 10;			// ��˸ʱ������Ĭ��ÿ��10֡��˸һ�Σ�
//static BOOL g_enableTwinkle	= FALSE;			//�������ƿ��أ�FALSE	��������TRUE, ����
static BOOL g_ModelInitFinish	= FALSE;				//�Ƿ��ʼ�����

/*for ACoreIDE ������ͷ�ļ��ж���������ƶ���Դ�ļ��У�ͷ�ļ�����extern��������*/
stList_Head	m_listModelData;			//ģ�ͳ�ʼ������

/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** �ڲ���������
**-----------------------------------------------------------
*/
void SetModelInitFlag(BOOL flag);
BOOL GetModelInitFlag();
//void drawCommonModel(LPModelRender pCommonModel) ;
void drawCommonModel(LPModelRender pCommonModel , sGLRENDERSCENE* pSceneModelDraw)  ;

BOOL LoadModelDisplayFile();
BOOL GenerateModelTree();
BOOL SearchModel3ds(int model_type, LPModel3DS* p_model_3ds);
BOOL ReadModelDisplayFile(const f_char_t* chfileName);



/*-----------------------------------------------------------
** ��������
**-----------------------------------------------------------
*/
/*.BH--------------------------------------------------------
**
** ������: BOOL InitModel()
**
** ����:   ��ȡģ���ļ��Ͷ�ȡ��̬��ʾ�ļ�
**
** �������:  ��
**
** ��������� ����
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ������ú���
**
**.EH--------------------------------------------------------
*/
BOOL InitModelData()
{
    // 0.��ʼ��ģ�ʹ洢����
    CreateList(&m_listModelData);

    // 1.��ȡģ���ļ�
    if(LoadModelConfigFile() == FALSE)
        return FALSE;

#if 0
    // 2.��ȡ��̬��ʾ�ļ�,ʵ��ʹ�ö�̬���ӣ��ù�����ʱע�ͣ�����Ҫ��ʱ���ٷſ�
    if(LoadModelDisplayFile() == FALSE)
        return FALSE;
#endif

    // 3.������ȷ
    SetModelInitFlag(TRUE);

    return TRUE;

}

/*.BH--------------------------------------------------------
**
** ������: LoadModelConfigFile
**
** ����:   ��ȡģ���ļ�
**
** �������:
**
** ��������� ����
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ��һ��ĺ���
**
**.EH--------------------------------------------------------
*/
BOOL LoadModelConfigFile()
{
    f_char_t file_path[1024] = {0};

    // 1.��ʼ��ģ���ļ�����������
    printf("��ʼ��ģ����Ϣ... \n");
    //���ɶ����������ļ�

    memset(file_path, 0, 1024);
    strncpy(file_path, modelpath, 512);

    //ѡ���ȡ�����ļ�: Ĭ���Ƕ�����
#if FlagReadConfig
    strncat(file_path, "modeldata.cfg", 13);
#else
    strncat(file_path,"storemodelData.dat",18);
#endif

    if(FALSE == ReadModelConfigFile(file_path))
    {
        printf("��ʼ��ģ��ʧ��\n");
        return FALSE;
    }

    //��ʼ����ɣ�����FlagֵΪTRUE������

    printf("��ʼ��ģ�ͳɹ�\n");

    // 2.�γ���״�ṹ,һ��ģ�Ϳ����ɶ��ģ�����
    if(FALSE ==  GenerateModelTree())
        return FALSE;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: ReadModelConfigFile
**
** ����:   ��ȡģ���ļ������ñ�
**
** �������:  ������Ϣ��·��
**
** ��������� ��������ֵ
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ�ڶ���ĺ���
**
**.EH--------------------------------------------------------
*/
BOOL ReadModelConfigFile(const f_char_t* chfileName)
{
    FILE *pFile = NULL;
    f_int32_t ret=0;
    f_char_t strBuf[256] = {0};
    f_char_t strTempName[64] = {0};
    f_int32_t iModelCount = 0;
//	LPFreatureLayer pLayer = NULL;
//	f_int32_t iLayer = AIR_FEATURE_LAYER_MODEL;
//	LPAirModel pModelData = NULL;
    f_char_t strTemp[256] = {0};
    f_int32_t i = 0;

//	LPNode pNode = NULL;
    LPModelAll pData = NULL;

    if(chfileName == NULL)
    {
        return FALSE;
    }
    pFile = fopen(chfileName, "rb");
    if(pFile == NULL)
    {
        printf("open %s failed.\n",chfileName);
        return FALSE;
    }

    // ��ȡģ��������Ϣ
    while(1)
    {
        ret = readtxtfile_getInt(pFile);
        if (ret == 0)
        {
            break;
        }
        pData = (LPModelAll)NewFixedMemory(sizeof(ModelAll));

        pData->id = ret;

        memset(strTemp, '0', 256);
        readtxtfile_getStr(pFile, strTemp, 256);
        memcpy(pData->name , strTemp, 256);

        memset(strTempName, '0', 64);
        readtxtfile_getStr(pFile, strTempName, 64);
        memcpy(pData->model3ds, strTempName, 64);

        pData->model_size = readtxtfile_getFloat(pFile);

        pData->child_num = readtxtfile_getInt(pFile);

        for(i = 0; i<pData->child_num; i++)
        {
            pData->child_id[i] = readtxtfile_getInt(pFile);
        }

        readtxtfile_nextline(pFile);
        //������άģ��
        LoadModelData(pData);

        {
            /**�ҵ�ͷ���֮��**/
            stList_Head *pstListHead = NULL;
            pstListHead = &(m_listModelData);
            LIST_ADD(&pData->stListHead, pstListHead);
        }

        printf("... ��ʼ��ģ��id = %d\n", pData->id);

    }

    fclose(pFile);
    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: LoadModelData
**
** ����: ����ģ�����ݣ�*.3ds�ļ���
**
** �������:  ģ�����ݽṹ��ָ��
**
** ��������� ģ�����ݽṹ��ָ��
**
** ����ֵ��TRUE--���سɹ�
**        FALSE--����ʧ��
**
** ���ע��:  �˺�����Ϊ������ĺ���
**
**.EH--------------------------------------------------------
*/
BOOL LoadModelData(LPModelAll pModelData)
{
    //����ѭ������ȫ��ģ�����������ָ��
    stList_Head *pstListHead = NULL;
    stList_Head *pstTmpList = NULL;

    //ĳ��ģ�����ݽڵ�ָ��
    LPModelAll pModelNode = NULL;
    //�Ƿ���سɹ�
    BOOL bLoad = FALSE;
    //ģ�������ļ�(*.3ds)·��
    f_char_t chFileModel[1024] = {0};

    //��ģ�����ݽṹ��ָ��Ϊ�գ�ֱ�ӷ��ؼ���ʧ��
    if(pModelData == NULL)
    {
        return FALSE;
    }

    //ָ��ȫ��ģ����������
    pstListHead = &(m_listModelData);

    //����ȫ��ģ����������
    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
        if( NULL != pstTmpList )
        {
            pModelNode = ASDE_LIST_ENTRY(pstTmpList, ModelAll, stListHead);
            //������ģ�͵�3ds�ļ��������������Ѽ���ģʽ��3ds�ļ�������ͬ������Ϊ��ģ���Ѽ���
            if (strncmp(pModelNode->model3ds, pModelData->model3ds, 64) == 0)
            {
                //ֱ�ӽ�ģ�;�����Ϣָ�븳ֵ����ǰ������ģ�ͣ�������ɣ�����ѭ��
                (pModelData->pModelMess) = (pModelNode->pModelMess);
                bLoad = TRUE;
                break;
            }
        }
    }

    //���������ģ����ȫ��ģ�����������в����ڣ��������
    if (bLoad == FALSE)
    {
        //��ȡ������ģ�͵�3ds�ļ�·��
        strcpy(chFileModel, modelpath);
        strncat(chFileModel, pModelData->model3ds, sizeof(pModelData->model3ds));
        strncat(chFileModel, "/", 1);
        strncat(chFileModel, pModelData->model3ds, sizeof(pModelData->model3ds));
        strncat(chFileModel, ".3ds", 4);
        //����3ds�ļ�����ȡģ�͵ľ�����Ϣ
        Init_3DS(&(pModelData->pModelMess), chFileModel);

    }
    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: GenerateModelTree
**
** ����:   ������״����������ṹ
**
** �������:  ��
**
** ��������� ��
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ�ڶ���ĺ���
**
**.EH--------------------------------------------------------
*/
BOOL GenerateModelTree()
{
    LPModelAll pData = NULL;
    LPModel3DS pChild3ds = NULL;
    int i = 0;

    stList_Head *pstListHead = NULL;
    stList_Head *pstTmpList = NULL;

    pstListHead = &(m_listModelData);

    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
        if( NULL != pstTmpList )
        {
            pData = ASDE_LIST_ENTRY(pstTmpList, ModelAll, stListHead);

            for(i = 0; i<pData->child_num; i++)
            {
                // ������ģ�͵�3ds����
                if(FALSE ==  SearchModel3ds(pData->child_id[i], &pChild3ds))
                    return FALSE;

                pData->child_3ds[i] = pChild3ds;
            }

        }
    }

    return TRUE;
}


/*.BH--------------------------------------------------------
**
** ������: SearchModel3ds
**
** ����:   ����3ds���ݵ�ָ��
**
** �������:  ģ�͵�type
**
** ��������� 3ds����ָ��
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ������ĺ���
**
**.EH--------------------------------------------------------
*/
BOOL SearchModel3ds(int model_type, LPModel3DS* p_model_3ds)
{
//	LPNode pNode = NULL;
    LPModelAll pModelNode = NULL;
    BOOL bLoad = FALSE;

    stList_Head *pstListHead = NULL;
    stList_Head *pstTmpList = NULL;


//	pNode = (LPNode)m_listModelData.m_pFirst;
    pstListHead = &(m_listModelData);
//	while(pNode != NULL)

    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
//		pModelNode = (LPModelAll)pNode->m_pCur;

        if( NULL != pstTmpList )
        {
            pModelNode = ASDE_LIST_ENTRY(pstTmpList, ModelAll, stListHead);

            if (pModelNode->id == model_type)
            {
                *p_model_3ds = &(pModelNode->pModelMess);
                bLoad = TRUE;
                break;
            }

        }



//		pNode = (LPNode)pNode->m_pNext;
    }

    return bLoad;
}


#if 0
/*.BH--------------------------------------------------------
**
** ������: RenderModelFeature
**
** ����:   ��Ⱦ��άģ��Ҫ��
**
** �������:  ��ʾЧ��0���ޣ�1��������2���߿�ģʽ��3���߿�+��
**							  4:	������+��, 5:�⻷Ч��
**
** ��������� ��
**
** ����ֵ��TRUE or FALSE
**
**
** ���ע��:  ��Ҫ������̬��ʾ
**
**.EH--------------------------------------------------------
*/
BOOL RenderModelFeature(int DisplayType)
{
    f_int32_t i = 0;
    f_int32_t j = 0;
    LPModelAll pData = NULL;
    LPNode pNode = NULL;
    void* pAirData = NULL;
    LPAirModel pAirModel = NULL;
    AirFeatureType aType = AIR_FEATURE_MODEL;
    f_float64_t * PlaneX = malloc(sizeof(f_float64_t));
    f_float64_t * PlaneY = malloc(sizeof(f_float64_t));
    f_float64_t * PlaneZ = malloc(sizeof(f_float64_t));
    f_float64_t * ModelX = malloc(sizeof(f_float64_t));
    f_float64_t * ModelY = malloc(sizeof(f_float64_t));
    f_float64_t * ModelZ = malloc(sizeof(f_float64_t));
    f_float64_t radius = 10;
    f_float64_t dis = 0;
    f_float64_t TempLeng = 0;
    f_float64_t z0 = 0;              //����߶�
    PT_4D color[4] = {0};

    //ģ�ͳ�ʼ����ɺ󣬷��ɼ���
    if(FALSE == GetModelInitFlag())
    {
        return FALSE;
    }

    // ���ƺ���Ҫ��
    pNode = (LPNode)m_listModelData.m_pFirst;
    while(pNode != NULL)
    {
        pData = (LPModelAll)pNode->m_pCur;

            //�ж��Ƿ����
            if(CheckModelRender() == TRUE)  //temp
            {
                LPModel3DS p3DModel = NULL;
//				pAirModel = (LPAirModel) pAirData;

                if(&(pData->pModelMess) !=NULL )
                {
                    //�жϾ����Ƿ�С��500km
                    //�������ظ߶�
                    if (FALSE == GetZ(pData->lon, pData->lat, &z0))
                    {
                        z0 = EARTH_RADIUS;
                    }
                        //return FALSE;
//					printf("%f\n",z0);
                    SphereToXYZ(
                                DEGREE_TO_RADIAN(GetPlaneLon()),
                                DEGREE_TO_RADIAN(GetPlaneLat()),
                                GetPlaneHei(),
                                PlaneX, PlaneY, PlaneZ, WGS84_R);
                    SphereToXYZ(
                                DEGREE_TO_RADIAN(pData->lon),
                                DEGREE_TO_RADIAN(pData->lat),
                                z0+pData->height,
                                ModelX, ModelY, ModelZ, WGS84_R);
                    TempLeng = (sqr(*PlaneX-*ModelX)+sqr(*PlaneY-*ModelY)+sqr(*PlaneZ-*ModelZ));
                    if(TempLeng < sqr(RenderModelRange))
                    {
                        //�ж��Ƿ����Ӿ�����
                        radius = (pData->scale) * (pData->size);
//					if(InFrustum(*ModelX, *ModelY, *ModelZ, radius, &g_SphereRender.m_Render.m_Frustum, &dis) == TRUE)
                        if(InFrustum(*ModelX, *ModelY, *ModelZ, radius, GetFrustumModel(), &dis) == TRUE)
                        {
                            //����ģ��
                    //		p3DModel = &((LPModelNode)((LPModel3DS)pAirModel->m_pModelData))->m_Model;
                            p3DModel = &(pData->pModelMess);
                            //�������ظ߶�
                            //GetZ((pData->lon),(pData->lat),&z0);

                            drawCommonModel(p3DModel, 0, TRUE, pData->lon, pData->lat, pData->height+z0,
                                            pData->pitch, pData->yaw, pData->roll, pData->scale, DisplayType,FALSE,color);

                        }
                    }

                }
            }
//		}
        pNode = (LPNode)pNode->m_pNext;
    }

    //�ͷ��ڴ�
    free(PlaneX);
    free(PlaneY);
    free(PlaneZ);
    free(ModelX);
    free(ModelY);
    free(ModelZ);

    return TRUE;
}

#endif

//�ж��Ƿ����ģ��
BOOL CheckModelRender()
{
//	if(g_bRenderAirModel == FALSE)
//		{return FALSE;}
//	else
        {return TRUE;}
}

//��ȡ�ɻ��ĵ�ǰ����
f_float64_t GetPlaneLat(sGLRENDERSCENE* pModelScene)
{
    //return g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLat;
//	f_float64_t ret = g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLat;

//	f_float64_t ret = ((sMAPHANDLE*)((sGLRENDERSCENE*)pScene[0])->map_handle)->cam_geo_pos.lat;

    f_float64_t ret = pModelScene->camParam.m_geoptEye.lat;

    return ret;
}
//��ȡ�ɻ��ĵ�ǰγ��
f_float64_t GetPlaneLon(sGLRENDERSCENE* pModelScene)
{
//	f_float64_t ret = g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLon;
    f_float64_t ret = pModelScene->camParam.m_geoptEye.lon;

    return ret;
}
//��ȡ�ɻ��ĵ�ǰ�߶�
f_float64_t GetPlaneHei(sGLRENDERSCENE* pModelScene)
{
//	f_float64_t ret = g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei;
    f_float64_t ret = pModelScene->camParam.m_geoptEye.height;

    return ret;
}

//��ȡ�Ӿ�����Ϣ
LPFRUSTUM GetFrustumModel(sGLRENDERSCENE* pModelScene)
{
//	return &g_SphereRender.m_Render.m_Frustum;

    return  &(pModelScene->frustum);
}

//��ȡ���ý������
f_float32_t GetView_near(sGLRENDERSCENE* pModelScene)
{
    return  pModelScene->camctrl_param.view_near;
}

//��ȡԶ�ý������
f_float32_t GetView_far(sGLRENDERSCENE* pModelScene)
{
    return  pModelScene->camctrl_param.view_far;
}


/*.BH--------------------------------------------------------
**
** ������: ModelConfigOut
**
** ����:   ��ʼ��ʱ����ͼ�����ȡģ����Ϣ
**
** �������:  ��
**
** ��������� ģ����Ϣid
**
** ����ֵ��TRUE or FALSE
**
**
** ���ע��:  ����ӿڳ���
**
**.EH--------------------------------------------------------
*/
BOOL ModelConfigOut(int* id)
{
//	LPNode pNode = NULL;
    stList_Head *pstListHead = NULL;
    stList_Head *pstTmpList = NULL;
    LPModelAll pData = NULL;

//	pNode = (LPNode)m_listModelData.m_pFirst;
    pstListHead = &(m_listModelData);
    //�ж��Ƿ����ģ������
//	if(pNode == NULL)
//		{return FALSE;}
    //���ģ�͵�id, name
//	while(pNode != NULL)

    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
//		pData = (LPModelAll)pNode->m_pCur;

        if( NULL != pstTmpList )
        {
            pData = ASDE_LIST_ENTRY(pstTmpList, ModelAll, stListHead);

            (*id) = pData->id;
            printf("%d\n",*id);
            (*id)++;

        }

//		pNode = (LPNode)pNode->m_pNext;
    }

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: RenderModelFeatureDynamic
**
** ����:   ��̬��Ⱦ��άģ��
**
** �������:  ģ��id, lat , lon, hei, yaw, pitch, roll, scale
**				��ʾЧ��:
**							0���ޣ�1��������2���߿�ģʽ��
**							3���߿�+��,  4:	������+��, 5:�⻷Ч��
**				��ɫ����: �߿�,��Ļ��,�⻷,����ɫ
**
** ��������� ��
**
** ����ֵ��TRUE or FALSE
**
**
** ���ע��:  ����ӿں�����̬��ʾ
**
**.EH--------------------------------------------------------
*/
BOOL RenderModelFeatureDynamic(LPModelRender pModelDynamic , sGLRENDERSCENE* pModelSceneDynamic)
{
    // ����ģ��
    drawCommonModel(pModelDynamic, pModelSceneDynamic);

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: ModelFeatureAutoRunSend
**
** ����:   ��άģ�����ܳ���--���ò���
**
** �������:  ��
**
** ��������� ��
**
** ����ֵ��TRUE or FALSE
**
**
** ���ע��:  ����ӿں���--ģ�����ܳ���--���ò���--Win32--����
**
**.EH--------------------------------------------------------
*/
#if 0
BOOL ModelFeatureAutoRunSend()
{
    double BeginLonLat[2] = {120.4,22.5};
    double EndLonLat[2]   = {121,23};
    double StepLonLat[2]  = {0.0001,0.0001};
    int LonI = 0;
    int LatJ = 0;
    int LonINum = (EndLonLat[0] - BeginLonLat[0])/StepLonLat[0];
    int LatJNum = (EndLonLat[1] - BeginLonLat[1])/StepLonLat[1];
    double CurModelLonLat[2] = {0};
    double R_Model = 0.01;
    int CountI=0;

//	for (LonI = 0; LonI < LonINum; LonI++)
    {
    for(CountI=0;CountI<10;CountI++)
        {
        //��һȦ��ֵ
        CurModelPosLonlat[2][CountI] =CurModelPosLonlat[2][CountI] + 0.02;
        CurModelPosLonlat[0][CountI] = BeginLonLat[0]+R_Model*cos(CurModelPosLonlat[2][CountI]);
        CurModelPosLonlat[1][CountI] = BeginLonLat[1]+R_Model*sin(-CurModelPosLonlat[2][CountI]);
        //�ڶ�Ȧ��ֵ
        CurModelPosLonlat2[2][CountI] =CurModelPosLonlat2[2][CountI] + 0.02;
        CurModelPosLonlat2[0][CountI] = BeginLonLat[0]+R_Model*0.5*cos(CurModelPosLonlat2[2][CountI]);
        CurModelPosLonlat2[1][CountI] = BeginLonLat[1]+R_Model*0.5*sin(-CurModelPosLonlat2[2][CountI]);
        }
    }

}
#endif
/*.BH--------------------------------------------------------
**
** ������: ModelFeatureAutoRunRecv
**
** ����:   ��άģ�����ܳ���--���ղ���
**
** �������:  ��
**
** ��������� ��
**
** ����ֵ��TRUE or FALSE
**
**
** ���ע��:  ����ӿں���--ģ�����ܳ���--���ղ���
**
**.EH--------------------------------------------------------
*/
#if 0
#if WIN32
BOOL ModelFeatureAutoRunRecv()
{
    PT_4D color[4] = {0};
    int i=0;
    for (i=0;i<10;i++)
    {
        RenderModelFeatureDynamic(20203,CurModelPosLonlat[1][i],CurModelPosLonlat[0][i],0,CurModelPosLonlat[2][i]/PI*180,0,0,50,0,FALSE,0,color);
        RenderModelFeatureDynamic(20202,CurModelPosLonlat2[1][i],CurModelPosLonlat2[0][i],0,CurModelPosLonlat[2][i]/PI*180-180,0,0,50,0,FALSE,0,color);
    }
//	RenderModelFeatureDynamic(10109,CurModelPosLonlat[1],CurModelPosLonlat[0],0,0,0,0,50,0);
//	printf("%f,,,%f\n",CurModelPosLonlat[0],CurModelPosLonlat[1]);

}
#else

//1.�������ܲ���
void ModelDynamicParamSet()
{
    static double BeginLonLat[2] = {120.4,22.5};
    static double R_Model = 0.01;
    int i = 0;
    for(;;)
    {
        for(i=0;i<10;i++)
        {
    CurModelPosLonlat[2][i] =CurModelPosLonlat[2][i] + 0.03; //���Ŵ�����ʱ��
    CurModelPosLonlat[0][i] = BeginLonLat[0]+R_Model*cos(CurModelPosLonlat[2][i]);
    CurModelPosLonlat[1][i] = BeginLonLat[1]+R_Model*sin(-CurModelPosLonlat[2][i]);

    CurModelPosLonlat2[2][i] =CurModelPosLonlat2[2][i] + 0.03; //���Ŵ�����ʱ��
    CurModelPosLonlat2[0][i] = BeginLonLat[0]+R_Model*0.5*cos(CurModelPosLonlat2[2][i]);
    CurModelPosLonlat2[1][i] = BeginLonLat[1]+R_Model*0.5*sin(-CurModelPosLonlat2[2][i]);
        }
//	printf("%f\n",CurModelPosLonlat[0][i]);
    taskDelay(1);
//	if(CurModelPosLonlat[2] < -1000)
//	{
//		for(i=0;i<10;i++)
//			CurModelPosLonlat[2][i] =0;

//	}
    }
}
//2.����ģ��
BOOL ModelFeatureAutoRunRecv()
{
    //��ȡģ�����ܲ���
//	RenderModelFeatureDynamic(10109,CurModelPosLonlat[1],CurModelPosLonlat[0],0,CurModelPosLonlat[2]/3.14*180,0,0,50,0);
//	printf("%f,,,%f\n",CurModelPosLonlat[0],CurModelPosLonlat[1]);
    int i=0;
    for (i=0;i<10;i++)
    {
        RenderModelFeatureDynamic(20203,CurModelPosLonlat[1][i],CurModelPosLonlat[0][i],0,CurModelPosLonlat[2][i]/PI*180,0,0,50,0,0);
        RenderModelFeatureDynamic(20202,CurModelPosLonlat2[1][i],CurModelPosLonlat2[0][i],0,CurModelPosLonlat2[2][i]/PI*180-180,0,0,50,0,0);
    }
}

#endif

#endif
/*.BH--------------------------------------------------------
**
** ������: SetModelInitFlag
**
** ����:   ����ģ�ͳ�ʼ����ɵ�״̬
**
** �������:  TURE OR FALSE
**
** ��������� ��
**
** ����ֵ:		��
**
**
** ���ע��:  ����ӿں���--ģ�ͳ�ʼ����ɺ�����״̬Ϊ�����
**
**.EH--------------------------------------------------------
*/
void SetModelInitFlag(BOOL flag)
{
    g_ModelInitFinish = flag;
}

/*.BH--------------------------------------------------------
**
** ������: GetModelInitFlag
**
** ����:   ��ȡ ģ�ͳ�ʼ����ɵ�״̬
**
** �������:  ��
**
** ��������� ��
**
** ����ֵ:	TRUE OR FALSE
**
**
** ���ע��:  ����ӿں���--��ȡģ�ͳ�ʼ���Ƿ����
**
**.EH--------------------------------------------------------
*/
BOOL GetModelInitFlag()
{
    return g_ModelInitFinish;
}

/*.BH--------------------------------------------------------
**
** ������: drawCommonModel
**
** ����:   �ڲ�����ģ�͵ĺ������ھ�̬�Ͷ�̬����ʱ����
**
** �������:  ģ����Ϣ����γ�ߵȣ�ģʽѡ��
**
** ��������� ��
**
** ����ֵ:	��
**
**
** ���ע��:  �ڲ�������,�߶��ǵ���߶�+��Ը߶�
**
**.EH--------------------------------------------------------
*/
static int s_nTwinkleTickPre = 0;		//��һ�ε�tick��
static int s_nTwinkleTickStep = 300;	//����ģ�͵�tick��
void drawCommonModel(LPModelRender pCommonModel , sGLRENDERSCENE* pSceneModelDraw)
{
    int s_nTwinkleTickNow = 0;		//��ǰ��tick ��
    switch(pCommonModel->m_modelDisplaytype)
    {

        //ģ��������ʾ��ÿ��10֡��һ��,�޸�Ϊ10��ticks ����һ��
        case 1:
#ifdef	WIN32
                s_nTwinkleTickNow = GetTickCount();
#else
                s_nTwinkleTickNow = tickCountGet();
#endif
                if(( s_nTwinkleTickNow - s_nTwinkleTickPre) < s_nTwinkleTickStep)
                {
                    // �ڵ�һ�������ڻ���ģ��
                    RenderModelLLH(pCommonModel, pSceneModelDraw);
                }
                else if(( s_nTwinkleTickNow - s_nTwinkleTickPre) < (2* s_nTwinkleTickStep))
                {
                    // �ڵڶ��������ڲ�����ģ��
                }
                else
                {
                    // ����2�����ڣ�ѭ����һ������
                    s_nTwinkleTickPre = s_nTwinkleTickNow;
                }
                break;
#if 0   //�ڱ�ĺ������Ѿ������������
        //ģ����ʾ���Ч��
        case 3:
//				pCommonModel->m_colorNum_use = 0;
//				pCommonModel->m_modelDisplaytype = 0;
                set_model_colorNumUse(pCommonModel, 0);
                set_model_displaytype(pCommonModel, 0);
                RenderModelLLH(pCommonModel, pSceneModelDraw);

//				pCommonModel->m_modelDisplaytype = 2;
                set_model_displaytype(pCommonModel, 2);
                pCommonModel->m_modelScale *= 1.02;
                pCommonModel->m_MatrixChange = TRUE;
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                //ģ��Ч����ԭΪ3, ��С��ԭ
                set_model_displaytype(pCommonModel, 3);
                pCommonModel->m_modelScale *= (1/1.02);
                pCommonModel->m_MatrixChange = TRUE;

                break;

        //��������ʾЧ��
        case 4:
                glColor4f(pCommonModel->m_color[1].x,
                        pCommonModel->m_color[1].y,
                        pCommonModel->m_color[1].z,
                        pCommonModel->m_color[1].w);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(5.0f);
                glDepthMask(GL_FALSE);
//				pCommonModel->m_colorNum_use = 1;
//				pCommonModel->m_modelDisplaytype = 4;
                set_model_colorNumUse(pCommonModel, 1);
                set_model_displaytype(pCommonModel, 4);
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                glColor3f(1.0f, 1.0f, 1.0f);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDepthMask(GL_TRUE);
//				pCommonModel->m_colorNum_use = 0;
//				pCommonModel->m_modelDisplaytype = 0;
                set_model_colorNumUse(pCommonModel, 0);
                set_model_displaytype(pCommonModel, 0);
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                //ģ��Ч����ԭΪ4
                set_model_displaytype(pCommonModel, 4);

                break;

        //�⻷Ч��
        case 5:
                glColor4f(pCommonModel->m_color[2].x,
                        pCommonModel->m_color[2].y,
                        pCommonModel->m_color[2].z,
                        pCommonModel->m_color[2].w);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnableEx(GL_BLEND);
                glDepthMask(GL_FALSE);
//				pCommonModel->m_colorNum_use = 2;
//				pCommonModel->m_modelDisplaytype = 5;
                set_model_colorNumUse(pCommonModel, 2);
                set_model_displaytype(pCommonModel, 5);
                pCommonModel->m_modelScale *= 1.05;
                pCommonModel->m_MatrixChange = TRUE;
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                glDisableEx(GL_BLEND);
                glDepthMask(GL_TRUE);
                glColor3f(1.0f, 1.0f, 1.0f);
//				pCommonModel->m_colorNum_use = 0;
//				pCommonModel->m_modelDisplaytype = 0;
                set_model_colorNumUse(pCommonModel, 0);
                set_model_displaytype(pCommonModel, 0);
                pCommonModel->m_modelScale *= (1/1.05);
                pCommonModel->m_MatrixChange = TRUE;
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                //ģ��Ч����ԭΪ5
                set_model_displaytype(pCommonModel, 5);

                break;
#endif
        //��ɫЧ��
        case 6:
//				pCommonModel->m_colorNum_use = 3;
                //set_model_colorNumUse(pCommonModel, 3);
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                break;

        //������ʾЧ��0, �߿���ʾЧ��2
        case 0:
        case 2:
        default:
                RenderModelLLH(pCommonModel, pSceneModelDraw);
                break;
        }

//	//��������������
//	s_nTwinkleCount++;
//	if(s_nTwinkleCount > 20)
//	{
//		s_nTwinkleCount = 0;
//	}

}

/*.BH--------------------------------------------------------
**
** ������: LoadModelDisplayFile
**
** ����:   ��ȡ��̬��ʾ�����ļ�
**
** �������:
**
** ��������� ����
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ��һ��ĺ���
**
**.EH--------------------------------------------------------
*/
BOOL LoadModelDisplayFile()
{
    f_char_t file_path[1024] = {0};
    int i = 0;

    printf("��ȡ��̬��ʾ�����ļ�... \n");

    memset(file_path, 0, 1024);
    strncpy(file_path, modelpath, 512);

    // 1.ѡ���ȡ�����ļ�
    strncat(file_path, "modelDisplay.cfg", 16);

    // 2.���Ӷ����ͼ�����ľ�̬ģ����ʾ��ʼ��
    for(i=0;i<SCENE_NUM;i++)
    {
        AttachpScene2Model(pScene[i]);

        if(FALSE == ReadModelDisplayFile(file_path))
        {
            printf("��ȡ��̬��ʾ�����ļ�ʧ��\n");
            return FALSE;
        }

    }

    printf("��ȡ��̬��ʾ�����ļ��ɹ�\n");

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: ReadModelDisplayFile
**
** ����:   ��ȡ��̬��ʾ�ļ���Ϣ��
**
** �������:  ������Ϣ��·��
**
** ��������� ��
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �˺�����Ϊ�ڶ���ĺ��������뵽����������
**
**.EH--------------------------------------------------------
*/
BOOL ReadModelDisplayFile(const f_char_t* chfileName)
{
    FILE *pFile = NULL;
    f_int32_t ret=0;

    ModelRender ModelStaticRender = {0};

    if(chfileName == NULL)
    {
        return FALSE;
    }
    pFile = fopen(chfileName, "rb");
    if(pFile == NULL)
    {
        printf("open %s failed.\n",chfileName);
        return FALSE;
    }

    // ��ȡģ��������Ϣ
    while(1)
    {
        ret = readtxtfile_getInt(pFile);
        if (ret == 0)
        {
            break;
        }

        //model type
        ModelStaticRender.m_modeltype = ret;

        //model lon, lat, hei
        ModelStaticRender.m_modelPos.y = readtxtfile_getFloat(pFile);
        ModelStaticRender.m_modelPos.x = readtxtfile_getFloat(pFile);
        ModelStaticRender.m_modelPos.z = readtxtfile_getFloat(pFile);

        ModelStaticRender.m_heimode = 0;	//��Ը߶�

        //model pitch, roll, yaw
        ModelStaticRender.m_modelAngle.y = readtxtfile_getFloat(pFile);
        ModelStaticRender.m_modelAngle.z = readtxtfile_getFloat(pFile);
        ModelStaticRender.m_modelAngle.x = readtxtfile_getFloat(pFile);

        //model scale
        ModelStaticRender.m_modelScale =  readtxtfile_getFloat(pFile);

        //����sizeģ�ͳߴ�
        //pData->size = readtxtfile_getFloat(pFile);

        //���뵽����������
        ModelStaticRender.m_modelID =
            NewModel(ModelStaticRender.m_modeltype);

        if(ModelStaticRender.m_modelID == FALSE)
        {
            return FALSE;
        }

        if(FALSE == SetModelPos(ModelStaticRender.m_modelID, ModelStaticRender.m_modelPos))
        {
            return FALSE;
        }

        if(FALSE == SetModelAngle(ModelStaticRender.m_modelID, ModelStaticRender.m_modelAngle))
        {
            return FALSE;
        }

        if(FALSE == SetModelScale(ModelStaticRender.m_modelID, ModelStaticRender.m_modelScale))
        {
            return FALSE;
        }

        //��ȡ��һ��
        readtxtfile_nextline(pFile);

    }

    fclose(pFile);
    return TRUE;
}




