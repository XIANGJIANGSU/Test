/**
 * @file osAdapter.h
 * @brief ���ļ��ṩ�����ź��������������ϵͳ��صĽӿں���
 * @author 615��ͼ�Ŷ�
 * @date 2016-04-22
 * @version v1.0.0
 * @copyright 615��ͼ�Ŷ�\n
 * ������ʷ��    ����        ������      ��������  \n
 * 
*/

#ifndef _osAdapter_h_ 
#define _osAdapter_h_ 

#ifdef WIN32
#include <windows.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
#include <semaphore.h>
#else
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>
#include <vxCpuLib.h>
#endif

#ifdef ACOREOS
#include <tasks.h>
#endif

#include "../define/mbaseType.h"


#define Android_system			//���尲������ϵͳ�ĺ궨��
#undef Android_system				//���尲������ϵͳ�ĺ궨��

#ifdef Android_system
#define MAPNODEDELNUM	30
#else
#define MAPNODEDELNUM	90
#endif



#define SEM_TAKE_ERROR  -1
#define SEM_TAKE_OK    0


// win32
#ifdef WIN32
#define FOREVER_WAIT    		-1
#define WAIT_NO          		 0

//#define UNBREAKABLE_VX   						0x0002
#define FP_TASK_VX       		 0x0008
//#define PRIVATE_ENV_VX   						0x0080
#define NO_STACK_FILL_VX 	 0x0100

#define EMPTY_SEM      		 0
#define FULL_SEM         		 1		

#define Q_FIFO_SEM    		 0
#define Q_PRIORITY_SEM  		 1

#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)

#define WAIT_FOREVER -1
#define FOREVER_WAIT     -1
#define WAIT_NO          0

#define SEM_TAKE_ERROR  -1
#define SEM_TAKE_OK    	0

#define UNBREAKABLE_VX   0x0002
#define FP_TASK_VX       0x0008
#define PRIVATE_ENV_VX   0x0080
#define NO_STACK_FILL_VX 0x0100

#define EMPTY_SEM        0
#define SEM_EMPTY   	 0
#define FULL_SEM         1
#define SEM_FULL     	1	

#define Q_FIFO_SEM       0
#define Q_PRIORITY_SEM   1
#define SEM_Q_FIFO    	0
#define SEM_Q_PRIORITY   1

// vxworks
#else
#define FOREVER_WAIT    		WAIT_FOREVER		// -1
#define WAIT_NO          		NO_WAIT 			//0

//#define UNBREAKABLE_VX   						0x0002
#define FP_TASK_VX       		VX_FP_TASK 			//0x0008
//#define PRIVATE_ENV_VX   						0x0080
#define NO_STACK_FILL_VX 	VX_NO_STACK_FILL 	//0x0100

#define EMPTY_SEM      		SEM_EMPTY  			// 0
#define FULL_SEM         		SEM_FULL 			// 1		

#define Q_FIFO_SEM    		SEM_Q_FIFO 			//   0
#define Q_PRIORITY_SEM  		SEM_Q_PRIORITY 		// 1

#endif

#ifdef WIN32
typedef HANDLE SEMID;
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
typedef sem_t* SEMID;
typedef sem_t SEMID_T;
#else
typedef SEM_ID SEMID;
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @fn SEMID createBSem(f_int32_t options, f_int32_t initialState)
 *  @brief This routine allocates and initializes a binary semaphore.
 *  @param[in] options:Tasks can be queued on a priority basis or a first-in-first-out basis. 
                       These options are Q_PRIORITY_SEM (0x1) and Q_FIFO_SEM (0x0), respectively.
 *  @param[in] initialState:The semaphore is initialized to the initialState of either FULL_SEM (1) or EMPTY_SEM (0).
 *  @exception void.
 *  @return SEMID,semID.
 * @see 
 * @note
*/
SEMID createBSem(f_int32_t options, f_int32_t initialState);

/**
 * @fn f_int32_t takeSem(SEMID semId, f_int32_t timeout)
 *  @brief ��ȡ�ź���.
 *  @param[in] semId �ź���ID��. 
 *  @param[in] timeout timeout in ticks.  
 *  @exception void.
 *  @return f_int32_t status.
 * @see 
 * @note
*/
f_int32_t takeSem(SEMID semId, f_int32_t timeout);

/**
 * @fn void giveSem(SEMID semId)
 *  @brief �ͷ��ź���.
 *  @param[in] semId �ź���ID��. 
 *  @exception void.
 *  @return void.
 * @see 
 * @note
*/
void giveSem(SEMID semId);

/**
 * @fn f_int32_t spawnTask(f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int32_t arg)
 *  @brief ��������.
 *  @param[in] name ������.
 *  @param[in] priority �������ȼ�.
 *  @param[in] options  ����ѡ��. 
 *  @param[in] stackSize ��ջ��С��BYTE��.
 *  @param[in] entryPt ������ں���. 
 *  @param[in] arg ����.  
 *  @exception entryPtError ��ں���Ϊ��.
 *  @return f_int32_t taskID.
 * @see 
 * @note
*/
f_int32_t spawnTask(f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int64_t arg);

int taskGetPriority(int* policy,int* priority);

#ifndef WIN32
/**
 * @fn f_int32_t spawnTaskAffinity(f_int32_t coreId, f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int32_t arg)
 *  @brief �����������.
 *  @param[in] coreId ��id.
 *  @param[in] name ������.
 *  @param[in] priority �������ȼ�.
 *  @param[in] options  ����ѡ��.
 *  @param[in] stackSize ��ջ��С��BYTE��.
 *  @param[in] entryPt ������ں���.
 *  @param[in] arg ����.
 *  @exception entryPtError ��ں���Ϊ��.
 *  @return f_int32_t taskID.
 * @see
 * @note
*/
f_int32_t spawnTaskAffinity(f_int32_t coreId, f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int64_t arg);

#endif

#ifdef ACOREOS
/**
 * @fn f_int32_t spawnTaskTM3(f_uint32_t coreid_affinity, f_uint32_t initial_priority, f_uint32_t stack_size,
		f_uint8_t* task_name, ACoreOs_task_entry task_func, f_uint32_t argument, void* tstTaskid)
 *  @brief ����3�´�������.
 *  @param[in] coreid_affinity �������˺�.
 *  @param[in] initial_priority �������ȼ�.
 *  @param[in] stack_size  ��ջ��С��BYTE��.
 *  @param[in] task_name ��������.
 *  @param[in] task_func ������ں���.
 *  @param[in] argument ������ں�������.
 *  @param[in] tstTaskid ����ID.
 *  @exception entryPtError ��ں���Ϊ��.
 *  @return f_int32_t taskID.
 * @see
 * @note
*/
f_int32_t spawnTaskTM3(f_uint32_t coreid_affinity, f_uint32_t initial_priority, f_uint32_t stack_size,
		f_uint8_t* task_name, ACoreOs_task_entry task_func, f_uint32_t argument, void* tstTaskid);
#endif

/**
 * @fn f_uint64_t tickCountGet(void)
 *  @brief ��ȡtick counter.  
 *  @exception void.
 *  @return f_uint64_t.
 * @see 
 * @note
*/
f_uint64_t tickCountGet(void);

/**
 * @fn f_int32_t sysClkRateGet()
 *  @brief ��ȡϵͳʱ��Ƶ��.  
 *  @exception void.
 *  @return f_int32_t.
 * @see 
 * @note
*/
f_int32_t getSysClkRate();


#ifdef __cplusplus
}
#endif // __cplusplus

#endif 
