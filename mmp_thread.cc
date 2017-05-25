#include <pthread.h>
#include <time.h>
#include <iostream>

#include "mmp_user.h"

void *LoopTransfer(void *rt_mem_p)
{
    int i=0;
    rt_mem_t *rt_mem = (rt_mem_t *) rt_mem_p;
    struct timespec sleep_spec;
    sleep_spec.tv_sec = 0;
    sleep_spec.tv_nsec = 5000000;
    //std::cout<<"start transfer"<<std::endl;
    while(!rt_mem->appfinish)
    {
        //Transfer every 50 milliseconds for dram shenanigans
        //std::cout<<"trans"<<std::endl;
        rt_mem->do_transfer();
        // i++;
        // if (i==5)
        // {
        //     rt_mem->gac();
        //     i=0;
        // }
        //printf("i:%d\n",i);
        //rt_mem->check_self();
        nanosleep(&sleep_spec, NULL);
    }
    //std::cout<<"reach here"<<std::endl;
    rt_mem->do_transfer();

    pthread_exit(NULL);
}
