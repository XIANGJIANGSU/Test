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
extern int bLoadUseList;  //不从磁盘获取文件列表，而直接给出结果

int search_dir_uselist(char * token, char * dir_path, ChainHead **head)
{
	ChainNode * tempNode     = NULL;    /*文件链表临时节点指针*/

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
			/*将该节点插入链表表头*/
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
** 函数名: search_dir
**
** 描述: 从目录中搜索包含指定字符串的文件路径
**
** 输入参数:  char * token: 用于搜索的指定字符串字符串；
**            char * dir_path：当前搜索的目录路径。
**            ChainHead *head: 所有包含指定字符串的文件路径链表。
**
** 输出参数： 无。
**
** 返回值： 0：操作成功；
**          -1: 输入参数错误；
**          -2: 创建头节点失败；
**          -3：文件夹打开失败；
**          -4：读取文件夹信息出错。
**
** 设计注记:  无。
**
**.EH--------------------------------------------------------
*/
int search_dir(char * token, char * dir_path, ChainHead **head)
{
	int retval               = 0;       /*临时调用函数的返回值*/
	char * search_result     = NULL;    /*搜索文件名的返回值*/
	DIR * base_dir_p         = NULL;    /*文件夹指针*/
	struct dirent * next_dir = NULL;    /*文件夹信息指针*/
	struct stat fileStat;               /*文件状态*/
	ChainNode * tempNode     = NULL;    /*文件链表临时节点指针*/
	int i = 0;                          /*临时循环变量*/
	char tempFileName[256]    = {0};	/*临时存储文件名的小写转换*/
    char tempPath[1024]       = {0};    /*临时用于搜索的文件夹路径*/
    char tempPathAll[1024]    = {0};	/*临时存储文件名的路径地址*/

    memset(tempPath,0,sizeof (tempPath));
    memset(tempPathAll,0,sizeof (tempPathAll));

	if(NULL == token || NULL == dir_path)
		return -1;
	
	/*初始化文件链表*/
	if(NULL == *head)
	{
		/*不存在，尝试重新创建链首节点*/
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
	
	/*遍历指定文件夹里的子文件夹*/
	while(1)
	{
		/*读子文件夹*/
        //errno = OK;
		next_dir = readdir(base_dir_p);
		if(NULL == next_dir)
		{
#if 0
			if(OK == errno)
			{
				/*搜索到文件夹末尾*/
				closedir(base_dir_p);
				return 0;
			}else
			{
				/*搜索出错*/
				closedir(base_dir_p);
				printf("search dir error.\n");
				return -4;
			}
#else
			/*搜索到文件夹末尾*/
			closedir(base_dir_p);
			return 0;
#endif
		}
		/*读到文件或文件名*/
		/*获取文件状态，判断是否是文件夹*/
        memset(tempPath,0,sizeof (tempPath));   // 要先清空旧状态
		strcpy(tempPath, dir_path);
        strcat(tempPath, next_dir->d_name);
        strcat(tempPath, "/");

        // 判断当前路径是否为文件夹
        #if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
            if(next_dir->d_type != DT_DIR)
        #else
            stat(tempPath, &fileStat);
            if(FALSE == S_ISDIR(fileStat.st_mode))
        #endif

        /*不是文件夹，判断是否包含指定字符*/
        {
			/*先将文件名转成小写，输入的token必须是小写，这样无论电子盘中数据文件名是大写还是小写都没有问题*/
			memset(tempFileName,0,sizeof (tempFileName));   // 要先清空旧状态
			for(i = 0; i < strlen(next_dir->d_name); i++)
			{
				tempFileName[i] = (char)tolower(next_dir->d_name[i]);
			}
			search_result = strstr(tempFileName, token);
			if(NULL != search_result)
			{
				/*是要查找的文件*/
				//strcpy(tempPathAll, dir_path);
				//strcat(tempPathAll, next_dir->d_name);
				strcpy(tempPath, next_dir->d_name);
				tempNode = (ChainNode *)NewAlterableMemory(sizeof(ChainNode));
				if( (NULL != tempNode) && (NULL != *head) )
				{
					/*将该节点插入链表表头*/
					strcpy(tempNode->path, tempPath);
					strcpy(tempNode->pathALL, tempPathAll);
					tempNode->next = (*head)->first;
					(*head)->first = tempNode;
					(*head)->number++;						
				}
			}
			continue;
        }

        // 是文件夹，进入递归搜索
        else
		{
			if((strcmp(next_dir->d_name, ".") != 0) && (strcmp(next_dir->d_name, "..") != 0))
			{
				/*如果是文件夹，搜索下一级目录*/
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
