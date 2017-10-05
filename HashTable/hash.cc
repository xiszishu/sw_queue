#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "../lat.h"
#include "../flush.h"
#include "hash.h"

dual log[LOGSIZE];
int numa;

int ops,hashtable_size;
double timer_begin,timer_end,sum;

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
    while(hashArray[i].key!=-1)
    {
        i++;
        i%=hashtable_size;
        if (i==hashIndex) return;
    }
    //hashArray[i].data=temp.data;
    strcpy(hashArray[i].data,temp.data);
    hashArray[i].key=temp.key;
    flush((intptr_t *)&hashArray[i],sizeof(DataItem));
    asm_mfence();

    ++numa%=LOGSIZE;
    log[numa]=std::make_pair(&hashArray[i],temp);
    flush((intptr_t *)&log[numa],sizeof(dual));
    asm_mfence();
    //rt_mem->write_literal(&temp, sizeof(DataItem), &hashArray[i]);
    //item->data = data;
    //item->key = key;

    //get the hash
    //int i,hashIndex = hashCode(key);

    //move in array until an empty or deleted cell
    // while(hashArray != NULL && hashArray[hashIndex].key != -1)
    // {
    //    //go to next cell
    //    ++hashIndex;
    //    //wrap around the table
    //    hashIndex %= SIZE;
    // }
    // hashArray[hashIndex] = item;
}

void deleteH(DataItem* item)
{
   int key = item->key;

   //deletenum++;
   //get the hash
   //struct DataItem temp = *item;
   //assign a dummy item at deleted position
   *item = dummyItem;
   flush((intptr_t *)item,sizeof(DataItem));
   asm_mfence();

   ++numa%=LOGSIZE;
   log[numa]=std::make_pair(item,dummyItem);
   flush((intptr_t *)&log[numa],sizeof(dual));
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
        //printf("search key:%d\n",hashArray[i-1].key);
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
    //dummyItem.data=-1;
    dummyItem.key=-1;
    file1>>hashtable_size>>ops;
    file1.close();
    hashArray = (DataItem*) calloc(sizeof(DataItem),hashtable_size);
    //dummyItem = (struct DataItem*) malloc(sizeof(struct DataItem));
    //dummyItem.data = -1;
    dummyItem.key=-1;
    numa=0;

    buildH();
    //display();
    timer_begin=GetWallTime();

    for (i=1;i<=ops;i++)
    {
        temp.key=rand()%(hashtable_size);
        //printf("rand key:%d\n",temp.key);
        //temp.data=rand();
        for (int j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        strcpy(temp.data,ch);

        it=search(temp.key);
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
    //printf("deleted:%d\n",deletenum);
    //display();
    return 0;
}
