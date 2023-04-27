#ifndef __TQSFILE_H__
#define __TQSFILE_H__

#include "../define/mbaseType.h"
#include "../mapApi/common.h"
 
#ifdef __cplusplus
extern "C" {
#endif

/*
	打开一个TQT文件，
	参数：cstrFileName -- 文件名称
	返回值：
	       文件句柄
*/
VOIDPtr tqsfileOpen(f_uint64_t rootlevel, f_uint64_t rootxidx, f_uint64_t rootyidx, const f_char_t * cstrFileName);

/*
	关闭一个已经打开的TQT文件，释放文件占用的资源
	参数：tqsfile -- 用tqsfileOpen返回的TQT文件句柄
	返回值：
	     0 -- 成功
		-1 -- 文件句柄无效
*/
f_int32_t tqsfileClose(VOIDPtr tqsfile);

/*
	得到一个已经打开的TQT文件中树的层数
	参数：tqsfile -- 用tqsfileOpen返回的TQT文件句柄
	返回值：
	    >0 -- 树的层数
		-1 -- 文件句柄无效
*/
f_int32_t tqsfileGetLevels(VOIDPtr tqsfile);

/*
	得到一个已经打开的TQT文件中树的起始层号
	参数：tqsfile -- 用tqsfileOpen返回的SQS文件句柄
	返回值：
	    >0 -- 树的起始层号
		-1 -- 文件句柄无效
*/
f_int32_t tqsfileGetLevelstart(VOIDPtr tqsfile);

/*
	得到一个已经打开的TQT文件中基本图块的大小
	参数：tqsfile -- 用tqsfileOpen返回的TQT文件句柄
	返回值：
	    >0 -- 基本图块的大小
		-1 -- 文件句柄无效
*/
f_int32_t tqsfileGetTileSize(VOIDPtr tqsfile);

/*
功能：判断节点是否在文件的范围内
输入：    
    level -- 指定的层，从0开始
	xidx, yidx -- 基本图块在层中的编号
	param  文件句柄所在的地址
输出：
    0  在
    -1 不在
*/
f_int32_t imgTqsIsExist(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param);

f_int32_t loadtqs(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);
f_int32_t tqs2tex(f_uint64_t param , f_uint8_t *pImg);

f_int32_t tqs2texRGBA(f_uint64_t param , f_uint8_t *pImg);
f_int32_t loadtqsRGBA(f_int32_t level, f_int32_t xidx, f_int32_t yidx, f_uint64_t param, f_uint8_t ** ppImg);


#ifdef __cplusplus
}
#endif

#endif /* __TQTFILE_H */
