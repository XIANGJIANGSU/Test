/*-----------------------------------------------------------
**
** ��Ȩ: �й��������ߵ�����о���, 2015��
**
** �ļ���: PathStore.c
**
** ����: ���ļ������ɻ�������¼��������ȡ���ܵĺ�����
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
**		2016-12-26 9:19 LPF �������ļ�
**
**
**-----------------------------------------------------------
*/
/*-----------------------------------------------------------
** ͷ�ļ�����
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
** ��������
**-----------------------------------------------------------
*/


/*-----------------------------------------------------------
** �������ͺ�����
**-----------------------------------------------------------
*/

#define AIRLINEFILENUM 				16	//���߼�¼�ļ���������


/*-----------------------------------------------------------
** ȫ�ֱ�������
**-----------------------------------------------------------
*/
static BOOL PathFileExist = FALSE;		//�����ļ��Ƿ����
static FILE *  file_p = NULL;				//�����ļ����
static stList_Head AirLineStore[AIRLINEFILENUM];		//�ڲ��洢�ĺ�����������
static int AirLineCount[AIRLINEFILENUM] ;	//���߼�¼����ļ�������
static int AirLineNum = 0;				//���߼�¼�ļ����ܸ���
static stList_Head AirLinePNode[AIRLINEFILENUM];		//���߼�¼����ĵ�ǰ�ڵ����
static stList_Head m_listPathData;				//��ȡ�����ļ�������
static int AirLineNumCount[AIRLINEFILENUM];	//���߼�¼�����ÿ������Ľڵ����
/*-----------------------------------------------------------
** �ⲿ��������
**-----------------------------------------------------------
*/
//extern RenderStates g_RenderState;
extern BOOL AirLineRunICD ;	//ͨ��UDP ���յĺ����˶����� lpf add 2017-2-13 10:54:24
extern int AirLineSpeedICD ;		//ͨ��UDP ���յĺ����˶�����lpf add 2017-2-13 10:55:00
/*-----------------------------------------------------------
** �ڲ���������
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
** ��������
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
** ������: AirLineRecord
**
** ����:  ���߼�¼����,��¼pos��angle
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ⲿ�ӿ�1, ��һ�㺯������sphereRender.c��ʹ��
**
**.EH--------------------------------------------------------
*/
BOOL AirLineRecord()
{
    // 1.�ж��Ƿ�ʼ��¼
    if(BeginAirlineStore() == TRUE)
    {
        // 2.���ɺ��߼�¼�ļ�
        if(PathFileExist == FALSE)
        {
            if(FALSE == 	CreateAirlineFile(&file_p))
            {
                return FALSE;
            }
        }

        // 3.�洢�ɻ���pos��angle
        if(WriteAirlineFile(file_p) == FALSE)
        {
            return FALSE;
        }
    }

    // 4.�ж��Ƿ�رռ�¼���߹���
    if(EndAirlineStore() == TRUE)
    {
        // 5.�ر��ļ���������¼
        if(PathFileExist == TRUE)
        {
            CloseAirlineFile(file_p);

        }
    }

    return TRUE;

}

