/*-----------------------------------------------------------
**
** 版权: 中国航空无线电电子研究所, 2015年
**
** 文件名: PathStore.c
**
** 描述: 本文件包含飞机航迹记录及航迹读取功能的函数。
**
** 定义的函数:
**
**
** 设计注记:
**
** 作者：
**		LPF。
**
**
** 更改历史:
**		2016-12-26 9:19 LPF 创建此文件
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** 头文件引用
**-----------------------------------------------------------
*/
#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#include "sys/stat.h"
#include "unistd.h"
#include <dirent.h>

#else
#include <dirent.h>
#include <stat.h>
#include <unistd.h>
#include <sysLib.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>


#include "../define/mbaseType.h"
#include "../define/macrodefine.h"
#include "../engine/libList.h"
#include "../mapApp/appHead.h"
#include "3DSLoader.h"
#include "PathStore.h"
#include "ModelApi.h"

#include "../../../include_vecMap/mapCommon.h"


/*-----------------------------------------------------------
** 类型声明
**-----------------------------------------------------------
*/


/*-----------------------------------------------------------
** 文字量和宏声明
**-----------------------------------------------------------
*/

#define AIRLINEFILENUM 				16	//航线记录文件的最大个数


/*-----------------------------------------------------------
** 全局变量定义
**-----------------------------------------------------------
*/
static BOOL PathFileExist = FALSE;		//航线文件是否存在
static FILE *  file_p = NULL;				//航线文件句柄
static stList_Head AirLineStore[AIRLINEFILENUM];		//内部存储的航线数据链表
static int AirLineCount[AIRLINEFILENUM] ;	//航线记录链表的计数变量
static int AirLineNum = 0;				//航线记录文件的总个数
static stList_Head AirLinePNode[AIRLINEFILENUM];		//航线记录链表的当前节点变量
static stList_Head m_listPathData;				//读取航线文件的链表
static int AirLineNumCount[AIRLINEFILENUM];	//航线记录链表的每个链表的节点个数
/*-----------------------------------------------------------
** 外部变量声明
**-----------------------------------------------------------
*/
//extern RenderStates g_RenderState;
extern BOOL AirLineRunICD ;	//通过UDP 接收的航线运动命令 lpf add 2017-2-13 10:54:24
extern int AirLineSpeedICD ;		//通过UDP 接收的航线运动倍数lpf add 2017-2-13 10:55:00
/*-----------------------------------------------------------
** 内部函数声明
**-----------------------------------------------------------
*/
BOOL BeginAirlineStore();
BOOL CreateAirlineFile(FILE** fp);
BOOL EndAirlineStore();
BOOL CloseAirlineFile(FILE* fp);
int OutPathFileName(char * filepath, char filenamelist[][256]);
BOOL ReadSingleLineFile(char * filepah, stList_Head * airlinelist);
BOOL SetModelAirlineSingle(stList_Head airline, f_int32_t airline_num,f_int32_t modelid, stList_Head pNodeNow);
BOOL ClacDeleteFile(char filenamelist[][256], int file_num, int *out_fileID);
BOOL BeginModelAirlineSet();
BOOL EndModelAirlineSet();
BOOL WriteAirlineFile(FILE* fp, sGLRENDERSCENE* pModelScene);

/*-----------------------------------------------------------
** 函数定义
**-----------------------------------------------------------
*/

void InitPathList()
{
    int i = 0;

    LIST_INIT(&m_listPathData);
    for (i=0; i<AIRLINEFILENUM; i++)
    {
        LIST_INIT(&AirLineStore[i]);
        LIST_INIT(&AirLinePNode[i]);
    }

}



#if 0
/*.BH--------------------------------------------------------
**
** 函数名: AirLineRecord
**
** 描述:  航线记录功能,记录pos和angle
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  外部接口1, 第一层函数，在sphereRender.c中使用
**
**.EH--------------------------------------------------------
*/
BOOL AirLineRecord()
{
    // 1.判断是否开始记录
    if(BeginAirlineStore() == TRUE)
    {
        // 2.生成航线记录文件
        if(PathFileExist == FALSE)
        {
            if(FALSE == 	CreateAirlineFile(&file_p))
            {
                return FALSE;
            }
        }

        // 3.存储飞机的pos和angle
        if(WriteAirlineFile(file_p) == FALSE)
        {
            return FALSE;
        }
    }

    // 4.判断是否关闭记录航线功能
    if(EndAirlineStore() == TRUE)
    {
        // 5.关闭文件，结束记录
        if(PathFileExist == TRUE)
        {
            CloseAirlineFile(file_p);

        }
    }

    return TRUE;

}

