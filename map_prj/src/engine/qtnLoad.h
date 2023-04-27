#ifndef _qtnLoad_h_ 
#define _qtnLoad_h_ 

#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	
/*
	判断是否需要重新调度qtn 数据
	参数：pHandle -- 全局唯一的树句柄
	      pScene  -- 绘制场景句柄
    返回值：
	      无
*/
void QtnSqlJudge(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene);


/*
	从sqlite数据库中重新查找并更新地名数据
	参数：pHandle -- 全局唯一的树句柄
    返回值：
	      无
*/
void QtnSqlLoad(sMAPHANDLE *pHandle);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 