/*.BH--------------------------------------------------------
**
** ������: BeginAirlineStore
**
** ����:  ���߼�¼����--�ж��Ƿ�ʼ��¼����
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����1, �ڶ��㺯��,  ���������湦��Ϊ����
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
** ������: CreateAirlineFile
**
** ����:  ���߼�¼����--���ɺ��߼�¼�ļ�
**
** �������:  ��
**
** ���������	FILE* fp
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����2, �ڶ��㺯��
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
    // 0.�ж��Ƿ�����·��
    if (CreateDirectory(filepath_name,NULL) != 0)
    {
        printf("�ɹ��������߼�¼�ļ���\n");
    }
#else
    if (NULL == opendir(filepath_name))
    {
        if (OK == mkdir(filepath_name))
        {
            printf("�ɹ��������߼�¼�ļ���\n");
        }

    }

#endif

    // 1.�����ļ���,���ݵ�ǰϵͳʱ�������ļ���
#ifdef WIN32
    GetLocalTime(&sys_time);

    sprintf(file_name, "%4d-%02d-%02d-%02d-%02d-%02d",
        sys_time.wYear, sys_time.wMonth, sys_time.wDay, sys_time.wHour, sys_time.wMinute, sys_time.wSecond);
#else
    sprintf(file_name, "%d",tickGet());
#endif
    strncat(file_name,".dat",4);

    // 2.�����ļ�
    strcat(filepath_name, "//");
    strcat(filepath_name, file_name);

    // 2.1 �ж��ļ������Ƿ񳬹����ƣ������Ļ�ɾ�����ϵ�
    {
        char filename_list[AIRLINEFILENUM][256] = {0};			//���߼�¼�ļ����ļ���
        int file_num = 0;									//���߼�¼�ļ��ĸ���
        char filedelete_name[1024] = {0};						//��ɾ�����ļ�����
        int file_deleteid = 0;
        char filepath_namecal[1024] = AIRLINEPATH;

        // 2.1.1.�����ļ��б�
        file_num = OutPathFileName(filepath_namecal, filename_list);
        if(file_num >= 16)
        {

            // 2.1.2.ѡ�����ϵ��ļ�
            ClacDeleteFile(filename_list, file_num, &file_deleteid);

            // 2.1.3.ɾ�����ϵ��ļ�
            memcpy(filedelete_name,AIRLINEPATH,256);
            strcat(filedelete_name, "//");
            strcat(filedelete_name, filename_list[file_deleteid]);

            if(EOF == remove(filedelete_name))
            {
                printf("��������߼�¼�ļ��������޷�ɾ�����߼�¼�ļ�\n");
                return FALSE;

            }
            else
            {
                printf("��������߼�¼�ļ�������ɾ����¼�ļ��ɹ�\n");
            }

        }

    }
    *fp = fopen(filepath_name,"wb+");

    // 3.���÷���ֵ
    if(*fp == NULL)
    {
        ret = FALSE;
        printf("���ɺ��߼�¼�ļ�ʧ��\n");
        }
    else
    {
        ret = TRUE;
        printf("���ɺ��߼�¼�ļ��ɹ�\n");
        }

    // 4.д�뿪ʼ��־
    if(ret == TRUE)
    {
        fwrite(Flag_begin,sizeof(Flag_begin),1,*fp);
        printf("��ʼд�뺽�߼�¼�ļ�...\n");
    }

    // 5.����
    PathFileExist = TRUE;
    return ret;

}

/*.BH--------------------------------------------------------
**
** ������: EndAirlineStore
**
** ����:  ���߼�¼����--�жϽ�����¼����
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����3, �ڶ��㺯��,  ���������湦��Ϊ����
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
** ������: CloseAirlineFile
**
** ����:  ���߼�¼����--�رռ�¼�ļ�
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����4, �ڶ��㺯��
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

    printf("...����д�뺽�߼�¼�ļ�\n\n");

    PathFileExist = FALSE;
    fp = NULL;

    return TRUE;
}

/*.BH--------------------------------------------------------
**
** ������: WriteAirlineFile
**
** ����:  ���߼�¼����--����д���¼�ļ���
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����5, �ڶ��㺯��
**
**.EH--------------------------------------------------------
*/
BOOL WriteAirlineFile(FILE* fp, sGLRENDERSCENE* pModelScene)
{
    ModelState s_modelpath = {0};
    int i = 0;

    // 1.׼������
    s_modelpath.m_flag = 12341234;

    s_modelpath.m_modelPos.x = pModelScene->geopt_pos_input.lat;
    s_modelpath.m_modelPos.y = pModelScene->geopt_pos_input.lon;
    s_modelpath.m_modelPos.z = pModelScene->geopt_pos_input.height;

    s_modelpath.m_modelAngle.x = 360.0 - pModelScene->attitude_input.yaw;
    s_modelpath.m_modelAngle.y = pModelScene->attitude_input.pitch;
    s_modelpath.m_modelAngle.z = pModelScene->attitude_input.roll;

    // 2.д���ļ�
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

    // 3.����ֵ
    return TRUE;
}

#endif

/*.BH--------------------------------------------------------
**
** ������: AirLineRead
**
** ����:  ��ȡ���߼�¼�ļ�
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ⲿ�ӿ�2, ��һ�㺯��,�ڳ����ʼ����ʹ��
**
**.EH--------------------------------------------------------
*/
BOOL AirLineRead()
{
    char file_path[256] = AIRLINEPATH;					//���߼�¼�ļ���·��
    char filename_list[AIRLINEFILENUM][256] = {0};			//���߼�¼�ļ����ļ���
    int i = 0;
    int file_num = 0;									//��ȡ��file����
    char filepath_read[256] = AIRLINEPATH;				//����ȡ�����ļ���·��

    // 0.��ʼ������
    InitPathList();

    // 1.�����ļ��б�
    file_num = OutPathFileName(file_path, filename_list);
    if(FALSE == file_num)
    {
        return FALSE;
    }

    AirLineNum = file_num;		//�洢���߼�¼�ļ����ܸ���

    // 2.��ȡ�����ļ�
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
** ������: OutPathFileName
**
** ����:  ��ȡ���߼�¼�ļ�--����ļ����б�
**
** �������:  �ļ�·��
**
** �������:  �ļ����б�
**
** ����ֵ��FALSE ʧ�� or  �ļ�����
**
**
** ���ע��:  �ڲ�����6, �ڶ��㺯��,����X8C���ݼ��ز��ִ���
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

    // 1.windows��ʹ��FindFirstFile��Ҫ����·������*.dat�ſ���
    strcat(filepath,"/");
    strcat(filepath,"*.dat");

    // 2.ѭ�������ļ���
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

//	printf("��ʼ��ȡ�ļ���\n");
    dirReturn = opendir(filepath);
//	printf("���ļ��н�����\n");
    if(dirReturn == NULL)
    {	return FALSE;
        printf("���ļ���ʧ��\n");
    }
//	printf("���ļ��гɹ�\n");
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
//	printf("�ر��ļ���\n");
    printf("�����ļ����� = %d\n",NumData);
    for(i=0;i<NumData;i++)
    {
        printf("%s\n",filenamelist[i]);
    }

    return NumData;

}

