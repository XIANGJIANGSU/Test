#ifdef WIN32
    #include <memory.h>
    #include <time.h>
    #include <windows.h>
    #include <winsock.h>
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
    #include <memory.h>
    #include <sys/time.h>
    #include <sys/socket.h>
    #include <pthread.h>
#else
    #include <inetLib.h>
    #include <semLib.h>
#ifndef SYLIXOS
    #include <sockLib.h>
#else
    #include <sys/socket.h>
#endif
    #include <taskLib.h>
    #include <time.h>
    #include <vxWorks.h>
    #include <cpuset.h>
#endif

#include "osAdapter.h"

/**
 * @fn SEMID createBSem(f_int32_t options, f_int32_t initialState)
 *  @brief This routine allocates and initializes a binary semaphore.
 *  @param[in] options:Tasks can be queued on a priority basis or a first-in-first-out basis.
                       These options are Q_PRIORITY_SEM (0x1) and Q_FIFO_SEM (0x0), respectively.
 *  @param[in] initialState:The semaphore is initialized to the initialState of either FULL_SEM (1) or EMPTY_SEM (0).
 *  @exception void.
 *  @return unsigned SEMID,semID.
 * @see
 * @note
*/
SEMID createBSem(f_int32_t options, f_int32_t initialState)
{
    SEMID sem;
#ifdef WIN32
    sem = CreateSemaphore(NULL, 0, 1, NULL);
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
    sem =(SEMID) malloc(sizeof(SEMID_T));              // SEMID is a pointer, so must malloc memory for its object
    memset(sem, 0, sizeof(SEMID_T));	               // init the value of the pointer 
	// todo: when to release the memory ???????
	sem_init(sem, options, initialState);
#else
    sem = semBCreate(options, initialState);
#endif
    return(sem);
}

/**
 * @fn f_int32_t takeSem(SEMID semId, f_int32_t timeout)
 *  @brief 获取信号量.
 *  @param[in] semId 信号量ID号.
 *  @param[in] timeout timeout in ticks.
 *  @exception void.
 *  @return f_int32_t status.
 * @see
 * @note
*/
f_int32_t takeSem(SEMID semId, f_int32_t timeout)
{
    f_int32_t status = SEM_TAKE_ERROR;
    if(NULL != semId)
    {
    #ifdef WIN32
        if(WaitForSingleObject(semId, timeout) == WAIT_OBJECT_0)
            status = SEM_TAKE_OK;
    #elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
		if(-1 == timeout)
			status = sem_wait(semId);    //blocking
		else
		{
            struct timeval now;
            struct timespec out_time;

            gettimeofday(&now, NULL);
            out_time.tv_sec = now.tv_sec + timeout/1000.0;
            out_time.tv_nsec = now.tv_usec * 1000;

            sem_timedwait(semId, &out_time);
        }
    #else
        if(semTake(semId, timeout) == OK)
            status = SEM_TAKE_OK;
    #endif
    }
    return(status);
}

/**
 * @fn void giveSem(SEMID semId)
 *  @brief 释放信号量.
 *  @param[in] semId 信号量ID号.
 *  @exception void.
 *  @return void.
 * @see
 * @note
*/
void giveSem(SEMID semId)
{
	int value =0;
    if(NULL != semId)
    {
    #ifdef WIN32
        ReleaseSemaphore(semId, 1, NULL);
    #elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        sem_getvalue(semId,&value);
        if(value == 0)
            sem_post(semId);
    #else
        semGive(semId);
    #endif
    }
}

