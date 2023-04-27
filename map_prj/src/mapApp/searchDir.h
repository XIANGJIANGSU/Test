#ifndef SEARCHDIR_H
#define SEARCHDIR_H



/*-----------------------------------------------------------
** ���Ͷ���
**-----------------------------------------------------------
*/
/*����ṹ*/
typedef struct chain_node
{
	char   path[128];           /*·��*/
	char   pathALL[256];	/*����·�����������ļ���*/
	struct chain_node * next;   /*��һ���ڵ�ָ��*/
}ChainNode;

typedef struct head_node
{
	int number;        /*��Ŀ*/
	ChainNode * first; /*��һ���ڵ�ָ��*/
}ChainHead;


int search_dir(char * token, char * dir_path, ChainHead **head);


#endif