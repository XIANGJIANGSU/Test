#ifndef WIN32

// linux
#if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#include "sys/stat.h"
#include "unistd.h"
#include "dirent.h"

// vxworks
#else
#include "vxworks.h"
#include "dirent.h"

#ifndef SYLIXOS
#include "stat.h"
#endif

#endif


#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "searchDir.h"
#include "dirent.h"
#include "../engine/memoryPool.h"

#ifdef ACOREOS
#include <errno.h>
#endif


#if 1 
extern int bLoadUseList;  //���Ӵ��̻�ȡ�ļ��б���ֱ�Ӹ������

int search_dir_uselist(char * token, char * dir_path, ChainHead **head)
{
	ChainNode * tempNode     = NULL;    /*�ļ�������ʱ�ڵ�ָ��*/

	printf("%s: %s, %s\n", __FUNCTION__, token, dir_path);
	if (strcmp(token, ".ter") == 0)
	{
		int		i;
		const char *pTerFiles[] = {
				"0_0_0_6.ter",
				"0_0_1_6.ter",
				"0_0_2_6.ter",
				"0_0_3_6.ter",
				"0_1_0_6.ter",
				"0_1_1_6.ter",
				"0_1_2_6.ter",
				"0_1_3_6.ter"
		};
		for (i=0; i<sizeof(pTerFiles)/sizeof(pTerFiles[0]); i++)
		{
			tempNode = (ChainNode *)NewAlterableMemory(sizeof(ChainNode));
			/*���ýڵ���������ͷ*/
			strcpy(tempNode->path, pTerFiles[i]);
			sprintf(tempNode->pathALL, "%s/0-5/", dir_path);
			tempNode->next = (*head)->first;
			(*head)->first = tempNode;
			(*head)->number++;
			printf("%s,%d. add file: %s\n", __FUNCTION__, __LINE__, tempNode->path);
		}
	}
	return 0;
}
#endif

/*.BH--------------------------------------------------------
**
** ������: search_dir
**
** ����: ��Ŀ¼����������ָ���ַ������ļ�·��
**
** �������:  char * token: ����������ָ���ַ����ַ�����
**            char * dir_path����ǰ������Ŀ¼·����
**            ChainHead *head: ���а���ָ���ַ������ļ�·������
**
** ��������� �ޡ�
**
** ����ֵ�� 0�������ɹ���
**          -1: �����������
**          -2: ����ͷ�ڵ�ʧ�ܣ�
**          -3���ļ��д�ʧ�ܣ�
**          -4����ȡ�ļ�����Ϣ����
**
** ���ע��:  �ޡ�
**
**.EH--------------------------------------------------------
*/
int search_dir(char * token, char * dir_path, ChainHead **head)
{
	int retval               = 0;       /*��ʱ���ú����ķ���ֵ*/
	char * search_result     = NULL;    /*�����ļ����ķ���ֵ*/
	DIR * base_dir_p         = NULL;    /*�ļ���ָ��*/
	struct dirent * next_dir = NULL;    /*�ļ�����Ϣָ��*/
	struct stat fileStat;               /*�ļ�״̬*/
	ChainNode * tempNode     = NULL;    /*�ļ�������ʱ�ڵ�ָ��*/
	int i = 0;                          /*��ʱѭ������*/
	char tempFileName[256]    = {0};	/*��ʱ�洢�ļ�����Сдת��*/
    char tempPath[1024]       = {0};    /*��ʱ�����������ļ���·��*/
    char tempPathAll[1024]    = {0};	/*��ʱ�洢�ļ�����·����ַ*/

    memset(tempPath,0,sizeof (tempPath));
    memset(tempPathAll,0,sizeof (tempPathAll));

	if(NULL == token || NULL == dir_path)
		return -1;
	
	/*��ʼ���ļ�����*/
	if(NULL == *head)
	{
		/*�����ڣ��������´������׽ڵ�*/
		*head = (ChainHead*)NewAlterableMemory(sizeof(ChainHead));
		if(NULL == *head)
		{
			printf("create head node error.\n");
			return -2;
		}
		(*head)->number = 0;
		(*head)->first  = NULL;			
	}
	
	if(bLoadUseList)
	{ 
		return search_dir_uselist(token, dir_path, head);
	}

	strcpy(tempPathAll, dir_path);

	
	strcat(dir_path, "/");
	base_dir_p = opendir(dir_path);
	if(NULL == base_dir_p)
	{
		printf("open base dir <%s> error.\n",tempPathAll);
		return -3;
	}
	
	/*����ָ���ļ���������ļ���*/
	while(1)
	{
		/*�����ļ���*/
        //errno = OK;
		next_dir = readdir(base_dir_p);
		if(NULL == next_dir)
		{
#if 0
			if(OK == errno)
			{
				/*�������ļ���ĩβ*/
				closedir(base_dir_p);
				return 0;
			}else
			{
				/*��������*/
				closedir(base_dir_p);
				printf("search dir error.\n");
				return -4;
			}
#else
			/*�������ļ���ĩβ*/
			closedir(base_dir_p);
			return 0;
#endif
		}
		/*�����ļ����ļ���*/
		/*��ȡ�ļ�״̬���ж��Ƿ����ļ���*/
        memset(tempPath,0,sizeof (tempPath));   // Ҫ����վ�״̬
		strcpy(tempPath, dir_path);
        strcat(tempPath, next_dir->d_name);
        strcat(tempPath, "/");

        // �жϵ�ǰ·���Ƿ�Ϊ�ļ���
        #if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
            if(next_dir->d_type != DT_DIR)
        #else
            stat(tempPath, &fileStat);
            if(FALSE == S_ISDIR(fileStat.st_mode))
        #endif

        /*�����ļ��У��ж��Ƿ����ָ���ַ�*/
        {
			/*�Ƚ��ļ���ת��Сд�������token������Сд���������۵������������ļ����Ǵ�д����Сд��û������*/
			memset(tempFileName,0,sizeof (tempFileName));   // Ҫ����վ�״̬
			for(i = 0; i < strlen(next_dir->d_name); i++)
			{
				tempFileName[i] = (char)tolower(next_dir->d_name[i]);
			}
			search_result = strstr(tempFileName, token);
			if(NULL != search_result)
			{
				/*��Ҫ���ҵ��ļ�*/
				//strcpy(tempPathAll, dir_path);
				//strcat(tempPathAll, next_dir->d_name);
				strcpy(tempPath, next_dir->d_name);
				tempNode = (ChainNode *)NewAlterableMemory(sizeof(ChainNode));
				if( (NULL != tempNode) && (NULL != *head) )
				{
					/*���ýڵ���������ͷ*/
					strcpy(tempNode->path, tempPath);
					strcpy(tempNode->pathALL, tempPathAll);
					tempNode->next = (*head)->first;
					(*head)->first = tempNode;
					(*head)->number++;						
				}
			}
			continue;
        }

        // ���ļ��У�����ݹ�����
        else
		{
			if((strcmp(next_dir->d_name, ".") != 0) && (strcmp(next_dir->d_name, "..") != 0))
			{
				/*������ļ��У�������һ��Ŀ¼*/
				retval = search_dir(token, tempPath, head);
				if(0 != retval)
				{
					closedir(base_dir_p);
					return retval;
				}				
			}
		}
	}  /* while(1) */
}

#endif