/*.BH--------------------------------------------------------
**
** 函数名: BeginAirlineStore
**
** 描述:  航线记录功能--判断是否开始记录航线
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数1, 第二层函数,  地形抛物面功能为开关
**
**.EH--------------------------------------------------------
*/
BOOL BeginAirlineStore()
{
//	if(g_RenderState.isTerrainProfile == TRUE)
    if(0)
    {
        return TRUE;

    }
    else
    {
        return FALSE;
    }


}

/*.BH--------------------------------------------------------
**
** 函数名: CreateAirlineFile
**
** 描述:  航线记录功能--生成航线记录文件
**
** 输入参数:  无
**
** 输出参数：	FILE* fp
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数2, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL CreateAirlineFile(FILE** fp)
{
    char file_name[1024] = {0};
    char Flag_begin[8] = "AABB";
    BOOL ret = FALSE;
    char filepath_name[1024] = AIRLINEPATH;
#ifdef WIN32
    SYSTEMTIME sys_time;
#endif

#ifdef WIN32
    // 0.判断是否生成路径
    if (CreateDirectory(filepath_name,NULL) != 0)
    {
        printf("成功创建航线记录文件夹\n");
    }
#else
    if (NULL == opendir(filepath_name))
    {
        if (OK == mkdir(filepath_name))
        {
            printf("成功创建航线记录文件夹\n");
        }

    }

#endif

    // 1.生成文件名,根据当前系统时间生成文件名
#ifdef WIN32
    GetLocalTime(&sys_time);

    sprintf(file_name, "%4d-%02d-%02d-%02d-%02d-%02d",
        sys_time.wYear, sys_time.wMonth, sys_time.wDay, sys_time.wHour, sys_time.wMinute, sys_time.wSecond);
#else
    sprintf(file_name, "%d",tickGet());
#endif
    strncat(file_name,".dat",4);

    // 2.生成文件
    strcat(filepath_name, "//");
    strcat(filepath_name, file_name);

    // 2.1 判断文件个数是否超过限制，超过的话删除最老的
    {
        char filename_list[AIRLINEFILENUM][256] = {0};			//航线记录文件的文件名
        int file_num = 0;									//航线记录文件的个数
        char filedelete_name[1024] = {0};						//待删除的文件名称
        int file_deleteid = 0;
        char filepath_namecal[1024] = AIRLINEPATH;

        // 2.1.1.查找文件列表
        file_num = OutPathFileName(filepath_namecal, filename_list);
        if(file_num >= 16)
        {

            // 2.1.2.选择最老的文件
            ClacDeleteFile(filename_list, file_num, &file_deleteid);

            // 2.1.3.删除最老的文件
            memcpy(filedelete_name,AIRLINEPATH,256);
            strcat(filedelete_name, "//");
            strcat(filedelete_name, filename_list[file_deleteid]);

            if(EOF == remove(filedelete_name))
            {
                printf("超过最大航线记录文件个数，无法删除航线记录文件\n");
                return FALSE;

            }
            else
            {
                printf("超过最大航线记录文件个数，删除记录文件成功\n");
            }

        }

    }
    *fp = fopen(filepath_name,"wb+");

    // 3.设置返回值
    if(*fp == NULL)
    {
        ret = FALSE;
        printf("生成航线记录文件失败\n");
        }
    else
    {
        ret = TRUE;
        printf("生成航线记录文件成功\n");
        }

    // 4.写入开始标志
    if(ret == TRUE)
    {
        fwrite(Flag_begin,sizeof(Flag_begin),1,*fp);
        printf("开始写入航线记录文件...\n");
    }

    // 5.返回
    PathFileExist = TRUE;
    return ret;

}

/*.BH--------------------------------------------------------
**
** 函数名: EndAirlineStore
**
** 描述:  航线记录功能--判断结束记录航线
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数3, 第二层函数,  地形抛物面功能为开关
**
**.EH--------------------------------------------------------
*/
BOOL EndAirlineStore()
{
//	if(g_RenderState.isTerrainProfile == FALSE)
    if(1)
    {
        return TRUE;

    }
    else
    {
        return FALSE;
    }

}

