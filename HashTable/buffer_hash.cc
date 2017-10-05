#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "hash.h"

double timer_begin,timer_end,sum;
int ops,hashtable_size;
rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

DataItem* hashArray;
DataItem dummyItem;
DataItem* item;

int hashCode(int key) {
    return key % hashtable_size;
}

DataItem *search(int key)
{
    DataItem temp;
   //get the hash
    int i,hashIndex = hashCode(key);
    i=hashIndex;
    //move in array until an empty
    do
    {
        temp = *((DataItem *) rt_mem->read(&hashArray[i]));
        //if(hashArray[i].key == key)
        //    return &hashArray[i];
        if(temp.key == key)
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
    DataItem cur;
   //struct DataItem item = (struct DataItem*) malloc(sizeof(struct DataItem));
    DataItem *it;
    int i,hashIndex = hashCode(temp.key);
    //i=hashIndex;
    //move in array until an empty
    //cout<<hashIndex<<endl;
    i=hashIndex;
    cur = *((DataItem *) rt_mem->read(&hashArray[i]));
    while(cur.key != -1)
    {
        i++;
        i%=hashtable_size;
        if (i==hashIndex) return;
        cur = *((DataItem *) rt_mem->read(&hashArray[i]));
    }
    rt_mem->write_literal(&temp, sizeof(DataItem), &hashArray[i]);
}

void deleteH(DataItem* item)
{
   int key = item->key;
   rt_mem->write_literal(&dummyItem, sizeof(DataItem), item);
   return ;

}

void display()
{
   int i = 0;
   for(i = 0; i<hashtable_size; i++) {
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
    int i,j,search_key;
    DataItem temp;
    DataItem* it;
    char ch[256]={};
    std::ifstream file1;
    file1.open("hashtable.txt");
    dummyItem.key=-1;
    file1>>hashtable_size>>ops;
    file1.close();
    hashArray = (DataItem*) calloc(sizeof(DataItem),hashtable_size);
    //dummyItem = (struct DataItem*) malloc(sizeof(struct DataItem));
    dummyItem.key = -1;

    buildH();
    //display();
    timer_begin=GetWallTime();
    for (i=1;i<=ops;i++)
    {
        temp.key=rand()%(hashtable_size);
        //temp.data=rand();
        for (int j=0;j<255;j++)
            ch[j]='a'+rand()%26;
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
        //std::cout<<i<<std::endl;
    }
    //rt_mem->appfinish=1;
    timer_end=GetWallTime();
    sum+=timer_end-timer_begin;
    printf("time: %.15lf\n",sum);
    //display();

   return 0;
}