#endif
/*.BH--------------------------------------------------------
**
** ������: ReadSingleLineFile
**
** ����:  ��ȡ���߼�¼�ļ�--��ȡ���庽���ļ�
**
** �������:  �ļ�·����
**
** �������:  ������������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����6, �ڶ��㺯��
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

    // 1.�򿪺����ļ�
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

    // 2.����ǰ8���ֽ��ж��Ƿ�Ƿ�
    fread(&readflag, sizeof(readflag), 1, pFile);
    if(strncmp(readflag, "AABB",4) != 0)
    {
        printf("read path file failed \n");
    }

    // 3.��ȡ�����ļ�
    while(1)
    {
        if (feof(pFile))
        {
            break;
        }

        pData = (LPModelState)NewFixedMemory(sizeof(ModelState));

        if ( fread(pTempRead,sizeof(ModelState) - sizeof(stList_Head),1,pFile) != 1)
        {
            //�˴�Ӧ���Ƕ������ļ�β����BBAA��
            continue;
//			printf("the model line file read error!\n");
        }

        // 4.�ж϶�ȡ�������Ƿ�Ƿ�

#ifndef WIN32
        // 4.1�ж��Ƿ���д�С��ת�����жϺ��߼�¼�ļ��Ƿ���PC�����ɵ�
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

        // 5.��ȡ������ȷ������뵽������
        memcpy(pData, pTempRead, sizeof(ModelState));

        //��������
        {
            /**�ҵ�ͷ���֮��**/
            stList_Head *pstListHead = NULL;
            pstListHead = (airlinelist);
            LIST_ADD(&(pData->stListHead), pstListHead);
            //LIST_ADD_BEFORE(&pData->stListHead, pstListHead);
        }




    }

    // 4.�ر��ļ�
    fclose(pFile);
    free(pTempRead);
    return TRUE;

}