/*.BH--------------------------------------------------------
**
** 函数名: CloseAirlineFile
**
** 描述:  航线记录功能--关闭记录文件
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数4, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL CloseAirlineFile(FILE* fp)
{
    char Flag_end[8] = "BBAA";

    if (fp == NULL)
    {
        return FALSE;
        printf("the model path file write error!\n");

    }

    fwrite(Flag_end,sizeof(Flag_end),1,fp);

    fclose(fp);

    printf("...结束写入航线记录文件\n\n");

    PathFileExist = FALSE;
    fp = NULL;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: WriteAirlineFile
**
** 描述:  航线记录功能--内容写入记录文件夹
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数5, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL WriteAirlineFile(FILE* fp, sGLRENDERSCENE* pModelScene)
{
    ModelState s_modelpath = {0};
    int i = 0;

    // 1.准备数据
    s_modelpath.m_flag = 12341234;

    s_modelpath.m_modelPos.x = pModelScene->geopt_pos_input.lat;
    s_modelpath.m_modelPos.y = pModelScene->geopt_pos_input.lon;
    s_modelpath.m_modelPos.z = pModelScene->geopt_pos_input.height;

    s_modelpath.m_modelAngle.x = 360.0 - pModelScene->attitude_input.yaw;
    s_modelpath.m_modelAngle.y = pModelScene->attitude_input.pitch;
    s_modelpath.m_modelAngle.z = pModelScene->attitude_input.roll;

    // 2.写入文件
    if (fp == NULL)
    {
        return FALSE;
        printf("the model path file write error!\n");

    }
    if (fwrite(&s_modelpath,sizeof(s_modelpath),1,fp) != 1)
    {
        return FALSE;
        printf("the model path file write error!\n");
    }

    // 3.返回值
    return TRUE;
}

#endif

/*.BH--------------------------------------------------------
**
** 函数名: AirLineRead
**
** 描述:  读取航线记录文件
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  外部接口2, 第一层函数,在程序初始化中使用
**
**.EH--------------------------------------------------------
*/
BOOL AirLineRead()
{
    char file_path[256] = AIRLINEPATH;					//航线记录文件的路径
    char filename_list[AIRLINEFILENUM][256] = {0};			//航线记录文件的文件名
    int i = 0;
    int file_num = 0;									//读取的file个数
    char filepath_read[256] = AIRLINEPATH;				//待读取单个文件的路径

    // 0.初始化链表
    InitPathList();

    // 1.查找文件列表
    file_num = OutPathFileName(file_path, filename_list);
    if(FALSE == file_num)
    {
        return FALSE;
    }

    AirLineNum = file_num;		//存储航线记录文件的总个数

    // 2.读取单个文件
    for(i=0; i< file_num; i++)
    {
//		filepath_read = AIRLINEPATH;
        memcpy(filepath_read,AIRLINEPATH,256);
        strcat(filepath_read, "//");
        strcat(filepath_read, filename_list[i]);

        ReadSingleLineFile(filepath_read, &(AirLineStore[i]));

        AirLineNumCount[i] = GetNodeNum(&(AirLineStore[i]));
    }



    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: OutPathFileName
**
** 描述:  读取航线记录文件--输出文件名列表
**
** 输入参数:  文件路径
**
** 输出参数:  文件名列表
**
** 返回值：FALSE 失败 or  文件个数
**
**
** 设计注记:  内部函数6, 第二层函数,参照X8C数据加载部分代码
**
**.EH--------------------------------------------------------
*/
#ifdef WIN32
int OutPathFileName(char * filepath, char filenamelist[][256])
{
    WIN32_FIND_DATA  ffd ;
    HANDLE hFind ;
    int i=0;
    char test[256] = {0};

    // 1.windows下使用FindFirstFile需要加入路径名和*.dat才可以
    strcat(filepath,"/");
    strcat(filepath,"*.dat");

    // 2.循环搜索文件名
    hFind = FindFirstFile(filepath, &ffd);
    do
    {
        if(hFind != INVALID_HANDLE_VALUE)
        {
            strncpy(filenamelist[i] ,ffd.cFileName,128);
            i++;
        }
    }
    while(FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);

    return i;
}
#else
int OutPathFileName(char * filepath, char filenamelist[][256])
{
    int j =0;
    int i =0;
//	char NameList[64][256] = {0};
    FILE * fp = NULL;
    DIR * dirReturn = NULL;
    struct dirent * direntReturn = NULL;
    int NumData = 0;

//	printf("开始读取文件夹\n");
    dirReturn = opendir(filepath);
//	printf("打开文件夹进行中\n");
    if(dirReturn == NULL)
    {	return FALSE;
        printf("打开文件夹失败\n");
    }
//	printf("打开文件夹成功\n");
    while(1)
    {
        direntReturn = readdir(dirReturn);
        if(direntReturn == NULL)
        {
            break;
        }

        if(strlen(direntReturn->d_name) <= 4)
        {continue;}
        strcpy(filenamelist[i] ,direntReturn->d_name);
        i++;
    }

    NumData = i;

    closedir(dirReturn);
//	printf("关闭文件夹\n");
    printf("航线文件个数 = %d\n",NumData);
    for(i=0;i<NumData;i++)
    {
        printf("%s\n",filenamelist[i]);
    }

    return NumData;

}

#endif
/*.BH--------------------------------------------------------
**
** 函数名: ReadSingleLineFile
**
** 描述:  读取航线记录文件--读取具体航线文件
**
** 输入参数:  文件路径名
**
** 输出参数:  航线数据链表
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数6, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL ReadSingleLineFile(char * filepath, stList_Head * airlinelist)
{
    FILE *pFile = NULL;

    stList_Head *pstListHead = (airlinelist);
    stList_Head *pstTmpList = NULL;

    LPModelState pTempRead = NULL;
    LPModelState pData = NULL;
    char readflag[8] = {0};

    pTempRead = malloc(sizeof(ModelState));
    memset(pTempRead,'0',sizeof(ModelState));

    // 1.打开航线文件
    if(filepath == NULL)
    {
        return FALSE;
    }
    pFile = fopen(filepath, "rb");
    if(pFile == NULL)
    {
        printf("open %s failed.\n",filepath);
        fclose(pFile);
        return FALSE;
    }

    // 2.根据前8个字节判断是否非法
    fread(&readflag, sizeof(readflag), 1, pFile);
    if(strncmp(readflag, "AABB",4) != 0)
    {
        printf("read path file failed \n");
    }

    // 3.读取航线文件
    while(1)
    {
        if (feof(pFile))
        {
            break;
        }

        pData = (LPModelState)NewFixedMemory(sizeof(ModelState));

        if ( fread(pTempRead,sizeof(ModelState) - sizeof(stList_Head),1,pFile) != 1)
        {
            //此处应该是读到的文件尾部，BBAA；
            continue;
//			printf("the model line file read error!\n");
        }

        // 4.判断读取的数据是否非法

#ifndef WIN32
        // 4.1判断是否进行大小端转换，判断航线记录文件是否是PC下生成的
        if(NULL != strrchr(filepath,'-'))
        {

            pTempRead->m_flag=ConvertL2B_int(pTempRead->m_flag);
            pTempRead->m_modelAngle.x = ConvertL2B_double(pTempRead->m_modelAngle.x);
            pTempRead->m_modelAngle.y = ConvertL2B_double(pTempRead->m_modelAngle.y);
            pTempRead->m_modelAngle.z = ConvertL2B_double(pTempRead->m_modelAngle.z);
            pTempRead->m_modelPos.x = ConvertL2B_double(pTempRead->m_modelPos.x);
            pTempRead->m_modelPos.y = ConvertL2B_double(pTempRead->m_modelPos.y);
            pTempRead->m_modelPos.z = ConvertL2B_double(pTempRead->m_modelPos.z);
        }
#endif

        if (pTempRead->m_flag != 12341234)
        {
            continue;
        }

        // 5.读取数据正确，则加入到链表中
        memcpy(pData, pTempRead, sizeof(ModelState));

        //加入链表
        {
            /**挂到头结点之后**/
            stList_Head *pstListHead = NULL;
            pstListHead = (airlinelist);
            LIST_ADD(&(pData->stListHead), pstListHead);
            //LIST_ADD_BEFORE(&pData->stListHead, pstListHead);
        }




    }

    // 4.关闭文件
    fclose(pFile);
    free(pTempRead);
    return TRUE;

}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelAirline
**
** 描述:  根据航线记录数据，赋值给模型位置、姿态
**
** 输入参数:  模型ID 的指针, 不超过AIRLINEFILENUM 值,模型的个数
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  外部接口3, 第一层函数，在外部setParam中使用
**
**.EH--------------------------------------------------------
*/
static BOOL SetTraceBegin = FALSE;
static BOOL SetTraceEnd = FALSE;
static int countmodelset = 0;		//lpf add for test 模型赋值计数
BOOL SetModelAirline(f_int32_t * model_id, f_int32_t model_Num)
{
    int i = 0;
#if 0
    double moveset = 0.00001;		//步进长度
    int moveset_num = (1/moveset)*(0.1);	//步进总步数

    //lpf add 2017-3-1 14:14:37
    if(countmodelset > moveset_num)
    {
        moveset *= -1;

        if(countmodelset > (moveset_num*2))
        {
            countmodelset = 0;
        }
    }

    for(i=0;i<model_Num;i++)
    {
        SetModelUser(model_id[i], TRUE,0);
        SetModelPosAdd(model_id[i], moveset, 0);

    }
    countmodelset ++;
    return TRUE;

    // 设置速度控制量
    if((tickGet() % 16 ) != 0)
    {
        return FALSE;
    }
#endif
    // 0.判断是否开始赋值
    if(BeginModelAirlineSet() == TRUE)
    {
        // 1.针对模型赋值
        for(i=0; i<model_Num; i++)
        {
            // 2.只有计数值小于链表数时才有意义
            if(AirLineCount[i]  < AirLineNumCount[i] )
            //if(AirLineCount[i]  < 2000)
            {
                if(SetTraceBegin == FALSE)
                {
                    // 2.1.打开尾迹开关
                    SetModelUser(model_id[i], TRUE, 0);
                    AirLinePNode[i] = AirLineStore[i];
                }


                // 3. 赋值给具体的模型
                SetModelAirlineSingle(AirLineStore[i], AirLineCount[i] , model_id[i], AirLinePNode[i]);

                // 4.循环计数变量递增
                AirLineCount[i] ++;
                AirLinePNode[i] = *(AirLinePNode[i].pPriv);
            }
            else
            {
                AirLineCount[i] = 0;
                AirLinePNode[i] = AirLineStore[i];
            }
        }

        SetTraceBegin = TRUE;
        SetTraceEnd = FALSE;

        return TRUE;
    }

    // 5.判断是否结束赋值
    if (EndModelAirlineSet() == TRUE)
    {
        // 6.模型计数清空
        for(i=0; i<model_Num; i++)
        {
                // 7.循环计数变量为0
                AirLineCount[i] = 0;
                AirLinePNode[i] = AirLineStore[i];

                if(SetTraceEnd == FALSE)
                {
                    // 8.关闭尾迹开关
                    SetModelUser(model_id[i], FALSE, 0);
                }


        }

        SetTraceEnd = TRUE;
        SetTraceBegin = FALSE;
        return FALSE;
    }


    return TRUE;

}

