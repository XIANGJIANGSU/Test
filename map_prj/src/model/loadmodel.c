/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: loadmodel.c
**
** 描述: 本文件包含三维模型配置信息的读入函数和三维模型的绘制函数。
**
** 定义的函数:
**       LoadAirFeatureFile();                      -----------读入模型的配置信息文件
**
** 设计注记:
**
** 作者：
**		LPF。
**
**
** 更改历史:
**		2015-4-16  15:22 LPF 创建文件；
**		2015-5-13  8:45  LPF 配置表新增size
**    	2015-5-19  9:05  LPF 修改读取配置文件的程序
**		2015-7-3   9:17  LPF 删除与 其他文件中的关联信息，增加id读取接口，添加动态显示模型；
**		2015-9-10  15:13 LPF 增加 模型自跑程序
**		2015-10-13 10:32 LPF 增加读取二进制配置信息文件的程序与生成二进制文件的程序
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 头文件引用
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
** 类型声明
**-----------------------------------------------------------
*/


/*-----------------------------------------------------------
** 文字量和宏声明
**-----------------------------------------------------------
*
#ifdef WIN32
#define modelpath "../../../../DataFile/model/"
#else
#define modelpath "/ahci00/mapData3d/DataFile/model/"
#endif
*/
#define RenderModelRange 500000
#define FlagMakeBiConfig 0			//是否生成二进制配置文件的标志
#define FlagReadConfig   1			// 0：读取二进制配置文件；1：读取txt配置文件
#define ElementArg  2*PI/10			// 模型自跑的平均角度差
/*-----------------------------------------------------------
** 全局变量定义
**-----------------------------------------------------------
*/
static double CurModelPosLonlat[3][10]
        ={{120.4},
          {22.5},
        {ElementArg,ElementArg*2,ElementArg*3,ElementArg*4,
        ElementArg*5,ElementArg*6,ElementArg*7,ElementArg*8,ElementArg*9}};		//自跑全局变量
static double CurModelPosLonlat2[3][10]
        ={{120.4},
        {22.5},
        {ElementArg,ElementArg*2,ElementArg*3,ElementArg*4,
        ElementArg*5,ElementArg*6,ElementArg*7,ElementArg*8,ElementArg*9}};		//自跑全局变量

//static f_uint32_t s_nTwinkleCount = 0;			// 报警闪烁计数器（以帧为单位）
//static f_uint32_t s_nTwinkleStep = 10;			// 闪烁时间间隔（默认每隔10帧闪烁一次）
//static BOOL g_enableTwinkle	= FALSE;			//闪动控制开关，FALSE	不闪动，TRUE, 闪动
static BOOL g_ModelInitFinish	= FALSE;				//是否初始化完成

/*for ACoreIDE 不能在头文件中定义变量，移动至源文件中，头文件中用extern进行声明*/
stList_Head	m_listModelData;			//模型初始化链表

/*-----------------------------------------------------------
** 外部变量声明
**-----------------------------------------------------------
*/

/*-----------------------------------------------------------
** 内部函数声明
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
** 函数定义
**-----------------------------------------------------------
*/
/*.BH--------------------------------------------------------
**
** 函数名: BOOL InitModel()
**
** 描述:   读取模型文件和读取静态显示文件
**
** 输入参数:  无
**
** 输出参数： 链表
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为外面调用函数
**
**.EH--------------------------------------------------------
*/
BOOL InitModelData()
{
    // 0.初始化模型存储链表
    CreateList(&m_listModelData);

    // 1.读取模型文件
    if(LoadModelConfigFile() == FALSE)
        return FALSE;

#if 0
    // 2.读取静态显示文件,实际使用动态增加，该功能暂时注释，有需要的时候再放开
    if(LoadModelDisplayFile() == FALSE)
        return FALSE;
#endif

    // 3.返回正确
    SetModelInitFlag(TRUE);

    return TRUE;

}

