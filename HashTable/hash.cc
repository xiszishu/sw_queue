#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <fstream>
// #include "../mmp_user.h"
// #include "../mmp_init.h"
// #include "../lat.h"
// #include "../flush.h"
#include "./hash.h"

typedef std::pair <DataItem*, DataItem> dual;
double timer_begin,timer_end,sum;
dual log[LOGSIZE];
int numa;

int ops,hashtable_size;

DataItem* hashArray;
DataItem dummyItem;
DataItem* item;

int hashCode(int key)
{
    return key % hashtable_size;
}

DataItem *search(int key)
{
   //get the hash
    int i,hashIndex = hashCode(key);
    i=hashIndex;
    //move in array until an empty
    do
    {
        if(hashArray[i].key == key)
           return &hashArray[i];
        //go to next cell
        ++i;
        //wrap around the table
        i%= hashtable_size;
    }while(hashIndex!=i);
    return NULL;
}

void insertH(DataItem temp)
{
    //struct DataItem item = (struct DataItem*) malloc(sizeof(struct DataItem));
    DataItem *it;
    int i,hashIndex = hashCode(temp.key);
    //i=hashIndex;
    //move in array until an empty
    //cout<<hashIndex<<endl;
    i=hashIndex;
    while(hashArray[i].key != -1)
    {
        i++;
        i%=hashtable_size;
        if (i==hashIndex) return;
    }
    //hashArray[i].data=temp.data;
    strcpy(hashArray[i].data,temp.data);
    hashArray[i].key=temp.key;
    emulate_latency_ns_fence(40000);
    flush((intptr_t *)&((hashArray[i].key)),64);
    flush((intptr_t *)(&(hashArray[i].data[0])),256);

    asm_mfence();

    ++numa%=LOGSIZE;
    log[numa]=std::make_pair(&hashArray[i],temp);
    emulate_latency_ns_fence(40000);
    //asm_clflush((intptr_t *)&((log[numa])));
    flush((intptr_t *)&((log[numa].first)),64);
    flush((intptr_t *)&((log[numa].second.key)),64);
    flush((intptr_t *)(&(log[numa].second.data[0])),256);

    asm_mfence();

    return;
}

void deleteH(DataItem* item)
{
   int key = item->key;

   //get the hash
   //struct DataItem temp = *item;
   //assign a dummy item at deleted position
   *item = dummyItem;
   emulate_latency_ns_fence(40000);
   flush((intptr_t *)&((item->key)),64);
   flush((intptr_t *)(&(item->data[0])),256);
   asm_mfence();

   ++numa%=LOGSIZE;
   log[numa]=std::make_pair(item,dummyItem);
   emulate_latency_ns_fence(40000);
   flush((intptr_t *)&((log[numa].first)),64);
   flush((intptr_t *)&((log[numa].second.key)),64);
   flush((intptr_t *)(&(log[numa].second.data[0])),256);
   asm_mfence();
   //rt_mem->write_literal(&dummyItem, sizeof(DataItem), item);
   return ;
}

void display()
{
   int i = 0;
   for(i = 0; i<hashtable_size; i++)
   {
       //if(hashArray[i] != NULL)
         printf(" (%d,%d)",hashArray[i].key,hashArray[i].data);
         //else
         //printf(" ~~ ");
   }

   printf("\n");
}
void buildH()
{
    char ch[256]={};
    srand(time(NULL));
    for (int i=1;i<=hashtable_size;i++)
    {
        hashArray[i-1].key=i-1;
        //hashArray[i-1].data=rand();
        //cout<<i<<endl;
        for (int j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        strcpy(hashArray[i-1].data,ch);
    }
}
int main()
{
    int search_key;
    DataItem temp;
    DataItem* it;
    std::ifstream file1;
    char ch[256]={};
    file1.open("hashtable.txt");
    //dummyItem.data=-1;
    dummyItem.key=-1;
    file1>>hashtable_size>>ops;
    file1.close();
    hashArray = (DataItem*) calloc(sizeof(DataItem),hashtable_size);

    dummyItem.key = -1;
    numa=0;

    buildH();
    //display();
    timer_begin=GetWallTime();

    for (int i=1;i<=ops;i++)
    {
        temp.key=rand()%(hashtable_size);
        for (int j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        //temp.data=ch;
        strcpy(temp.data,ch);
        it=search(search_key);
        //cout<<"-----------"<<endl;
        if (it==NULL)
        {
            insertH(temp);
        }
        else
        {
            deleteH(it);
        }
    }
    timer_end=GetWallTime();
    sum+=timer_end-timer_begin;
    printf("time: %.15lf\n",sum);
    //display();
    return 0;
}