/*.BH--------------------------------------------------------
**
** 函数名: SetModelAirlineSingle
**
** 描述:  根据航线记录数据，赋值给模型位置、姿态--具体赋值给某一个模型
**
** 输入参数:  航线记录数据链表、链表中的数据序号、模型ID、当前需要赋值的节点指针
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数7, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL SetModelAirlineSingle(stList_Head airline, f_int32_t airline_num,f_int32_t modelid, stList_Head pNodeNow)
{
    /*
    int i = 0;

    LPNode pNode = (LPNode)airline->m_pFirst;
    LPModelState lpsinglemodel = NULL;

    while(pNode != NULL)
    {
        lpsinglemodel = (LPModelState)pNode->m_pCur;

        if(i == airline_num)
        {
            SetModelPos(modelid, lpsinglemodel->m_modelPos);
            SetModelAngle(modelid, lpsinglemodel->m_modelAngle);
        }

        pNode = pNode->m_pNext;
        i ++;
    }
    */
    LPModelState lpsinglemodel = NULL;

    if(&pNodeNow == NULL)
    {
        return FALSE;
    }

    lpsinglemodel = ASDE_LIST_ENTRY(pNodeNow.pPriv, ModelState, stListHead);

    if(lpsinglemodel == NULL)
    {
        return FALSE;
    }


    //临时调整航线从台湾到上海