/*.BH--------------------------------------------------------
**
** 函数名: LoadModelConfigFile
**
** 描述:   读取模型文件
**
** 输入参数:
**
** 输出参数： 链表
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为第一层的函数
**
**.EH--------------------------------------------------------
*/
BOOL LoadModelConfigFile()
{
    f_char_t file_path[1024] = {0};

    // 1.初始化模型文件，生成链表
    printf("初始化模型信息... \n");
    //生成二进制配置文件

    memset(file_path, 0, 1024);
    strncpy(file_path, modelpath, 512);

    //选择读取配置文件: 默认是二进制
#if FlagReadConfig
    strncat(file_path, "modeldata.cfg", 13);
#else
    strncat(file_path,"storemodelData.dat",18);
#endif

    if(FALSE == ReadModelConfigFile(file_path))
    {
        printf("初始化模型失败\n");
        return FALSE;
    }

    //初始化完成，设置Flag值为TRUE并返回

    printf("初始化模型成功\n");

    // 2.形成树状结构,一个模型可以由多个模型组成
    if(FALSE ==  GenerateModelTree())
        return FALSE;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: ReadModelConfigFile
**
** 描述:   读取模型文件的配置表
**
** 输入参数:  配置信息表路径
**
** 输出参数： 链表、索引值
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为第二层的函数
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

    // 读取模型配置信息
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
        //加载三维模型
        LoadModelData(pData);

        {
            /**挂到头结点之后**/
            stList_Head *pstListHead = NULL;
            pstListHead = &(m_listModelData);
            LIST_ADD(&pData->stListHead, pstListHead);
        }

        printf("... 初始化模型id = %d\n", pData->id);

    }

    fclose(pFile);
    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: LoadModelData