/**
 * @fn f_int32_t spawnTask(f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int32_t arg)
 *  @brief 创建任务.
 *  @param[in] name 任务名.
 *  @param[in] priority 任务优先级.
 *             linux下：优先级从小到大(1~99)
 *  @param[in] options  任务选项.
 *  @param[in] stackSize 堆栈大小（BYTE）.
 *  @param[in] entryPt 任务入口函数.
 *  @param[in] arg 参数.
 *  @exception entryPtError 入口函数为空.
 *  @return f_int32_t taskID.
 * @see
 * @note
*/
f_int32_t spawnTask(f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int64_t arg)
{
    f_int32_t taskID = 0;

#if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)	
	pthread_t temp_tid;
	pthread_attr_t attr;
	struct sched_param sche_priority;
	f_int32_t err;
#endif

    if(NULL == entryPt)
        return 0;

    // WIN32
#ifdef WIN32
    taskID = (f_int32_t)CreateThread(NULL, stackSize, entryPt, (void *)arg, 0, NULL);

    // linux
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
    err = pthread_attr_init(&attr);
	if(0 != err)
	{
        printf("XXXXXXXXX create pthread arrt error!!!!!!!!!!!\n");
		return 0;
	}
    #if defined(__linux__)
        // pthread_attr_setschedpolicy(&attr,SCHED_OTHER);
        pthread_attr_setschedpolicy(&attr,SCHED_RR);
    #else
        pthread_attr_setschedpolicy(&attr,SCHED_NORMAL);
    #endif
	sche_priority.sched_priority = priority;
	pthread_attr_setschedparam(&attr,&sche_priority);
	pthread_attr_setstacksize(&attr,stackSize);
	err=pthread_create(&temp_tid,&attr,entryPt,(void*)arg);
	pthread_attr_destroy(&attr);

	if(0 != err)
	{
       printf("XXXXXXXXXXXXX create pthread error!!!!!!!!!!!!\n");
		 return 0;
	}

	taskID = (f_int32_t)temp_tid;
    printf("[spawnTask]  create a thread: %d is created.\n",taskID);

    //vxworks
#else
    taskID = taskSpawn(name, priority, options, stackSize, (FUNCPTR)entryPt, arg, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

    return taskID;
}

// 获取当前任务优先级信息
int taskGetPriority(int* policy,int* priority)
{
    #if defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
        struct sched_param my_param; 
        pthread_getschedparam(pthread_self(),policy,&my_param);
        *priority = my_param.sched_priority;
    #endif
}

#ifndef WIN32
#ifndef __linux__
/**
 * @fn f_int32_t spawnTaskAffinity(f_int32_t coreId, f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int32_t arg)
 *  @brief 创建绑核任务.
 *  @param[in] coreId 核id.
 *  @param[in] name 任务名.
 *  @param[in] priority 任务优先级.
 *  @param[in] options  任务选项.
 *  @param[in] stackSize 堆栈大小（BYTE）.
 *  @param[in] entryPt 任务入口函数.
 *  @param[in] arg 参数.
 *  @exception entryPtError 入口函数为空.
 *  @return f_int32_t taskID.
 * @see
 * @note
*/
f_int32_t spawnTaskAffinity(f_int32_t coreId, f_char_t *name, f_int32_t priority, f_int32_t options, f_int32_t stackSize, void *entryPt, f_int64_t arg)
{
    f_int32_t taskID = 0;
    cpuset_t affinity;

    if(NULL == entryPt)
        return 0;

    taskID = taskCreate(name, priority, options, stackSize, (FUNCPTR)entryPt, arg, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    if(0 == taskID)
    {
        return -1;
    }

    CPUSET_ZERO(affinity);
    CPUSET_SET(affinity, coreId);

    if( ERROR == taskCpuAffinitySet(taskID, affinity) )
    {
        taskDelete(taskID);
        return -2;
    }

    taskActivate(taskID);

    return taskID;
}
#endif
#endif

#ifdef ACOREOS
/**
 * @fn f_int32_t spawnTaskTM3(f_uint32_t coreid_affinity, f_uint32_t initial_priority, f_uint32_t stack_size,
        f_uint8_t* task_name, ACoreOs_task_entry task_func, f_uint32_t argument, void* tstTaskid)
 *  @brief 天脉3下创建任务.
 *  @param[in] coreid_affinity 处理器核号.
 *  @param[in] initial_priority 任务优先级.
 *  @param[in] stack_size  堆栈大小（BYTE）.
 *  @param[in] task_name 任务名称.
 *  @param[in] task_func 任务入口函数.
 *  @param[in] argument 任务入口函数参数.
 *  @param[in] tstTaskid 任务ID.
 *  @exception entryPtError 入口函数为空.
 *  @return f_int32_t taskID.
 * @see
 * @note
*/
f_int32_t spawnTaskTM3(f_uint32_t coreid_affinity, f_uint32_t initial_priority, f_uint32_t stack_size,
        f_uint8_t* task_name, ACoreOs_task_entry task_func, f_uint32_t argument, void* tstTaskid)
{
    ACoreOs_Task_Param taskCreateParam;
    f_int32_t retCode = ACOREOS_SUCCESSFUL;

    /* 在此加入应用代码，任务无亲和属性设置ACOREOS_TASK_NO_AFFINITY，否则给定处理器核号 */
    taskCreateParam.affinity = coreid_affinity;
    taskCreateParam.attribute_set =  ACOREOS_PREEMPT | ACOREOS_TIMESLICE;
    taskCreateParam.domain = (void *)ACOREOS_KERNEL_ID;
    taskCreateParam.initial_priority = initial_priority;
    taskCreateParam.stack_size = stack_size;

    retCode = ACoreOs_task_create((ACoreOs_name)task_name, &taskCreateParam, &tstTaskid);
    if ( retCode != ACOREOS_SUCCESSFUL )
    {
        printf("Failed to create mapMain, return code:%#x!\n",retCode);
        return retCode;
    }

    retCode = ACoreOs_task_start(tstTaskid, (ACoreOs_task_entry)task_func, argument);
    if ( retCode != ACOREOS_SUCCESSFUL )
    {
        printf("Failed to start mapMain, return code:%d!\n", retCode);
        return retCode;
    }

    return retCode;
}
#endif

/**
 * @fn f_uint64_t tickCountGet(void)
 *  @brief 获取tick counter.
 *  @exception void.
 *  @return f_uint64_t.
 * @see
 * @note
*/
f_uint64_t tickCountGet(void)
{
    f_uint64_t count = 0;

    // win32
#ifdef WIN32
    count = GetTickCount();

    // linux
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
   struct timespec tickcount;
    clock_gettime(CLOCK_MONOTONIC,&tickcount);
	count=1000*tickcount.tv_sec+tickcount.tv_nsec/1000000;

    // vxworks
#else
    count = tickGet();
#endif
    return(count);
}

/**
 * @fn f_int32_t sysClkRateGet()
 *  @brief 获取系统时钟频率.
 *  @exception void.
 *  @return f_int32_t.
 * @see
 * @note
*/
f_int32_t getSysClkRate()
{
    // win32
#ifdef WIN32
    return(1000);
    // linux
#elif defined(__ANDROID__) || defined(ANDROID) || defined(__linux__)
    return(1000);    //to recheck
    // vxworks
#else
    return( sysClkRateGet() );
#endif
}