#if 1
    {
        PT_3D temp_taiwan_2_sh = {0};
        temp_taiwan_2_sh.x = lpsinglemodel->m_modelPos.x + 8.15;
        temp_taiwan_2_sh.y = lpsinglemodel->m_modelPos.y - 16.52;
        temp_taiwan_2_sh.z = lpsinglemodel->m_modelPos.z + 500;

        SetModelPos(modelid, temp_taiwan_2_sh);
    }
#else
    SetModelPos(modelid, lpsinglemodel->m_modelPos);
#endif
    SetModelAngle(modelid, lpsinglemodel->m_modelAngle);

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** 函数名: BeginModelAirlineSet
**
** 描述:  根据航线记录数据，赋值给模型位置、姿态--开始赋值给模型
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数8, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL BeginModelAirlineSet()
{
//	if(g_RenderState.isBirdViewOpen == TRUE)
    if(1)
    {
        return TRUE;

    }
    else
    {
        return FALSE;
    }
}

/*.BH--------------------------------------------------------
**
** 函数名: EndModelAirlineSet
**
** 描述:  根据航线记录数据，赋值给模型位置、姿态--结束赋值给模型
**
** 输入参数:  无
**
** 输出参数：无
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数9, 第二层函数
**
**.EH--------------------------------------------------------
*/
BOOL EndModelAirlineSet()
{
//	if(g_RenderState.isBirdViewOpen == FALSE)
    if(0)
    {
        return TRUE;

    }
    else
    {
        return FALSE;
    }
}

