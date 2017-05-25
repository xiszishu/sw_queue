#include "lat.h"
#include <iostream>
//double timer_begin,timer_end,sum;
int main()
{
    int i;
    timer_begin=GetWallTime();
    for (i=0;i<1000000;i++)
    emulate_latency_ns_fence(20000);
    timer_end=GetWallTime();
    sum+=timer_end-timer_begin;
    printf("time: %.15lf\n",sum);

    return 0;
}
