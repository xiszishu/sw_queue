#include <stdio.h>
#include <iostream>
#include <time.h>
#include <iostream>
#include <fstream>
#include "../mmp_user.h"
#include "../mmp_init.h"
#include "../lat.h"

#define LOGSIZE 1000000

rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

struct DataItem
{
    int key;
    int data;
};
typedef struct DataItem DataItem;

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
    DataItem a;
    h[++heapsize]=ele;
    i=heapsize;
    a=  *((DataItem *) rt_mem->read(&h[i/2]));
    while (a.key>ele.key)
    {
        //h[i]=h[i/2];
        rt_mem->write_literal(&a, sizeof(long), &h[i]);
        i/=2;
        a=  *((DataItem *) rt_mem->read(&h[i/2]));
    }
    //h[i]=ele;
    rt_mem->write_literal(&ele,sizeof(DataItem), &h[i]);
}
DataItem Delete()
{
    int i,j;
    DataItem a,b;
    DataItem minEle=h[1],nowEle=h[heapsize];
    heapsize--;
    for (i=1;i*2<=heapsize;i=j)
    {
        j=i*2;
        a=  *((DataItem *) rt_mem->read(&h[j]));
        b=  *((DataItem *) rt_mem->read(&h[j+1]));
        if ((j!=heapsize)&&(b.key>a.key))
        {
            j++;
            a=b;
        }
        if (nowEle.key>a.key)
        {
            //h[i]=h[j];
            rt_mem->write_literal(&h[j], sizeof(long), &h[i]);
        }
        else break;
    }
    //h[j]=nowEle;
    rt_mem->write_literal(&nowEle, sizeof(long), &h[j]);

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
    rt_mem->appfinish=1;
    pthread_join(rt_mem->th1, NULL);
    timer_end=GetWallTime();
    sum+=timer_end-timer_begin;
    printf("time: %.15lf\n",sum);

    return 0;
}
