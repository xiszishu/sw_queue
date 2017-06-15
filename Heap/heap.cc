#include <stdio.h>
#include <iostream>
#include <time.h>
#include <iostream>
#include <fstream>
#include "../lat.h"
#include "../flush.h"
#define LOGSIZE 1000000

struct DataItem
{
    int key;
    int data;
};
typedef struct DataItem DataItem;

double timer_begin,timer_end,sum;
typedef std::pair <DataItem*, DataItem> dual;
dual log[LOGSIZE];
int numa;

DataItem h[1000000];
int heapsize,size,ops;
void init()
{
    heapsize=0;
    h[0].key=-1;
    h[0].data=-1;
}
void insert(DataItem ele)
{
    int i;
    h[++heapsize]=ele;
    i=heapsize;
    while (h[i/2].key>ele.key)
    {
        h[i]=h[i/2];
        asm_clflush((intptr_t *)&((h[i])));
        ++numa%=LOGSIZE;
        log[numa]=std::make_pair(&h[i],h[i]);
        asm_clflush((intptr_t *)&((log[numa])));
        asm_mfence();
        i/=2;
    }
    h[i]=ele;
    asm_clflush((intptr_t *)&((h[i])));
    ++numa%=LOGSIZE;
    log[numa]=std::make_pair(&h[i],ele);
    asm_clflush((intptr_t *)&((log[numa])));
    asm_mfence();
}
DataItem Delete()
{
    int i,j;
    DataItem minEle=h[1],nowEle=h[heapsize];
    heapsize--;
    for (i=1;i*2<=heapsize;i=j)
    {
        j=i*2;
        if (j!=heapsize && h[j+1].key>h[j].key)
        {
            j++;
        }
        if (nowEle.key>h[j].key)
        {
            h[i]=h[j];
            asm_clflush((intptr_t *)&((h[i])));
            ++numa%=LOGSIZE;
            log[numa]=std::make_pair(&h[i],h[i]);
            asm_clflush((intptr_t *)&((log[numa])));
            asm_mfence();
            //emulate_latency_ns_fence(2000);
        }
        else break;
    }
    h[j]=nowEle;
    asm_clflush((intptr_t *)&((h[j])));
    //emulate_latency_ns_fence(2000);

    ++numa%=LOGSIZE;
    log[numa]=std::make_pair(&h[j],nowEle);
    asm_clflush((intptr_t *)&((log[numa])));
    asm_mfence();
    //emulate_latency_ns_fence(2000);

    return minEle;
}
int main()
{
    int i,j;
    DataItem temp;
    init();
    std::ifstream file1;
    file1.open("heap.txt");
    file1>>size>>ops;
    numa=0;

    timer_begin=GetWallTime();
    for (i=1;i<=size;i++)
    {
        temp.key=rand()%(size);
        temp.data=rand();
        insert(temp);
    }
    for (i=1;i<=ops;i++)
    {
        if (i%2)
        {
            temp=Delete();
            //printf("%d %d\n",temp.key,temp.data);
        }
        else
        {
            temp.key=rand()%(size);
            temp.data=rand();
            insert(temp);
        }
    }
    timer_end=GetWallTime();
    sum+=timer_end-timer_begin;
    printf("time: %.15lf\n",sum);

    return 0;
}
