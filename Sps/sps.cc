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
#include "sps.h"
//#include "defines.h"

using namespace std;

//typedef mystring ch[256];
typedef pair <int, string> Int_Pair;
typedef std::pair <int, string> dual;
dual log[LOGSIZE];
int numa;

double timer_begin,timer_end,sum;
//rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

int build_array(vector<string>& a, int n)
{
    int i,j;
    char ch[256]={};
    //ch[21]='\0';
    srand(time(NULL));
    //memset(c);
    for (i = 0; i < n; i++)
    {
        //for (j=0;j<255;j++)
        //    ch[j]='a'+rand()%26;
        for (j=0;j<255;j++)
            ch[j]='a'+rand()%26;
        a[i]=ch;
    }
    return 0;
}

void array_swap(vector<string>& a, int n, int i)
{
  //mcsim_skip_instrs_begin();
  int  k1, k2;
  //mystring temp;
  string temp,a1,a2;
  //string a1,a2;

  srand(time(NULL)+i*i);
  k1 = rand() % n;
  k2 = rand() % n;

  ++numa%=LOGSIZE;
  log[numa]=std::make_pair(k1,a[k1]);
  emulate_latency_ns_fence(1000);
  asm_clflush((intptr_t *)&((log[numa].first)));
  asm_clflush((intptr_t *)(&(log[numa].second[0])));
  asm_clflush((intptr_t *)(&(log[numa].second[0])+64));
  asm_clflush((intptr_t *)(&(log[numa].second[0])+128));
  asm_clflush((intptr_t *)(&(log[numa].second[0])+192));

  asm_mfence();
  ++numa%=LOGSIZE;
  log[numa]=std::make_pair(k2,a[k2]);
  emulate_latency_ns_fence(1000);
  asm_clflush((intptr_t *)&((log[numa].first)));
  asm_clflush((intptr_t *)(&(log[numa].second[0])));
  asm_clflush((intptr_t *)(&(log[numa].second[0])+64));
  asm_clflush((intptr_t *)(&(log[numa].second[0])+128));
  asm_clflush((intptr_t *)(&(log[numa].second[0])+192));
  asm_mfence();

  temp=a[k1];

  a[k1] = a[k2];
  emulate_latency_ns_fence(1000);
  asm_clflush((intptr_t *)&((a[k1])));
  asm_clflush((intptr_t *)(&(a[k1][0])+64));
  asm_clflush((intptr_t *)(&(a[k1][0])+128));
  asm_clflush((intptr_t *)(&(a[k1][0])+192));
  asm_mfence();

  //rt_mem->write_literal(&a1, sizeof(long), &a[k2]);
  a[k2] = temp;
  emulate_latency_ns_fence(1000);
  asm_clflush((intptr_t *)&((a[k2])));
  asm_clflush((intptr_t *)(&(a[k2][0])+64));
  asm_clflush((intptr_t *)(&(a[k2][0])+128));
  asm_clflush((intptr_t *)(&(a[k2][0])+192));
  asm_mfence();
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

  vector<string> array(item_count);
  map<int, string> undolog, redolog;

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
  int batch=100;
  for (int k=1;k<=batch;k++)
  {
  timer_begin=GetWallTime();
  // randomly swaps between entries
  for (i = 0; i < swaps; i++)
  {
    array_swap(array,  item_count, i); // swap two entris at a time
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
#ifdef SPS_DEBUG
  ofstream now;
  now.open("now.debug");
  print_array(array, item_count, now);
#endif

  return 0;
  //mcsim_skip_instrs_end();
}
