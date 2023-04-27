#ifndef __BILDATA_H
#define __BILDATA_H

typedef void * BILDATA;

#define INVALID_DEM_VALUE (-9999)
#define BILDATAKEY	0xBDAEDADE
#define BILLEVELS	4
#define MAXBILBUF	100
#define BILSIZE	    256
#define BILSIZE2    (BILSIZE * BILSIZE)
#define H_SCALE_DEGREE 3
#define V_SCALE_DEGREE 3
#define MAXBLOCKBILFILE 20

/*高程数据文件操作管理结构体*/
typedef struct tagBILBlockFile
{
	struct tagBILBlockFile * pPrev, * pNext;  /*高程数据文件链表指针,分别指向前一个和后一个数据文件*/
	FILE *pFile;                              /*高程数据文件指针*/
	char blockFileName[256];                  /*高程数据文件路径*/
}sBILBlockFILE;

/*单图幅高程数据存储管理结构体*/
typedef struct tagBILFile
{
	struct tagBILFile * pPrev, * pNext;       /*高程数据图幅链表指针,分别指向前一个和后一个图幅*/
	int hstart, vstart, hend, vend;           /*高程数据图幅左下角经度、左下角纬度、右下角经度、右下角纬度*/
	short data[1];                            /*真实高程数据,会根据实际图幅数据大小动态申请内存*/
}sBILFILE;

/*单层级高程数据管理结构体*/
typedef struct tagBILLevelData
{
	char pathname[256];           /*单层级高程数据所在的目录*/
	int  hscale;                  /*单层级高程数据中每个图幅(每个文件包含多幅图)代表的经度跨度,1/128秒为单位*/
	int  vscale;                  /*单层级高程数据中每个图幅(每个文件包含多幅图)代表的纬度跨度,1/128秒为单位*/
	int  bufCount;                /*所有高程数据文件中存放的高程图幅个数*/
	int  blockCount;              /*高程数据文件总个数*/
	sBILFILE bufHeader;	          /*管理单个图幅数据*/
	sBILBlockFILE blockHeader;    /*打开的单个高程数据文件指针管理*/
}sBILLEVEL;

/*所有层级高程数据管理总结构体*/
typedef struct tagBILDATA
{
	unsigned long key;             /*must be BILDATAKEY*/
	sBILLEVEL levels[BILLEVELS];   /*不同层级的高程数据管理句柄,支持4个不同层级*/
}sBILDATA;

#ifdef __cplusplus
extern "C" {
#endif

/*
	生成一个所有层级高程数据管理总结构体的句柄，
	参数：pBILFile -- 如果成功则返回文件句柄供后续调用使用
	返回值：
	     0 -- 成功，文件句柄在pBILFile中
		-1 -- 内存分配失败
*/
int bildataCreate(BILDATA * pBILFile);

/*
	删除BILDATA句柄，释放所占用的资源
	参数：bildata -- 用bildataCreate返回的BILDATA句柄
	返回值：
	     0 -- 成功
		-1 -- 句柄无效
*/
int bildataDestroy(BILDATA bildata);

/*
	更新指定层级的高程数据目录、高程图幅经度跨度、高程图幅纬度跨度到所有层级高程数据管理总结构体中
	参数：bildata -- 用bildataCreate返回的BILDATA句柄
		  level -- 层号 0 ~ 3
	      cstrPathName -- 以/结尾的目录名，表示该层数据所在目录
		  hscale -- 一个图幅的经度跨度, 1/128秒单位
		  vscale -- 一个图幅的纬度跨度，1/128秒单位
    返回值：
	      0 -- 成功
		 -1 -- 句柄无效
		 -2 -- 层级参数无效
*/
int bildataSetDataPath(BILDATA bildata, int level, const char * cstrPathName, int hscale, int vscale);

/*
	功能：从指定层级的原始高程数据中查询指定经纬度的高程
	参数：bildata -- 用bildataCreate返回的BILDATA句柄
		  level -- 层号 0-3
		  h -- 经度, 1/128秒为单位
		  v -- 纬度, 1/128秒为单位
	返回值：
		  >-9999: 正常高程
		   -9999: 原始高程数据不存在
		  -10001: 原始高程数据超出范围
		  -10002: 数据句柄无效
		  -10003: 参数无效
		  -10004: 缓存空间出错
*/
short bildataGetHeight(BILDATA bildata, int level, int h, int v);

/*
 * 显示当前内存中已载入的图幅高程数量及已打开的图幅数据文件数量
 * */
void bildataPrintCacheCount(BILDATA bildata, int level);

#ifdef __cplusplus
}
#endif

#endif /* __BILFILE_H */