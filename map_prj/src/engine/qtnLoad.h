#ifndef _qtnLoad_h_ 
#define _qtnLoad_h_ 

#include "mapRender.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	
/*
	�ж��Ƿ���Ҫ���µ���qtn ����
	������pHandle -- ȫ��Ψһ�������
	      pScene  -- ���Ƴ������
    ����ֵ��
	      ��
*/
void QtnSqlJudge(sMAPHANDLE *pHandle, sGLRENDERSCENE *pScene);


/*
	��sqlite���ݿ������²��Ҳ����µ�������
	������pHandle -- ȫ��Ψһ�������
    ����ֵ��
	      ��
*/
void QtnSqlLoad(sMAPHANDLE *pHandle);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 