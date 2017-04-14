#include <pthread.h>
#include <time.h>
#include <iostream>

#include "mmp_user.h"

void *LoopTransfer(void *rt_mem_p)
{
    rt_mem_t *rt_mem = (rt_mem_t *) rt_mem_p;
    struct timespec sleep_spec;
    sleep_spec.tv_sec = 0;
    sleep_spec.tv_nsec = 50000000;
    while(!rt_mem->appfinish)
    {
        //Transfer every 50 milliseconds for dram shenanigans
        //std::cout<<"trans"<<std::endl;
        rt_mem->do_transfer();
        //rt_mem->check_self();
        nanosleep(&sleep_spec, NULL);
    }
    rt_mem->do_transfer();

    pthread_exit(NULL);
}
