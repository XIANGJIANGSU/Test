/*
注意：以下函数是windows环境下的函数，嵌入式环境下还需另外实现
_finddata_t
_getcwd
_chdir
_findfirst
_findnext
_findclose
*/

#ifdef WIN32
#include <direct.h>
#include <io.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#else
#include "ioLib.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "searchDir.h"
#include "fileSort.h"
#include "../engine/memoryPool.h"

#if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#define _MAX_PATH 260     //260 comes from stdlib.h of windows
#endif

static void swap(sQTFName *x, sQTFName *y)
{
    sQTFName temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

static f_int32_t choose_pivot(f_int32_t i, f_int32_t j)
{
    return((i + j)/2);
}

static f_int32_t get_quicksort_key(sQTFName *list)
{
	if(list)
	{
		return (list->rootlevel + list->rootyidx);
	}else{
		return -1;
	}
	
}

static void quicksort(sQTFName *list, f_int32_t m, f_int32_t n)
{
    f_int32_t key, i, j, k;
	if(m < n)
	{
	    k = choose_pivot(m, n);
		swap(&list[m], &list[k]);
		//key = list[m].rootlevel;
		key = get_quicksort_key(&list[m]);
		i = m + 1;
		j = n;
		while(i <= j)
		{
		    //while((i <= n) && (list[i].rootlevel <= key))
			while((i <= n) && (get_quicksort_key(&list[i]) <= key))
				i++;
			//while((j >= m) && (list[j].rootlevel > key))
			while((j >= m) && (get_quicksort_key(&list[j]) > key))
				j--;
			if(i < j)
				swap(&list[i], &list[j]);
		}
		swap(&list[m], &list[j]);
		quicksort(list, m, j-1);
		quicksort(list, j+1, n);
	}
}

static void printlist(sQTFName *list, f_int32_t n)
{
    f_int32_t i;
	for(i = 0; i < n; i++)
		printf("%s\n", list[i].name);
}

static void nameAnalyze(sQTFName *fName)
{
	f_char_t name[64] = {0};
	f_char_t num[16] = {0};
	f_char_t *pdest;
    f_int32_t  result;
	f_char_t find = '_';
	f_char_t find_dot = '.';
	f_int32_t  find_count = 0;

	if(NULL != fName)
	{
	    strcpy(name, fName->name);
		
		do
		{
		    pdest = strchr(name, find);
		    result = pdest - name + 1;

			if( pdest != NULL )
			{
				strncpy(num, name, pdest - name);
				memset(name, '0', pdest - name + 1);
				switch(find_count)
				{
				case 0:
					fName->rootlevel = atoi(num);
					break;
				case 1:
					fName->rootyidx = atoi(num);
					break;
				case 2:
					fName->rootxidx = atoi(num);
					break;
				default:
					break;
				}
				find_count++;
			}
		}while(pdest != NULL);

		pdest = strchr(name, find_dot);
		result = pdest - name + 1;
		
		if( pdest != NULL )
		{
			strncpy(num, name, pdest - name);
			fName->level = atoi(num);
		}
	}
}

int bLoadUseList = 0;  //1-不从磁盘获取文件列表，而直接给出结果,嵌入式调试使用,PC下该值不管是多少都从磁盘中获取
static f_char_t temp_dir_path[1024][1024];

f_int32_t getSortedFile(f_char_t *path, sQTFName **fName_list, char * filetype)
{
    ChainHead *head = NULL;
	f_int64_t done;
    char new_dir[1024], cur_dir[1024], old_dir[1024];
	f_int32_t  file_count = 0;
	f_int32_t ret = -1;
	f_uint32_t index=0;

	f_char_t temp_dir[1024] = {0};
	f_int32_t temp_dir_num = 0;
	int i = 0;

#ifdef WIN32
	struct _finddata_t ffblk;
#endif

	memset(new_dir,0,sizeof(new_dir));
	memset(cur_dir,0,sizeof(cur_dir));
	memset(old_dir,0,sizeof(old_dir));

	/*bLoadUseList=0,从磁盘查询文件，切换到数据所在目录*/
	if(!bLoadUseList)
	{
		
#if defined ACOREOS || defined SYLIXOS
		/*翼辉和天脉下的chdir、ioDefPathGet函数不支持，不再尝试打开数据目录，直接将数据目录存放到cur_dir*/
		strcpy(cur_dir, path);
#else

		/* 获取当前目录，存放到old_dir*/
	#ifdef WIN32	
		_getcwd( old_dir, _MAX_PATH);
    #elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        getcwd( old_dir, _MAX_PATH);
	#else 
		ioDefPathGet(old_dir);
	#endif

		/* 切换到3d数据路径，存放到new_dir */
		strcpy(new_dir, path);

	#ifdef WIN32
		if(_chdir(new_dir))
	#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
		if(chdir(new_dir))
	#else
		
	#endif
		{
            printf("dir<%s> open failed.\n",new_dir);
			return 0;
		}

		/* 获取切换后当前目录，存放到cur_dir，此时应该是3d数据路径 */
	#ifdef WIN32	
		_getcwd( cur_dir, _MAX_PATH);
    #elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        getcwd( cur_dir, _MAX_PATH);
	#else 
		ioDefPathGet(cur_dir);
	#endif

		/* 避免目录中/和\的问题，此处不再进行比较 */
	#if 0
		/* 将new_dir和cur_dir的所有字符都转成小写，进行比较，确认当前已经切换到3d数据路径 */
		for(index=0; index<strlen(new_dir);index++)
		{
			new_dir[index] = (char)tolower(new_dir[index]);
		}
		for(index=0; index<strlen(cur_dir);index++)
		{
			cur_dir[index] = (char)tolower(cur_dir[index]);
		}
		
		if(0 != strcmp(new_dir, cur_dir))
		{
			perror("\n not into expend dir.");
			return 0;
		}
	#endif

#endif

	}else
	{
		/*bLoadUseList=1,不从磁盘查询文件,search_dir中直接将文件名给出,只影响嵌入式,不影响windows*/
		strcpy(cur_dir, path);
		strcpy(new_dir, path);
#ifdef WIN32
		if(_chdir(new_dir))
		{
			perror("dir <%s> open failed.\n",new_dir);
			return 0;
		}
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
		if(chdir(new_dir))
		{
			printf("dir <%s> open failed.\n",new_dir);
			return 0;
		}
#endif
	}
	
#ifdef WIN32
    {
        /* 查找当前路径下文件夹的个数，并记录文件夹名字到temp_dir_path */
        done = _findfirst("*.*", &ffblk);
        do
        {
            if((ffblk.attrib == _A_SUBDIR)&&(0 != strcmp(ffblk.name,"."))&&(0 != strcmp(ffblk.name,"..")))
            {
                temp_dir_num ++;
                memcpy(temp_dir_path[temp_dir_num -1], ffblk.name, sizeof(ffblk.name));
            }
        } while (-1 != _findnext(done,&ffblk));


        for (i=0 ; i<temp_dir_num; i++)
        {
            /* 切换当前目录到子文件夹目录下 */
            strncpy(temp_dir, cur_dir, sizeof(cur_dir));
            strcat(temp_dir, "/");
            strcat(temp_dir, temp_dir_path[i]);
            _chdir(temp_dir);

            /* 第一次查找当前路径下后缀名为filetype的文件个数，仅计数不记录文件名 */
            done = _findfirst(filetype, &ffblk);
            do
            {
                if((-1 != done) && ((ffblk.attrib == _A_NORMAL)||(ffblk.attrib == _A_ARCH)))
                {
                    ret = _findnext(done,&ffblk);
                    file_count++;
                }
            }while(0 == ret);
        }
    }

#else
    /* 统计后缀为*.ter的文件总数:第一次查找当前路径下后缀名为filetype的文件个数，仅计数不记录文件名 */
	done = search_dir(filetype, cur_dir, &head);
	if(0 == done)
	{
		ChainNode* node = head->first;		
		ChainNode* tmpNode;
		while(node)
		{
			tmpNode = node->next;
			//printf("%s\n", node->path);
			DeleteAlterableMemory(node);
			node = tmpNode;
			file_count++;
		}
		head->first = NULL;
	}
#endif

	/* 切换回3d数据根目录 */
#ifdef WIN32
	if(_chdir(new_dir))
	{
		perror("dir <%s> open failed.\n",new_dir);
		return 0;
	}
#else
    {
        #if defined ACOREOS || defined SYLIXOS
            /*翼辉和天脉下的chdir函数不支持，不再切换回3d数据根目录*/
        #else
            if(chdir(new_dir))
            {
                printf("dir <%s> open failed.\n",new_dir);
                return 0;
            }
        #endif
    }
#endif



	/* 动态申请file_count个sQTFName结构体用于存放数据文件名及文件目录 */
	if(file_count)
	{
		*fName_list = (sQTFName *)NewFixedMemory(sizeof(sQTFName) * file_count);
		if(NULL == *fName_list)
		{
			printf("fName_list = NULL\n");
			return 0;
		}
		memset((void *)(*fName_list), 0, sizeof(sQTFName) * file_count);
	}

	/* 将后缀名为filetype的文件个数重置为0 */
	file_count = 0;

#ifdef WIN32

	
			
	for (i=0 ; i<temp_dir_num; i++)
	{
		/* 切换当前目录到子文件夹目录下 */
		strncpy(temp_dir, cur_dir, sizeof(cur_dir)); 
		strcat(temp_dir, "/");
		strcat(temp_dir, temp_dir_path[i]);
		_chdir(temp_dir);

		/* 第二次查找当前路径下后缀名为filetype的文件个数，计数且记录文件名、文件夹路径 */
		done = _findfirst(filetype, &ffblk);
		do
		{
			if((-1 != done) && ((ffblk.attrib == _A_NORMAL)||(ffblk.attrib == _A_ARCH)))
			{
				strcpy((*fName_list)[file_count].name, ffblk.name);
				strcpy((*fName_list)[file_count].pathname, temp_dir_path[i]);
				/* 根据文件名得到起始层级、x方向瓦片索引、y方向瓦片索引、子树总层级 */
				nameAnalyze(&(*fName_list)[file_count]);

				ret = _findnext(done,&ffblk);
				file_count++;
			}
		}while(0 == ret);		
	}

#else
	/* 第二次查找当前路径下后缀名为filetype的文件个数，计数且记录文件名、文件夹路径 */
	done = search_dir(filetype, cur_dir, &head);
	if(0 == done)
	{
		ChainNode* node = head->first;	
		ChainNode* tmpNode;
		while(node)
		{
			strcpy((*fName_list)[file_count].name, node->path);
			strcpy((*fName_list)[file_count].pathname, node->pathALL);
			nameAnalyze(&(*fName_list)[file_count]);
			
			tmpNode = node->next;
			//printf("%s\n", node->path);
			DeleteAlterableMemory(node);
			node = tmpNode;
			file_count++;
		}
		head->first = NULL;
	}
#endif

	/* 切换回进入此函数前系统所在的目录 */
#ifdef WIN32
	_chdir(old_dir);
	_findclose(done);
#else

#if defined ACOREOS || defined SYLIXOS
	/*翼辉和天脉下的chdir函数不支持，不再切换回进入此函数前系统所在的目录*/
#else
	chdir(old_dir);
#endif

#endif

	if(file_count)
	{
		/* 根据起始层级从小到大对结构体*fName_list进行排序 */
		quicksort(*fName_list, 0, file_count-1);
		/* 打印查找到的所有filetype后缀的文件名 */
		printlist(*fName_list, file_count);
	}
	
	/* 返回查找到的所有filetype后缀的文件个数 */
	return(file_count);
}