/*.BH--------------------------------------------------------
**
** ������: SetModelAirline
**
** ����:  ���ݺ��߼�¼���ݣ���ֵ��ģ��λ�á���̬
**
** �������:  ģ��ID ��ָ��, ������AIRLINEFILENUM ֵ,ģ�͵ĸ���
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ⲿ�ӿ�3, ��һ�㺯�������ⲿsetParam��ʹ��
**
**.EH--------------------------------------------------------
*/
static BOOL SetTraceBegin = FALSE;
static BOOL SetTraceEnd = FALSE;
static int countmodelset = 0;		//lpf add for test ģ�͸�ֵ����
BOOL SetModelAirline(f_int32_t * model_id, f_int32_t model_Num)
{
    int i = 0;
#if 0
    double moveset = 0.00001;		//��������
    int moveset_num = (1/moveset)*(0.1);	//�����ܲ���

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

    // �����ٶȿ�����
    if((tickGet() % 16 ) != 0)
    {
        return FALSE;
    }
#endif
    // 0.�ж��Ƿ�ʼ��ֵ
    if(BeginModelAirlineSet() == TRUE)
    {
        // 1.���ģ�͸�ֵ
        for(i=0; i<model_Num; i++)
        {
            // 2.ֻ�м���ֵС��������ʱ��������
            if(AirLineCount[i]  < AirLineNumCount[i] )
            //if(AirLineCount[i]  < 2000)
            {
                if(SetTraceBegin == FALSE)
                {
                    // 2.1.��β������
                    SetModelUser(model_id[i], TRUE, 0);
                    AirLinePNode[i] = AirLineStore[i];
                }


                // 3. ��ֵ�������ģ��
                SetModelAirlineSingle(AirLineStore[i], AirLineCount[i] , model_id[i], AirLinePNode[i]);

                // 4.ѭ��������������
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

    // 5.�ж��Ƿ������ֵ
    if (EndModelAirlineSet() == TRUE)
    {
        // 6.ģ�ͼ������
        for(i=0; i<model_Num; i++)
        {
                // 7.ѭ����������Ϊ0
                AirLineCount[i] = 0;
                AirLinePNode[i] = AirLineStore[i];

                if(SetTraceEnd == FALSE)
                {
                    // 8.�ر�β������
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
** ������: SetModelAirlineSingle
**
** ����:  ���ݺ��߼�¼���ݣ���ֵ��ģ��λ�á���̬--���帳ֵ��ĳһ��ģ��
**
** �������:  ���߼�¼�������������е�������š�ģ��ID����ǰ��Ҫ��ֵ�Ľڵ�ָ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����7, �ڶ��㺯��
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


    //��ʱ�������ߴ�̨�嵽�Ϻ�
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
** ������: BeginModelAirlineSet
**
** ����:  ���ݺ��߼�¼���ݣ���ֵ��ģ��λ�á���̬--��ʼ��ֵ��ģ��
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����8, �ڶ��㺯��
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
** ������: EndModelAirlineSet
**
** ����:  ���ݺ��߼�¼���ݣ���ֵ��ģ��λ�á���̬--������ֵ��ģ��
**
** �������:  ��
**
** �����������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����9, �ڶ��㺯��
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
** ������: OutAirline
**
** ����:  ������ߵ������
**
** �������:  ������������
**
** ������������ߵ������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ⲿ�ӿ�4, ��һ�㺯�������ⲿsetParam��ʹ��
**
**.EH--------------------------------------------------------
*
static f_int32_t OutAirLineCount;				//�ⲿ����ĺ��ߵ��������
static LPNode OutAirLinePNode[AIRLINEFILENUM];					//�ⲿ����ĺ��ߵ�ָ�����
BOOL OutAirline(f_int32_t airline_id, LPModelState airline_out)
{
    // 1.ֻ�м���ֵС��������ʱ�����
    if(OutAirLineCount  >= AirLineStore[airline_id].m_nSize)
    {
        OutAirLineCount = 0;
        OutAirLinePNode[airline_id] = (LPNode)AirLineStore[airline_id].m_pFirst;
    }

    // 2.���Ҷ�Ӧ�ĺ��ߵ�
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
            //��麽�������Ƿ�������
            if (OutAirLinePNode[airline_id] == NULL)
            {
                return FALSE;
            }

            lpsinglemodel = (LPModelState)OutAirLinePNode[airline_id]->m_pCur;
            if(lpsinglemodel == NULL)
            {
                return FALSE;
            }
            // 3.������ߵ�����
//			if(i == OutAirLineCount)
            {
                memcpy(airline_out, lpsinglemodel, sizeof(ModelState));
                //break;

            }

//			pNode = pNode->m_pNext;
//			i ++;
        }

        // 4.ѭ��������������
            // ���뺽���˶��ļ��ٹ���
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
** ������: ClacDeleteFile
**
** ����:  �������ϵ��ļ�
**
** �������:  �ļ����б�
**
** ������������ϵ��ļ������
**
** ����ֵ��TRUE �ɹ�or FALSE ʧ��
**
**
** ���ע��:  �ڲ�����10
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

    // 1.�ļ���ȥ��.dat
    for(i=0; i<file_num; i++)
    {
        strncpy(&(filename_clac[i][0]), &(filenamelist[i][0]), 19);	//ֻ����ǰ19�������ַ�
    }

    // 2.�ļ����ַ���ת��ֵ
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

    // 3.������С����ֵ
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
����:���Դ��ļ��ж�ȡ�������ݣ���ֵ���ӵ�
*
extern sCTRPARAM3D  cmdParam[2][4];
extern sFLYPARAM    flyParam[2][4];
extern int AirLineNumICD;
void TestAirLineSet()
{
#if 1
    int airline_num = AirLineNumICD;		//ѡ��ĺ��߼�¼�ļ����,��udp �н���
    ModelState airline_choose = {0};	//����ĺ��߼�¼����

    // 1.���в����Ķ�ȡ��ֵ
    OutAirline(airline_num, &airline_choose);

    flyParam[0][0].lat = airline_choose.m_modelPos.x;
    flyParam[0][0].lon = airline_choose.m_modelPos.y;
    flyParam[0][0].height = airline_choose.m_modelPos.z;

    flyParam[0][0].yaw = airline_choose.m_modelAngle.x;
    flyParam[0][0].pitch = airline_choose.m_modelAngle.y;
    flyParam[0][0].roll =airline_choose.m_modelAngle.z;

    // 2.���ֿ�����������
    cmdParam[0][0].base.gRunType = 0;
    cmdParam[0][0].base.gFrazeType = 0;
    cmdParam[0][0].base.gdayNightMode = 0;
    cmdParam[0][0].base.gRoam = 0;

    cmdParam[0][0].base.measure_distance = 30.0f *1000;	 //������Ӧ
     cmdParam[0][0].base.g2d3dMod = 1;					//��ά
     cmdParam[0][0].base.g3dViewMod = 1;				//����
#endif
}

*/
