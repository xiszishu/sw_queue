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
typedef pair <int, long> Int_Pair;
typedef std::pair <int, int> dual;
dual log[LOGSIZE];
int numa;

double timer_begin,timer_end,sum;
//rt_mem_t *rt_mem = get_mmp_initializer()->initialize();

int build_array(vector<long>& a, int n)
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
        a[i]=rand();
        //cout<<sizeof(a[i])<<endl;
        //cout<<a[i]<<endl;
        //cout<<"****************"<<endl;
    }
    return 0;
}

void array_swap(vector<long>& a, map<int, long>& undolog, map<int, long>& redolog, int n, int i)
{
  //mcsim_skip_instrs_begin();
  int  k1, k2;
  //mystring temp;
  long temp,a1,a2;
  //string a1,a2;
  long b[1000];

  srand(time(NULL)+i*i);
  k1 = rand() % n;
  k2 = rand() % n;

  //cout << "swaps a[" << k1 << "] and a[" << k2 << "]" << endl;
  //mcsim_skip_instrs_end();
  // mcsim_log_begin();
  //undolog.insert(Int_Pair(k1, a[k1]));
  //undolog.insert(Int_Pair(k2, a[k2]));
  //redolog.insert(Int_Pair(k1, a[k2]));
  //redolog.insert(Int_Pair(k2, a[k1]));
  //mcsim_log_end();
  //mcsim_mem_fence(); // clflush+fence
  //temp  = a[k1];
  ++numa%=LOGSIZE;
  log[numa]=std::make_pair(k1,a[k1]);
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((log[numa])));
  ++numa%=LOGSIZE;
  log[numa]=std::make_pair(k2,a[k2]);
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((log[numa])));
  //asm_mfence();

  temp=a[k1];

  //a1 = *((long *) rt_mem->read(&a[k1]));
  //a2 = *((long *) rt_mem->read(&a[k2]));
  //rt_mem->write_literal((void *) ((long) b[k2]), sizeof(long), &b[k1]);
  //rt_mem->write_literal((void *) ((long) b[k1]), sizeof(long), &b[k2]);

  //rt_mem->txend();

  //rt_mem->write_literal(&a2, sizeof(long), &a[k1]);
  a[k1] = a[k2];
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((a[k1])));
  //rt_mem->write_literal(&a1, sizeof(long), &a[k2]);
  a[k2] = temp;
  emulate_latency_ns_fence(1000);
  //asm_clflush((intptr_t *)&((a[k2])));
  //asm_mfence();
}

void print_array(vector<long>& a, int n, ofstream& file)
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

  vector<long> array(item_count);
  map<int, long> undolog, redolog;

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
    array_swap(array, undolog, redolog, item_count, i); // swap two entris at a time
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