**
** 描述: 加载模型数据（*.3ds文件）
**
** 输入参数:  模型数据结构体指针
**
** 输出参数： 模型数据结构体指针
**
** 返回值：TRUE--加载成功
**        FALSE--加载失败
**
** 设计注记:  此函数作为第三层的函数
**
**.EH--------------------------------------------------------
*/
BOOL LoadModelData(LPModelAll pModelData)
{
    //用于循环遍历全局模型数据链表的指针
    stList_Head *pstListHead = NULL;
    stList_Head *pstTmpList = NULL;

    //某个模型数据节点指针
    LPModelAll pModelNode = NULL;
    //是否加载成功
    BOOL bLoad = FALSE;
    //模型数据文件(*.3ds)路径
    f_char_t chFileModel[1024] = {0};

    //若模型数据结构体指针为空，直接返回加载失败
    if(pModelData == NULL)
    {
        return FALSE;
    }

    //指向全局模型数据链表
    pstListHead = &(m_listModelData);

    //遍历全局模型数据链表
    LIST_FOR_EACH_ITEM(pstTmpList, pstListHead)
    {
        if( NULL != pstTmpList )
        {
            pModelNode = ASDE_LIST_ENTRY(pstTmpList, ModelAll, stListHead);
            //待加载模型的3ds文件名称与链表中已加载模式的3ds文件名称相同，则认为该模型已加载
            if (strncmp(pModelNode->model3ds, pModelData->model3ds, 64) == 0)
            {
                //直接将模型具体信息指针赋值给当前待加载模型，加载完成，跳出循环
                (pModelData->pModelMess) = (pModelNode->pModelMess);
                bLoad = TRUE;
                break;
            }
        }
    }

    //如果待加载模型在全局模型数据链表中不存在，则需加载
    if (bLoad == FALSE)
    {
        //获取待加载模型的3ds文件路径
        strcpy(chFileModel, modelpath);
        strncat(chFileModel, pModelData->model3ds, sizeof(pModelData->model3ds));
        strncat(chFileModel, "/", 1);
        strncat(chFileModel, pModelData->model3ds, sizeof(pModelData->model3ds));
        strncat(chFileModel, ".3ds", 4);
        //加载3ds文件，获取模型的具体信息
        Init_3DS(&(pModelData->pModelMess), chFileModel);

    }
    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: GenerateModelTree
**
** 描述:   生成树状链表的完整结构
**
** 输入参数:  无
**
** 输出参数： 无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为第二层的函数
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
                // 查找子模型的3ds数据
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
** 函数名: SearchModel3ds
**
** 描述:   查找3ds数据的指针
**
** 输入参数:  模型的type
**
** 输出参数： 3ds数据指针
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为第三层的函数
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
** 函数名: RenderModelFeature
**
** 描述:   渲染三维模型要素
**
** 输入参数:  显示效果0：无，1：闪动，2：线框模式，3：线框+面
**							  4:	包罗线+面, 5:光环效果
**
** 输出参数： 无
**
** 返回值：TRUE or FALSE
**
**
** 设计注记:  主要函数静态显示
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
    f_float64_t z0 = 0;              //地面高度
    PT_4D color[4] = {0};

    //模型初始化完成后，方可继续
    if(FALSE == GetModelInitFlag())
    {
        return FALSE;
    }

    // 绘制航空要素
    pNode = (LPNode)m_listModelData.m_pFirst;
    while(pNode != NULL)
    {
        pData = (LPModelAll)pNode->m_pCur;

            //判断是否绘制
            if(CheckModelRender() == TRUE)  //temp
            {
                LPModel3DS p3DModel = NULL;
//				pAirModel = (LPAirModel) pAirData;

                if(&(pData->pModelMess) !=NULL )
                {
                    //判断距离是否小于500km
                    //测量当地高度
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
                        //判断是否在视景体内
                        radius = (pData->scale) * (pData->size);
//					if(InFrustum(*ModelX, *ModelY, *ModelZ, radius, &g_SphereRender.m_Render.m_Frustum, &dis) == TRUE)
                        if(InFrustum(*ModelX, *ModelY, *ModelZ, radius, GetFrustumModel(), &dis) == TRUE)
                        {
                            //绘制模型
                    //		p3DModel = &((LPModelNode)((LPModel3DS)pAirModel->m_pModelData))->m_Model;
                            p3DModel = &(pData->pModelMess);
                            //测量当地高度
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

    //释放内存
    free(PlaneX);
    free(PlaneY);
    free(PlaneZ);
    free(ModelX);
    free(ModelY);
    free(ModelZ);

    return TRUE;
}

#endif

//判断是否绘制模型
BOOL CheckModelRender()
{
//	if(g_bRenderAirModel == FALSE)
//		{return FALSE;}
//	else
        {return TRUE;}
}

//读取飞机的当前经度
f_float64_t GetPlaneLat(sGLRENDERSCENE* pModelScene)
{
    //return g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLat;
//	f_float64_t ret = g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLat;

//	f_float64_t ret = ((sMAPHANDLE*)((sGLRENDERSCENE*)pScene[0])->map_handle)->cam_geo_pos.lat;

    f_float64_t ret = pModelScene->camParam.m_geoptEye.lat;

    return ret;
}
//读取飞机的当前纬度
f_float64_t GetPlaneLon(sGLRENDERSCENE* pModelScene)
{
//	f_float64_t ret = g_pAirPlanes[g_nSelectPlane].m_Entity.m_nLon;
    f_float64_t ret = pModelScene->camParam.m_geoptEye.lon;

    return ret;
}
//读取飞机的当前高度
f_float64_t GetPlaneHei(sGLRENDERSCENE* pModelScene)
{
//	f_float64_t ret = g_pAirPlanes[g_nSelectPlane].m_Entity.m_nHei;
    f_float64_t ret = pModelScene->camParam.m_geoptEye.height;

    return ret;
}

//读取视景体信息
LPFRUSTUM GetFrustumModel(sGLRENDERSCENE* pModelScene)
{
//	return &g_SphereRender.m_Render.m_Frustum;

    return  &(pModelScene->frustum);
}

//获取近裁截面距离
f_float32_t GetView_near(sGLRENDERSCENE* pModelScene)
{
    return  pModelScene->camctrl_param.view_near;
}

//获取远裁截面距离
f_float32_t GetView_far(sGLRENDERSCENE* pModelScene)
{
    return  pModelScene->camctrl_param.view_far;
}


/*.BH--------------------------------------------------------
**
** 函数名: ModelConfigOut
**
** 描述:   初始化时，绘图程序读取模型信息
**
** 输入参数:  无
**
** 输出参数： 模型信息id
**
** 返回值：TRUE or FALSE
**
**
** 设计注记:  对外接口程序
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
    //判断是否存在模型数据
//	if(pNode == NULL)
//		{return FALSE;}
    //输出模型的id, name
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
** 函数名: RenderModelFeatureDynamic
**
** 描述:   动态渲染三维模型
**
** 输入参数:  模型id, lat , lon, hei, yaw, pitch, roll, scale
**				显示效果:
**							0：无，1：闪动，2：线框模式，
**							3：线框+面,  4:	包罗线+面, 5:光环效果
**				颜色数组: 线框,屏幕框,光环,纯颜色
**
** 输出参数： 无
**
** 返回值：TRUE or FALSE
**
**
** 设计注记:  对外接口函数动态显示
**
**.EH--------------------------------------------------------
*/
BOOL RenderModelFeatureDynamic(LPModelRender pModelDynamic , sGLRENDERSCENE* pModelSceneDynamic)
{
    // 绘制模型
    drawCommonModel(pModelDynamic, pModelSceneDynamic);

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: ModelFeatureAutoRunSend
**
** 描述:   三维模型自跑程序--设置参数
**
** 输入参数:  无
**
** 输出参数： 无
**
** 返回值：TRUE or FALSE
**
**
** 设计注记:  对外接口函数--模型自跑程序--设置参数--Win32--废弃
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
        //第一圈赋值
        CurModelPosLonlat[2][CountI] =CurModelPosLonlat[2][CountI] + 0.02;
        CurModelPosLonlat[0][CountI] = BeginLonLat[0]+R_Model*cos(CurModelPosLonlat[2][CountI]);
        CurModelPosLonlat[1][CountI] = BeginLonLat[1]+R_Model*sin(-CurModelPosLonlat[2][CountI]);
        //第二圈赋值
        CurModelPosLonlat2[2][CountI] =CurModelPosLonlat2[2][CountI] + 0.02;
        CurModelPosLonlat2[0][CountI] = BeginLonLat[0]+R_Model*0.5*cos(CurModelPosLonlat2[2][CountI]);
        CurModelPosLonlat2[1][CountI] = BeginLonLat[1]+R_Model*0.5*sin(-CurModelPosLonlat2[2][CountI]);
        }
    }

}
#endif
/*.BH--------------------------------------------------------
**
** 函数名: ModelFeatureAutoRunRecv
**
** 描述:   三维模型自跑程序--接收参数
**
** 输入参数:  无
**
** 输出参数： 无
**
** 返回值：TRUE or FALSE
**
**
** 设计注记:  对外接口函数--模型自跑程序--接收参数
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

//1.设置自跑参数
void ModelDynamicParamSet()
{
    static double BeginLonLat[2] = {120.4,22.5};
    static double R_Model = 0.01;
    int i = 0;
    for(;;)
    {
        for(i=0;i<10;i++)
        {
    CurModelPosLonlat[2][i] =CurModelPosLonlat[2][i] + 0.03; //减号代表逆时针
    CurModelPosLonlat[0][i] = BeginLonLat[0]+R_Model*cos(CurModelPosLonlat[2][i]);
    CurModelPosLonlat[1][i] = BeginLonLat[1]+R_Model*sin(-CurModelPosLonlat[2][i]);

    CurModelPosLonlat2[2][i] =CurModelPosLonlat2[2][i] + 0.03; //减号代表逆时针
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
//2.绘制模型
BOOL ModelFeatureAutoRunRecv()
{
    //获取模型自跑参数
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
** 函数名: SetModelInitFlag
**
** 描述:   设置模型初始化完成的状态
**
** 输入参数:  TURE OR FALSE
**
** 输出参数： 无
**
** 返回值:		无
**
**
** 设计注记:  对外接口函数--模型初始化完成后，设置状态为已完成
**
**.EH--------------------------------------------------------
*/
void SetModelInitFlag(BOOL flag)
{
    g_ModelInitFinish = flag;
}

/*.BH--------------------------------------------------------
**
** 函数名: GetModelInitFlag
**
** 描述:   获取 模型初始化完成的状态
**
** 输入参数:  无
**
** 输出参数： 无
**
** 返回值:	TRUE OR FALSE
**
**
** 设计注记:  对外接口函数--获取模型初始化是否完成
**
**.EH--------------------------------------------------------
*/
BOOL GetModelInitFlag()
{
    return g_ModelInitFinish;
}

/*.BH--------------------------------------------------------
**
** 函数名: drawCommonModel
**
** 描述:   内部绘制模型的函数，在静态和动态绘制时调用
**
** 输入参数:  模型信息，经纬高等，模式选择
**
** 输出参数： 无
**
** 返回值:	无
**
**
** 设计注记:  内部处理函数,高度是地面高度+相对高度
**
**.EH--------------------------------------------------------
*/
static int s_nTwinkleTickPre = 0;		//上一次的tick数
static int s_nTwinkleTickStep = 300;	//闪动模型的tick数
void drawCommonModel(LPModelRender pCommonModel , sGLRENDERSCENE* pSceneModelDraw)
{
    int s_nTwinkleTickNow = 0;		//当前的tick 数
    switch(pCommonModel->m_modelDisplaytype)
    {

        //模型闪动显示，每隔10帧闪一次,修改为10个ticks 闪动一次
        case 1:
#ifdef	WIN32
                s_nTwinkleTickNow = GetTickCount();
#else
                s_nTwinkleTickNow = tickCountGet();
#endif
                if(( s_nTwinkleTickNow - s_nTwinkleTickPre) < s_nTwinkleTickStep)
                {
                    // 在第一个周期内绘制模型
                    RenderModelLLH(pCommonModel, pSceneModelDraw);
                }
                else if(( s_nTwinkleTickNow - s_nTwinkleTickPre) < (2* s_nTwinkleTickStep))
                {
                    // 在第二个周期内不绘制模型
                }
                else
                {
                    // 超过2个周期，循环第一个周期
                    s_nTwinkleTickPre = s_nTwinkleTickNow;
                }
                break;
#if 0   //在别的函数中已经处理以下情况
        //模型显示外框效果
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

                //模型效果复原为3, 大小复原
                set_model_displaytype(pCommonModel, 3);
                pCommonModel->m_modelScale *= (1/1.02);
                pCommonModel->m_MatrixChange = TRUE;

                break;

        //包罗线显示效果
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

                //模型效果复原为4
                set_model_displaytype(pCommonModel, 4);

                break;

        //光环效果
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

                //模型效果复原为5
                set_model_displaytype(pCommonModel, 5);

                break;
#endif
        //纯色效果
        case 6:
//				pCommonModel->m_colorNum_use = 3;
                //set_model_colorNumUse(pCommonModel, 3);
                RenderModelLLH(pCommonModel, pSceneModelDraw);

                break;

        //正常显示效果0, 线框显示效果2
        case 0:
        case 2:
        default:
                RenderModelLLH(pCommonModel, pSceneModelDraw);
                break;
        }

//	//闪动控制量递增
//	s_nTwinkleCount++;
//	if(s_nTwinkleCount > 20)
//	{
//		s_nTwinkleCount = 0;
//	}

}

/*.BH--------------------------------------------------------
**
** 函数名: LoadModelDisplayFile
**
** 描述:   读取静态显示配置文件
**
** 输入参数:
**
** 输出参数： 链表
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为第一层的函数
**
**.EH--------------------------------------------------------
*/
BOOL LoadModelDisplayFile()
{
    f_char_t file_path[1024] = {0};
    int i = 0;

    printf("读取静态显示配置文件... \n");

    memset(file_path, 0, 1024);
    strncpy(file_path, modelpath, 512);

    // 1.选择读取配置文件
    strncat(file_path, "modelDisplay.cfg", 16);

    // 2.增加多个地图场景的静态模型显示初始化
    for(i=0;i<SCENE_NUM;i++)
    {
        AttachpScene2Model(pScene[i]);

        if(FALSE == ReadModelDisplayFile(file_path))
        {
            printf("读取静态显示配置文件失败\n");
            return FALSE;
        }

    }

    printf("读取静态显示配置文件成功\n");

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: ReadModelDisplayFile
**
** 描述:   读取静态显示文件信息表
**
** 输入参数:  配置信息表路径
**
** 输出参数： 无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  此函数作为第二层的函数，加入到绘制链表中
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

    // 读取模型配置信息
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

        ModelStaticRender.m_heimode = 0;	//相对高度

        //model pitch, roll, yaw
        ModelStaticRender.m_modelAngle.y = readtxtfile_getFloat(pFile);
        ModelStaticRender.m_modelAngle.z = readtxtfile_getFloat(pFile);
        ModelStaticRender.m_modelAngle.x = readtxtfile_getFloat(pFile);

        //model scale
        ModelStaticRender.m_modelScale =  readtxtfile_getFloat(pFile);

        //跳过size模型尺寸
        //pData->size = readtxtfile_getFloat(pFile);

        //加入到绘制链表中
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

        //读取下一行
        readtxtfile_nextline(pFile);

    }

    fclose(pFile);
    return TRUE;
}