/*.BH--------------------------------------------------------
**
** 函数名: OutAirline
**
** 描述:  输出航线点的数据
**
** 输入参数:  航线链表的序号
**
** 输出参数：航线点的数据
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  外部接口4, 第一层函数，在外部setParam中使用
**
**.EH--------------------------------------------------------
*
static f_int32_t OutAirLineCount;				//外部输出的航线点计数变量
static LPNode OutAirLinePNode[AIRLINEFILENUM];					//外部输出的航线点指针变量
BOOL OutAirline(f_int32_t airline_id, LPModelState airline_out)
{
    // 1.只有计数值小于链表数时才输出
    if(OutAirLineCount  >= AirLineStore[airline_id].m_nSize)
    {
        OutAirLineCount = 0;
        OutAirLinePNode[airline_id] = (LPNode)AirLineStore[airline_id].m_pFirst;
    }

    // 2.查找对应的航线点
    {
        int i = 0;
//		LPNode pNode = (LPNode)AirLineStore[airline_id].m_pFirst;
        LPModelState lpsinglemodel = NULL;

        if(OutAirLinePNode[airline_id] == NULL)
        {
            OutAirLinePNode[airline_id] = (LPNode)AirLineStore[airline_id].m_pFirst;
        }

//		while(pNode != NULL)
        {
//			lpsinglemodel = (LPModelState)pNode->m_pCur;
            //检查航线链表是否有数据
            if (OutAirLinePNode[airline_id] == NULL)
            {
                return FALSE;
            }

            lpsinglemodel = (LPModelState)OutAirLinePNode[airline_id]->m_pCur;
            if(lpsinglemodel == NULL)
            {
                return FALSE;
            }
            // 3.输出航线点数据
//			if(i == OutAirLineCount)
            {
                memcpy(airline_out, lpsinglemodel, sizeof(ModelState));
                //break;

            }

//			pNode = pNode->m_pNext;
//			i ++;
        }

        // 4.循环计数变量递增
            // 加入航线运动的加速功能
            if(AirLineSpeedICD <= 1)
                {
                OutAirLineCount ++;
                OutAirLinePNode[airline_id] = OutAirLinePNode[airline_id]->m_pNext;
            }
            else
                {
                OutAirLineCount += AirLineSpeedICD;
                for(i=0;i<AirLineSpeedICD;i++)
                {
                    if (OutAirLinePNode[airline_id]->m_pNext != NULL)
                    {
                        OutAirLinePNode[airline_id] = OutAirLinePNode[airline_id]->m_pNext;
                    }

                }
            }

    }

    return TRUE;
}
*/
/*.BH--------------------------------------------------------
**
** 函数名: ClacDeleteFile
**
** 描述:  计算最老的文件
**
** 输入参数:  文件名列表
**
** 输出参数：最老的文件名序号
**
** 返回值：TRUE 成功or FALSE 失败
**
**
** 设计注记:  内部函数10
**
**.EH--------------------------------------------------------
*
BOOL ClacDeleteFile(char filenamelist[][256], int file_num, int *out_fileID)
{
    int i = 0;
    char filename_clac[AIRLINEFILENUM][256] = {0};
#ifdef WIN32
    LONG64 filename_integra[AIRLINEFILENUM] = {0};
    LONG64 min_file = 0;

#else
    int filename_integra[AIRLINEFILENUM] = {0};
    int min_file = 0;
#endif
    int min_fileid = 0;

    // 1.文件名去掉.dat
    for(i=0; i<file_num; i++)
    {
        strncpy(&(filename_clac[i][0]), &(filenamelist[i][0]), 19);	//只拷贝前19个数字字符
    }

    // 2.文件名字符串转数值
    for(i=0; i<file_num; i++)
    {
        int a,b,c,d,e,f;
#ifdef WIN32
        sscanf(filename_clac[i],"%d-%d-%d-%d-%d-%d",&a,&b,&c,&d,&e,&f);

        filename_integra[i] = f + e*100 + d*10000 + c * 1000000 + b * 100000000 + a * 10000000000;
#else
        char sdat[16]= {0};
        sscanf(filename_clac[i],"%d.%s",&a,&sdat);
        filename_integra[i] = filename_clac[i];
#endif
    }

    // 3.计算最小的数值
    min_file = filename_integra[0];
    for(i=0; i<file_num; i++)
    {
        if(min_file > filename_integra[i])
        {
            min_file = filename_integra[i];
            min_fileid = i;
        }
    }

    *out_fileID = min_fileid;

    return TRUE;
}

*/
/*
功能:测试从文件中读取航线数据，赋值给视点
*
extern sCTRPARAM3D  cmdParam[2][4];
extern sFLYPARAM    flyParam[2][4];
extern int AirLineNumICD;
void TestAirLineSet()
{
#if 1
    int airline_num = AirLineNumICD;		//选择的航线记录文件序号,从udp 中接收
    ModelState airline_choose = {0};	//输出的航线记录数据

    // 1.飞行参数的读取赋值
    OutAirline(airline_num, &airline_choose);

    flyParam[0][0].lat = airline_choose.m_modelPos.x;
    flyParam[0][0].lon = airline_choose.m_modelPos.y;
    flyParam[0][0].height = airline_choose.m_modelPos.z;

    flyParam[0][0].yaw = airline_choose.m_modelAngle.x;
    flyParam[0][0].pitch = airline_choose.m_modelAngle.y;
    flyParam[0][0].roll =airline_choose.m_modelAngle.z;

    // 2.部分控制量的设置
    cmdParam[0][0].base.gRunType = 0;
    cmdParam[0][0].base.gFrazeType = 0;
    cmdParam[0][0].base.gdayNightMode = 0;
    cmdParam[0][0].base.gRoam = 0;

    cmdParam[0][0].base.measure_distance = 30.0f *1000;	 //量程响应
     cmdParam[0][0].base.g2d3dMod = 1;					//三维
     cmdParam[0][0].base.g3dViewMod = 1;				//座舱
#endif
}

*/
