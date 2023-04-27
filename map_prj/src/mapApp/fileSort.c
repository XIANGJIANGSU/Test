/*
ע�⣺���º�����windows�����µĺ�����Ƕ��ʽ�����»�������ʵ��
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

int bLoadUseList = 0;  //1-���Ӵ��̻�ȡ�ļ��б���ֱ�Ӹ������,Ƕ��ʽ����ʹ��,PC�¸�ֵ�����Ƕ��ٶ��Ӵ����л�ȡ
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

	/*bLoadUseList=0,�Ӵ��̲�ѯ�ļ����л�����������Ŀ¼*/
	if(!bLoadUseList)
	{
		
#if defined ACOREOS || defined SYLIXOS
		/*��Ժ������µ�chdir��ioDefPathGet������֧�֣����ٳ��Դ�����Ŀ¼��ֱ�ӽ�����Ŀ¼��ŵ�cur_dir*/
		strcpy(cur_dir, path);
#else

		/* ��ȡ��ǰĿ¼����ŵ�old_dir*/
	#ifdef WIN32	
		_getcwd( old_dir, _MAX_PATH);
    #elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        getcwd( old_dir, _MAX_PATH);
	#else 
		ioDefPathGet(old_dir);
	#endif

		/* �л���3d����·������ŵ�new_dir */
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

		/* ��ȡ�л���ǰĿ¼����ŵ�cur_dir����ʱӦ����3d����·�� */
	#ifdef WIN32	
		_getcwd( cur_dir, _MAX_PATH);
    #elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        getcwd( cur_dir, _MAX_PATH);
	#else 
		ioDefPathGet(cur_dir);
	#endif

		/* ����Ŀ¼��/��\�����⣬�˴����ٽ��бȽ� */
	#if 0
		/* ��new_dir��cur_dir�������ַ���ת��Сд�����бȽϣ�ȷ�ϵ�ǰ�Ѿ��л���3d����·�� */
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
		/*bLoadUseList=1,���Ӵ��̲�ѯ�ļ�,search_dir��ֱ�ӽ��ļ�������,ֻӰ��Ƕ��ʽ,��Ӱ��windows*/
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
        /* ���ҵ�ǰ·�����ļ��еĸ���������¼�ļ������ֵ�temp_dir_path */
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
            /* �л���ǰĿ¼�����ļ���Ŀ¼�� */
            strncpy(temp_dir, cur_dir, sizeof(cur_dir));
            strcat(temp_dir, "/");
            strcat(temp_dir, temp_dir_path[i]);
            _chdir(temp_dir);

            /* ��һ�β��ҵ�ǰ·���º�׺��Ϊfiletype���ļ�����������������¼�ļ��� */
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
    /* ͳ�ƺ�׺Ϊ*.ter���ļ�����:��һ�β��ҵ�ǰ·���º�׺��Ϊfiletype���ļ�����������������¼�ļ��� */
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

	/* �л���3d���ݸ�Ŀ¼ */
#ifdef WIN32
	if(_chdir(new_dir))
	{
		perror("dir <%s> open failed.\n",new_dir);
		return 0;
	}
#else
    {
        #if defined ACOREOS || defined SYLIXOS
            /*��Ժ������µ�chdir������֧�֣������л���3d���ݸ�Ŀ¼*/
        #else
            if(chdir(new_dir))
            {
                printf("dir <%s> open failed.\n",new_dir);
                return 0;
            }
        #endif
    }
#endif



	/* ��̬����file_count��sQTFName�ṹ�����ڴ�������ļ������ļ�Ŀ¼ */
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

	/* ����׺��Ϊfiletype���ļ���������Ϊ0 */
	file_count = 0;

#ifdef WIN32

	
			
	for (i=0 ; i<temp_dir_num; i++)
	{
		/* �л���ǰĿ¼�����ļ���Ŀ¼�� */
		strncpy(temp_dir, cur_dir, sizeof(cur_dir)); 
		strcat(temp_dir, "/");
		strcat(temp_dir, temp_dir_path[i]);
		_chdir(temp_dir);

		/* �ڶ��β��ҵ�ǰ·���º�׺��Ϊfiletype���ļ������������Ҽ�¼�ļ������ļ���·�� */
		done = _findfirst(filetype, &ffblk);
		do
		{
			if((-1 != done) && ((ffblk.attrib == _A_NORMAL)||(ffblk.attrib == _A_ARCH)))
			{
				strcpy((*fName_list)[file_count].name, ffblk.name);
				strcpy((*fName_list)[file_count].pathname, temp_dir_path[i]);
				/* �����ļ����õ���ʼ�㼶��x������Ƭ������y������Ƭ�����������ܲ㼶 */
				nameAnalyze(&(*fName_list)[file_count]);

				ret = _findnext(done,&ffblk);
				file_count++;
			}
		}while(0 == ret);		
	}

#else
	/* �ڶ��β��ҵ�ǰ·���º�׺��Ϊfiletype���ļ������������Ҽ�¼�ļ������ļ���·�� */
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

	/* �л��ؽ���˺���ǰϵͳ���ڵ�Ŀ¼ */
#ifdef WIN32
	_chdir(old_dir);
	_findclose(done);
#else

#if defined ACOREOS || defined SYLIXOS
	/*��Ժ������µ�chdir������֧�֣������л��ؽ���˺���ǰϵͳ���ڵ�Ŀ¼*/
#else
	chdir(old_dir);
#endif

#endif

	if(file_count)
	{
		/* ������ʼ�㼶��С����Խṹ��*fName_list�������� */
		quicksort(*fName_list, 0, file_count-1);
		/* ��ӡ���ҵ�������filetype��׺���ļ��� */
		printlist(*fName_list, file_count);
	}
	
	/* ���ز��ҵ�������filetype��׺���ļ����� */
	return(file_count);
}

