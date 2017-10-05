#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
//#include "defines.h"
#include "../mmp_user.h"
#include "../mmp_init.h"
#include "../lat.h"
#include "../flush.h"
#define VEC_SZ 65536
#define ITEM_COUNT 1000000
#define LOGSIZE 1000000

using namespace std;

//typedef mystring ch[256];
typedef std::pair <int, char *> Int_Pair;
typedef std::pair <int, char *> dual;
dual log[LOGSIZE];
int numa;

double timer_begin,timer_end,sum;
//rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

int build_array(vector<char *>& a, int n)
{
    char ch[256]={};
    //ch[21]='\0';
    srand(time(NULL));
    //memset(c);
    for (int i=0; i < n; i++)
    {
        a[i]=malloc(256);
        for (int j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        //a[i]=rand();
        strcpy(a[i],ch);
    }
    for (int i=0;i<LOGSIZE;i++)
    {
        log[i].second=malloc(256);
    }
    return 0;
}

void array_swap(vector<char *>& a, int n, int i)
{
  //mcsim_skip_instrs_begin();
  int  k1, k2;
  //mystring temp;
  char temp[256],a1[256],a2[256];
  //string a1,a2;
  long b[1000];

  srand(time(NULL)+i*i);
  k1 = rand() % n;
  k2 = rand() % n;

  ++numa%=LOGSIZE;
  log[numa].first=k1;
  strcpy(log[numa].second,a[k1]);
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((log[numa])));
  ++numa%=LOGSIZE;
  log[numa].first=k2;
  strcpy(log[numa].second,a[k2]);
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((log[numa])));
  //asm_mfence();

  //temp=a[k1];
  strcpy(temp,a[k1]);

  //a[k1] = a[k2];
  strcpy(a[k1],a[k2]);
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((a[k1])));
  //rt_mem->write_literal(&a1, sizeof(long), &a[k2]);
  //a[k2] = temp;
  strcpy(a[k2],temp);
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((a[k2])));
  //asm_mfence();
}

void print_array(vector<string>& a, int n, ofstream& file)
{
  int i;

  for (i = 0; i < n; i++)
    file << a[i] << endl;
}

int main(int argc, char **argv)
{
    int i,swaps,item_count;
    std::ifstream file1;
    file1.open("sps.txt");
    file1>>item_count>>swaps;
    file1.close();

    vector<char *> array(item_count);

    // Initialization: build an array with random intergers
    if (build_array(array, item_count)) {
        cerr << "Fails to build an array" << endl;
        return -1;
    }

  //mcsim_skip_instrs_end();

#ifdef SPS_DEBUG
    ofstream orig;
    orig.open("orig.debug");
    print_array(array, item_count, orig);
#endif

    sum=0;
    numa=0;
    int batch=1;
    for (int k=1;k<=batch;k++)
    {
        timer_begin=GetWallTime();
        // randomly swaps between entries
        for (i = 0; i < swaps; i++)
        {
            array_swap(array, item_count, i); // swap two entris at a time
            //printf("%d\n",i);
            //mcsim_mem_fence();  //clflush+fence
        }
        //printf("finish swaps\n");

        timer_end=GetWallTime();
        sum+=timer_end-timer_begin;

        //mcsim_skip_instrs_begin();
        //cout << "done swaps " << i << endl;

        // make sure log structures are not dummy, will not discard by compile+O3
        //cout << "dummy: undolog.size= " << undolog.size() << endl;
        //cout << "dummy: redolog.size= " << redolog.size() << endl;
    }

    printf("%.15lf\n",sum/batch);
    cout << "done swaps " << i << endl;

    for (int i=0;i<item_count;i++)
        free(array[i]);
    for (int i=0;i<LOGSIZE;i++)
        free(log[i].second);
#ifdef SPS_DEBUG
    ofstream now;
    now.open("now.debug");
    print_array(array, item_count, now);
#endif
  
    return 0;
    //mcsim_skip_instrs_end();
}
