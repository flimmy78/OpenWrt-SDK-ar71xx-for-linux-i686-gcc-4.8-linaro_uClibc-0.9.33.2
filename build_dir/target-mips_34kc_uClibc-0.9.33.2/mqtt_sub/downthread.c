#include "downthread.h"
#include "listmanager.h"
#include "ramrt.h"
#define DRIVE_COM1

void* thread_work(void* pvoid)
{
    thread_run = 1;
    while (thread_run)
    {
        //                printf("thread_work::thread_run = %d \n", thread_run);
        if (cmd_list != NULL && cmd_list->iAllNum > 0 && cmd_list->iAllNum < MAXLISTSIZE)
        {
            //            printf("list  allnum=%d \n",cmd_list->iAllNum);
            tagTaskItem *item ;
            pthread_mutex_lock(&down_work_mutex);
            item= list_begin(cmd_list);
            printf("list first item data = %s,length = %d,iallnum=%d\n",item->data,item->length,cmd_list->iAllNum);

            if ((strstr(item->data,"Control") != NULL)) //如果是驱动1的下发指令
            {
                printf("control test1111!!!!!!!\n");
                if (0 == getDownDriver3())
                {
                    setDownDriver3(1);
                    setDownCmdLengthCom1(item->length);
                    setDownCmdCom1(item->data);
                    printf("control test2222!!!!!!!\n");
                }
            }
            else if ((strstr(item->data,"Control2") != NULL))//如果是驱动2的下发指令
            {
                if (0 == getDownDriver4())
                {
                    setDownDriver4(1);
                    setDownCmdLengthCom2(item->length);
                    setDownCmdCom2(item->data);
                }
            }

            if ((strstr(item->data,"UploadAll") != NULL))
            {
                printf("UploadAll\n");
#ifdef DRIVE_COM1
                if(!getDownDriver1())
                {
                setDownDriver1(2);
                setDownCmdLengthCom1(item->length);
                setDownCmdCom1(item->data);
                }
#else
                if(!getDownDriver2())//
                {
                setDownDriver2(2);
                setDownCmdLengthCom2(item->length);
                setDownCmdCom2(item->data);
                }
#endif
            }
           // printf("list first item data = %s,length = %d,iallnum=%d\n",item->data,item->length,cmd_list->iAllNum);
            pop_front(cmd_list);//删除最前的一个元素
            printf("delete first item \n");
            pthread_mutex_unlock(&down_work_mutex);
        }
        sleep(THREAD_SLEEP_SPAN);
    }
    printf("thread is stop\n");
}

int down_thread_init()
{
    int res = pthread_mutex_init(&down_work_mutex, NULL);
    if ( 0 != res)
    {
        printf("Mutex init failed!");
        return 0;
    }
    res = pthread_create(&thread_down, NULL, thread_work, NULL);
    if ( 0 != res)
    {
        printf("Thread creation failed!");
        return 0;
    }
    return 1;
}
